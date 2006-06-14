// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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

/**
        \file
        \brief Implements the classes and support structures for 
                handling RenderMan Curves primitives.
        \author Jonathan Merritt (j.merritt@pgrad.unimelb.edu.au)
*/

#include <stdio.h>
#include <string.h>
#include "aqsis.h"
#include "imagebuffer.h"
#include "micropolygon.h"
#include "renderer.h"
#include "patch.h"
#include "vector2d.h"
#include "vector3d.h"
#include "curves.h"
START_NAMESPACE( Aqsis )


static TqUlong hwidth = CqString::hash("width");
static TqUlong hcwidth = CqString::hash("constantwidth");
static TqUlong hp = CqString::hash("P");
static TqUlong hu = CqString::hash("u");
static TqUlong hn = CqString::hash("N");
static TqUlong hv = CqString::hash("v");

/**
 * CqCurve constructor.
 */
CqCurve::CqCurve() : CqSurface()
{
	m_widthParamIndex = -1;
	m_constantwidthParamIndex = -1;
	m_splitDecision = Split_Undecided;

	STATS_INC( GPR_crv );
}



/**
 * CqCurve copy constructor.
 */
/* CqCurve::CqCurve( const CqCurve &from ) : CqSurface()
 * {
 * 	( *this ) = from;
 * 
 * 	STATS_INC( GPR_crv );
 * }
 */



/**
 * CqCurve destructor.
 */
CqCurve::~CqCurve()
{ }


/**
 * Adds a primitive variable to the list of user parameters.  This method
 * caches the indexes of the "width" and "constantwidth" parameters within
 * the array of user parameters for later access.
 *
 * @param pParam        Pointer to the parameter to add.
 */
void CqCurve::AddPrimitiveVariable( CqParameter* pParam )
{

	// add the primitive variable using the superclass method
	CqSurface::AddPrimitiveVariable( pParam );

	// trap the indexes of "width" and "constantwidth" parameters
	if ( pParam->hash() == hwidth )
	{
		assert( m_widthParamIndex == -1 );
		m_widthParamIndex = m_aUserParams.size() - 1;
	}
	else if ( pParam->hash() == hcwidth )
	{
		assert( m_constantwidthParamIndex == -1 );
		m_constantwidthParamIndex = m_aUserParams.size() - 1;
	}

}



/**
 * Calculates bounds for a CqCurve.
 *
 * NOTE: This method makes the same assumptions as 
 * CqSurfacePatchBicubic::Bound() does about the convex-hull property of the
 * curve.  This is fine most of the time, but the user can specify basis
 * matrices like Catmull-Rom, which are non-convex.
 *
 * FIXME: Make sure that all hulls which reach this method are convex!
 *
 * @return CqBound object containing the bounds.
 */
CqBound CqCurve::Bound() const
{

	// Get the boundary in camera space.
	CqVector3D vecA( FLT_MAX, FLT_MAX, FLT_MAX );
	CqVector3D vecB( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqFloat maxCameraSpaceWidth = 0;
	TqUint nWidthParams = cVarying();
	for ( TqUint i = 0; i < ( *P() ).Size(); i++ )
	{
		// expand the boundary if necessary to accomodate the
		//  current vertex
		CqVector3D vecV = P()->pValue( i )[0];
		if ( vecV.x() < vecA.x() )
			vecA.x( vecV.x() );
		if ( vecV.y() < vecA.y() )
			vecA.y( vecV.y() );
		if ( vecV.x() > vecB.x() )
			vecB.x( vecV.x() );
		if ( vecV.y() > vecB.y() )
			vecB.y( vecV.y() );
		if ( vecV.z() < vecA.z() )
			vecA.z( vecV.z() );
		if ( vecV.z() > vecB.z() )
			vecB.z( vecV.z() );

		// increase the maximum camera space width of the curve if
		//  necessary
		if ( i < nWidthParams )
		{
			TqFloat camSpaceWidth = width()->pValue( i )[0];
			if ( camSpaceWidth > maxCameraSpaceWidth )
			{
				maxCameraSpaceWidth = camSpaceWidth;
			}
		}

	}

	// increase the size of the boundary by half the width of the
	//  curve in camera space
	vecA -= ( maxCameraSpaceWidth / 2.0 );
	vecB += ( maxCameraSpaceWidth / 2.0 );

	// return the boundary
	CqBound	B;
	B.vecMin() = vecA;
	B.vecMax() = vecB;
	return ( AdjustBoundForTransformationMotion( B ) );
}



/**
 * CqCurve CloneData function
 *
 */
void CqCurve::CloneData(CqCurve* clone) const
{
	CqSurface::CloneData(clone);
	clone->m_widthParamIndex = m_widthParamIndex;
	clone->m_constantwidthParamIndex = m_constantwidthParamIndex;
}



/**
 * Returns the approximate "length" of an edge of a grid in raster space.
 *
 * @return Approximate grid length.
 */
TqFloat CqCurve::GetGridLength() const
{

	// we want to find the number of micropolygons per grid - the default
	//  is 256 (16x16 micropolygon grid).
	TqFloat micropolysPerGrid = 256;
	const TqInt* poptGridSize =
	    QGetRenderContext() ->poptCurrent()->GetIntegerOption(
	        "limits", "gridsize"
	    );
	if ( poptGridSize != NULL )
	{
		micropolysPerGrid =
		    static_cast<TqFloat>( poptGridSize[ 0 ] ) *
		    static_cast<TqFloat>( poptGridSize[ 1 ] );
	}

	// find the shading rate
	TqFloat ShadingRate = pAttributes() ->GetFloatAttribute(
	                          "System", "ShadingRate"
	                      ) [ 0 ];

	// we assume that the grids are square and take the square root to find
	//  the number of micropolygons along one side
	TqFloat mpgsAlongSide = sqrt( micropolysPerGrid );

	// now, the number of pixels (raster space length) taken up by one
	//  micropolygon is given by 1 / shading rate.  So, to find the length
	//  in raster space of the edge of the micropolygon grid, we divide its
	//  length (in micropolygons) by the shading rate
	return mpgsAlongSide / ShadingRate;

}



/**
 * Populates the "width" parameter if it is not already present (ie supplied
 * by the user).  The "width" is populated either by the value of
 * "constantwidth", or by the default object-space width 1.0.
 */
void CqCurve::PopulateWidth()
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
		widthP->pValue( i )[0] = widthvalue;
	}

	// add the width array to the curve as a primitive variable
	AddPrimitiveVariable( widthP );
}



/**
 * Sets the default primitive variables.
 *
 * @param bUseDef_st
 */
void CqCurve::SetDefaultPrimitiveVariables( TqBool bUseDef_st )
{
	// we don't want any default primitive variables.

	// s,t are set to u,v for curves
}



/**
 * CqLinearCurveSegment constructor.
 */
CqLinearCurveSegment::CqLinearCurveSegment() : CqCurve()
{ }



/**
 * CqLinearCurveSegment copy constructor.
 */
/* CqLinearCurveSegment::CqLinearCurveSegment( const CqLinearCurveSegment &from ) :
 * 		CqCurve()
 * {
 * 	( *this ) = from;
 * }
 */



/**
 * CqLinearCurveSegment destructor.
 */
CqLinearCurveSegment::~CqLinearCurveSegment()
{ }



/**
 * Create a clone of this curve surface
 *
 */
CqSurface* CqLinearCurveSegment::Clone() const
{
	CqLinearCurveSegment* clone = new CqLinearCurveSegment();
	CqCurve::CloneData( clone );
	return ( clone );
}




/**
 * Implements natural subdivision for this curve segment.
 *
 * @param pParam        Original parameter.
 * @param pParam1       First new parameter.
 * @param pParam2       Second new parameter.
 * @param u             true if the split is along u (should
 *                              always be false!)
 */
