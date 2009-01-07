// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
		\brief Implements the classes for handling micropolygrids and micropolygons.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	"aqsis.h"
#include	"stats.h"
#include	"imagebuffer.h"
#include	"micropolygon.h"
#include	"renderer.h"
#include	"surface.h"
#include	"lights.h"
#include	"shaders.h"
#include	"trimcurve.h"
#include	"spline.h"
#include	"bucketprocessor.h"

#include	"mpdump.h"

namespace Aqsis {


CqObjectPool<CqMicroPolygon> CqMicroPolygon::m_thePool;
CqObjectPool<CqMovingMicroPolygonKey>	CqMovingMicroPolygonKey::m_thePool;

void CqMicroPolyGridBase::CacheGridInfo()
{
	m_CurrentGridInfo.m_IsMatte = this->pAttributes() ->GetIntegerAttribute( "System", "Matte" ) [ 0 ] == 1;

	// this is true if the mpgs can safely be occlusion culled.
	m_CurrentGridInfo.m_IsCullable = !( QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "DisplayMode" ) [ 0 ] & ModeZ ) && !(this->pCSGNode());

	m_CurrentGridInfo.m_UsesDataMap = !(QGetRenderContext() ->GetMapOfOutputDataEntries().empty());

	m_CurrentGridInfo.m_ShadingRate = this->pAttributes() ->GetFloatAttribute( "System", "ShadingRate" ) [ 0 ];
	m_CurrentGridInfo.m_ShutterOpenTime = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Shutter" ) [ 0 ];
	m_CurrentGridInfo.m_ShutterCloseTime = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Shutter" ) [ 1 ];

	m_CurrentGridInfo.m_LodBounds = this->pAttributes() ->GetFloatAttribute( "System", "LevelOfDetailBounds" );
}


//---------------------------------------------------------------------
/** Default constructor
 */

CqMicroPolyGrid::CqMicroPolyGrid() : CqMicroPolyGridBase(),
		m_bShadingNormals( false ),
		m_bGeometricNormals( false ), 
		m_pShaderExecEnv(new CqShaderExecEnv(QGetRenderContextI()))
{
	STATS_INC( GRD_allocated );
	STATS_INC( GRD_current );
	STATS_INC( GRD_allocated );
	TqInt cGRD = STATS_GETI( GRD_current );
	TqInt cPeak = STATS_GETI( GRD_peak );
	STATS_SETI( GRD_peak, cGRD > cPeak ? cGRD : cPeak );
}


//---------------------------------------------------------------------
/** Destructor
 */

CqMicroPolyGrid::~CqMicroPolyGrid()
{
	assert( RefCount() <= 0 );

	STATS_INC( GRD_deallocated );
	STATS_DEC( GRD_current );

	// Delete any cloned shader output variables.
	std::vector<IqShaderData*>::iterator outputVar;
	for( outputVar = m_apShaderOutputVariables.begin(); outputVar != m_apShaderOutputVariables.end(); outputVar++ )
		if( (*outputVar) )
			delete( (*outputVar) );
	m_apShaderOutputVariables.clear();
}


//---------------------------------------------------------------------
/** Initialise the grid ready for processing.
 * \param cu Integer grid resolution.
 * \param cv Integer grid resolution.
 * \param pSurface CqSurface pointer to associated GPrim.
 */

void CqMicroPolyGrid::Initialise( TqInt cu, TqInt cv, const boost::shared_ptr<CqSurface>& pSurface )
{
	// Initialise the shader execution environment
	TqInt lUses = -1;
	if ( pSurface )
	{
		lUses = pSurface->Uses();
		m_pSurface = pSurface;

		m_pCSGNode = pSurface->pCSGNode();
	}
	lUses |= QGetRenderContext()->pDDmanager()->Uses();

	/// \note This should delete through the interface that created it.

	m_pShaderExecEnv->Initialise( cu, cv, numMicroPolygons(cu, cv), numShadingPoints(cu, cv), hasValidDerivatives(), pSurface->pAttributes(), pSurface->pTransform(), pSurface->pAttributes()->pshadSurface(QGetRenderContext()->Time()).get(), lUses );

	boost::shared_ptr<IqShader> pshadSurface = pSurface ->pAttributes() ->pshadSurface(QGetRenderContext()->Time());
	boost::shared_ptr<IqShader> pshadDisplacement = pSurface ->pAttributes() ->pshadDisplacement(QGetRenderContext()->Time());
	boost::shared_ptr<IqShader> pshadAtmosphere = pSurface ->pAttributes() ->pshadAtmosphere(QGetRenderContext()->Time());

	if ( pshadSurface )
		pshadSurface->Initialise( cu, cv, numShadingPoints(cu, cv), m_pShaderExecEnv.get() );
	if ( pshadDisplacement )
		pshadDisplacement->Initialise( cu, cv, numShadingPoints(cu, cv), m_pShaderExecEnv.get() );
	if ( pshadAtmosphere )
		pshadAtmosphere->Initialise( cu, cv, numShadingPoints(cu, cv), m_pShaderExecEnv.get() );

	// Initialise the local/public culled variable.
	m_CulledPolys.SetSize( numShadingPoints(cu, cv) );
	m_CulledPolys.SetAll( false );

	TqInt size = numMicroPolygons(cu, cv);
	CacheGridInfo();

	STATS_INC( GRD_size_4 + clamp<TqInt>(CqStats::stats_log2(size) - 2, 0, 7) );
}

//---------------------------------------------------------------------
/** Build the normals list for the micropolygons in the grid.
 */

void CqMicroPolyGrid::CalcNormals()
{
	if ( NULL == pVar(EnvVars_P) || NULL == pVar(EnvVars_N) )
		return ;

	// We flip the normals if the 'current orientation' differs from the
	// 'coordinate system orientation' - see RiSpec 'Orientation and Sides'
	//
	// The RiSpec is a little tricky to interpret on this point, so here's a
	// summary of their intent using a sphere as an example:
	//
	// No matter which coordinate transformation you perform, the normals of a
	// sphere should always end up pointing outwards from the centre by
	// default.  To make them point inwards, use RiReverseOrientation().
	//
	// The mess happens since you want this to be true even when using
	// transformations which change the handedness, such as RiScale(1,1,-1).
	// By convention, the normal is
	//
	//   N = dPdu x dPdv
	//
	// This is fine in the default coordinate system.  However, the direction
	// of the cross product must be reversed if the formula is to give the
	// correct normal after RiScale(1,1,-1) or similar transformations.
	bool CSO = this->pSurface()->pTransform()->GetHandedness(this->pSurface()->pTransform()->Time(0));
	bool O = pAttributes() ->GetIntegerAttribute( "System", "Orientation" ) [ 0 ] != 0;
	bool flipNormals = O ^ CSO;

	const CqVector3D* vecMP[ 4 ];
	CqVector3D	vecN, vecTemp;
	CqVector3D	vecFailsafeN;

	const CqVector3D* pP;
	pVar(EnvVars_P) ->GetPointPtr( pP );
	IqShaderData* pNg = pVar(EnvVars_Ng);

	// Calculate each normal from the top left, top right and bottom left points.
	register TqInt ur = uGridRes();
	register TqInt vr = vGridRes();

	// Create a failsafe normal from the corners of the grid, in case we encounter degenerate MP's
	vecFailsafeN = ( pP[ur] - pP[0] ) % ( pP[(vr*(ur+1))+ur] - pP[0] );
	vecFailsafeN.Unit();

	TqInt igrid = 0;
	TqInt iv;
	for ( iv = 0; iv < vr; iv++ )
	{
		TqInt iu;
		for ( iu = 0; iu < ur; iu++ )
		{
			vecMP[ 0 ] = &pP[ igrid ];
			vecMP[ 1 ] = &pP[ igrid + 1 ];
			vecMP[ 2 ] = &pP[ igrid + ur + 2 ];
			vecMP[ 3 ] = &pP[ igrid + ur + 1];
			TqInt a=0, b=1, c=2;
			CqVector3D vecBA = ( *vecMP[ b ] ) - ( *vecMP[ a ] );
			CqVector3D vecCA = ( *vecMP[ c ] ) - ( *vecMP[ a ] );
			TqFloat bma = vecBA.Magnitude();
			TqFloat cma = vecCA.Magnitude();
			if( bma < FLT_EPSILON )
			{
				b = 3;
				vecBA = ( *vecMP[ b ] ) - ( *vecMP[ a ] );
				bma = vecBA.Magnitude();
			}


			if( ( bma > FLT_EPSILON ) &&
			        ( cma > FLT_EPSILON ) &&
			        ( vecBA != vecCA ) )
			{
				vecN = vecBA % vecCA;	// Cross product is normal.*/
				vecN.Unit();
				if(flipNormals)
					vecN = -vecN;
			}
			else
			{
				//assert(false);
				vecN = vecFailsafeN;
			}

			pNg->SetNormal( vecN, igrid );
			igrid++;
			// If we are at the last row, last row normal to the same.
			if ( iv == vr - 1 )
			{
				CqVector3D vecNN( vecN );
				if ( vr > 2 )
				{
					CqVector3D vecNm1, vecNm2;
					pNg->GetNormal( vecNm1, ( ( vr - 1 ) * ( ur + 1 ) ) + iu );
					pNg->GetNormal( vecNm2, ( ( vr - 2 ) * ( ur + 1 ) ) + iu );
					vecNN = ( vecNm1 - vecNm2 ) + vecN;
				}
				pNg->SetNormal( vecNN, ( vr * ( ur + 1 ) ) + iu );
			}
		}
		// Set the last one on the row to the same.
		CqVector3D vecNN( vecN );
		if ( igrid > 2 )
		{
			CqVector3D vecNm1, vecNm2;
			pNg->GetNormal( vecNm1, igrid - 1 );
			pNg->GetNormal( vecNm2, igrid - 2 );
			vecNN = ( vecNm1 - vecNm2 ) + vecN;
		}
		pNg->SetNormal( vecNN, igrid );
		igrid++;
	}
	// Set the very last corner value to the last normal calculated.
	CqVector3D vecNN( vecN );
	if ( vr > 2 && ur > 2 )
	{
		CqVector3D vecNm1, vecNm2;
		pNg->GetNormal( vecNm1, ( vr - 1 ) * ( ur - 1 ) - 1 );
		pNg->GetNormal( vecNm2, ( vr - 2 ) * ( ur - 2 ) - 1 );
		vecNN = ( vecNm1 - vecNm2 ) + vecN;
	}
	pNg->SetNormal( vecNN, ( vr + 1 ) * ( ur + 1 ) - 1 );
}

