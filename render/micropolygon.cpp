// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\author Paul C. Gregory (pgregory@aqsis.com)
        \author Andy Gill (buzz@ucky.com)
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
#include	"focus.h"

START_NAMESPACE( Aqsis )


DEFINE_STATIC_MEMORYPOOL( CqMicroPolygon, 512 );
DEFINE_STATIC_MEMORYPOOL( CqMovingMicroPolygonKey, 512 );

//---------------------------------------------------------------------
/** Default constructor
 */

CqMicroPolyGrid::CqMicroPolyGrid() : CqMicroPolyGridBase(),
		m_bShadingNormals( TqFalse ),
		m_bGeometricNormals( TqFalse ),
		m_pSurface( NULL ),
		m_pCSGNode( NULL ),
		m_pShaderExecEnv( NULL ),
		m_fTriangular( TqFalse )

{
	QGetRenderContext() ->Stats().IncGridsAllocated();
}


//---------------------------------------------------------------------
/** Destructor
 */

CqMicroPolyGrid::~CqMicroPolyGrid()
{
	assert( RefCount() <= 0 );

	QGetRenderContext() ->Stats().IncGridsDeallocated();

	// Release the reference to the attributes.
	if ( m_pSurface != 0 ) RELEASEREF( m_pSurface );
	m_pSurface = 0;

	// Release the reference to the CSG node.
	if ( m_pCSGNode != 0 ) RELEASEREF( m_pCSGNode );
	m_pCSGNode = 0;

	// Delete.
	/// \note This should delete throught the interface that created it.
	if ( m_pShaderExecEnv != 0 ) 
	{
		RELEASEREF( m_pShaderExecEnv );
		m_pShaderExecEnv = 0;
	}

	// Delete any cloned shader output variables.
	std::vector<IqShaderData*>::iterator outputVar;
	for( outputVar = m_apShaderOutputVariables.begin(); outputVar != m_apShaderOutputVariables.end(); outputVar++ )
		if( (*outputVar) )	delete( (*outputVar) );
	m_apShaderOutputVariables.clear();
}

//---------------------------------------------------------------------
/** Constructor
 */

CqMicroPolyGrid::CqMicroPolyGrid( TqInt cu, TqInt cv, CqSurface* pSurface ) :
		m_bShadingNormals( TqFalse ),
		m_bGeometricNormals( TqFalse ),
		m_pSurface( NULL ),
		m_pShaderExecEnv( NULL ),
		m_fTriangular( TqFalse )

{
	QGetRenderContext() ->Stats().IncGridsAllocated();
	// Initialise the shader execution environment

	m_pShaderExecEnv = new CqShaderExecEnv;
	ADDREF( m_pShaderExecEnv );
	Initialise( cu, cv, pSurface );
}


//---------------------------------------------------------------------
/** Initialise the grid ready for processing.
 * \param cu Integer grid resolution.
 * \param cv Integer grid resolution.
 * \param pSurface CqSurface pointer to associated GPrim.
 */

void CqMicroPolyGrid::Initialise( TqInt cu, TqInt cv, CqSurface* pSurface )
{
	// Initialise the shader execution environment
	TqInt lUses = -1;
	if ( pSurface )
	{
		lUses = pSurface->Uses();
		m_pSurface = pSurface;
		ADDREF( m_pSurface );

		m_pCSGNode = pSurface->pCSGNode();
		if ( m_pCSGNode ) ADDREF( m_pCSGNode );
	}
	lUses |= QGetRenderContext()->pDDmanager()->Uses();

	/// \note This should delete through the interface that created it.

	m_pShaderExecEnv->Initialise( cu, cv, pSurface, pSurface->pAttributes() ->pshadSurface(), lUses );

	IqShader* pshadSurface = pSurface ->pAttributes() ->pshadSurface();
	IqShader* pshadDisplacement = pSurface ->pAttributes() ->pshadDisplacement();
	IqShader* pshadAtmosphere = pSurface ->pAttributes() ->pshadAtmosphere();

	if ( NULL != pshadSurface ) pshadSurface->Initialise( cu, cv, m_pShaderExecEnv );
	if ( NULL != pshadDisplacement ) pshadDisplacement->Initialise( cu, cv, m_pShaderExecEnv );
	if ( NULL != pshadAtmosphere ) pshadAtmosphere->Initialise( cu, cv, m_pShaderExecEnv );

	// Initialise the local/public culled variable.
	m_CulledPolys.SetSize( ( cu + 1 ) * ( cv + 1 ) );
	m_CulledPolys.SetAll( TqFalse );
}

//---------------------------------------------------------------------
/** Build the normals list for the micorpolygons in the grid.
 */

