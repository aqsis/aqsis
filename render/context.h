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
		\brief Declares the RenderMan context classes.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is context.h included already?
#ifndef CONTEXT_H_INCLUDED
#define CONTEXT_H_INCLUDED 1

#include	<vector>

#include	"specific.h"

#include	"options.h"
#include	"attributes.h"
#include	"transform.h"
#include	"messages.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)

class CqLightsource;

//----------------------------------------------------------------------
/** Abstract base class to handle the current context of the renderer,
 * stores information about the current scoping and previous contexts.
 */

class CqContext
{
	public:
						/** Default constructor
						 * \param pconParent a pointer to the previous context.
						 */
						CqContext(CqContext* pconParent=0);
	virtual				~CqContext() {}

	virtual				CqContext*	CreateMainContext();
	virtual				CqContext*	CreateFrameContext();
	virtual				CqContext*	CreateWorldContext();
	virtual				CqContext*	CreateAttributeContext();
	virtual				CqContext*	CreateTransformContext();
	virtual				CqContext*	CreateSolidContext();
	virtual				CqContext*	CreateObjectContext();
	virtual				CqContext*	CreateMotionContext(TqInt N, TqFloat times[]);

						/** Delete the main context, overridable per derived class.
						 * \warning If called at this level it is an error, as only the appropriate context can delete itself.
						 */
	virtual				void		DeleteMainContext()	{CqBasicError(0,Severity_Fatal,"Invalid Context Nesting");}
						/** Delete the main context, overridable per derived class.
						 * \warning If called at this level it is an error, as only the appropriate context can delete itself.
						 */
	virtual				void		DeleteFrameContext() {CqBasicError(0,Severity_Fatal,"Invalid Context Nesting");}
						/** Delete the main context, overridable per derived class.
						 * \warning If caled at this level it is an error, as only the appropriate context can delete itself.
						 */
	virtual				void		DeleteWorldContext() {CqBasicError(0,Severity_Fatal,"Invalid Context Nesting");}
						/** Delete the main context, overridable per derived class.
						 * \warning If called at this level it is an error, as only the appropriate context can delete itself.
						 */
	virtual				void		DeleteAttributeContext() {CqBasicError(0,Severity_Fatal,"Invalid Context Nesting");}
						/** Delete the main context, overridable per derived class.
						 * \warning If called at this level it is an error, as only the appropriate context can delete itself.
						 */
	virtual				void		DeleteTransformContext() {CqBasicError(0,Severity_Fatal,"Invalid Context Nesting");}
						/** Delete the main context, overridable per derived class.
						 * \warning If called at this level it is an error, as only the appropriate context can delete itself.
						 */
	virtual				void		DeleteSolidContext() {CqBasicError(0,Severity_Fatal,"Invalid Context Nesting");}
						/** Delete the main context, overridable per derived class.
						 * \warning If called at this level it is an error, as only the appropriate context can delete itself.
						 */
	virtual				void		DeleteObjectContext() {CqBasicError(0,Severity_Fatal,"Invalid Context Nesting");}
						/** Delete the main context, overridable per derived class.
						 * \warning If called at this level it is an error, as only the appropriate context can delete itself.
						 */
	virtual				void		DeleteMotionContext() {CqBasicError(0,Severity_Fatal,"Invalid Context Nesting");}

	virtual				CqOptions&		optCurrent()=0;
						/** Get a read only pointer to the current attributes.
						 * \return a pointer to the current attributes.
						 */
	virtual	const	CqAttributes*	pattrCurrent() const {return(m_pattrCurrent);}
						/** Get a pointer to the current attributes suitable for writing.
						 * \return an attribute pointer.
						 */
	virtual			CqAttributes*	pattrWriteCurrent()	{
															m_pattrCurrent=m_pattrCurrent->Write();
															return(m_pattrCurrent);
														}
						/** Get a read only pointer to the current transform.
						 * \return a pointer to the current transform.
						 */
	virtual	const	CqTransform*	ptransCurrent()	const {return(m_ptransCurrent);}
						/** Get a pointer to the current transform suitable for writing.
						 * \return a transform pointer.
						 */
	virtual			CqTransform*	ptransWriteCurrent()	{
															m_ptransCurrent=m_ptransCurrent->Write();
															return(m_ptransCurrent);
														}
						/** Get the current time, used only within Motion blocks, all other contexts return 0.
						 * \return the current frame time as a float.
						 */
	virtual			TqFloat			Time() const		{return(0.0f);}
						/// Advance the current frame time to the next specified time.
	virtual			void			AdvanceTime()		{}
						/** Is this a motion block, should be overridden per derived class.
						 * \return boolean indicating whether this is a motion block.
						 */
	virtual			TqBool		fMotionBlock() const{return(TqFalse);}

