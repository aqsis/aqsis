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
                handling Linear Curves primitives.
        \author Jonathan Merritt (j.merritt@pgrad.unimelb.edu.au)
*/

#include "aqsis.h"
#include "patch.h"
#include "vector3d.h"
#include "curves.h"
START_NAMESPACE( Aqsis )


static const TqUlong hp = CqString::hash("P");
static const TqUlong hu = CqString::hash("u");
static const TqUlong hn = CqString::hash("N");
static const TqUlong hv = CqString::hash("v");

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
    bool u
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
			NaturalSubdivide( ( *iUP ), pNewA, pNewB, false );
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
 * Constructor for a CqLinearCurvesGroup.
 *
 * @param ncurves       Number of curves in the group.
 * @param nvertices     Number of vertices per curve.
 * @param periodic      true if the curves in the group are periodic.
 */
CqLinearCurvesGroup::CqLinearCurvesGroup(
    TqInt ncurves, TqInt nvertices[], bool periodic
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


END_NAMESPACE( Aqsis )
