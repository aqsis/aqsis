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
		\brief Declares the classes and support structures for handling polygons.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef POLYGON_H_INCLUDED
#define POLYGON_H_INCLUDED 1

#include	"surface.h"
#include	"vector4d.h"

#include	"specific.h"	// Needed for namespace macros.

START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------
/** \class CqPolygonBase
 * Polygon base class for split and dice functionality.
 */

class CqPolygonBase
{
	public:
				CqPolygonBase()		{}
		virtual	~CqPolygonBase()	{}

		virtual	CqBound		Bound() const;
		virtual	CqMicroPolyGridBase* Dice();
		virtual	TqInt		Split(std::vector<CqBasicSurface*>& aSplits);
		virtual TqBool		Diceable();

							/** Get a reference to the surface this polygon is associated with.
							 */	
		virtual	const CqSurface& Surface()const=0;
							/** Get a reference to the surface this polygon is associated with.
							 */	
		virtual	CqSurface&	Surface()=0;

							/** Get a reference to the polygon point at the specified index.
							 * \param i Integer index in of the vertex in question.
							 */
		virtual	const	CqVector4D& PolyP(TqInt i) const=0;
							/** Get a reference to the polygon normal at the specified index.
							 * \param i Integer index in of the vertex in question.
							 */
		virtual	const	CqVector3D& PolyN(TqInt i) const=0;
							/** Get a reference to the polygon vertex color at the specified index.
							 * \param i Integer index in of the vertex in question.
							 */
		virtual	const	CqColor& PolyCs(TqInt i) const=0;
							/** Get a reference to the polygon vertex opacity at the specified index.
							 * \param i Integer index in of the vertex in question.
							 */
		virtual	const	CqColor& PolyOs(TqInt i) const=0;
							/** Get a reference to the polygon texture s coordinate at the specified index.
							 * \param i Integer index in of the vertex in question.
							 */
		virtual	const	TqFloat& Polys(TqInt i) const=0;
							/** Get a reference to the polygon texture t coordinate at the specified index.
							 * \param i Integer index in of the vertex in question.
							 */
		virtual	const	TqFloat& Polyt(TqInt i) const=0;
							/** Get a reference to the polygon surface u coordinate at the specified index.
							 * \param i Integer index in of the vertex in question.
							 */
		virtual	const	TqFloat& Polyu(TqInt i) const=0;
							/** Get a reference to the polygon surface v coordinate at the specified index.
							 * \param i Integer index in of the vertex in question.
							 */
		virtual	const	TqFloat& Polyv(TqInt i) const=0;

							/** Determine whether the GPrim has user specified normals.
							 */
		virtual	TqBool		bHasNormals() const=0;
							/** Determine whether the GPrim has user specified texture s coordinates.
							 */
		virtual	TqBool		bHass() const=0;
							/** Determine whether the GPrim has user specified texture s coordinates.
							 */
		virtual	TqBool		bHast() const=0;
							/** Determine whether the GPrim has user specified surface u coordinates.
							 */
		virtual	TqBool		bHasu() const=0;
							/** Determine whether the GPrim has user specified surface u coordinates.
							 */
		virtual	TqBool		bHasv() const=0;
							/** Determine whether the GPrim has user specified vertex colors.
							 */
		virtual	TqBool		bHasCs() const=0;
							/** Determine whether the GPrim has user specified vertex opacities.
							 */
		virtual	TqBool		bHasOs() const=0;

							/** Get the number of vertices in this polygon.
							 */
		virtual	TqInt		NumVertices() const=0;

							/** Get a pointer to the attributes state associated with this polygon.
							 */
		virtual	const CqAttributes*	pAttributes() const=0;
							/** Get a pointer to the transfrom associated with this polygon.
							 */
		virtual	const CqTransform*	pTransform() const=0;

							/** Get a bit vector representing the standard shader variables this polygon needs.
							 */
				const TqInt	PolyUses()	const	{return(Surface().Uses());}
};


//----------------------------------------------------------------------
/** \class CqSurfacePolygon
 * Polygon surface primitive.
 */

class CqSurfacePolygon : public CqSurface, public CqPolygonBase
{
	public:
				CqSurfacePolygon(TqInt cVertices);
				CqSurfacePolygon(const CqSurfacePolygon& From);
		virtual	~CqSurfacePolygon();

				CqSurfacePolygon& operator=(const CqSurfacePolygon& From);
		TqBool	CheckDegenerate() const;

