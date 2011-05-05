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
		\brief Implements the classes for subdivision surfaces.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	"subdivision2.h"

#include	<fstream>
#include	<vector>

#include	"patch.h"
#include	"micropolygon.h"
#include	<aqsis/math/vectorcast.h>

namespace Aqsis {


//------------------------------------------------------------------------------
/**
 *	Constructor.
 */

CqSubdivision2::CqSubdivision2( )
		: CqMotionSpec<boost::shared_ptr<CqPolygonPoints> >( boost::shared_ptr<CqPolygonPoints>() ),
		m_bInterpolateBoundary( false ),
		m_faceVertexParams(),
		m_fFinalised(false)
{}


//------------------------------------------------------------------------------
/**
 *	Constructor.
 */

CqSubdivision2::CqSubdivision2( const boost::shared_ptr<CqPolygonPoints>& pPoints )
	:  CqMotionSpec<boost::shared_ptr<CqPolygonPoints> >(pPoints),
	m_bInterpolateBoundary( false ),
	m_faceVertexParams(),
	m_fFinalised(false)
{
	// Store the reference to our points.
	AddTimeSlot( 0, pPoints );

	STATS_INC( GPR_subdiv );

	// Cache the facevertex user params from the first time slot.
	for(std::vector<CqParameter*>::iterator iUP = pPoints->aUserParams().begin();
		iUP != pPoints->aUserParams().end(); iUP++)
	{
		if((*iUP)->Class() == class_facevertex)
			m_faceVertexParams.push_back(*iUP);
	}
}


//------------------------------------------------------------------------------
/**
 *	Destructor.
 */

CqSubdivision2::~CqSubdivision2()
{
	// Delete the array of laths generated during the facet adding phase.
	for(std::vector<CqLath*>::const_iterator iLath=apLaths().begin(); iLath!=apLaths().end(); iLath++)
		if(*iLath)
			delete(*iLath);
}


//------------------------------------------------------------------------------
/**
 *	Get a pointer to a lath referencing the specified facet index.
 *	The returned lath pointer can be any lath on the edge of the facet.
 *	Asserts if the facet index is invalid.
 *
 *	@param	iIndex	Index of the facet to query.
 *
 *	@return			Pointer to a lath on the facet.
 */
CqLath* CqSubdivision2::pFacet(TqInt iIndex)
{
	assert(iIndex < static_cast<TqInt>(m_apFacets.size()));
	return(m_apFacets[iIndex]);
}

//------------------------------------------------------------------------------
/**
 *	Get a pointer to a lath which references the specified vertex index.
 *	The returned lath pointer can be any lath which references the vertex.
 *	Asserts if the vertex index is invalid.
 *
 *	@param	iIndex	Index of the vertex to query.
 *
 *	@return			Pointer to a lath on the vertex.
 */
CqLath* CqSubdivision2::pVertex(TqInt iIndex)
{
	assert(iIndex < static_cast<TqInt>(m_aapVertices.size()) && m_aapVertices[iIndex].size() >= 1);
	return(m_aapVertices[iIndex][0]);
}

//------------------------------------------------------------------------------
/**
 *	Get a pointer to a lath referencing the specified facet index.
 *	The returned lath pointer can be any lath on the edge of the facet.
 *	Asserts if the facet index is invalid.
 *
 *	@param	iIndex	Index of the facet to query.
 *
 *	@return			Pointer to a lath on the facet.
 */
const CqLath* CqSubdivision2::pFacet(TqInt iIndex) const
{
	assert(iIndex < static_cast<TqInt>(m_apFacets.size()));
	return(m_apFacets[iIndex]);
}

//------------------------------------------------------------------------------
/**
 *	Get a pointer to a lath which references the specified vertex index.
 *	The returned lath pointer can be any lath which references the vertex.
 *	Asserts if the vertex index is invalid.
 *
 *	@param	iIndex	Index of the vertex to query.
 *
 *	@return			Pointer to a lath on the vertex.
 */
const CqLath* CqSubdivision2::pVertex(TqInt iIndex) const
{
	assert(iIndex < static_cast<TqInt>(m_aapVertices.size()) && m_aapVertices[iIndex].size() >= 1);
	return(m_aapVertices[iIndex][0]);
}


//------------------------------------------------------------------------------
/**
 *	Initialise the topology class to store the specified number of vertices.
 *	Use this function to prepare the topology structure to receive a number of
 *	vertices then use SetVertex to initialise them.
 *
 *	@param	cVerts	Then number of vertices that will be needed.
 */
void CqSubdivision2::Prepare(TqInt cVerts)
{
	// Initialise the array of vertex indexes to the appropriate size.
	m_aapVertices.resize(cVerts);

	m_fFinalised=false;
}

CqVector3D CqSubdivision2::limitPoint(CqLath* vert)
{
	// To compute the limit point, we make use of a limit mask for
	// Catmull-Clark subdivision.  For the standard Catmull-Clark scheme this
	// has a surprisingly simple form which is given in the literature, for
	// example, see
	//
	// "Efficient, Fair Interpolation using Catmull-Clark Surfaces" by Mark
	// Halstead, Tony DeRose and Michael Kass, SIGGRAPH 1993 (Proceedings of
	// the 20th annual conference on Computer graphics and interactive
	// techniques)
	//
	// Aqsis uses a genralised Catmull-Clark scheme which has been modified to
	// allow creasing, sharp corners and interpolation of the manifold
	// boundary.  This makes things more complicated:
	//
	// * Creases pose a difficulty since they introduce possibly several extra
	//   parameters into the subdivision matrix and presumably make the limit
	//   mask more difficult to compute.  For now we just ignore creases and hope
	//   for the best!
	//
	// * For boundary vertices the subdivision matrix is simple and we can
	//   compute the limit mask by hand (see below)
	//
	// * For sharp corners the vertex is stationary under subdivision so this
	//   case is trivial.

	const CqVector3D vPos = vectorCast<CqVector3D>(
			pPoints()->P()->pValue()[vert->VertexIndex()]);

	// Sharp corners don't move under subdivision; just return them.
	if(CornerSharpness(vert) > 0.0f)
		return vPos;

	// We need to make sure that all parent faces of vert are subdivided, since
	// we need the positions as input to the limit point calculation.
	if(vert->pParentFacet())
	{
		CqLath* v = vert->pParentFacet();
		// TODO: Do this more efficiently!
		const CqLath* const v0 = v;
		do
		{
			subdivideNeighbourFaces(v);
			v = v->cf();
		} while(v != v0);
	}
	// Grab a pointer to the positions.  It's very important that we do this
	// *after* the possible subdivision steps above, or the array may have been
	// reallocated.
	const CqVector4D* P = pPoints()->P()->pValue();

	if(vert->isBoundaryVertex())
	{
		// Consider a mesh boundary in the neighbourhood of a vertex v:
		//
		// ... --- e1 ------- v ------- e2 --- ...
		//         |          |         |
		//         |          |         |
		//         .          .         .
		//         .          .         .
		//
		// With the "interpolateboundary" tag, the neighbourhood of v is
		// subdivided according to the subdivision matrix
		//
		//         [6 1 1]
		// S = 1/8 [4 4 0]
		//         [4 0 4]
		//
		// where the order of matrix elements corresponds to [v, e1, e2].  S
		// maps a neighbourhood of the vertex v at step N to the neighbourhood
		// at step N+1 - it gives us the new position of v and the position of
		// the two new edge vertices on the boundary.  That is, after one step
		// of subdivision of the neighbourhood of v, the above diagram becomes
		//
		// ... --- e1 --e1'-- v'--e2'-- e2 --- ...
		//         |    .     |    .    |
		//         |    .     |    .    |
		//         .          .         .
		//         .          .         .
		//
		// with  [v' e1' e2']^T  =  S * [v e1 e2]^T
		//
		// Note that S is independent of the valence of the vertex v for
		// boundary subdivision.
		//
		//
		// As required by the theory, the largest eigenvalue of S is 1.  The
		// associated left-eigenvector gives the desired limit mask,
		//
		// l1 = 1/6 [4 1 1]
		//
		// where l1 has been normalised so that the components add to 1.  That
		// is, the position of v on the limit surface is
		// 
		// v_limit = 1/6 * (4*v + 1*e1 + 1*e2)
		//
		// (In hindsight this probably amounts to simple b-spline subdivision,
		// but the above gives a nice simple illustration of the general method
		// used to derive limit masks.  Unfortunately the eigenanalysis is much
		// more tricky in most cases.)

		if(vert->isCornerVertex())
		{
			// Special case for corner vertices - these don't move under
			// subdivision
			return vPos;
		}

		// Now we know we're on a boundary with more than two edges

		// get clockwise edge vertex, e1
		const CqLath* v = vert;
		while(v->cv())
			v = v->cv();
		CqVector3D edgeSum = vectorCast<CqVector3D>(P[v->ccf()->VertexIndex()]);

		// get anticlocwise edge vertex, e2
		v = vert;
		while(v->ccv())
			v = v->ccv();
		edgeSum += vectorCast<CqVector3D>(P[v->cf()->VertexIndex()]);

		return (4.0/6)*vPos + (1.0/6)*edgeSum;
	}
	else
	{
		// Else use the general limit mask for an interior part of the mesh.
		// This is discussed in various places in the literature, see [Halsted
		// 1993] for example.  For the classic Catmull-Clark rules, the mask is
		//
		// v' = (n^2 * v  +  sum_i (4*e_i + f_i)) / (n*(n+5))
		//
		// where n is the valence of the vertex v, e_i are the vertices
		// attached to adjoining edges, and f_i are the remaining vertices on
		// adjoining faces.  In this form the limit mask is only valid for
		// quadrilateral neighbourhoods (guaranteed after one step of
		// subdivision) for which there is a single f_i per face.
		//
		// For the general case, we can adjust the meaning of f_i so that the
		// formula above still gives the correct results.  Consider a non-quad
		// face, eg
		//
		//
		//   f4       e1     f1
		//     +------+------+
		//     |      |      |
		//     |      |      |
		//     |      |      |
		//  e4 +------+------+ e2
		//     |      |v      \                                              .
		//     |      |        \                                             .
		//     |      |    .C   + g1
		//     +------+e3      /
		//   f3        \      /
		//              +----+ g2
		//              g3          * f2
		//
		//
		// The limit mask above doesn't apply for the non-quadrilateral face.
		// However, the position of the centroid, C, is the only way in which
		// the extra vertices g_i effect the neighbourhood of v after the first
		// subdivision step.  This means that we just need to compute a fake
		// value for the vertex f2 to replace the g_i so that the centroid is
		// the same:
		//
		// f2 = (4/m - 1)*(v + e2 + e3)  +  4/m * sum_i g_i
		//
		// where m is the number of edges on the face.  For more discussion,
		// see page 11 of
		//
		//   "Fast C 2 Interpolating Subdivision Surfaces using Iterative
		//   Inversion of Stationary Subdivision Rules", Andrew Thall, 2003,
		//   Technical Report TR02-001, UNC-Chapel Hill.
		//

		const CqLath* faceVert = vert;
		CqVector3D eSum;
		CqVector3D fSum;
		TqInt numEdges = 0;
		do
		{
			// Add edge onto edge sum.
			const CqLath* const e = faceVert->cf();
			eSum += vectorCast<CqVector3D>(P[e->VertexIndex()]);
			// Add up remaining face verts.  For a quad mesh there will only be
			// one of these.
			// Add face vert to face sum.
			const CqLath* f = e->cf();
			if(f->cf()->cf() == faceVert)
			{
				fSum += vectorCast<CqVector3D>(P[f->VertexIndex()]);
			}
			else
			{
				// This is the special case of a non-quadrilateral face.  As
				// described abeove, we need to compute the sum of the
				// additional vertices.
				CqVector3D gSum;
				TqInt numVerts = 3;
				const CqLath* const eNext = faceVert->ccf();
				while(f != eNext)
				{
					gSum += vectorCast<CqVector3D>(P[f->VertexIndex()]);
					++numVerts;
					f = f->cf();
				}
				fSum += (4.0/numVerts - 1) * ( vPos
						+ vectorCast<CqVector3D>(P[e->VertexIndex()])
						+ vectorCast<CqVector3D>(P[eNext->VertexIndex()]) )
					+ 4.0/numVerts*gSum;
			}

			faceVert = faceVert->cv();
			++numEdges;
		}
		while(faceVert != vert);

		return 1.0/(numEdges*(numEdges+5)) * (numEdges*numEdges*vPos + 4*eSum + fSum);
	}
}

//------------------------------------------------------------------------------
namespace {

/** \brief Determine whether a facevertex parameter is discontinuous at the
 * given vertex.
 *
 * Primitive variables of storage class "facevertex" cannot be sensibly
 * interpolated by the usual vertex interpolation rules if the various values
 * associated with a vertex are not the same.  This function notices such
 * discontinuities.
 *
 * \param pParam - geometric parameter of class "facevertex" to test for discontinuitiy
 * \param pVert - vertex in the topology data structure
 * \param arrayIndex - array index for pParam.
 */
template<class TypeA, class TypeB>
inline bool isDiscontinuousFaceVertex(const CqParameterTyped<TypeA, TypeB>* pParam,
		CqLath* pVert, TqInt arrayIndex)
{
	TypeA currVertVal = pParam->pValue(pVert->FaceVertexIndex())[arrayIndex];
	// Get the facets which share this vertex.
	std::vector<CqLath*> aQvf;
	pVert->Qvf(aQvf);
	for(std::vector<CqLath*>::const_iterator iVf = aQvf.begin();
			iVf != aQvf.end(); ++iVf)
	{
		if(!isClose(currVertVal, pParam->pValue((*iVf)->FaceVertexIndex())[arrayIndex]))
			return true;
	}
	return false;
}

/** \brief Determine whether a facevertex parameter is discontinuous on the
 * given edge.
 *
 * \see isDiscontinuousFaceVertex
 *
 * \param pParam - geometric parameter of class "facevertex" to test for discontinuitiy
 * \param pEdge - edge in the topology data structure
 * \param arrayIndex - array index for pParam.
 */
template<class TypeA, class TypeB>
inline bool isDiscontinuousFaceVertexEdge(const CqParameterTyped<TypeA, TypeB>* pParam,
		CqLath* pEdge, TqInt arrayIndex)
{
	CqLath* pCompanion = pEdge->ec();
	if(pCompanion == NULL)
	{
		// We're on a boundary; this edge cannot have discontinuous facevertex values.
		return false;
	}
	return !isClose(pParam->pValue(pEdge->FaceVertexIndex())[arrayIndex],
					pParam->pValue(pEdge->cv()->FaceVertexIndex())[arrayIndex])
		|| !isClose(pParam->pValue(pCompanion->FaceVertexIndex())[arrayIndex],
					pParam->pValue(pCompanion->cv()->FaceVertexIndex())[arrayIndex]);
}

} // unnamed namespace

//------------------------------------------------------------------------------
template<class TypeA, class TypeB>
void CqSubdivision2::CreateVertex(CqParameter* pParamToModify,
		CqLath* pVertex, TqInt iIndex)
{
	CqParameterTyped<TypeA, TypeB>* pParam
		= static_cast<CqParameterTyped<TypeA, TypeB>*>(pParamToModify);
	for(TqInt arrayindex = 0, arraysize = pParam->Count();
			arrayindex < arraysize; arrayindex++ )
	{
		TypeA S = TypeA(0.0f);
		TypeA Q = TypeA(0.0f);
		TypeA R = TypeA(0.0f);
		TqInt n;

		if(pParam->Class() == class_vertex || pParam->Class() == class_facevertex)
		{
			// Get a pointer to the appropriate index accessor function on CqLath based on class.
			TqInt (CqLath::*IndexFunction)() const;
			if( pParam->Class() == class_vertex )
				IndexFunction = &CqLath::VertexIndex;
			else
			{
				IndexFunction = &CqLath::FaceVertexIndex;
				// Perform a special check for whether all values at the vertex
				// agree.  If they don't, we make the vertex a "hard" one.
				//
				// It's possible that this check should be extended to all the
				// vertices which are connected by edges to this one, but it's
				// tricky to say whether this is entirely necessary.  Initial
				// results appear to look good without bothering to do this
				// extra step.
				//
				// HOWEVER, this is a prime place to start looking if
				// facevertex interpolation seems a little strange in the
				// future.
				if(isDiscontinuousFaceVertex(pParam, pVertex, arrayindex))
				{
					pParam->pValue(iIndex)[arrayindex]
						= pParam->pValue((pVertex->*IndexFunction)())[arrayindex];
					continue;
				}
			}

			// Determine if we have a boundary vertex.
			if( pVertex->isBoundaryVertex() )
			{
				// The vertex is on a boundary.
				/// \note If "interpolateboundary" is not specified, we will never see this as
				/// the boundary facets aren't rendered. So we don't need to check for "interpolateboundary" here.
				std::vector<CqLath*> apQve;
				pVertex->Qve(apQve);
				// Is the valence == 2 ?
				if( apQve.size() == 2 )
				{
					// Yes, boundary with valence 2 is corner.
					pParam->pValue( iIndex )[arrayindex] = pParam->pValue( (pVertex->*IndexFunction)() )[arrayindex];
				}
				else
				{
					// No, boundary is average of two adjacent boundary edges, and original point.
					// Get the midpoints of the adjacent boundary edges
					std::vector<CqLath*> aQve;
					pVertex->Qve( aQve );

					TqInt cBoundaryEdges = 0;
					std::vector<CqLath*>::iterator iE;
					for( iE = aQve.begin(); iE != aQve.end(); iE++ )
					{
						// Only consider the boundary edges.
						if( NULL == (*iE)->ec() )
						{
							if( (*iE)->VertexIndex() == pVertex->VertexIndex() )
								R += pParam->pValue( ((*iE)->ccf()->*IndexFunction)() )[arrayindex];
							else
								R += pParam->pValue( ((*iE)->*IndexFunction)() )[arrayindex];
							cBoundaryEdges++;
						}
					}
					assert( cBoundaryEdges == 2 );

					// Get the current vertex;
					S = pParam->pValue( (pVertex->*IndexFunction)() )[arrayindex];
					pParam->pValue( iIndex )[arrayindex] =
						static_cast<TypeA>( ( R + ( S * 6.0f ) ) / 8.0f );
				}
			}
			else
			{
				// Check if a sharp corner vertex.
				if( CornerSharpness( pVertex ) > 0.0f )
				{
					pParam->pValue( iIndex )[arrayindex] = pParam->pValue( (pVertex->*IndexFunction)() )[arrayindex];
				}
				else
				{
					// Check if crease vertex.
					std::vector<CqLath*> aQve;
					pVertex->Qve( aQve );

					CqLath* hardEdge1 = NULL;
					CqLath* hardEdge2 = NULL;
					CqLath* hardEdge3 = NULL;
					TqInt se = 0;
					std::vector<CqLath*>::iterator iEdge;
					for( iEdge = aQve.begin(); iEdge != aQve.end(); iEdge++ )
					{
						float h = EdgeSharpness( (*iEdge) );
						if( hardEdge1 == NULL || h > EdgeSharpness(hardEdge1) )
						{
							hardEdge3 = hardEdge2;
							hardEdge2 = hardEdge1;
							hardEdge1 = *iEdge;
						}
						else if( hardEdge2 == NULL || h > EdgeSharpness(hardEdge2) )
						{
							hardEdge3 = hardEdge2;
							hardEdge2 = *iEdge;
						}
						else if( hardEdge3 == NULL || h > EdgeSharpness(hardEdge3) )
						{
							hardEdge3 = *iEdge;
						}

						if( h > 0.0f )
						{
							se++;
							//		printf("h = %f\n", h);
						}
					}

					TypeA softPos;
					TypeA semiSharpPos;
					TypeA sharpPos;
					// Smooth
					// Vertex point is...
					//    Q     2R     S(n-3)
					//   --- + ---- + --------
					//    n      n        n
					//
					// Q = Average of face points surrounding old vertex
					// R = average of midpoints of edges surrounding old vertex
					// S = old vertex
					// n = number of edges sharing the old vertex.

					n = aQve.size();

					// Get the face points of the surrounding faces
					std::vector<CqLath*> aQvf;
					pVertex->Qvf( aQvf );
					std::vector<CqLath*>::iterator iF;
					for( iF = aQvf.begin(); iF != aQvf.end(); iF++ )
					{
						std::vector<CqLath*> aQfv;
						(*iF)->Qfv(aQfv);
						std::vector<CqLath*>::iterator iV;
						TypeA Val = TypeA(0.0f);
						for( iV = aQfv.begin(); iV != aQfv.end(); iV++ )
							Val += pParam->pValue( ((*iV)->*IndexFunction)() )[arrayindex];
						Val = static_cast<TypeA>( Val / static_cast<TqFloat>( aQfv.size() ) );
						Q += Val;
					}
					Q /= aQvf.size();
					Q /= n;

					// Get the midpoints of the surrounding edges
					TypeA A = pParam->pValue( (pVertex->*IndexFunction)() )[arrayindex];
					TypeA B = TypeA(0.0f);
					std::vector<CqLath*>::iterator iE;
					for( iE = aQve.begin(); iE != aQve.end(); iE++ )
					{
						B = pParam->pValue( ((*iE)->ccf()->*IndexFunction)() )[arrayindex];
						R += static_cast<TypeA>( (A+B)/2.0f );
					}
					R = static_cast<TypeA>( R * 2.0f );
					R /= n;
					R /= n;

					// Get the current vertex;
					S = pParam->pValue( (pVertex->*IndexFunction)() )[arrayindex];
					S = static_cast<TypeA>( S * static_cast<TqFloat>(n-3) );
					S /= n;

					semiSharpPos = static_cast<TypeA>( ( R + ( S * 6.0f ) ) / 8.0f );
					//pParam->pValue( iIndex )[0] = Q+R+S;
					softPos = Q+R+S;

					if( se >= 2 )
					{
						// Crease
						// Get the midpoints of the surrounding 2 hardest edges
						R = pParam->pValue((hardEdge1->ccf()->*IndexFunction)() )[arrayindex];
						R = R + pParam->pValue((hardEdge2->ccf()->*IndexFunction)() )[arrayindex];

						// Get the current vertex;
						S = pParam->pValue( (pVertex->*IndexFunction)() )[arrayindex];
						semiSharpPos = static_cast<TypeA>( ( R + ( S * 6.0f ) ) / 8.0f );
					}

					sharpPos = pParam->pValue( (pVertex->*IndexFunction)() )[arrayindex];

					// Blend the three values together weighted by the sharpness values.
					TypeA Pos;
					float h2 = hardEdge2 != NULL ? EdgeSharpness(hardEdge2) : 0.0f;
					float h3 = hardEdge3 != NULL ? EdgeSharpness(hardEdge3) : 0.0f;
					Pos = static_cast<TypeA>( (1.0f - h2)*softPos );
					Pos = static_cast<TypeA>( Pos + (h2 - h3)*semiSharpPos );
					Pos = static_cast<TypeA>( Pos + h3*sharpPos );
					pParam->pValue( iIndex )[arrayindex] = Pos;
				}
			}
		}
		else
		{
			// Get a pointer to the appropriate index accessor function on CqLath based on class.
			TqInt (CqLath::*IndexFunction)() const;
			if( pParam->Class() == class_varying )
				IndexFunction = &CqLath::VertexIndex;
			else
				IndexFunction = &CqLath::FaceVertexIndex;

			TypeA A = pParam->pValue( (pVertex->*IndexFunction)() )[arrayindex];
			pParam->pValue( iIndex )[arrayindex] = A;
		}
	}
}

//------------------------------------------------------------------------------
template<class TypeA, class TypeB>
void CqSubdivision2::DuplicateVertex(CqParameter* pParamToModify, CqLath* pVertex, TqInt iIndex)
{
	CqParameterTyped<TypeA, TypeB>* pParam
		= static_cast<CqParameterTyped<TypeA, TypeB>*>(pParamToModify);
	for(TqInt arrayindex = 0, arraysize = pParam->Count(); arrayindex < arraysize; arrayindex++ )
	{
		if(pParam->Class() == class_vertex || pParam->Class() == class_facevertex)
		{
			// Get a pointer to the appropriate index accessor function on CqLath based on class.
			TqInt (CqLath::*IndexFunction)() const;
			if( pParam->Class() == class_vertex )
				IndexFunction = &CqLath::VertexIndex;
			else
				IndexFunction = &CqLath::FaceVertexIndex;

			pParam->pValue(iIndex)[arrayindex]	= pParam->pValue((pVertex->*IndexFunction)())[arrayindex];
		}
		else
		{
			// Get a pointer to the appropriate index accessor function on CqLath based on class.
			TqInt (CqLath::*IndexFunction)() const;
			if( pParam->Class() == class_varying )
				IndexFunction = &CqLath::VertexIndex;
			else
				IndexFunction = &CqLath::FaceVertexIndex;

			TypeA A = pParam->pValue( (pVertex->*IndexFunction)() )[arrayindex];
			pParam->pValue( iIndex )[arrayindex] = A;
		}
	}
}

//------------------------------------------------------------------------------
/**
 *	Add a completely new vertex to the list.
 *	Appends a new vertex to the end of the list, updating the referencing
 *	table as well.
 *
 *	@return			The index of the new point.
 */
void CqSubdivision2::AddVertex(CqLath* pVertex, TqInt& iVIndex, TqInt& iFVIndex)
{
	iFVIndex=0;

	// If -1 is passed in as the 'vertex' class index, we must create a new value.
	bool fNewVertex = iVIndex < 0;

	std::vector<CqParameter*>::iterator iUP;
	TqInt iTime;

	for( iTime = 0; iTime < cTimes(); iTime++ )
	{
		for( iUP = pPoints( iTime )->aUserParams().begin(); iUP != pPoints( iTime )->aUserParams().end(); iUP++ )
		{
			TqInt iIndex = ( *iUP )->Size();
			// Store the index in the return variable based on its type.
			if( ( *iUP )->Class() == class_vertex || ( *iUP )->Class() == class_varying )
			{
				if( fNewVertex )
				{
					assert( iVIndex<0 || iVIndex==iIndex );
					iVIndex = iIndex;
					( *iUP )->SetSize( iIndex+1 );
					// Resize the vertex lath
					m_aapVertices.resize(iVIndex+1);
				}
				else
					continue;
			}
			else if( ( *iUP )->Class() == class_facevarying || ( *iUP )->Class() == class_facevertex )
			{
				assert( iFVIndex==0 || iFVIndex==iIndex );
				iFVIndex = iIndex;
				( *iUP )->SetSize( iIndex+1 );
			}
			else
				continue;

			switch((*iUP)->Type())
			{
				case type_float:
					CreateVertex<TqFloat, TqFloat>(*iUP, pVertex, iIndex);
					break;
				case type_integer:
					CreateVertex<TqInt, TqFloat>(*iUP, pVertex, iIndex);
					break;
				case type_point:
				case type_normal:
				case type_vector:
					CreateVertex<CqVector3D, CqVector3D>(*iUP, pVertex, iIndex);
					break;
				case type_color:
					CreateVertex<CqColor, CqColor>(*iUP, pVertex, iIndex);
					break;
				case type_hpoint:
					CreateVertex<CqVector4D, CqVector3D>(*iUP, pVertex, iIndex);
					break;
				case type_string:
					//CreateVertex<CqString, CqString>(*iUP, pVertex, iIndex);
					break;
				case type_matrix:
					//CreateVertex<CqMatrix, CqMatrix>(*iUP, pVertex, iIndex);
					break;
				default:
					// left blank to avoid compiler warnings about unhandled types
					break;
			}
		}
	}
}

//------------------------------------------------------------------------------
/**
 *	Duplicate an existing vertex in the mesh.
 *	Appends a new vertex to the end of the list, updating the referencing
 *	table as well.
 *
 *	@return			The index of the new point.
 */
void CqSubdivision2::DuplicateVertex(CqLath* pVertex, TqInt& iVIndex, TqInt& iFVIndex)
{
	iFVIndex=0;

	// If -1 is passed in as the 'vertex' class index, we must create a new value.
	bool fNewVertex = iVIndex < 0;

	std::vector<CqParameter*>::iterator iUP;
	TqInt iTime;

	for( iTime = 0; iTime < cTimes(); iTime++ )
	{
		for( iUP = pPoints( iTime )->aUserParams().begin(); iUP != pPoints( iTime )->aUserParams().end(); iUP++ )
		{
			TqInt iIndex = ( *iUP )->Size();
			// Store the index in the return variable based on its type.
			if( ( *iUP )->Class() == class_vertex || ( *iUP )->Class() == class_varying )
			{
				if( fNewVertex )
				{
					assert( iVIndex<0 || iVIndex==iIndex );
					iVIndex = iIndex;
					( *iUP )->SetSize( iIndex+1 );
					// Resize the vertex lath
					m_aapVertices.resize(iVIndex+1);
				}
				else
					continue;
			}
			else if( ( *iUP )->Class() == class_facevarying || ( *iUP )->Class() == class_facevertex )
			{
				assert( iFVIndex==0 || iFVIndex==iIndex );
				iFVIndex = iIndex;
				( *iUP )->SetSize( iIndex+1 );
			}
			else
				continue;

			switch((*iUP)->Type())
			{
				case type_float:
					DuplicateVertex<TqFloat, TqFloat>(*iUP, pVertex, iIndex);
					break;
				case type_integer:
					DuplicateVertex<TqInt, TqFloat>(*iUP, pVertex, iIndex);
					break;
				case type_point:
				case type_normal:
				case type_vector:
					DuplicateVertex<CqVector3D, CqVector3D>(*iUP, pVertex, iIndex);
					break;
				case type_color:
					DuplicateVertex<CqColor, CqColor>(*iUP, pVertex, iIndex);
					break;
				case type_hpoint:
					DuplicateVertex<CqVector4D, CqVector3D>(*iUP, pVertex, iIndex);
					break;
				case type_string:
					//DuplicateVertex<CqString, CqString>(*iUP, pVertex, iIndex);
					break;
				case type_matrix:
					//DuplicateVertex<CqMatrix, CqMatrix>(*iUP, pVertex, iIndex);
					break;
				default:
					// left blank to avoid compiler warnings about unhandled types
					break;
			}
		}
	}
}


//------------------------------------------------------------------------------
template<class TypeA, class TypeB>
void CqSubdivision2::CreateEdgeVertex(CqParameter* pParamToModify,
		CqLath* pEdge, TqInt iIndex)
{
	CqParameterTyped<TypeA, TypeB>* pParam
		= static_cast<CqParameterTyped<TypeA, TypeB>*>(pParamToModify);
	for(TqInt arrayindex = 0, arraysize = pParam->Count();
			arrayindex < arraysize; arrayindex++ )
	{
		TypeA A = TypeA(0.0f);
		TypeA B = TypeA(0.0f);
		TypeA C = TypeA(0.0f);

		if(pParam->Class() == class_vertex || pParam->Class() == class_facevertex)
		{
			bool disctsFaceVertex = false;
			// Get a pointer to the appropriate index accessor function on CqLath based on class.
			TqInt (CqLath::*IndexFunction)() const;
			if( pParam->Class() == class_vertex )
				IndexFunction = &CqLath::VertexIndex;
			else
			{
				IndexFunction = &CqLath::FaceVertexIndex;

				// If either of the adjoining vertices are discontinuous, this
				// edge should also be - make sure to interpolate it as a fully
				// hard edge.
				disctsFaceVertex = isDiscontinuousFaceVertexEdge(pParam, pEdge,
						arrayindex);
			}

			if( NULL != pEdge->ec() && !disctsFaceVertex)
			{
				// Edge point is the average of the centrepoint of the original edge and the
				// average of the two new face points of the adjacent faces.
				std::vector<CqLath*> aQef;
				pEdge->Qef( aQef );
				std::vector<CqLath*>::iterator iF;
				for( iF = aQef.begin(); iF != aQef.end(); iF++ )
				{
					std::vector<CqLath*> aQfv;
					(*iF)->Qfv(aQfv);
					std::vector<CqLath*>::iterator iV;
					TypeA Val = TypeA(0.0f);
					for( iV = aQfv.begin(); iV != aQfv.end(); iV++ )
						Val += pParam->pValue( ((*iV)->*IndexFunction)() )[arrayindex];
					Val = static_cast<TypeA>( Val / static_cast<TqFloat>( aQfv.size() ) );
					C += Val;
				}
				C = static_cast<TypeA>( C / static_cast<TqFloat>(aQef.size()) );

				A = pParam->pValue( (pEdge->*IndexFunction)() )[arrayindex];
				B = pParam->pValue( (pEdge->ccf()->*IndexFunction)() )[arrayindex];

				float h = EdgeSharpness( pEdge );
				A = static_cast<TypeA>( ((1.0f+h)*(A+B)) / 2.0f );
				A = static_cast<TypeA>( (A + (1.0f-h)*C) / 2.0f );
			}
			else
			{
				// Edge point lies on a boundary - in this case it's just the
				// average of the two adjoining boundary vertices.
				A = pParam->pValue( (pEdge->*IndexFunction)() )[arrayindex];
				B = pParam->pValue( (pEdge->ccf()->*IndexFunction)() )[arrayindex];
				A = static_cast<TypeA>( (A+B)/2.0f );
			}
		}
		else
		{
			// Get a pointer to the appropriate index accessor function on CqLath based on class.
			TqInt (CqLath::*IndexFunction)() const;
			if( pParam->Class() == class_varying )
				IndexFunction = &CqLath::VertexIndex;
			else
				IndexFunction = &CqLath::FaceVertexIndex;

			A = pParam->pValue( (pEdge->*IndexFunction)() )[arrayindex];
			B = pParam->pValue( (pEdge->ccf()->*IndexFunction)() )[arrayindex];
			A = static_cast<TypeA>( (A+B)/2.0f );
		}
		pParam->pValue( iIndex )[arrayindex] = A;
	}
}

//------------------------------------------------------------------------------
/**
 *	Add a completely new vertex to the list.
 *	Appends a new vertex to the end of the list, updating the referencing
 *	table as well.
 *
 *	@return			The index of the new point.
 */
void CqSubdivision2::AddEdgeVertex(CqLath* pVertex, TqInt& iVIndex, TqInt& iFVIndex )
{
	iFVIndex=0;

	// If -1 is passed in as the 'vertex' class index, we must create a new value.
	bool fNewVertex = iVIndex < 0;

	std::vector<CqParameter*>::iterator iUP;
	TqInt iTime;

	for( iTime = 0; iTime < cTimes(); iTime ++ )
	{
		for ( iUP = pPoints( iTime )->aUserParams().begin(); iUP != pPoints( iTime )->aUserParams().end(); iUP++ )
		{
			TqInt iIndex = ( *iUP )->Size();
			// Store the index in the return variable based on its type.
			if( ( *iUP )->Class() == class_vertex || ( *iUP )->Class() == class_varying )
			{
				if( fNewVertex )
				{
					assert( iVIndex<0 || iVIndex==iIndex );
					iVIndex=iIndex;
					( *iUP )->SetSize( iIndex+1 );
					// Resize the vertex lath
					m_aapVertices.resize(iVIndex+1);
				}
				else
					continue;
			}
			else if( ( *iUP )->Class() == class_facevarying  || ( *iUP )->Class() == class_facevertex )
			{
				assert( iFVIndex==0 || iFVIndex==iIndex );
				iFVIndex = iIndex;
				( *iUP )->SetSize( iIndex+1 );
			}
			else
				continue;

			switch((*iUP)->Type())
			{
				case type_float:
					CreateEdgeVertex<TqFloat, TqFloat>( *iUP, pVertex, iIndex );
					break;
				case type_integer:
					CreateEdgeVertex<TqInt, TqFloat>(*iUP, pVertex, iIndex);
					break;
				case type_point:
				case type_normal:
				case type_vector:
					CreateEdgeVertex<CqVector3D, CqVector3D>(*iUP, pVertex, iIndex);
					break;
				case type_color:
					CreateEdgeVertex<CqColor, CqColor>(*iUP, pVertex, iIndex);
					break;
				case type_hpoint:
					CreateEdgeVertex<CqVector4D, CqVector3D>(*iUP, pVertex, iIndex);
					break;
				case type_string:
					//CreateEdgeVertex<CqString, CqString>(*iUP, pVertex, iIndex);
					break;
				case type_matrix:
					//CreateEdgeVertex<CqMatrix, CqMatrix>(*iUP, pVertex, iIndex);
					break;
				default:
					// left blank to avoid compiler warnings about unhandled types
					break;
			}
		}
	}
}


//------------------------------------------------------------------------------
template<class TypeA, class TypeB>
void CqSubdivision2::CreateFaceVertex(CqParameter* pParamToModify,
		CqLath* pFace, TqInt iIndex)
{
	CqParameterTyped<TypeA, TypeB>* pParam
		= static_cast<CqParameterTyped<TypeA, TypeB>*>(pParamToModify);
	// Get a pointer to the appropriate index accessor function on CqLath based on class.
	TqInt (CqLath::*IndexFunction)() const;
	if( pParam->Class() == class_vertex || pParam->Class() == class_varying)
		IndexFunction = &CqLath::VertexIndex;
	else
		IndexFunction = &CqLath::FaceVertexIndex;
	// Face point is just the average of the original faces vertices.
	std::vector<CqLath*> aQfv;
	pFace->Qfv(aQfv);
	for(TqInt arrayindex = 0, arraysize = pParam->Count();
			arrayindex < arraysize; arrayindex++ )
	{
		std::vector<CqLath*>::iterator iV;
		TypeA Val = TypeA(0.0f);
		for( iV = aQfv.begin(); iV != aQfv.end(); iV++ )
		{
			assert( ((*iV)->*IndexFunction)() >= 0 &&
					((*iV)->*IndexFunction)() < static_cast<TqInt>(pParam->Size()) );
			Val += pParam->pValue( ((*iV)->*IndexFunction)() )[arrayindex];
		}
		Val = static_cast<TypeA>( Val / static_cast<TqFloat>( aQfv.size() ) );
		pParam->pValue( iIndex )[arrayindex] = Val;
	}
}


//------------------------------------------------------------------------------
/**
 *	Add a completely new vertex to the list.
 *	Appends a new vertex to the end of the list, updating the referencing
 *	table as well.
 *
 *	@return			The index of the new point.
 */
void CqSubdivision2::AddFaceVertex(CqLath* pVertex, TqInt& iVIndex, TqInt& iFVIndex)
{
	iVIndex=0;
	iFVIndex=0;

	std::vector<CqParameter*>::iterator iUP;
	TqInt iTime;

	for( iTime = 0; iTime < cTimes(); iTime++ )
	{
		for ( iUP = pPoints( iTime )->aUserParams().begin(); iUP != pPoints( iTime )->aUserParams().end(); iUP++ )
		{
			if( ( *iUP )->Class() == class_uniform )
				continue;

			TqInt iIndex = ( *iUP )->Size();
			( *iUP )->SetSize( iIndex+1 );
			// Store the index in the return variable based on its type.
			if( ( *iUP )->Class() == class_vertex || ( *iUP )->Class() == class_varying )
			{
				assert( iVIndex==0 || iVIndex==iIndex );
				iVIndex = iIndex;
			}
			else if( ( *iUP )->Class() == class_facevarying  || ( *iUP )->Class() == class_facevertex )
			{
				assert( iFVIndex==0 || iFVIndex==iIndex );
				iFVIndex = iIndex;
			}

			switch((*iUP)->Type())
			{
				case type_float:
					CreateFaceVertex<TqFloat, TqFloat>(*iUP, pVertex, iIndex );
					break;
				case type_integer:
					CreateFaceVertex<TqInt, TqFloat>(*iUP, pVertex, iIndex );
					break;
				case type_point:
				case type_normal:
				case type_vector:
					CreateFaceVertex<CqVector3D, CqVector3D>(*iUP, pVertex, iIndex );
					break;
				case type_color:
					CreateFaceVertex<CqColor, CqColor>(*iUP, pVertex, iIndex );
					break;
				case type_hpoint:
					CreateFaceVertex<CqVector4D, CqVector3D>(*iUP, pVertex, iIndex );
					break;
				case type_string:
					//CreateFaceVertex<CqString, CqString>(*iUP, pVertex, iIndex );
					break;
				case type_matrix:
					//CreateFaceVertex<CqMatrix, CqMatrix>(*iUP, pVertex, iIndex );
					break;
				default:
					// left blank to avoid compiler warnings about unhandled types
					break;
			}
		}
	}

	// Resize the vertex lath
	m_aapVertices.resize(iVIndex+1);
}


//------------------------------------------------------------------------------
/**
 *	Add a new facet to the topology structure.
 *	Adds the facet by adding new laths for the specified vertex indices, and
 *	linking them to each other clockwise about the facet. By convention, as
 *	outside of the topology structure facets are stored counter clockwise, the
 *	vertex indices should be passed to this function as counter clockwise and
 *	they will be internally altered to specify the facet as clockwise.
 *
 *	@param	cVerts		The number of vertices in the facet.
 *	@param	pIndices	Pointer to an array of vertex indices.
 *
 *	@return				Pointer to one of the laths which represent this new
 *						facet in the topology structure.
 */
CqLath* CqSubdivision2::AddFacet(TqInt cVerts, TqInt* pIndices, TqInt iFVIndex)
{
	CqLath* pLastLath=NULL;
	CqLath* pFirstLath=NULL;
	// Add the laths for this facet, referencing the appropriate vertexes as we go.
	for(TqInt iVert = 0; iVert < cVerts; iVert++)
	{
		CqLath* pNewLath = new CqLath();
		pNewLath->SetVertexIndex(pIndices[iVert]);
		pNewLath->SetFaceVertexIndex(iFVIndex+iVert);

		if(pLastLath)
			pNewLath->SetpClockwiseFacet(pLastLath);

		m_apLaths.push_back(pNewLath);
		pLastLath = pNewLath;
		if(iVert == 0)
			pFirstLath = pLastLath;

		// We also need to keep up to date a complete list of which laths refer to which
		// vertices to aid us in finalising the topology structure later.
		m_aapVertices[pIndices[iVert]].push_back(pLastLath);
	}
	// complete the chain by linking the last one as the next clockwise one to the first.
	pFirstLath->SetpClockwiseFacet(pLastLath);

	// Add the start lath in as the one referring to this facet in the list.
	m_apFacets.push_back(pFirstLath);

	return(pFirstLath);
}

//------------------------------------------------------------------------------
/**
 *	Add a new facet to the topology structure.
 *	Adds the facet by adding new laths for the specified vertex indices, and
 *	linking them to each other clockwise about the facet. By convention, as
 *	outside of the topology structure facets are stored counter clockwise, the
 *	vertex indices should be passed to this function as counter clockwise and
 *	they will be internally altered to specify the facet as clockwise.
 *
 *	@param	cVerts		The number of vertices in the facet.
 *	@param	pIndices	Pointer to an array of vertex indices.
 *	@param	pFIndices	Pointer to an array of face vertex indices.
 *
 *	@return				Pointer to one of the laths which represent this new
 *						facet in the topology structure.
 */
CqLath* CqSubdivision2::AddFacet(TqInt cVerts, TqInt* pIndices, TqInt* pFVIndices)
{
	CqLath* pLastLath=NULL;
	CqLath* pFirstLath=NULL;
	// Add the laths for this facet, referencing the appropriate vertexes as we go.
	for(TqInt iVert = 0; iVert < cVerts; iVert++)
	{
		CqLath* pNewLath = new CqLath();
		pNewLath->SetVertexIndex(pIndices[iVert]);
		pNewLath->SetFaceVertexIndex(pFVIndices[iVert]);

		if(pLastLath)
			pNewLath->SetpClockwiseFacet(pLastLath);

		m_apLaths.push_back(pNewLath);
		pLastLath = pNewLath;
		if(iVert == 0)
			pFirstLath = pLastLath;

		// We also need to keep up to date a complete list of which laths refer to which
		// vertices to aid us in finalising the topology structure later.
		m_aapVertices[pIndices[iVert]].push_back(pLastLath);
	}
	// complete the chain by linking the last one as the next clockwise one to the first.
	pFirstLath->SetpClockwiseFacet(pLastLath);

	// Add the start lath in as the one referring to this facet in the list.
	m_apFacets.push_back(pFirstLath);

	return(pFirstLath);
}


CqSubdivision2* CqSubdivision2::Clone() const
{
	// Create a clone of the points class.
	boost::shared_ptr<CqPolygonPoints> newPoints(static_cast<CqPolygonPoints*>(pPoints()->Clone()));

	// Create a clone of the sds, and rebuild it with the data in this one.
	CqSubdivision2* clone = new CqSubdivision2(newPoints);
	clone->Prepare(cVertices());

	clone->m_bInterpolateBoundary = m_bInterpolateBoundary;
	clone->m_mapHoles = m_mapHoles;

	// Create the faces in the new surface.
	TqInt i;
	for(i=0; i<cFacets(); i++)
	{
		// Read the facet indices.
		const CqLath* faceLath = pFacet(i);
		std::vector<const CqLath*> Qfv;
		faceLath->Qfv(Qfv);
		TqInt* pV = new TqInt[Qfv.size()];
		TqInt* pFV = new TqInt[Qfv.size()];
		std::vector<const CqLath*>::iterator j;
		TqInt index = 0;
		for(j=Qfv.begin(); j!=Qfv.end(); j++, index++)
		{
			pV[index] = (*j)->VertexIndex();
			pFV[index] = (*j)->FaceVertexIndex();
		}
		clone->AddFacet(Qfv.size(), pV, pFV);
		delete[](pV);
		delete[](pFV);
	}
	clone->Finalise();

	return(clone);
}

//------------------------------------------------------------------------------
/**
 *	Finalise the linkage of the laths.
 *	After adding vertices and facets, call this to complete the linkage of the
 *	laths. To overcome any non-manifold areas in the mesh, this function may
 *	change the topology in order to produce a manifold mesh, or series of
 *	manifold meshes. This also means that all facets in the mesh may no longer
 *	be joined in a complete loop, so care must be taken when traversing the
 *	topology to ensure that all facets are processed.
 */
bool CqSubdivision2::Finalise()
{
	CqString objname( "unnamed" );
	const CqString* pattrName = pPoints()->pAttributes()->GetStringAttribute( "identifier", "name" );
	if ( pattrName != 0 )
		objname = pattrName[ 0 ];
	std::vector<std::vector<CqLath*> >::iterator ivert;
	for(TqInt i = 0; i < static_cast<TqInt>(m_aapVertices.size()); ++i)
	{
		ivert = m_aapVertices.begin() + i;
		TqInt cLaths = (*ivert).size();

		// If there is only one lath, it can't be connected to anything.
		if(cLaths<=1)
			continue;

		// Create an array for the laths on this vertex that have been visited.
		std::vector<bool>  aVisited;
		aVisited.resize(cLaths);
		TqInt cVisited = 0;

		// Initialise it to all false.
		aVisited.assign(cLaths, false);

		CqLath* pCurrent = (*ivert)[0];
		CqLath* pStart = pCurrent;
		TqInt iCurrent = 0;
		TqInt iStart = 0;

		bool fDone = false;
		while(!fDone)
		{
			// Find a clockwise vertex match for the counterclockwise vertex index of this lath.
			TqInt ccwVertex = pCurrent->ccf()->VertexIndex();
			TqInt iLath = 0;
			for(iLath = 0; iLath < cLaths; iLath++)
			{
				// Only check non-visited laths.
				if(!aVisited[iLath] && (*ivert)[iLath]->cf()->VertexIndex() == ccwVertex)
				{
					pCurrent->SetpClockwiseVertex((*ivert)[iLath]);
					pCurrent = (*ivert)[iLath];
					iCurrent = iLath;
					// Mark the linked to lath as visited.
					aVisited[iLath] = true;
					cVisited++;

					break;
				}
			}
			// If we didn't find a match then we are done.
			fDone = iLath==cLaths;
		}

		// If the last lath wasn't linked, then we have a boundary condition, so
		// start again from the initial lath and process backwards.
		if(NULL == pCurrent->cv())
		{
			fDone = false;
			while(!fDone)
			{
				// Find a counterclockwise vertex match for the clockwise vertex index of this lath.
				TqInt cwVertex = pStart->cf()->VertexIndex();
				TqInt iLath = 0;
				for(iLath = 0; iLath < cLaths; iLath++)
				{
					// Only check non-visited laths.
					if(!aVisited[iLath] && (*ivert)[iLath]->ccf()->VertexIndex() == cwVertex)
					{
						// Link the current to the match.
						(*ivert)[iLath]->SetpClockwiseVertex(pStart);
						// Mark the linked to lath as visited.
						aVisited[iStart] = true;
						cVisited++;
						pStart = (*ivert)[iLath];
						iStart = iLath;

						break;
					}
				}
				// If we didn't find a match then we are done.
				fDone = iLath==cLaths;
			}
		}
		aVisited[iStart] = true;
		cVisited++;
		// If we have not visited all the laths referencing this vertex, then we have a non-manifold situation.
		if(cVisited < cLaths)
		{
			Aqsis::log() << error << "Found a non-manifold vertex in the control hull of object \"" << objname.c_str() << "\" at vertex " << pCurrent->VertexIndex() << std::endl;
			// Now create a duplicate vertex at the same position as this one, and move all
			// remaining laths to point to that.
			TqInt iNewVert=-1, iNewFVert;
			DuplicateVertex(pCurrent, iNewVert, iNewFVert);
			
			for(TqInt iLath = 0; iLath < static_cast<TqInt>(m_aapVertices[i].size()); ++iLath)
			{
				if(!aVisited[iLath])
				{
					std::vector<CqLath*>::iterator lath = m_aapVertices[i].begin() + iLath;
					(*lath)->SetVertexIndex(iNewVert);
					(*lath)->SetFaceVertexIndex(iNewFVert);
					m_aapVertices[iNewVert].push_back((*lath));
					m_aapVertices[i].erase(lath);
					aVisited.erase(aVisited.begin() + iLath);
					--iLath;
				}
			}
		}
	}

	m_fFinalised = true;
	return( true );
}


#define modulo(a, b) (a * b >= 0 ? a % b : (a % b) + b)
struct SqFaceLathList
{
	CqLath* pA, *pB, *pC, *pD;
};

void CqSubdivision2::SubdivideFace(CqLath* pFace, std::vector<CqLath*>& apSubFaces)
{
	assert(pFace);

	// If this has already been subdivided then skip it.
	if( pFace->pFaceVertex() )
	{
		apSubFaces.clear();
		std::vector<CqLath*> aQvf;
		pFace->pFaceVertex()->Qvf(aQvf);
		// Fill in the lath references for the starting points of the faces.
		// Reorder them so that they are all in the same orientaion as their parent, if possible.
		// This is due to the fact that the subdivision algorithm results in 4 quads with the '2' vertex
		// in the middle point, we need to rotate them to restore the original orientation by choosing the
		// next one round for each subsequent quad.
		std::vector<CqLath*>::iterator iVF;
		TqInt i = 0;
		for( iVF = aQvf.begin(); iVF != aQvf.end(); iVF++, i++ )
		{
			CqLath* pLathF = (*iVF)->ccf()->ccf();
			TqInt r = i;
			while( r-- > 0)
				pLathF = pLathF->ccf();
			apSubFaces.push_back( pLathF );
		}
		return;
	}

	// First make sure that the appropriate neighbour facets have been subdivided if this is >0 level face.
	if( pFace->pParentFacet() )
	{
		// loop through all our neighbour faces.
		// we don't use Qff here because we can handle multiple copies
		// of each face faster than it can.
		std::vector<CqLath*> parentVertices;
		pFace->pParentFacet()->Qfv( parentVertices );

		std::vector<CqLath*>::iterator vertexIt;
		std::vector<CqLath*>::iterator vertexEnd = parentVertices.end();
		for( vertexIt = parentVertices.begin(); vertexIt != vertexEnd; ++vertexIt )
			subdivideNeighbourFaces(*vertexIt);
	}

	std::vector<CqLath*> aQfv;
	std::vector<TqInt> aVertices;
	std::vector<TqInt> aFVertices;

	pFace->Qfv(aQfv);
	TqInt n = aQfv.size();

	aVertices.resize((2*n)+1);
	aFVertices.resize((2*n)+1);

	// Clear the return array for subdface indices.
	apSubFaces.clear();

	// First of all setup the points.
	TqInt i;

	// Create new point for the face midpoint.
	TqInt iVert=-1, iFVert=-1;
	AddFaceVertex(pFace, iVert, iFVert);

	// Store the index, for later lath creation
	aVertices[2*n] = iVert;
	aFVertices[2*n] = iFVert;

	// Create new points for the edge midpoints.
	for(i = 0; i < n; i++)
	{
		TqInt iVert=-1, iFVert=-2;
		// Create new vertices for the edge mid points.
		if( aQfv[i]->ec() && NULL != aQfv[i]->ec()->pMidVertex() )
			// There is already a next level vertex for this, so reuse the 'vertex' class index.
			iVert = aQfv[i]->ec()->pMidVertex()->VertexIndex();
		// Create new vertex for the edge midpoint.
		AddEdgeVertex(aQfv[i], iVert, iFVert);

		// Store the index, for later lath creation
		aVertices[i+n] = iVert;
		aFVertices[i+n] = iFVert;
	}

	// Create new points for the existing vertices
	for(i = 0; i < n; i++)
	{
		TqInt iVert=-1, iFVert=-3;
		// Create new vertices for the original points.
		if( aQfv[i]->pChildVertex() )
			// There is already a next level vertex for this, so reuse the 'vertex' class index.
			iVert = aQfv[i]->pChildVertex()->VertexIndex();

		// Create a new vertex for the next level
		AddVertex(aQfv[i], iVert, iFVert);

		// Store the index, for later lath creation
		aVertices[i] = iVert;
		aFVertices[i] = iFVert;
	}

	// Now create new laths for the new facets
	std::vector<SqFaceLathList>	apFaceLaths;
	apFaceLaths.resize(n);

	for( i = 0; i < n; i++ )
	{
		// For each facet, create 4 laths and join them in the order of the facet
		CqLath* pLathA = apFaceLaths[i].pA = new CqLath( aVertices[i], aFVertices[i] );
		m_apLaths.push_back(pLathA);
		CqLath* pLathB = apFaceLaths[i].pB = new CqLath( aVertices[(modulo((i+1),n))+n], aFVertices[(modulo((i+1),n))+n] );
		m_apLaths.push_back(pLathB);
		CqLath* pLathC = apFaceLaths[i].pC = new CqLath( aVertices[2*n], aFVertices[2*n] );
		m_apLaths.push_back(pLathC);
		CqLath* pLathD = apFaceLaths[i].pD = new CqLath( aVertices[i+n], aFVertices[i+n] );
		m_apLaths.push_back(pLathD);
		pLathA->SetpClockwiseFacet(pLathB);
		pLathB->SetpClockwiseFacet(pLathC);
		pLathC->SetpClockwiseFacet(pLathD);
		pLathD->SetpClockwiseFacet(pLathA);
		// Attach the new face laths to the corner of the parent facet from
		// which this face is decended.
		pLathA->SetpParentFacet(aQfv[i]);
		pLathB->SetpParentFacet(aQfv[i]);
		pLathC->SetpParentFacet(aQfv[i]);
		pLathD->SetpParentFacet(aQfv[i]);

		// Fill in the vertex references table for these vertices.
		m_aapVertices[pLathA->VertexIndex()].push_back(pLathA);
		m_aapVertices[pLathB->VertexIndex()].push_back(pLathB);
		m_aapVertices[pLathC->VertexIndex()].push_back(pLathC);
		m_aapVertices[pLathD->VertexIndex()].push_back(pLathD);

		// Set the child vertex pointer for all laths which reference the A vertex of this facet
		// so that we can use them when subdividing other faces.
		CqLath* pNextV = aQfv[i];
		do
		{
			pNextV->SetpChildVertex(pLathA);
			pNextV = pNextV->cv();
		}
		while( pNextV && pNextV != aQfv[i]);
		// Make sure that if we have hit a boundary, we go backwards from the start point until we hit the boundary that
		// way as well.
		if(NULL == pNextV)
		{
			pNextV = aQfv[i]->ccv();
			// We know we are going to hit a boundary in this direction as well so we can just look for that
			// case as a terminator.
			while( pNextV )
			{
				assert( pNextV != aQfv[i] );
				pNextV->SetpChildVertex(pLathA);
				pNextV = pNextV->ccv();
			}
		}

		// For this edge of the original face, set a ponter to the new midpoint lath, so that we can
		// use it when subdividing neighbour facets.
		aQfv[i]->SetpMidVertex(pLathD);

		// Transfer sharpness information
		float sharpness = EdgeSharpness( aQfv[ i ] );
		if( sharpness > 0.0f )
			AddSharpEdge( pLathA, sharpness * sharpness );

		sharpness = EdgeSharpness( aQfv[ modulo( (i+1),n ) ] );
		if( sharpness > 0.0f )
			AddSharpEdge( pLathB, sharpness * sharpness );

		if( CornerSharpness( aQfv[ i ] ) > 0.0f )
			AddSharpCorner( pLathA, CornerSharpness( aQfv[ i ] ) );

		// Fill in the lath references for the starting points of the faces.
		// Reorder them so that they are all in the same orientaion as their parent, if possible.
		// This is due to the fact that the subdivision algorithm results in 4 quads with the '2' vertex
		// in the middle point, we need to rotate them to restore the original orientation by choosing the
		// next one round for each subsequent quad.
		CqLath* pLathF = pLathA;
		TqInt r = i;
		while( r-- > 0)
			pLathF = pLathF->ccf();
		apSubFaces.push_back( pLathF );
		m_apFacets.push_back( pLathF );
	}

	// Now connect up the laths we have created.
	// The clcckwise face connections will have already been made, we need to fixup and clockwise
	// vertex connections we can.
	for( i = 0; i < n; i++ )
	{
		// Set the facet point reference for all laths representing this facet.
		aQfv[i]->SetpFaceVertex(apFaceLaths[i].pC);
		// Connect midpoints clockwise vertex pointers.
		apFaceLaths[((i+1)%n)].pD->SetpClockwiseVertex( apFaceLaths[i].pB );
		// Connect all laths around the new face point.
		apFaceLaths[i].pC->SetpClockwiseVertex( apFaceLaths[ ((i+1)%n) ].pC );

		// Connect the new corner vertices, this is only possible if neighbouring facets have previously been
		// subdivided.
		std::vector<CqLath*>::iterator iVertLath;
		for( iVertLath = m_aapVertices[apFaceLaths[i].pA->VertexIndex()].begin(); iVertLath != m_aapVertices[apFaceLaths[i].pA->VertexIndex()].end(); iVertLath++ )
		{
			if( (*iVertLath)->cf()->VertexIndex() == apFaceLaths[i].pD->VertexIndex() )
				apFaceLaths[i].pA->SetpClockwiseVertex( (*iVertLath ) );
			if( (*iVertLath)->ccf()->VertexIndex() == apFaceLaths[i].pB->VertexIndex() )
				(*iVertLath)->SetpClockwiseVertex( apFaceLaths[i].pA );
		}
	}

	for( i = 0; i < n; i++ )
	{
		// Connect the new edge midpoint vertices to any neighbours, this is only possible if neighbouring facets have previously been
		// subdivided.
		std::vector<CqLath*>::iterator iVertLath;
		for( iVertLath = m_aapVertices[apFaceLaths[i].pB->VertexIndex()].begin(); iVertLath != m_aapVertices[apFaceLaths[i].pB->VertexIndex()].end(); iVertLath++ )
		{
			if( (*iVertLath)->cf()->VertexIndex() == apFaceLaths[i].pA->VertexIndex() )
				apFaceLaths[i].pB->SetpClockwiseVertex( (*iVertLath ) );
		}
		for( iVertLath = m_aapVertices[apFaceLaths[i].pD->VertexIndex()].begin(); iVertLath != m_aapVertices[apFaceLaths[i].pD->VertexIndex()].end(); iVertLath++ )
		{
			if( (*iVertLath)->ccf()->VertexIndex() == apFaceLaths[i].pA->VertexIndex() )
				(*iVertLath )->SetpClockwiseVertex( apFaceLaths[i].pD );
		}
	}
	//OutputInfo("out.dat");
}

/// Subdivide all faces around the given vertex.
void CqSubdivision2::subdivideNeighbourFaces(CqLath* vert)
{
	CqLath* f = vert;
	std::vector<CqLath*> dummySubFaces;
	// Step over all the faces sourrounding the vertex, and make sure they're
	// subdivided.
	do
	{
		if(!f->pFaceVertex())
			SubdivideFace(f, dummySubFaces);
		f = f->cv();
	}
	while(f && f != vert);
	if(!f)
	{
		// If f was NULL at the end of the previous loop then a boundary was
		// encountered and we need to go in the other direction too.
		f = vert->ccv();
		while(f)
		{
			if(!f->pFaceVertex())
				SubdivideFace(f, dummySubFaces);
			f = f->ccv();
		}
	}
}

void CqSubdivision2::OutputMesh(const char* fname, std::vector<CqLath*>* paFaces)
{
	std::ofstream file(fname);
	std::vector<CqLath*> aQfv;

	TqInt i;
	for( i = 0; i < cVertices(); i++ )
	{
		CqVector3D vec = vectorCast<CqVector3D>(pPoints()->P()->pValue()[ pVertex( i )->VertexIndex() ]);
		file << "v " << vec.x() << " " << vec.y() << " " << vec.z() << std::endl;
	}


	for(i = 0; i < cFacets(); i++)
	{
		if( NULL == pFacet(i)->pFaceVertex())
		{
			pFacet(i)->Qfv(aQfv);
			TqUint j;
			file << "f ";
			for( j = 0; j < aQfv.size(); j++ )
				file << aQfv[j]->VertexIndex()+1 << " ";
			file << std::endl;
		}
	}

	if( paFaces)
	{
		file << "g CurrentFace" << std::endl;
		for(i = 0; i < static_cast<TqInt>( paFaces->size() )
		        ;
		        i++)
		{
			(*paFaces)[i]->Qfv(aQfv);
			TqUint j;
			file << "f ";
			for( j = 0; j < aQfv.size(); j++ )
				file << aQfv[j]->VertexIndex()+1 << " ";
			file << std::endl;
		}
	}

	file.close();
}


void CqSubdivision2::OutputInfo(const char* fname, std::vector<CqLath*>* paFaces)
{
	std::ofstream file(fname);
	std::vector<CqLath*> aQfv;

	std::vector<CqLath*>* paLaths = paFaces;

	if( NULL == paLaths )
		paLaths = &m_apFacets;

	paLaths = &m_apLaths;

	CqMatrix matCameraToObject0;
	QGetRenderContext() ->matSpaceToSpace( "camera", "object", NULL, pPoints()->pTransform().get(), pPoints()->pTransform()->Time(0), matCameraToObject0 );

	for(TqUint i = 0; i < paLaths->size(); i++)
	{
		CqLath* pL = (*paLaths)[i];
		file << i << " - " << pL << " - "	<<
		pL->VertexIndex() << " - " <<
		pL->FaceVertexIndex() << " - (cf) ";
		if( pL->cf() )
			file << pL->cf();
		else
			file << "***";
		file << " - (cv) ";

		if(pL->cv())
			file << pL->cv();
		else
			file << "***";

		CqVector3D vecP = vectorCast<CqVector3D>(pPoints()->P()->pValue(pL->VertexIndex())[0]);
		vecP = matCameraToObject0 * vecP;
		file << "[P=" << vecP << "]";

		file << std::endl;
	}

	file.close();
}


void	CqSurfaceSubdivisionPatch::Bound(CqBound* bound) const
{
	assert( pTopology() );
	assert( pTopology()->pPoints() );
	assert( pFace() );

	// First make sure that the appropriate neighbour facets have been subdivided if this is >0 level face.
	if( pFace()->pParentFacet() )
	{
		std::vector<CqLath*> aQff;
		std::vector<CqLath*> apSubFaces2;
		pFace()->pParentFacet()->Qff( aQff );
		std::vector<CqLath*>::iterator iF;
		for( iF = aQff.begin(); iF != aQff.end(); iF++ )
		{
			CqLath* face = *iF;
			if (NULL == face->pFaceVertex())
				pTopology()->SubdivideFace(face, apSubFaces2);
		}
	}


	// Get the laths of the surrounding faces.
	std::vector<CqLath*> aQff;
	pFace()->Qff(aQff);
	std::vector<CqLath*>::iterator iFF;
	for( iFF = aQff.begin(); iFF != aQff.end(); iFF++ )
	{
		// Get the laths that reference the vertices of this face
		std::vector<CqLath*> aQfv;
		(*iFF)->Qfv(aQfv);

		// Now get the vertices, and form the bound.
		std::vector<CqLath*>::iterator iQfv;
		for( iQfv = aQfv.begin(); iQfv != aQfv.end(); iQfv++ )
		{
			TqInt iTime;
			for( iTime = 0; iTime < pTopology()->cTimes(); iTime++ )
				bound->Encapsulate(vectorCast<CqVector3D>(pTopology()->pPoints( iTime )->P()->pValue((*iQfv)->VertexIndex())[0]));
		}
	}

	AdjustBoundForTransformationMotion( bound );
}


CqMicroPolyGridBase* CqSurfaceSubdivisionPatch::Dice()
{
	boost::shared_ptr<CqSubdivision2> pSurface;
	std::vector<CqMicroPolyGridBase*> apGrids;

	pSurface = Extract(0);
	boost::shared_ptr<CqSurfaceSubdivisionPatch> pPatch( new CqSurfaceSubdivisionPatch(pSurface, pSurface->pFacet(0), 0) );
	pPatch->m_uDiceSize = m_uDiceSize;
	pPatch->m_vDiceSize = m_vDiceSize;
	CqMicroPolyGridBase* pGrid = pPatch->DiceExtract();
	return pGrid;
}


/** Dice the patch this primitive represents.
 * Subdivide recursively the appropriate number of times, then extract the information into 
 * a MPG structure.
 */

CqMicroPolyGridBase* CqSurfaceSubdivisionPatch::DiceExtract()
{
	// Dice rate table                  0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16
	static const TqInt aDiceSizes[] = { 0, 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };
	                                  
	assert( pTopology() );
	assert( pTopology()->pPoints() );
	assert( pFace() );

	TqInt dicesize = min(max(m_uDiceSize, m_vDiceSize), 16);

	TqInt sdcount = aDiceSizes[ dicesize ];
	dicesize = 1 << sdcount;
	TqInt lUses = Uses();

	std::vector<CqMicroPolyGrid*> apGrids;

	TqInt iTime;
	for( iTime = 0; iTime < pTopology()->cTimes(); iTime++ )
	{
		CqMicroPolyGrid* pGrid = new CqMicroPolyGrid();
		pGrid->Initialise( dicesize, dicesize, pTopology()->pPoints() );

		boost::shared_ptr<CqPolygonPoints> pMotionPoints = pTopology()->pPoints( iTime );

		TqInt isd;
		std::vector<CqLath*> apSubFace1, apSubFace2;
		apSubFace1.push_back(pFace());

		for( isd = 0; isd < sdcount; isd++ )
		{
			apSubFace2.clear();
			std::vector<CqLath*>::iterator iSF;
			for( iSF = apSubFace1.begin(); iSF != apSubFace1.end(); iSF++ )
			{
				// Subdivide this face, storing the resulting new face indices.
				std::vector<CqLath*> apSubFaceTemp;
				pTopology()->SubdivideFace( (*iSF), apSubFaceTemp );
				// Now combine these into the new face indices for this subdivision level.
				apSubFace2.insert(apSubFace2.end(), apSubFaceTemp.begin(), apSubFaceTemp.end());
			}
			// Now swap the new level's indices for the old before repeating at the next level, if appropriate.
			apSubFace1.swap(apSubFace2);
		}

		// Now we use the first face index to start our extraction
		TqInt nc, nr, c, r;
		nc = nr = dicesize;
		r = 0;

		CqLath* pLath, *pTemp;
		pLath = apSubFace1[0];
		pTemp = pLath;

		// Get data from pLath
		TqInt indexA = 0;
		StoreDice( pGrid, pMotionPoints, pLath, indexA );

		indexA++;
		pLath = pLath->ccf();
		c = 0;
		while( c < nc )
		{
			StoreDice(pGrid, pMotionPoints, pLath, indexA);
			if( c < ( nc - 1 ) )
				pLath = pLath->cv()->ccf();

			indexA++;
			c++;
		}
		r++;

		while( r <= nr )
		{
			pLath = pTemp->cf();
			if( r < nr )
				pTemp = pLath->ccv();

			// Get data from pLath
			TqInt indexA = ( r * ( nc + 1 ) );
			StoreDice( pGrid, pMotionPoints, pLath, indexA );
			indexA++;
			pLath = pLath->cf();
			c = 0;
			while( c < nc )
			{
				StoreDice( pGrid, pMotionPoints, pLath, indexA );
				if( c < ( nc - 1 ) )
					pLath = pLath->ccv()->cf();

				indexA++;
				c++;
			}

			r++;
		}
		// If the color and opacity are not defined, use the system values.
		if ( USES( lUses, EnvVars_Cs ) && !pTopology()->pPoints()->bHasVar(EnvVars_Cs) )
		{
			if ( pAttributes() ->GetColorAttribute( "System", "Color" ) )
				pGrid->pVar(EnvVars_Cs) ->SetColor( pAttributes() ->GetColorAttribute( "System", "Color" ) [ 0 ] );
			else
				pGrid->pVar(EnvVars_Cs) ->SetColor( CqColor( 1, 1, 1 ) );
		}

		if ( USES( lUses, EnvVars_Os ) && !pTopology()->pPoints()->bHasVar(EnvVars_Os) )
		{
			if ( pAttributes() ->GetColorAttribute( "System", "Opacity" ) )
				pGrid->pVar(EnvVars_Os) ->SetColor( pAttributes() ->GetColorAttribute( "System", "Opacity" ) [ 0 ] );
			else
				pGrid->pVar(EnvVars_Os) ->SetColor( CqColor( 1, 1, 1 ) );
		}
		apGrids.push_back( pGrid );

		// Fill in u/v if required.
		if ( USES( lUses, EnvVars_u ) && !pTopology()->pPoints()->bHasVar(EnvVars_u) )
		{
			TqInt iv, iu;
			for ( iv = 0; iv <= dicesize; iv++ )
			{
				TqFloat v = ( 1.0f / ( dicesize + 1 ) ) * iv;
				for ( iu = 0; iu <= dicesize; iu++ )
				{
					TqFloat u = ( 1.0f / ( dicesize + 1 ) ) * iu;
					TqInt igrid = ( iv * ( dicesize + 1 ) ) + iu;
					pGrid->pVar(EnvVars_u)->SetFloat( BilinearEvaluate( 0.0f, 1.0f, 0.0f, 1.0f, u, v ), igrid );
				}
			}
		}

		if ( USES( lUses, EnvVars_v ) && !pTopology()->pPoints()->bHasVar(EnvVars_v) )
		{
			TqInt iv, iu;
			for ( iv = 0; iv <= dicesize; iv++ )
			{
				TqFloat v = ( 1.0f / ( dicesize + 1 ) ) * iv;
				for ( iu = 0; iu <= dicesize; iu++ )
				{
					TqFloat u = ( 1.0f / ( dicesize + 1 ) ) * iu;
					TqInt igrid = ( iv * ( dicesize + 1 ) ) + iu;
					pGrid->pVar(EnvVars_v)->SetFloat( BilinearEvaluate( 0.0f, 0.0f, 1.0f, 1.0f, u, v ), igrid );
				}
			}
		}

		// Fill in s/t if required.
		if ( USES( lUses, EnvVars_s ) && !pTopology()->pPoints()->bHasVar(EnvVars_s) )
		{
			pGrid->pVar(EnvVars_s)->SetValueFromVariable( pGrid->pVar(EnvVars_u) );
		}

		if ( USES( lUses, EnvVars_t ) && !pTopology()->pPoints()->bHasVar(EnvVars_t) )
		{
			pGrid->pVar(EnvVars_t)->SetValueFromVariable( pGrid->pVar(EnvVars_v) );
		}
	}

	if( apGrids.size() == 1 )
		return( apGrids[ 0 ] );
	else
	{
		CqMotionMicroPolyGrid * pGrid = new CqMotionMicroPolyGrid;
		TqInt i;
		for ( i = 0; i < pTopology()->cTimes(); i++ )
			pGrid->AddTimeSlot( pTopology()->Time( i ), apGrids[ i ] );
		pGrid->Initialise(dicesize, dicesize, pTopology()->pPoints() );
		return( pGrid );
	}
}

void CqSurfaceSubdivisionPatch::StoreDiceAPVar(
		const boost::shared_ptr<IqShader>& pShader, CqParameter* pParam,
		TqUint ivA, TqInt ifvA, TqUint indexA)
{
	// Find the argument
	IqShaderData * pArg = pShader->FindArgument( pParam->strName() );
	if ( pArg )
	{
		TqInt index = 0;
		switch(pParam->Class())
		{
			case class_constant:
				break;
			case class_uniform:
				index = m_FaceIndex;
				break;
			case class_varying:
			case class_vertex:
				index = ivA;
				break;
			case class_facevarying:
			case class_facevertex:
				index = ifvA;
				break;
			case class_invalid:
				assert(0 && "encountered primvar class_invalid!");
				return;
		}

		switch ( pParam->Type() )
		{
				case type_float:
				{
					CqParameterTyped<TqFloat, TqFloat>* pNParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam );
					pArg->SetValue( *pNParam->pValue( index ), indexA );
				}
				break;

				case type_integer:
				{
					CqParameterTyped<TqInt, TqFloat>* pNParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam );
					pArg->SetValue( *pNParam->pValue( index ), indexA );
				}
				break;

				case type_point:
				case type_vector:
				case type_normal:
				{
					CqParameterTyped<CqVector3D, CqVector3D>* pNParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam );
					pArg->SetValue( *pNParam->pValue( index ), indexA );
				}
				break;

