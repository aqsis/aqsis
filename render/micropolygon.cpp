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

START_NAMESPACE( Aqsis )


DEFINE_STATIC_MEMORYPOOL( CqMicroPolygonStatic );

//---------------------------------------------------------------------
/** Default constructor
 */

CqMicroPolyGrid::CqMicroPolyGrid() : CqMicroPolyGridBase(), 
									 m_bShadingNormals( TqFalse ), 
									 m_bGeometricNormals( TqFalse ), 
									 m_pSurface( NULL ), 
									 m_pCSGNode( NULL ), 
									 m_pShaderExecEnv( NULL ),
									 m_vfCulled( TqFalse )
{
	QGetRenderContext() ->Stats().IncGridsAllocated();
}


//---------------------------------------------------------------------
/** Destructor
 */

CqMicroPolyGrid::~CqMicroPolyGrid()
{
	QGetRenderContext() ->Stats().IncGridsDeallocated();

	// Release the reference to the attributes.
	if ( m_pSurface != 0 ) m_pSurface->Release();
	m_pSurface = 0;

	// Release the reference to the CSG node.
	if ( m_pCSGNode != 0 ) m_pCSGNode->Release();
	m_pCSGNode = 0;

	// Delete.
	/// \note This should delete throught the interface that created it.
	if ( m_pShaderExecEnv != 0 ) delete( m_pShaderExecEnv );
	m_pShaderExecEnv = 0;
}

//---------------------------------------------------------------------
/** Constructor
 */

CqMicroPolyGrid::CqMicroPolyGrid( TqInt cu, TqInt cv, CqSurface* pSurface ) : 
									m_bShadingNormals( TqFalse ), 
									m_bGeometricNormals( TqFalse ), 
									m_pSurface( NULL ),
									m_pShaderExecEnv( NULL ),
								    m_vfCulled( TqFalse )
{
	QGetRenderContext() ->Stats().IncGridsAllocated();
	// Initialise the shader execution environment
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
		m_pSurface->AddRef();

		m_pCSGNode = pSurface->pCSGNode();
		if ( m_pCSGNode ) m_pCSGNode->AddRef();
	}

	/// \note This should delete through the interface that created it.
	if( NULL != m_pShaderExecEnv )	
		delete( m_pShaderExecEnv );

	m_pShaderExecEnv = new CqShaderExecEnv;
	m_pShaderExecEnv->Initialise( cu, cv, pSurface, pSurface->pAttributes()->pshadSurface(), lUses );

	// Initialise the local/public culled variable.
	m_vfCulled = TqFalse;
}


//---------------------------------------------------------------------
/** Build the normals list for the micorpolygons in the grid.
 */

void CqMicroPolyGrid::CalcNormals()
{
	if( NULL == P() || NULL == N() )	return;
	
	// Get the handedness of the coordinate system (at the time of creation) and
	// the coordinate system specified, to check for normal flipping.
	//	EqOrientation CSO=pSurface()->pAttributes()->eCoordsysOrientation();
	TqInt O = pAttributes() ->GetIntegerAttribute("System", "Orientation")[0];
	float neg = 1;
	if ( O != OrientationLH ) neg = -1;
	CqVector3D  vecMP[ 4 ];
	CqVector3D	vecN, vecTemp;
	CqVector3D	vecLastN( 0, 0, 0 );

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
			P()->GetPoint( vecMP[ 0 ], igrid);
			P()->GetPoint( vecMP[ 1 ], igrid + 1);
			P()->GetPoint( vecMP[ 2 ], igrid + ur + 1 );
			P()->GetPoint( vecMP[ 3 ], igrid + ur );
			int a, b, c;
			a = 0;
			b = a + 1;
			while ( ( ( vecMP[ a ] - vecMP[ b ] ).Magnitude() < FLT_EPSILON ) && b < 3 ) b++;
			if ( b < 3 )
			{
				c = b + 1;
				while ( ( ( vecMP[ c ] - vecMP[ b ] ).Magnitude() < FLT_EPSILON || ( vecMP[ c ] - vecMP[ a ] ).Magnitude() < FLT_EPSILON ) && c < 3 )
					c++;
				if ( c <= 3 )
				{
					vecN = ( vecMP[ b ] - vecMP[ a ] ) % ( vecMP[ c ] - vecMP[ a ] );	// Cross product is normal.*/
					vecN.Unit();
					// Flip the normal if the 'current orientation' differs from the 'coordinate system orientation'
					// see RiSpec 'Orientation and Sides'
					vecN *= neg;
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
			Ng()->SetNormal( vecN, igrid );
			igrid++;
			// If we are at the last row, last row normal to the same.
			if ( iv == vr - 1 ) 
				Ng()->SetNormal( vecN, ( vr * ( ur + 1 ) ) + iu );
		}
		// Set the last one on the row to the same.
		Ng()->SetNormal( vecN, igrid );
		igrid++;
	}
	// Set the very last corner value to the last normal calculated.
	Ng()->SetNormal( vecN, ( vr + 1 ) * ( ur + 1 ) - 1 );
}


