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
		\brief Declares the classes for handling micropolygrids and micropolygons.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef MICROPOLYGON_H_INCLUDED
#define MICROPOLYGON_H_INCLUDED 1

#include	"ri.h"

#include	"specific.h"	// Needed for namespace macros.

#include	"memorypool.h"

#include	"color.h"
#include	"list.h"
#include	"bound.h"
#include	"vector2d.h"
#include	"vector3d.h"
#include	"vector4d.h"
#include	"shaderexecenv.h"
#include	"shadervariable.h"
#include	"motion.h"

START_NAMESPACE(Aqsis)

class CqVector3D;
class CqImageBuffer;
class CqSurface;

//----------------------------------------------------------------------
/** \class CqMicroPolyGridBase
 * Base class from which all MicroPolyGrids are derived.
 */

class CqMicroPolyGridBase
{
	public:
					CqMicroPolyGridBase()	{}
		virtual		~CqMicroPolyGridBase()	{}

					/** Pure virtual function, splits the grid into micropolys.
					 * \param pImage Pointer to the image buffer being rendered.
					 * \param iBucket Index of the bucket begin processed.
					 * \param xmin The minimum x pixel, taking into account clipping etc.
					 * \param xmax The maximum x pixel, taking into account clipping etc.
					 * \param ymin The minimum y pixel, taking into account clipping etc.
					 * \param xmax The maximum y pixel, taking into account clipping etc.
					 */
	virtual	void	Split(CqImageBuffer* pImage, TqInt iBucket, long xmin, long xmax, long ymin, long ymax)=0;
					/** Pure virtual, project the grid into raster space.
					 */
	virtual void	Project()=0;
					/** Pure virtual, get the bound of the grid.
					 * \return CqBound class representing the conservative boundary.
					 */
	virtual CqBound	Bound()=0;
					/** Pure virtual, get a pointer to the surface this grid belongs.
					 * \return Pointer to surface, only valid during grid shading.
					 */
	virtual CqSurface*	pSurface() const=0;
	private:
};


//----------------------------------------------------------------------
/** \class CqMicroPolyGrid
 * Class which stores a grid of micropolygons.
 */

class CqMicroPolyGrid : public CqMicroPolyGridBase, public CqShaderExecEnv
{
	public:
					CqMicroPolyGrid();
					CqMicroPolyGrid(TqInt cu, TqInt cv, CqSurface* pSurface);
		virtual		~CqMicroPolyGrid();

			void	CalcNormals();
					/** Set the normals flag, indicating this grid has normals already specified.
					 * \param f The new state of the flag.
					 */
			void	SetNormals(TqBool f)	{m_fNormals=f;}
			void	Shade();
			void	Initialise(TqInt cu, TqInt cv, CqSurface* pSurface);
			
					/** Increase the count of references to this grid.
					 */
			void	AddRef()			{m_cReferences++;}
					/** Decrease the count of references to this grid. Delete it if no more references.
					 */
			void	Release()			{
											assert(m_cReferences>0);
											m_cReferences--;
											if(m_cReferences==0)
												delete(this);

										}

	// Overrides from CqMicroPolyGridBase
	virtual	void	Split(CqImageBuffer* pImage, TqInt iBucket, long xmin, long xmax, long ymin, long ymax);
	virtual void	Project();
	virtual CqBound	Bound();
					/** Get a pointer to the surface which this grid belongs.
					 * \return Surface pointer, only valid during shading.
					 */
	virtual CqSurface*	pSurface() const	{return(CqShaderExecEnv::pSurface());}

	private:
			TqBool	m_fNormals;			///< Flag indicating normals have been filled in and don't need to be calculated during shading.
			TqInt	m_cReferences;		///< Count of references to this grid.
};


//----------------------------------------------------------------------
/** \class CqMotionMicroPolyGrid
 * Class which stores info about motion blurred micropolygrids.
 */

class CqMotionMicroPolyGrid : public CqMicroPolyGridBase, public CqMotionSpec<CqMicroPolyGridBase*>
{
	public:
					CqMotionMicroPolyGrid()	: CqMicroPolyGridBase(), CqMotionSpec<CqMicroPolyGridBase*>(0) {}
		virtual		~CqMotionMicroPolyGrid()	{}

	// Overrides from CqMicroPolyGridBase
	virtual	void	Split(CqImageBuffer* pImage, TqInt iBucket, long xmin, long xmax, long ymin, long ymax);
	virtual void	Project();
	virtual CqBound	Bound();
					/** Get a pointer to the surface which this grid belongs.
					 * Actually returns the surface pointer from the first timeslot.
					 * \return Surface pointer, only valid during shading.
					 */
	virtual CqSurface*	pSurface() const	{return(static_cast<CqMicroPolyGrid*>(GetMotionObject(Time(0)))->pSurface());}