				case type_hpoint:
				{
					CqParameterTyped<CqVector4D, CqVector3D>* pNParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam );
					pArg->SetValue( vectorCast<CqVector3D>(*pNParam->pValue( index )), indexA );
				}
				break;

				case type_string:
				{
					CqParameterTyped<CqString, CqString>* pNParam = static_cast<CqParameterTyped<CqString, CqString>*>( pParam );
					pArg->SetValue( *pNParam->pValue( index ), indexA );
				}
				break;

				case type_color:
				{
					CqParameterTyped<CqColor, CqColor>* pNParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam );
					pArg->SetValue( *pNParam->pValue( index ), indexA );
				}
				break;

				case type_matrix:
				{
					CqParameterTyped<CqMatrix, CqMatrix>* pNParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam );
					pArg->SetValue( *pNParam->pValue( index ), indexA );
				}
				break;

				default:
				{
					// left blank to avoid compiler warnings about unhandled types
				}
				break;
		}
	}
}


void CqSurfaceSubdivisionPatch::StoreDice( CqMicroPolyGrid* pGrid, const boost::shared_ptr<CqPolygonPoints>& pPoints, CqLath* vert, TqInt iData)
{
	TqInt lUses = m_Uses;
	TqInt lDone = 0;

	TqInt iParam = vert->VertexIndex();
	TqInt iFVParam = vert->FaceVertexIndex();

	pGrid->pVar(EnvVars_P)->SetPoint(m_pTopology->limitPoint(vert), iData);

	// Special cases for s and t if "st" exists, it should override s and t.
	CqParameter* pParam;
	if( ( pParam = pPoints->FindUserParam("st") ) != NULL )
	{
		TqInt index = iParam;
		if( pParam->Class() == class_facevarying  || pParam->Class() == class_facevertex )
			index = iFVParam;
		CqParameterTyped<TqFloat, TqFloat>* pSTParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>(pParam);
		if ( USES( lUses, EnvVars_s ) && ( NULL != pGrid->pVar(EnvVars_s) ) )
			pGrid->pVar( EnvVars_s )->SetFloat( pSTParam->pValue( index )[0], iData);
		if ( USES( lUses, EnvVars_t ) && ( NULL != pGrid->pVar(EnvVars_t) ) )
			pGrid->pVar( EnvVars_t )->SetFloat( pSTParam->pValue( index )[1], iData);
		DONE( lDone, EnvVars_s);
		DONE( lDone, EnvVars_t);
	}

	if ( USES( lUses, EnvVars_s ) && ( NULL != pGrid->pVar(EnvVars_s) ) && ( pPoints->bHasVar(EnvVars_s) ) && !isDONE(lDone, EnvVars_s ) )
	{
		if( pPoints->s()->Class() == class_varying || pPoints->s()->Class() == class_vertex )
			pGrid->pVar(EnvVars_s) ->SetFloat( pPoints->s()->pValue( iParam )[0], iData );
		else if( pPoints->s()->Class() == class_facevarying || pPoints->s()->Class() == class_facevertex )
			pGrid->pVar(EnvVars_s) ->SetFloat( pPoints->s()->pValue( iFVParam )[0], iData );
		else if( pPoints->s()->Class() == class_uniform )
			pGrid->pVar(EnvVars_s) ->SetFloat( pPoints->s()->pValue( 0 )[0], iData );
	}

	if ( USES( lUses, EnvVars_t ) && ( NULL != pGrid->pVar(EnvVars_t) ) && ( pPoints->bHasVar(EnvVars_t) ) && !isDONE(lDone, EnvVars_t ) )
	{
		if( pPoints->t()->Class() == class_varying || pPoints->t()->Class() == class_vertex )
			pGrid->pVar(EnvVars_t) ->SetFloat( pPoints->t()->pValue( iParam )[0], iData );
		else if( pPoints->t()->Class() == class_facevarying || pPoints->t()->Class() == class_facevertex )
			pGrid->pVar(EnvVars_t) ->SetFloat( pPoints->t()->pValue( iFVParam )[0], iData );
		else if( pPoints->t()->Class() == class_uniform )
			pGrid->pVar(EnvVars_t) ->SetFloat( pPoints->t()->pValue( 0 )[0], iData );
	}

	if ( USES( lUses, EnvVars_Cs ) && ( pGrid->pVar(EnvVars_Cs) ) && ( pPoints->bHasVar(EnvVars_Cs) ) )
	{
		if( pPoints->Cs()->Class() == class_varying || pPoints->Cs()->Class() == class_vertex )
			pGrid->pVar(EnvVars_Cs) ->SetColor( pPoints->Cs()->pValue(iParam)[0], iData );
		else if( pPoints->Cs()->Class() == class_facevarying || pPoints->Cs()->Class() == class_facevertex )
			pGrid->pVar(EnvVars_Cs) ->SetColor( pPoints->Cs()->pValue(iFVParam)[0], iData );
		else if( pPoints->Cs()->Class() == class_uniform )
			pGrid->pVar(EnvVars_Cs) ->SetColor( pPoints->Cs()->pValue(0)[0], iData );
	}

	if ( USES( lUses, EnvVars_Os ) && ( pGrid->pVar(EnvVars_Os) ) && ( pPoints->bHasVar(EnvVars_Os) ) )
	{
		if( pPoints->Os()->Class() == class_varying || pPoints->Os()->Class() == class_vertex )
			pGrid->pVar(EnvVars_Os) ->SetColor( pPoints->Os()->pValue(iParam)[0], iData );
		else if( pPoints->Os()->Class() == class_facevarying || pPoints->Os()->Class() == class_facevertex )
			pGrid->pVar(EnvVars_Os) ->SetColor( pPoints->Os()->pValue(iFVParam)[0], iData );
		else if( pPoints->Os()->Class() == class_uniform )
			pGrid->pVar(EnvVars_Os) ->SetColor( pPoints->Os()->pValue(0)[0], iData );
	}

	// Now lets store the diced user specified primitive variables.
	std::vector<CqParameter*>::iterator iUP;
	for ( iUP = pPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++ )
	{
		/// \todo: Must transform point/vector/normal/matrix parameter variables from 'object' space to current before setting.
		boost::shared_ptr<IqShader> pShader;
		if ( pShader=pGrid->pAttributes() ->pshadSurface(m_Time) )
			StoreDiceAPVar( pShader, ( *iUP ), iParam, iFVParam, iData );

		if ( pShader=pGrid->pAttributes() ->pshadDisplacement(m_Time) )
			StoreDiceAPVar( pShader, ( *iUP ), iParam, iFVParam, iData );

		if ( pShader=pGrid->pAttributes() ->pshadAtmosphere(m_Time) )
			StoreDiceAPVar( pShader, ( *iUP ), iParam, iFVParam, iData );
	}
}