void CqMicroPolyGrid::CalcSurfaceDerivatives()
{
	/// \todo <b>Code review</b>: This function should probably belong in the shaderexecenv.
	// It could then be easily modified to use the new centered difference functions in the shaderexecenv
	bool bdpu, bdpv;
	TqInt lUses = pSurface() ->Uses();
	bdpu = ( USES( lUses, EnvVars_dPdu ) );
	bdpv = ( USES( lUses, EnvVars_dPdv ) );
	IqShaderData * pSDP = pVar(EnvVars_P);
	static CqVector3D	Defvec( 0, 0, 0 );

	TqInt i;

	TqInt gsmin1 = m_pShaderExecEnv->shadingPointCount() - 1;
	for ( i = gsmin1; i >= 0; i-- )
	{
		if ( bdpu )
		{
			pVar(EnvVars_dPdu) ->SetVector( SO_DuType<CqVector3D>( pSDP, i, m_pShaderExecEnv.get(), Defvec ), i );
		}
		if ( bdpv )
		{
			pVar(EnvVars_dPdv) ->SetVector( SO_DvType<CqVector3D>( pSDP, i, m_pShaderExecEnv.get(), Defvec ), i );
		}
	}
}


//---------------------------------------------------------------------
void CqMicroPolyGrid::ExpandGridBoundaries(TqFloat amount)
{
	// Anti grid-crack hack!  We expand the grid by pushing the outer verts
	// along the vectors connecting themselves to the next layer of verts in:
	//
	//        ^  ^  ^
	//        |  |  |
	//
	//  <--   x--x--x ...
	//        |  |  |
	//  <--   x--x--x ...
	//        |  |  .
	//  <--   x--x.
	//        .
	//
	CqVector3D* pP;
	pVar(EnvVars_P)->GetPointPtr(pP);

	const TqInt numVertsU = uGridRes() + 1;
	const TqInt totVerts = numVertsU*(vGridRes() + 1);

	// Before expanding each edge, we perform a quick check to guess whether it
	// is degenerate.  We avoid checking the vertices of a row against each
	// other directly, since this will result in the usual sensitivity to
	// floating point errors.  Instead, we check whether the length of the row
	// is less than sqrt(degeneracyRatio) times the length of the following
	// row.
	const TqFloat degeneracyRatio = 1e-8;

	if((pP[0] - pP[numVertsU-1]).Magnitude2()
			> degeneracyRatio*(pP[numVertsU] - pP[2*numVertsU-1]).Magnitude2())
	{
		// Expand first u-row in -v direction.
		for(TqInt iu = 0; iu < numVertsU; ++iu)
			pP[iu] = (1+amount)*pP[iu] - amount*pP[iu+numVertsU];
	}
	if((pP[totVerts-numVertsU] - pP[totVerts-1]).Magnitude2()
			> degeneracyRatio*(pP[totVerts-2*numVertsU] - pP[totVerts-numVertsU-1]).Magnitude2())
	{
		// Expand last u-row in +v direction.
		for(TqInt iu = totVerts-numVertsU; iu < totVerts; ++iu)
			pP[iu] = (1+amount)*pP[iu] - amount*pP[iu-numVertsU];
	}
	if((pP[0] - pP[totVerts-numVertsU]).Magnitude2()
			> degeneracyRatio*(pP[1] - pP[totVerts-numVertsU+1]).Magnitude2())
	{
		// Expand first v-column in -u direction.
		for(TqInt iv = 0; iv < totVerts; iv += numVertsU)
			pP[iv] = (1+amount)*pP[iv] - amount*pP[iv+1];
	}
	if((pP[numVertsU-1] - pP[totVerts-1]).Magnitude2()
			> degeneracyRatio*(pP[numVertsU-2] - pP[totVerts-2]).Magnitude2())
	{
		// Expand last v-column in +u direction.
		for(TqInt iv = numVertsU-1; iv < totVerts; iv += numVertsU)
			pP[iv] = (1+amount)*pP[iv] - amount*pP[iv-1];
	}
}


//---------------------------------------------------------------------
/** Shade the grid using the surface parameters of the surface passed and store the color values for each micropolygon.
 */

void CqMicroPolyGrid::Shade( bool canCullGrid )
{
	// Sanity checks
	if ( NULL == pVar(EnvVars_P) || NULL == pVar(EnvVars_I) )
		return ;

	TqInt lUses = pSurface() ->Uses();
	TqInt gs = m_pShaderExecEnv->shadingPointCount();
	TqInt gsmin1 = gs - 1;

	// Expand grids to prevent grid cracking if enabled
	const TqFloat* gridExpand = pAttributes()->GetFloatAttribute("aqsis", "expandgrids");
	if(gridExpand && *gridExpand > 0)
		ExpandGridBoundaries(*gridExpand);

	const CqVector3D* pP;
	pVar(EnvVars_P) ->GetPointPtr( pP );
	const CqColor* pOs = NULL;
	if ( USES( lUses, EnvVars_Os ) )
		pVar(EnvVars_Os) ->GetColorPtr( pOs );
	const CqColor* pCs = NULL;
	if ( USES( lUses, EnvVars_Cs ) )
		pVar(EnvVars_Cs) ->GetColorPtr( pCs );
	IqShaderData* pI = pVar(EnvVars_I);
	const CqVector3D* pN = NULL;
	if ( USES( lUses, EnvVars_N ) )
		pVar(EnvVars_N) ->GetNormalPtr( pN );

	// Calculate geometric normals if not specified by the surface.
	if ( !bGeometricNormals() && USES( lUses, EnvVars_Ng ) )
		CalcNormals();

	// If shading normals are not explicitly specified, they default to the geometric normal.
	if ( !bShadingNormals() && USES( lUses, EnvVars_N ) && NULL != pVar(EnvVars_Ng) && NULL != pVar(EnvVars_N) )
		pVar(EnvVars_N) ->SetValueFromVariable( pVar(EnvVars_Ng) );

	// Set eye position - always at the origin in the shading coord system.
	if ( USES( lUses, EnvVars_E ) )
		pVar(EnvVars_E)->SetVector(CqVector3D(0, 0, 0));

	// Set du and dv if necessary.  This code assumes that du and dv are
	// constant across every grid. (Looks to be a good assumption at svn r2117.)
	/// \todo: Should this be a method of the shaderexecenv?
	if(USES(lUses, EnvVars_du))
		setDu();
	if(USES(lUses, EnvVars_dv))
		setDv();

	// Set I, the incident ray direction; this is just equal to P in shading
	// (camera) coords for a projective camera transformation, or (0,0,1) for
	// orthographic.
	switch(QGetRenderContext()->GetIntegerOption("System", "Projection")[0])
	{
		case ProjectionOrthographic:
			pI->SetVector(CqVector3D(0,0,1));
			break;
		case ProjectionPerspective:
		default:
			pI->SetValueFromVariable(pVar(EnvVars_P));
			break;
	}

	// Calculate surface derivatives if necessary.
	if ( USES( lUses, EnvVars_dPdu ) || USES( lUses, EnvVars_dPdv ) )
		CalcSurfaceDerivatives();

	// Initialize surface color Ci to black
	if ( USES( lUses, EnvVars_Ci ) )
		pVar(EnvVars_Ci) ->SetColor( gColBlack );
	// Initialize surface opacity Oi to opaque
	if ( USES( lUses, EnvVars_Oi ) )
		pVar(EnvVars_Oi) ->SetColor( gColWhite );

	if(USES(lUses, EnvVars_Os))
	{
		if(QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "DisplayMode" ) [ 0 ] & ModeZ)
		{
			// As we are rendering a standard depth map, anything that is semi-transparent
			// doesn't contribute, so remove any semi-transparent micropolygons.
			AQSIS_TIME_SCOPE(Transparency_culling_micropolygons);

			TqInt cCulled = 0;
			for (TqInt i = gsmin1; i >= 0; i-- )
			{
				if ( pOs[ i ] != gColWhite )
				{
					cCulled ++;
					m_CulledPolys.SetValue( i, true );
				}
				else
					break;
			}

			if ( canCullGrid && cCulled == gs )
			{
				m_fCulled = true;
				STATS_INC( GRD_culled );
				DeleteVariables( true );
				return ;
			}
		}
		if(QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "DisplayMode" ) [ 0 ] & ModeRGB)
		{
			// When rendering a color image, cull any micropolygons that are fully transparent.
			AQSIS_TIME_SCOPE(Transparency_culling_micropolygons);

			TqInt cCulled = 0;
			for (TqInt i = gsmin1; i >= 0; i-- )
			{
				if ( pOs[ i ] == gColBlack )
				{
					cCulled ++;
					m_CulledPolys.SetValue( i, true );
				}
				else
					break;
			}

			if ( canCullGrid && cCulled == gs )
			{
				m_fCulled = true;
				STATS_INC( GRD_culled );
				DeleteVariables( true );
				return ;
			}
		}
	}

	boost::shared_ptr<IqShader> pshadDisplacement = pSurface()->pAttributes()->pshadDisplacement(QGetRenderContext()->Time());
	if ( pshadDisplacement )
	{
		AQSIS_TIME_SCOPE(Displacement_shading);
		pshadDisplacement->Evaluate( m_pShaderExecEnv.get() );

		// Re-calculate geometric normals and surface derivatives after displacement.
		// \note: This is a bit overkill, might be a better way of doing it.
		if ( USES( lUses, EnvVars_Ng ) )
			CalcNormals();
		if ( USES( lUses, EnvVars_dPdu ) || USES( lUses, EnvVars_dPdv ) )
			CalcSurfaceDerivatives();
	}

	// Now try and cull any hidden MPs if Sides==1
	if ( ( pAttributes() ->GetIntegerAttribute( "System", "Sides" ) [ 0 ] == 1 ) && !m_pCSGNode )
	{
		AQSIS_TIME_SCOPE(Backface_culling);

		TqInt cCulled = 0;
		const CqVector3D* pNg = NULL;
		pVar(EnvVars_Ng) ->GetNormalPtr( pNg );
		// When backface culling, we must use the geometric normal (Ng) as
		// this is the normal that properly represents the actual micropolygon
		// geometry. However, if the primitive specifies custom normals as
		// primitive variables, the direction of those should be honored, in case
		// the user has intentionally switched the surface direction.
		// Therefore, we compare the direction of Ng with that of N and flip Ng 
		// if they don't match.
		for (TqInt i = gsmin1; i >= 0; i-- )
		{
			TqFloat s = 1.0f;
			if(NULL != pN)
				s = ( ( pN[i] * pNg[i] ) < 0.0f ) ? -1.0f : 1.0f;
			// Calulate the direction the MPG is facing.
			if ( ( ( s * pNg[ i ] ) * pP[ i ] ) >= 0 )
			{
				cCulled++;
				STATS_INC( MPG_culled );
				m_CulledPolys.SetValue( i, true );
			}
		}

		// If the whole grid is culled don't bother going any further.
		if ( canCullGrid && cCulled == gs )
		{
			m_fCulled = true;
			STATS_INC( GRD_culled );
			DeleteVariables( true );
			return ;
		}
	}

	// Now shade the grid.
	boost::shared_ptr<IqShader> pshadSurface = pSurface() ->pAttributes() ->pshadSurface(QGetRenderContext()->Time());
	if ( pshadSurface )
	{
		AQSIS_TIME_SCOPE(Surface_shading);
		m_pShaderExecEnv->SetCurrentSurface(pSurface());
		pshadSurface->Evaluate( m_pShaderExecEnv.get() );
	}

	// Perform atmosphere shading
	boost::shared_ptr<IqShader> pshadAtmosphere = pSurface()->pAttributes()->pshadAtmosphere(QGetRenderContext()->Time());
	if ( pshadAtmosphere )
	{
		AQSIS_TIME_SCOPE(Atmosphere_shading);
		pshadAtmosphere->Evaluate( m_pShaderExecEnv.get() );
	}

	// Cull any MPGs whose alpha is completely transparent after shading.
	if ( USES( lUses, EnvVars_Os ) && QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "DisplayMode" ) [ 0 ] & ModeRGB )
	{
		AQSIS_TIME_SCOPE(Transparency_culling_micropolygons);
		
		TqInt cCulled = 0;
		for (TqInt i = gsmin1; i >= 0; i-- )
		{
			if ( pOs[ i ] == gColBlack )
			{
				cCulled ++;
				m_CulledPolys.SetValue( i, true );
			}
			else
				break;
		}

		if ( cCulled == gs )
		{
			m_fCulled = true;
			STATS_INC( GRD_culled );
			DeleteVariables( true );
			return ;
		}
	}
	DeleteVariables( false );

	STATS_INC( GRD_shd_size_4 + clamp<TqInt>( CqStats::stats_log2(
					m_pShaderExecEnv->shadingPointCount() ) - 2, 0, 7 ) );
}