	virtual		void		ClearMotionObject(CqMicroPolyGridBase*& A) const	{}
					/** Overridden from CqMotionSpec, does nothing.
					 */
	virtual		CqMicroPolyGridBase* ConcatMotionObjects(CqMicroPolyGridBase* const & A, CqMicroPolyGridBase* const & B) const	{return(B);}
					/** Overridden from CqMotionSpec, does nothing.
					 */
	virtual		CqMicroPolyGridBase* LinearInterpolateMotionObjects(TqFloat Fraction, CqMicroPolyGridBase* const & A, CqMicroPolyGridBase* const & B) const {return(A);}

	private:
};


//----------------------------------------------------------------------
/** \class CqMicroPolygonBase
 * Abstract base class from which static and motion micropolygons are derived.
 */

class CqMicroPolygonBase
{
	public:
					CqMicroPolygonBase();
					CqMicroPolygonBase(const CqMicroPolygonBase& From);
	virtual			~CqMicroPolygonBase();
	
	public:
					/** Set up the pointer to the grid this micropoly came from.
					 * \param pGrid CqMicroPolyGrid pointer.
					 */
			void	SetGrid(CqMicroPolyGrid* pGrid)
											{
												if(m_pGrid)	m_pGrid->Release();
												m_pGrid=pGrid;
												m_pGrid->AddRef();
											}
					/** Get the pointer to the grid this micropoly came from.
					 * \return Pointer to the CqMicroPolyGrid.
					 */
			CqMicroPolyGrid* pGrid() const	{return(m_pGrid);}
					/** Set the index within the donor grid.
					 * \param Index Integer grid index.
					 */
			void	SetIndex(TqInt Index)	{
												assert(m_pGrid!=0 && m_pGrid->GridSize()>Index);
												m_Index=Index;
											}
					/** Release this micropolys reference to the donor grid.
					 */
			void	Detatch()				{
												if(m_pGrid!=0)
												{
													m_pGrid->Release();
													m_pGrid=0;
												}
											}
					/** Increment the reference count on this micropoly.
					 */
			void	AddRef()				{m_RefCount++;}
					/** Decrement the reference count on this micropoly. Delete it if no longer referenced.
					 */
			void	Release()				{
												m_RefCount--;
												if(m_RefCount<=0)
													delete(this);
											}
					/** Get the color of this micropoly.
					 * \return CqColor reference.
					 */
	const	CqColor&	colColor()const	{return(m_pGrid->Ci()[m_Index]);}
					/** Get the opacity of this micropoly.
					 * \return CqColor reference.
					 */
	const	CqColor&	colOpacity()const	{return(m_pGrid->Oi()[m_Index]);}

					/** Assigment operator, copies contents of donor micropoly while safely deleting old contents.
					 * \param From Donor micropoly.
					 */
			CqMicroPolygonBase& operator=(const CqMicroPolygonBase& From)
											{
												if(m_pGrid!=NULL)	m_pGrid->Release();
												m_pGrid=From.m_pGrid;
												m_pGrid->AddRef();
												m_Index=From.m_Index;

												return(*this);
											}		
	// Overridables
					/** Pure virtual, get the bound of the micropoly.
					 * \param fForce Flag indicating do not get the stored bound, but recalculate it.
					 * \return CqBound representing the conservative bound.
					 */
	virtual	CqBound&	Bound(TqBool fForce=TqFalse)=0;
					/** Pure virtual, get the bound of the micropoly.
					 * \return CqBound representing the conservative bound.
					 */
	virtual const CqBound&	Bound() const=0;
					/** Pure virtual, check if the sample point is within the micropoly.
					 * \param vecSample 2D sample point.
					 * \param time The frame time at which to check.
					 * \param D storage to put the depth at the sample point if success.
					 * \return Boolean success.
					 */
	virtual	TqBool	Sample(CqVector2D& vecSample, TqFloat time, TqFloat& D)=0;

	protected:
			CqMicroPolyGrid*	m_pGrid;		///< Pointer to the donor grid.
			TqInt				m_Index;		///< Index within the donor grid.
			TqInt				m_RefCount;		///< Number of references to this micropoly.
};


//----------------------------------------------------------------------
/** \class CqMicroPolygonStaticBase
 * Base lass for static micropolygons. Stores point information about the geometry of the micropoly.
 */

class CqMicroPolygonStaticBase
{
	public:
					CqMicroPolygonStaticBase()	{}
					CqMicroPolygonStaticBase(const CqVector3D& vA, const CqVector3D& vB, const CqVector3D& vC, const CqVector3D& vD)	
										{
											Initialise(vA,vB,vC,vD);
										}
					CqMicroPolygonStaticBase(const CqMicroPolygonStaticBase& From)
										{
											*this=From;
										}
//	protected:
	// Private destructor, as destruction should be via Release()
	virtual			~CqMicroPolygonStaticBase()	{}
	
	public:
			CqMicroPolygonStaticBase& operator=(const CqMicroPolygonStaticBase& From)
											{
												m_vecPoints[0]=From.m_vecPoints[0];
												m_vecPoints[1]=From.m_vecPoints[1];
												m_vecPoints[2]=From.m_vecPoints[2];
												m_vecPoints[3]=From.m_vecPoints[3];
												m_vecN=From.m_vecN;
												m_D=From.m_D;
												return(*this);
											}		
			CqBound	Bound() const;
			void	Initialise(const CqVector3D& vA, const CqVector3D& vB, const CqVector3D& vC, const CqVector3D& vD);
			TqBool	fContains(const CqVector2D& vecP, TqFloat& Depth);
			CqMicroPolygonStaticBase&	LinearInterpolate(TqFloat Fraction, const CqMicroPolygonStaticBase& MPA, const CqMicroPolygonStaticBase& MPB);