namespace {

/** \brief Extract vertex and facevertex indices for the neighbourhood of a
 * regular patch.
 *
 * The neighbourhood of a regular interior SDS patch consists of 9 patches.
 * Primitive variables to be interpolated are either attached to the vertices
 * or to the "facevertices".
 *
 *  0-----------1-----------2-----------3   <-- vertex
 *  | 0       1 | 2       3 | 4       5 | <-- facevertex
 *  |           |           |           |
 *  |           |           |           |
 *  | 6       7 | 8       9 | 10     11 |
 *  4-----------5-----------6-----------7
 *  | 12     13 | 14     15 | 16     17 |
 *  |           |           |           |
 *  |           |           |           |
 *  | 18     19 | 20     21 | 22     23 |
 *  8-----------9-----------10----------11
 *  | 24     25 | 26     27 | 28     29 |
 *  |           |           |           |
 *  |           |           |           |
 *  | 30     31 | 32     33 | 34     35 |
 *  12----------13----------14----------15
 *
 *
 * As shown, there are 4x4 vertex indices, and 6x6 face vertex indices.  The
 * numbers shown above show which indices of vertIdx and faceVertIdx correspond
 * to which positions in the neighbourhood of the central patch.
 *
 * \param face - lath for the central patch
 * \param vertIdx - output array for vertex indices
 * \param faceVertIdx - output array for facevertex indices
 */
void getNbhdIndices(CqLath* face, TqInt vertIdx[16], TqInt faceVertIdx[36])
{
	// The following extracts the indices from the lath-based topology; it's a
	// bit messy and difficult to automate, which is why it's written out in
	// full.
	TqInt* vIdx = vertIdx;
	TqInt* fvIdx = faceVertIdx;
#	define VGET *vIdx++ = v->VertexIndex()
#	define FVGET *fvIdx++ = v->FaceVertexIndex()
	// first column of patches
	CqLath* vCol = face->cv()->cv()->cf()->cf();
	CqLath* v = vCol;           VGET; FVGET;
	v = v->ccf();               VGET; FVGET;
	v = v->cv();                      FVGET;
	v = v->ccf();               VGET; FVGET;
	v = v->cv();                      FVGET;
	v = v->ccf();               VGET; FVGET;
	vCol = vCol->cf();
	v = vCol;                         FVGET;
	v = v->cf();                      FVGET;
	v = v->ccv();                     FVGET;
	v = v->cf();                      FVGET;
	v = v->ccv();                     FVGET;
	v = v->cf();                      FVGET;
	// second column of patches
	vCol = vCol->ccv();
	v = vCol;                   VGET; FVGET;
	v = v->ccf();               VGET; FVGET;
	v = v->cv();                      FVGET;
	v = v->ccf();               VGET; FVGET;
	v = v->cv();                      FVGET;
	v = v->ccf();               VGET; FVGET;
	vCol = vCol->cf();
	v = vCol;                   VGET; FVGET;
	v = v->cf();                VGET; FVGET;
	v = v->ccv();                     FVGET;
	v = v->cf();                VGET; FVGET;
	v = v->ccv();                     FVGET;
	v = v->cf();                VGET; FVGET;
	// third column of patches
	vCol = vCol->ccv();
	v = vCol;                         FVGET;
	v = v->ccf();                     FVGET;
	v = v->cv();                      FVGET;
	v = v->ccf();                     FVGET;
	v = v->cv();                      FVGET;
	v = v->ccf();                     FVGET;
	vCol = vCol->cf();
	v = vCol;                   VGET; FVGET;
	v = v->cf();                VGET; FVGET;
	v = v->ccv();                     FVGET;
	v = v->cf();                VGET; FVGET;
	v = v->ccv();                     FVGET;
	v = v->cf();                VGET; FVGET;
#	undef VGET
#	undef FVGET
}

} // anon namespace