void CqMicroPolyGrid::CalcNormals()
{
	if ( NULL == P() || NULL == N() ) return ;

	// Get the handedness of the coordinate system (at the time of creation) and
	// the coordinate system specified, to check for normal flipping.
	//	EqOrientation CSO=pSurface()->pAttributes()->eCoordsysOrientation();
	TqInt O = pAttributes() ->GetIntegerAttribute( "System", "Orientation" ) [ 0 ];
	const CqVector3D* vecMP[ 4 ];
	CqVector3D	vecN, vecTemp;
	CqVector3D	vecLastN( 0, 0, 0 );

	const CqVector3D* pP;
	P() ->GetPointPtr( pP );
	IqShaderData* pNg = Ng();

	// Calculate each normal from the top left, top right and bottom left points.
	register TqInt ur = uGridRes();
	register TqInt vr = vGridRes();
	TqInt igrid = 0;
	TqInt iv;
	for ( iv = 0; iv < vr; iv++ )
	{
		TqInt iu;
		for ( iu = 0; iu < ur; iu++ )
		{
			vecMP[ 0 ] = &pP[ igrid ];
			vecMP[ 1 ] = &pP[ igrid + 1 ];
			vecMP[ 2 ] = &pP[ igrid + ur + 1 ];
			vecMP[ 3 ] = &pP[ igrid + ur ];
			int a, b, c;
			a = 0;
			b = a + 1;
			while ( ( ( ( *vecMP[ a ] ) - ( *vecMP[ b ] ) ).Magnitude() < FLT_EPSILON ) && b < 3 ) b++;
			if ( b < 3 )
			{
				c = b + 1;
				while ( ( ( ( *vecMP[ c ] ) - ( *vecMP[ b ] ) ).Magnitude() < FLT_EPSILON || ( ( *vecMP[ c ] ) - ( *vecMP[ a ] ) ).Magnitude() < FLT_EPSILON ) && c < 3 )
					c++;
				if ( c <= 3 )
				{
					vecN = ( ( *vecMP[ b ] ) - ( *vecMP[ a ] ) ) % ( ( *vecMP[ c ] ) - ( *vecMP[ a ] ) );	// Cross product is normal.*/
					vecN.Unit();
					// Flip the normal if the 'current orientation' differs from the 'coordinate system orientation'
					// see RiSpec 'Orientation and Sides'
					vecN = ( O == OrientationLH ) ? vecN : -vecN;
					vecLastN = vecN;
				}
				else
				{
					//assert(false);
					vecN = vecLastN;
				}
			}
			else
			{
				//assert(false);
				vecN = vecLastN;
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


//---------------------------------------------------------------------
/** Shade the grid using the surface parameters of the surface passed and store the color values for each micropolygon.
 */

void CqMicroPolyGrid::Shade()
{
	register TqInt i;


	// Sanity checks
	if ( NULL == P() || NULL == I() ) return ;

	static CqVector3D	vecE( 0, 0, 0 );
	static CqVector3D	Defvec( 0, 0, 0 );
	

	CqStats& theStats = QGetRenderContext() ->Stats();

	IqShader* pshadSurface = pSurface() ->pAttributes() ->pshadSurface();
	IqShader* pshadDisplacement = pSurface() ->pAttributes() ->pshadDisplacement();
	IqShader* pshadAtmosphere = pSurface() ->pAttributes() ->pshadAtmosphere();

	TqInt lUses = pSurface() ->Uses();
	TqInt gs = GridSize();
	TqInt gsmin1 = gs - 1;
	long cCulled = 0;


	const CqVector3D* pP;
	P() ->GetPointPtr( pP );
	const CqColor* pOs = NULL;
	if ( USES( lUses, EnvVars_Os ) ) Os() ->GetColorPtr( pOs );
	const CqColor* pCs = NULL;
	if ( USES( lUses, EnvVars_Cs ) ) Cs() ->GetColorPtr( pCs );
	IqShaderData* pI = I();
	const CqVector3D* pN = NULL;
	if ( USES( lUses, EnvVars_N ) ) N() ->GetNormalPtr( pN );

	// Calculate geometric normals if not specified by the surface.
	if ( !bGeometricNormals() && USES( lUses, EnvVars_Ng ) )
		CalcNormals();

	// If shading normals are not explicitly specified, they default to the geometric normal.
	if ( !bShadingNormals() && USES( lUses, EnvVars_N ) && NULL != Ng() && NULL != N() )
		N() ->SetValueFromVariable( Ng() );

	// Setup uniform variables.
	if ( USES( lUses, EnvVars_E ) ) E() ->SetVector( vecE );
	if ( USES( lUses, EnvVars_du ) )
	{
		for ( i = gsmin1; i >= 0; i-- )
		{
			TqFloat v1, v2;
			TqInt uRes = uGridRes();
			TqInt GridX = i % ( uRes + 1 );

			if ( GridX < uRes )
			{
				u() ->GetValue( v1, i + 1 );
				u() ->GetValue( v2, i );
				du() ->SetFloat( v1 - v2, i );
			}
			else
			{
				u() ->GetValue( v1, i );
				u() ->GetValue( v2, i - 1 );
				du() ->SetFloat( v1 - v2, i );
			}
		}
	}
	if ( USES( lUses, EnvVars_dv ) )
	{
		for ( i = gsmin1; i >= 0; i-- )
		{
			TqFloat v1, v2;
			TqInt uRes = uGridRes();
			TqInt vRes = vGridRes();
			TqInt GridY = ( i / ( uRes + 1 ) );

			if ( GridY < vRes )
			{
				v() ->GetValue( v1, i + uRes + 1 );
				v() ->GetValue( v2, i );
				dv() ->SetFloat( v1 - v2, i );
			}
			else
			{
				v() ->GetValue( v1, i );
				v() ->GetValue( v2, i - ( uRes + 1 ) );
				dv() ->SetFloat( v1 - v2, i );
			}
		}
	}

	if ( USES( lUses, EnvVars_Ci ) ) Ci() ->SetColor( gColBlack );
	if ( USES( lUses, EnvVars_Oi ) ) Oi() ->SetColor( gColWhite );

	// Setup varying variables.
	TqBool bdpu, bdpv;
	bdpu = ( USES( lUses, EnvVars_dPdu ) );
	bdpv = ( USES( lUses, EnvVars_dPdv ) );
	IqShaderData * pSDP = P();

	for ( i = gsmin1; i >= 0; i-- )
	{
		pI->SetVector( pP[ i ], i );

		if ( bdpu )
		{
			dPdu() ->SetVector( SO_DuType<CqVector3D>( pSDP, i, m_pShaderExecEnv, Defvec ), i );
		}
		if ( bdpv )
		{
			dPdv() ->SetVector( SO_DvType<CqVector3D>( pSDP, i, m_pShaderExecEnv, Defvec ), i );
		}
	}
	// Now try and cull any transparent MPs
	if ( USES( lUses, EnvVars_Os ) && QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "DisplayMode" ) [ 0 ] & ModeZ )
	{
		theStats.OcclusionCullTimer().Start();
		for ( i = gsmin1; i >= 0; i-- )
		{
			if ( pOs[ i ] != gColWhite )
			{
				cCulled ++;
				m_CulledPolys.SetValue( i, TqTrue );
			}
			else
				break;
		}
		theStats.OcclusionCullTimer().Stop();

		if ( cCulled == gs )
		{
			m_fCulled = TqTrue;
			theStats.IncCulledGrids();
			DeleteVariables( TqTrue );
			return ;
		}

	}


	// Now try and cull any true transparent MPs
	cCulled = 0;
	if ( USES( lUses, EnvVars_Os ) && QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "DisplayMode" ) [ 0 ] & ModeRGB )
	{
		theStats.OcclusionCullTimer().Start();
		for ( i = gsmin1; i >= 0; i-- )
		{
			if ( pOs[ i ] == gColBlack )
			{
				cCulled ++;
				m_CulledPolys.SetValue( i, TqTrue );
			}
			else
				break;
		}
		theStats.OcclusionCullTimer().Stop();

		if ( cCulled == gs )
		{
			m_fCulled = TqTrue;
			theStats.IncCulledGrids();
			DeleteVariables( TqTrue );
			return ;
		}
	}

	if ( pshadDisplacement != 0 )
	{
		theStats.DisplacementTimer().Start();
		pshadDisplacement->Evaluate( m_pShaderExecEnv );
		theStats.DisplacementTimer().Stop();
	}

	// Now try and cull any hidden MPs if Sides==1
	if ( pAttributes() ->GetIntegerAttribute( "System", "Sides" ) [ 0 ] == 1 && m_pCSGNode == NULL )
	{
		cCulled = 0;
		theStats.OcclusionCullTimer().Start();
		for ( i = gsmin1; i >= 0; i-- )
		{
			// Calulate the direction the MPG is facing.
			if ( ( pN[ i ] * pP[ i ] ) >= 0 )
			{
				cCulled++;
				m_CulledPolys.SetValue( i, TqTrue );
			} else break;
		}
		theStats.OcclusionCullTimer().Stop();

		// If the whole grid is culled don't bother going any further.
		if ( cCulled == gs )
		{
			m_fCulled = TqTrue;
			theStats.IncCulledGrids();
			DeleteVariables( TqTrue );
			return ;
		}

	}

	// Now shade the grid.
	theStats.SurfaceTimer().Start();
	if ( NULL != pshadSurface )
	{
		pshadSurface->Evaluate( m_pShaderExecEnv );
	}
	theStats.SurfaceTimer().Stop();

	// Now try and cull any true transparent MPs (assigned by the shader code

	cCulled = 0;
	if ( USES( lUses, EnvVars_Os ) && QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "DisplayMode" ) [ 0 ] & ModeRGB )
	{
		theStats.OcclusionCullTimer().Start();
		for ( i = gsmin1; i >= 0; i-- )
		{
			if ( pOs[ i ] == gColBlack )
			{
				cCulled ++;
				m_CulledPolys.SetValue( i, TqTrue );
			}
			else
				break;
		}
		theStats.OcclusionCullTimer().Stop();

		if ( cCulled == gs )
		{
			m_fCulled = TqTrue;
			theStats.IncCulledGrids();
			DeleteVariables( TqTrue );
			return ;
		}
	}
	// Now perform atmosphere shading
	if ( pshadAtmosphere != 0 )
	{
		theStats.AtmosphereTimer().Start();
		pshadAtmosphere->Evaluate( m_pShaderExecEnv );
		theStats.AtmosphereTimer().Stop();
	}

	DeleteVariables( TqFalse );
}

//---------------------------------------------------------------------
/** Transfer any shader variables marked as "otuput" as they may be needed by the display devices.
 */

void CqMicroPolyGrid::TransferOutputVariables()
{
	IqShader* pShader = this->pAttributes()->pshadSurface();
	
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
void CqMicroPolyGrid::DeleteVariables( TqBool all )
{
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "Cs" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Cs );
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "Os" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Os );
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "du" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_du );
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "dv" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_dv );
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "L" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_L );
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "Cl" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Cl );
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "Ol" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Ol );
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "dPdu" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_dPdu );
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "dPdv" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_dPdv );

	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "s" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_s );
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "t" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_t );
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "I" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_I );

	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "Ps" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Ps );
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "E" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_E );
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "ncomps" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_ncomps );
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "time" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_time );
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "alpha" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_alpha );

	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "N" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_N );
	if (  /*!QGetRenderContext() ->pDDmanager()->fDisplayNeeds( "u" ) ||*/ all ) 		// \note: Needed by trim curves, need to work out how to check for their existence.
		m_pShaderExecEnv->DeleteVariable( EnvVars_u );
	if (  /*!QGetRenderContext() ->pDDmanager()->fDisplayNeeds( "v" ) ||*/ all ) 		// \note: Needed by trim curves, need to work out how to check for their existence.
		m_pShaderExecEnv->DeleteVariable( EnvVars_v );
	if ( all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_P );
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "Ng" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Ng );
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "Ci" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Ci );
	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "Oi" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Oi );

	if ( !QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "Ns" ) || all )
		m_pShaderExecEnv->DeleteVariable( EnvVars_Ns );
}