						/** Get a pointer to the previuos context.
						 * \return a context pointer.
						 */
						CqContext*	pconParent()		{return(m_pconParent);}
						/** Get a reference to the current transformation matrix, the result of combining all transformations up to this point.
						 * \return a matrix reference.
						 */
				const	CqMatrix&	matCurrent(TqFloat time=0.0f) const	{return(ptransCurrent()->matObjectToWorld(time));}

//	protected:
			CqAttributes* m_pattrCurrent;		///< The current attributes.
			CqTransform*  m_ptransCurrent;		///< The current transformation.
	private:
			CqContext*	m_pconParent;			///< The previous context.
};


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiBegin/RiEnd.
 */

class CqMainContext : public CqContext
{
	public:
						CqMainContext(CqContext* pconParent=0);
	virtual				~CqMainContext();

						/** Create a main context.
						 * \warning It is an error to call this within a main context block.
						 */
	virtual	CqContext*	CreateMainContext()	{return(0);}		// Error

						/** Delete the main context.
						 * \attention This is the only valid context deletion from within this block.
						 */
	virtual	void		DeleteMainContext()	{delete(this);}

						/** Get a reference to the currently stored options.
						 * \return an options reference.
						 */
	virtual			CqOptions&		optCurrent()		{return(m_optCurrent);}
 
	private:
		CqOptions		m_optCurrent;	///< Current graphics environment options.
};


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiFrameBegin/RiFrameEnd.
 */

class CqFrameContext : public CqContext
{
	public:
						CqFrameContext(CqContext* pconParent=0);
	virtual				~CqFrameContext();

						/** Create a main context.
						 * \warning It is an error to call this within a frame context.
						 */
	virtual	CqContext*	CreateMainContext()	 {return(0);}	// Error
						/** Create a frame context.
						 * \warning It is an error to call this within a frame context.
						 */
	virtual	CqContext*	CreateFrameContext() {return(0);}	// Error
						/** Create a solid context.
						 * \warning It is an error to call this within a frame context.
						 */
	virtual	CqContext*	CreateSolidContext() {return(0);}	// Error

						/** Delete the frame context.
						 * \attention This is the only valid context deletion from within this block.
						 */
	virtual	void		DeleteFrameContext()	{delete(this);}

						/** Get a reference to the currently stored options.
						 * \return an options reference.
						 */
	virtual			CqOptions&		optCurrent()		{return(m_optCurrent);}
 
	private:
		CqOptions		m_optCurrent;	///< Current graphics environment components
};


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiWorldBegin/RiWorldEnd.
 */

class CqWorldContext : public CqContext
{
	public:
						CqWorldContext(CqContext* pconParent=0);
	virtual				~CqWorldContext();

						/** Create a main context.
						 * \warning It is an error to call this within a world context.
						 */
	virtual	CqContext*	CreateMainContext()	 {return(0);}	// Error
						/** Create a frame context.
						 * \warning It is an error to call this within a world context.
						 */
	virtual	CqContext*	CreateFrameContext() {return(0);}	// Error
						/** Create a world context.
						 * \warning It is an error to call this within a world context.
						 */
	virtual	CqContext*	CreateWorldContext() {return(0);}	// Error

						/** Delete the world context.
						 * \attention This is the only valid context deletion from within this block.
						 */
	virtual	void		DeleteWorldContext()	{delete(this);}

						/** Get a reference to the options at the parent context, as world context doesn't store options.
						 * \return an options reference.
						 */
	virtual			CqOptions&		optCurrent()		{return(pconParent()->optCurrent());}

	private:
};


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiAttributeBegin/RiAttributeEnd.
 */

class CqAttributeContext : public CqContext
{
	public:
						CqAttributeContext(CqContext* pconParent=0);
	virtual				~CqAttributeContext();