TqInt CqSurfaceSubdivisionPatch::Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
{
	assert( pTopology() );
	assert( pTopology()->pPoints() );
	assert( pFace() );

	if( pTopology()->CanUsePatch( pFace() ) )
	{
		TqInt vertIdx[4*4];
		TqInt faceVertIdx[6*6];

		// Extract vertex & facevertex indices.
		getNbhdIndices(pFace(), vertIdx, faceVertIdx);

		std::vector< boost::shared_ptr<CqSurfacePatchBicubic> > apSurfaces;
		// Create a surface patch
		boost::shared_ptr<CqSurfacePatchBicubic> pSurface( new CqSurfacePatchBicubic() );
		// Fill in default values for all primitive variables not explicitly specified.
		pSurface->SetSurfaceParameters( *pTopology()->pPoints( 0 ) );

		std::vector<CqParameter*>::iterator iUP;
		std::vector<CqParameter*>::iterator end = pTopology()->pPoints( 0 )->aUserParams().end();
		for ( iUP = pTopology()->pPoints( 0 )->aUserParams().begin(); iUP != end; iUP++ )
		{
			if ( ( *iUP ) ->Class() == class_varying )
			{
				// Copy any 'varying' class primitive variables.
				CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
				pNewUP->SetSize( pSurface->cVarying() );
				pNewUP->SetValue( ( *iUP ), 0, vertIdx[5] );
				pNewUP->SetValue( ( *iUP ), 1, vertIdx[6] );
				pNewUP->SetValue( ( *iUP ), 2, vertIdx[9] );
				pNewUP->SetValue( ( *iUP ), 3, vertIdx[10] );
				pSurface->AddPrimitiveVariable( pNewUP );
			}
			else if ( ( *iUP ) ->Class() == class_vertex )
			{
				// Copy any 'vertex' class primitive variables.
				CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
				pNewUP->SetSize( pSurface->cVertex() );
				TqUint i;
				for( i = 0; i < pSurface->cVertex(); i++ )
					pNewUP->SetValue( ( *iUP ), i, vertIdx[i] );
				pSurface->AddPrimitiveVariable( pNewUP );
			}
			else if ( ( *iUP ) ->Class() == class_facevarying )
			{
				// Copy any 'facevarying' class primitive variables.
				CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
				// The output patch gets one facevarying value per corner.
				pNewUP->SetSize(4);
				pNewUP->SetValue( *iUP, 0, faceVertIdx[14] );
				pNewUP->SetValue( *iUP, 1, faceVertIdx[15] );
				pNewUP->SetValue( *iUP, 2, faceVertIdx[20] );
				pNewUP->SetValue( *iUP, 3, faceVertIdx[21] );
				pSurface->AddPrimitiveVariable( pNewUP );
			}
			else if ( ( *iUP ) ->Class() == class_facevertex )
			{
				// Convert facevertex values into vertex variables.  We only
				// convert to patches when the facevertex data is continuous.
				CqParameter* pNewUP = CqParameter::Create(
						CqPrimvarToken(class_vertex, (*iUP)->Type(),
							           (*iUP)->Count(), (*iUP)->strName()) );
				pNewUP->SetSize(16);
				pNewUP->SetValue(*iUP, 0, faceVertIdx[0]);
				pNewUP->SetValue(*iUP, 1, faceVertIdx[2]);
				pNewUP->SetValue(*iUP, 2, faceVertIdx[3]);
				pNewUP->SetValue(*iUP, 3, faceVertIdx[5]);
				pNewUP->SetValue(*iUP, 4, faceVertIdx[12]);
				pNewUP->SetValue(*iUP, 5, faceVertIdx[14]);
				pNewUP->SetValue(*iUP, 6, faceVertIdx[15]);
				pNewUP->SetValue(*iUP, 7, faceVertIdx[17]);
				pNewUP->SetValue(*iUP, 8, faceVertIdx[18]);
				pNewUP->SetValue(*iUP, 9, faceVertIdx[20]);
				pNewUP->SetValue(*iUP, 10, faceVertIdx[21]);
				pNewUP->SetValue(*iUP, 11, faceVertIdx[23]);
				pNewUP->SetValue(*iUP, 12, faceVertIdx[30]);
				pNewUP->SetValue(*iUP, 13, faceVertIdx[32]);
				pNewUP->SetValue(*iUP, 14, faceVertIdx[33]);
				pNewUP->SetValue(*iUP, 15, faceVertIdx[35]);
				pSurface->AddPrimitiveVariable( pNewUP );
			}
			else if ( ( *iUP ) ->Class() == class_uniform )
			{
				// Copy any 'uniform' class primitive variables.
				CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
				pNewUP->SetSize( pSurface->cUniform() );
				pNewUP->SetValue( ( *iUP ), 0, m_FaceIndex );
				pSurface->AddPrimitiveVariable( pNewUP );
			}
			else if ( ( *iUP ) ->Class() == class_constant )
			{
				// Copy any 'constant' class primitive variables.
				CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
				pNewUP->SetSize( 1 );
				pNewUP->SetValue( ( *iUP ), 0, 0 );
				pSurface->AddPrimitiveVariable( pNewUP );
			}
		}

		// Need to get rid of any 'h' values added to the "P" variables during multiplication.
		TqUint i;
		for( i = 0; i < pSurface->cVertex(); i++ )
			pSurface->P()->pValue(i)[0].Homogenize();

		CqMatrix matuBasis( RiBSplineBasis );
		CqMatrix matvBasis( RiBSplineBasis );
		pSurface->ConvertToBezierBasis( matuBasis, matvBasis );

		TqInt iUses = Uses();

		// If the shader needs s/t or u/v, and s/t is not specified, then at this point store the object space x,y coordinates.
		if ( USES( iUses, EnvVars_s ) || USES( iUses, EnvVars_t ) || USES( iUses, EnvVars_u ) || USES( iUses, EnvVars_v ) )
		{
			if ( USES( iUses, EnvVars_s ) && !pTopology()->pPoints()->bHasVar(EnvVars_s) )
			{
				CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pNewUP = new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "s" );
				pNewUP->SetSize( pSurface->cVarying() );

				pNewUP->pValue() [ 0 ] = 0.0f;
				pNewUP->pValue() [ 1 ] = 1.0f;
				pNewUP->pValue() [ 2 ] = 0.0f;
				pNewUP->pValue() [ 3 ] = 1.0f;

				pSurface->AddPrimitiveVariable( pNewUP );
			}

			if ( USES( iUses, EnvVars_t ) && !pTopology()->pPoints()->bHasVar(EnvVars_t) )
			{
				CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pNewUP = new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "t" );
				pNewUP->SetSize( pSurface->cVarying() );

				pNewUP->pValue() [ 0 ] = 0.0f;
				pNewUP->pValue() [ 1 ] = 0.0f;
				pNewUP->pValue() [ 2 ] = 1.0f;
				pNewUP->pValue() [ 3 ] = 1.0f;

				pSurface->AddPrimitiveVariable( pNewUP );
			}

			if ( USES( iUses, EnvVars_u ) && !pTopology()->pPoints()->bHasVar(EnvVars_u) )
			{
				CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pNewUP = new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "u" );
				pNewUP->SetSize( pSurface->cVarying() );

				pNewUP->pValue() [ 0 ] = 0.0f;
				pNewUP->pValue() [ 1 ] = 1.0f;
				pNewUP->pValue() [ 2 ] = 0.0f;
				pNewUP->pValue() [ 3 ] = 1.0f;

				pSurface->AddPrimitiveVariable( pNewUP );
			}

			if ( USES( iUses, EnvVars_v ) && !pTopology()->pPoints()->bHasVar(EnvVars_v) )
			{
				CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pNewUP = new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "v" );
				pNewUP->SetSize( pSurface->cVarying() );

				pNewUP->pValue() [ 0 ] = 0.0f;
				pNewUP->pValue() [ 1 ] = 0.0f;
				pNewUP->pValue() [ 2 ] = 1.0f;
				pNewUP->pValue() [ 3 ] = 1.0f;

				pSurface->AddPrimitiveVariable( pNewUP );
			}
		}
		aSplits.push_back(pSurface);
	}
	else
	{
		// Subdivide the face, and create new patches for the subfaces.
		std::vector<CqLath*> apSubFaces;
		pTopology()->SubdivideFace( pFace(), apSubFaces );

		// Now create new patch objects for each subface.
		std::vector<CqLath*>::iterator iSF;
		for( iSF = apSubFaces.begin(); iSF != apSubFaces.end(); iSF++ )
		{
			// Create a new patch object, note the use of the same face index as the current patch, as "uniform" values won't change,
			// so can be shared up the subdivision stack.
			boost::shared_ptr<CqSurfaceSubdivisionPatch> pNew( new CqSurfaceSubdivisionPatch( pTopology(), (*iSF), m_FaceIndex ) );
			aSplits.push_back(pNew);
		}
	}

	return(aSplits.size());
}

