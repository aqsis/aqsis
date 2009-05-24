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

/**
        \file
        \brief Implements the classes and support structures for 
                handling Bicubic Curves primitives.
        \author Jonathan Merritt (j.merritt@pgrad.unimelb.edu.au)
*/

#include <stdio.h>
#include <string.h>

#include "imagebuffer.h"
#include "micropolygon.h"
#include "renderer.h"
#include "patch.h"
#include <aqsis/math/vector2d.h>
#include <aqsis/math/vector3d.h>
#include "curves.h"

namespace Aqsis {


static TqUlong hp = CqString::hash("P");
static TqUlong hu = CqString::hash("u");
static TqUlong hn = CqString::hash("N");
static TqUlong hv = CqString::hash("v");

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


namespace {

/** \brief Implementation of natural subdivision for cubic curves
 */
template <class T, class SLT>
void cubicCurveNatSubdiv(
	CqParameter* pParam,
	CqParameter* pResult1,
	CqParameter* pResult2
)
{
	CqParameterTyped<T, SLT>* pTParam = static_cast<CqParameterTyped<T, SLT>*>( pParam );
	CqParameterTyped<T, SLT>* pTResult1 = static_cast<CqParameterTyped<T, SLT>*>( pResult1 );
	CqParameterTyped<T, SLT>* pTResult2 = static_cast<CqParameterTyped<T, SLT>*>( pResult2 );

	pTResult1->pValue() [ 0 ] = pTParam->pValue() [ 0 ];
	pTResult1->pValue() [ 1 ] = static_cast<T>( ( pTParam->pValue() [ 0 ] + pTParam->pValue() [ 1 ] ) / 2.0f );
	pTResult1->pValue() [ 2 ] = static_cast<T>( pTResult1->pValue() [ 1 ] / 2.0f + ( pTParam->pValue() [ 1 ] + pTParam->pValue() [ 2 ] ) / 4.0f );

	pTResult2->pValue() [ 3 ] = pTParam->pValue() [ 3 ];
	pTResult2->pValue() [ 2 ] = static_cast<T>( ( pTParam->pValue() [ 2 ] + pTParam->pValue() [ 3 ] ) / 2.0f );
	pTResult2->pValue() [ 1 ] = static_cast<T>( pTResult2->pValue() [ 2 ] / 2.0f + ( pTParam->pValue() [ 1 ] + pTParam->pValue() [ 2 ] ) / 4.0f );

	pTResult1->pValue() [ 3 ] = static_cast<T>( ( pTResult1->pValue() [ 2 ] + pTResult2->pValue() [ 1 ] ) / 2.0f );
	pTResult2->pValue() [ 0 ] = pTResult1->pValue() [ 3 ];
}

} // unnamed namespace

void CqCubicCurveSegment::NaturalSubdivide(
    CqParameter* pParam,
    CqParameter* pParam1, CqParameter* pParam2,
    bool u
)
{
	assert( u == false );
	switch ( pParam->Type() )
	{
		case type_float:
			cubicCurveNatSubdiv<TqFloat, TqFloat>(pParam, pParam1, pParam2);
			break;
		case type_integer:
			cubicCurveNatSubdiv<TqInt, TqFloat>(pParam, pParam1, pParam2);
			break;
		case type_point:
		case type_vector:
		case type_normal:
			cubicCurveNatSubdiv<CqVector3D, CqVector3D>(pParam, pParam1, pParam2);
			break;
		case type_hpoint:
			cubicCurveNatSubdiv<CqVector4D, CqVector3D>(pParam, pParam1, pParam2);
			break;
		case type_color:
			cubicCurveNatSubdiv<CqColor, CqColor>(pParam, pParam1, pParam2);
			break;
		case type_string:
			cubicCurveNatSubdiv<CqString, CqString>(pParam, pParam1, pParam2);
			break;
		case type_matrix:
			/// \todo Why is this removed?
			//cubicCurveNatSubdiv<CqMatrix>( pParam, pParam1, pParam2);
			//break;
		default:
			break;
	}
}


namespace {

/** \brief Implementation of natural subdivision for varying parameters on
 * cubic curves.
 */
template <class T, class SLT>
void cubicCurveVaryingNatSubdiv(
	CqParameter* pParam,
	CqParameter* pResult1,
	CqParameter* pResult2
)
{
	CqParameterTyped<T, SLT>* pTParam = static_cast<CqParameterTyped<T, SLT>*>( pParam );
	CqParameterTyped<T, SLT>* pTResult1 = static_cast<CqParameterTyped<T, SLT>*>( pResult1 );
	CqParameterTyped<T, SLT>* pTResult2 = static_cast<CqParameterTyped<T, SLT>*>( pResult2 );

	pTResult1->pValue() [ 0 ] = pTParam->pValue() [ 0 ];
	pTResult1->pValue() [ 1 ] = pTResult2->pValue() [ 0 ] = static_cast<T>( ( pTParam->pValue() [ 0 ] + pTParam->pValue() [ 1 ] ) * 0.5f );
	pTResult2->pValue() [ 1 ] = pTParam->pValue() [ 1 ];
}

} // unnamed namespace

void CqCubicCurveSegment::VaryingNaturalSubdivide(
    CqParameter* pParam,
    CqParameter* pParam1, CqParameter* pParam2,
    bool u
)
{
	assert( u == false );
	switch ( pParam->Type() )
	{
		case type_float:
			cubicCurveVaryingNatSubdiv<TqFloat, TqFloat>(pParam, pParam1, pParam2);
			break;
		case type_integer:
			cubicCurveVaryingNatSubdiv<TqInt, TqFloat>(pParam, pParam1, pParam2);
			break;
		case type_point:
		case type_vector:
		case type_normal:
			cubicCurveVaryingNatSubdiv<CqVector3D, CqVector3D>(pParam, pParam1, pParam2);
			break;
		case type_hpoint:
			cubicCurveVaryingNatSubdiv<CqVector4D, CqVector3D>(pParam, pParam1, pParam2);
			break;
		case type_color:
			cubicCurveVaryingNatSubdiv<CqColor, CqColor>(pParam, pParam1, pParam2);
			break;
		case type_string:
			cubicCurveVaryingNatSubdiv<CqString, CqString>(pParam, pParam1, pParam2);
			break;
		case type_matrix:
			/// \todo Why is this removed?
			//cubicCurveVaryingNatSubdiv<CqMatrix, CqMatrix>(pParam, pParam1, pParam2);
			//break;
		default:
			break;
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
				assert(0 && "Unknown split decision");
	}
	return 0;
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
	aSplits[ 0 ] ->SetSplitCount( SplitCount() + 1 );

	aSplits[ 1 ] ->SetSurfaceParameters( *this );
	aSplits[ 1 ] ->SetSplitCount( SplitCount() + 1 );

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
			VaryingNaturalSubdivide( ( *iUP ), pNewA, pNewB, false );
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

namespace {

// Helper function for CalculateTangent

/** \brief Choose out of the three possible endpoint tangents for a bezier curve.
 *
 * This function tries to make sure curves with degenerate endpoints are
 * treated properly - it first estimates the overall size of the curve, then
 * measures the length of the provided tangents against this size to make sure
 * that they're not degenerate.
 *
 * This function is probably of comparable cost to computing the tangent using
 * a numerical derivative instead, hopefully it's a little more accurate...
 *
 * tangent1 is considered first, followed by tangent2 and finally tangent3.
 */
CqVector3D chooseEndpointTangent(const CqVector3D& tangent1, const CqVector3D& tangent2,
		const CqVector3D& tangent3)
{
	// Determine the "too small" length scale for the curve.
	TqFloat len1Sqd = tangent1.Magnitude2();
	TqFloat len2Sqd = tangent2.Magnitude2();
	TqFloat len3Sqd = tangent3.Magnitude2();
	// If tangents are shorter than a given small multiple of the overall curve size,
	// we will call them degenerate, and pass on to the next candidate tangent.
	TqFloat tooSmallLen = 1e-6 * max(max(len1Sqd, len2Sqd), len3Sqd);

	// Select a nondegenerate tangent.
	if(len1Sqd > tooSmallLen)
		return tangent1;
	else if(len2Sqd > tooSmallLen)
		return tangent2;
	else
		return tangent3;
}

} // unnamed namespace

CqVector3D	CqCubicCurveSegment::CalculateTangent(TqFloat u)
{
	// Read 3D vertices into the array pg.
	CqVector3D pg[4];
	for(TqInt i=0; i <= 3; i++)
		pg[i] = vectorCast<CqVector3D>(*P()->pValue(i));

	if(u == 0.0f)
		return chooseEndpointTangent(pg[1] - pg[0], pg[2] - pg[0], pg[3] - pg[0]);
	else if(u == 1.0f)
		return chooseEndpointTangent(pg[3] - pg[2], pg[3] - pg[1], pg[3] - pg[0]);
	else
	{
		// Generic case - may be derived by taking the derivative of the
		// parametric form of a bezier curve.  We leave off a factor of 3,
		// since we're interested in the direction, not the magnitude.
		// (Magnitude is dependent on the parametrization, not an intrinsic
		// property of the curve.)
		TqFloat u2 = u*u;
		return (-u2 + 2*u - 1)*pg[0] + (3*u2 - 4*u + 1)*pg[1]
			+ (-3*u2 + 2*u)*pg[2] + u2*pg[3];
	}
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

	CqVector3D direction0 = CalculateTangent(0.00);
	CqVector3D direction3 = CalculateTangent(1.00);

	CqVector3D direction1 = CalculateTangent(0.333);
	CqVector3D direction2 = CalculateTangent(0.666);

	CqVector3D normal0, normal1, normal2, normal3;
	GetNormal( 0, normal0 );
	GetNormal( 1, normal3 );
	normal1 = ( ( normal3 - normal0 ) / 3.0f ) + normal0;
	normal2 = ( ( ( normal3 - normal0 ) / 3.0f ) * 2.0f ) + normal0;

	CqVector3D widthOffset02 = (normal0 % direction0).Unit();
	CqVector3D widthOffset12 = (normal1 % direction1).Unit();
	CqVector3D widthOffset22 = (normal2 % direction2).Unit();
	CqVector3D widthOffset32 = (normal3 % direction3).Unit();

	TqFloat width0 = width()->pValue( 0 )[0];
	TqFloat width3 = width()->pValue( 1 )[0];
	TqFloat width1 = ( ( width3 - width0 ) / 3.0f ) + width0;
	TqFloat width2 = ( ( ( width3 - width0 ) / 3.0f ) * 2.0f ) + width0;

	widthOffset02 *= width0 / 6.0;
	widthOffset12 *= width1 / 6.0;
	widthOffset22 *= width2 / 6.0;
	widthOffset32 *= width3 / 6.0;

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
	pPatch->P()->pValue( 0  )[0] = P()->pValue( 0 )[0] + vectorCast<CqVector4D>(widthOffset0);
	pPatch->P()->pValue( 1  )[0] = P()->pValue( 0 )[0] + vectorCast<CqVector4D>(widthOffset02);
	pPatch->P()->pValue( 2  )[0] = P()->pValue( 0 )[0] - vectorCast<CqVector4D>(widthOffset02);
	pPatch->P()->pValue( 3  )[0] = P()->pValue( 0 )[0] - vectorCast<CqVector4D>(widthOffset0);

	pPatch->P()->pValue( 4  )[0] = P()->pValue( 1 )[0] + vectorCast<CqVector4D>(widthOffset1);
	pPatch->P()->pValue( 5  )[0] = P()->pValue( 1 )[0] + vectorCast<CqVector4D>(widthOffset12);
	pPatch->P()->pValue( 6  )[0] = P()->pValue( 1 )[0] - vectorCast<CqVector4D>(widthOffset12);
	pPatch->P()->pValue( 7  )[0] = P()->pValue( 1 )[0] - vectorCast<CqVector4D>(widthOffset1);

	pPatch->P()->pValue( 8  )[0] = P()->pValue( 2 )[0] + vectorCast<CqVector4D>(widthOffset2);
	pPatch->P()->pValue( 9  )[0] = P()->pValue( 2 )[0] + vectorCast<CqVector4D>(widthOffset22);
	pPatch->P()->pValue( 10 )[0] = P()->pValue( 2 )[0] - vectorCast<CqVector4D>(widthOffset22);
	pPatch->P()->pValue( 11 )[0] = P()->pValue( 2 )[0] - vectorCast<CqVector4D>(widthOffset2);

	pPatch->P()->pValue( 12 )[0] = P()->pValue( 3 )[0] + vectorCast<CqVector4D>(widthOffset3);
	pPatch->P()->pValue( 13 )[0] = P()->pValue( 3 )[0] + vectorCast<CqVector4D>(widthOffset32);
	pPatch->P()->pValue( 14 )[0] = P()->pValue( 3 )[0] - vectorCast<CqVector4D>(widthOffset32);
	pPatch->P()->pValue( 15 )[0] = P()->pValue( 3 )[0] - vectorCast<CqVector4D>(widthOffset3);

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


//------------------------------------------------------------------------------
// CqCubicCurvesGroup implementation
//------------------------------------------------------------------------------

namespace {

/** Get the number of piecewise cubic segments in a curve
 *
 * \param numVerts - number of vertices in the curve
 * \param step - number of verticies per segment
 * \param isPeriodic - true if the curve is periodic.
 */
inline TqInt segmentsPerCurve(TqInt numVerts, TqInt step, bool isPeriodic)
{
	if(isPeriodic)
		return numVerts / step;
	else
		return (numVerts - 4)/step + 1;
}

} // unnamed namespace


/**
 * Constructor for a CqCubicCurvesGroup.
 *
 * @param ncurves       Number of curves in the group.
 * @param nvertices     Number of vertices per curve.
 * @param periodic      true if curves in the group are periodic.
 */
CqCubicCurvesGroup::CqCubicCurvesGroup(
    TqInt ncurves, TqInt nvertices[], bool periodic
) : CqCurvesGroup(),
	m_nVertsBezier(0),
	m_basisTrans()
{
	// Inverse of the bezier basis (only computed once).
	static CqMatrix bezierInv(CqMatrix(RiBezierBasis).Inverse());

	m_basisTrans = pAttributes()->GetMatrixAttribute("System", "Basis")[1] * bezierInv;

	m_ncurves = ncurves;
	m_periodic = periodic;

	const TqInt vStep = pAttributes()->GetIntegerAttribute("System", "BasisStep")[1];
	// add up the total number of vertices
	m_nTotalVerts = 0;
	TqInt i;
	for ( i = 0; i < ncurves; i++ )
	{
		m_nTotalVerts += nvertices[ i ];
		m_nVertsBezier += 4*segmentsPerCurve(nvertices[i], vStep, m_periodic);
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

template<typename DataT, typename SLDataT>
CqParameter* CqCubicCurvesGroup::convertToBezierBasis(CqParameter* param)
{
	// input must be of the vertex storage class.
	assert(param->Class() == class_vertex);
	// Cast parameter to the specific type we're dealing with.
	typedef CqParameterTyped<DataT, SLDataT> TqParam;
	TqParam* inParam = static_cast<TqParam*>(param);
	// Number of array elements in the parameters.
	TqInt arrSize = inParam->Count();
	// Make a new parameter where we'll store the results of the basis conversion.
	TqParam* newParam = static_cast<TqParam*>(
			inParam->CloneType(inParam->strName().c_str(), arrSize));
	newParam->SetSize(m_nVertsBezier);
	// Step between cubic curve segments in current basis.
	const TqInt vStep = pAttributes()->GetIntegerAttribute("System", "BasisStep")[1];
	// Vertex index to the start of the current curve
	TqInt currCurveStart = 0;
	// Index into the full parameter array for the converted output vertices.
	TqInt outVertIdx = 0;
	for(TqInt curveNum = 0; curveNum < m_ncurves; ++curveNum)
	{
		const TqInt numVerts = m_nvertices[curveNum];
		// Number of piecewise cubic segments in the current curve.
		TqInt numSegs = segmentsPerCurve(numVerts, vStep, m_periodic);

		// Iterate over segments in current curve, and convert each to the
		// bezier basis.
		TqInt segVertIdx = 0;
		for(TqInt segNum = 0; segNum < numSegs; ++segNum)
		{
			// Get pointers to the input data 
			DataT* p0 = inParam->pValue(segVertIdx + currCurveStart);
			DataT* p1 = inParam->pValue((segVertIdx + 1) % numVerts + currCurveStart);
			DataT* p2 = inParam->pValue((segVertIdx + 2) % numVerts + currCurveStart);
			DataT* p3 = inParam->pValue((segVertIdx + 3) % numVerts + currCurveStart);
			// Get pointers to the output data
			DataT* po0 = newParam->pValue(outVertIdx);
			DataT* po1 = newParam->pValue(outVertIdx + 1);
			DataT* po2 = newParam->pValue(outVertIdx + 2);
			DataT* po3 = newParam->pValue(outVertIdx + 3);
			for(TqInt i = 0; i < arrSize; ++i)
			{
				// Convert current segment to bezier basis and store.
				//
				// This is just matrix-vector multiplication, where the
				// "vector" to be multiplied is [p0, p1, p2, p3] and made of
				// type DataT.  This works whenever DataT live in a vector
				// space (ie, everything from colours to floats).
				//
				// It even works for hpoints (CqVector4D), given that the
				// correct homogenous addition operators have been defined for
				// hpoints.
				po0[i] = m_basisTrans[0][0]*p0[i] + m_basisTrans[0][1]*p1[i]
					+ m_basisTrans[0][2]*p2[i] + m_basisTrans[0][3]*p3[i];
				po1[i] = m_basisTrans[1][0]*p0[i] + m_basisTrans[1][1]*p1[i]
					+ m_basisTrans[1][2]*p2[i] + m_basisTrans[1][3]*p3[i];
				po2[i] = m_basisTrans[2][0]*p0[i] + m_basisTrans[2][1]*p1[i]
					+ m_basisTrans[2][2]*p2[i] + m_basisTrans[2][3]*p3[i];
				po3[i] = m_basisTrans[3][0]*p0[i] + m_basisTrans[3][1]*p1[i]
					+ m_basisTrans[3][2]*p2[i] + m_basisTrans[3][3]*p3[i];
			}
			segVertIdx += vStep;
			outVertIdx += 4;
		}
		currCurveStart += numVerts;
	}
	return newParam;
}

void CqCubicCurvesGroup::AddPrimitiveVariable(CqParameter* param)
{
	// Vertex class primitives have to be handled specially - we need to
	// transform them into bezier control points from whatever basis they're
	// currently in.
	//
	// Since some bases (eg, power) can result in curves which are
	// discontinuous across segments, the step of the converted curves is 4.
	/// \todo Decide whether this is really necessary, or can we assume step 3?
	if(param->Class() == class_vertex)
	{
		CqParameter* newParam = 0;
		switch(param->Type())
		{
			case type_float:
				newParam = convertToBezierBasis<TqFloat, TqFloat>(param);
				break;
			case type_point:
			case type_normal:
			case type_vector:
				newParam = convertToBezierBasis<CqVector3D, CqVector3D>(param);
				break;
			case type_hpoint:
				newParam = convertToBezierBasis<CqVector4D, CqVector3D>(param);
				break;
			case type_color:
				newParam = convertToBezierBasis<CqColor, CqColor>(param);
				break;
			case type_matrix:
				newParam = convertToBezierBasis<CqMatrix, CqMatrix>(param);
				break;
			default:
				assert(0 && "Unsupported type for vertex class interpolation");
				delete param;
				return;
		}
		delete param;
		param = newParam;
	}
	CqCurvesGroup::AddPrimitiveVariable(param);
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
	TqInt i;

	if (m_periodic) 
	{
		for(i = 0; i < m_ncurves; ++i)
		{
			const TqUint segment_count = (m_nvertices[i] / vStep) ;
			varying_count += segment_count ;
		}
	} else 
	{
		for(i = 0; i < m_ncurves; ++i)
		{
			const TqUint segment_count = ((m_nvertices[i] - 4) / vStep + 1);
			varying_count += segment_count + 1;
		}
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

	TqInt curveVertexIndexStart = 0;     //< Start vertex index of the current curve.
	TqInt curveVaryingIndexStart = 0;     //< Start varying index of the current curve.
	TqInt curveUniformIndexStart = 0;     //< Start uniform index of the current curve.
	TqInt nsplits = 0;     //< Number of split objects we've created.

	// process each curve in the group.  at this level, a curve is a
	//  set of joined piecewise-cubic curve segments.  curveN is the
	//  index of the current curve.
	for ( TqInt curveN = 0; curveN < m_ncurves; curveN++ )
	{
		TqInt nVertex = m_nvertices[ curveN ];
		// find the total number of piecewise cubic segments in the
		// current curve.
		const TqInt npcSegs = segmentsPerCurve(nVertex, vStep, m_periodic);

		// find the number of varying parameters in the current curve
		TqInt nVarying = m_periodic ? npcSegs : npcSegs + 1;

		// the current varying index within the current curve group
		TqInt segmentVaryingIndex = 0;

		// process each piecewise cubic segment within the current
		//  curve.  pcN is the index of the current piecewise
		//  cubic curve segment within the current curve
		for ( TqInt pcN = 0; pcN < npcSegs; pcN++ )
		{
			// Each segment needs four vertex indexes, which we calculate here.
			TqInt vi[ 4 ];
			vi[ 0 ] = 4*pcN + curveVertexIndexStart;
			vi[ 1 ] = 4*pcN + 1 + curveVertexIndexStart;
			vi[ 2 ] = 4*pcN + 2 + curveVertexIndexStart;
			vi[ 3 ] = 4*pcN + 3 + curveVertexIndexStart;

			// we also need two varying indexes.  once again, we
			//  wrap around
			TqInt vai[ 2 ];
			vai[ 0 ] = ((segmentVaryingIndex+0)%nVarying) + curveVaryingIndexStart;
			vai[ 1 ] = ((segmentVaryingIndex+1)%nVarying) + curveVaryingIndexStart;

			// now, we need to find the value of v at the start
			//  and end of the current piecewise cubic curve
			//  segment
			TqFloat vstart = ( TqFloat ) pcN / ( TqFloat ) ( npcSegs );
			TqFloat vend = ( TqFloat ) ( pcN + 1 ) / ( TqFloat ) ( npcSegs );

			// create a new object for the current curve segment
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
				switch((*iUP)->Class())
				{
					case class_vertex:
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
						break;
					case class_varying:
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
						break;
					case class_uniform:
						{
							// copy "uniform" class variables
							CqParameter * pNewUP =
								( *iUP ) ->CloneType(
									( *iUP ) ->strName().c_str(),
									( *iUP ) ->Count()
								);
							pNewUP->SetSize( pSeg->cUniform() );

							pNewUP->SetValue( ( *iUP ), 0, curveUniformIndexStart );
							pSeg->AddPrimitiveVariable( pNewUP );
						}
						break;
					case class_constant:
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
						}
						break;
					default:
						// ignore any other interpolation classes (should warn?)
						break;
				}
			} // for each user parameter

			segmentVaryingIndex++;
			nsplits++;

			aSplits.push_back( pSeg );
		}

		curveVertexIndexStart += 4*npcSegs;
		curveVaryingIndexStart += nVarying;
		// we've finished our current curve, so we can get the next
		//  uniform parameter.  there's one uniform parameter per
		//  facet, so each curve corresponds to a facet.
		curveUniformIndexStart++;
	}

	return nsplits;

}


/** \brief Calculate bounds for a set of cubic curves.
 *
 * The curves are bounded by finding the minimum and maximum coordinates of the
 * Bezier control points.  This works because Bezier curves lie inside the
 * convex hull of their control points.
 *
 * The bound is then expanded by the maximum curve width.
 */
void CqCubicCurvesGroup::Bound(CqBound* bound) const
{
	// Start with an empty bound
	CqVector3D boundMin(FLT_MAX, FLT_MAX, FLT_MAX);
	CqVector3D boundMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	// Get bounds of Bezier control hull.
	for(TqInt i = 0, end = P()->Size(); i < end; ++i)
	{
		CqVector3D p = vectorCast<CqVector3D>(P()->pValue(i)[0]);
		boundMin = min(boundMin, p);
		boundMax = max(boundMax, p);
	}

	// Find maximum width of the curve
	TqFloat maxWidth = 0;
	for(TqInt i = 0, end = width()->Size(); i < end; ++i)
		maxWidth = max(maxWidth, width()->pValue(i)[0]);

	// Expand bound by max curve width
	bound->vecMin() = boundMin - maxWidth/2;
	bound->vecMax() = boundMax + maxWidth/2;

	AdjustBoundForTransformationMotion( bound );
}



} // namespace Aqsis
