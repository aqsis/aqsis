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

#include	"aqsis.h"

#include	"options.h"
#include	"attributes.h"
#include	"transform.h"
//#include	"messages.h"
#include	"list.h"
#include	"csgtree.h"

START_NAMESPACE( Aqsis )

class CqLightsource;
class CqBasicSurface;
class CqDeformingSurface;

enum EqModeBlock
{
    MainModeBlock = 0,
    FrameModeBlock,
    WorldModeBlock,
    AttributeModeBlock,
    TransformModeBlock,
    CsgModeBlock,
    ObjectModeBlock,
    MotionModeBlock,

    MainModeBlock_last,
};

//----------------------------------------------------------------------
/** Abstract base class to handle the current context of the renderer,
 * stores information about the current scoping and previous contexts.
 */

class CqModeBlock : public CqRefCount
{
public:
    /** Default constructor
     * \param pconParent a pointer to the previous context.
     */
    CqModeBlock( CqModeBlock* pconParent = 0, EqModeBlock modetype = MainModeBlock);
    virtual	~CqModeBlock();

    virtual	CqModeBlock*	BeginMainModeBlock();
    virtual	CqModeBlock*	BeginFrameModeBlock();
    virtual	CqModeBlock*	BeginWorldModeBlock();
    virtual	CqModeBlock*	BeginAttributeModeBlock();
    virtual	CqModeBlock*	BeginTransformModeBlock();
    virtual	CqModeBlock*	BeginSolidModeBlock( CqString& type );
    virtual	CqModeBlock*	BeginObjectModeBlock();
    virtual	CqModeBlock*	BeginMotionModeBlock( TqInt N, TqFloat times[] );

#ifdef _DEBUG
    CqString className() const { return CqString("CqModeBlock"); }
#endif

    /** Delete the main context, overridable per derived class.
     * \warning If called at this level it is an error, as only the appropriate context can delete itself.
     */
    virtual	void	EndMainModeBlock()
    {
        logInvalidNesting();
    }
    /** Delete the main context, overridable per derived class.
     * \warning If called at this level it is an error, as only the appropriate context can delete itself.
     */
    virtual	void	EndFrameModeBlock()
    {
        logInvalidNesting();
    }
    /** Delete the main context, overridable per derived class.
     * \warning If caled at this level it is an error, as only the appropriate context can delete itself.
     */
    virtual	void	EndWorldModeBlock()
    {
        logInvalidNesting();
    }
    /** Delete the main context, overridable per derived class.
     * \warning If called at this level it is an error, as only the appropriate context can delete itself.
     */
    virtual	void	EndAttributeModeBlock()
    {
        logInvalidNesting();
    }
    /** Delete the main context, overridable per derived class.
     * \warning If called at this level it is an error, as only the appropriate context can delete itself.
     */
    virtual	void	EndTransformModeBlock()
    {
        logInvalidNesting();
    }
    /** Delete the main context, overridable per derived class.
     * \warning If called at this level it is an error, as only the appropriate context can delete itself.
     */
    virtual	void	EndSolidModeBlock()
    {
        logInvalidNesting();
    }
    /** Delete the main context, overridable per derived class.
     * \warning If called at this level it is an error, as only the appropriate context can delete itself.
     */
    virtual	void	EndObjectModeBlock()
    {
        logInvalidNesting();
    }
    /** Delete the main context, overridable per derived class.
     * \warning If called at this level it is an error, as only the appropriate context can delete itself.
     */
    virtual	void	EndMotionModeBlock()
    {
        logInvalidNesting();
    }