bool CqSurfaceSubdivisionPatch::Diceable(const CqMatrix& matCtoR)
{
	assert( pTopology() );
	assert( pTopology()->pPoints() );
	assert( pFace() );

	// If the cull check showed that the primitive cannot be diced due to crossing the e and hither planes,
	// then we can return immediately.
	if ( !m_fDiceable )
		return ( false );

	// If we can use a patch, don't dice, as dicing a patch is much quicker.
	if( pTopology()->CanUsePatch( pFace() ) )
		return(false);

	// Get the laths that reference the vertices of this face
	std::vector<CqLath*> aQfv;
	pFace()->Qfv(aQfv);

	// Cannot dice if not 4 points
	if ( aQfv.size() != 4 )
		return ( false );

	// Otherwise we should continue to try to find the most advantageous split
	// direction, OR the dice size.
	//
	//
	// Computing the dice size for a subdivision surface is slightly tricky.
	//
	// We can't just use the current corners of the patch, since a small patch
	// may become arbitrarily large under subdivision if it has large
	// neighbouring patches.  For example, consider the following mesh,
	//
	// +---+----------------------------------------------+
	// |   |                                              |
	// |   |                                              |
	// |   |                                              |
	// |   |                                              |
	// |   |                                              |
	// +---+----------------------------------------------+
	// 
	// and perform one subdivision step -
	// 
	// 1) Add new vertices and move old ones
	// 
	// o = new face or edge vertex
	// X = moved vertex
	// 
	// +-o-+---X------------------o-----------------------+
	// |   |                                              |
	// |   |                                              |
	// o o |   o <-- note how     o                       o
	// |   |         much this                            |
	// |   |         moves                                |
	// +-o-+---X------------------o-----------------------+
	//      
	// 
	// 2) Add edges.
	// 
	// +-+-----+------------------+-----------------------+
	// | |     |                  |                       |
	// | |     |                  |                       |
	// +-+-----+------------------+-----------------------+
	// | |     |                  |                       |
	// | |     |                  |                       |
	// +-+-----+------------------+-----------------------+
	//  <----->
	//    |
	// this whole section came from the small original face on the left, and
	// has area more than twice the original size of the patch.
	//
	// Instead, we've got to compute the positions of the vertices on the limit
	// surface:

	CqVector3D	hull[4];
	for (TqInt i = 0; i < 4; i++ )
		hull[i] = matCtoR*pTopology()->limitPoint(aQfv[i]);

	TqFloat uLen = max(
			(hull[1] - hull[0]).Magnitude2(),
			(hull[2] - hull[3]).Magnitude2());
	TqFloat vLen = max(
			(hull[3] - hull[0]).Magnitude2(),
			(hull[2] - hull[1]).Magnitude2());

	TqFloat shadingRate = AdjustedShadingRate();
	uLen = sqrt(uLen/shadingRate);
	vLen = sqrt(vLen/shadingRate);

	m_SplitDir = ( uLen > vLen ) ? SplitDir_U : SplitDir_V;

	m_uDiceSize = max<TqInt>(lround(uLen), 1);
	m_vDiceSize = max<TqInt>(lround(vLen), 1);

	// Note Subd surfaces always have a power of 2 dice rate because they
	// are diced by recursive subdivision. Hence no need to set it explicitly.

	if ( uLen < FLT_EPSILON || vLen < FLT_EPSILON )
	{
		m_fDiscard = true;
		return ( false );
	}

	// because splitting to a bicubic patch is so much faster than dicing by
	// recursive subdivision, the grid size is made smaller than usual to give
	// us more chance to break regular parts off as a patch.
	TqFloat gs = 8.0f;
	const TqFloat* poptGridSize = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "SqrtGridSize" );
	if( poptGridSize )
		gs = poptGridSize[0] / 2.0f;

	if ( m_uDiceSize > gs)
		return false;
	if ( m_vDiceSize > gs)
		return false;

	return ( true );
}