//---------------------------------------------------------------------
/** Shade the grid using the surface parameters of the surface passed and store the color values for each micropolygon.
 */

void CqMicroPolyGrid::Shade()
{
	// Sanity checks
	if( NULL == P() || NULL == I() )	return;
	
	static CqVector3D	vecE( 0, 0, 0 );

	IqShader* pshadSurface = pSurface() ->pAttributes() ->pshadSurface();
	IqShader* pshadDisplacement = pSurface() ->pAttributes() ->pshadDisplacement();
	IqShader* pshadAtmosphere = pSurface() ->pAttributes() ->pshadAtmosphere();

	TqInt lUses = pSurface() ->Uses();
	TqInt gs = GridSize();
	long cCulled = 0;


	// Calculate geometric normals if not specified by the surface.
	if ( !bGeometricNormals() )	CalcNormals();
	// If shading normals are not explicitly specified, they default to the geometric normal.
	if ( !bShadingNormals() && USES( lUses, EnvVars_N ) && NULL != Ng() && NULL != N() ) 
	{
		N()->SetValueFromVariable( Ng() );
	}

	// Setup uniform variables.
	if ( USES( lUses, EnvVars_E ) ) E()->SetVector( vecE );
	if ( USES( lUses, EnvVars_du ) )
	{
		TqFloat u1, u2, u3; 
		u()->GetFloat( u1, 0 );
		u()->GetFloat( u2, uGridRes() );
		u()->GetFloat( u3, vGridRes() * ( uGridRes() + 1 ) );
		TqFloat adu = ( ( u2 - u1 ) / uGridRes() );
		TqFloat bdu = ( ( u3 - u1 ) / vGridRes() );
		du()->SetFloat( adu + bdu );
	}
	if ( USES( lUses, EnvVars_dv ) )
	{
		TqFloat v1, v2, v3; 
		v()->GetFloat( v1, 0 );
		v()->GetFloat( v2, uGridRes() );
		v()->GetFloat( v3, vGridRes() * ( uGridRes() + 1 ) );
		TqFloat adv = ( ( v2 - v1 ) / uGridRes() );
		TqFloat bdv = ( ( v3 - v1 ) / vGridRes() );
		dv()->SetFloat( adv + bdv );
	}

	if ( USES( lUses, EnvVars_Ci ) ) Ci()->SetColor( gColBlack );
	if ( USES( lUses, EnvVars_Oi ) ) Oi()->SetColor( gColWhite );

	// Setup varying variables.
	TqInt i = 0;
	do
	{
		// Convert to 3D now, as all operations in SL are in 3D not 4D.
		CqVector3D vecTemp;
		P()->GetPoint( vecTemp, i );
		I()->SetVector( vecTemp, i );
		if ( USES( lUses, EnvVars_dPdu ) ) dPdu()->SetVector( SO_DuType<CqVector3D>( P(), i, m_pShaderExecEnv ), i );
		if ( USES( lUses, EnvVars_dPdv ) ) dPdv()->SetVector( SO_DvType<CqVector3D>( P(), i, m_pShaderExecEnv ), i );
	}
	while ( ++i < GridSize() );

	// Now try and cull any transparent MPs
	if ( USES( lUses, EnvVars_Os ) && QGetRenderContext() ->optCurrent().GetIntegerOption("System", "DisplayMode")[0] & ModeZ )
	{
		QGetRenderContext() ->Stats().OcclusionCullTimer().Start();
		TqInt i = 0;
		do
		{
			CqColor opacity;
			Os()->GetColor( opacity, i );
			if ( ( opacity != gColWhite ) )
				cCulled ++;
			else break;
		}
		while ( ++i < GridSize() );
		QGetRenderContext() ->Stats().OcclusionCullTimer().Stop();

		if ( cCulled == gs )
		{
			m_vfCulled = TqTrue;
			QGetRenderContext() ->Stats().IncCulledGrids();
			return ;
		}

	}

	// Now try and cull any true transparent MPs
	cCulled = 0;
	if ( USES( lUses, EnvVars_Os ) && QGetRenderContext() ->optCurrent().GetIntegerOption("System", "DisplayMode")[0] & ModeRGB )
	{
		QGetRenderContext() ->Stats().OcclusionCullTimer().Start();
		TqInt i = 0;
		do
		{
			CqColor opacity;
			Os()->GetColor( opacity, i );
			if ( ( opacity == gColBlack ) )
				cCulled ++;
			else break;
		}
		while ( ++i < GridSize() );
		QGetRenderContext() ->Stats().OcclusionCullTimer().Stop();

		if ( cCulled == gs )
		{
			m_vfCulled = TqTrue;
			QGetRenderContext() ->Stats().IncCulledGrids();
			return ;
		}
	}

	if ( pshadDisplacement != 0 )
	{
		QGetRenderContext() ->Stats().DisplacementTimer().Start();
		pshadDisplacement->Initialise( uGridRes(), vGridRes(), m_pShaderExecEnv );
		pshadDisplacement->Evaluate( m_pShaderExecEnv );
		QGetRenderContext() ->Stats().DisplacementTimer().Stop();
	}

	// Now try and cull any hidden MPs if Sides==1
	if ( pAttributes() ->GetIntegerAttribute("System", "Sides")[0] == 1 && m_pCSGNode == NULL )
	{
		cCulled = 0;
		QGetRenderContext() ->Stats().OcclusionCullTimer().Start();
		TqInt i = 0;
		do
		{
			// Calulate the direction the MPG is facing.
			CqVector3D vecN, vecP;
			N()->GetNormal( vecN, i );
			P()->GetPoint( vecP, i );
			if ( ( vecN * vecP ) >= 0 )
				cCulled++;
		}
		while ( ++i < GridSize() );
		QGetRenderContext() ->Stats().OcclusionCullTimer().Stop();

		// If the whole grid is culled don't bother going any further.
		if ( cCulled == gs )
		{
			m_vfCulled = TqTrue;
			QGetRenderContext() ->Stats().IncCulledGrids();
			return ;
		}

	}

	// Now shade the grid.
	QGetRenderContext() ->Stats().SurfaceTimer().Start();
	if( NULL != pshadSurface ) 
	{
		pshadSurface->Initialise( uGridRes(), vGridRes(), m_pShaderExecEnv );
		pshadSurface->Evaluate( m_pShaderExecEnv );
	}
	QGetRenderContext() ->Stats().SurfaceTimer().Stop();

	// Now try and cull any true transparent MPs (assigned by the shader code
	static CqRandom rand;

	cCulled = 0;
	if ( USES( lUses, EnvVars_Os ) && QGetRenderContext() ->optCurrent().GetIntegerOption("System", "DisplayMode")[0] & ModeRGB )
	{
		QGetRenderContext() ->Stats().OcclusionCullTimer().Start();
		TqInt i = 0;
		do
		{
			CqColor opacity;
			Os()->GetColor( opacity, i );
			if ( ( opacity == gColBlack ) )
				cCulled ++;
			else break;
		}
		while ( ++i < GridSize() );
		QGetRenderContext() ->Stats().OcclusionCullTimer().Stop();

		if ( cCulled == gs )
		{
			m_vfCulled = TqTrue;
			QGetRenderContext() ->Stats().IncCulledGrids();
			return ;
		}
	}
	// Now perform atmosphere shading
	if ( pshadAtmosphere != 0 )
	{
		QGetRenderContext() ->Stats().AtmosphereTimer().Start();
		pshadAtmosphere->Initialise( uGridRes(), vGridRes(), m_pShaderExecEnv );
		pshadAtmosphere->Evaluate( m_pShaderExecEnv );
		QGetRenderContext() ->Stats().AtmosphereTimer().Stop();
	}

	// Delete unneeded variables so that we don't use up unnecessary memory
	m_pShaderExecEnv->DeleteVariable( EnvVars_Cs );
	m_pShaderExecEnv->DeleteVariable( EnvVars_Os );
	//DeleteVariable( EnvVars_Ng );
	m_pShaderExecEnv->DeleteVariable( EnvVars_du );
	m_pShaderExecEnv->DeleteVariable( EnvVars_dv );
	m_pShaderExecEnv->DeleteVariable( EnvVars_L );
	m_pShaderExecEnv->DeleteVariable( EnvVars_Cl );
	m_pShaderExecEnv->DeleteVariable( EnvVars_Ol );
	//DeleteVariable(EnvVars_P);
	m_pShaderExecEnv->DeleteVariable( EnvVars_dPdu );
	m_pShaderExecEnv->DeleteVariable( EnvVars_dPdv );
	//	DeleteVariable(EnvVars_N);
	//	DeleteVariable(EnvVars_u);
	//	DeleteVariable(EnvVars_v);
	m_pShaderExecEnv->DeleteVariable( EnvVars_s );
	m_pShaderExecEnv->DeleteVariable( EnvVars_t );
	m_pShaderExecEnv->DeleteVariable( EnvVars_I );
	//DeleteVariable(EnvVars_Ci);
	//DeleteVariable(EnvVars_Oi);
	m_pShaderExecEnv->DeleteVariable( EnvVars_Ps );
	m_pShaderExecEnv->DeleteVariable( EnvVars_E );
	m_pShaderExecEnv->DeleteVariable( EnvVars_ncomps );
	m_pShaderExecEnv->DeleteVariable( EnvVars_time );
	m_pShaderExecEnv->DeleteVariable( EnvVars_alpha );
}


//---------------------------------------------------------------------
/** Project the grid from camera space into raster space.
 */

void CqMicroPolyGrid::Project()
{
	if( NULL == P() )
		return;
	
	CqMatrix matCameraToRaster = QGetRenderContext() ->matSpaceToSpace( "camera", "raster" );
	// Transform the whole grid to hybrid camera/raster space
	TqInt i = 0;
	do
	{
		CqVector3D	vecP;
		P()->GetPoint( vecP, i );
		TqFloat zdepth = vecP.z();
		vecP = matCameraToRaster * vecP;
		vecP.z( zdepth );
		P()->SetPoint( vecP, i );
	}
	while ( ++i < GridSize() );
}


//---------------------------------------------------------------------
/** Bound the grid in its current space, usually raster
 */

CqBound CqMicroPolyGrid::Bound()
{
	CqBound B( FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX );
	// Get all point in the grid.
	TqInt i = 0;
	do
	{
		CqVector3D vecTemp;
		P()->GetPoint( vecTemp, i );
		B.Encapsulate( vecTemp );
	}
	while ( ++i < GridSize() );
	return ( B );
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
	if( NULL == P() )	
		return;
	
	TqInt cu = uGridRes();
	TqInt cv = vGridRes();

	// Get the required trim curve sense, if specified, defaults to "inside".
	const CqString* pattrTrimSense = pAttributes() ->GetStringAttribute( "trimcurve", "sense" );
	CqString strTrimSense( "inside" );
	if ( pattrTrimSense != 0 ) strTrimSense = pattrTrimSense[ 0 ];
	TqBool bOutside = strTrimSense == "outside";

	AddRef();

	TqInt iv;
	for ( iv = 0; iv < cv; iv++ )
	{
		TqInt iu;
		for ( iu = 0; iu < cu; iu++ )
		{
			TqInt iIndex = ( iv * ( cu + 1 ) ) + iu;

			// If culled, ignore it
			if ( m_vfCulled )
			{
				QGetRenderContext() ->Stats().IncCulledMPGs( 1 );
				continue;
			}

			// If the MPG is trimmed then don't add it.
			TqBool fTrimmed = TqFalse;
			if ( pSurface() ->bCanBeTrimmed() && NULL != u() && NULL != v() )
			{
				TqFloat fu, fv;
				u()->GetFloat( fu, iIndex ); 
				v()->GetFloat( fv, iIndex );
				TqBool fTrimA = pSurface() ->bIsPointTrimmed( CqVector2D( fu, fv ) );
				u()->GetFloat( fu, iIndex + 1 ); 
				v()->GetFloat( fv, iIndex + 1 );
				TqBool fTrimB = pSurface() ->bIsPointTrimmed( CqVector2D( fu, fv ) );
				u()->GetFloat( fu, iIndex + cu + 2); 
				v()->GetFloat( fv, iIndex +cu + 2 );
				TqBool fTrimC = pSurface() ->bIsPointTrimmed( CqVector2D( fu, fv ) );
				u()->GetFloat( fu, iIndex + cu + 1 ); 
				v()->GetFloat( fv, iIndex +cu + 1 );
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

			CqMicroPolygonStatic *pNew = new CqMicroPolygonStatic();
			pNew->SetGrid( this );
			pNew->SetIndex( iIndex );
			if ( fTrimmed ) pNew->MarkTrimmed();
			CqVector3D vec1, vec2, vec3, vec4;
			P()->GetPoint( vec1, iIndex );
			P()->GetPoint( vec2, iIndex + 1 );
			P()->GetPoint( vec3, iIndex + cu + 2 );
			P()->GetPoint( vec4, iIndex + cu + 1 );
			pNew->Initialise( vec1, vec2, vec3, vec4 );
			pNew->GetTotalBound( TqTrue );

			pImage->AddMPG( pNew );
		}
	}

	Release();
}


//---------------------------------------------------------------------
/** Project all grids from camera to raster space.
 */

void CqMotionMicroPolyGrid::Project()
{
	CqMatrix matCameraToRaster = QGetRenderContext() ->matSpaceToSpace( "camera", "raster" );
	// Transform all grids to hybrid camera/raster space
	TqInt iTime;
	for ( iTime = 0; iTime < cTimes(); iTime++ )
	{
		CqMicroPolyGrid* pGrid = static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( iTime ) ) );
		pGrid->Project();
	}
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
/** Bound all grids in their current space, usually raster.
 */

CqBound CqMotionMicroPolyGrid::Bound()
{
	CqBound B( FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX );
	// Bound all grids.
	TqInt iTime;
	for ( iTime = 0; iTime < cTimes(); iTime++ )
	{
		CqMicroPolyGrid* pGrid = static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( iTime ) ) );
		B = B.Combine( pGrid->Bound() );
	}
	return ( B );
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

	pGridA->AddRef();

	TqInt iv;
	for ( iv = 0; iv < cv; iv++ )
	{
		TqInt iu;
		for ( iu = 0; iu < cu; iu++ )
		{
			// If culled, ignore it
			if ( pGridA->vfCulled() )
			{
				QGetRenderContext() ->Stats().IncCulledMPGs( 1 );
				continue;
			}

			CqMicroPolygonMotion *pNew = new CqMicroPolygonMotion();
			pNew->SetGrid( pGridA );
			TqInt iIndex = ( iv * ( cu + 1 ) ) + iu;
			pNew->SetIndex( iIndex );
			for ( iTime = 0; iTime < cTimes(); iTime++ )
			{
				CqMicroPolyGrid* pGridT = static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( iTime ) ) );
				CqVector3D vec1, vec2, vec3, vec4;
				pGridA->P()->GetPoint( vec1, iIndex );
				pGridA->P()->GetPoint( vec2, iIndex + 1 );
				pGridA->P()->GetPoint( vec3, iIndex + cu + 2 );
				pGridA->P()->GetPoint( vec4, iIndex + cu + 1 );
				pNew->Initialise( vec1, vec2, vec3, vec4, Time( iTime ) );
			}
			pNew->GetTotalBound( TqTrue );

			pImage->AddMPG( pNew );
		}
	}

	pGridA->Release();

	// Delete the donor motion grids, as their work is done.
	for ( iTime = 1; iTime < cTimes(); iTime++ )
		delete( GetMotionObject( Time( iTime ) ) );
}


//---------------------------------------------------------------------
/** Default constructor
 */

CqMicroPolygonBase::CqMicroPolygonBase() : m_pGrid( 0 ), m_RefCount( 0 ), m_bHit( TqFalse )
{
	QGetRenderContext() ->Stats().IncMPGsAllocated();
}


//---------------------------------------------------------------------
/** Copy constructor
 */

CqMicroPolygonBase::CqMicroPolygonBase( const CqMicroPolygonBase& From ) : m_bHit( TqFalse )
{
	QGetRenderContext() ->Stats().IncMPGsAllocated();
	*this = From;
}


//---------------------------------------------------------------------
/** Destructor
 */

CqMicroPolygonBase::~CqMicroPolygonBase()
{
	if ( m_pGrid ) m_pGrid->Release();
	QGetRenderContext() ->Stats().IncMPGsDeallocated();
	if( !m_bHit )
		QGetRenderContext() ->Stats().IncMissedMPGs();
}


//---------------------------------------------------------------------
/** Store the vectors of the micropolygon.
 * \param vA 3D Vector.
 * \param vB 3D Vector.
 * \param vC 3D Vector.
 * \param vD 3D Vector.
 */

void CqMicroPolygonStaticBase::Initialise( const CqVector3D& vA, const CqVector3D& vB, const CqVector3D& vC, const CqVector3D& vD )
{
	// Check for degenerate case, if any of the neighbouring points are the same, shuffle them down, and 
	// duplicate the last point exactly. Exact duplication of the last two points is used as a marker in the 
	// fContains function to indicate degeneracy. If more that two points are coincident, we are in real trouble!
	const CqVector3D& vvB = (vA-vB).Magnitude()<1e-2?vC:vB;
	const CqVector3D& vvC = (vvB-vC).Magnitude()<1e-2?vD:vC;
	const CqVector3D& vvD = (vvC-vD).Magnitude()<1e-2?vvC:vD;

	// Determine whether the MPG is CW or CCW, must be CCW for fContains to work.
	bool fFlip = ( ( vA.x() - vvB.x() ) * ( vvB.y() - vvC.y() ) ) >= ( ( vA.y() - vvB.y() ) * ( vvB.x() - vvC.x() ) );

	if ( !fFlip )
	{
		m_vecPoints[ 0 ] = vA;
		m_vecPoints[ 1 ] = vvD;
		m_vecPoints[ 2 ] = vvC;
		m_vecPoints[ 3 ] = vvB;
	}
	else
	{
		m_vecPoints[ 0 ] = vA;
		m_vecPoints[ 1 ] = vvB;
		m_vecPoints[ 2 ] = vvC;
		m_vecPoints[ 3 ] = vvD;
	}

	m_vecN = (vA - vvB) % (vvC - vvB);
	m_vecN.Unit();
	m_D = m_vecN * vA;
}


//---------------------------------------------------------------------
/** Determinde whether the 2D point specified lies within this micropolygon.
 * \param vecP 2D vector to test for containment.
 * \param Depth Place to put the depth if valid intersection.
 * \return Boolean indicating sample hit.
 */

TqBool CqMicroPolygonStaticBase::fContains( const CqVector2D& vecP, TqFloat& Depth )
{
	// Check against each line of the quad, if outside any then point is outside MPG, therefore early exit.
	TqFloat r1, r2, r3, r4;
	TqFloat x = vecP.x(), y = vecP.y();
	TqFloat x0 = m_vecPoints[ 0 ].x(), y0 = m_vecPoints[ 0 ].y(), x1 = m_vecPoints[ 1 ].x(), y1 = m_vecPoints[ 1 ].y();
	if ( ( r1 = ( y - y0 ) * ( x1 - x0 ) - ( x - x0 ) * ( y1 - y0 ) ) < 0 ) return ( TqFalse );
	x0 = x1; y0 = y1; x1 = m_vecPoints[ 2 ].x(); y1 = m_vecPoints[ 2 ].y();
	if ( ( r2 = ( y - y0 ) * ( x1 - x0 ) - ( x - x0 ) * ( y1 - y0 ) ) < 0 ) return ( TqFalse );
	x0 = x1; y0 = y1; x1 = m_vecPoints[ 3 ].x(); y1 = m_vecPoints[ 3 ].y();
	if ( ( r3 = ( y - y0 ) * ( x1 - x0 ) - ( x - x0 ) * ( y1 - y0 ) ) < 0 ) return ( TqFalse );
	x0 = x1; y0 = y1; x1 = m_vecPoints[ 0 ].x(); y1 = m_vecPoints[ 0 ].y();
	
	// Check for degeneracy.
	if ( ! ( x0 == x1 && y0 == y1 ) )
		if ( ( r4 = ( y - y0 ) * ( x1 - x0 ) - ( x - x0 ) * ( y1 - y0 ) ) < 0 ) return ( TqFalse );

	Depth = ( m_D - ( m_vecN.x() * vecP.x() ) - ( m_vecN.y() * vecP.y() ) ) / m_vecN.z();

	return ( TqTrue );
}


//---------------------------------------------------------------------
/** Fill this micropolygons data as the linear interpolation of the two specified MPGs at Fraction.
 * \param Fraction Distance between the two MPGs to interpolate to, 0-1.
 * \param MPA Start MPG.
 * \param MPB End MPG.
 * \return Reference to this.
 */

CqMicroPolygonStaticBase& CqMicroPolygonStaticBase::LinearInterpolate( TqFloat Fraction, const CqMicroPolygonStaticBase& MPA, const CqMicroPolygonStaticBase& MPB )
{
	TqFloat F1 = 1.0f - Fraction;
	m_vecPoints[ 0 ] = ( F1 * MPA.m_vecPoints[ 0 ] ) + ( Fraction * MPB.m_vecPoints[ 0 ] );
	m_vecPoints[ 1 ] = ( F1 * MPA.m_vecPoints[ 1 ] ) + ( Fraction * MPB.m_vecPoints[ 1 ] );
	m_vecPoints[ 2 ] = ( F1 * MPA.m_vecPoints[ 2 ] ) + ( Fraction * MPB.m_vecPoints[ 2 ] );
	m_vecPoints[ 3 ] = ( F1 * MPA.m_vecPoints[ 3 ] ) + ( Fraction * MPB.m_vecPoints[ 3 ] );
	m_vecN = ( F1 * MPA.m_vecN ) + ( Fraction * MPB.m_vecN );
	m_D = ( F1 * MPA.m_D ) + ( Fraction * MPB.m_D );

	return ( *this );
}


CqVector2D CqMicroPolygonStaticBase::ReverseBilinear( CqVector2D& v )
{
	CqVector2D kA, kB, kC, kD;

	kA = CqVector2D( m_vecPoints[ 0 ] );
	kB = CqVector2D( m_vecPoints[ 1 ] ) - kA;
	kC = CqVector2D( m_vecPoints[ 3 ] ) - kA;
	kD = CqVector2D( m_vecPoints[ 2 ] ) + kA - CqVector2D( m_vecPoints[ 1 ] ) - CqVector2D( m_vecPoints[ 3 ] );

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

CqBound CqMicroPolygonStaticBase::GetTotalBound() const
{
	CqBound Bound;
	// Calculate the boundary, and store the indexes in the cache.
	const CqVector3D& A = m_vecPoints[ 0 ];
	const CqVector3D& B = m_vecPoints[ 1 ];
	const CqVector3D& C = m_vecPoints[ 2 ];
	const CqVector3D& D = m_vecPoints[ 3 ];

	Bound.vecMin().x( MIN( A.x(), MIN( B.x(), MIN( C.x(), D.x() ) ) ) );
	Bound.vecMin().y( MIN( A.y(), MIN( B.y(), MIN( C.y(), D.y() ) ) ) );
	Bound.vecMin().z( MIN( A.z(), MIN( B.z(), MIN( C.z(), D.z() ) ) ) );
	Bound.vecMax().x( MAX( A.x(), MAX( B.x(), MAX( C.x(), D.x() ) ) ) );
	Bound.vecMax().y( MAX( A.y(), MAX( B.y(), MAX( C.y(), D.y() ) ) ) );
	Bound.vecMax().z( MAX( A.z(), MAX( B.z(), MAX( C.z(), D.z() ) ) ) );

	return ( Bound );
}


//---------------------------------------------------------------------
/** Sample the specified point against the MPG at the specified time.
 * \param vecP 2D vector to sample against.
 * \param time Shutter time to sample at.
 * \param D Storage for depth if valid hit.
 * \return Boolean indicating smaple hit.
 */

TqBool CqMicroPolygonStatic::Sample( CqVector2D& vecSample, TqFloat time, TqFloat& D )
{
	if ( fContains( vecSample, D ) )
	{
		// Now check if it is trimmed.
		if ( m_fTrimmed )
		{
			// Get the required trim curve sense, if specified, defaults to "inside".
			const CqString * pattrTrimSense = pGrid() ->pAttributes() ->GetStringAttribute( "trimcurve", "sense" );
			CqString strTrimSense( "inside" );
			if ( pattrTrimSense != 0 ) strTrimSense = pattrTrimSense[ 0 ];
			TqBool bOutside = strTrimSense == "outside";

			CqVector2D vecUV = ReverseBilinear( vecSample );

			TqFloat u, v;

			pGrid() ->u()->GetFloat( u, m_Index );
			pGrid() ->v()->GetFloat( v, m_Index );
			CqVector2D uvA( u, v );
			
			pGrid() ->u()->GetFloat( u, m_Index + 1 ); 
			pGrid() ->v()->GetFloat( v, m_Index + 1 );
			CqVector2D uvB( u, v );
			
			pGrid() ->u()->GetFloat( u, m_Index + pGrid() ->uGridRes() + 1 );	
			pGrid() ->v()->GetFloat( v, m_Index + pGrid() ->uGridRes() + 1 );
			CqVector2D uvC( u, v );
			
			pGrid() ->u()->GetFloat( u, m_Index + pGrid() ->uGridRes() + 2 );	
			pGrid() ->v()->GetFloat( v, m_Index + pGrid() ->uGridRes() + 2 );
			CqVector2D uvD( u, v );

			CqVector2D vR = BilinearEvaluate( uvA, uvB, uvC, uvD, vecUV.x(), vecUV.y() );

			if ( pGrid() ->pSurface() ->bCanBeTrimmed() && pGrid() ->pSurface() ->bIsPointTrimmed( vR ) && !bOutside )
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

CqBound& CqMicroPolygonStatic::GetTotalBound( TqBool fForce )
{
	if ( fForce )
		m_Bound = CqMicroPolygonStaticBase::GetTotalBound();
	return ( m_Bound );
}


//---------------------------------------------------------------------
/** Calculate the 2D boundary of this micropolygon,
 * \param fForce Boolean flag to force the recalculation of the cached bound.
 */

CqBound& CqMicroPolygonMotion::GetTotalBound( TqBool fForce )
{
	if ( fForce )
	{
		// Calculate the boundary from the various motion positions.
		m_Bound = GetMotionObject( Time( 0 ) ).GetTotalBound();
		TqInt i;
		for ( i = 1; i < cTimes(); i++ )
			m_Bound = m_Bound.Combine( GetMotionObject( Time( i ) ).GetTotalBound() );

	}
	return ( m_Bound );
}

//---------------------------------------------------------------------
/** Calculate a list of 2D bounds for this micropolygon,
 */
void CqMicroPolygonMotion::BuildBoundList()
{
	m_BoundList.Clear();

	CqBound start = GetMotionObject( Time( 0 ) ).GetTotalBound();
	TqFloat startTime = Time( 0 );
	for ( TqInt i = 1; i < cTimes(); i++ )
	{
		CqBound end = GetMotionObject( Time( i ) ).GetTotalBound();
		CqBound mid0( start );
		CqBound mid1;
		TqFloat endTime = Time( i );
		TqFloat time = startTime;

		TqInt d;
		// arbitary number of divisions, should be related to screen size of blur
		TqInt divisions = 10;
		for ( d = 1; d <= divisions; d++ )
		{
			TqFloat delta = ( float ) d / ( float ) divisions;
			CqVector3D min = delta * ( end.vecMin() - start.vecMin() ) + start.vecMin();
			CqVector3D max = delta * ( end.vecMax() - start.vecMax() ) + start.vecMax();
			mid1 = CqBound( min, max );

			CqBound* combinedBound = new CqBound;
			*combinedBound = mid0.Combine( mid1 );
			m_BoundList.Add( combinedBound, time );

			time = delta * ( endTime - startTime ) + startTime;
			mid0 = mid1;
		}
		start = end;
		startTime = endTime;
	}
	m_BoundReady = TqTrue;
}


//---------------------------------------------------------------------
/** Sample the specified point against the MPG at the specified time.
 * \param vecP 2D vector to sample against.
 * \param time Shutter time to sample at.
 * \param D Storage for depth if valid hit.
 * \return Boolean indicating smaple hit.
 */

TqBool CqMicroPolygonMotion::Sample( CqVector2D& vecSample, TqFloat time, TqFloat& D )
{
	CqMicroPolygonStaticBase MP = GetMotionObjectInterpolated( time );
	if ( MP.fContains( vecSample, D ) )
	{
		//		QGetRenderContext()->Stats().IncSampleHits();  This shouldn't be here, should it? [MB]
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

void CqMicroPolygonMotion::Initialise( const CqVector3D& vA, const CqVector3D& vB, const CqVector3D& vC, const CqVector3D& vD, TqFloat time )
{
	// Add a new planeset at the specified time.
	CqMicroPolygonStaticBase MP( vA, vB, vC, vD );
	AddTimeSlot( time, MP );
	if ( Time( 0 ) == time )
		m_Bound = MP.GetTotalBound();
	else
		ExpandBound( MP );
}

//---------------------------------------------------------------------
/** Expand the stored bound to include the specified micropolygon.
 */

void CqMicroPolygonMotion::ExpandBound( const CqMicroPolygonStaticBase& MP )
{
	m_Bound.Combine( MP.GetTotalBound() );
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