    virtual	CqOptions&	optCurrent() = 0;
    /** Get a read only pointer to the current attributes.
     * \return a pointer to the current attributes.
     */
    virtual	const	CqAttributes*	pattrCurrent() const
    {
        return ( m_pattrCurrent );
    }
    /** Set the current set of attributes
     * \return a pointer to the old attributes.
     */
    virtual	const	CqAttributes*	pattrCurrent(CqAttributes* newattrs)
    {
        CqAttributes *prev = m_pattrCurrent;
        m_pattrCurrent = newattrs;
        return ( prev );
    }
    /** Get a pointer to the current attributes suitable for writing.
     * \return an attribute pointer.
     */
    virtual	CqAttributes*	pattrWriteCurrent()
    {
        m_pattrCurrent = m_pattrCurrent->Write();
        return ( m_pattrCurrent );
    }
    /** Get a read only pointer to the current transform.
     * \return a pointer to the current transform.
     */
    virtual	const	CqTransform*	ptransCurrent() const
    {
        return ( m_ptransCurrent );
    }
    /** Set the current transform.
     * \return a pointer to the old transform.
     */
    virtual	const	CqTransform*	ptransCurrent(CqTransform* newtrans)
    {
        CqTransform *prev = m_ptransCurrent;
        m_ptransCurrent = newtrans;
        return ( prev );
    }
    /** Get a pointer to the current transform suitable for writing.
     * \return a transform pointer.
     */
    virtual	CqTransform*	ptransWriteCurrent()
    {
        m_ptransCurrent = m_ptransCurrent->Write();
        return ( m_ptransCurrent );
    }
    /** Get the current time, used only within Motion blocks, all other contexts return 0.
     * \return the current frame time as a float.
     */
    virtual	TqFloat	Time() const
    {
        return ( 0.0f );
    }
    /// Advance the current frame time to the next specified time.
    virtual	void	AdvanceTime()
    {}
    /** Is this a motion block, should be overridden per derived class.
     * \return boolean indicating whether this is a motion block.
     */
    virtual	TqBool	fMotionBlock() const
    {
        return ( TqFalse );
    }

    /** Get a pointer to the previuos context.
     * \return a context pointer.
     */
    CqModeBlock*	pconParent()
    {
        return ( m_pconParent );
    }

    /** Get the type.
     * \return a m_ModeType;
     */
    EqModeBlock Type()
    {
        return ( m_modetype );
    }
    /** Get a reference to the current transformation matrix, the result of combining all transformations up to this point.
     * \return a matrix reference.
     */
    const	CqMatrix&	matCurrent( TqFloat time = 0.0f ) const
    {
        return ( ptransCurrent() ->matObjectToWorld( time ) );
    }

    virtual	void	AddContextLightSource( CqLightsource* pLS )
    {
        if ( pconParent() )
            pconParent() ->AddContextLightSource( pLS );
    }

    virtual	TqBool	isSolid()
    {
        return ( ( NULL != pconParent() ) ? pconParent() ->isSolid() : TqFalse );
    }
    /** Get a pointer to the CSG tree node related to this context.
     * \return an options reference.
     */
    virtual	CqCSGTreeNode*	pCSGNode()
    {
        return ( ( NULL != pconParent() ) ? pconParent() ->pCSGNode() : NULL );
    }

    /** Log invalid context nesting
     */
    virtual void logInvalidNesting() const;

public:
    static	CqList<CqCSGTreeNode>	m_lCSGTrees;
public:
    CqAttributes* m_pattrCurrent;		///< The current attributes.
    CqTransform* m_ptransCurrent;		///< The current transformation.

private:
    CqModeBlock*	m_pconParent;			///< The previous context.
    EqModeBlock		m_modetype;				///< The current type of motionblock in order to double check the delete
}
;


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiBegin/RiEnd.
 */

class CqMainModeBlock : public CqModeBlock
{
public:
    CqMainModeBlock( CqModeBlock* pconParent = 0 );
    virtual	~CqMainModeBlock();

    /** Create a main context.
     * \warning It is an error to call this within a main context block.
     */
    virtual	CqModeBlock*	BeginMainModeBlock()
    {
        return ( 0 );
    }		// Error

    /** Delete the main context.
     * \attention This is the only valid context deletion from within this block.
     */
    virtual	void	EndMainModeBlock()
    {
    }

    /** Get a reference to the currently stored options.
     * \return an options reference.
     */
    virtual	CqOptions&	optCurrent()
    {
        return ( m_optCurrent );
    }

private:
    CqOptions	m_optCurrent;	///< Current graphics environment options.
}
;


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiFrameBegin/RiFrameEnd.
 */

class CqFrameModeBlock : public CqModeBlock
{
public:
    CqFrameModeBlock( CqModeBlock* pconParent = 0 );
    virtual	~CqFrameModeBlock();

    /** Create a main context.
     * \warning It is an error to call this within a frame context.
     */
    virtual	CqModeBlock*	BeginMainModeBlock()
    {
        return ( 0 );
    }	// Error
    /** Create a frame context.
     * \warning It is an error to call this within a frame context.
     */
    virtual	CqModeBlock*	BeginFrameModeBlock()
    {
        return ( 0 );
    }	// Error
    /** Create a solid context.
     * \warning It is an error to call this within a frame context.
     */
    virtual	CqModeBlock*	BeginSolidModeBlock( CqString& type )
    {
        return ( 0 );
    }	// Error

    /** Delete the frame context.
     * \attention This is the only valid context deletion from within this block.
     */
    virtual	void	EndFrameModeBlock()
    {
    }

    /** Get a reference to the currently stored options.
     * \return an options reference.
     */
    virtual	CqOptions&	optCurrent()
    {
        return ( m_optCurrent );
    }

private:
    CqOptions	m_optCurrent;	///< Current graphics environment components
}
;


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiWorldBegin/RiWorldEnd.
 */

class CqWorldModeBlock : public CqModeBlock
{
public:
    CqWorldModeBlock( CqModeBlock* pconParent = 0 );
    virtual	~CqWorldModeBlock();

    /** Create a main context.
     * \warning It is an error to call this within a world context.
     */
    virtual	CqModeBlock*	BeginMainModeBlock()
    {
        return ( 0 );
    }	// Error
    /** Create a frame context.
     * \warning It is an error to call this within a world context.
     */
    virtual	CqModeBlock*	BeginFrameModeBlock()
    {
        return ( 0 );
    }	// Error
    /** Create a world context.
     * \warning It is an error to call this within a world context.
     */
    virtual	CqModeBlock*	BeginWorldModeBlock()
    {
        return ( 0 );
    }	// Error

    /** Delete the world context.
     * \attention This is the only valid context deletion from within this block.
     */
    virtual	void	EndWorldModeBlock()
    {
    }

    /** Get a reference to the options at the parent context, as world context doesn't store options.
     * \return an options reference.
     */
    virtual	CqOptions&	optCurrent()
    {
        return ( pconParent() ->optCurrent() );
    }

    virtual	void	AddContextLightSource( CqLightsource* pLS );

private:
    std::vector<CqLightsource*>	m_apWorldLights;
};


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiAttributeBegin/RiAttributeEnd.
 */

class CqAttributeModeBlock : public CqModeBlock
{
public:
    CqAttributeModeBlock( CqModeBlock* pconParent = 0 );
    virtual	~CqAttributeModeBlock();

    /** Create a main context.
     * \warning It is an error to call this within a attribute context.
     */
    virtual	CqModeBlock*	BeginMainModeBlock()
    {
        return ( 0 );
    }	// Error
    /** Create a frame context.
     * \warning It is an error to call this within a attribute context.
     */
    virtual	CqModeBlock*	BeginFrameModeBlock()
    {
        return ( 0 );
    }	// Error
    /** Create a world context.
     * \warning It is an error to call this within a attribute context.
     */
    virtual	CqModeBlock*	BeginWorldModeBlock()
    {
        return ( 0 );
    }	// Error

    /** Delete the attribute context.
     * \attention This is the only valid context deletion from within this block.
     */
    virtual	void	EndAttributeModeBlock()
    {
    }

    /** Get a reference to the options at the parent context, as attribute context doesn't store options.
     * \return an options reference.
     */
    virtual	CqOptions&	optCurrent()
    {
        return ( pconParent() ->optCurrent() );
    }

private:
};


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiTransformBegin/RiTransformEnd.
 */

class CqTransformModeBlock : public CqModeBlock
{
public:
    CqTransformModeBlock( CqModeBlock* pconParent = 0 );
    virtual	~CqTransformModeBlock();

    /** Create a main context.
     * \warning It is an error to call this within a transform context.
     */
    virtual	CqModeBlock*	BeginMainModeBlock()
    {
        return ( 0 );
    }	// Error
    /** Create a frame context.
     * \warning It is an error to call this within a transform context.
     */
    virtual	CqModeBlock*	BeginFrameModeBlock()
    {
        return ( 0 );
    }	// Error
    /** Create a world context.
     * \warning It is an error to call this within a transform context.
     */
    virtual	CqModeBlock*	BeginWorldModeBlock()
    {
        return ( 0 );
    }	// Error

    /** Delete the transform context.
     * \attention This is the only valid context deletion from within this block.
     */
    virtual	void	EndTransformModeBlock()
    {
    }

    /** Get a reference to the options at the parent context, as transform context doesn't store options.
     * \return an options reference.
     */
    virtual	CqOptions&	optCurrent()
    {
        return ( pconParent() ->optCurrent() );
    }

private:
};


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiSolidBegin/RiSolidEnd.
 */

class CqSolidModeBlock : public CqModeBlock
{
public:
    CqSolidModeBlock( CqString& type, CqModeBlock* pconParent = 0 );
    virtual	~CqSolidModeBlock();

    /** Create a main context.
     * \warning It is an error to call this within a solid context.
     */
    virtual	CqModeBlock*	BeginMainModeBlock()
    {
        return ( 0 );
    }	// Error
    /** Create a frame context.
     * \warning It is an error to call this within a solid context.
     */
    virtual	CqModeBlock*	BeginFrameModeBlock()
    {
        return ( 0 );
    }	// Error
    /** Create a world context.
     * \warning It is an error to call this within a solid context.
     */
    virtual	CqModeBlock*	BeginWorldModeBlock()
    {
        return ( 0 );
    }	// Error

    /** Delete the solid context.
     * \attention This is the only valid context deletion from within this block.
     */
    virtual	void	EndSolidModeBlock()
    {
    }

    /** Get a reference to the options at the parent context, as solid context doesn't store options.
     * \return an options reference.
     */
    virtual	CqOptions&	optCurrent()
    {
        return ( pconParent() ->optCurrent() );
    }


    virtual	TqBool	isSolid()
    {
        return ( TqTrue );
    }
    virtual	CqCSGTreeNode*	pCSGNode()
    {
        return ( m_pCSGNode );
    }

private:
    CqCSGTreeNode*	m_pCSGNode;			///< Pointer to the node in the CSG tree for this level in the solid definition.
}
;


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiObjectBegin/RiObjectEnd.
 */

class CqObjectModeBlock : public CqModeBlock
{
public:
    CqObjectModeBlock( CqModeBlock* pconParent = 0 );
    virtual	~CqObjectModeBlock();

    /** Create a main context.
     * \warning It is an error to call this within an object context.
     */
    virtual	CqModeBlock*	BeginMainModeBlock()
    {
        return ( 0 );
    }		// Error
    /** Create a frame context.
     * \warning It is an error to call this within an object context.
     */
    virtual	CqModeBlock*	BeginFrameModeBlock()
    {
        return ( 0 );
    }		// Error
    /** Create a world context.
     * \warning It is an error to call this within an object context.
     */
    virtual	CqModeBlock*	BeginWorldModeBlock()
    {
        return ( 0 );
    }		// Error

    /** Delete the object context.
     * \attention This is the only valid context deletion from within this block.
     */
    virtual	void	EndObjectModeBlock()
    {
    }

    /** Get a reference to the options at the parent context, as object context doesn't store options.
     * \return an options reference.
     */
    virtual	CqOptions&	optCurrent()
    {
        return ( pconParent() ->optCurrent() );
    }
    /** Get a pointer suitable for writing to the attributes at the parent context, as object context doesn't store attributes.
     * \return an attributes pointer.
     */
    virtual	const	CqAttributes*	pattrCurrent()
    {
        return ( pconParent() ->pattrCurrent() );
    }
    /** Get a pointer to the attributes at the parent context, as object context doesn't store attributes.
     * \return an attributes pointer.
     */
    virtual	CqAttributes*	pattrWriteCurrent()
    {
        assert( TqFalse ); return ( 0 );
    }	// Illegal to change attributes here.

private:
};


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiMotionBegin/RiMotionEnd.
 */

class CqMotionModeBlock : public CqModeBlock
{
public:
    CqMotionModeBlock( TqInt N, TqFloat times[], CqModeBlock* pconParent = 0 );
    virtual	~CqMotionModeBlock();