namespace {

/** \brief Check that all provided facevertex pairs are continuous.
 *
 * This function checks that the parameter "parUntyped" is continuous (equal)
 * between each provided pair facevertex indices.
 *
 * \param parUntyped - parameter to check.
 * \param fvIndex - an array of pairs of facevertex indices (length
 *                  2*numPairs).  parUntyped is checked for equality at the
 *                  indices fvIndex[2*i] fvIndex[2*i+1] for all i between 0 and
 *                  numPairs-1
 * \param numPairs - number of facevertex index pairs residing in fvIndex.
 */
template<typename T, typename SLT>
bool allFvertContinuous(const CqParameter& parUntyped, TqInt* fvIndex,
		TqInt numPairs)
{
	const CqParameterTyped<T, SLT>& par
		= static_cast<const CqParameterTyped<T, SLT>&>(parUntyped);
	TqInt arLen = par.ArrayLength();
	for(TqInt i = 0; i < numPairs; ++i)
	{
		const T* val1 = par.pValue(fvIndex[2*i]);
		const T* val2 = par.pValue(fvIndex[2*i+1]);
		for(TqInt arIdx = 0; arIdx < arLen; ++arIdx)
		{
			if( !isClose(val1[arIdx], val2[arIdx]) )
				return false;
		}
	}
	return true;
}

} // anon namespace