void CqLinearCurveSegment::NaturalSubdivide(
    CqParameter* pParam,
    CqParameter* pParam1, CqParameter* pParam2,
    TqBool u
)
{

	assert( u == false );
	switch ( pParam->Type() )
	{
			case type_float:
			{
				CqParameterTyped<TqFloat, TqFloat>* pTParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam );
				CqParameterTyped<TqFloat, TqFloat>* pTResult1 = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam1 );
				CqParameterTyped<TqFloat, TqFloat>* pTResult2 = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_integer:
			{
				CqParameterTyped<TqInt, TqFloat>* pTParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam );
				CqParameterTyped<TqInt, TqFloat>* pTResult1 = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam1 );
				CqParameterTyped<TqInt, TqFloat>* pTResult2 = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_point:
			case type_vector:
			case type_normal:
			{
				CqParameterTyped<CqVector3D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam );
				CqParameterTyped<CqVector3D, CqVector3D>* pTResult1 = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam1 );
				CqParameterTyped<CqVector3D, CqVector3D>* pTResult2 = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_hpoint:
			{
				CqParameterTyped<CqVector4D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam );
				CqParameterTyped<CqVector4D, CqVector3D>* pTResult1 = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam1 );
				CqParameterTyped<CqVector4D, CqVector3D>* pTResult2 = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}


			case type_color:
			{
				CqParameterTyped<CqColor, CqColor>* pTParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam );
				CqParameterTyped<CqColor, CqColor>* pTResult1 = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam1 );
				CqParameterTyped<CqColor, CqColor>* pTResult2 = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_string:
			{
				CqParameterTyped<CqString, CqString>* pTParam = static_cast<CqParameterTyped<CqString, CqString>*>( pParam );
				CqParameterTyped<CqString, CqString>* pTResult1 = static_cast<CqParameterTyped<CqString, CqString>*>( pParam1 );
				CqParameterTyped<CqString, CqString>* pTResult2 = static_cast<CqParameterTyped<CqString, CqString>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_matrix:
			{
				//			CqParameterTyped<CqMatrix, CqMatrix>* pTParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam );
				//			CqParameterTyped<CqMatrix, CqMatrix>* pTResult1 = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam1 );
				//			CqParameterTyped<CqMatrix, CqMatrix>* pTResult2 = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam2 );
				//			TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				//			break;
			}

			default:
			{
				break;
			}
	}

}



/**
 * Splits a CqLinearCurveSegment into either two smaller segments or a
 * patch.
 *
 * @param aSplits       Vector to store the split objects in.
 *
 * @return      The number of objects we've created.
 */
TqInt CqLinearCurveSegment::Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
{
	// Split based on the decision
	switch( m_splitDecision )
	{
			case Split_Patch:
			{
				// split into a patch
				TqInt cPatches = SplitToPatch( aSplits );
				STATS_INC( GEO_crv_splits );
				STATS_INC( GEO_crv_patch );
				STATS_SETI( GEO_crv_patch_created, STATS_GETI( GEO_crv_patch_created ) + cPatches );

				return cPatches;
			}

			case Split_Curve:
			{
				// split into smaller curves
				TqInt cCurves = SplitToCurves( aSplits );
				STATS_INC( GEO_crv_splits );
				STATS_INC( GEO_crv_crv );
				STATS_SETI( GEO_crv_crv_created, STATS_GETI( GEO_crv_crv_created ) + cCurves );

				return cCurves;
			}

			default:
			throw;
	}
}



/**
 * Splits a linear curve segment into two smaller curves.
 *
 * @param aSplits       Vector of split surfaces to add the segment to.
 *
 * @return Number of created objects.
 */
