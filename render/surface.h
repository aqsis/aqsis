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
		\brief Declares the base GPrim handling classes.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef SURFACE_H_INCLUDED
//{
#define SURFACE_H_INCLUDED 1

#include	"ri.h"

#include	"attributes.h"
#include	"transform.h"
#include	"list.h"
#include	"matrix.h"
#include	"parameters.h"
#include	"specific.h"
#include	"bound.h"
#include	"micropolygon.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)


//----------------------------------------------------------------------
/** \class CqBasicSurface
 * Abstract base surface class, which provides interfaces to geometry.  
 */

class CqBasicSurface : public CqListEntry<CqBasicSurface>
{
	public:
						CqBasicSurface();
						CqBasicSurface(const CqBasicSurface& From);
	virtual				~CqBasicSurface()	{
											// Release our reference on the current attributes.
											if(m_pAttributes)
												m_pAttributes->UnReference();
											m_pAttributes=0;
											if(m_pTransform)
												m_pTransform->UnReference();
											m_pTransform=0;
										}

						/** Get the gemoetric bound of this GPrim.
						 */
	virtual	CqBound		Bound() const=0;
						/** Dice this GPrim.
						 * \return A pointer to a new micropolygrid..
						 */
	virtual	CqMicroPolyGridBase* Dice()=0;
						/** Split this GPrim into a number of other GPrims.
						 * \param aSplits A reference to a CqBasicSurface array to fill in with the new GPrim pointers.
						 * \return Integer count of new GPrims created.
						 */
	virtual	TqInt		Split(std::vector<CqBasicSurface*>& aSplits)=0;
						/** Determine whether this GPrim is diceable at its current size.
						 */
	virtual TqBool		Diceable()=0;

	virtual	void		Reset()		{}

						/** Transform this GPrim using the specified matrices.
						 * \param matTx Reference to the transformation matrix.
						 * \param matITTx Reference to the inverse transpose of the transformation matrix, used to transform normals.
						 * \param matRTx Reference to the rotation only transformation matrix, used to transform vectors.
						 */
	virtual void		Transform(const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx)=0;
						/** Get the number of uniform parameters required for this GPrim.
						 */
	virtual	TqInt		cUniform() const=0;
						/** Get the number of varying parameters required for this GPrim.
						 */
	virtual	TqInt		cVarying() const=0;
						/** Get the number of vertex parameters required for this GPrim.
						 */
	virtual	TqInt		cVertex() const=0;

	virtual CqString	strName() const;
	virtual	TqInt		Uses() const;

						/** Get a pointer to the attributes state associated with this GPrim.
						 * \return A pointer to a CqAttributes class.
						 */
	const	CqAttributes* pAttributes()	const {return(m_pAttributes);}
						/** Get a pointer to the transformation state associated with this GPrim.
						 * \return A pointer to a CqTransform class.
						 */
	const	CqTransform* pTransform()	const {return(m_pTransform);}
	virtual	void		SetSurfaceParameters(const CqBasicSurface& From);
						/** Force this GPrim to be undiceable, usually if it crosses the epsilon and eye plane.
						 */
	virtual	void		ForceUndiceable()	{m_fDiceable=TqFalse; m_EyeSplitCount++;}
						/** Force this GPrim to be discarded, usually if it has been split too many times due to crossing the epsilon and eye planes..
						 */
	virtual	void		Discard()			{m_fDiscard=TqTrue;}

						/** Determine whether this GPrim is to be discardrd.
						 */
			TqBool		fDiscard() const	{return(m_fDiscard);}
						/** Get the number of times this GPrim has been split because if crossing the epsilon and eye planes.
						 */
			TqInt		EyeSplitCount() const	{return(m_EyeSplitCount);}
						/** Copy the information about splitting and dicing from the specified GPrim.
						 * \param From A CqBasicSurface reference to copy the information from.
						 */
			void		CopySplitInfo(const CqBasicSurface& From)
											{
												m_uDiceSize=From.m_uDiceSize;
												m_vDiceSize=From.m_vDiceSize;
												m_SplitDir=From.m_SplitDir;
											}

			void		ExpandBoundForMotion(CqBound& Bound);

			CqBasicSurface&	operator=(const CqBasicSurface& From);

			TqBool		m_fDiceable;		///< Flag to indicate that this GPrim is diceable.
			TqBool		m_fDiscard;			///< Flag to indicate that this GPrim is to be discarded.
			TqInt		m_EyeSplitCount;	///< The number of times this GPrim has been split because if crossing the epsilon and eye planes.
	protected:
			CqAttributes* m_pAttributes;	///< Pointer to the attributes state associated with this GPrim.
			CqTransform* m_pTransform;		///< Pointer to the transformation state associated with this GPrim.