/**
 * Determine if the topology surrounding the specified face is suitable for
 * conversion to a bicubic patch.
 */
bool CqSubdivision2::CanUsePatch( CqLath* pFace )
{
	// If the patch is a quad with each corner having valence 4, and no special features,
	// we can just create a B-Spline patch.
	if( pFace->cQfv() != 4 )
		return( false );

	std::vector<CqLath*> aQff, aQfv;
	pFace->Qfv(aQfv);
	std::vector<CqLath*>::iterator iFV;
	for( iFV = aQfv.begin(); iFV!=aQfv.end(); iFV++ )
	{
		// Check if all vertices are valence 4.
		if( (*iFV)->cQvv() != 4 )
			return( false );

		// Check if all edges incident on the face vertices are smooth.
		std::vector<CqLath*> aQve;
		(*iFV)->Qve(aQve);
		std::vector<CqLath*>::iterator iVE;
		for( iVE = aQve.begin(); iVE!=aQve.end(); iVE++ )
		{
			if( EdgeSharpness((*iVE)) != 0.0f ||
			        CornerSharpness((*iVE)) != 0.0f )
				return( false );
		}

		// Check if no internal boundaries.
		CqLath* pEnd = (*iFV)->cv();
		while( (*iFV) != pEnd )
		{
			if( NULL == pEnd )
				return( false );
			pEnd = pEnd->cv();
		}
	}

	// Check local neighbourhood of patch is 9 quads.
	pFace->Qff(aQff);
	if( aQff.size() != 9 )
		return( false );

	std::vector<CqLath*>::iterator iFF;
	for( iFF = aQff.begin(); iFF!=aQff.end(); iFF++ )
	{
		if( (*iFF)->cQfv() != 4 )
			return( false );
	}

	// Finally check if the "facevertex" values match at the patch vertices, as
	// the interpolation scheme for most discontinuous facevertex data can't be
	// represented using the vertex interpolation scheme of a bspline patch.
	if(m_faceVertexParams.size() != 0)
	{
		// Get facevertex indices for patches in the current patch
		// neighbourhood.
		TqInt vertIdx[16];
		TqInt fvertIdx[36];
		getNbhdIndices(pFace, vertIdx, fvertIdx);

		// Pairs of indices into fvertIdx which we should check to make sure
		// that all facevertex parameters are continuous across over the
		// current patch neighbourhood.  See docs for getNbhdIndices() for the
		// geometric meaning of the indices.
		TqInt pairsToCheckIdx[] = {
			// Pairs around edges of patch
			1,2,  3,4,  6,12,  11,17,  18,24,  23,29,  31,32,  33,34,
			// Vertices in the middle.
			7,14,   8,14,  13,14,
			9,15,   10,15,  16,15,
			19,20,  25,20,  26,20,
			22,21,  27,21,  28,21
		};
		const TqInt maxNumPairs = sizeof(pairsToCheckIdx)/(2*sizeof(TqInt));

		// Look up the face vertex indices for pairs, and remove any pairs which
		// are definitely equal due to both face vertex indices in the pair
		// being equal:
		TqInt pairsToCheck[2*maxNumPairs];
		TqInt numPairs = 0;
		for(TqInt i = 0; i < maxNumPairs; ++i)
		{
			TqInt fvIdx1 = fvertIdx[pairsToCheckIdx[2*i]];
			TqInt fvIdx2 = fvertIdx[pairsToCheckIdx[2*i+1]];
			if(fvIdx1 != fvIdx2)
			{
				pairsToCheck[2*numPairs] = fvIdx1;
				pairsToCheck[2*numPairs+1] = fvIdx2;
				++numPairs;
			}
		}

		// Now check that the facevarying data is continuous between each pair
		// of indices extracted above.
		for(std::vector<CqParameter*>::iterator par = m_faceVertexParams.begin(),
			parEnd = m_faceVertexParams.end(); par != parEnd; ++par)
		{
			bool cont = false;
			switch((*par)->Type())
			{
				case type_float:
					cont = allFvertContinuous<TqFloat, TqFloat>(**par, pairsToCheck, numPairs);
					break;
				case type_point:
				case type_normal:
				case type_vector:
					cont = allFvertContinuous<CqVector3D, CqVector3D>(**par, pairsToCheck, numPairs);
					break;
				case type_color:
					cont = allFvertContinuous<CqColor, CqColor>(**par, pairsToCheck, numPairs);
					break;
				case type_hpoint:
					cont = allFvertContinuous<CqVector4D, CqVector3D>(**par, pairsToCheck, numPairs);
					break;
				case type_matrix:
					cont = allFvertContinuous<CqMatrix, CqMatrix>(**par, pairsToCheck, numPairs);
					break;
				default:
					// Ignore remaining parameter types, since they don't make sense to
					// subdivide.
					break;
			}
			if(!cont)
				return false;
		}
	}

	return( true );
}