		// Overridden fro mCqSurface.
		virtual	CqBound		Bound() const			{return(CqPolygonBase::Bound());}
		virtual	CqMicroPolyGridBase* Dice()				{return(CqPolygonBase::Dice());}
		virtual	TqInt		Split(std::vector<CqBasicSurface*>& aSplits)	
													{return(CqPolygonBase::Split(aSplits));}
		virtual TqBool		Diceable()				{return(CqPolygonBase::Diceable());}

		virtual void		Transform(const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx);
		virtual	TqInt		cUniform() const	{return(1);}
		virtual	TqInt		cVarying() const	{return(m_cVertices);}
		virtual	TqInt		cVertex() const		{return(m_cVertices);}

		// Overridden from CqPolygonBase
		virtual	const CqSurface& Surface() const		{return(*this);}
		virtual	CqSurface&	Surface()			{return(*this);}

		virtual	const	CqVector4D& PolyP(TqInt i) const	{return(P()[i]);}
		virtual	const	CqVector3D& PolyN(TqInt i) const	{return(N()[i]);}
		virtual	const	CqColor& PolyCs(TqInt i) const		{return(Cs()[i]);}
		virtual	const	CqColor& PolyOs(TqInt i) const		{return(Os()[i]);}
		virtual	const	TqFloat& Polys(TqInt i) const		{return(s()[i]);}
		virtual	const	TqFloat& Polyt(TqInt i) const		{return(t()[i]);}
		virtual	const	TqFloat& Polyu(TqInt i) const		{return(u()[i]);}
		virtual	const	TqFloat& Polyv(TqInt i) const		{return(v()[i]);}

		virtual	TqBool		bHasNormals() const			{return(N().Size()>=P().Size());}
		virtual	TqBool		bHass() const				{return(s().Size()>=P().Size());}
		virtual	TqBool		bHast() const				{return(t().Size()>=P().Size());}
		virtual	TqBool		bHasu() const				{return(u().Size()>=P().Size());}
		virtual	TqBool		bHasv() const				{return(v().Size()>=P().Size());}
		virtual	TqBool		bHasCs() const				{return(Cs().Size()>=P().Size());}
		virtual	TqBool		bHasOs() const				{return(Os().Size()>=P().Size());}

		virtual	TqInt		NumVertices() const				{return(P().Size());}

		virtual	const CqAttributes*	pAttributes() const	{return(CqSurface::pAttributes());}
		virtual	const CqTransform*	pTransform() const	{return(CqSurface::pTransform());}

				void	TransferDefaultSurfaceParameters();

	protected:
		TqInt	m_cVertices;	///< Count of vertices in this polygon.
};


//----------------------------------------------------------------------
/** \class CqPolygonPoints
 * Points array for PointsPolygons surface primitive.
 */

class CqPolygonPoints : public CqSurface
{
	public:
				CqPolygonPoints(TqInt cVertices)	:	m_cReferences(0),	
														m_cVertices(cVertices),
														m_Transformed(TqFalse)
															{}
		virtual	~CqPolygonPoints()							{}

		// Overridden from CqSurface.
		// NOTE: These should never be called.
		virtual	CqBound		Bound() const		{static CqBound bTemp; return(bTemp);}
		virtual	CqMicroPolyGridBase* Dice()			{return(0);}
		virtual	TqInt		Split(std::vector<CqBasicSurface*>& aSplits)
												{return(0);}
		virtual TqBool		Diceable()			{return(TqFalse);}

		virtual	void		Transform(const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx);
		virtual	TqInt		cUniform() const	{return(m_cReferences);}
		virtual	TqInt		cVarying() const	{return(m_cVertices);}
		virtual	TqInt		cVertex() const		{return(m_cVertices);}

							/** Determine whether the list contains user specified normals.
							 */
		virtual	TqBool		bHasNormals() const	{return(N().Size()>=P().Size());}
							/** Determine whether the list contains texture s coordinates.
							 */
		virtual	TqBool		bHass() const		{return(s().Size()>=P().Size());}
							/** Determine whether the list contains texture s coordinates.
							 */
		virtual	TqBool		bHast() const		{return(t().Size()>=P().Size());}
							/** Determine whether the list contains surface u coordinates.
							 */
		virtual	TqBool		bHasu() const		{return(u().Size()>=P().Size());}
							/** Determine whether the list contains surface v coordinates.
							 */
		virtual	TqBool		bHasv() const		{return(v().Size()>=P().Size());}
							/** Determine whether the list contains vertex colors.
							 */
		virtual	TqBool		bHasCs() const		{return(Cs().Size()>=P().Size());}
							/** Determine whether the list contains vertex opacities.
							 */
		virtual	TqBool		bHasOs() const		{return(Os().Size()>=P().Size());}

							
							/** Increment the number of polygons referercing this list.
							 */		
				void		Reference()			{m_cReferences++;}
							/** Decrement the number of polygons referercing this list, delete it if not needed.
							 */		
				void		UnReference()		{
													m_cReferences--;
													if(m_cReferences<=0)
														delete(this);
												}
				void		TransferDefaultSurfaceParameters();
							/** Get the number of vertices in the list.
							 */
		virtual	TqInt		NumVertices() const	{return(P().Size());}