//---------------------------------------------------------------------
/** Split the shaded grid into microploygons, and insert them into the relevant buckets in the image buffer.
 * \param pImage Pointer to image being rendered into.
 * \param iBucket Integer index of bucket being processed.
 * \param xmin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param xmax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 */

void CqMicroPolyGrid::Split( CqImageBuffer* pImage, TqInt iBucket, long xmin, long xmax, long ymin, long ymax )
{
	if ( NULL == P() )
		return ;

	QGetRenderContext() ->Stats().MakeProject().Start();
	CqMatrix matCameraToRaster = QGetRenderContext() ->matSpaceToSpace( "camera", "raster" );
	CqMatrix matCameraToObject0 = QGetRenderContext() ->matSpaceToSpace( "camera", "object", CqMatrix(), pSurface() ->pTransform() ->matObjectToWorld( pSurface() ->pTransform() ->Time( 0 ) ) );

	// Transform the whole grid to hybrid camera/raster space
	CqVector3D* pP;
	P() ->GetPointPtr( pP );

	// Get an array of P's for all time positions.
	std::vector<std::vector<CqVector3D> > aaPtimes;
	aaPtimes.resize( pSurface() ->pTransform() ->cTimes() );

	TqInt iTime, tTime = pSurface() ->pTransform() ->cTimes();
	CqMatrix matObjectToCameraT;
	register TqInt i;
	TqInt gsmin1;
	gsmin1 = GridSize() - 1;

	for ( iTime = 1; iTime < tTime; iTime++ )
	{
		matObjectToCameraT = QGetRenderContext() ->matSpaceToSpace( "object", "camera", CqMatrix(), pSurface() ->pTransform() ->matObjectToWorld( pSurface() ->pTransform() ->Time( iTime ) ) );
		aaPtimes[ iTime ].resize( gsmin1 + 1 );

		for ( i = gsmin1; i >= 0; i-- )
		{
			CqVector3D Point( pP[ i ] );

			// Only do the complex transform if motion blurred.
			//Point = matObjectToCameraT * matCameraToObject0 * Point;
			Point = matCameraToObject0 * Point;
			Point = matObjectToCameraT * Point;

			// Make sure to retain camera space 'z' coordinate.
			TqFloat zdepth = Point.z();
			aaPtimes[ iTime ][ i ] = matCameraToRaster * Point;
			aaPtimes[ iTime ][ i ].z( zdepth );
		}
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

	QGetRenderContext() ->Stats().MakeProject().Stop();


	TqInt cu = uGridRes();
	TqInt cv = vGridRes();

	// Get the required trim curve sense, if specified, defaults to "inside".
	const CqString* pattrTrimSense = pAttributes() ->GetStringAttribute( "trimcurve", "sense" );
	CqString strTrimSense( "inside" );
	if ( pattrTrimSense != 0 ) strTrimSense = pattrTrimSense[ 0 ];
	TqBool bOutside = strTrimSense == "outside";

	// Determine whether we need to bother with trimming or not.
	TqBool bCanBeTrimmed = pSurface() ->bCanBeTrimmed() && NULL != u() && NULL != v();

	ADDREF( this );

	TqInt iv;
	for ( iv = 0; iv < cv; iv++ )
	{
		TqInt iu;
		for ( iu = 0; iu < cu; iu++ )
		{
			TqInt iIndex = ( iv * ( cu + 1 ) ) + iu;

			// If culled don't bother.
			if ( m_CulledPolys.Value( iIndex ) )
			{
				//theStats.IncCulledMPGs();
				continue;
			}

			// If the MPG is trimmed then don't add it.
			TqBool fTrimmed = TqFalse;
			if ( bCanBeTrimmed )
			{
				TqFloat fu, fv;
				u() ->GetFloat( fu, iIndex );
				v() ->GetFloat( fv, iIndex );
				TqBool fTrimA = pSurface() ->bIsPointTrimmed( CqVector2D( fu, fv ) );
				u() ->GetFloat( fu, iIndex + 1 );
				v() ->GetFloat( fv, iIndex + 1 );
				TqBool fTrimB = pSurface() ->bIsPointTrimmed( CqVector2D( fu, fv ) );
				u() ->GetFloat( fu, iIndex + cu + 2 );
				v() ->GetFloat( fv, iIndex + cu + 2 );
				TqBool fTrimC = pSurface() ->bIsPointTrimmed( CqVector2D( fu, fv ) );
				u() ->GetFloat( fu, iIndex + cu + 1 );
				v() ->GetFloat( fv, iIndex + cu + 1 );
				TqBool fTrimD = pSurface() ->bIsPointTrimmed( CqVector2D( fu, fv ) );

				if ( bOutside )
				{
					fTrimA = !fTrimA;
					fTrimB = !fTrimB;
					fTrimC = !fTrimC;
					fTrimD = !fTrimD;
				}

				// If al points are trimmed discard the MPG
				if ( fTrimA && fTrimB && fTrimC && fTrimD )
					continue;

				// If any points are trimmed mark the MPG as needing to be trim checked.
				//fTrimmed = fTrimA || fTrimB || fTrimC || fTrimD;
				if ( fTrimA || fTrimB || fTrimC || fTrimD )
					fTrimmed = TqTrue;
			}

			if ( tTime > 1 )
			{
				CqMicroPolygonMotion * pNew = new CqMicroPolygonMotion();
				pNew->SetGrid( this );
				pNew->SetIndex( iIndex );
				if ( fTrimmed ) pNew->MarkTrimmed();
				for ( iTime = 0; iTime < tTime; iTime++ )
					pNew->AppendKey( aaPtimes[ iTime ][ iIndex ], aaPtimes[ iTime ][ iIndex + 1 ], aaPtimes[ iTime ][ iIndex + cu + 2 ], aaPtimes[ iTime ][ iIndex + cu + 1 ], pSurface() ->pTransform() ->Time( iTime ) );
				pImage->AddMPG( pNew );
			}
			else
			{
				CqMicroPolygon *pNew = new CqMicroPolygon();
				pNew->SetGrid( this );
				pNew->SetIndex( iIndex );
				if ( fTrimmed ) pNew->MarkTrimmed();
				pNew->Initialise();
				pNew->GetTotalBound( TqTrue );
				pImage->AddMPG( pNew );
			}
		}
	}

	RELEASEREF( this );
}


//---------------------------------------------------------------------
/** Shade the primary grid.
 */

void CqMotionMicroPolyGrid::Shade()
{
	CqMicroPolyGrid * pGrid = static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( 0 ) ) );
	pGrid->Shade();
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
 * \param pImage Pointer to image being rendered into.
 * \param iBucket Integer index of bucket being processed.
 * \param xmin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param xmax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 */