			TqInt		m_uDiceSize;		///< Calculated dice size to achieve an appropriate shading rate.
			TqInt		m_vDiceSize;		///< Calculated dice size to achieve an appropriate shading rate.
			enum EqSplitDir
			{
				SplitDir_U,
				SplitDir_V,
			};
			EqSplitDir	m_SplitDir;			///< The direction to split this GPrim to achieve best results.
};


//----------------------------------------------------------------------
/** \class CqSurface
 * Abstract base surface class, which provides interfaces to a geometric surface.  
 */

class _qShareC CqSurface : public CqBasicSurface
{
	public:
						CqSurface();
						CqSurface(const CqSurface& From);
	virtual				~CqSurface()	{}

	virtual	void		SetDefaultPrimitiveVariables(TqBool bUseDef_st=TqTrue);

			CqSurface&	operator=(const CqSurface& From);

						/** Get a reference the to P default parameter.
						 */
			CqParameterTypedVarying<CqVector4D, Type_hPoint>& P()	{return(m_P);}
						/** Get a reference the to N default parameter.
						 */
			CqParameterTypedVarying<CqVector3D, Type_Normal>& N()	{return(m_N);}
						/** Get a reference the to Cq default parameter.
						 */
			CqParameterTypedVarying<CqColor, Type_Color>& Cs()		{return(m_Cs);}
						/** Get a reference the to Os default parameter.
						 */
			CqParameterTypedVarying<CqColor, Type_Color>& Os()		{return(m_Os);}
						/** Get a reference the to s default parameter.
						 */
			CqParameterTypedVarying<TqFloat, Type_Float>& s()		{return(m_s);}
						/** Get a reference the to t default parameter.
						 */
			CqParameterTypedVarying<TqFloat, Type_Float>& t()		{return(m_t);}
						/** Get a reference the to u default parameter.
						 */
			CqParameterTypedVarying<TqFloat, Type_Float>& u()		{return(m_u);}
						/** Get a reference the to v default parameter.
						 */
			CqParameterTypedVarying<TqFloat, Type_Float>& v()		{return(m_v);}

						/** Get a reference the to P default parameter.
						 */
	const	CqParameterTypedVarying<CqVector4D, Type_hPoint>& P() const	{return(m_P);}
						/** Get a reference the to N default parameter.
						 */
	const	CqParameterTypedVarying<CqVector3D, Type_Normal>& N() const	{return(m_N);}
						/** Get a reference the to Cq default parameter.
						 */
	const	CqParameterTypedVarying<CqColor, Type_Color>& Cs() const	{return(m_Cs);}
						/** Get a reference the to Os default parameter.
						 */
	const	CqParameterTypedVarying<CqColor, Type_Color>& Os() const	{return(m_Os);}
						/** Get a reference the to s default parameter.
						 */
	const	CqParameterTypedVarying<TqFloat, Type_Float>& s() const		{return(m_s);}
						/** Get a reference the to t default parameter.
						 */
	const	CqParameterTypedVarying<TqFloat, Type_Float>& t() const		{return(m_t);}
						/** Get a reference the to u default parameter.
						 */
	const	CqParameterTypedVarying<TqFloat, Type_Float>& u() const		{return(m_u);}
						/** Get a reference the to v default parameter.
						 */
	const	CqParameterTypedVarying<TqFloat, Type_Float>& v() const		{return(m_v);}

	protected:
			// Default primitive variables
			CqParameterTypedVarying<CqVector4D, Type_hPoint>	m_P;		///< Default parameter P.
			CqParameterTypedVarying<CqVector3D, Type_Normal>	m_N;		///< Default parameter N.
			CqParameterTypedVarying<CqColor, Type_Color>		m_Cs;		///< Default parameter Cq.
			CqParameterTypedVarying<CqColor, Type_Color>		m_Os;		///< Default parameter Os.
			CqParameterTypedVarying<TqFloat, Type_Float>			m_s;		///< Default parameter s.
			CqParameterTypedVarying<TqFloat, Type_Float>			m_t;		///< Default parameter t.
			CqParameterTypedVarying<TqFloat, Type_Float>			m_u;		///< Default parameter u.
			CqParameterTypedVarying<TqFloat, Type_Float>			m_v;		///< Default parameter v.

};


//----------------------------------------------------------------------
/** \class CqMotionSurface
 * Templatised class containing a series of motion stages of a specific surface type for motion blurring.
 */

template<class T>
class CqMotionSurface : public CqBasicSurface, public CqMotionSpec<T>
{
	public:
						CqMotionSurface(const T& a) : CqBasicSurface(), CqMotionSpec<T>(a)	{}
						CqMotionSurface(const CqMotionSurface<T>& From) : CqBasicSurface(From), CqMotionSpec<T>(From) {}
	virtual				~CqMotionSurface()	
										{
											TqInt i;
											for(i=0; i<cTimes(); i++)
												delete(GetMotionObject(Time(i)));
										}