TqInt CqLinearCurveSegment::SplitToCurves(
    std::vector<boost::shared_ptr<CqSurface> >& aSplits
)
{

	// split into more curves
	//  This bit right here looks a lot like CqSurface::Split().
	//  The difference is that we *don't* want the default splitter
	//  to handle varying class variables because it inconveniently
	//  sets them up to have 4 elements.

	aSplits.push_back( boost::shared_ptr<CqSurface>( new CqLinearCurveSegment ) );
	aSplits.push_back( boost::shared_ptr<CqSurface>( new CqLinearCurveSegment ) );

	aSplits[ 0 ] ->SetSurfaceParameters( *this );
	aSplits[ 0 ] ->SetEyeSplitCount( EyeSplitCount() );

	aSplits[ 1 ] ->SetSurfaceParameters( *this );
	aSplits[ 1 ] ->SetEyeSplitCount( EyeSplitCount() );

	// Iterate through any user parameters, subdividing and storing
	//  the second value in the target surface.
	std::vector<CqParameter*>::iterator iUP;
	for ( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{

		// clone the parameters
		CqParameter* pNewA = ( *iUP ) ->Clone();
		CqParameter* pNewB = ( *iUP ) ->Clone();

		// let the standard system handle all but varying class
		//  primitive variables
		if ( ( *iUP ) ->Class() == class_varying )
		{
			// for varying class variables, we want to
			//  handle them the same way as vertex class
			//  variables for the simple case of a
			//  CqSingleCurveLinear
			NaturalSubdivide( ( *iUP ), pNewA, pNewB, TqFalse );
		}
		else
		{
			( *iUP ) ->Subdivide( pNewA, pNewB, false, this );
		}

		static_cast<CqSurface*>( aSplits[ 0 ].get() ) -> AddPrimitiveVariable( pNewA );
		static_cast<CqSurface*>( aSplits[ 1 ].get() ) -> AddPrimitiveVariable( pNewB );

	}

	return 2;

}



/**
 * Converts a linear curve segment into a patch for rendering.
 *
 * @param aSplits       Vector of split surfaces to add the segment to.
 *
 * @return Number of created objects.
 */
TqInt CqLinearCurveSegment::SplitToPatch(
    std::vector<boost::shared_ptr<CqSurface> >& aSplits
)
{

	// first, we find the following vectors:
	//  direction     - from the first point to the second along the line
	//                      segment
	//  normal0       - normal at the first point
	//  normal1       - normal at the second point
	//  widthOffset0  - offset to account for the width of the patch at
	//                      the first point
	//  widthOffset1  - offset to account for the width of the patch at
	//                      the second point
	CqVector3D direction = P()->pValue( 1 )[0] - P()->pValue( 0 )[0];
	CqVector3D normal0, normal1;
	GetNormal( 0, normal0 );
	GetNormal( 1, normal1 );
	normal0.Unit();
	normal1.Unit();
	CqVector3D widthOffset0 = normal0 % direction;
	CqVector3D widthOffset1 = normal1 % direction;
	widthOffset0 *=
	    width()->pValue( 0 )[0] / widthOffset0.Magnitude() / 2.0;
	widthOffset1 *=
	    width()->pValue( 1 )[0] / widthOffset1.Magnitude() / 2.0;

	// next, we create the bilinear patch
	boost::shared_ptr<CqSurfacePatchBilinear> pPatch( new CqSurfacePatchBilinear() );
	pPatch->SetSurfaceParameters( *this );
	pPatch->SetDefaultPrimitiveVariables();

	// set the points on the patch
	pPatch->AddPrimitiveVariable(
	    new CqParameterTypedVertex <
	    CqVector4D, type_hpoint, CqVector3D
	    > ( "P", 1 )
	);
	pPatch->P() ->SetSize( 4 );
	pPatch->P()->pValue( 0 )[0] = static_cast<CqVector3D>( P()->pValue( 0 )[0] ) + widthOffset0;
	pPatch->P()->pValue( 1 )[0] = static_cast<CqVector3D>( P()->pValue( 0 )[0] ) - widthOffset0;
	pPatch->P()->pValue( 2 )[0] = static_cast<CqVector3D>( P()->pValue( 1 )[0] ) + widthOffset1;
	pPatch->P()->pValue( 3 )[0] = static_cast<CqVector3D>( P()->pValue( 1 )[0] ) - widthOffset1;

	// set the normals on the patch
	/*    pPatch->AddPrimitiveVariable(
	        new CqParameterTypedVertex <
	        CqVector3D, type_normal, CqVector3D
	        > ( "N", 0 )
	    );
	    pPatch->N() ->SetSize( 4 );
	    pPatch->N()->pValue( 0 )[0] = pPatch->N()->pValue( 1 )[0] = normal0;
	    pPatch->N()->pValue( 2 )[0] = pPatch->N()->pValue( 3 )[0] = normal1;
	*/
	TqInt bUses = Uses();

	// set u, v coordinates of the patch
	if ( USES( bUses, EnvVars_u ) || USES( bUses, EnvVars_v ) )
	{
		pPatch->u()->pValue( 0 )[0] = pPatch->u()->pValue( 2 )[0] = 0.0;
		pPatch->u()->pValue( 1 )[0] = pPatch->u()->pValue( 3 )[0] = 1.0;
		pPatch->v()->pValue( 0 )[0] = pPatch->v()->pValue( 1 )[0] = v()->pValue( 0 )[0];
		pPatch->v()->pValue( 2 )[0] = pPatch->v()->pValue( 3 )[0] = v()->pValue( 1 )[0];
	}

	// helllllp!!! WHAT DO I DO WITH s,t!!!???
	//  for now, they're set equal to u and v
	if ( USES( bUses, EnvVars_s ) || USES( bUses, EnvVars_t ) )
	{
		pPatch->s()->pValue( 0 )[0] = pPatch->s()->pValue( 2 )[0] = 0.0;
		pPatch->s()->pValue( 1 )[0] = pPatch->s()->pValue( 3 )[0] = 1.0;
		pPatch->t()->pValue( 0 )[0] = pPatch->t()->pValue( 1 )[0] = v()->pValue( 0 )[0];
		pPatch->t()->pValue( 2 )[0] = pPatch->t()->pValue( 3 )[0] = v()->pValue( 1 )[0];
	}

	// set any remaining user parameters
	std::vector<CqParameter*>::iterator iUP;
	for ( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		if (
		    ( ( *iUP ) ->hash() != hp ) &&
		    ( ( *iUP ) ->hash() != hn ) &&
		    ( ( *iUP ) ->hash() != hu ) &&
		    ( ( *iUP ) ->hash() != hv )
		)
		{

			if (
			    ( ( *iUP ) ->Class() == class_varying ) ||
			    ( ( *iUP ) ->Class() == class_vertex )
			)
			{
				// copy "varying" or "vertex" class
				//  variables
				CqParameter * pNewUP =
				    ( *iUP ) ->CloneType(
				        ( *iUP ) ->strName().c_str(),
				        ( *iUP ) ->Count()
				    );
				assert(
				    pPatch->cVarying() ==
				    pPatch->cVertex()
				);
				pNewUP->SetSize( pPatch->cVarying() );

				pNewUP->SetValue( ( *iUP ), 0, 0 );
				pNewUP->SetValue( ( *iUP ), 1, 0 );
				pNewUP->SetValue( ( *iUP ), 2, 1 );
				pNewUP->SetValue( ( *iUP ), 3, 1 );
				pPatch->AddPrimitiveVariable( pNewUP );

			}
			else if (
			    ( ( *iUP ) ->Class() == class_uniform ) ||
			    ( ( *iUP ) ->Class() == class_constant )
			)
			{

				// copy "uniform" or "constant" class variables
				CqParameter * pNewUP =
				    ( *iUP ) ->CloneType(
				        ( *iUP ) ->strName().c_str(),
				        ( *iUP ) ->Count()
				    );
				assert( pPatch->cUniform() == 1 );
				pNewUP->SetSize( pPatch->cUniform() );

				pNewUP->SetValue( ( *iUP ), 0, 0 );
				pPatch->AddPrimitiveVariable( pNewUP );
			}
		}
	}

	// add the patch to the split surfaces vector
	aSplits.push_back( pPatch );

	return 1;
}



/**
 * CqCubicCurveSegment constructor.
 */
CqCubicCurveSegment::CqCubicCurveSegment() : CqCurve()
{ }



/**
 * CqCubicCurveSegment copy constructor.
 */
/* CqCubicCurveSegment::CqCubicCurveSegment( const CqCubicCurveSegment &from )
 * 		: CqCurve()
 * {
 * 	( *this ) = from;
 * }
 */



/**
 * CqCubicCurveSegment destructor.
 */
CqCubicCurveSegment::~CqCubicCurveSegment()
{ }



/**
 * Create a clone of this curve surface
 *
 */
CqSurface* CqCubicCurveSegment::Clone() const
{
	CqCubicCurveSegment* clone = new CqCubicCurveSegment();
	CqCurve::CloneData( clone );
	return ( clone );
}



/**
 * Implements natural subdivision for this curve segment.
 *
 * @param pParam        Original parameter.
 * @param pParam1       First new parameter.
 * @param pParam2       Second new parameter.
 * @param u             true if the split is along u (should
 *                              always be false!)
 */
void CqCubicCurveSegment::NaturalSubdivide(
    CqParameter* pParam,
    CqParameter* pParam1, CqParameter* pParam2,
    TqBool u
)
{

	assert( u == false );
	switch ( pParam->Type() )
	{
			case type_float:
			{
				CqParameterTyped<TqFloat, TqFloat>* pTParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam );
				CqParameterTyped<TqFloat, TqFloat>* pTResult1 = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam1 );
				CqParameterTyped<TqFloat, TqFloat>* pTResult2 = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_integer:
			{
				CqParameterTyped<TqInt, TqFloat>* pTParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam );
				CqParameterTyped<TqInt, TqFloat>* pTResult1 = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam1 );
				CqParameterTyped<TqInt, TqFloat>* pTResult2 = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_point:
			case type_vector:
			case type_normal:
			{
				CqParameterTyped<CqVector3D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam );
				CqParameterTyped<CqVector3D, CqVector3D>* pTResult1 = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam1 );
				CqParameterTyped<CqVector3D, CqVector3D>* pTResult2 = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_hpoint:
			{
				CqParameterTyped<CqVector4D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam );
				CqParameterTyped<CqVector4D, CqVector3D>* pTResult1 = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam1 );
				CqParameterTyped<CqVector4D, CqVector3D>* pTResult2 = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}


			case type_color:
			{
				CqParameterTyped<CqColor, CqColor>* pTParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam );
				CqParameterTyped<CqColor, CqColor>* pTResult1 = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam1 );
				CqParameterTyped<CqColor, CqColor>* pTResult2 = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_string:
			{
				CqParameterTyped<CqString, CqString>* pTParam = static_cast<CqParameterTyped<CqString, CqString>*>( pParam );
				CqParameterTyped<CqString, CqString>* pTResult1 = static_cast<CqParameterTyped<CqString, CqString>*>( pParam1 );
				CqParameterTyped<CqString, CqString>* pTResult2 = static_cast<CqParameterTyped<CqString, CqString>*>( pParam2 );
				TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_matrix:
			{
				//			CqParameterTyped<CqMatrix, CqMatrix>* pTParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam );
				//			CqParameterTyped<CqMatrix, CqMatrix>* pTResult1 = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam1 );
				//			CqParameterTyped<CqMatrix, CqMatrix>* pTResult2 = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam2 );
				//			TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				//			break;
			}

			default:
			{
				break;
			}
	}

}


/**
 * Implements natural subdivision for this curve segment.
 *
 * @param pParam        Original parameter.
 * @param pParam1       First new parameter.
 * @param pParam2       Second new parameter.
 * @param u             true if the split is along u (should
 *                              always be false!)
 */