void CqMotionMicroPolyGrid::Split( CqImageBuffer* pImage, TqInt iBucket, long xmin, long xmax, long ymin, long ymax )
{
	// Get the main object, the one that was shaded.
	CqMicroPolyGrid * pGridA = static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( 0 ) ) );
	TqInt cu = pGridA->uGridRes();
	TqInt cv = pGridA->vGridRes();
	TqInt iTime;

	CqMatrix matCameraToRaster = QGetRenderContext() ->matSpaceToSpace( "camera", "raster" );

	ADDREF( pGridA );

	// Get an array of P's for all time positions.
	std::vector<std::vector<CqVector3D> > aaPtimes;
	aaPtimes.resize( cTimes() );

	TqInt tTime = cTimes();
	CqMatrix matObjectToCameraT;
	register TqInt i;
	TqInt gsmin1;
	gsmin1 = pGridA->GridSize() - 1;

	for ( iTime = 0; iTime < tTime; iTime++ )
	{
		matObjectToCameraT = QGetRenderContext() ->matSpaceToSpace( "object", "camera", CqMatrix(), pSurface() ->pTransform() ->matObjectToWorld( pSurface() ->pTransform() ->Time( iTime ) ) );
		aaPtimes[ iTime ].resize( gsmin1 + 1 );

		// Transform the whole grid to hybrid camera/raster space
		CqMicroPolyGrid* pg = static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( iTime ) ) );
		CqVector3D* pP;
		pg->P() ->GetPointPtr( pP );

		for ( i = gsmin1; i >= 0; i-- )
		{
			CqVector3D Point( pP[ i ] );

			// Make sure to retain camera space 'z' coordinate.
			TqFloat zdepth = Point.z();
			aaPtimes[ iTime ][ i ] = matCameraToRaster * Point;
			aaPtimes[ iTime ][ i ].z( zdepth );
			pP[ i ] = aaPtimes[ iTime ][ i ];
		}
	}

	// Get the required trim curve sense, if specified, defaults to "inside".
	const CqString* pattrTrimSense = pAttributes() ->GetStringAttribute( "trimcurve", "sense" );
	CqString strTrimSense( "inside" );
	if ( pattrTrimSense != 0 ) strTrimSense = pattrTrimSense[ 0 ];
	TqBool bOutside = strTrimSense == "outside";

	// Determine whether we need to bother with trimming or not.
	TqBool bCanBeTrimmed = pSurface() ->bCanBeTrimmed() && NULL != pGridA->u() && NULL != pGridA->v();

	TqInt iv;
	for ( iv = 0; iv < cv; iv++ )
	{
		TqInt iu;
		for ( iu = 0; iu < cu; iu++ )
		{
			TqInt iIndex = ( iv * ( cu + 1 ) ) + iu;

			// If culled don't bother.
			if ( pGridA->CulledPolys().Value( iIndex ) )
			{
				//theStats.IncCulledMPGs();
				continue;
			}

			// If the MPG is trimmed then don't add it.
			TqBool fTrimmed = TqFalse;
			if ( bCanBeTrimmed )
			{
				TqFloat fu, fv;
				pGridA->u() ->GetFloat( fu, iIndex );
				pGridA->v() ->GetFloat( fv, iIndex );
				TqBool fTrimA = pSurface() ->bIsPointTrimmed( CqVector2D( fu, fv ) );
				pGridA->u() ->GetFloat( fu, iIndex + 1 );
				pGridA->v() ->GetFloat( fv, iIndex + 1 );
				TqBool fTrimB = pSurface() ->bIsPointTrimmed( CqVector2D( fu, fv ) );
				pGridA->u() ->GetFloat( fu, iIndex + cu + 2 );
				pGridA->v() ->GetFloat( fv, iIndex + cu + 2 );
				TqBool fTrimC = pSurface() ->bIsPointTrimmed( CqVector2D( fu, fv ) );
				pGridA->u() ->GetFloat( fu, iIndex + cu + 1 );
				pGridA->v() ->GetFloat( fv, iIndex + cu + 1 );
				TqBool fTrimD = pSurface() ->bIsPointTrimmed( CqVector2D( fu, fv ) );

				if ( bOutside )
				{
					fTrimA = !fTrimA;
					fTrimB = !fTrimB;
					fTrimC = !fTrimC;
					fTrimD = !fTrimD;
				}

				// If al points are trimmed discard the MPG
				if ( fTrimA && fTrimB && fTrimC && fTrimD )
					continue;

				// If any points are trimmed mark the MPG as needing to be trim checked.
				//fTrimmed = fTrimA || fTrimB || fTrimC || fTrimD;
				if ( fTrimA || fTrimB || fTrimC || fTrimD )
					fTrimmed = TqTrue;
			}

			CqMicroPolygonMotion *pNew = new CqMicroPolygonMotion();
			pNew->SetGrid( pGridA );
			pNew->SetIndex( iIndex );
			for ( iTime = 0; iTime < cTimes(); iTime++ )
				pNew->AppendKey( aaPtimes[ iTime ][ iIndex ], aaPtimes[ iTime ][ iIndex + 1 ], aaPtimes[ iTime ][ iIndex + cu + 2 ], aaPtimes[ iTime ][ iIndex + cu + 1 ], Time( iTime ) );
			pImage->AddMPG( pNew );
		}
	}

	RELEASEREF( pGridA );

	// Delete the donor motion grids, as their work is done.
	for ( iTime = 1; iTime < cTimes(); iTime++ )
	{
		CqMicroPolyGrid* pg = static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( iTime ) ) );
		if ( NULL != pg )
			RELEASEREF( pg );
	}
	//		delete( GetMotionObject( Time( iTime ) ) );
}


//---------------------------------------------------------------------
/** Default constructor
 */

CqMicroPolygon::CqMicroPolygon() : m_pGrid( 0 ), m_Flags( 0 )
{
	QGetRenderContext() ->Stats().IncMPGsAllocated();
}


//---------------------------------------------------------------------
/** Destructor
 */

CqMicroPolygon::~CqMicroPolygon()
{
	if ( m_pGrid ) RELEASEREF( m_pGrid );
	QGetRenderContext() ->Stats().IncMPGsDeallocated();
	if ( IsHit() )
		QGetRenderContext() ->Stats().IncMissedMPGs();
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
	m_pGrid->P() ->GetPointPtr( pP );
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
}


//---------------------------------------------------------------------
/** Determinde whether the 2D point specified lies within this micropolygon.
 * \param vecP 2D vector to test for containment.
 * \param Depth Place to put the depth if valid intersection.
 * \param time The frame time at which to check containment.
 * \return Boolean indicating sample hit.
 */

TqBool CqMicroPolygon::fContains( const CqVector2D& vecP, TqFloat& Depth, TqFloat time ) const
{
	// Check against each line of the quad, if outside any then point is outside MPG, therefore early exit.
	TqFloat r1, r2, r3, r4;
	TqFloat x = vecP.x(), y = vecP.y();
	TqFloat x0 = PointA().x(), y0 = PointA().y(), x1 = PointB().x(), y1 = PointB().y();
	if ( ( r1 = ( y - y0 ) * ( x1 - x0 ) - ( x - x0 ) * ( y1 - y0 ) ) <= 0 ) return ( TqFalse );
	x0 = x1; y0 = y1; x1 = PointC().x(); y1 = PointC().y();
	if ( ( r2 = ( y - y0 ) * ( x1 - x0 ) - ( x - x0 ) * ( y1 - y0 ) ) <= 0 ) return ( TqFalse );

	// Check for degeneracy.
	if ( !( IsDegenerate() ) )
	{
		x0 = x1; y0 = y1; x1 = PointD().x(); y1 = PointD().y();
		if ( ( r3 = ( y - y0 ) * ( x1 - x0 ) - ( x - x0 ) * ( y1 - y0 ) ) < 0 ) return ( TqFalse );
		x0 = x1; y0 = y1; x1 = PointA().x(); y1 = PointA().y();
		if ( ( r4 = ( y - y0 ) * ( x1 - x0 ) - ( x - x0 ) * ( y1 - y0 ) ) < 0 ) return ( TqFalse );
	}
	else
	{
		x0 = x1; y0 = y1; x1 = PointA().x(); y1 = PointA().y();
		if ( ( r3 = ( y - y0 ) * ( x1 - x0 ) - ( x - x0 ) * ( y1 - y0 ) ) < 0 ) return ( TqFalse );
	}

	CqVector3D vecN = ( PointA() - PointB() ) % ( PointC() - PointB() );
	vecN.Unit();
	TqFloat D = vecN * PointA();

	Depth = ( D - ( vecN.x() * vecP.x() ) - ( vecN.y() * vecP.y() ) ) / vecN.z();

	return ( TqTrue );
}


CqVector2D CqMicroPolygon::ReverseBilinear( const CqVector2D& v )
{
	CqVector2D kA, kB, kC, kD;

	kA = CqVector2D( PointA() );
	kB = CqVector2D( PointB() ) - kA;
	kC = CqVector2D( PointD() ) - kA;
	kD = CqVector2D( PointC() ) + kA - CqVector2D( PointB() ) - CqVector2D( PointD() );

	TqFloat fBCdet = kB.x() * kC.y() - kB.y() * kC.x();
	TqFloat fCDdet = kC.y() * kD.x() - kC.x() * kD.y();

	CqVector2D kDiff = kA - v;
	TqFloat fABdet = kDiff.y() * kB.x() - kDiff.x() * kB.y();
	TqFloat fADdet = kDiff.y() * kD.x() - kDiff.x() * kD.y();
	TqFloat fA = fCDdet;
	TqFloat fB = fADdet + fBCdet;
	TqFloat fC = fABdet;
	CqVector2D kResult;

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

	return ( kResult );
}



//---------------------------------------------------------------------
/** Sample the specified point against the MPG at the specified time.
 * \param vecSample 2D vector to sample against.
 * \param time Shutter time to sample at.
 * \param D Storage for depth if valid hit.
 * \return Boolean indicating smaple hit.
 */

TqBool CqMicroPolygon::Sample( const CqVector2D& vecSample, TqFloat& D, TqFloat time )
{
	if ( fContains( vecSample, D ) )
	{
		// Now check if it is trimmed.
		if ( IsTrimmed() )
		{
			// Get the required trim curve sense, if specified, defaults to "inside".
			const CqString * pattrTrimSense = pGrid() ->pAttributes() ->GetStringAttribute( "trimcurve", "sense" );
			CqString strTrimSense( "inside" );
			if ( pattrTrimSense != 0 ) strTrimSense = pattrTrimSense[ 0 ];
			TqBool bOutside = strTrimSense == "outside";

			CqVector2D vecUV = ReverseBilinear( vecSample );

			TqFloat u, v;

			pGrid() ->u() ->GetFloat( u, m_Index );
			pGrid() ->v() ->GetFloat( v, m_Index );
			CqVector2D uvA( u, v );

			pGrid() ->u() ->GetFloat( u, m_Index + 1 );
			pGrid() ->v() ->GetFloat( v, m_Index + 1 );
			CqVector2D uvB( u, v );

			pGrid() ->u() ->GetFloat( u, m_Index + pGrid() ->uGridRes() + 1 );
			pGrid() ->v() ->GetFloat( v, m_Index + pGrid() ->uGridRes() + 1 );
			CqVector2D uvC( u, v );

			pGrid() ->u() ->GetFloat( u, m_Index + pGrid() ->uGridRes() + 2 );
			pGrid() ->v() ->GetFloat( v, m_Index + pGrid() ->uGridRes() + 2 );
			CqVector2D uvD( u, v );

			CqVector2D vR = BilinearEvaluate( uvA, uvB, uvC, uvD, vecUV.x(), vecUV.y() );

			if ( pGrid() ->pSurface() ->bCanBeTrimmed() && pGrid() ->pSurface() ->bIsPointTrimmed( vR ) && !bOutside )
				return ( TqFalse );
		}

		if ( pGrid() ->fTriangular() )
		{
			CqVector3D vO;
			pGrid() ->P() ->GetPoint( vO, 0 );
			CqVector3D vA;
			pGrid() ->P() ->GetPoint( vA, pGrid() ->uGridRes() );
			CqVector3D vB;
			pGrid() ->P() ->GetPoint( vB, pGrid() ->vGridRes() * ( pGrid() ->uGridRes() + 1 ) );

			TqBool clockwise;
			CqVector3D E1 = vA - vO;
			CqVector3D E2 = vB - vO;
			if ( ( E1.x() * E2.y() - E1.y() * E2.x() ) >= 0 ) clockwise = TqTrue;
			else	clockwise = TqFalse;


			TqFloat Ax = vA.x();
			TqFloat Ay = vA.y();
			TqFloat Bx = vB.x();
			TqFloat By = vB.y();

			TqFloat v = ( Ay - By ) * vecSample.x() + ( Bx - Ax ) * vecSample.y() + ( Ax * By - Bx * Ay );
			if ( ( ( clockwise ) && ( v <= 0 ) ) || ( ( !clockwise ) && ( v >= 0 ) ) )
				return ( TqFalse );

		}

		return ( TqTrue );
	}
	else
		return ( TqFalse );
}

//---------------------------------------------------------------------
/** Calculate the 2D boundary of this micropolygon,
 */

CqBound CqMicroPolygon::GetTotalBound( TqBool fForce )
{
	CqVector3D * pP;
	m_pGrid->P() ->GetPointPtr( pP );
	if ( fForce )
	{
		// Calculate the boundary, and store the indexes in the cache.
            	const CqVector3D& B = pP[ m_Index + 1 ];
		const CqVector3D& C = pP[ m_Index + m_pGrid->uGridRes() + 2 ];
		const CqVector3D& D = pP[ m_Index + m_pGrid->uGridRes() + 1 ];

		TqShort BCMinX = 0;
		TqShort BCMaxX = 0;
		TqShort BCMinY = 0;
		TqShort BCMaxY = 0;
		TqShort BCMinZ = 0;
		TqShort BCMaxZ = 0;
		m_BoundCode = 0xe4;
		TqInt TempIndexTable[ 4 ] = {	GetCodedIndex( m_BoundCode, 0 ),
		                                GetCodedIndex( m_BoundCode, 1 ),
		                                GetCodedIndex( m_BoundCode, 2 ),
		                                GetCodedIndex( m_BoundCode, 3 ) };
		if ( B.x() < pP[ TempIndexTable[ BCMinX ] ].x() ) BCMinX = 1;
		if ( B.x() > pP[ TempIndexTable[ BCMaxX ] ].x() ) BCMaxX = 1;
		if ( B.y() < pP[ TempIndexTable[ BCMinY ] ].y() ) BCMinY = 1;
		if ( B.y() > pP[ TempIndexTable[ BCMaxY ] ].y() ) BCMaxY = 1;
		if ( B.z() < pP[ TempIndexTable[ BCMinZ ] ].z() ) BCMinZ = 1;
		if ( B.z() > pP[ TempIndexTable[ BCMaxZ ] ].z() ) BCMaxZ = 1;

		if ( C.x() < pP[ TempIndexTable[ BCMinX ] ].x() ) BCMinX = 2;
		if ( C.x() > pP[ TempIndexTable[ BCMaxX ] ].x() ) BCMaxX = 2;
		if ( C.y() < pP[ TempIndexTable[ BCMinY ] ].y() ) BCMinY = 2;
		if ( C.y() > pP[ TempIndexTable[ BCMaxY ] ].y() ) BCMaxY = 2;
		if ( C.z() < pP[ TempIndexTable[ BCMinZ ] ].z() ) BCMinZ = 2;
		if ( C.z() > pP[ TempIndexTable[ BCMaxZ ] ].z() ) BCMaxZ = 2;

		if ( !IsDegenerate() )
		{
			if ( D.x() < pP[ TempIndexTable[ BCMinX ] ].x() ) BCMinX = 3;
			if ( D.x() > pP[ TempIndexTable[ BCMaxX ] ].x() ) BCMaxX = 3;
			if ( D.y() < pP[ TempIndexTable[ BCMinY ] ].y() ) BCMinY = 3;
			if ( D.y() > pP[ TempIndexTable[ BCMaxY ] ].y() ) BCMaxY = 3;
			if ( D.z() < pP[ TempIndexTable[ BCMinZ ] ].z() ) BCMinZ = 3;
			if ( D.z() > pP[ TempIndexTable[ BCMaxZ ] ].z() ) BCMaxZ = 3;
		}
		m_BoundCode = ( ( BCMinX & 0x3 ) |
		              ( ( BCMinY & 0x3 ) << 2 ) |
		              ( ( BCMinZ & 0x3 ) << 4 ) |
		              ( ( BCMaxX & 0x3 ) << 6 ) |
		              ( ( BCMaxY & 0x3 ) << 8 ) |
		              ( ( BCMaxZ & 0x3 ) << 10 ) );
	}
	CqBound B( pP[ GetCodedIndex( m_BoundCode, 0 ) ].x(), pP[ GetCodedIndex( m_BoundCode, 1 ) ].y(), pP[ GetCodedIndex( m_BoundCode, 2 ) ].z(),
	           pP[ GetCodedIndex( m_BoundCode, 3 ) ].x(), pP[ GetCodedIndex( m_BoundCode, 4 ) ].y(), pP[ GetCodedIndex( m_BoundCode, 5 ) ].z() );

	// Adjust for DOF
	if ( QGetRenderContext() ->UsingDepthOfField() )
	{
		const TqFloat * dofdata = QGetRenderContext() ->GetDepthOfFieldData();

		TqFloat C = MAX( CircleOfConfusion( dofdata, B.vecMin().z() ), CircleOfConfusion( dofdata, B.vecMax().z() ) );

		TqFloat sx = QGetRenderContext() ->GetDepthOfFieldScaleX();
		TqFloat sy = QGetRenderContext() ->GetDepthOfFieldScaleY();
		

		B.vecMin().x( B.vecMin().x() - C * sx );
		B.vecMin().y( B.vecMin().y() - C * sy );
		B.vecMax().x( B.vecMax().x() + C * sx );
		B.vecMax().y( B.vecMax().y() + C * sy );
	}

	return ( B );
}


//---------------------------------------------------------------------
/** Calculate the 2D boundary of this micropolygon,
 * \param fForce Boolean flag to force the recalculation of the cached bound.
 */

CqBound CqMicroPolygonMotion::GetTotalBound( TqBool fForce )
{
	if ( fForce )
	{
		assert( NULL != m_Keys[ 0 ] );

		m_Bound = m_Keys[ 0 ] ->GetTotalBound();
		std::vector<CqMovingMicroPolygonKey*>::iterator i;
		for ( i = m_Keys.begin(); i != m_Keys.end(); i++ )
			m_Bound.Encapsulate( ( *i ) ->GetTotalBound() );
	}
	return ( m_Bound );
}

//---------------------------------------------------------------------
/** Calculate a list of 2D bounds for this micropolygon,
 */
void CqMicroPolygonMotion::BuildBoundList()
{
	m_BoundList.Clear();
	
	assert( NULL != m_Keys[ 0 ] );

	CqBound start = m_Keys[ 0 ] ->GetTotalBound();
	TqFloat	startTime = m_Times[ 0 ];
	TqInt cTimes = m_Keys.size();
	for ( TqInt i = 1; i < cTimes; i++ )
	{
		CqBound end = m_Keys[ i ] ->GetTotalBound();
		CqBound mid0( start );
		CqBound mid1;
		TqFloat endTime = m_Times[ i ];
		TqFloat time = startTime;

		TqInt d;
		// arbitary number of divisions, should be related to screen size of blur
		TqInt divisions = 4;
		TqFloat delta = 1.0f / static_cast<TqFloat>( divisions );
		m_BoundList.SetSize( divisions );
		for ( d = 1; d <= divisions; d++ )
		{
			mid1.vecMin() = delta * ( end.vecMin() - start.vecMin() ) + start.vecMin();
			mid1.vecMax() = delta * ( end.vecMax() - start.vecMax() ) + start.vecMax();
			m_BoundList.Set( d - 1, mid0.Combine( mid1 ), time );
			time = delta * ( endTime - startTime ) + startTime;
			mid0 = mid1;
			delta += delta;
		}
		start = end;
		startTime = endTime;
	}
	m_BoundReady = TqTrue;
}


//---------------------------------------------------------------------
/** Sample the specified point against the MPG at the specified time.
 * \param vecSample 2D vector to sample against.
 * \param time Shutter time to sample at.
 * \param D Storage for depth if valid hit.
 * \return Boolean indicating smaple hit.
 */

TqBool CqMicroPolygonMotion::Sample( const CqVector2D& vecSample, TqFloat& D, TqFloat time )
{
	if ( fContains( vecSample, D, time ) )
	{
		// Now check if it is trimmed.
		if ( IsTrimmed() )
		{
			// Get the required trim curve sense, if specified, defaults to "inside".

			/// \todo: Implement trimming of motion blurred surfaces!
			/*			const CqString * pattrTrimSense = pGrid() ->pAttributes() ->GetStringAttribute( "trimcurve", "sense" );
						CqString strTrimSense( "inside" );
						if ( pattrTrimSense != 0 ) strTrimSense = pattrTrimSense[ 0 ];
						TqBool bOutside = strTrimSense == "outside";
			 
						CqVector2D vecUV = ReverseBilinear( vecSample );
			 
						TqFloat u, v;
			 
						pGrid() ->u() ->GetFloat( u, m_Index );
						pGrid() ->v() ->GetFloat( v, m_Index );
						CqVector2D uvA( u, v );
			 
						pGrid() ->u() ->GetFloat( u, m_Index + 1 );
						pGrid() ->v() ->GetFloat( v, m_Index + 1 );
						CqVector2D uvB( u, v );
			 
						pGrid() ->u() ->GetFloat( u, m_Index + pGrid() ->uGridRes() + 1 );
						pGrid() ->v() ->GetFloat( v, m_Index + pGrid() ->uGridRes() + 1 );
						CqVector2D uvC( u, v );
			 
						pGrid() ->u() ->GetFloat( u, m_Index + pGrid() ->uGridRes() + 2 );
						pGrid() ->v() ->GetFloat( v, m_Index + pGrid() ->uGridRes() + 2 );
						CqVector2D uvD( u, v );
			 
						CqVector2D vR = BilinearEvaluate( uvA, uvB, uvC, uvD, vecUV.x(), vecUV.y() );
			 
						if ( pGrid() ->pSurface() ->bCanBeTrimmed() && pGrid() ->pSurface() ->bIsPointTrimmed( vR ) && !bOutside )
							return ( TqFalse );*/
		}

		if ( pGrid() ->fTriangular() )
		{
			CqVector3D vO;
			pGrid() ->P() ->GetPoint( vO, 0 );
			CqVector3D vA;
			pGrid() ->P() ->GetPoint( vA, pGrid() ->uGridRes() );
			CqVector3D vB;
			pGrid() ->P() ->GetPoint( vB, pGrid() ->vGridRes() * ( pGrid() ->uGridRes() + 1 ) );

			TqBool clockwise;
			CqVector3D E1 = vA - vO;
			CqVector3D E2 = vB - vO;
			if ( ( E1.x() * E2.y() - E1.y() * E2.x() ) >= 0 ) clockwise = TqTrue;
			else	clockwise = TqFalse;


			TqFloat Ax = vA.x();
			TqFloat Ay = vA.y();
			TqFloat Bx = vB.x();
			TqFloat By = vB.y();

			TqFloat v = ( Ay - By ) * vecSample.x() + ( Bx - Ax ) * vecSample.y() + ( Ax * By - Bx * Ay );
			if ( ( ( clockwise ) && ( v <= 0 ) ) || ( ( !clockwise ) && ( v >= 0 ) ) )
				return ( TqFalse );

		}

		return ( TqTrue );
	}
	else
		return ( TqFalse );
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
		m_Bound = pMP->GetTotalBound();
	else
		m_Bound.Encapsulate( pMP->GetTotalBound() );
}


//---------------------------------------------------------------------
/** Determinde whether the 2D point specified lies within this micropolygon.
 * \param vecP 2D vector to test for containment.
 * \param Depth Place to put the depth if valid intersection.
 * \param time The frame time at which to check containment.
 * \return Boolean indicating sample hit.
 */

TqBool CqMicroPolygonMotion::fContains( const CqVector2D& vecP, TqFloat& Depth, TqFloat time ) const
{
	TqInt iIndex = 0;
	TqFloat Fraction = 0.0f;
	TqBool Exact = TqTrue;

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

	if ( Exact )
	{
		CqMovingMicroPolygonKey * pMP1 = m_Keys[ iIndex ];
		return ( pMP1->fContains( vecP, Depth, time ) );
	}
	else
	{
		TqFloat F1 = 1.0f - Fraction;
		CqMovingMicroPolygonKey* pMP1 = m_Keys[ iIndex ];
		CqMovingMicroPolygonKey* pMP2 = m_Keys[ iIndex + 1 ];
		// Check against each line of the quad, if outside any then point is outside MPG, therefore early exit.
		TqFloat r1, r2, r3, r4;
		TqFloat x = vecP.x(), y = vecP.y();
		TqFloat x0 = ( F1 * pMP1->m_Point0.x() ) + ( Fraction * pMP2->m_Point0.x() ),
		             y0 = ( F1 * pMP1->m_Point0.y() ) + ( Fraction * pMP2->m_Point0.y() ),
		                  x1 = ( F1 * pMP1->m_Point1.x() ) + ( Fraction * pMP2->m_Point1.x() ),
		                       y1 = ( F1 * pMP1->m_Point1.y() ) + ( Fraction * pMP2->m_Point1.y() );
		TqFloat x0_hold = x0;
		TqFloat y0_hold = y0;
		if ( ( r1 = ( y - y0 ) * ( x1 - x0 ) - ( x - x0 ) * ( y1 - y0 ) ) <= 0 ) return ( TqFalse );
		x0 = x1;
		y0 = y1;
		x1 = ( F1 * pMP1->m_Point2.x() ) + ( Fraction * pMP2->m_Point2.x() );
		y1 = ( F1 * pMP1->m_Point2.y() ) + ( Fraction * pMP2->m_Point2.y() );
		if ( ( r2 = ( y - y0 ) * ( x1 - x0 ) - ( x - x0 ) * ( y1 - y0 ) ) <= 0 ) return ( TqFalse );
		x0 = x1;
		y0 = y1;
		x1 = ( F1 * pMP1->m_Point3.x() ) + ( Fraction * pMP2->m_Point3.x() );
		y1 = ( F1 * pMP1->m_Point3.y() ) + ( Fraction * pMP2->m_Point3.y() );
		if ( ( r3 = ( y - y0 ) * ( x1 - x0 ) - ( x - x0 ) * ( y1 - y0 ) ) < 0 ) return ( TqFalse );

		// Check for degeneracy.
		if ( ! ( x1 == x0_hold && y1 == y0_hold ) )
			if ( ( r4 = ( y - y1 ) * ( x0_hold - x1 ) - ( x - x1 ) * ( y0_hold - y1 ) ) < 0 ) return ( TqFalse );

		CqVector3D vecN = ( F1 * pMP1->m_N ) + ( Fraction * pMP2->m_N );
		TqFloat D = ( F1 * pMP1->m_D ) + ( Fraction * pMP2->m_D );
		Depth = ( D - ( vecN.x() * vecP.x() ) - ( vecN.y() * vecP.y() ) ) / vecN.z();

		return ( TqTrue );
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
}


//---------------------------------------------------------------------
/** Determinde whether the 2D point specified lies within this micropolygon.
 * \param vecP 2D vector to test for containment.
 * \param Depth Place to put the depth if valid intersection.
 * \param time The frame time at which to check containment.
 * \return Boolean indicating sample hit.
 */

TqBool CqMovingMicroPolygonKey::fContains( const CqVector2D& vecP, TqFloat& Depth, TqFloat time ) const
{
	// Check against each line of the quad, if outside any then point is outside MPG, therefore early exit.
	TqFloat r1, r2, r3, r4;
	TqFloat x = vecP.x(), y = vecP.y();
	TqFloat x0 = m_Point0.x(), y0 = m_Point0.y(), x1 = m_Point1.x(), y1 = m_Point1.y();
	if ( ( r1 = ( y - y0 ) * ( x1 - x0 ) - ( x - x0 ) * ( y1 - y0 ) ) <= 0 ) return ( TqFalse );
	x0 = x1; y0 = y1; x1 = m_Point2.x(); y1 = m_Point2.y();
	if ( ( r2 = ( y - y0 ) * ( x1 - x0 ) - ( x - x0 ) * ( y1 - y0 ) ) <= 0 ) return ( TqFalse );
	x0 = x1; y0 = y1; x1 = m_Point3.x(); y1 = m_Point3.y();
	if ( ( r3 = ( y - y0 ) * ( x1 - x0 ) - ( x - x0 ) * ( y1 - y0 ) ) < 0 ) return ( TqFalse );
	x0 = x1; y0 = y1; x1 = m_Point0.x(); y1 = m_Point0.y();

	// Check for degeneracy.
	if ( ! ( x0 == x1 && y0 == y1 ) )
		if ( ( r4 = ( y - y0 ) * ( x1 - x0 ) - ( x - x0 ) * ( y1 - y0 ) ) < 0 ) return ( TqFalse );

	Depth = ( m_D - ( m_N.x() * vecP.x() ) - ( m_N.y() * vecP.y() ) ) / m_N.z();

	return ( TqTrue );
}


CqVector2D CqMovingMicroPolygonKey::ReverseBilinear( const CqVector2D& v )
{
	CqVector2D kA, kB, kC, kD;

	kA = CqVector2D( m_Point0 );
	kB = CqVector2D( m_Point1 ) - kA;
	kC = CqVector2D( m_Point3 ) - kA;
	kD = CqVector2D( m_Point2 ) + kA - CqVector2D( m_Point1 ) - CqVector2D( m_Point3 );

	TqFloat fBCdet = kB.x() * kC.y() - kB.y() * kC.x();
	TqFloat fCDdet = kC.y() * kD.x() - kC.x() * kD.y();

	CqVector2D kDiff = kA - v;
	TqFloat fABdet = kDiff.y() * kB.x() - kDiff.x() * kB.y();
	TqFloat fADdet = kDiff.y() * kD.x() - kDiff.x() * kD.y();
	TqFloat fA = fCDdet;
	TqFloat fB = fADdet + fBCdet;
	TqFloat fC = fABdet;
	CqVector2D kResult;

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

	return ( kResult );
}


//---------------------------------------------------------------------
/** Calculate the 2D boundary of this micropolygon,
 */

CqBound CqMovingMicroPolygonKey::GetTotalBound() const
{
	CqBound Bound;
	// Calculate the boundary, and store the indexes in the cache.
	Bound.vecMin().x( MIN( m_Point0.x(), MIN( m_Point1.x(), MIN( m_Point2.x(), m_Point3.x() ) ) ) );
	Bound.vecMin().y( MIN( m_Point0.y(), MIN( m_Point1.y(), MIN( m_Point2.y(), m_Point3.y() ) ) ) );
	Bound.vecMin().z( MIN( m_Point0.z(), MIN( m_Point1.z(), MIN( m_Point2.z(), m_Point3.z() ) ) ) );
	Bound.vecMax().x( MAX( m_Point0.x(), MAX( m_Point1.x(), MAX( m_Point2.x(), m_Point3.x() ) ) ) );
	Bound.vecMax().y( MAX( m_Point0.y(), MAX( m_Point1.y(), MAX( m_Point2.y(), m_Point3.y() ) ) ) );
	Bound.vecMax().z( MAX( m_Point0.z(), MAX( m_Point1.z(), MAX( m_Point2.z(), m_Point3.z() ) ) ) );

	return ( Bound );
}



END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