//---------------------------------------------------------------------
/** Transfer any shader variables marked as "otuput" as they may be needed by the display devices.
 */

void CqMicroPolyGrid::TransferOutputVariables()
{
	boost::shared_ptr<IqShader> pShader = this->pAttributes()->pshadSurface(QGetRenderContext()->Time());

	// Only bother transferring ones that have been used in a RiDisplay request.
	std::map<std::string, CqRenderer::SqOutputDataEntry>& outputVars = QGetRenderContext()->GetMapOfOutputDataEntries();
	std::map<std::string, CqRenderer::SqOutputDataEntry>::iterator outputVar;
	for( outputVar = outputVars.begin(); outputVar != outputVars.end(); outputVar++ )
	{
		IqShaderData* outputData = pShader->FindArgument( outputVar->first );
		if( NULL != outputData )
		{
			IqShaderData* newOutputData = outputData->Clone();
			m_apShaderOutputVariables.push_back( newOutputData );
		}
	}
}

//---------------------------------------------------------------------
/**
 * Delete unneeded variables so that we don't use up unnecessary memory
 */
void CqMicroPolyGrid::DeleteVariables( bool all )
{
   	IqDDManager *pManager = QGetRenderContext() ->pDDmanager();

	if ( all || ! pManager->fDisplayNeeds( "Cs" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Cs );
	if ( all || !pManager->fDisplayNeeds( "Os" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Os );
	if ( all || !pManager->fDisplayNeeds( "du" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_du );
	if ( all || !pManager->fDisplayNeeds( "dv" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_dv );
	if ( all || !pManager->fDisplayNeeds( "L" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_L );
	if ( all || !pManager->fDisplayNeeds( "Cl" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Cl );
	if ( all || !pManager->fDisplayNeeds( "Ol" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Ol );
	if ( all || !pManager->fDisplayNeeds( "dPdu" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_dPdu );
	if ( all || !pManager->fDisplayNeeds( "dPdv" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_dPdv );

	if ( all || !pManager->fDisplayNeeds( "s" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_s );
	if ( all || !pManager->fDisplayNeeds( "t" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_t );
	if ( all || !pManager->fDisplayNeeds( "I" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_I );

	if ( all || !pManager->fDisplayNeeds( "Ps" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Ps );
	if ( all || !pManager->fDisplayNeeds( "E" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_E );
	if ( all || !pManager->fDisplayNeeds( "ncomps" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_ncomps );
	if ( all || !pManager->fDisplayNeeds( "time" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_time );
	if ( all || !pManager->fDisplayNeeds( "alpha" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_alpha );

	// Needed by ?
	if ( all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_N );

	// (u,v) are needed by trim curves; need to work out how to check for their
	// existence.
	if ( all ) 		
		m_pShaderExecEnv->DeleteVariable( EnvVars_u );
	if ( all ) 		
		m_pShaderExecEnv->DeleteVariable( EnvVars_v );
	if ( all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_P );
	// Needed by backface culling.
	if ( all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Ng );

	if ( all || !pManager->fDisplayNeeds( "Ci" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Ci );
	if ( all || !pManager->fDisplayNeeds( "Oi" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Oi );
	if ( all || !pManager->fDisplayNeeds( "Ns" ) )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Ns );
}



//---------------------------------------------------------------------
/** Split the shaded grid into microploygons, and insert them into the relevant buckets in the image buffer.
 * \param xmin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param xmax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 */

void CqMicroPolyGrid::Split( long xmin, long xmax, long ymin, long ymax )
{
	if ( NULL == pVar(EnvVars_P) )
		return ;

	TqInt cu = uGridRes();
	TqInt cv = vGridRes();

	AQSIS_TIMER_START(Project_points);
	CqMatrix matCameraToRaster;
	QGetRenderContext() ->matSpaceToSpace( "camera", "raster", NULL, NULL, QGetRenderContext()->Time(), matCameraToRaster );
	CqMatrix matCameraToObject0;
	QGetRenderContext() ->matSpaceToSpace( "camera", "object", NULL, pSurface() ->pTransform().get(), QGetRenderContext()->Time(), matCameraToObject0 );

	// Transform the whole grid to hybrid camera/raster space
	CqVector3D* pP;
	pVar(EnvVars_P) ->GetPointPtr( pP );

	// Get an array of P's for all time positions, the transformation keyframes take 
	// into account transformation of the camera, as well as transformation of the object.
	// The list of keyframes is the union of the two.
	TqInt iTime;
	IqTransformPtr objectTransform = pSurface()->pTransform();
	CqTransformPtr cameraTransform = QGetRenderContext()->GetCameraTransform();
	TqInt objectTimes = objectTransform->cTimes();
	TqInt cameraTimes = cameraTransform->cTimes();
	std::map<TqFloat, TqInt> keyframeTimes;
	// Add all the object transformation times to the list of keyframe points.
	for(iTime = 0; iTime < objectTimes; iTime++)
		keyframeTimes[objectTransform->Time(iTime)] = iTime;
	if(cameraTimes > 1)
		for(iTime = 0; iTime < cameraTimes; iTime++)
			keyframeTimes[cameraTransform->Time(iTime)] = iTime;

	TqInt tTime = keyframeTimes.size();
	std::vector<std::vector<CqVector3D> > aaPtimes;
	aaPtimes.resize( tTime );

	CqMatrix matObjectToCameraT;
	register TqInt i;
	TqInt gsmin1;
	gsmin1 = m_pShaderExecEnv->shadingPointCount() - 1;

	std::map<TqFloat, TqInt>::iterator keyFrame;
	for ( keyFrame = keyframeTimes.begin(); keyFrame!=keyframeTimes.end(); keyFrame++ )
	{
		QGetRenderContext() ->matSpaceToSpace( "object", "camera", NULL, pSurface() ->pTransform().get(), keyFrame->first, matObjectToCameraT );
		aaPtimes[ keyFrame->second ].resize( gsmin1 + 1 );

		for ( i = gsmin1; i >= 0; i-- )
		{
			CqVector3D Point( pP[ i ] );

			// Only do the complex transform if motion blurred.
			//Point = matObjectToCameraT * matCameraToObject0 * Point;
			Point = matCameraToObject0 * Point;
			Point = matObjectToCameraT * Point;

			// Make sure to retain camera space 'z' coordinate.
			TqFloat zdepth = Point.z();
			aaPtimes[ keyFrame->second ][ i ] = matCameraToRaster * Point;
			aaPtimes[ keyFrame->second ][ i ].z( zdepth );
		}
		SqTriangleSplitLine sl;
		CqVector3D v0, v1, v2;
		v0 = aaPtimes[ keyFrame->second ][ 0 ];
		v1 = aaPtimes[ keyFrame->second ][ cu ];
		v2 = aaPtimes[ keyFrame->second ][ cv * ( cu + 1 ) ];
		// Check for clockwise, swap if not.
		if( ( ( v1.x() - v0.x() ) * ( v2.y() - v0.y() ) - ( v1.y() - v0.y() ) * ( v2.x() - v0.x() ) ) >= 0 )
		{
			sl.m_TriangleSplitPoint1 = v1;
			sl.m_TriangleSplitPoint2 = v2;
		}
		else
		{
			sl.m_TriangleSplitPoint1 = v2;
			sl.m_TriangleSplitPoint2 = v1;
		}
		m_TriangleSplitLine.AddTimeSlot(keyFrame->first, sl );
	}

	for ( i = gsmin1; i >= 0; i-- )
	{
		aaPtimes[ 0 ].resize( gsmin1 + 1 );
		// Make sure to retain camera space 'z' coordinate.
		TqFloat zdepth = pP[ i ].z();
		aaPtimes[ 0 ][ i ] = matCameraToRaster * pP[ i ];
		aaPtimes[ 0 ][ i ].z( zdepth );
		pP[ i ] = aaPtimes[ 0 ][ i ];
	}

	SqTriangleSplitLine sl;
	CqVector3D v0, v1, v2;
	v0 = aaPtimes[ 0 ][ 0 ];
	v1 = aaPtimes[ 0 ][ cu ];
	v2 = aaPtimes[ 0 ][ cv * ( cu + 1 ) ];
	// Check for clockwise, swap if not.
	if( ( ( v1.x() - v0.x() ) * ( v2.y() - v0.y() ) - ( v1.y() - v0.y() ) * ( v2.x() - v0.x() ) ) >= 0 )
	{
		sl.m_TriangleSplitPoint1 = v1;
		sl.m_TriangleSplitPoint2 = v2;
	}
	else
	{
		sl.m_TriangleSplitPoint1 = v2;
		sl.m_TriangleSplitPoint2 = v1;
	}
	m_TriangleSplitLine.AddTimeSlot(keyframeTimes.begin()->first, sl );

	AQSIS_TIMER_STOP(Project_points);

	AQSIS_TIMER_START(Bust_grids);
	// Get the required trim curve sense, if specified, defaults to "inside".
	const CqString* pattrTrimSense = pAttributes() ->GetStringAttribute( "trimcurve", "sense" );
	CqString strTrimSense( "inside" );
	if ( pattrTrimSense != 0 )
		strTrimSense = pattrTrimSense[ 0 ];
	bool bOutside = strTrimSense == "outside";

	// Determine whether we need to bother with trimming or not.
	bool bCanBeTrimmed = pSurface() ->bCanBeTrimmed() && NULL != pVar(EnvVars_u) && NULL != pVar(EnvVars_v);

	ADDREF( this );

	TqInt iv;
//	bool tooSmall_ = false;
//	TqFloat smallArea = 1.0;
//	TqFloat bigArea = 0.0;
	for ( iv = 0; iv < cv; iv++ )
	{
		TqInt iu;
		for ( iu = 0; iu < cu; iu++ )
		{
			TqInt iIndex = ( iv * ( cu + 1 ) ) + iu;

			// If culled don't bother.
			if ( m_CulledPolys.Value( iIndex ) )
			{
				STATS_INC(MPG_culled);
				continue;
			}

			// If the MPG is trimmed then don't add it.
			bool fTrimmed = false;
			if ( bCanBeTrimmed )
			{
				TqFloat u1, v1, u2, v2, u3, v3, u4, v4;

				pVar(EnvVars_u) ->GetFloat( u1, iIndex );
				pVar(EnvVars_v) ->GetFloat( v1, iIndex );
				pVar(EnvVars_u) ->GetFloat( u2, iIndex + 1 );
				pVar(EnvVars_v) ->GetFloat( v2, iIndex + 1 );
				pVar(EnvVars_u) ->GetFloat( u3, iIndex + cu + 2 );
				pVar(EnvVars_v) ->GetFloat( v3, iIndex + cu + 2 );
				pVar(EnvVars_u) ->GetFloat( u4, iIndex + cu + 1 );
				pVar(EnvVars_v) ->GetFloat( v4, iIndex + cu + 1 );

				CqVector2D vecA(u1, v1);
				CqVector2D vecB(u2, v2);
				CqVector2D vecC(u3, v3);
				CqVector2D vecD(u4, v4);

				bool fTrimA = pSurface() ->bIsPointTrimmed( vecA );
				bool fTrimB = pSurface() ->bIsPointTrimmed( vecB );
				bool fTrimC = pSurface() ->bIsPointTrimmed( vecC );
				bool fTrimD = pSurface() ->bIsPointTrimmed( vecD );

				if ( bOutside )
				{
					fTrimA = !fTrimA;
					fTrimB = !fTrimB;
					fTrimC = !fTrimC;
					fTrimD = !fTrimD;
				}

				// If all points are trimmed, need to check if the MP spans the trim curve at all, if not, then
				// we can discard it altogether.
				if ( fTrimA && fTrimB && fTrimC && fTrimD )
				{
					if(!pSurface()->bIsLineIntersecting(vecA, vecB) &&
					        !pSurface()->bIsLineIntersecting(vecB, vecC) &&
					        !pSurface()->bIsLineIntersecting(vecC, vecD) &&
					        !pSurface()->bIsLineIntersecting(vecD, vecA) )
					{
						STATS_INC( MPG_trimmedout );
						continue;
					}
				}

				// If any points are trimmed mark the MPG as needing to be trim checked.
				//fTrimmed = fTrimA || fTrimB || fTrimC || fTrimD;
				if ( fTrimA || fTrimB || fTrimC || fTrimD )
					fTrimmed = true;
			}

			if ( tTime > 1 )
			{
				boost::shared_ptr<CqMicroPolygonMotion> pNew(new CqMicroPolygonMotion(this, iIndex));
				if ( fTrimmed )
					pNew->MarkTrimmed();
				std::map<TqFloat, TqInt>::iterator keyFrame;
				for ( keyFrame = keyframeTimes.begin(); keyFrame!=keyframeTimes.end(); keyFrame++ )
					pNew->AppendKey( aaPtimes[ keyFrame->second ][ iIndex ], aaPtimes[ keyFrame->second ][ iIndex + 1 ], aaPtimes[ keyFrame->second ][ iIndex + cu + 2 ], aaPtimes[ keyFrame->second ][ iIndex + cu + 1 ],  keyFrame->first);
				boost::shared_ptr<CqMicroPolygon> pTemp(pNew);
				QGetRenderContext()->pImage()->AddMPG( pTemp );
			}
			else
			{
				boost::shared_ptr<CqMicroPolygon> pNew(new CqMicroPolygon(this, iIndex));
				if ( fTrimmed )
					pNew->MarkTrimmed();
				pNew->Initialise();
				QGetRenderContext()->pImage()->AddMPG( pNew );
			}

			// Calculate MPG area
			TqFloat area = 0.0f;
			area += ( aaPtimes[ 0 ][ iIndex ].x() * aaPtimes[ 0 ][iIndex + 1 ].y() ) - ( aaPtimes[ 0 ][ iIndex ].y() * aaPtimes[ 0 ][ iIndex + 1 ].x() );
			area += ( aaPtimes[ 0 ][ iIndex + 1].x() * aaPtimes[ 0 ][iIndex + cu + 2 ].y() ) - ( aaPtimes[ 0 ][ iIndex + 1].y() * aaPtimes[ 0 ][ iIndex + cu + 2 ].x() );
			area += ( aaPtimes[ 0 ][ iIndex + cu + 2].x() * aaPtimes[ 0 ][iIndex + cu + 1 ].y() ) - ( aaPtimes[ 0 ][ iIndex + cu + 2 ].y() * aaPtimes[ 0 ][ iIndex + cu + 1 ].x() );
			area += ( aaPtimes[ 0 ][ iIndex + cu + 1].x() * aaPtimes[ 0 ][iIndex ].y() ) - ( aaPtimes[ 0 ][ iIndex + cu + 1 ].y() * aaPtimes[ 0 ][ iIndex ].x() );
			area *= 0.5f;
			area = fabs(area);

			STATS_SETF( MPG_average_area, STATS_GETF( MPG_average_area ) + area );
			if( area < STATS_GETF( MPG_min_area ) )
				STATS_SETF( MPG_min_area, area );
			if( area > STATS_GETF( MPG_max_area ) )
				STATS_SETF( MPG_max_area, area );

		//	smallArea = std::min(smallArea, area);
		//	bigArea = std::max(bigArea, area);
		//	if(area < 0.005)
		//	{
		//		tooSmall_ = true;
		//	}
		}
	}
	AQSIS_TIMER_STOP(Bust_grids);
//	if(tooSmall_)
//	{
//		CqString objname( "unnamed" );
//		const CqString* pattrName = pAttributes()->GetStringAttribute( "identifier", "name" );
//		if ( pattrName != 0 )
//			objname = pattrName[ 0 ];
//		Aqsis::log() << error << "Primitive \"" << objname.c_str() << "\" resulted in very small MPS (" << smallArea << ", " << bigArea << ")" << cu << ", " << cv << std::endl;
//	}

	RELEASEREF( this );
}


void CqMicroPolyGridBase::TriangleSplitPoints(CqVector3D& v1, CqVector3D& v2, TqFloat Time)
{
	// Workout where in the keyframe sequence the requested point is.
	SqTriangleSplitLine sl = m_TriangleSplitLine.GetMotionObjectInterpolated( Time );
	v1 = sl.m_TriangleSplitPoint1;
	v2 = sl.m_TriangleSplitPoint2;
}


CqMotionMicroPolyGrid::~CqMotionMicroPolyGrid()
{
	TqInt iTime;
	for ( iTime = 0; iTime < cTimes(); iTime++ )
	{
		CqMicroPolyGrid* pg = static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( iTime ) ) );
		if ( NULL != pg )
			RELEASEREF( pg );
	}
}


//---------------------------------------------------------------------
/** Shade the primary grid.
 */

void CqMotionMicroPolyGrid::Shade( bool canCullGrid )
{
	CqMicroPolyGrid * pGrid = static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( 0 ) ) );
	pGrid->Shade(false);
}


//---------------------------------------------------------------------
/** Transfer shader output variables for the primary grid.
 */

void CqMotionMicroPolyGrid::TransferOutputVariables()
{
	CqMicroPolyGrid * pGrid = static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( 0 ) ) );
	pGrid->TransferOutputVariables();
}


//---------------------------------------------------------------------
/** Split the micropolygrid into individual MPGs,
 * \param xmin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param xmax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 */

void CqMotionMicroPolyGrid::Split( long xmin, long xmax, long ymin, long ymax )
{
	TqInt lUses = pSurface() ->Uses();
	// Get the main object, the one that was shaded.
	CqMicroPolyGrid * pGridA = static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( 0 ) ) );
	TqInt cu = pGridA->uGridRes();
	TqInt cv = pGridA->vGridRes();
	TqInt iTime;

	AQSIS_TIMER_START(Project_points);
	CqMatrix matCameraToRaster;
	QGetRenderContext() ->matSpaceToSpace( "camera", "raster", NULL, NULL, QGetRenderContext()->Time(), matCameraToRaster );
	// Check to see if this surface is single sided, if so, we can do backface culling.
	bool canBeBFCulled = ( pAttributes() ->GetIntegerAttribute( "System", "Sides" ) [ 0 ] == 1 ) && !pGridA->usesCSG();

	ADDREF( pGridA );

	// Get an array of P's for all time positions.
	std::vector<std::vector<CqVector3D> > aaPtimes;
	aaPtimes.resize( cTimes() );
	
	TqInt tTime = cTimes();
	CqMatrix matObjectToCameraT;
	register TqInt i;
	TqInt gsmin1;
	gsmin1 = pGridA->pShaderExecEnv()->shadingPointCount() - 1;

	// Store a count of backface culling for each MP, if it equals the number of timeslots, then
	// the MP can be culled.
	std::vector<TqInt> totalBFCulled;
	totalBFCulled.assign( gsmin1 + 1, 0 );

	for ( iTime = 0; iTime < tTime; iTime++ )
	{
		QGetRenderContext() ->matSpaceToSpace( "object", "camera", NULL, pSurface() ->pTransform().get(), pSurface()->pTransform()->Time(iTime), matObjectToCameraT  );
		aaPtimes[ iTime ].resize( gsmin1 + 1 );

		// Transform the whole grid to hybrid camera/raster space
		CqMicroPolyGrid* pg = static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( iTime ) ) );
		CqVector3D* pP;
		pg->pVar(EnvVars_P) ->GetPointPtr( pP );
		CqVector3D* pNg;
		pg->pVar(EnvVars_Ng) ->GetPointPtr( pNg );
		CqVector3D* pN = NULL;
		if ( USES( lUses, EnvVars_N ) )
			pg->pVar(EnvVars_N) ->GetPointPtr( pN );


		// When backface culling, we must use the geometric normal (Ng) as
		// this is the normal that properly represents the actual micropolygon
		// geometry. However, if the primitive specifies custom normals as
		// primitive variables, the direction of those should be honored, in case
		// the user has intentionally switched the surface direction.
		// Therefore, we compare the direction of Ng with that of N and flip Ng 
		// if they don't match.

		for ( i = gsmin1; i >= 0; i-- )
		{
			CqVector3D Point( pP[ i ] );

			// Make sure to retain camera space 'z' coordinate.
			TqFloat zdepth = Point.z();
			aaPtimes[ iTime ][ i ] = matCameraToRaster * Point;
			aaPtimes[ iTime ][ i ].z( zdepth );
			pP[ i ] = aaPtimes[ iTime ][ i ];

			// Now try and cull any hidden MPs if Sides==1
			if ( canBeBFCulled )
			{
				TqFloat s = 1.0f;
				if( NULL != pN )
					s = ( ( pN[i] * pNg[i] ) < 0.0f ) ? -1.0f : 1.0f;
				AQSIS_TIME_SCOPE(Backface_culling);
				if ( (  ( s * pNg[ i ] ) * pP[ i ] ) >= 0 )
					totalBFCulled[i]++;
			}
		}
		SqTriangleSplitLine sl;
		CqVector3D v0, v1, v2;
		v0 = aaPtimes[ iTime ][ 0 ];
		v1 = aaPtimes[ iTime ][ cu ];
		v2 = aaPtimes[ iTime ][ cv * ( cu + 1 ) ];
		// Check for clockwise, swap if not.
		if( ( ( v1.x() - v0.x() ) * ( v2.y() - v0.y() ) - ( v1.y() - v0.y() ) * ( v2.x() - v0.x() ) ) >= 0 )
		{
			sl.m_TriangleSplitPoint1 = v1;
			sl.m_TriangleSplitPoint2 = v2;
		}
		else
		{
			sl.m_TriangleSplitPoint1 = v2;
			sl.m_TriangleSplitPoint2 = v1;
		}
		m_TriangleSplitLine.AddTimeSlot(Time( iTime ), sl );
	}
	AQSIS_TIMER_STOP(Project_points);

	AQSIS_TIMER_START(Bust_grids);
	// Get the required trim curve sense, if specified, defaults to "inside".
	const CqString* pattrTrimSense = pAttributes() ->GetStringAttribute( "trimcurve", "sense" );
	CqString strTrimSense( "inside" );
	if ( pattrTrimSense != 0 )
		strTrimSense = pattrTrimSense[ 0 ];
	bool bOutside = strTrimSense == "outside";

	// Determine whether we need to bother with trimming or not.
	bool bCanBeTrimmed = pSurface() ->bCanBeTrimmed() && NULL != pGridA->pVar(EnvVars_u) && NULL != pGridA->pVar(EnvVars_v);

	TqInt iv;
	TqInt totalTimes = cTimes();
	for ( iv = 0; iv < cv; iv++ )
	{
		TqInt iu;
		for ( iu = 0; iu < cu; iu++ )
		{
			TqInt iIndex = ( iv * ( cu + 1 ) ) + iu;

			// If culled don't bother.
			if ( totalBFCulled[iIndex] == totalTimes )
			{
				STATS_INC( MPG_culled );
				continue;
			}

			// If the MPG is trimmed then don't add it.
			bool fTrimmed = false;
			if ( bCanBeTrimmed )
			{
				TqFloat u1, v1, u2, v2, u3, v3, u4, v4;

				pGridA->pVar(EnvVars_u) ->GetFloat( u1, iIndex );
				pGridA->pVar(EnvVars_v) ->GetFloat( v1, iIndex );
				pGridA->pVar(EnvVars_u) ->GetFloat( u2, iIndex + 1 );
				pGridA->pVar(EnvVars_v) ->GetFloat( v2, iIndex + 1 );
				pGridA->pVar(EnvVars_u) ->GetFloat( u3, iIndex + cu + 2 );
				pGridA->pVar(EnvVars_v) ->GetFloat( v3, iIndex + cu + 2 );
				pGridA->pVar(EnvVars_u) ->GetFloat( u4, iIndex + cu + 1 );
				pGridA->pVar(EnvVars_v) ->GetFloat( v4, iIndex + cu + 1 );

				CqVector2D vecA(u1, v1);
				CqVector2D vecB(u2, v2);
				CqVector2D vecC(u3, v3);
				CqVector2D vecD(u4, v4);

				bool fTrimA = pSurface() ->bIsPointTrimmed( vecA );
				bool fTrimB = pSurface() ->bIsPointTrimmed( vecB );
				bool fTrimC = pSurface() ->bIsPointTrimmed( vecC );
				bool fTrimD = pSurface() ->bIsPointTrimmed( vecD );

				if ( bOutside )
				{
					fTrimA = !fTrimA;
					fTrimB = !fTrimB;
					fTrimC = !fTrimC;
					fTrimD = !fTrimD;
				}

				// If all points are trimmed, need to check if the MP spans the trim curve at all, if not, then
				// we can discard it altogether.
				if ( fTrimA && fTrimB && fTrimC && fTrimD )
				{
					if(!pSurface()->bIsLineIntersecting(vecA, vecB) &&
					        !pSurface()->bIsLineIntersecting(vecB, vecC) &&
					        !pSurface()->bIsLineIntersecting(vecC, vecD) &&
					        !pSurface()->bIsLineIntersecting(vecD, vecA) )
					{
						STATS_INC( MPG_trimmedout );
						continue;
					}
				}

				// If any points are trimmed mark the MPG as needing to be trim checked.
				//fTrimmed = fTrimA || fTrimB || fTrimC || fTrimD;
				if ( fTrimA || fTrimB || fTrimC || fTrimD )
					fTrimmed = true;
			}

			boost::shared_ptr<CqMicroPolygonMotion> pNew( new CqMicroPolygonMotion( this, iIndex ) );
			for ( iTime = 0; iTime < cTimes(); iTime++ )
				pNew->AppendKey( aaPtimes[ iTime ][ iIndex ], aaPtimes[ iTime ][ iIndex + 1 ], aaPtimes[ iTime ][ iIndex + cu + 2 ], aaPtimes[ iTime ][ iIndex + cu + 1 ], Time( iTime ) );
			boost::shared_ptr<CqMicroPolygon> pTemp( pNew );
			QGetRenderContext()->pImage()->AddMPG( pTemp );
		}
	}
	AQSIS_TIMER_STOP(Bust_grids);

	RELEASEREF( pGridA );

	// Delete the donor motion grids, as their work is done.
	/*    for ( iTime = 1; iTime < cTimes(); iTime++ )
	    {
	        CqMicroPolyGrid* pg = static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( iTime ) ) );
	        if ( NULL != pg )
	            RELEASEREF( pg );
	    }
	    //		delete( GetMotionObject( Time( iTime ) ) );
	*/
}


//---------------------------------------------------------------------
/** Default constructor
 */

CqMicroPolygon::CqMicroPolygon(CqMicroPolyGridBase* pGrid, TqInt Index ) : m_pGrid( pGrid ), m_Index(Index), m_Flags( 0 )
{
	STATS_INC( MPG_allocated );
	STATS_INC( MPG_current );
	TqInt cMPG = STATS_GETI( MPG_current );
	TqInt cPeak = STATS_GETI( MPG_peak );
	STATS_SETI( MPG_peak, cMPG > cPeak ? cMPG : cPeak );
	ADDREF(pGrid);
}


//---------------------------------------------------------------------
/** Destructor
 */

CqMicroPolygon::~CqMicroPolygon()
{
	if ( m_pGrid )
		RELEASEREF( m_pGrid );
	STATS_INC( MPG_deallocated );
	STATS_DEC( MPG_current );
	if ( !IsHit() )
		STATS_INC( MPG_missed );
}


//---------------------------------------------------------------------
/** Initialise the information within the micro polygon used during sampling.
 */

void CqMicroPolygon::Initialise()
{
	// Check for degenerate case, if any of the neighbouring points are the same, shuffle them down, and
	// duplicate the last point exactly. Exact duplication of the last two points is used as a marker in the
	// fContains function to indicate degeneracy. If more that two points are coincident, we are in real trouble!
	TqInt cu = m_pGrid->uGridRes();
	TqInt IndexA = m_Index;
	TqInt IndexB = m_Index + 1;
	TqInt IndexC = m_Index + cu + 2;
	TqInt IndexD = m_Index + cu + 1;

	TqShort CodeA = 0;
	TqShort CodeB = 1;
	TqShort CodeC = 2;
	TqShort CodeD = 3;

	const CqVector3D* pP;
	m_pGrid->pVar(EnvVars_P) ->GetPointPtr( pP );
	if ( ( pP[ IndexA ] - pP[ IndexB ] ).Magnitude2() < 1e-8 )
	{
		// A--B is degenerate
		IndexB = IndexC;
		CodeB = CodeC;
		IndexC = IndexD;
		CodeC = CodeD;
		IndexD = -1;
		CodeD = -1;
	}
	else if ( ( pP[ IndexB ] - pP[ IndexC ] ).Magnitude2() < 1e-8 )
	{
		// B--C is degenerate
		IndexB = IndexC;
		CodeB = CodeC;
		IndexC = IndexD;
		CodeC = CodeD;
		IndexD = -1;
		CodeD = -1;
	}
	else if ( ( pP[ IndexC ] - pP[ IndexD ] ).Magnitude2() < 1e-8 )
	{
		// C--D is degenerate
		IndexC = IndexD;
		CodeC = CodeD;
		IndexD = -1;
		CodeD = -1;
	}
	else if ( ( pP[ IndexD ] - pP[ IndexA ] ).Magnitude2() < 1e-8 )
	{
		// D--A is degenerate
		IndexD = IndexC;
		CodeD = CodeC;
		IndexD = -1;
		CodeD = -1;
	}

	const CqVector3D& vA2 = pP[ IndexA ];
	const CqVector3D& vB2 = pP[ IndexB ];
	const CqVector3D& vC2 = pP[ IndexC ];

	// Determine whether the MPG is CW or CCW, must be CCW for fContains to work.
	bool fFlip = ( ( vA2.x() - vB2.x() ) * ( vB2.y() - vC2.y() ) ) >= ( ( vA2.y() - vB2.y() ) * ( vB2.x() - vC2.x() ) );

	m_IndexCode = 0;

	if ( !fFlip )
	{
		m_IndexCode = ( CodeD == -1 ) ?
		              ( ( CodeA & 0x3 ) | ( ( CodeC & 0x3 ) << 2 ) | ( ( CodeB & 0x3 ) << 4 ) | 0x8000000 ) :
		              ( ( CodeA & 0x3 ) | ( ( CodeD & 0x3 ) << 2 ) | ( ( CodeC & 0x3 ) << 4 ) | ( ( CodeB & 0x3 ) << 6 ) );
	}
	else
	{
		m_IndexCode = ( CodeD == -1 ) ?
		              ( ( CodeA & 0x3 ) | ( ( CodeB & 0x3 ) << 2 ) | ( ( CodeC & 0x3 ) << 4 ) | 0x8000000 ) :
		              ( ( CodeA & 0x3 ) | ( ( CodeB & 0x3 ) << 2 ) | ( ( CodeC & 0x3 ) << 4 ) | ( ( CodeD & 0x3 ) << 6 ) );
	}

	CalculateBound();
}


//---------------------------------------------------------------------
/** Determinde whether the 2D point specified lies within this micropolygon.
 * \param vecP 2D vector to test for containment.
 * \param Depth Place to put the depth if valid intersection.
 * \param time The frame time at which to check containment.
 * \return Boolean indicating sample hit.
 */

bool CqMicroPolygon::fContains( CqHitTestCache& hitTestCache, const CqVector2D& vecP, TqFloat& Depth, TqFloat time) const
{
	// AGG - optimised version of above.
	TqFloat x = vecP.x(), y = vecP.y();

	// start with the edge that failed last time to get the most benefit
	// from an early exit.
	int e = hitTestCache.m_LastFailedEdge;
	int prev = e - 1;
	if(prev < 0)
		prev = 3;
	for(int i=0; i<4; ++i)
	{
		// test which side of the edge the sample point lies.
		// the first two edges are tested with <= and the second two with <
		// this is so every sample point lies on exactly one side of the edge,
		// ie if it is exactly coincident with the edge it can't be on both
		// or neither sides.
		if(e & 2)
		{
			if( (( y - hitTestCache.m_Y[e]) * hitTestCache.m_YMultiplier[e] ) -
			        (( x - hitTestCache.m_X[e]) * hitTestCache.m_XMultiplier[e] ) < 0)
			{
				hitTestCache.m_LastFailedEdge = e;
				return false;
			}
		}
		else
		{
			if( (( y - hitTestCache.m_Y[e]) * hitTestCache.m_YMultiplier[e] ) -
			        (( x - hitTestCache.m_X[e]) * hitTestCache.m_XMultiplier[e] ) <= 0)
			{
				hitTestCache.m_LastFailedEdge = e;
				return false;
			}
		}

		// move to next edge, wrapping to zero at four.
		prev = e;
		e = (e+1) & 3;
	}

	Depth = ( hitTestCache.m_D - ( hitTestCache.m_VecN.x() * vecP.x() ) -
	          ( hitTestCache.m_VecN.y() * vecP.y() ) ) * hitTestCache.m_OneOverVecNZ;

	return true;
}

//---------------------------------------------------------------------
/** Cache some values needed for the point in poly test.
 * This must be called prior to calling fContains() on a mpg.
 */

inline void CqMicroPolygon::CacheHitTestValues(CqHitTestCache* cache, CqVector3D* points) const
{
	int j = 3;
	for(int i=0; i<4; ++i)
	{
		cache->m_YMultiplier[i] = points[i].x() - points[j].x();
		cache->m_XMultiplier[i] = points[i].y() - points[j].y();
		cache->m_X[i] = points[j].x();
		cache->m_Y[i] = points[j].y();
		j = i;
	}

	// if the mpg is degenerate then we repeat edge c=>a so we still have four
	// edges (it makes the test in fContains() simpler).
	if(IsDegenerate())
	{
		for(int i=2; i<4; ++i)
		{
			cache->m_YMultiplier[i] = points[3].x() - points[1].x();
			cache->m_XMultiplier[i] = points[3].y() - points[1].y();
			cache->m_X[i] = points[1].x();
			cache->m_Y[i] = points[1].y();
		}
	}

	cache->m_VecN = (points[3] - points[0]) % (points[1] - points[0]);
	cache->m_VecN.Unit();
	cache->m_D = cache->m_VecN * points[3];
	cache->m_OneOverVecNZ = 1.0 / cache->m_VecN.z();

	cache->m_LastFailedEdge = 0;
}

void CqMicroPolygon::CacheHitTestValues(CqHitTestCache* cache) const
{
	CqVector3D points[4] = { PointB(), PointC(), PointD(), PointA() };
	CacheHitTestValues(cache, points);
}

void CqMicroPolygon::CacheCocMultipliers(CqHitTestCache& cache) const
{
	const CqRenderer* renderContext = QGetRenderContext();
	cache.cocMult[0] = renderContext->GetCircleOfConfusion(PointB().z());
	cache.cocMult[1] = renderContext->GetCircleOfConfusion(PointC().z());
	cache.cocMult[2] = renderContext->GetCircleOfConfusion(PointD().z());
	cache.cocMult[3] = renderContext->GetCircleOfConfusion(PointA().z());

	cache.cocMultMin = min(min(min(cache.cocMult[0], cache.cocMult[1]),
				cache.cocMult[2]), cache.cocMult[3]);
	cache.cocMultMax = max(max(max(cache.cocMult[0], cache.cocMult[1]),
				cache.cocMult[2]), cache.cocMult[3]);
}

void CqMicroPolygon::CacheOutputInterpCoeffs(SqMpgSampleInfo& cache) const
{
	if(cache.smoothInterpolation)
		CacheOutputInterpCoeffsSmooth(cache);
	else
		CacheOutputInterpCoeffsConstant(cache);
}

void CqMicroPolygon::InterpolateOutputs(const SqMpgSampleInfo& cache,
	const CqVector2D& pos, CqColor& outCol, CqColor& outOpac) const
{
	if(cache.smoothInterpolation)
	{
		outCol = cache.color + cache.colorMultX*pos.x() + cache.colorMultY*pos.y();
		outOpac = cache.opacity + cache.opacityMultX*pos.x() + cache.opacityMultY*pos.y();
	}
	else
	{
		outCol = cache.color;
		outOpac = cache.opacity;
	}
}

void CqMicroPolygon::CacheOutputInterpCoeffsConstant(SqMpgSampleInfo& cache) const
{
	if ( QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "Ci" ) )
	{
		cache.color = *colColor();
	}
	else
	{
		cache.color = gColWhite;
	}

	if ( QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "Oi" ) )
	{
		cache.opacity = *colOpacity();
		cache.occludes = cache.opacity >= gColWhite;
	}
	else
	{
		cache.opacity = gColWhite;
		cache.occludes = true;
	}
}

void CqMicroPolygon::CacheOutputInterpCoeffsSmooth(SqMpgSampleInfo& cache) const
{
	// Get 2D coordinates of verts in the (x,y) plane.
	CqVector2D p1(vectorCast<CqVector2D>(PointA()));
	CqVector2D p2(vectorCast<CqVector2D>(PointB()));
	CqVector2D p3(vectorCast<CqVector2D>(PointC()));
	CqVector2D p4(vectorCast<CqVector2D>(PointD()));

	// 2D diagonal vectors for the micropolygon.  The order of the vertices
	// (p1, p2, p3, p4) here winds around the micropoly, rather than taking the
	// usual order expected from RiPatch.
	CqVector2D d1 = p3 - p1;
	CqVector2D d2 = p4 - p2;
	// Center point
	CqVector2D pAvg = 0.25*(p1 + p2 + p3 + p4);

	// For each component of the colour, we compute a linear approximation to
	// the component over the micropolygon.  This is done by computing a
	// "normal" vector to a plane constructed such that the colour component
	// takes the place of the depth, "z".
	//
	// This is essentially the same method used to compute the depth of a
	// sample inside the fContains() function.

	TqFloat Nz = d1.x()*d2.y() - d1.y()*d2.x();

	if(QGetRenderContext()->pDDmanager()->fDisplayNeeds( "Ci" ))
	{
		const CqColor* pCi = NULL;
		m_pGrid->pVar(EnvVars_Ci)->GetColorPtr(pCi);

		const CqColor& c1 = pCi[GetCodedIndex(m_IndexCode, 0)];
		const CqColor& c2 = pCi[GetCodedIndex(m_IndexCode, 1)];
		const CqColor& c3 = pCi[GetCodedIndex(m_IndexCode, 2)];
		const CqColor& c4 = pCi[GetCodedIndex(m_IndexCode, 3)];
		const CqColor cAvg = 0.25*(c1 + c2 + c3 + c4);

		if(Nz != 0)
		{
			// Compute smooth shading coefficients for Ci.
			CqColor c31 = c3 - c1;
			CqColor c42 = c4 - c2;

			CqColor Nx = d1.y()*c42 - c31*d2.y();
			CqColor Ny = -d1.x()*c42 + c31*d2.x();

			TqFloat NzInv = 1/Nz;
			cache.color = (Nx*pAvg.x() + Ny*pAvg.y())*NzInv + cAvg;
			cache.colorMultX = -Nx*NzInv;
			cache.colorMultY = -Ny*NzInv;
		}
		else
		{
			// Degenerate to flat shading if the linear approx yields an
			// infinitely steep plane.
			cache.color = c1;
			cache.colorMultX = gColBlack;
			cache.colorMultY = gColBlack;
		}
	}
	else
	{
		cache.color = gColWhite;
		cache.colorMultX = gColBlack;
		cache.colorMultY = gColBlack;
	}

	if(QGetRenderContext()->pDDmanager()->fDisplayNeeds( "Oi" ))
	{
		const CqColor* pOi = NULL;
		m_pGrid->pVar(EnvVars_Oi)->GetColorPtr(pOi);

		const CqColor& o1 = pOi[GetCodedIndex(m_IndexCode, 0)];
		const CqColor& o2 = pOi[GetCodedIndex(m_IndexCode, 1)];
		const CqColor& o3 = pOi[GetCodedIndex(m_IndexCode, 2)];
		const CqColor& o4 = pOi[GetCodedIndex(m_IndexCode, 3)];
		const CqColor oAvg = 0.25*(o1 + o2 + o3 + o4);

		if(Nz != 0)
		{
			// Compute smooth shading coefficients for Oi.
			CqColor o31 = o3 - o1;
			CqColor o42 = o4 - o2;

			CqColor Nx = d1.y()*o42 - o31*d2.y();
			CqColor Ny = -d1.x()*o42 + o31*d2.x();

			TqFloat NzInv = 1/Nz;
			cache.opacity = (Nx*pAvg.x() + Ny*pAvg.y())*NzInv + oAvg;
			cache.opacityMultX = -Nx*NzInv;
			cache.opacityMultY = -Ny*NzInv;

			// The micropoly occludes if the values on all vertices do.
			cache.occludes = (o1 >= gColWhite) && (o2 >= gColWhite)
				&& (o3 >= gColWhite) && (o4 >= gColWhite);
		}
		else
		{
			cache.opacity = oAvg;
			cache.opacityMultX = gColBlack;
			cache.opacityMultY = gColBlack;
			cache.occludes = (cache.opacity >= gColWhite);
		}
	}
	else
	{
		cache.opacity = gColWhite;
		cache.opacityMultX = gColBlack;
		cache.opacityMultY = gColBlack;
		cache.occludes = true;
	}
}

CqVector2D CqMicroPolygon::ReverseBilinear( const CqVector2D& v ) const
{
	CqVector2D kA, kB, kC, kD;
	CqVector2D kResult;
	bool flip = false;

	kA = vectorCast<CqVector2D>( PointA() );
	kB = vectorCast<CqVector2D>( PointB() );
	kC = vectorCast<CqVector2D>( PointD() );
	kD = vectorCast<CqVector2D>( PointC() );

	if(fabs(kB.x() - kA.x()) < fabs(kC.x() - kA.x()) )
	{
		CqVector2D temp = kC;
		kC = kB;
		kB = temp;
		//flip = true;
	}

	kD += kA - kB - kC;
	kB -= kA;
	kC -= kA;

	TqFloat fBCdet = kB.x() * kC.y() - kB.y() * kC.x();
	TqFloat fCDdet = kC.y() * kD.x() - kC.x() * kD.y();

	CqVector2D kDiff = kA - v;
	TqFloat fABdet = kDiff.y() * kB.x() - kDiff.x() * kB.y();
	TqFloat fADdet = kDiff.y() * kD.x() - kDiff.x() * kD.y();
	TqFloat fA = fCDdet;
	TqFloat fB = fADdet + fBCdet;
	TqFloat fC = fABdet;

	if ( fabs( fA ) >= 1.0e-6 )
	{
		// t-equation is quadratic
		TqFloat fDiscr = sqrt( fabs( fB * fB - 4.0f * fA * fC ) );
		kResult.y( ( -fB + fDiscr ) / ( 2.0f * fA ) );
		if ( kResult.y() < 0.0f || kResult.y() > 1.0f )
		{
			kResult.y( ( -fB - fDiscr ) / ( 2.0f * fA ) );
			if ( kResult.y() < 0.0f || kResult.y() > 1.0f )
			{
				// point p not inside quadrilateral, return invalid result
				return ( CqVector2D( -1.0f, -1.0f ) );
			}
		}
	}
	else
	{
		// t-equation is linear
		kResult.y( -fC / fB );
	}
	kResult.x( -( kDiff.x() + kResult.y() * kC.x() ) / ( kB.x() + kResult.y() * kD.x() ) );
	if(flip)
	{
		TqFloat temp = kResult.x();
		kResult.x(kResult.y());
		kResult.y(temp);
	}

	return ( kResult );
}

inline bool CqMicroPolygon::dofSampleInBound(const CqBound& bound,
		const CqHitTestCache& cache, const SqSampleData& sample)
{
	CqVector2D dofOffset = sample.m_DofOffset;
	CqVector2D samplePos = sample.m_Position;
	// Compute the two ends of a line segment on which the sample would
	// lie after offsetting by the "true" CoC multiplier*DoF offset.  The true
	// offset can't be calculated without knowing the sample hit depth, but
	// these allow us to put bounds on what it can be.
	CqVector2D cocMin = samplePos + compMul(cache.cocMultMin, dofOffset);
	CqVector2D cocMax = samplePos + compMul(cache.cocMultMax, dofOffset);
	// cocMin and cocMax define a bounding box, but may not be the bottom-left
	// and top-right.  We swap the components as necessary so that cocMin is
	// the bottom left and cocMax is the top-right.
	if(dofOffset.x() < 0)
		std::swap(cocMin.x(), cocMax.x());
	if(dofOffset.y() < 0)
		std::swap(cocMin.y(), cocMax.y());
	return bound.Intersects(cocMin, cocMax);
}

//---------------------------------------------------------------------
/** Sample the specified point against the MPG at the specified time.
 * \param vecSample 2D vector to sample against.
 * \param time Shutter time to sample at.
 * \param D Storage for depth if valid hit.
 * \return Boolean indicating smaple hit.
 */

bool CqMicroPolygon::Sample( CqHitTestCache& hitTestCache, const SqSampleData& sample, TqFloat& D, TqFloat time, bool UsingDof ) const
{
	CqVector2D vecSample = sample.m_Position;

	if(UsingDof)
	{
		// For DoF, we first check whether the sample position can possibly
		// fall inside the tight bounding box for the micropolygon.  This
		// allows us to reject a lot of points before the more expensive
		// point-in-polygon test takes place.
		if(!dofSampleInBound(m_Bound, hitTestCache, sample))
			return false;

		// When using DoF, we need to adjust the micropolygon point positions
		// along the opposite of the direction of the DoF offset for the
		// current sample.
		CqVector2D* coc = &hitTestCache.cocMult[0];
		CqVector2D dofOffset = sample.m_DofOffset;
		CqVector3D points[4] = {
			PointB() - vectorCast<CqVector3D>(compMul(coc[0], dofOffset)),
			PointC() - vectorCast<CqVector3D>(compMul(coc[1], dofOffset)),
			PointD() - vectorCast<CqVector3D>(compMul(coc[2], dofOffset)),
			PointA() - vectorCast<CqVector3D>(compMul(coc[3], dofOffset))
		};
		// Having displaced and slightly distorted the micropolygon, we now
		// need to calculate the hit test coefficients.
		CacheHitTestValues(&hitTestCache, points);
	}

	if ( fContains( hitTestCache, vecSample, D, time ) )
	{
		// Now check if it is trimmed.
		if ( IsTrimmed() )
		{
			// Get the required trim curve sense, if specified, defaults to "inside".
			const CqString * pattrTrimSense = pGrid() ->pAttributes() ->GetStringAttribute( "trimcurve", "sense" );
			CqString strTrimSense( "inside" );
			if ( pattrTrimSense != 0 )
				strTrimSense = pattrTrimSense[ 0 ];
			bool bOutside = strTrimSense == "outside";

			CqVector2D vecUV = ReverseBilinear( vecSample );

			TqFloat u, v;

			pGrid() ->pVar(EnvVars_u) ->GetFloat( u, m_Index );
			pGrid() ->pVar(EnvVars_v) ->GetFloat( v, m_Index );
			CqVector2D uvA( u, v );

			pGrid() ->pVar(EnvVars_u) ->GetFloat( u, m_Index + 1 );
			pGrid() ->pVar(EnvVars_v) ->GetFloat( v, m_Index + 1 );
			CqVector2D uvB( u, v );

			pGrid() ->pVar(EnvVars_u) ->GetFloat( u, m_Index + pGrid() ->uGridRes() + 1 );
			pGrid() ->pVar(EnvVars_v) ->GetFloat( v, m_Index + pGrid() ->uGridRes() + 1 );
			CqVector2D uvC( u, v );

			pGrid() ->pVar(EnvVars_u) ->GetFloat( u, m_Index + pGrid() ->uGridRes() + 2 );
			pGrid() ->pVar(EnvVars_v) ->GetFloat( v, m_Index + pGrid() ->uGridRes() + 2 );
			CqVector2D uvD( u, v );

			CqVector2D vR = BilinearEvaluate( uvA, uvB, uvC, uvD, vecUV.x(), vecUV.y() );

			if ( pGrid() ->pSurface() ->bCanBeTrimmed() && pGrid() ->pSurface() ->bIsPointTrimmed( vR ) && !bOutside )
			{
				STATS_INC( MPG_trimmed );
				return ( false );
			}
		}

		if ( pGrid() ->fTriangular() )
		{
			CqVector3D vA, vB;
			pGrid()->TriangleSplitPoints( vA, vB, time );
			TqFloat Ax = vA.x();
			TqFloat Ay = vA.y();
			TqFloat Bx = vB.x();
			TqFloat By = vB.y();

			TqFloat v = ( Ay - By ) * vecSample.x() + ( Bx - Ax ) * vecSample.y() + ( Ax * By - Bx * Ay );
			if ( v <= 0 )
				return ( false );
		}

		return ( true );
	}
	else
		return ( false );
}

//---------------------------------------------------------------------
void CqMicroPolygon::CalculateBound()
{
	CqVector3D * pP;
	m_pGrid->pVar(EnvVars_P) ->GetPointPtr( pP );
	TqInt cu = m_pGrid->uGridRes();
	const CqVector3D& A = pP[ m_Index ];
	const CqVector3D& B = pP[ m_Index + 1 ];
	const CqVector3D& C = pP[ m_Index + cu + 2 ];
	const CqVector3D& D = pP[ m_Index + cu + 1 ];
	m_Bound = CqBound( min(min(min(A,B),C),D), max(max(max(A,B),C),D) );
}


//---------------------------------------------------------------------
/** Calculate a list of 2D bounds for this micropolygon,
 */
void CqMicroPolygonMotion::BuildBoundList(TqUint timeRanges)
{
	TqFloat opentime = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Shutter" ) [ 0 ];
	TqFloat closetime = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Shutter" ) [ 1 ];
	// TODO: This version of shadingrate isn't accurate if using
	// GeometricApproximation "focusfactor" 
	TqFloat shadingrate = pGrid() ->pAttributes() ->GetFloatAttribute( "System", "ShadingRate" ) [ 0 ];

	m_BoundList.Clear();

	assert( NULL != m_Keys[ 0 ] );

	// compute an approximation of the distance travelled in raster space,
	// we use this to guide how many sub-bounds to calcuate. note, it's much
	// better for this to be fast than accurate, it's just a guide.
	TqFloat dx = fabs(m_Keys.front()->m_Point0.x() - m_Keys.back()->m_Point0.x());
	TqFloat dy = fabs(m_Keys.front()->m_Point0.y() - m_Keys.back()->m_Point0.y());
	TqUint d = static_cast<int>((dx + dy) / shadingrate) + 1; // d is always >= 1

	TqUint divisions = min(d, timeRanges);
	TqFloat dt = (closetime - opentime) / divisions;
	TqFloat time = opentime + dt;
	TqInt startKey = 0;
	TqUint endKey = 1;
	CqBound bound = m_Keys[startKey]->GetBound();

	m_BoundList.SetSize( divisions );

	// create a bound for each time period.
	for(TqUint i = 0; i < divisions; i++)
	{
		// find the fist key with a time greater than our end time.
		while(time > m_Times[endKey] && endKey < m_Keys.size() - 1)
			++endKey;

		// interpolate between this key and the previous to get the
		// bound at our end time.
		TqInt endKey_1 = endKey - 1;
		const CqBound& end0 = m_Keys[endKey_1]->GetBound();
		TqFloat end0Time = m_Times[endKey_1];
		const CqBound& end1 = m_Keys[endKey]->GetBound();
		TqFloat end1Time = m_Times[endKey];

		TqFloat mix = (time - end0Time) / (end1Time - end0Time);
		CqBound mid(end0);
		mid.vecMin() += mix * (end1.vecMin() - end0.vecMin());
		mid.vecMax() += mix * (end1.vecMax() - end0.vecMax());

		// combine with our starting bound.
		bound.Encapsulate(&mid);

		// now combine the bound with any keys that fall between our start
		// and end times.
		while(startKey < endKey_1)
		{
			startKey++;
			CqBound B(m_Keys[startKey]->GetBound());
			bound.Encapsulate(&B);
		}

		m_BoundList.Set( i, bound, time - dt );

		// now set our new start to our current end ready for the next bound.
		bound = mid;
		time += dt;
	}

	m_BoundReady = true;
}


//---------------------------------------------------------------------
/** Sample the specified point against the MPG at the specified time.
 * \param vecSample 2D vector to sample against.
 * \param time Shutter time to sample at.
 * \param D Storage for depth if valid hit.
 * \return Boolean indicating smaple hit.
 */

bool CqMicroPolygonMotion::Sample( CqHitTestCache& hitTestCache, const SqSampleData& sample, TqFloat& D, TqFloat time, bool UsingDof ) const
{
	const CqVector2D vecSample = sample.m_Position;
	CqVector3D points[4];

	// Calculate the position in time of the MP.
	TqInt iIndex = 0;
	TqFloat Fraction = 0.0f;
	bool Exact = true;

	if ( time > m_Times.front() )
	{
		if ( time >= m_Times.back() )
			iIndex = m_Times.size() - 1;
		else
		{
			// Find the appropriate time span.
			iIndex = 0;
			while ( time >= m_Times[ iIndex + 1 ] )
				iIndex += 1;
			Fraction = ( time - m_Times[ iIndex ] ) / ( m_Times[ iIndex + 1 ] - m_Times[ iIndex ] );
			Exact = ( m_Times[ iIndex ] == time );
		}
	}

	// Interpolate the bounding box along the motion segment to get the
	// bounding box at the appropriate sample time.
	CqBound tightBound;
	if(Exact)
	{
		tightBound = m_Keys[iIndex]->GetBound();
	}
	else
	{
		const CqBound& bound1 = m_Keys[iIndex]->GetBound();
		const CqBound& bound2 = m_Keys[iIndex+1]->GetBound();
		tightBound = CqBound(
			(1-Fraction)*bound1.vecMin() + Fraction*bound2.vecMin(),
			(1-Fraction)*bound1.vecMax() + Fraction*bound2.vecMax()
		);
	}
	// Cull potential sample hits before the more expensive point-in-polygon
	// test by checking that the sample actually lies within the tight bound.
	if(UsingDof)
	{
		// For DoF, we also need to displace the sample pos by the
		// minimum & maximum DoF offsets before checking against the tight bound.
		//
		// A possible optimization for scenes which have lots of motion toward
		// the camera is to use the interpolated bound depths to get the CoC
		// multipliers directly, but this proved to be about 8% slower on the
		// simple example, dofmb.rib, in the RTS.
		/*
		const CqRenderer* renderContext = QGetRenderContext();
		CqVector2D cocMult1 = renderContext->GetCircleOfConfusion(tightBound.vecMin().z());
		CqVector2D cocMult2 = renderContext->GetCircleOfConfusion(tightBound.vecMax().z());
		*/
		if(!dofSampleInBound(tightBound, hitTestCache, sample))
			return false;
	}
	else
	{
		if( !tightBound.Contains2D(vecSample) )
			return false;
	}

	// If we get to here, it's fairly likely that the sample will actually hit
	// the micropolygon so we need to compute the actual position of the
	// micropolygon vertices at the sample time.

	// Interpolate the polygon vertices along the motion segment.
	if ( Exact )
	{
		CqMovingMicroPolygonKey * pMP1 = m_Keys[ iIndex ];
		points[1] = pMP1->m_Point0;
		points[2] = pMP1->m_Point1;
		points[3] = pMP1->m_Point2;
		points[0] = pMP1->m_Point3;
	}
	else
	{
		TqFloat F1 = 1.0f - Fraction;
		CqMovingMicroPolygonKey* pMP1 = m_Keys[ iIndex ];
		CqMovingMicroPolygonKey* pMP2 = m_Keys[ iIndex + 1 ];
		points[1] = ( F1 * pMP1->m_Point0 ) + ( Fraction * pMP2->m_Point0 );
		points[2] = ( F1 * pMP1->m_Point1 ) + ( Fraction * pMP2->m_Point1 );
		points[3] = ( F1 * pMP1->m_Point2 ) + ( Fraction * pMP2->m_Point2 );
		points[0] = ( F1 * pMP1->m_Point3 ) + ( Fraction * pMP2->m_Point3 );
	}

	if(UsingDof)
	{
		const CqRenderer* renderContext = QGetRenderContext();
		// Adjust the micropolygon vertices by the DoF offest.
		CqVector2D dofOffset = sample.m_DofOffset;
		points[0] -= vectorCast<CqVector3D>(compMul(renderContext->GetCircleOfConfusion(
						points[0].z()), dofOffset));
		points[1] -= vectorCast<CqVector3D>(compMul(renderContext->GetCircleOfConfusion(
						points[1].z()), dofOffset));
		points[2] -= vectorCast<CqVector3D>(compMul(renderContext->GetCircleOfConfusion(
						points[2].z()), dofOffset));
		points[3] -= vectorCast<CqVector3D>(compMul(renderContext->GetCircleOfConfusion(
						points[3].z()), dofOffset));
	}
	// Fill in the hit test coefficients for the current sample.
	CacheHitTestValues(&hitTestCache, points);

	if ( CqMicroPolygon::fContains(hitTestCache, vecSample, D, time) )
	{
		// Now check if it is trimmed.
		if ( IsTrimmed() )
		{
			// Get the required trim curve sense, if specified, defaults to "inside".

			/// \todo: Implement trimming of motion blurred surfaces!
		}

		if ( pGrid() ->fTriangular() )
		{
			CqVector3D vA, vB;
			pGrid()->TriangleSplitPoints( vA, vB, time );
			TqFloat Ax = vA.x();
			TqFloat Ay = vA.y();
			TqFloat Bx = vB.x();
			TqFloat By = vB.y();

			TqFloat v = ( Ay - By ) * vecSample.x() + ( Bx - Ax ) * vecSample.y() + ( Ax * By - Bx * Ay );
			if( v <= 0 )
				return ( false );
		}
		return ( true );
	}
	else
		return ( false );
}

void CqMicroPolygonMotion::CacheCocMultipliers(CqHitTestCache& cache) const
{
	// Here we cache the CoC range for the full MB bound to be used in the
	// tight DoF bounds test.  We can't calculate and cache the CoC values for
	// the four corners of the micropolygon like in the static case, since the
	// depth is a function of time in general.
	const CqRenderer* renderContext = QGetRenderContext();
	CqVector2D cocMult1 = renderContext->GetCircleOfConfusion(GetBound().vecMin().z());
	CqVector2D cocMult2 = renderContext->GetCircleOfConfusion(GetBound().vecMax().z());
	if(renderContext->MinCoCForBound(GetBound()) == 0)
	{
		// special case for when the bound crosses the focal plane, in which
		// case the min() of the CoC multipliers retrieved above doesn't
		// actually give the correct minimum.
		cache.cocMultMin = CqVector2D(0,0);
	}
	else
		cache.cocMultMin = min(cocMult1, cocMult2);
	cache.cocMultMax = max(cocMult1, cocMult2);
}

//---------------------------------------------------------------------
/** Store the vectors of the micropolygon at the specified shutter time.
 * \param vA 3D Vector.
 * \param vB 3D Vector.
 * \param vC 3D Vector.
 * \param vD 3D Vector.
 * \param time Float shutter time that this MPG represents.
 */

void CqMicroPolygonMotion::AppendKey( const CqVector3D& vA, const CqVector3D& vB, const CqVector3D& vC, const CqVector3D& vD, TqFloat time )
{
	//	assert( time >= m_Times.back() );

	// Add a new planeset at the specified time.
	CqMovingMicroPolygonKey * pMP = new CqMovingMicroPolygonKey( vA, vB, vC, vD );
	m_Times.push_back( time );
	m_Keys.push_back( pMP );
	if ( m_Times.size() == 1 )
		m_Bound = pMP->GetBound();
	else
	{
		CqBound B(pMP->GetBound());
		m_Bound.Encapsulate( &B );
	}
}


//---------------------------------------------------------------------
/** Store the vectors of the micropolygon.
 * \param vA 3D Vector.
 * \param vB 3D Vector.
 * \param vC 3D Vector.
 * \param vD 3D Vector.
 */

void CqMovingMicroPolygonKey::Initialise( const CqVector3D& vA, const CqVector3D& vB, const CqVector3D& vC, const CqVector3D& vD )
{
	// Check for degenerate case, if any of the neighbouring points are the same, shuffle them down, and
	// duplicate the last point exactly. Exact duplication of the last two points is used as a marker in the
	// fContains function to indicate degeneracy. If more that two points are coincident, we are in real trouble!
	const CqVector3D & vvB = ( vA - vB ).Magnitude() < 1e-2 ? vC : vB;
	const CqVector3D& vvC = ( vvB - vC ).Magnitude() < 1e-2 ? vD : vC;
	const CqVector3D& vvD = ( vvC - vD ).Magnitude() < 1e-2 ? vvC : vD;

	// Determine whether the MPG is CW or CCW, must be CCW for fContains to work.
	bool fFlip = ( ( vA.x() - vvB.x() ) * ( vvB.y() - vvC.y() ) ) >= ( ( vA.y() - vvB.y() ) * ( vvB.x() - vvC.x() ) );

	if ( !fFlip )
	{
		m_Point0 = vA;
		m_Point1 = vvD;
		m_Point2 = vvC;
		m_Point3 = vvB;
	}
	else
	{
		m_Point0 = vA;
		m_Point1 = vvB;
		m_Point2 = vvC;
		m_Point3 = vvD;
	}

	m_N = ( vA - vvB ) % ( vvC - vvB );
	m_N.Unit();
	m_D = m_N * vA;

	m_BoundReady = false;
}


//---------------------------------------------------------------------
/** Calculate the 2D boundary of this micropolygon,
 */

const CqBound& CqMovingMicroPolygonKey::GetBound()
{
	if(m_BoundReady)
		return m_Bound;

	// Calculate the boundary, and store the indexes in the cache.
	m_Bound.vecMin().x( min( m_Point0.x(), min( m_Point1.x(), min( m_Point2.x(), m_Point3.x() ) ) ) );
	m_Bound.vecMin().y( min( m_Point0.y(), min( m_Point1.y(), min( m_Point2.y(), m_Point3.y() ) ) ) );
	m_Bound.vecMin().z( min( m_Point0.z(), min( m_Point1.z(), min( m_Point2.z(), m_Point3.z() ) ) ) );
	m_Bound.vecMax().x( max( m_Point0.x(), max( m_Point1.x(), max( m_Point2.x(), m_Point3.x() ) ) ) );
	m_Bound.vecMax().y( max( m_Point0.y(), max( m_Point1.y(), max( m_Point2.y(), m_Point3.y() ) ) ) );
	m_Bound.vecMax().z( max( m_Point0.z(), max( m_Point1.z(), max( m_Point2.z(), m_Point3.z() ) ) ) );

	m_BoundReady = true;
	return ( m_Bound );
}


} // namespace Aqsis
//---------------------------------------------------------------------