						/** Create a main context.
						 * \warning It is an error to call this within a attribute context.
						 */
	virtual	CqContext*	CreateMainContext()	 {return(0);}	// Error
						/** Create a frame context.
						 * \warning It is an error to call this within a attribute context.
						 */
	virtual	CqContext*	CreateFrameContext() {return(0);}	// Error
						/** Create a world context.
						 * \warning It is an error to call this within a attribute context.
						 */
	virtual	CqContext*	CreateWorldContext() {return(0);}	// Error

						/** Delete the attribute context.
						 * \attention This is the only valid context deletion from within this block.
						 */
	virtual	void		DeleteAttributeContext()	{delete(this);}

						/** Get a reference to the options at the parent context, as attribute context doesn't store options.
						 * \return an options reference.
						 */
	virtual			CqOptions&		optCurrent()		{return(pconParent()->optCurrent());}
 
	private:
};


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiTransformBegin/RiTransformEnd.
 */

class CqTransformContext : public CqContext
{
	public:
						CqTransformContext(CqContext* pconParent=0);
	virtual				~CqTransformContext();

						/** Create a main context.
						 * \warning It is an error to call this within a transform context.
						 */
	virtual	CqContext*	CreateMainContext()	 {return(0);}	// Error
						/** Create a frame context.
						 * \warning It is an error to call this within a transform context.
						 */
	virtual	CqContext*	CreateFrameContext() {return(0);}	// Error
						/** Create a world context.
						 * \warning It is an error to call this within a transform context.
						 */
	virtual	CqContext*	CreateWorldContext() {return(0);}	// Error

						/** Delete the transform context.
						 * \attention This is the only valid context deletion from within this block.
						 */
	virtual	void		DeleteTransformContext()	{delete(this);}

						/** Get a reference to the options at the parent context, as transform context doesn't store options.
						 * \return an options reference.
						 */
	virtual			CqOptions&		optCurrent()		{return(pconParent()->optCurrent());}
 
	private:
};


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiSolidBegin/RiSolidEnd.
 */

class CqSolidContext : public CqContext
{
	public:
						CqSolidContext(CqContext* pconParent=0);
	virtual				~CqSolidContext();

						/** Create a main context.
						 * \warning It is an error to call this within a solid context.
						 */
	virtual	CqContext*	CreateMainContext()	 {return(0);}	// Error
						/** Create a frame context.
						 * \warning It is an error to call this within a solid context.
						 */
	virtual	CqContext*	CreateFrameContext() {return(0);}	// Error
						/** Create a world context.
						 * \warning It is an error to call this within a solid context.
						 */
	virtual	CqContext*	CreateWorldContext() {return(0);}	// Error

						/** Delete the solid context.
						 * \attention This is the only valid context deletion from within this block.
						 */
	virtual	void		DeleteSolidContext()	{delete(this);}

						/** Get a reference to the options at the parent context, as solid context doesn't store options.
						 * \return an options reference.
						 */
	virtual			CqOptions&		optCurrent()		{return(pconParent()->optCurrent());}
 
	private:
};


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiObjectBegin/RiObjectEnd.
 */

class CqObjectContext : public CqContext
{
	public:
						CqObjectContext(CqContext* pconParent=0);
	virtual				~CqObjectContext();

						/** Create a main context.
						 * \warning It is an error to call this within an object context.
						 */
	virtual	CqContext*	CreateMainContext()	{return(0);}		// Error
						/** Create a frame context.
						 * \warning It is an error to call this within an object context.
						 */
	virtual	CqContext*	CreateFrameContext() {return(0);}		// Error
						/** Create a world context.
						 * \warning It is an error to call this within an object context.
						 */
	virtual	CqContext*	CreateWorldContext() {return(0);}		// Error

						/** Delete the object context.
						 * \attention This is the only valid context deletion from within this block.
						 */
	virtual	void		DeleteObjectContext()	{delete(this);}

						/** Get a reference to the options at the parent context, as object context doesn't store options.
						 * \return an options reference.
						 */
	virtual			CqOptions&		optCurrent()		{return(pconParent()->optCurrent());}
						/** Get a pointer suitable for writing to the attributes at the parent context, as object context doesn't store attributes.
						 * \return an attributes pointer.
						 */
	virtual	const	CqAttributes*	pattrCurrent()		{return(pconParent()->pattrCurrent());}
						/** Get a pointer to the attributes at the parent context, as object context doesn't store attributes.
						 * \return an attributes pointer.
						 */
	virtual			CqAttributes*	pattrWriteCurrent()	{assert(TqFalse); return(0);}	// Illegal to change attributes here.
 