    /** Create a main context.
     * \warning It is an error to call this within a motion context.
     */
    virtual	CqModeBlock*	BeginMainModeBlock()
    {
        return ( 0 );
    }		// Error
    /** Create a frame context.
     * \warning It is an error to call this within a motion context.
     */
    virtual	CqModeBlock*	BeginFrameModeBlock()
    {
        return ( 0 );
    }		// Error
    /** Create a world context.
     * \warning It is an error to call this within a motion context.
     */
    virtual	CqModeBlock*	BeginWorldModeBlock()
    {
        return ( 0 );
    }		// Error
    /** Create a attribute context.
     * \warning It is an error to call this within a motion context.
     */
    virtual	CqModeBlock*	BeginAttributeModeBlock()
    {
        return ( 0 );
    }	// Error
    /** Create a transform context.
     * \warning It is an error to call this within a motion context.
     */
    virtual	CqModeBlock*	BeginTransformModeBlock()
    {
        return ( 0 );
    }	// Error
    /** Create a solid context.
     * \warning It is an error to call this within a motion context.
     */
    virtual	CqModeBlock*	BeginSolidModeBlock( CqString& type )
    {
        return ( 0 );
    }		// Error
    /** Create a object context.
     * \warning It is an error to call this within a motion context.
     */
    virtual	CqModeBlock*	BeginObjectModeBlock()
    {
        return ( 0 );
    }		// Error
    /** Create a motion context.
     * \warning It is an error to call this within a motion context.
     */
    virtual	CqModeBlock*	BeginMotionModeBlock( TqInt N, TqFloat times[] )
    {
        return ( 0 );
    }		// Error

    /** Delete the object context.
     * \attention This is the only valid context deletion from within this block.
     */
    virtual	void	EndMotionModeBlock();

    /** Get a reference to the options at the parent context, as motion context doesn't store options.
     * \return an options reference.
     */
    virtual	CqOptions&	optCurrent()
    {
        return ( pconParent() ->optCurrent() );
    }

    /** Get the current time, as specified at initialisation of the block.
     * \return the current time as a float, or if beyond the last time specified, 0.
     */
    virtual	TqFloat	Time() const
    {
        if ( m_iTime < m_aTimes.size() )
            return ( m_aTimes[ m_iTime ] );
        else
            return ( 0.0f );
    }
    /// Advance the current time to the next specified time.
    virtual	void	AdvanceTime()
    {
        m_iTime++;
    }
    /** Indicate that this is a motion block.
     * \return boolean indicating whether this is a motion block.
     */
    virtual	TqBool	fMotionBlock() const
    {
        return ( TqTrue );
    }
    /** Get the CqDeformingSurface, if generating a deformation motion blur sequence.
     */
    virtual CqDeformingSurface* GetDeformingSurface() const
    {
        return( m_pDeformingSurface );
    }
    /** Set the CqDeformingSurface, if generating a deformation motion blur sequence.
     */
    virtual void SetDeformingSurface( CqDeformingSurface* pMotionSurface);

private:
    TqUint	m_iTime;		///< The index of the current frame time.
    std::vector<TqFloat>	m_aTimes;		///< An array of specified frame times.
    CqDeformingSurface*		m_pDeformingSurface;
}
;


//***
// Note: These inline functions are defined here to allow full definition of all
//		 required classes first.
//***

//----------------------------------------------------------------------
/** Create a new main context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline CqModeBlock* CqModeBlock::BeginMainModeBlock()
{
    return ( new CqMainModeBlock( this ) );
}


//----------------------------------------------------------------------
/** Create a new frame context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline CqModeBlock* CqModeBlock::BeginFrameModeBlock()
{
    return ( new CqFrameModeBlock( this ) );
}


//----------------------------------------------------------------------
/** Create a new world context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline CqModeBlock* CqModeBlock::BeginWorldModeBlock()
{
    return ( new CqWorldModeBlock( this ) );
}


//----------------------------------------------------------------------
/** Create a new attribute context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline CqModeBlock* CqModeBlock::BeginAttributeModeBlock()
{
    return ( new CqAttributeModeBlock( this ) );
}


//----------------------------------------------------------------------
/** Create a new transform context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline CqModeBlock* CqModeBlock::BeginTransformModeBlock()
{
    return ( new CqTransformModeBlock( this ) );
}


//----------------------------------------------------------------------
/** Create a new solid context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline CqModeBlock* CqModeBlock::BeginSolidModeBlock( CqString& type )
{
    return ( new CqSolidModeBlock( type, this ) );
}


//----------------------------------------------------------------------
/** Create a new object context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline CqModeBlock* CqModeBlock::BeginObjectModeBlock()
{
    return ( new CqObjectModeBlock( this ) );
}


//----------------------------------------------------------------------
/** Create a new motion context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline CqModeBlock* CqModeBlock::BeginMotionModeBlock( TqInt N, TqFloat times[] )
{
    return ( new CqMotionModeBlock( N, times, this ) );
}

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif
