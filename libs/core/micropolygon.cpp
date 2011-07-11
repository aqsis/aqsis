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

#include	<aqsis/aqsis.h>
#include	"stats.h"
#include	"imagebuffer.h"
#include	"micropolygon.h"
#include	"renderer.h"
#include	"surface.h"
#include	"lights.h"
#include	"shaders.h"
#include	"trimcurve.h"
#include	<aqsis/math/derivatives.h>
#include	"bucketprocessor.h"

#include	"mpdump.h"

namespace Aqsis {


CqObjectPool<CqMicroPolygon> CqMicroPolygon::m_thePool;
CqObjectPool<CqMovingMicroPolygonKey>	CqMovingMicroPolygonKey::m_thePool;

void CqMicroPolyGridBase::CacheGridInfo(const boost::shared_ptr<const CqSurface>& surface)
{
	const IqAttributes& attrs = *pAttributes();
	// Determine the matte flag type.
	switch(attrs.GetIntegerAttribute("System", "Matte")[0])
	{
		case 0:  m_CurrentGridInfo.matteFlag = 0;                              break;
		default: m_CurrentGridInfo.matteFlag = SqImageSample::Flag_Matte;      break;
		case 2:  m_CurrentGridInfo.matteFlag = SqImageSample::Flag_MatteAlpha; break;
	}

	// Cache the shading interpolation type.
	m_CurrentGridInfo.useSmoothShading = attrs.GetIntegerAttribute("System",
			"ShadingInterpolation")[0] == ShadingInterp_Smooth;

	m_CurrentGridInfo.usesDataMap
		= !(QGetRenderContext() ->GetMapOfOutputDataEntries().empty());

	m_CurrentGridInfo.lodBounds
		= attrs.GetFloatAttribute("System", "LevelOfDetailBounds");
}


//---------------------------------------------------------------------
/** Default constructor
 */

CqMicroPolyGrid::CqMicroPolyGrid() : CqMicroPolyGridBase(),
		m_bShadingNormals( false ),
		m_bGeometricNormals( false ), 
		m_pShaderExecEnv(IqShaderExecEnv::create(QGetRenderContextI()))
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
	CacheGridInfo(pSurface);

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

	const CqVector3D* pP = 0;
	pVar(EnvVars_P)->GetPointPtr(pP);
	CqVector3D* pNg = 0;
	pVar(EnvVars_Ng)->GetNormalPtr(pNg);

	TqInt uRes = uGridRes()+1;
	TqInt vRes = vGridRes()+1;

	// For numerical robustness we need to estimate the expected length of N
	// so we can detect degenerate situations where N came out to be
	// essentially equal to zero.  We have N = dP_u x dP_v, so that the
	// expected N length is
	//
	//   length(N) ~ length(dP_u)*length(dP_v)
	//
	// We estimate the lengths of dP_u and dP_v using the grid diagonal:
	//
	//   length(dP_u) ~ length(dP_v) ~ length_of_grid_diagonal / number_of_micropolys_on_diag
	//
	const TqFloat expecNlen = (pP[0] - pP[uRes*vRes-1]).Magnitude2()/(uRes*uRes + vRes*vRes);
	// Our tolerance scaling for lengths to be considered "too small" is:
	const TqFloat eps = 100*FLT_EPSILON;
	// tolerances for N^2 & dP_u^2 / dP_v^2 to be considered too small:
	const TqFloat epsNlen2 = expecNlen*expecNlen*eps*eps;
	const TqFloat epsdPlen2 = expecNlen*eps*eps;

	CqGridDiff d = m_pShaderExecEnv->GridDiff();
	for(TqInt v = 0, i = 0; v < vRes; ++v)
	{
		for(TqInt u = 0; u < uRes; ++u, ++i)
		{
			CqVector3D dP_u = d.diffU(pP, u, v);
			CqVector3D dP_v = d.diffV(pP, u, v);
			CqVector3D N = dP_u % dP_v;
			if(N.Magnitude2() < epsNlen2)
			{
				// If the normal is too small, the grid is probably locally
				// degenerate; try some neighbouring points as a fallback to
				// compute a guess at the normal for the current shading point.
				if(dP_u.Magnitude2() < epsdPlen2)
					dP_u = d.diffU(pP, u, v > 0 ? v-1 : v+1);
				if(dP_v.Magnitude2() < epsdPlen2)
					dP_v = d.diffV(pP, u > 0 ? u-1 : u+1, v);
				N = dP_u % dP_v;
			}
			if(flipNormals)
				N = -N;
			N.Unit();
			pNg[i] = N;
		}
	}
}

void CqMicroPolyGrid::CalcSurfaceDerivatives()
{
	/// \todo <b>Code review</b>: This function redoes work which is already done in CalcNormals
	const CqVector3D* pP = 0;
	pVar(EnvVars_P)->GetPointPtr(pP);

	TqInt lUses = pSurface() ->Uses();

	TqFloat invDu = 1;
	CqVector3D* dPdu = 0;
	if(USES(lUses, EnvVars_dPdu))
	{
		pVar(EnvVars_dPdu)->GetVectorPtr(dPdu);
		pVar(EnvVars_du)->GetFloat(invDu);
		invDu = 1/invDu;
	}

	TqFloat invDv = 1;
	CqVector3D* dPdv = 0;
	if(USES(lUses, EnvVars_dPdv))
	{
		pVar(EnvVars_dPdv)->GetVectorPtr(dPdv);
		pVar(EnvVars_dv)->GetFloat(invDv);
		invDv = 1/invDv;
	}

	TqInt uRes = uGridRes()+1;
	TqInt vRes = vGridRes()+1;

	CqGridDiff d = m_pShaderExecEnv->GridDiff();
	for(TqInt v = 0, i = 0; v < vRes; ++v)
	{
		for(TqInt u = 0; u < uRes; ++u, ++i)
		{
			if(dPdu)
				dPdu[i] = d.diffU(pP, u, v)*invDu;
			if(dPdv)
				dPdv[i] = d.diffV(pP, u, v)*invDv;
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

	// Set I, the incident ray direction.
	switch(QGetRenderContext()->GetIntegerOption("System", "Projection")[0])
	{
		case ProjectionOrthographic:
			{
				// For an orthographic camera, all incoming rays are parallel
				// and in the (0,0,1) direction.  The length of I is set to the
				// z-component of P so that it represents the distance from the
				// ray origin (somewhere on the xy plane) to the current
				// shading point.
				const CqVector3D* pP = 0;
				pVar(EnvVars_P)->GetPointPtr(pP);
				CqVector3D* pI = 0;
				pVar(EnvVars_I)->GetVectorPtr(pI);
				for(TqInt i = 0; i < gs; ++i)
					pI[i] = CqVector3D(0,0,pP[i].z());
			}
			break;
		case ProjectionPerspective:
		default:
			// I is just equal to P in shading (camera) coords for a projective
			// camera transformation.
			pVar(EnvVars_I)->SetValueFromVariable(pVar(EnvVars_P));
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
	if ( ( pAttributes() ->GetIntegerAttribute( "System", "Sides" ) [ 0 ] == 1 ) && !m_pCSGNode &&
		 ( pAttributes() ->GetIntegerAttributeDef( "cull", "backfacing", 1 ) == 1 ) )
	{
		AQSIS_TIME_SCOPE(Backface_culling);

		TqInt cCulled = 0;
		const CqVector3D* pP = NULL;
		pVar(EnvVars_P) ->GetPointPtr( pP );
		const CqVector3D* pN = NULL;
		if ( USES( lUses, EnvVars_N ) )
			pVar(EnvVars_N) ->GetNormalPtr( pN );
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
	const CqColor* zThr = QGetRenderContext()->poptCurrent()
	                      ->GetColorOption("limits", "zthreshold");
	if ( USES( lUses, EnvVars_Oi ) && !(zThr && *zThr == gColBlack) )
	{
		AQSIS_TIME_SCOPE(Transparency_culling_micropolygons);

		const CqColor* pOi = NULL;
		pVar(EnvVars_Oi)->GetColorPtr( pOi );
		
		TqInt cCulled = 0;
		for (TqInt i = gsmin1; i >= 0; i-- )
		{
			if ( pOi[ i ] == gColBlack )
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
	DeleteVariables( false );

	STATS_INC( GRD_shd_size_4 + clamp<TqInt>( CqStats::stats_log2(
					m_pShaderExecEnv->shadingPointCount() ) - 2, 0, 7 ) );
}

//---------------------------------------------------------------------
/** Transfer any shader variables marked as "otuput" as they may be needed by the display devices.
 */

void CqMicroPolyGrid::TransferOutputVariables()
{
	boost::shared_ptr<IqShader> pSurface = this->pAttributes()->pshadSurface(QGetRenderContext()->Time());
	boost::shared_ptr<IqShader> pAtmosphere = this->pAttributes()->pshadAtmosphere(QGetRenderContext()->Time());

	// Only bother transferring ones that have been used in a RiDisplay request.
	std::map<std::string, CqRenderer::SqOutputDataEntry>& outputVars = QGetRenderContext()->GetMapOfOutputDataEntries();
	std::map<std::string, CqRenderer::SqOutputDataEntry>::iterator outputVar;
	for( outputVar = outputVars.begin(); outputVar != outputVars.end(); outputVar++ )
	{
		IqShaderData* outputData = pSurface->FindArgument( outputVar->first );
		if( !outputData && pAtmosphere )
			outputData = pAtmosphere->FindArgument( outputVar->first );
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
	{
		// Oi is almost always needed, even in z-buffer mode.  The only time
		// it's not needed is when the zthreshold color is [0,0,0], which makes
		// all surfaces (even fully transparent) make it into the depth output.
		const CqColor* zThr = QGetRenderContext()->poptCurrent()
		                      ->GetColorOption( "limits", "zthreshold" );
		if ( all || (zThr && *zThr == CqColor(0.0f)) )
			m_pShaderExecEnv->DeleteVariable( EnvVars_Oi );
	}
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
					pNew->AppendKey( aaPtimes[ keyFrame->second ][ iIndex ], aaPtimes[ keyFrame->second ][ iIndex + 1 ], aaPtimes[ keyFrame->second ][ iIndex + cu + 1 ], aaPtimes[ keyFrame->second ][ iIndex + cu + 2 ],  keyFrame->first);
				pNew->Initialise();
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
	bool canBeBFCulled = ( pAttributes() ->GetIntegerAttribute( "System", "Sides" ) [ 0 ] == 1 ) && !pGridA->usesCSG() &&
						 ( pAttributes() ->GetIntegerAttributeDef( "cull", "backfacing", 1 ) == 1 );

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

		CqMicroPolyGrid* pg = static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( iTime ) ) );
		CqVector3D* pP;
		pg->pVar(EnvVars_P) ->GetPointPtr( pP );
		CqVector3D* pNg;
		pg->pVar(EnvVars_Ng)->GetNormalPtr(pNg);
		CqVector3D* pN = NULL;
		if ( USES( lUses, EnvVars_N ) )
			pg->pVar(EnvVars_N)->GetNormalPtr(pN);

		// Cull any hidden MPs if Sides==1.  Note that this has to happen
		// *before* the transformation into hybrid camera/raster space below.
		if ( canBeBFCulled )
		{
			AQSIS_TIME_SCOPE(Backface_culling);
			if(iTime > 0)
			{
				// The primary grid already has geometric normals computed
				// during shading, but additional grid keys don't so we need
				// to compute them here.
				pg->CalcNormals();
			}
			for ( i = gsmin1; i >= 0; i-- )
			{
				// When backface culling, we must use the geometric normal (Ng)
				// as this is the normal that properly represents the actual
				// micropolygon geometry. However, if the primitive specifies
				// custom normals as primitive variables, the direction of
				// those should be honored, in case the user has intentionally
				// switched the surface direction.  Therefore, we compare the
				// direction of Ng with that of N and flip Ng if they don't
				// match.
				TqFloat s = 1.0f;
				if(pN)
					s = (pN[i] * pNg[i] < 0.0f) ? -1.0f : 1.0f;
				if(s * pNg[i] * pP[i] >= 0)
					totalBFCulled[i]++;
			}
		}

		// Transform the whole grid to hybrid camera/raster space
		for ( i = gsmin1; i >= 0; i-- )
		{
			CqVector3D Point( pP[ i ] );

			// Make sure to retain camera space 'z' coordinate.
			TqFloat zdepth = Point.z();
			aaPtimes[ iTime ][ i ] = matCameraToRaster * Point;
			aaPtimes[ iTime ][ i ].z( zdepth );
			pP[ i ] = aaPtimes[ iTime ][ i ];
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
				pNew->AppendKey( aaPtimes[ iTime ][ iIndex ], aaPtimes[ iTime ][ iIndex + 1 ], aaPtimes[ iTime ][ iIndex + cu + 1 ], aaPtimes[ iTime ][ iIndex + cu + 2 ], Time( iTime ) );
			pNew->Initialise();
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
void CqMicroPolygon::Initialise()
{
	ComputeVertexOrder();
	CalculateBound();
}

void CqMicroPolygon::GetVertices(CqVector3D P[4]) const
{
	CqVector3D* pP;
	m_pGrid->pVar(EnvVars_P) ->GetPointPtr(pP);
	TqInt cu = m_pGrid->uGridRes();
	P[0] = pP[ m_Index ];
	P[1] = pP[ m_Index + 1 ];
	P[2] = pP[ m_Index + cu + 1 ];
	P[3] = pP[ m_Index + cu + 2 ];
}

void CqMicroPolygon::ComputeVertexOrder()
{
	// Check for degenerate case, if any of the neighbouring points are the
	// same, shuffle them down, and duplicate the last point exactly.
	// Degeneracy is indicated with the bit Degeneracy_Mask in m_IndexCode.  If more
	// that two points are coincident, we are in real trouble!
	TqInt cu = m_pGrid->uGridRes();
	TqInt IndexA = m_Index;
	TqInt IndexB = m_Index + 1;
	TqInt IndexC = m_Index + cu + 2;
	TqInt IndexD = m_Index + cu + 1;

	TqShort CodeA = 0;
	TqShort CodeB = 1;
	TqShort CodeC = 3;
	TqShort CodeD = 2;

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
		              ( ( CodeA & 0x3 ) | ( ( CodeC & 0x3 ) << 2 ) | ( ( CodeB & 0x3 ) << 4 ) | Degeneracy_Mask ) :
		              ( ( CodeA & 0x3 ) | ( ( CodeD & 0x3 ) << 2 ) | ( ( CodeC & 0x3 ) << 4 ) | ( ( CodeB & 0x3 ) << 6 ) );
	}
	else
	{
		m_IndexCode = ( CodeD == -1 ) ?
		              ( ( CodeA & 0x3 ) | ( ( CodeB & 0x3 ) << 2 ) | ( ( CodeC & 0x3 ) << 4 ) | Degeneracy_Mask ) :
		              ( ( CodeA & 0x3 ) | ( ( CodeB & 0x3 ) << 2 ) | ( ( CodeC & 0x3 ) << 4 ) | ( ( CodeD & 0x3 ) << 6 ) );
	}
}


//---------------------------------------------------------------------
/** Determinde whether the 2D point specified lies within this micropolygon.
 * \param vecP 2D vector to test for containment.
 * \param Depth Place to put the depth if valid intersection.
 * \param time The frame time at which to check containment.
 * \return Boolean indicating sample hit.
 */

bool CqMicroPolygon::fContains( CqHitTestCache& hitTestCache, const CqVector2D& vecP, TqFloat& Depth, CqVector2D& uv, TqFloat time) const
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

	uv = hitTestCache.xyToUV(vecP);
	const TqFloat* z = hitTestCache.z;
	Depth = bilerp(z[0], z[1], z[2], z[3], uv);

	return true;
}

//---------------------------------------------------------------------

void CqMicroPolygon::cachePointInPolyTest(CqHitTestCache& cache,
										  CqVector3D* pointsIn) const
{
	cache.z[0] = pointsIn[0].z();
	cache.z[1] = pointsIn[1].z();
	cache.z[2] = pointsIn[2].z();
	cache.z[3] = pointsIn[3].z();
	cache.xyToUV.setVertices(vectorCast<CqVector2D>(pointsIn[0]),
							 vectorCast<CqVector2D>(pointsIn[1]),
							 vectorCast<CqVector2D>(pointsIn[2]),
							 vectorCast<CqVector2D>(pointsIn[3]));

	// The point-in-poly test requires a carefully chosen order for the points,
	// and special cases when the micropoly is degenerate.  This information is
	// stored in m_IndexCode and used to reorder the points before calculating
	// the point-in-poly coefficients.
	const CqVector3D points[4] = {
		pointsIn[(m_IndexCode >> 2) & 0x3],
		pointsIn[(m_IndexCode >> 4) & 0x3],
		pointsIn[(m_IndexCode >> 6) & 0x3],
		pointsIn[(m_IndexCode ) & 0x3],
	};
	int j = 3;
	for(int i=0; i<4; ++i)
	{
		cache.m_YMultiplier[i] = points[i].x() - points[j].x();
		cache.m_XMultiplier[i] = points[i].y() - points[j].y();
		cache.m_X[i] = points[j].x();
		cache.m_Y[i] = points[j].y();
		j = i;
	}

	// if the mpg is degenerate then we repeat edge c=>a so we still have four
	// edges (it makes the test in fContains() simpler).
	if(m_IndexCode & Degeneracy_Mask)
	{
		for(int i=2; i<4; ++i)
		{
			cache.m_YMultiplier[i] = points[3].x() - points[1].x();
			cache.m_XMultiplier[i] = points[3].y() - points[1].y();
			cache.m_X[i] = points[1].x();
			cache.m_Y[i] = points[1].y();
		}
	}

	cache.m_LastFailedEdge = 0;
}

void CqMicroPolygon::CacheHitTestValues(CqHitTestCache& cache, bool usingDof) const
{
	// First grab and cache the points.
	const CqVector3D* gridP = 0;
	m_pGrid->pVar(EnvVars_P)->GetPointPtr(gridP);
	TqInt uGridRes = m_pGrid->uGridRes();
	CqVector3D* P = cache.P;
	P[0] = gridP[m_Index];
	P[1] = gridP[m_Index + 1];
	P[2] = gridP[m_Index + uGridRes + 1];
	P[3] = gridP[m_Index + uGridRes + 2];

	if(usingDof)
	{
		// When using depth of field, the circle of confusion multipliers for
		// each vertex of a static micropolygon are independent of the sample
		// point and can be cached.  The minimum and maximum of these
		// multipliers can also be cached, and used in the bounding box test
		// for fast sample rejection.
		const CqRenderer* renderContext = QGetRenderContext();
		cache.cocMult[0] = renderContext->GetCircleOfConfusion(P[0].z());
		cache.cocMult[1] = renderContext->GetCircleOfConfusion(P[1].z());
		cache.cocMult[2] = renderContext->GetCircleOfConfusion(P[2].z());
		cache.cocMult[3] = renderContext->GetCircleOfConfusion(P[3].z());

		cache.cocMultMin = min(min(cache.cocMult[0], cache.cocMult[1]),
							   min(cache.cocMult[2], cache.cocMult[3]));
		cache.cocMultMax = max(max(cache.cocMult[0], cache.cocMult[1]),
							   max(cache.cocMult[2], cache.cocMult[3]));
	}
	else
	{
		// When not using depth of field, the coefficients for the
		// point-in-polygon test itself can be cached, since the vertices of
		// the micropolygon aren't effected by any aspect of the sample points
		// themselves.
		cachePointInPolyTest(cache, P);
	}
}

void CqMicroPolygon::CacheOutputInterpCoeffs(SqMpgSampleInfo& cache) const
{
	if(cache.smoothInterpolation)
		CacheOutputInterpCoeffsSmooth(cache);
	else
		CacheOutputInterpCoeffsConstant(cache);
}


void CqMicroPolygon::InterpolateOutputs(const SqMpgSampleInfo& cache,
	const CqVector2D& uv, CqColor& outCol, CqColor& outOpac) const
{
	if(cache.smoothInterpolation)
	{
		// bilinear interpolation.
		TqFloat w0 = (1-uv.x())*(1-uv.y());
		TqFloat w1 = uv.x()*(1-uv.y());
		TqFloat w2 = (1-uv.x())*uv.y();
		TqFloat w3 = uv.x()*uv.y();

		outCol = w0*cache.col[0] + w1*cache.col[1] + w2*cache.col[2] + w3*cache.col[3];
		outOpac = w0*cache.opa[0] + w1*cache.opa[1] + w2*cache.opa[2] + w3*cache.opa[3];
	}
	else
	{
		outCol = cache.col[0];
		outOpac = cache.opa[0];
	}
}

void CqMicroPolygon::CacheOutputInterpCoeffsConstant(SqMpgSampleInfo& cache) const
{
	if(IqShaderData* Ci = m_pGrid->pVar(EnvVars_Ci))
	{
		const CqColor* col = 0;
		Ci->GetColorPtr(col);
		cache.col[0] = col[m_Index];
	}
	else
	{
		cache.col[0] = CqColor(1.0);
	}

	if(IqShaderData* Oi = m_pGrid->pVar(EnvVars_Oi))
	{
		const CqColor* opa = 0;
		Oi->GetColorPtr(opa);
		cache.opa[0] = opa[m_Index];
		cache.isOpaque = cache.opa[0] >= CqColor(1.0);
	}
	else
	{
		cache.opa[0] = CqColor(1.0);
		cache.isOpaque = true;
	}
}

void CqMicroPolygon::CacheOutputInterpCoeffsSmooth(SqMpgSampleInfo& cache) const
{
	TqInt indices[4] = {m_Index, m_Index+1, m_Index + pGrid()->uGridRes() + 1,
						m_Index + pGrid()->uGridRes() + 2};
	if(IqShaderData* Ci = m_pGrid->pVar(EnvVars_Ci))
	{
		const CqColor* pCi = NULL;
		Ci->GetColorPtr(pCi);

		for(TqInt i = 0; i < 4; ++i)
			cache.col[i] = pCi[indices[i]];
	}
	else
	{
		for(TqInt i = 0; i < 4; ++i)
			cache.col[i] = CqColor(1.0f);
	}

	if(IqShaderData* Oi = m_pGrid->pVar(EnvVars_Oi))
	{
		const CqColor* pOi = NULL;
		Oi->GetColorPtr(pOi);

		for(TqInt i = 0; i < 4; ++i)
			cache.opa[i] = pOi[indices[i]];

		// The micropoly isOpaque if the values on all vertices are.
		cache.isOpaque = (cache.opa[0] >= gColWhite) &&
						 (cache.opa[1] >= gColWhite) &&
						 (cache.opa[2] >= gColWhite) &&
						 (cache.opa[3] >= gColWhite);
	}
	else
	{
		for(TqInt i = 0; i < 4; ++i)
			cache.opa[i] = CqColor(1.0f);
		cache.isOpaque = true;
	}
}

inline bool CqMicroPolygon::dofSampleInBound(const CqBound& bound,
		const CqHitTestCache& cache, SqSampleData const& sample)
{
	CqVector2D dofOffset = sample.dofOffset;
	CqVector2D samplePos = sample.position;
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
 * \param uv - Output for parametric coordinates inside the micropolygon.
 * \return Boolean indicating smaple hit.
 */

bool CqMicroPolygon::Sample( CqHitTestCache& hitTestCache, SqSampleData const& sample, TqFloat& D, CqVector2D& uv, TqFloat time, bool UsingDof ) const
{
	CqVector2D vecSample = sample.position;

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
		CqVector2D* coc = hitTestCache.cocMult;
		CqVector2D dofOffset = sample.dofOffset;
		CqVector3D* P = hitTestCache.P;
		CqVector3D points[4] = {
			P[0] - vectorCast<CqVector3D>(compMul(coc[0], dofOffset)),
			P[1] - vectorCast<CqVector3D>(compMul(coc[1], dofOffset)),
			P[2] - vectorCast<CqVector3D>(compMul(coc[2], dofOffset)),
			P[3] - vectorCast<CqVector3D>(compMul(coc[3], dofOffset))
		};
		// Having displaced and slightly distorted the micropolygon, we now
		// need to calculate the hit test coefficients.
		cachePointInPolyTest(hitTestCache, points);
	}

	if ( fContains( hitTestCache, vecSample, D, uv, time ) )
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

			CqVector2D vR = BilinearEvaluate( uvA, uvB, uvC, uvD, uv.x(), uv.y() );

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

			CqVector2D hitPos = vecSample;
			if(UsingDof)
			{
				// DoF interacts with the triangle split line computation: the
				// micropolygon verts have been moved during the hit
				// calculation, so we need to move the apparent position of the
				// hit in the opposite direction before determining which side
				// of the triangle split line the hit lies on.
				CqVector2D cocMult = QGetRenderContext()->GetCircleOfConfusion(D);
				hitPos += compMul(cocMult, sample.dofOffset);
			}

			TqFloat v = (Ay - By)*hitPos.x() + (Bx - Ax)*hitPos.y() + (Ax*By - Bx*Ay);
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
// CqMicroPolygonMotion implementation
//---------------------------------------------------------------------

void CqMicroPolygonMotion::Initialise()
{
	ComputeVertexOrder();
}

//---------------------------------------------------------------------
/** Calculate a list of 2D bounds for this micropolygon,
 */
void CqMicroPolygonMotion::BuildBoundList(TqUint timeRanges)
{
	TqFloat opentime = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Shutter" ) [ 0 ];
	TqFloat closetime = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Shutter" ) [ 1 ];

	m_BoundList.Clear();

	assert( NULL != m_Keys[ 0 ] );

	// Compute an approximation of the number of micropolygon lengths moved in
	// raster space.  We use this to guide how many sub-bounds to calcuate.
	TqFloat polyLen2 = vectorCast<CqVector2D>(m_Keys.front()->GetBound().vecCross())
						.Magnitude2();
	TqFloat moveDist2 = (vectorCast<CqVector2D>(m_Keys.front()->m_Point0)
						- vectorCast<CqVector2D>(m_Keys.back()->m_Point0))
						.Magnitude2();
	TqInt polyLengthsMoved = max<TqInt>(1, lfloor(std::sqrt(moveDist2/polyLen2)));

	TqUint divisions = min<TqInt>(polyLengthsMoved, timeRanges);
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
 * \param uv - Output for parametric coordinates inside the micropolygon.
 * \return Boolean indicating smaple hit.
 */

bool CqMicroPolygonMotion::Sample( CqHitTestCache& hitTestCache, SqSampleData const& sample, TqFloat& D, CqVector2D& uv, TqFloat time, bool UsingDof ) const
{
	const CqVector2D vecSample = sample.position;
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
		points[0] = pMP1->m_Point0;
		points[1] = pMP1->m_Point1;
		points[2] = pMP1->m_Point2;
		points[3] = pMP1->m_Point3;
	}
	else
	{
		TqFloat F1 = 1.0f - Fraction;
		CqMovingMicroPolygonKey* pMP1 = m_Keys[ iIndex ];
		CqMovingMicroPolygonKey* pMP2 = m_Keys[ iIndex + 1 ];
		points[0] = ( F1 * pMP1->m_Point0 ) + ( Fraction * pMP2->m_Point0 );
		points[1] = ( F1 * pMP1->m_Point1 ) + ( Fraction * pMP2->m_Point1 );
		points[2] = ( F1 * pMP1->m_Point2 ) + ( Fraction * pMP2->m_Point2 );
		points[3] = ( F1 * pMP1->m_Point3 ) + ( Fraction * pMP2->m_Point3 );
	}

	if(UsingDof)
	{
		const CqRenderer* renderContext = QGetRenderContext();
		// Adjust the micropolygon vertices by the DoF offest.
		CqVector2D dofOffset = sample.dofOffset;
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
	cachePointInPolyTest(hitTestCache, points);

	if ( CqMicroPolygon::fContains(hitTestCache, vecSample, D, uv, time) )
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

			CqVector2D hitPos = vecSample;
			if(UsingDof)
			{
				// DoF interacts with the triangle split line computation: the
				// micropolygon verts have been moved during the hit
				// calculation, so we need to move the apparent position of the
				// hit in the opposite direction before determining which side
				// of the triangle split line the hit lies on.
				CqVector2D cocMult = QGetRenderContext()->GetCircleOfConfusion(D);
				hitPos += compMul(cocMult, sample.dofOffset);
			}

			TqFloat v = (Ay - By)*hitPos.x() + (Bx - Ax)*hitPos.y() + (Ax*By - Bx*Ay);
			if( v <= 0 )
				return ( false );
		}
		return ( true );
	}
	else
		return ( false );
}

void CqMicroPolygonMotion::CacheHitTestValues(CqHitTestCache& cache,
											  bool usingDof) const
{
	if(usingDof)
	{
		// Here we cache the CoC range for the full MB bound to be used in the
		// tight DoF bounds test.  We can't calculate and cache the CoC values
		// for the four corners of the micropolygon like in the static case,
		// since the depth is a function of time in general.
		const CqRenderer* renderContext = QGetRenderContext();
		CqVector2D coc1 = renderContext->GetCircleOfConfusion(GetBound().vecMin().z());
		CqVector2D coc2 = renderContext->GetCircleOfConfusion(GetBound().vecMax().z());
		if(renderContext->MinCoCForBound(GetBound()) == 0)
		{
			// special case for when the bound crosses the focal plane, in
			// which case the min() of the CoC multipliers retrieved above
			// doesn't actually give the correct minimum.
			cache.cocMultMin = CqVector2D(0,0);
		}
		else
			cache.cocMultMin = min(coc1, coc2);
		cache.cocMultMax = max(coc1, coc2);
	}
	// If using MB but not DoF, there's nothing that we can really usefully
	// cache here.
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