						/** Get combnied bound for all times
						 * \return CqBound representing the geometric boundary of this GPrim over all time slots.
						 */
	virtual	CqBound		Bound() const	{
											CqBound B(FLT_MAX,FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX);
											TqInt i;
											for(i=0; i<cTimes(); i++)
												B=B.Combine(GetMotionObject(Time(i))->Bound());

											return(B);
										}
						/** Dice this GPrim, creating a CqMotionMicroPolyGrid with all times in.
						 */
	virtual	CqMicroPolyGridBase* Dice()	{
											CqMotionMicroPolyGrid* pGrid=new CqMotionMicroPolyGrid;
											TqInt i;
											for(i=0; i<cTimes(); i++)
											{
												CqMicroPolyGridBase* pGrid2=GetMotionObject(Time(i))->Dice();
												pGrid->AddTimeSlot(Time(i),pGrid2);
											}
											return(pGrid);
										}
						/** Split this GPrim, creating a series of CqMotionSurfaces with all times in.
						 */
	virtual	TqInt		Split(std::vector<CqBasicSurface*>& aSplits)
										{
											std::vector<std::vector<CqBasicSurface*> > aaMotionSplits;
											aaMotionSplits.resize(cTimes());
											TqInt cSplits;
											TqInt i;
											for(i=0; i<cTimes(); i++)
												cSplits=GetMotionObject(Time(i))->Split(aaMotionSplits[i]);

											// Now build motion surfaces from the splits and pass them back.
											for(i=0; i<cSplits; i++)
											{
												CqMotionSurface<T>* pNewMotion=new CqMotionSurface<T>(0);
												pNewMotion->m_fDiceable=TqTrue;
												pNewMotion->m_EyeSplitCount=m_EyeSplitCount;
												TqInt j;
												for(j=0; j<cTimes(); j++)
													pNewMotion->AddTimeSlot(Time(j),reinterpret_cast<T>(aaMotionSplits[j][i]));
												aSplits.push_back(pNewMotion);
											}
											return(cSplits);
										}
						/** Determine if the prmary time slot is diceable, this is the one that is shaded, so 
						 * determines the dicing rate, which is then copied to the other times.
						 * \return Boolean indicating GPrim is diceable.
						 */
	virtual TqBool		Diceable()		{
											TqBool f=GetMotionObject(Time(0))->Diceable();
											// Copy the split info so that at each time slot, the gprims split the same.
											TqInt i;
											for(i=1; i<cTimes(); i++)
												GetMotionObject(Time(i))->CopySplitInfo(*GetMotionObject(Time(0)));
											return(f);
										}
	
						/** Transform all GPrims by the specified transformation matrices.
						 * \param matTx Reference to the transformation matrix.
						 * \param matITTx Reference to the inverse transpose of the transformation matrix, used to transform normals.
						 * \param matRTx Reference to the rotation only transformation matrix, used to transform vectors.
						 */ 
	virtual void		Transform(const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx)
										{
											GetMotionObject(0)->Transform(matTx, matITTx, matRTx);
										}

						/** Set the surface parameters of all GPrims to match those on the spefified one.
						 * \param From GPrim to copy parameters from.
						 */
	virtual	void		SetSurfaceParameters(const CqBasicSurface& From)
										{
											TqInt i;
											for(i=0; i<cTimes(); i++)
												GetMotionObject(Time(i))->SetSurfaceParameters(From);
										}
						/** Force all GPrims to be undiceable.
						 */
	virtual	void		ForceUndiceable()	
										{
											CqBasicSurface::ForceUndiceable();
											TqInt i;
											for(i=0; i<cTimes(); i++)
												GetMotionObject(Time(i))->ForceUndiceable();
										}

						/** Mark all GPrims to be discarded.
						 */
	virtual	void		Discard()		{
											CqBasicSurface::Discard();
											TqInt i;
											for(i=0; i<cTimes(); i++)
												GetMotionObject(Time(i))->Discard();
										}

	virtual	TqInt		cUniform() const	{return(GetMotionObject(Time(0))->cUniform());}
	virtual	TqInt		cVarying() const	{return(GetMotionObject(Time(0))->cVarying());}
	virtual	TqInt		cVertex() const		{return(GetMotionObject(Time(0))->cVertex());}

	// Overrides from CqMotionSpec		
	virtual		void		ClearMotionObject(T& A) const	{};
	virtual		T			ConcatMotionObjects(const T& A, const T& B) const	{return(A);}
	virtual		T			LinearInterpolateMotionObjects(TqFloat Fraction, const T& A, const T& B) const {return(A);}
	protected:
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

//}  // End of #ifdef SURFACE_H_INCLUDED
#endif