	protected:
			CqVector3D	m_vecPoints[4];		///< Array of 4 3D vectors representing the micropoly.
			CqVector3D	m_vecN;				///< The normal to the micropoly.
			TqFloat		m_D;				///< Distance of the plane from the origin, used for calculating sample depth.
};


//----------------------------------------------------------------------
/** \class CqMicroPolygonStatic
 * Class which stores a single static micropolygon.
 */

class CqMicroPolygonStatic : public CqMicroPolygonBase, public CqMicroPolygonStaticBase, public CqPoolable<CqMicroPolygonStatic>
{
	public:
					CqMicroPolygonStatic() : CqMicroPolygonBase(), CqMicroPolygonStaticBase()
										{}
					CqMicroPolygonStatic(const CqMicroPolygonStatic& From) : CqMicroPolygonBase(From), CqMicroPolygonStaticBase(From)
										{}

					/** Overridden operator new to allocate micropolys from a pool.
					 */
					void* operator new(size_t size)
										{
											return(m_thePool.Alloc(size));
										}

					/** Overridden operator delete to allocate micropolys from a pool.
					 */
					void operator delete(void* p)
										{
											m_thePool.DeAlloc(p,sizeof(CqMicroPolygonStatic));
										}
//	private:
	// Private destructor, as destruction should be via Release()
	virtual			~CqMicroPolygonStatic()	{}

	// overrides from CqMicroPolygonBase
	virtual	CqBound&	Bound(TqBool fForce=TqFalse);
					/** Pure virtual, get the bound of the micropoly.
					 * \return CqBound representing the conservative bound.
					 */
	virtual const CqBound&	Bound() const	{return(m_Bound);}
	virtual	TqBool	Sample(CqVector2D& vecSample, TqFloat time, TqFloat& D);
	
	private:
			CqBound		m_Bound;		///< Stored bound.
	
	static	CqMemoryPool<CqMicroPolygonStatic>	m_thePool;	///< Static pool to allocated micropolys from.
};


//----------------------------------------------------------------------
/** \class CqMicroPolygonMotion
 * Class which stores a single moving micropolygon.
 */

class CqMicroPolygonMotion : public CqMicroPolygonBase, public CqMotionSpec<CqMicroPolygonStaticBase>
{
	public:
					CqMicroPolygonMotion() : CqMicroPolygonBase(), CqMotionSpec<CqMicroPolygonStaticBase>(CqMicroPolygonStaticBase())
										{ }
					CqMicroPolygonMotion(const CqMicroPolygonMotion& From) : CqMicroPolygonBase(From), CqMotionSpec<CqMicroPolygonStaticBase>(From)
										{
											*this=From;
										}
//	private:
	virtual			~CqMicroPolygonMotion()	{}
	
	public:
			CqMicroPolygonMotion& operator=(const CqMicroPolygonMotion& From)
											{
												CqMotionSpec<CqMicroPolygonStaticBase>::operator=(From);
												return(*this);
											}		

			void		ExpandBound(const CqMicroPolygonStaticBase& MP);
			void		Initialise(const CqVector3D& vA, const CqVector3D& vB, const CqVector3D& vC, const CqVector3D& vD, TqFloat time);

	// Overrides from CqMicroPolygonBase
	virtual	CqBound&	Bound(TqBool fForce=TqFalse);
					/** Pure virtual, get the bound of the micropoly.
					 * \return CqBound representing the conservative bound.
					 */
	virtual const CqBound&	Bound() const	{return(m_Bound);}
	virtual	TqBool	Sample(CqVector2D& vecSample, TqFloat time, TqFloat& D);

	// Overrides from CqMotionSpec
	virtual		void		ClearMotionObject(CqMicroPolygonStaticBase& A) const	{}
					/** Overridden from CqMotionSpec, does nothing.
					 */
	virtual		CqMicroPolygonStaticBase	ConcatMotionObjects(const CqMicroPolygonStaticBase& A, const CqMicroPolygonStaticBase& B) const	{return(B);}
					/** Overridden from CqMotionSpec, get the position of the micropoly linearly interpolated between the two extremes.
					 * \param Fraction The fractional distance between the two micropolys 0-1.
					 * \param A The start position.
					 * \param B The end position.
					 * \return a new micropoly at the requested position.
					 */
	virtual		CqMicroPolygonStaticBase	LinearInterpolateMotionObjects(TqFloat Fraction, const CqMicroPolygonStaticBase& A, const CqMicroPolygonStaticBase& B) const
							{
								CqMicroPolygonStaticBase MP;
								return(MP.LinearInterpolate(Fraction,A,B));
							}
	private:
			CqBound		m_Bound;		///< Stored bound.
};



//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !MICROPOLYGON_H_INCLUDED
 