	private:
};


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiMotionBegin/RiMotionEnd.
 */

class CqMotionContext : public CqContext
{
	public:
						CqMotionContext(TqInt N, TqFloat times[], CqContext* pconParent=0);
	virtual				~CqMotionContext();

						/** Create a main context.
						 * \warning It is an error to call this within a motion context.
						 */
	virtual	CqContext*	CreateMainContext()	{return(0);}		// Error
						/** Create a frame context.
						 * \warning It is an error to call this within a motion context.
						 */
	virtual	CqContext*	CreateFrameContext() {return(0);}		// Error
						/** Create a world context.
						 * \warning It is an error to call this within a motion context.
						 */
	virtual	CqContext*	CreateWorldContext() {return(0);}		// Error
						/** Create a attribute context.
						 * \warning It is an error to call this within a motion context.
						 */
	virtual	CqContext*	CreateAttributeContext() {return(0);}	// Error
						/** Create a transform context.
						 * \warning It is an error to call this within a motion context.
						 */
	virtual	CqContext*	CreateTransformContext() {return(0);}	// Error
						/** Create a solid context.
						 * \warning It is an error to call this within a motion context.
						 */
	virtual	CqContext*	CreateSolidContext() {return(0);}		// Error
						/** Create a object context.
						 * \warning It is an error to call this within a motion context.
						 */
	virtual	CqContext*	CreateObjectContext() {return(0);}		// Error
						/** Create a motion context.
						 * \warning It is an error to call this within a motion context.
						 */
	virtual	CqContext*	CreateMotionContext(TqInt N, TqFloat times[]) {return(0);}		// Error

						/** Delete the object context.
						 * \attention This is the only valid context deletion from within this block.
						 */
	virtual	void		DeleteMotionContext()	{delete(this);}

						/** Get a reference to the options at the parent context, as motion context doesn't store options.
						 * \return an options reference.
						 */
	virtual			CqOptions&		optCurrent()		{return(pconParent()->optCurrent());}

						/** Get the current time, as specified at initialisation of the block.
						 * \return the current time as a float, or if beyond the last time specified, 0.
						 */
	virtual			TqFloat			Time() const		{
															if(m_iTime<m_aTimes.size())
																return(m_aTimes[m_iTime]);
															else
																return(0.0f);
														}
						/// Advance the current time to the next specified time.
	virtual			void			AdvanceTime()		{m_iTime++;}
						/** Indicate that this is a motion block.
						 * \return boolean indicating whether this is a motion block.
						 */
	virtual			TqBool		fMotionBlock() const{return(TqTrue);}
 
	private:
			TqInt				m_iTime;		///< The index of the current frame time.
			std::vector<TqFloat>	m_aTimes;		///< An array of specified frame times.
};


//***
// Note: These inline functions are defined here to allow full definition of all
//		 required classes first.
//***

//----------------------------------------------------------------------
/** Create a new main context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline CqContext* CqContext::CreateMainContext()
{ 
	return(new CqMainContext(this));
}


//----------------------------------------------------------------------
/** Create a new frame context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline CqContext* CqContext::CreateFrameContext()
{ 
	return(new CqFrameContext(this));
}


//----------------------------------------------------------------------
/** Create a new world context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline CqContext* CqContext::CreateWorldContext()
{ 
	return(new CqWorldContext(this));
}


//----------------------------------------------------------------------
/** Create a new attribute context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline CqContext* CqContext::CreateAttributeContext()
{ 
	return(new CqAttributeContext(this));
}


//----------------------------------------------------------------------
/** Create a new transform context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline CqContext* CqContext::CreateTransformContext()
{ 
	return(new CqTransformContext(this));
}


//----------------------------------------------------------------------
/** Create a new solid context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline CqContext* CqContext::CreateSolidContext()
{ 
	return(new CqSolidContext(this));
}


//----------------------------------------------------------------------
/** Create a new object context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline CqContext* CqContext::CreateObjectContext()
{ 
	return(new CqObjectContext(this));
}


//----------------------------------------------------------------------
/** Create a new motion context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline CqContext* CqContext::CreateMotionContext(TqInt N, TqFloat times[])
{ 
	return(new CqMotionContext(N,times, this));
}

//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif
