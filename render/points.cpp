// Aqsis
// Copyright c 1997 - 2001, Paul C. Gregory
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
		\brief Implements CqPoints using small regular polygon (first try) This is more or less an experimentation with the parser. Later a micropolygon grid will be used to be more efficient to shade/render.
		\author M. Joron (joron@sympatico.ca)
*/ 
/*    References:
 *          [PIXA89]  Pixar, The RenderMan Interface, Version 3.2, 
 *                    Richmond, CA, September 1989.  
 *
 */

#include	<math.h>

#include	"aqsis.h"
#include	"points.h"
#include	"micropolygon.h"
#include	"imagebuffer.h"
#include	"polygon.h"

#include	"ri.h"

START_NAMESPACE( Aqsis )

#define NBR_SEGMENTS 6

void CqPointsKDTreeData::SetpPoints( CqPoints* pPoints )
{
	pPoints->AddRef();
	if( NULL != m_pPoints )
		m_pPoints->Release();
	m_pPoints = pPoints;
}

bool CqPointsKDTreeData::CqPointsKDTreeDataComparator::operator()(TqInt a, TqInt b)	
{  
	return( ( ( *m_pPoints->pPoints()->P() )[ a ][m_Dim] ) < ( ( *m_pPoints->pPoints()->P() )[ b ][m_Dim] ) ); 
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqPoints::CqPoints( TqInt nvertices, CqPolygonPoints* pPoints ) : m_nVertices(nvertices), 
																  m_pPoints(pPoints), 
																  m_KDTree(&m_KDTreeData),
																  m_MaxWidth(0)
{
	assert( NULL != pPoints );
	assert( nvertices > 0 );

	m_widthParamIndex = -1;
	m_constantwidthParamIndex = -1;

	m_pPoints->AddRef();

	m_KDTreeData.SetpPoints( this );
	
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqPoints&	CqPoints::operator=( const CqPoints& From )
{
	CqSurface::operator=( From );
	m_nVertices = From.m_nVertices;
	m_KDTreeData.SetpPoints( this );

	if( NULL != m_pPoints )
		m_pPoints->Release();

	m_pPoints = From.m_pPoints;
	m_pPoints->AddRef();

	m_widthParamIndex = From.m_widthParamIndex;
	m_constantwidthParamIndex = From.m_constantwidthParamIndex;
	m_MaxWidth = From.m_MaxWidth;

	return ( *this );
}



//---------------------------------------------------------------------
/** Dice the quadric into a grid of MPGs for rendering.
 */

CqMicroPolyGridBase* CqPoints::Dice()
{
	CqMicroPolyGridPoints* pGrid = new CqMicroPolyGridPoints( nVertices(), 1, this );
	
	TqInt lUses = Uses();

	// Dice the primitive variables.
	if ( USES( lUses, EnvVars_Cs ) && ( NULL != pGrid->Cs() ) )
	{
		if ( pPoints()->bHasCs() )
			Cs() ->Dice( m_uDiceSize, m_vDiceSize, pGrid->Cs(), this );
		else if ( NULL != pAttributes() ->GetColorAttribute( "System", "Color" ) )
			pGrid->Cs() ->SetColor( pAttributes() ->GetColorAttribute( "System", "Color" ) [ 0 ] );
		else
			pGrid->Cs() ->SetColor( CqColor( 1, 1, 1 ) );
	}

	if ( USES( lUses, EnvVars_Os ) && ( NULL != pGrid->Os() ) )
	{
		if ( bHasOs() )
			Os() ->Dice( m_uDiceSize, m_vDiceSize, pGrid->Os(), this );
		else if ( NULL != pAttributes() ->GetColorAttribute( "System", "Opacity" ) )
			pGrid->Os() ->SetColor( pAttributes() ->GetColorAttribute( "System", "Opacity" ) [ 0 ] );
		else
			pGrid->Os() ->SetColor( CqColor( 1, 1, 1 ) );
	}

	if ( USES( lUses, EnvVars_s ) && ( NULL != pGrid->s() ) && pPoints()->bHass() )
		s() ->Dice( nVertices(), 1, pGrid->s(), this );

	if ( USES( lUses, EnvVars_t ) && ( NULL != pGrid->t() ) && pPoints()->bHast() )
		t() ->Dice( nVertices(), 1, pGrid->t(), this );

	if ( USES( lUses, EnvVars_u ) && ( NULL != pGrid->u() ) && pPoints()->bHasu() )
		u() ->Dice( nVertices(), 1, pGrid->u(), this );

	if ( USES( lUses, EnvVars_v ) && ( NULL != pGrid->v() ) && pPoints()->bHasv() )
		v() ->Dice( nVertices(), 1, pGrid->v(), this );


	if ( NULL != pGrid->P() )
		NaturalDice( pPoints()->P(), nVertices(), 1, pGrid->P() );

	// If the shaders need N and they have been explicitly specified, then bilinearly interpolate them.
	if ( USES( lUses, EnvVars_N ) && ( NULL != pGrid->N() ) && pPoints()->bHasN() )
	{
		N() ->Dice( nVertices(), 1, pGrid->N(), this );
		pGrid->SetbShadingNormals( TqTrue );
	}

	if ( USES( lUses, EnvVars_Ng ) )
	{
		CqVector3D	N(0,0,1);
		//N = QGetRenderContext() ->matSpaceToSpace( "camera", "object", CqMatrix(), pGrid->matObjectToWorld() ) * N;
		TqInt u;
		for ( u = 0; u <= nVertices(); u++ )
		{
			TqInt O = pAttributes() ->GetIntegerAttribute( "System", "Orientation" ) [ 0 ];
			N = ( O == OrientationLH ) ? N : -N;
			pGrid->Ng()->SetNormal( N, u );
		}
		pGrid->SetbGeometricNormals( TqTrue );
	}

	// Now we need to dice the user specified parameters as appropriate.
	std::vector<CqParameter*>::iterator iUP;
	std::vector<CqParameter*>::iterator end = pPoints()->aUserParams().end();
	for ( iUP = pPoints()->aUserParams().begin(); iUP != end ; iUP++ )
	{
		/// \todo: Must transform point/vector/normal/matrix parameter variables from 'object' space to current before setting.
		if ( NULL != pGrid->pAttributes() ->pshadSurface() )
			pGrid->pAttributes() ->pshadSurface() ->SetArgument( ( *iUP ), this );

		if ( NULL != pGrid->pAttributes() ->pshadDisplacement() )
			pGrid->pAttributes() ->pshadDisplacement() ->SetArgument( ( *iUP ), this );

		if ( NULL != pGrid->pAttributes() ->pshadAtmosphere() )
			pGrid->pAttributes() ->pshadAtmosphere() ->SetArgument( ( *iUP ), this );
	}

	return ( pGrid );
}


void CqPoints::NaturalDice( CqParameter* pParameter, TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pData )
{
	switch ( pParameter->Type() )
	{
			case type_float:
			{
				CqParameterTyped<TqFloat, TqFloat>* pTParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParameter );
				TypedNaturalDice( pTParam, pData );
				break;
			}

			case type_integer:
			{
				CqParameterTyped<TqInt, TqFloat>* pTParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParameter );
				TypedNaturalDice( pTParam, pData );
				break;
			}

			case type_point:
			case type_vector:
			case type_normal:
			{
				CqParameterTyped<CqVector3D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParameter );
				TypedNaturalDice( pTParam, pData );
				break;
			}

			case type_hpoint:
			{
				CqParameterTyped<CqVector4D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParameter );
				TypedNaturalDice( pTParam, pData );
				break;
			}

			case type_color:
			{
				CqParameterTyped<CqColor, CqColor>* pTParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParameter );
				TypedNaturalDice( pTParam, pData );
				break;
			}

			case type_string:
			{
				CqParameterTyped<CqString, CqString>* pTParam = static_cast<CqParameterTyped<CqString, CqString>*>( pParameter );
				TypedNaturalDice( pTParam, pData );
				break;
			}

			case type_matrix:
			{
				CqParameterTyped<CqMatrix, CqMatrix>* pTParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParameter );
				TypedNaturalDice( pTParam, pData );
				break;
			}
	}
}

//---------------------------------------------------------------------
/** Determine whether the quadric is suitable for dicing.
 */

TqBool	CqPoints::Diceable()
{
	TqInt gridsize;

	const TqInt* poptGridSize = QGetRenderContext() ->optCurrent().GetIntegerOption( "limits", "gridsize" );

	TqInt m_XBucketSize = 16;
	TqInt m_YBucketSize = 16;

	const TqInt* poptBucketSize = QGetRenderContext() ->optCurrent().GetIntegerOption( "limits", "bucketsize" );
	if ( poptBucketSize != 0 )
	{
		m_XBucketSize = poptBucketSize[ 0 ];
		m_YBucketSize = poptBucketSize[ 1 ];
	}
	TqFloat ShadingRate = pAttributes() ->GetFloatAttribute( "System", "ShadingRate" ) [ 0 ];

	if ( poptGridSize )
		gridsize = poptGridSize[ 0 ];
	else
		gridsize = static_cast<TqInt>( m_XBucketSize * m_XBucketSize / ShadingRate );

	if( nVertices() > gridsize )
		return ( TqFalse );
	else
		return ( TqTrue );
}


//---------------------------------------------------------------------
/** Transform the primitive variables for the primitive.
 */

void CqPoints::Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx )
{
	assert( NULL != m_pPoints );
	// Check and make sure we have indices to the width and constant width parameters if they have been specified.
	std::vector<CqParameter*>::iterator iUP;
	TqInt index = 0;
	for( iUP = pPoints()->aUserParams().begin(); iUP != pPoints()->aUserParams().end(); iUP++, index++ )
		if( (*iUP)->strName() == "constantwidth" && (*iUP)->Type() == type_float && (*iUP)->Class() == class_constant )
			m_constantwidthParamIndex = index;
		else if( (*iUP)->strName() == "width" && (*iUP)->Type() == type_float && (*iUP)->Class() == class_varying )
			m_widthParamIndex = index;

	// Make sure the camera space width values are set up correctly.
	PopulateWidth();

	for ( TqUint i = 0; i < cVarying(); i++ )
	{
		// first, create a horizontal vector in the new space which is
		//  the length of the current width in current space
		CqVector3D horiz( 1, 0, 0 );
		horiz = matITTx * horiz;
		horiz *= ( *width() ) [ i ] / horiz.Magnitude();

		// now, create two points; one at the vertex in current space
		//  and one which is offset horizontally in the new space by
		//  the width in the current space.  transform both points
		//  into the new space
		CqVector3D pt = ( *pPoints()->P() ) [ i ];
		CqVector3D pt_delta = pt + horiz;
		pt = matTx * pt;
		pt_delta = matTx * pt_delta;

		// finally, find the difference between the two points in
		//  the new space - this is the transformed width
		CqVector3D widthVector = pt_delta - pt;
		( *width() ) [ i ] = widthVector.Magnitude();
		m_MaxWidth = MAX( m_MaxWidth, widthVector.Magnitude() );
	}

	pPoints()->Transform( matTx, matITTx, matRTx );
}

//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqPoints::Bound() const
{
	CqBound	B;

	TqInt i;
	for( i = 0; i < nVertices(); i++ )
		B.Encapsulate( (*m_pPoints->P())[ m_KDTree.aLeaves()[ i ] ] );

	// Expand the bound to take into account the width of the particles.
	B.vecMax() += CqVector3D( m_MaxWidth, m_MaxWidth, m_MaxWidth );
	B.vecMin() += CqVector3D( m_MaxWidth, m_MaxWidth, m_MaxWidth );

	return ( B );
}

//---------------------------------------------------------------------
/** Split this GPrim into bicubic patches.
 */

TqInt CqPoints::Split( std::vector<CqBasicSurface*>& aSplits )
{
	TqInt median = nVertices()/2;
	// Split the KDTree and create two new primitives containing the split points set.
	CqPoints* pA = new CqPoints( *this );
	CqPoints* pB = new CqPoints( *this );

	pA->m_nVertices = median;
	pB->m_nVertices = nVertices()-median;

	pA->SetSurfaceParameters( *this );
	pB->SetSurfaceParameters( *this );

	KDTree().Subdivide( pA->KDTree(), pB->KDTree() );

	pA->AddRef();
	pB->AddRef();

	aSplits.push_back( pA );
	aSplits.push_back( pB );

	return( 2 );
}


//---------------------------------------------------------------------
/** Initialise the KDTree to contain all the points in the list. Settign the 
 * index list to the canonical form.
 */

void CqPoints::InitialiseKDTree()
{
	m_KDTree.aLeaves().reserve( nVertices() );
	TqInt i;
	for( i = 0; i < nVertices(); i++ )
		m_KDTree.aLeaves().push_back( i );
}


/**
 * Populates the "width" parameter if it is not already present (ie supplied
 * by the user).  The "width" is populated either by the value of
 * "constantwidth", or by the default object-space width 1.0.
 */
void CqPoints::PopulateWidth()
{
	// if the width parameter has been supplied by the user then bail
	//  immediately
	if ( width() != NULL )
		return ;

	// otherwise, find the value to fill the width array with; default
	//  value is 1.0 which can be overridden by the "constantwidth"
	//  parameter
	TqFloat widthvalue = 1.0;
	if ( constantwidth() != NULL )
	{
		widthvalue = *( constantwidth() ->pValue() );
	}

	// create and fill in the width array
	CqParameterTypedVarying<TqFloat, type_float, TqFloat>* widthP =
	    new CqParameterTypedVarying<TqFloat, type_float, TqFloat>(
	        "width"
	    );
	widthP->SetSize( cVarying() );
	for ( TqUint i = 0; i < cVarying(); i++ )
	{
		( *widthP ) [ i ] = widthvalue;
	}

	// add the width array to the curve as a primitive variable
	pPoints()->AddPrimitiveVariable( widthP );
	m_widthParamIndex = pPoints()->aUserParams().size() - 1;
}


void CqMicroPolyGridPoints::Split( CqImageBuffer* pImage, TqInt iBucket, long xmin, long xmax, long ymin, long ymax )
{
	if ( NULL == P() )
		return ;

	TqInt cu = uGridRes();	// Only need cu, as we know cv is 1.

	AddRef();

	CqVector3D* pP;
	P() ->GetPointPtr( pP );

	CqMatrix matObjToCamera = QGetRenderContext() ->matSpaceToSpace( "object", "camera", CqMatrix(), matObjectToWorld() );
	CqMatrix matCameraToRaster = QGetRenderContext() ->matSpaceToSpace( "camera", "raster" );

	CqVector3D vecdefOriginCamera = matObjToCamera * CqVector3D( 0.0f,0.0f,0.0f );
	CqVector3D vecdefOriginRaster = matCameraToRaster * CqVector3D( 0.0f,0.0f,0.0f );
	
	// Get a pointer to the surface, so that we can interrogate the "width" parameters.
	CqPoints* pPoints = static_cast<CqPoints*>( pSurface() );

	CqParameterTyped<CqVector4D, CqVector3D>* pPParam = pPoints->pPoints()->P();

	// Find out if the "constantwidth" parameter was specified.
	const CqParameterTypedConstant<TqFloat, type_float, TqFloat>* pConstantWidthParam = pPoints->constantwidth();
	CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pWidthParam = pPoints->width();

	TqFloat radius = 1.0f;

	if( NULL != pConstantWidthParam)
		radius = pConstantWidthParam->pValue()[ 0 ];

	TqInt iu;
	for ( iu = 0; iu < cu; iu++ )
	{
		CqVector3D Point = pP[ iu ];

		if( NULL != pWidthParam )
			radius = pWidthParam->pValue( pPoints->KDTree().aLeaves()[ iu ] )[ 0 ];
		
		CqVector3D vecCamP = pPParam->pValue( pPoints->KDTree().aLeaves()[ iu ] )[ 0 ];
		CqVector3D vecCamP2 = vecCamP + CqVector3D( radius, 0.0f, 0.0f );
		TqFloat ztemp = vecCamP2.z();
		CqVector3D vecRasP2 = matCameraToRaster * vecCamP2;
		vecRasP2.z( ztemp );
		TqFloat ras_radius = ( vecRasP2 - Point ).Magnitude();
		radius = ras_radius * 0.5f;

		CqVector3D p1( Point.x() - radius, Point.y() - radius, Point.z() );
		CqVector3D p2( Point.x() + radius, Point.y() - radius, Point.z() );
		CqVector3D p3( Point.x() + radius, Point.y() + radius, Point.z() );
		CqVector3D p4( Point.x() - radius, Point.y() + radius, Point.z() );
		
		CqMicroPolygonStatic *pNew = new CqMicroPolygonStatic();
		pNew->SetGrid( this );
		pNew->SetIndex( iu );
		pNew->Initialise( p1, p2, p3, p4 );
		pNew->GetTotalBound( TqTrue );

		pImage->AddMPG( pNew );
	}

	Release();
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------