void	CqSurfaceSubdivisionMesh::Bound(CqBound* bound) const
{
	if( m_pTopology && m_pTopology->pPoints() && m_pTopology->pPoints()->P() )
	{
		TqInt PointIndex;
		for( PointIndex = m_pTopology->pPoints()->P()->Size()-1; PointIndex >= 0; PointIndex-- )
			bound->Encapsulate( vectorCast<CqVector3D>(m_pTopology->pPoints()->P()->pValue()[PointIndex]) );
	}
	AdjustBoundForTransformationMotion( bound );
}

TqInt CqSurfaceSubdivisionMesh::Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
{
	TqInt	CreatedPolys = 0;
	TqInt	face;

	for ( face = 0; face < m_NumFaces; face++ )
	{
		// Don't add faces which are on the boundary, unless "interpolateboundary" is specified.
		if( ( !m_pTopology->pFacet( face )->isBoundaryFacet() ) || ( m_pTopology->isInterpolateBoundary() ) )
		{
			// Don't add "hole" faces
			if( !m_pTopology->isHoleFace( face ) )
			{
				// Add a patch surface to the bucket queue
				boost::shared_ptr<CqSurfaceSubdivisionPatch> pNew( new CqSurfaceSubdivisionPatch( m_pTopology, m_pTopology->pFacet( face ), face ) );
				aSplits.push_back( pNew );
				CreatedPolys++;
			}
		}
	}
	return( CreatedPolys );
}


boost::shared_ptr<CqSubdivision2> CqSurfaceSubdivisionPatch::Extract( TqInt iTime )
{
	assert( pTopology() );
	assert( pTopology()->pPoints() );
	assert( pFace() );

	// Find the point indices for the polygons surrounding this one.
	// Use a map to ensure that shared vertices are only counted once.
	std::map<TqInt, TqInt> Vertices;
	TqUint cVerts=0;
	std::vector<TqInt>	FVertices;

	std::vector<CqLath*> aQff;
	std::vector<CqLath*> apSubFaces2;
	pFace()->Qff( aQff );
	std::vector<CqLath*>::iterator iF;
	for( iF = aQff.begin(); iF != aQff.end(); iF++ )
	{
		std::vector<CqLath*> aQfv;
		(*iF)->Qfv( aQfv );
		std::vector<CqLath*>::reverse_iterator iV;
		for( iV = aQfv.rbegin(); iV != aQfv.rend(); iV++ )
		{
			if( Vertices.find((*iV)->VertexIndex()) == Vertices.end() )
			{
				Vertices[(*iV)->VertexIndex()] = cVerts;
				cVerts++;
			}
			FVertices.push_back( (*iV)->FaceVertexIndex() );
		}
	}

	// Create a storage class for all the points.
	boost::shared_ptr<CqPolygonPoints> pPointsClass( new CqPolygonPoints( cVerts, aQff.size(), FVertices.size() ) );
	// Fill in default values for all primitive variables not explicitly specified.
	pPointsClass->SetSurfaceParameters( *pTopology()->pPoints( iTime ) );

	boost::shared_ptr<CqSubdivision2> pSurface( new CqSubdivision2( pPointsClass ) );
	pSurface->Prepare( cVerts );

	std::vector<CqParameter*>::iterator iUP;
	std::vector<CqParameter*>::iterator end = pTopology()->pPoints( iTime)->aUserParams().end();
	for ( iUP = pTopology()->pPoints( iTime )->aUserParams().begin(); iUP != end; iUP++ )
	{
		if ( ( *iUP ) ->Class() == class_vertex || ( *iUP ) ->Class() == class_varying )
		{
			// Copy any 'vertex' or 'varying' class primitive variables.
			CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
			pNewUP->SetSize( cVerts );
			std::map<TqInt, TqInt>::iterator i;
			for( i = Vertices.begin(); i != Vertices.end(); i++ )
				pNewUP->SetValue( ( *iUP ), (*i).second, (*i).first );
			pSurface->pPoints()->AddPrimitiveVariable( pNewUP );
		}
		else if ( ( *iUP ) ->Class() == class_facevarying || ( *iUP )->Class() == class_facevertex )
		{
			// Copy any 'facevarying' class primitive variables.
			CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
			pNewUP->SetSize( FVertices.size() );
			std::vector<TqInt>::iterator i;
			TqInt iv = 0;
			for( i = FVertices.begin(); i != FVertices.end(); i++, iv++ )
				pNewUP->SetValue( ( *iUP ), iv, (*i) );
			pSurface->pPoints()->AddPrimitiveVariable( pNewUP );
		}
		else if ( ( *iUP ) ->Class() == class_uniform )
		{
			// Copy any 'uniform' class primitive variables.
			CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
			pNewUP->SetSize( pSurface->pPoints()->cUniform() );
			pNewUP->SetValue( ( *iUP ), 0, m_FaceIndex );
			pSurface->pPoints()->AddPrimitiveVariable( pNewUP );
		}
		else if ( ( *iUP ) ->Class() == class_constant )
		{
			// Copy any 'constant' class primitive variables.
			CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
			pNewUP->SetSize( 1 );
			pNewUP->SetValue( ( *iUP ), 0, 0 );
			pSurface->pPoints()->AddPrimitiveVariable( pNewUP );
		}
	}

	// Need to get rid of any 'h' values added to the "P" variables during multiplication.
	TqUint i;
	for( i = 0; i < cVerts; i++ )
		pSurface->pPoints()->P()->pValue(i)[0].Homogenize();

	TqInt iP = 0;
	for( iF = aQff.begin(); iF != aQff.end(); iF++ )
	{
		std::vector<TqInt> vertices;
		std::vector<CqLath*> aQfv;
		(*iF)->Qfv( aQfv );
		std::vector<CqLath*>::reverse_iterator iV;
		for( iV = aQfv.rbegin(); iV != aQfv.rend(); ++iV )
			vertices.push_back( Vertices[(*iV)->VertexIndex()] );
		pSurface->AddFacet( vertices.size(), &vertices[ 0 ], iP );
		iP += vertices.size();
	}
	pSurface->Finalise();
	return(pSurface);
}


CqSurface* CqSurfaceSubdivisionMesh::Clone() const
{
	boost::shared_ptr<CqSubdivision2> clone_subd(m_pTopology->Clone());
	CqSurfaceSubdivisionMesh* clone = new CqSurfaceSubdivisionMesh(clone_subd, m_NumFaces);
	CqSurface::CloneData(clone);
	// Now copy the tags information across.
	clone->m_aSharpEdges = m_aSharpEdges;

	std::vector<std::pair<std::pair<TqInt, TqInt>, TqFloat> >::const_iterator edge;
	for(edge = m_aSharpEdges.begin(); edge != m_aSharpEdges.end(); edge++) 
	{
		TqInt a = edge->first.first;
		TqInt b = edge->first.second;
		TqFloat s = edge->second;
		if ( a < clone->m_pTopology->cVertices() && b < clone->m_pTopology->cVertices() )
		{
			// Store the crease sharpness.
			CqLath* pEdge = clone->m_pTopology->pVertex( a );
			std::vector<CqLath*> aQve;
			pEdge->Qve( aQve );
			std::vector<CqLath*>::iterator iOpp;
			for( iOpp = aQve.begin(); iOpp != aQve.end(); ++iOpp )
			{
				if( ( NULL != (*iOpp)->ec() ) && (*iOpp)->ec()->VertexIndex() == b )
				{
					clone->m_pTopology->AddSharpEdge( (*iOpp), s );
					clone->m_pTopology->AddSharpEdge( (*iOpp)->ec(), s );
					break;
				}
			}
		}
	}

	clone->m_aSharpCorners = m_aSharpCorners;
	std::vector<std::pair<TqInt, TqFloat> >::const_iterator corner;
	for(corner = m_aSharpCorners.begin(); corner != m_aSharpCorners.end(); corner++) 
	{
		TqInt a = corner->first;
		TqFloat s = corner->second;
		if ( a < clone->m_pTopology->cVertices() )
		{
			// Store the corner sharpness.
			CqLath* pVertex = clone->m_pTopology->pVertex( a );
			clone->m_pTopology->AddSharpCorner( pVertex, s );
		}
	}


	return(clone);
}


} // namespace Aqsis