void CqCubicCurveSegment::VaryingNaturalSubdivide(
    CqParameter* pParam,
    CqParameter* pParam1, CqParameter* pParam2,
    TqBool u
)
{

	assert( u == false );
	switch ( pParam->Type() )
	{
			case type_float:
			{
				CqParameterTyped<TqFloat, TqFloat>* pTParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam );
				CqParameterTyped<TqFloat, TqFloat>* pTResult1 = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam1 );
				CqParameterTyped<TqFloat, TqFloat>* pTResult2 = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam2 );
				VaryingTypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_integer:
			{
				CqParameterTyped<TqInt, TqFloat>* pTParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam );
				CqParameterTyped<TqInt, TqFloat>* pTResult1 = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam1 );
				CqParameterTyped<TqInt, TqFloat>* pTResult2 = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam2 );
				VaryingTypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_point:
			case type_vector:
			case type_normal:
			{
				CqParameterTyped<CqVector3D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam );
				CqParameterTyped<CqVector3D, CqVector3D>* pTResult1 = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam1 );
				CqParameterTyped<CqVector3D, CqVector3D>* pTResult2 = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam2 );
				VaryingTypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_hpoint:
			{
				CqParameterTyped<CqVector4D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam );
				CqParameterTyped<CqVector4D, CqVector3D>* pTResult1 = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam1 );
				CqParameterTyped<CqVector4D, CqVector3D>* pTResult2 = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam2 );
				VaryingTypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}


			case type_color:
			{
				CqParameterTyped<CqColor, CqColor>* pTParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam );
				CqParameterTyped<CqColor, CqColor>* pTResult1 = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam1 );
				CqParameterTyped<CqColor, CqColor>* pTResult2 = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam2 );
				VaryingTypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_string:
			{
				CqParameterTyped<CqString, CqString>* pTParam = static_cast<CqParameterTyped<CqString, CqString>*>( pParam );
				CqParameterTyped<CqString, CqString>* pTResult1 = static_cast<CqParameterTyped<CqString, CqString>*>( pParam1 );
				CqParameterTyped<CqString, CqString>* pTResult2 = static_cast<CqParameterTyped<CqString, CqString>*>( pParam2 );
				VaryingTypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				break;
			}

			case type_matrix:
			{
				//			CqParameterTyped<CqMatrix, CqMatrix>* pTParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam );
				//			CqParameterTyped<CqMatrix, CqMatrix>* pTResult1 = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam1 );
				//			CqParameterTyped<CqMatrix, CqMatrix>* pTResult2 = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam2 );
				//			TypedNaturalSubdivide( pTParam, pTResult1, pTResult2, u );
				//			break;
			}

			default:
			{
				break;
			}
	}

}


/**
 * Splits a CqCubicCurveSegment into either two smaller segments or a
 * patch.
 *
 * @param aSplits       Vector to store the split objects in.
 *
 * @return      The number of objects we've created.
 */
TqInt CqCubicCurveSegment::Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
{
	// Split based on the decision
	switch( m_splitDecision )
	{
			case Split_Patch:
			{
				// split into a patch
				TqInt cPatches = SplitToPatch( aSplits );
				STATS_INC( GEO_crv_splits );
				STATS_INC( GEO_crv_patch );
				STATS_SETI( GEO_crv_patch_created, STATS_GETI( GEO_crv_patch_created ) + cPatches );

				return cPatches;
			}
			case Split_Curve:
			{
				// split into smaller curves
				TqInt cCurves = SplitToCurves( aSplits );
				STATS_INC( GEO_crv_splits );
				STATS_INC( GEO_crv_crv );
				STATS_SETI( GEO_crv_crv_created, STATS_GETI( GEO_crv_crv_created ) + cCurves );

				return cCurves;
			}
			default:
			throw;
	}
}



/**
 * Splits a cubic curve segment into two smaller curves.
 *
 * @param aSplits       Vector of split surfaces to add the segment to.
 *
 * @return Number of created objects.
 */
TqInt CqCubicCurveSegment::SplitToCurves(
    std::vector<boost::shared_ptr<CqSurface> >& aSplits
)
{

	// split into more curves
	//  This bit right here looks a lot like CqSurface::Split().
	//  The difference is that we *don't* want the default splitter
	//  to handle varying class variables because it inconveniently
	//  sets them up to have 4 elements.

	aSplits.push_back( boost::shared_ptr<CqSurface>( new CqCubicCurveSegment ) );
	aSplits.push_back( boost::shared_ptr<CqSurface>( new CqCubicCurveSegment ) );

	aSplits[ 0 ] ->SetSurfaceParameters( *this );
	aSplits[ 0 ] ->SetEyeSplitCount( EyeSplitCount() );

	aSplits[ 1 ] ->SetSurfaceParameters( *this );
	aSplits[ 1 ] ->SetEyeSplitCount( EyeSplitCount() );

	// Iterate through any user parameters, subdividing and storing
	//  the second value in the target surface.
	std::vector<CqParameter*>::iterator iUP;
	for ( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{

		// clone the parameters
		CqParameter* pNewA = ( *iUP ) ->Clone();
		CqParameter* pNewB = ( *iUP ) ->Clone();

		// let the standard system handle all but varying class
		//  primitive variables
		if ( ( *iUP ) ->Class() == class_varying )
		{
			// for varying class variables, we want to
			//  handle them the same way as vertex class
			//  variables for the simple case of a
			//  CqSingleCurveLinear
			VaryingNaturalSubdivide( ( *iUP ), pNewA, pNewB, TqFalse );
		}
		else
		{
			( *iUP ) ->Subdivide( pNewA, pNewB, false, this );
		}

		static_cast<CqSurface*>( aSplits[ 0 ].get() ) -> AddPrimitiveVariable( pNewA );
		static_cast<CqSurface*>( aSplits[ 1 ].get() ) -> AddPrimitiveVariable( pNewB );

	}

	return 2;

}


CqVector3D	CqCubicCurveSegment::CalculateTangent(TqFloat u)
{
	int i;
	std::vector<CqVector4D> pg(4), pg0(4);
	for(i=0; i <= 3; i++)
		pg[i] = pg0[i] =*P()->pValue( i );
	for(int j=1; j <= 3; j++)
	{
		for(int i=0; i <= 3-j; i++)
		{
			pg0[i]=pg[i];
			pg[i]=pg[i]*(1-u)+pg[i+1]*u;
		}
	}
	return(3*(pg[1]-pg0[0]));
}



/**
 * Converts a linear curve segment into a patch for rendering.
 *
 * @param aSplits       Vector of split surfaces to add the segment to.
 *
 * @return Number of created objects.
 */
TqInt CqCubicCurveSegment::SplitToPatch(
    std::vector<boost::shared_ptr<CqSurface> >& aSplits
)
{

	// first, we find the following vectors:
	//  direction     - from the first point to the second along the line
	//                      segment
	//  normal0       - normal at the first point
	//  normal1       - normal at the second point
	//  widthOffset0  - offset to account for the width of the patch at
	//                      the first point
	//  widthOffset1  - offset to account for the width of the patch at
	//                      the second point

	// \note: Not really happy about this, but by shifting the calculation value
	// slightly along the curve for tangent calculation of endpoints, we avoid
	// problems with curves that have duplicated points at one or other ends.
	// See bug #1102605
	CqVector3D direction0 = CalculateTangent(0.01);
	CqVector3D direction3 = CalculateTangent(0.99);

	CqVector3D direction1 = CalculateTangent(0.333);
	CqVector3D direction2 = CalculateTangent(0.666);

	CqVector3D normal0, normal1, normal2, normal3;
	GetNormal( 0, normal0 );
	GetNormal( 1, normal3 );
	normal1 = ( ( normal3 - normal0 ) / 3.0f ) + normal0;
	normal2 = ( ( ( normal3 - normal0 ) / 3.0f ) * 2.0f ) + normal0;

	normal0.Unit();
	normal1.Unit();
	normal2.Unit();
	normal3.Unit();

	CqVector3D widthOffset02 = normal0 % direction0;
	CqVector3D widthOffset12 = normal1 % direction1;
	CqVector3D widthOffset22 = normal2 % direction2;
	CqVector3D widthOffset32 = normal3 % direction3;

	TqFloat width0 = width()->pValue( 0 )[0];
	TqFloat width3 = width()->pValue( 1 )[0];
	TqFloat width1 = ( ( width3 - width0 ) / 3.0f ) + width0;
	TqFloat width2 = ( ( ( width3 - width0 ) / 3.0f ) * 2.0f ) + width0;

	widthOffset02 *=
	    width0 / widthOffset02.Magnitude() / 6.0;
	widthOffset12 *=
	    width1 / widthOffset12.Magnitude() / 6.0;
	widthOffset22 *=
	    width2 / widthOffset22.Magnitude() / 6.0;
	widthOffset32 *=
	    width3 / widthOffset32.Magnitude() / 6.0;

	CqVector3D widthOffset0 = widthOffset02 * 3;
	CqVector3D widthOffset1 = widthOffset12 * 3;
	CqVector3D widthOffset2 = widthOffset22 * 3;
	CqVector3D widthOffset3 = widthOffset32 * 3;

	// next, we create the bilinear patch
	boost::shared_ptr<CqSurfacePatchBicubic> pPatch( new CqSurfacePatchBicubic() );
	pPatch->SetSurfaceParameters( *this );
	pPatch->SetDefaultPrimitiveVariables();

	// set the points on the patch
	pPatch->AddPrimitiveVariable(
	    new CqParameterTypedVertex <
	    CqVector4D, type_hpoint, CqVector3D
	    > ( "P", 1 )
	);
	pPatch->P() ->SetSize( 16 );
	pPatch->P()->pValue( 0  )[0] = static_cast<CqVector3D>( P()->pValue( 0 )[0] ) + widthOffset0;
	pPatch->P()->pValue( 1  )[0] = static_cast<CqVector3D>( P()->pValue( 0 )[0] ) + widthOffset02;
	pPatch->P()->pValue( 2  )[0] = static_cast<CqVector3D>( P()->pValue( 0 )[0] ) - widthOffset02;
	pPatch->P()->pValue( 3  )[0] = static_cast<CqVector3D>( P()->pValue( 0 )[0] ) - widthOffset0;

	pPatch->P()->pValue( 4  )[0] = static_cast<CqVector3D>( P()->pValue( 1 )[0] ) + widthOffset1;
	pPatch->P()->pValue( 5  )[0] = static_cast<CqVector3D>( P()->pValue( 1 )[0] ) + widthOffset12;
	pPatch->P()->pValue( 6  )[0] = static_cast<CqVector3D>( P()->pValue( 1 )[0] ) - widthOffset12;
	pPatch->P()->pValue( 7  )[0] = static_cast<CqVector3D>( P()->pValue( 1 )[0] ) - widthOffset1;

	pPatch->P()->pValue( 8  )[0] = static_cast<CqVector3D>( P()->pValue( 2 )[0] ) + widthOffset2;
	pPatch->P()->pValue( 9  )[0] = static_cast<CqVector3D>( P()->pValue( 2 )[0] ) + widthOffset22;
	pPatch->P()->pValue( 10 )[0] = static_cast<CqVector3D>( P()->pValue( 2 )[0] ) - widthOffset22;
	pPatch->P()->pValue( 11 )[0] = static_cast<CqVector3D>( P()->pValue( 2 )[0] ) - widthOffset2;

	pPatch->P()->pValue( 12 )[0] = static_cast<CqVector3D>( P()->pValue( 3 )[0] ) + widthOffset3;
	pPatch->P()->pValue( 13 )[0] = static_cast<CqVector3D>( P()->pValue( 3 )[0] ) + widthOffset32;
	pPatch->P()->pValue( 14 )[0] = static_cast<CqVector3D>( P()->pValue( 3 )[0] ) - widthOffset32;
	pPatch->P()->pValue( 15 )[0] = static_cast<CqVector3D>( P()->pValue( 3 )[0] ) - widthOffset3;

	// set the normals on the patch
	//	pPatch->AddPrimitiveVariable(
	//	    new CqParameterTypedVertex <
	//	    CqVector3D, type_normal, CqVector3D
	//	    > ( "N", 0 )
	//	);
	//	pPatch->N() ->SetSize( 16 );
	//	( *pPatch->N() ) [ 0  ] = ( *pPatch->N() ) [ 1  ] = ( *pPatch->N() ) [ 2  ] = ( *pPatch->N() ) [ 3  ] = normal0;
	//	( *pPatch->N() ) [ 4  ] = ( *pPatch->N() ) [ 5  ] = ( *pPatch->N() ) [ 6  ] = ( *pPatch->N() ) [ 7  ] = normal1;
	//	( *pPatch->N() ) [ 8  ] = ( *pPatch->N() ) [ 9  ] = ( *pPatch->N() ) [ 10 ] = ( *pPatch->N() ) [ 11 ] = normal2;
	//	( *pPatch->N() ) [ 12 ] = ( *pPatch->N() ) [ 13 ] = ( *pPatch->N() ) [ 14 ] = ( *pPatch->N() ) [ 15 ] = normal3;

	TqInt bUses = Uses();

	// set u, v coordinates of the patch
	if ( USES( bUses, EnvVars_u ) || USES( bUses, EnvVars_v ) )
	{
		pPatch->u()->pValue( 0 )[0] = pPatch->u()->pValue( 2 )[0] = 0.0;
		pPatch->u()->pValue( 1 )[0] = pPatch->u()->pValue( 3 )[0] = 1.0;
		pPatch->v()->pValue( 0 )[0] = pPatch->v()->pValue( 1 )[0] = v()->pValue( 0 )[0];
		pPatch->v()->pValue( 2 )[0] = pPatch->v()->pValue( 3 )[0] = v()->pValue( 1 )[0];
	}

	// helllllp!!! WHAT DO I DO WITH s,t!!!???
	//  for now, they're set equal to u and v
	if ( USES( bUses, EnvVars_s ) || USES( bUses, EnvVars_t ) )
	{
		pPatch->s()->pValue( 0 )[0] = pPatch->s()->pValue( 2 )[0] = 0.0;
		pPatch->s()->pValue( 1 )[0] = pPatch->s()->pValue( 3 )[0] = 1.0;
		pPatch->t()->pValue( 0 )[0] = pPatch->t()->pValue( 1 )[0] = v()->pValue( 0 )[0];
		pPatch->t()->pValue( 2 )[0] = pPatch->t()->pValue( 3 )[0] = v()->pValue( 1 )[0];
	}

	// set any remaining user parameters
	std::vector<CqParameter*>::iterator iUP;
	for ( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		if (
		    ( ( *iUP ) ->hash() != hp ) &&
		    ( ( *iUP ) ->hash() != hn ) &&
		    ( ( *iUP ) ->hash() != hu ) &&
		    ( ( *iUP ) ->hash() != hv )
		)
		{

			if ( ( *iUP ) ->Class() == class_vertex )
			{
				// copy "vertex" class variables
				CqParameter * pNewUP =
				    ( *iUP ) ->CloneType(
				        ( *iUP ) ->strName().c_str(),
				        ( *iUP ) ->Count()
				    );
				pNewUP->SetSize( pPatch->cVertex() );

				pNewUP->SetValue( ( *iUP ), 0,  0 );
				pNewUP->SetValue( ( *iUP ), 1,  0 );
				pNewUP->SetValue( ( *iUP ), 2,  0 );
				pNewUP->SetValue( ( *iUP ), 3,  0 );
				pNewUP->SetValue( ( *iUP ), 4,  1 );
				pNewUP->SetValue( ( *iUP ), 5,  1 );
				pNewUP->SetValue( ( *iUP ), 6,  1 );
				pNewUP->SetValue( ( *iUP ), 7,  1 );
				pNewUP->SetValue( ( *iUP ), 8,  2 );
				pNewUP->SetValue( ( *iUP ), 9,  2 );
				pNewUP->SetValue( ( *iUP ), 10, 2 );
				pNewUP->SetValue( ( *iUP ), 11, 2 );
				pNewUP->SetValue( ( *iUP ), 12, 3 );
				pNewUP->SetValue( ( *iUP ), 13, 3 );
				pNewUP->SetValue( ( *iUP ), 14, 3 );
				pNewUP->SetValue( ( *iUP ), 15, 3 );
				pPatch->AddPrimitiveVariable( pNewUP );

			}
			else if ( ( *iUP ) ->Class() == class_varying )
			{
				// copy "varying" class variables
				CqParameter * pNewUP =
				    ( *iUP ) ->CloneType(
				        ( *iUP ) ->strName().c_str(),
				        ( *iUP ) ->Count()
				    );
				pNewUP->SetSize( pPatch->cVarying() );

				pNewUP->SetValue( ( *iUP ), 0, 0 );
				pNewUP->SetValue( ( *iUP ), 1, 0 );
				pNewUP->SetValue( ( *iUP ), 2, 1 );
				pNewUP->SetValue( ( *iUP ), 3, 1 );
				pPatch->AddPrimitiveVariable( pNewUP );

			}
			else if (
			    ( ( *iUP ) ->Class() == class_uniform ) ||
			    ( ( *iUP ) ->Class() == class_constant )
			)
			{

				// copy "uniform" or "constant" class variables
				CqParameter * pNewUP =
				    ( *iUP ) ->CloneType(
				        ( *iUP ) ->strName().c_str(),
				        ( *iUP ) ->Count()
				    );
				assert( pPatch->cUniform() == 1 );
				pNewUP->SetSize( pPatch->cUniform() );

				pNewUP->SetValue( ( *iUP ), 0, 0 );
				pPatch->AddPrimitiveVariable( pNewUP );
			}
		}
	}

	// add the patch to the split surfaces vector
	aSplits.push_back( pPatch );

	return 1;
}


//---------------------------------------------------------------------
/** Convert from the current basis into Bezier for processing.
 *
 *  \param matBasis	Basis to convert from.
 */

void CqCubicCurveSegment::ConvertToBezierBasis( CqMatrix& matBasis )
{
	static CqMatrix matMim1;
	TqInt i, j;

	if ( matMim1.fIdentity() )
	{
		for ( i = 0; i < 4; i++ )
			for ( j = 0; j < 4; j++ )
				matMim1[ i ][ j ] = RiBezierBasis[ i ][ j ];
		matMim1.SetfIdentity( TqFalse );
		matMim1 = matMim1.Inverse();
	}

	CqMatrix matMj = matBasis;
	CqMatrix matConv = matMj * matMim1;

	CqMatrix matCP;
	for ( i = 0; i < 4; i++ )
	{
		matCP[ 0 ][ i ] = P()->pValue(i)[0].x();
		matCP[ 1 ][ i ] = P()->pValue(i)[0].y();
		matCP[ 2 ][ i ] = P()->pValue(i)[0].z();
		matCP[ 3 ][ i ] = P()->pValue(i)[0].h();
	}
	matCP.SetfIdentity( TqFalse );

	matCP = matConv.Transpose() * matCP;

	for ( i = 0; i < 4; i++ )
	{
		P()->pValue(i)[0].x( matCP[ 0 ][ i ] );
		P()->pValue(i)[0].y( matCP[ 1 ][ i ] );
		P()->pValue(i)[0].z( matCP[ 2 ][ i ] );
		P()->pValue(i)[0].h( matCP[ 3 ][ i ] );
	}
}


/**
 * CqCurvesGroup constructor.
 */
CqCurvesGroup::CqCurvesGroup() : CqCurve(), m_ncurves( 0 ), m_periodic( TqFalse ),
		m_nTotalVerts( 0 )
{ }



/**
 * CqCurvesGroup copy constructor.
 */
/* CqCurvesGroup::CqCurvesGroup( const CqCurvesGroup& from ) : CqCurve()
 * {
 * 	( *this ) = from;
 * }
 */



/**
 * CqCurvesGroup destructor.
 */
CqCurvesGroup::~CqCurvesGroup()
{ }



/**
 * Clone the data from this curve group onto the one specified.
 *
 */
void CqCurvesGroup::CloneData( CqCurvesGroup* clone ) const
{
	CqCurve::CloneData(clone);

	// copy members
	clone->m_ncurves = m_ncurves;
	clone->m_periodic = m_periodic;
	clone->m_nvertices = m_nvertices;
	clone->m_nTotalVerts = m_nTotalVerts;
}




/**
 * Constructor for a CqLinearCurvesGroup.
 *
 * @param ncurves       Number of curves in the group.
 * @param nvertices     Number of vertices per curve.
 * @param periodic      true if the curves in the group are periodic.
 */
CqLinearCurvesGroup::CqLinearCurvesGroup(
    TqInt ncurves, TqInt nvertices[], TqBool periodic
) : CqCurvesGroup()
{

	assert( nvertices != NULL );

	m_ncurves = ncurves;
	m_periodic = periodic;

	// it makes no sense to have a periodic curve group with a segment
	//  that has only two vertices - check for this just in case
	//  because the cVarying equations don't work; also add up the total
	//  number of vertices
	m_nTotalVerts = 0;
	TqInt i;
	for ( i = 0; i < m_ncurves; i++ )
	{
		m_nTotalVerts += nvertices[ i ];
		if ( ( nvertices[ i ] <= 2 ) && m_periodic )
		{
			Aqsis::log() << warning << "Periodic linear curves should have more than two vertices" << std::endl;
		}
	}

	// copy the array of numbers of vertices
	m_nvertices.clear();
	m_nvertices.reserve( m_ncurves );
	for ( i = 0; i < m_ncurves; i++ )
	{
		m_nvertices.push_back( nvertices[ i ] );
	}

}



/**
 * CqLinearCurvesGroup destructor.
 */
CqLinearCurvesGroup::~CqLinearCurvesGroup()
{
	m_nvertices.clear();
}



/**
 * Create a clone of this curve group.
 *
 */
CqSurface* CqLinearCurvesGroup::Clone() const
{
	CqLinearCurvesGroup* clone = new CqLinearCurvesGroup();
	CqCurvesGroup::CloneData( clone );

	return ( clone );
}




/**
 * Splits a CqLinearCurvesGroup object.
 *
 * The initial, naiive implementation here is immediately to split the group of
 * curves into CqLinearCurveSegment objects.  Perhaps a better way would be to
 * manage splitting of curve groups into other curve groups until they're of a
 * small enough size to become curve segments... ?
 *
 * @param aSplits       Vector of split objects.
 */
TqInt CqLinearCurvesGroup::Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
{

	TqInt nSplits = 0;      // number of splits we've done

	TqInt bUses = Uses();

	// create each linear curve, filling in its variables as we go
	TqInt vertexI = 0;      // keeps track of the current vertex index
	TqInt uniformI = 0;     // keeps track of the uniform param index
	// we process all the curves in the group...
	for ( TqInt curveI = 0; curveI < m_ncurves; curveI++ )
	{
		TqInt lastSegment;
		if ( m_periodic )
		{
			lastSegment = m_nvertices[ curveI ];
		}
		else
		{
			lastSegment = m_nvertices[ curveI ] - 1;
		}
		TqInt firstVertex = vertexI;

		// for each curve, we then process all its segments
		for ( TqInt segI = 0; segI < lastSegment; segI++ )
		{

			TqInt nextVertex;
			if ( segI == ( m_nvertices[ curveI ] - 1 ) )
			{
				nextVertex = firstVertex;
			}
			else
			{
				nextVertex = vertexI + 1;
			}

			// create the new CqLinearCurveSegment for the current
			//  curve segment
			boost::shared_ptr<CqLinearCurveSegment> pSeg( new CqLinearCurveSegment() );
			pSeg->SetSurfaceParameters( *this );

			// set the value of "v"
			if ( USES( bUses, EnvVars_v ) )
			{
				TqFloat vv = ( TqFloat ) segI /
				             ( TqFloat ) lastSegment;
				TqFloat vvnext =
				    ( TqFloat ) ( segI + 1 ) /
				    ( TqFloat ) lastSegment;
				CqParameterTypedVarying <
				TqFloat, type_float, TqFloat
				> * pVP = new CqParameterTypedVarying <
				          TqFloat, type_float, TqFloat
				          > ( "v", 1 );
				pVP->SetSize( pSeg->cVarying() );
				pVP->pValue( 0 )[0] = vv;
				pVP->pValue( 1 )[0] = vvnext;
				pSeg->AddPrimitiveVariable( pVP );
			}

			// process user parameters
			std::vector<CqParameter*>::iterator iUP;
			for (
			    iUP = aUserParams().begin();
			    iUP != aUserParams().end();
			    iUP++
			)
			{
				if (
				    ( ( *iUP ) ->Class() == class_varying ) ||
				    ( ( *iUP ) ->Class() == class_vertex )
				)
				{
					// copy "varying" or "vertex" class
					//  variables
					CqParameter * pNewUP =
					    ( *iUP ) ->CloneType(
					        ( *iUP ) ->strName().c_str(),
					        ( *iUP ) ->Count()
					    );
					assert(
					    pSeg->cVarying() ==
					    pSeg->cVertex()
					);
					pNewUP->SetSize( pSeg->cVarying() );

					pNewUP->SetValue( ( *iUP ), 0, vertexI );
					pNewUP->SetValue( ( *iUP ), 1, nextVertex );
					pSeg->AddPrimitiveVariable( pNewUP );

				}
				else if ( ( *iUP ) ->Class() == class_uniform )
				{
					// copy "uniform" class variables
					CqParameter * pNewUP =
					    ( *iUP ) ->CloneType(
					        ( *iUP ) ->strName().c_str(),
					        ( *iUP ) ->Count()
					    );
					pNewUP->SetSize( pSeg->cUniform() );

					pNewUP->SetValue( ( *iUP ), 0, uniformI );
					pSeg->AddPrimitiveVariable( pNewUP );
				}
				else if ( ( *iUP ) ->Class() == class_constant )
				{
					// copy "constant" class variables
					CqParameter * pNewUP =
					    ( *iUP ) ->CloneType(
					        ( *iUP ) ->strName().c_str(),
					        ( *iUP ) ->Count()
					    );
					pNewUP->SetSize( 1 );

					pNewUP->SetValue( ( *iUP ), 0, 0 );
					pSeg->AddPrimitiveVariable( pNewUP );
				} // if

			} // for each parameter


			++vertexI;
			aSplits.push_back( pSeg );
			++nSplits;

		} // for each curve segment

		if ( !m_periodic )
		{
			++vertexI;
		}
		++uniformI;

	} // for each curve

	return nSplits;

}



/**
 * Transforms this GPrim using the specified matrices.
 *
 * @param matTx         Reference to the transformation matrix.
 * @param matITTx       Reference to the inverse transpose of the 
 *                        transformation matrix, used to transform normals.
 * @param matRTx        Reference to the rotation only transformation matrix, 
 *                        used to transform vectors.
 * @param iTime			The frame time at which to perform the transformation.
 */
void CqLinearCurvesGroup::Transform(
    const CqMatrix& matTx,
    const CqMatrix& matITTx,
    const CqMatrix& matRTx,
    TqInt iTime
)
{
	// First, we want to transform the width array.  For each curve in the
	//  group, there are as many width parameters as there are vertices,
	//  so each vertex matches exactly with a width; no stuffing around is
	//  required.
	PopulateWidth();
	assert( cVarying() == cVertex() );
	for ( TqUint i = 0; i < cVarying(); i++ )
	{
		// first, create a horizontal vector in the new space which is
		//  the length of the current width in current space
		CqVector3D horiz( 1, 0, 0 );
		horiz = matITTx * horiz;
		horiz *= width()->pValue( i )[0] / horiz.Magnitude();

		// now, create two points; one at the vertex in current space
		//  and one which is offset horizontally in the new space by
		//  the width in the current space.  transform both points
		//  into the new space
		CqVector3D pt = P()->pValue( i )[0];
		CqVector3D pt_delta = pt + horiz;
		pt = matTx * pt;
		pt_delta = matTx * pt_delta;

		// finally, find the difference between the two points in
		//  the new space - this is the transformed width
		CqVector3D widthVector = pt_delta - pt;
		width()->pValue( i )[0] = widthVector.Magnitude();
	}

	// finally, we want to call the base class transform
	CqCurve::Transform( matTx, matITTx, matRTx, iTime );
}



/**
 * Constructor for a CqCubicCurvesGroup.
 *
 * @param ncurves       Number of curves in the group.
 * @param nvertices     Number of vertices per curve.
 * @param periodic      true if curves in the group are periodic.
 */
CqCubicCurvesGroup::CqCubicCurvesGroup(
    TqInt ncurves, TqInt nvertices[], TqBool periodic
) : CqCurvesGroup()
{

	m_ncurves = ncurves;
	m_periodic = periodic;

	// add up the total number of vertices
	m_nTotalVerts = 0;
	TqInt i;
	for ( i = 0; i < ncurves; i++ )
	{
		m_nTotalVerts += nvertices[ i ];
	}

	// copy the array of numbers of vertices
	m_nvertices.clear();
	m_nvertices.reserve( m_ncurves );
	for ( i = 0; i < m_ncurves; i++ )
	{
		m_nvertices.push_back( nvertices[ i ] );
	}

}



/**
 * CqCubicCurvesGroup copy constructor.
 */
/* CqCubicCurvesGroup::CqCubicCurvesGroup( const CqCubicCurvesGroup &from ) :
 * 		CqCurvesGroup()
 * {
 * 	( *this ) = from;
 * }
 */



/**
 * CqCubicCurvesGroup destructor.
 */
CqCubicCurvesGroup::~CqCubicCurvesGroup()
{
	m_nvertices.clear();
}



/**
 * Returns the number of parameters of varying storage class that this curve
 * group has.
 *
 * @return      Number of varying parameters.
 */
TqUint CqCubicCurvesGroup::cVarying() const
{

	TqInt vStep = pAttributes() ->GetIntegerAttribute( "System", "BasisStep" ) [ 1 ];
	TqUint varying_count = 0;

	for(TqInt i = 0; i < m_ncurves; ++i)
	{
		const TqUint segment_count = m_periodic ? (m_nvertices[i] / vStep) : ((m_nvertices[i] - 4) / vStep + 1);
		varying_count += m_periodic ? segment_count : segment_count + 1;
	}

	return varying_count;
}



/**
 * Create a clone of this curve group.
 *
 */
CqSurface* CqCubicCurvesGroup::Clone() const
{
	CqCubicCurvesGroup* clone = new CqCubicCurvesGroup();
	CqCurvesGroup::CloneData( clone );

	return ( clone );
}




/**
 * Splits a CqCubicCurvesGroup object into a set of piecewise-cubic curve
 * segments.
 *
 * @param aSplits       Vector to contain the cubic curve segments that are
 *                              created.
 *
 * @return  The number of piecewise-cubic curve segments that have been
 *              created.
 */
TqInt CqCubicCurvesGroup::Split(
    std::vector<boost::shared_ptr<CqSurface> >& aSplits
)
{

	// number of points to skip between curves
	TqInt vStep =
	    pAttributes() ->GetIntegerAttribute( "System", "BasisStep" ) [ 1 ];

	// information about which parameters are used
	TqInt bUses = Uses();

	TqInt vertexI = 0;     //< Current vertex class index.
	TqInt varyingI = 0;     //< Current varying class index.
	TqInt uniformI = 0;     //< Current uniform class index.
	TqInt nsplits = 0;     //< Number of split objects we've created.

	// process each curve in the group.  at this level, a curve is a
	//  set of joined piecewise-cubic curve segments.  curveN is the
	//  index of the current curve.
	for ( TqInt curveN = 0; curveN < m_ncurves; curveN++ )
	{
		// find the total number of piecewise cubic segments in the
		//  current curve, accounting for periodic curves
		TqInt npcSegs;
		if ( m_periodic )
		{
			npcSegs = m_nvertices[ curveN ] / vStep;
		}
		else
		{
			npcSegs = ( m_nvertices[ curveN ] - 4 ) / vStep + 1;
		}

		// find the number of varying parameters in the current curve
		TqInt nVarying;
		if ( m_periodic )
		{
			nVarying = npcSegs;
		}
		else
		{
			nVarying = npcSegs + 1;
		}

		TqInt nextCurveVertexIndex = vertexI + m_nvertices[ curveN ];
		TqInt nextCurveVaryingIndex = varyingI + nVarying;

		// process each piecewise cubic segment within the current
		//  curve.  pcN is the index of the current piecewise
		//  cubic curve segment within the current curve
		for ( TqInt pcN = 0; pcN < npcSegs; pcN++ )
		{
			// the current vertex index within the current curve group
			TqInt cvi = 0;

			// the current varying index within the current curve group
			TqInt cva = 0;

			// each segment needs four vertex indexes, which we
			//  calculate here.  if the index goes beyond the
			//  number of vertices then we wrap it around,
			//  starting back at zero.
			TqInt vi[ 4 ];
			vi[ 0 ] = vertexI + cvi;
			++cvi;
			if ( cvi >= m_nvertices[ curveN ] )
				cvi = 0;
			vi[ 1 ] = vertexI + cvi;
			++cvi;
			if ( cvi >= m_nvertices[ curveN ] )
				cvi = 0;
			vi[ 2 ] = vertexI + cvi;
			++cvi;
			if ( cvi >= m_nvertices[ curveN ] )
				cvi = 0;
			vi[ 3 ] = vertexI + cvi;
			++cvi;

			// we also need two varying indexes.  once again, we
			//  wrap around
			TqInt vai[ 2 ];
			vai[ 0 ] = varyingI + cva;
			++cva;
			if ( cva >= nVarying )
				cva = 0;
			vai[ 1 ] = varyingI + cva;
			++cva;

			// now, we need to find the value of v at the start
			//  and end of the current piecewise cubic curve
			//  segment
			TqFloat vstart = ( TqFloat ) pcN / ( TqFloat ) ( npcSegs );
			TqFloat vend = ( TqFloat ) ( pcN + 1 ) / ( TqFloat ) ( npcSegs );

			// create the new CqLinearCurveSegment for the current
			//  curve segment
			boost::shared_ptr<CqCubicCurveSegment> pSeg( new CqCubicCurveSegment() );
			pSeg->SetSurfaceParameters( *this );

			// set the value of "v"
			if ( USES( bUses, EnvVars_v ) )
			{
				CqParameterTypedVarying <
				TqFloat, type_float, TqFloat
				> * pVP = new CqParameterTypedVarying <
				          TqFloat, type_float, TqFloat
				          > ( "v", 1 );
				pVP->SetSize( pSeg->cVarying() );
				pVP->pValue( 0 )[0] = vstart;
				pVP->pValue( 1 )[0] = vend;
				pSeg->AddPrimitiveVariable( pVP );
			}

			// process user parameters
			std::vector<CqParameter*>::iterator iUP;
			for (
			    iUP = aUserParams().begin();
			    iUP != aUserParams().end();
			    iUP++
			)
			{
				if ( ( *iUP ) ->Class() == class_vertex )
				{
					// copy "vertex" class variables
					CqParameter * pNewUP =
					    ( *iUP ) ->CloneType(
					        ( *iUP ) ->strName().c_str(),
					        ( *iUP ) ->Count()
					    );
					pNewUP->SetSize( pSeg->cVertex() );

					for ( TqInt i = 0; i < 4; i++ )
					{
						pNewUP->SetValue(
						    ( *iUP ), i, vi[ i ]
						);
					}
					pSeg->AddPrimitiveVariable( pNewUP );

				}
				else if ( ( *iUP ) ->Class() == class_varying )
				{
					// copy "varying" class variables
					CqParameter * pNewUP =
					    ( *iUP ) ->CloneType(
					        ( *iUP ) ->strName().c_str(),
					        ( *iUP ) ->Count()
					    );
					pNewUP->SetSize( pSeg->cVarying() );

					pNewUP->SetValue( ( *iUP ), 0, vai[ 0 ] );
					pNewUP->SetValue( ( *iUP ), 1, vai[ 1 ] );
					pSeg->AddPrimitiveVariable( pNewUP );
				}
				else if ( ( *iUP ) ->Class() == class_uniform )
				{
					// copy "uniform" class variables
					CqParameter * pNewUP =
					    ( *iUP ) ->CloneType(
					        ( *iUP ) ->strName().c_str(),
					        ( *iUP ) ->Count()
					    );
					pNewUP->SetSize( pSeg->cUniform() );

					pNewUP->SetValue( ( *iUP ), 0, uniformI );
					pSeg->AddPrimitiveVariable( pNewUP );
				}
				else if ( ( *iUP ) ->Class() == class_constant )
				{
					// copy "constant" class variables
					CqParameter * pNewUP =
					    ( *iUP ) ->CloneType(
					        ( *iUP ) ->strName().c_str(),
					        ( *iUP ) ->Count()
					    );
					pNewUP->SetSize( 1 );

					pNewUP->SetValue( ( *iUP ), 0, 0 );
					pSeg->AddPrimitiveVariable( pNewUP );
				} // if
			} // for each user parameter



			vertexI += vStep;
			varyingI++;
			nsplits++;

			CqMatrix matBasis = pAttributes() ->GetMatrixAttribute( "System", "Basis" ) [ 1 ];
			pSeg ->ConvertToBezierBasis( matBasis );

			aSplits.push_back( pSeg );
		}

		vertexI = nextCurveVertexIndex;
		varyingI = nextCurveVaryingIndex;
		// we've finished our current curve, so we can get the next
		//  uniform parameter.  there's one uniform parameter per
		//  facet, so each curve corresponds to a facet.
		uniformI++;
	}

	return nsplits;

}



/**
 * Transforms this GPrim using the specified matrices.
 *
 * @param matTx         Reference to the transformation matrix.
 * @param matITTx       Reference to the inverse transpose of the 
 *                        transformation matrix, used to transform normals.
 * @param matRTx        Reference to the rotation only transformation matrix, 
 *                        used to transform vectors.
 * @param iTime			The frame time at which to apply the transformation.
 */
void CqCubicCurvesGroup::Transform(
    const CqMatrix& matTx,
    const CqMatrix& matITTx,
    const CqMatrix& matRTx,
    TqInt iTime
)
{

	// make sure the "width" parameter is present
	PopulateWidth();

	// number of points to skip between curves
	const TqInt vStep =
	    pAttributes() ->GetIntegerAttribute( "System", "BasisStep" ) [ 1 ];


	// First, we want to transform the width array.  For cubic curve
	//  groups, there is one width parameter at each parametric corner.

	TqInt widthI = 0;
	TqInt vertexI = 0;

	// Process each curve in the group.  At this level, each single curve
	//  is a set of piecewise-cubic curves.
	for ( TqInt curveN = 0; curveN < m_ncurves; curveN++ )
	{

		// now, for each curve in the group, we want to know how many
		//  varying parameters there are, since this determines how
		//  many widths will need to be transformed for this curve
		TqInt nsegments;
		if ( m_periodic )
		{
			nsegments = m_nvertices[ curveN ] / vStep;
		}
		else
		{
			nsegments = ( m_nvertices[ curveN ] - 4 ) / vStep + 1;
		}
		TqInt nvarying;
		if ( m_periodic )
		{
			nvarying = nsegments;
		}
		else
		{
			nvarying = nsegments + 1;
		}

		TqInt nextCurveVertexIndex = vertexI + m_nvertices[ curveN ];

		// now we process all the widths for the current curve
		for ( TqInt ccwidth = 0; ccwidth < nvarying; ccwidth++ )
		{

			// first, create a horizontal vector in the new space
			//  which is the length of the current width in
			//  current space
			CqVector3D horiz( 1, 0, 0 );
			horiz = matITTx * horiz;
			horiz *= width()->pValue( widthI )[0] / horiz.Magnitude();

			// now, create two points; one at the vertex in
			//  current space and one which is offset horizontally
			//  in the new space by the width in the current space.
			//  transform both points into the new space
			CqVector3D pt = P()->pValue( vertexI )[0];
			CqVector3D pt_delta = pt + horiz;
			pt = matTx * pt;
			pt_delta = matTx * pt_delta;

			// finally, find the difference between the two
			//  points in the new space - this is the transformed
			//  width
			CqVector3D widthVector = pt_delta - pt;
			width()->pValue( widthI )[0] = widthVector.Magnitude();


			// we've finished the current width, so we move on
			//  to the next one.  this means incrementing the width
			//  index by 1, and the vertex index by vStep
			++widthI;
			//            vertexI += vStep;
			vertexI = (vertexI + vStep)%m_nvertices[curveN];
		}
		vertexI = nextCurveVertexIndex;
	}

	// finally, we want to call the base class transform
	CqCurve::Transform( matTx, matITTx, matRTx, iTime );


}



END_NAMESPACE( Aqsis )