	protected:
		TqInt			m_cVertices;		///< Count of vertices in this list.
		TqInt			m_cReferences;		///< Count of polygons referencing this list.
		TqBool			m_Transformed;		///< Flag indicatign that the list has been transformed.
};


//----------------------------------------------------------------------
/** \class CqSurfacePointsPolygon
 * Points polygon surface primitive, a single member of the above.
 */

class CqSurfacePointsPolygon : public CqBasicSurface, public CqPolygonBase
{
	public:
				CqSurfacePointsPolygon(CqPolygonPoints* pPoints)	:	CqBasicSurface(),
																		m_pPoints(pPoints)
																	{m_pPoints->Reference();}
				CqSurfacePointsPolygon(const CqSurfacePointsPolygon& From);
		virtual	~CqSurfacePointsPolygon()							{m_pPoints->UnReference();}

				std::vector<TqInt>&	aIndices()		{return(m_aIndices);}
				CqSurfacePointsPolygon& operator=(const CqSurfacePointsPolygon& From);
				TqInt		cVertices() const	{return(m_aIndices.size());}

		// Overridden from CqBasicSurface
		virtual	CqBound		Bound() const	{return(CqPolygonBase::Bound());}
		virtual	CqMicroPolyGridBase* Dice()		{return(CqPolygonBase::Dice());}
		virtual	TqInt		Split(std::vector<CqBasicSurface*>& aSplits)
											{return(CqPolygonBase::Split(aSplits));}
		virtual TqBool		Diceable()		{return(CqPolygonBase::Diceable());}

		virtual void		Transform(const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx)
											{m_pPoints->Transform(matTx,matITTx,matRTx);}
		// NOTE: These should never be called.
		virtual	TqInt		cUniform() const	{return(0);}
		virtual	TqInt		cVarying() const	{return(0);}
		virtual	TqInt		cVertex() const		{return(0);}
		//---------------

		// Overridden from CqPolygonBase
		virtual	const CqSurface& Surface() const			{return(*m_pPoints);}
		virtual		  CqSurface& Surface()				{return(*m_pPoints);}

		virtual	const CqVector4D& PolyP(TqInt i) const	{return(m_pPoints->P()[m_aIndices[i]]);}
		virtual	const CqVector3D& PolyN(TqInt i) const	{return(m_pPoints->N()[m_aIndices[i]]);}
		virtual	const CqColor& PolyCs(TqInt i) const		{return(m_pPoints->Cs()[m_aIndices[i]]);}
		virtual	const CqColor& PolyOs(TqInt i) const		{return(m_pPoints->Os()[m_aIndices[i]]);}
		virtual	const TqFloat& Polys(TqInt i) const		{return(m_pPoints->s()[m_aIndices[i]]);}
		virtual	const TqFloat& Polyt(TqInt i) const		{return(m_pPoints->t()[m_aIndices[i]]);}
		virtual	const TqFloat& Polyu(TqInt i) const		{return(m_pPoints->u()[m_aIndices[i]]);}
		virtual	const TqFloat& Polyv(TqInt i) const		{return(m_pPoints->v()[m_aIndices[i]]);}

		virtual	TqBool		bHasNormals() const			{return(m_pPoints->bHasNormals());}
		virtual	TqBool		bHass() const				{return(m_pPoints->bHass());}
		virtual	TqBool		bHast() const				{return(m_pPoints->bHast());}
		virtual	TqBool		bHasu() const				{return(m_pPoints->bHasu());}
		virtual	TqBool		bHasv() const				{return(m_pPoints->bHasv());}
		virtual	TqBool		bHasCs() const				{return(m_pPoints->bHasCs());}
		virtual	TqBool		bHasOs() const				{return(m_pPoints->bHasOs());}

		virtual	TqInt		NumVertices() const				{return(m_aIndices.size());}

		virtual	const CqAttributes*	pAttributes() const	{return(m_pPoints->pAttributes());}
		virtual	const CqTransform*	pTransform() const	{return(m_pPoints->pTransform());}

	protected:
		std::vector<TqInt>	m_aIndices;		///< Array of indices into the associated vertex list.
		CqPolygonPoints*	m_pPoints;		///< Pointer to the associated CqPolygonPoints class.
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !POLYGON_H_INCLUDED
