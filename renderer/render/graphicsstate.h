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
#include	<boost/shared_ptr.hpp>
#include	<boost/enable_shared_from_this.hpp>

#include	"aqsis.h"

#include	"options.h"
#include	"attributes.h"
#include	"transform.h"
//#include	"messages.h"
#include	"lights.h"
#include	"csgtree.h"

START_NAMESPACE( Aqsis )

class CqBasicSurface;
class CqDeformingSurface;

enum EqModeBlock
{
    Outside = 0,
    BeginEnd,
    Frame,
    World,
    Attribute,
    Transform,
    Solid,
    Object,
    Motion
};

//----------------------------------------------------------------------
/** Abstract base class to handle the current context of the renderer,
 * stores information about the current scoping and previous contexts.
 */

class CqModeBlock : public boost::enable_shared_from_this<CqModeBlock>
{
	public:
		/** Default constructor
		 * \param pconParent a pointer to the previous context.
		 */
		CqModeBlock( const boost::shared_ptr<CqModeBlock>& pconParent, EqModeBlock modetype = Outside);
		virtual	~CqModeBlock();

		virtual	boost::shared_ptr<CqModeBlock>	BeginMainModeBlock();
		virtual	boost::shared_ptr<CqModeBlock>	BeginFrameModeBlock();
		virtual	boost::shared_ptr<CqModeBlock>	BeginWorldModeBlock();
		virtual	boost::shared_ptr<CqModeBlock>	BeginAttributeModeBlock();
		virtual	boost::shared_ptr<CqModeBlock>	BeginTransformModeBlock();
		virtual	boost::shared_ptr<CqModeBlock>	BeginSolidModeBlock( CqString& type );
		virtual	boost::shared_ptr<CqModeBlock>	BeginObjectModeBlock();
		virtual	boost::shared_ptr<CqModeBlock>	BeginMotionModeBlock( TqInt N, TqFloat times[] );

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqModeBlock");
		}
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
		virtual	CqTransformPtr	ptransCurrent() const
		{
			return ( m_ptransCurrent );
		}
		/** Set the current transform.
		 * \return a pointer to the old transform.
		 */
		virtual	CqTransformPtr	ptransSetCurrent(const CqTransformPtr& NewTrans)
		{
			CqTransformPtr prev = m_ptransCurrent;
			m_ptransCurrent = NewTrans;
			return ( prev );
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
		/// Get the current frame index if in a motion block.
		virtual	TqInt	TimeIndex() const
		{
			return( 0 );
		}
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
		boost::shared_ptr<CqModeBlock>	pconParent()
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
		const	CqMatrix&	matCurrent( TqFloat time ) const
		{
			return ( ptransCurrent() ->matObjectToWorld( time ) );
		}

		virtual	void	AddContextLightSource( const CqLightsourcePtr& pLS )
		{
			if ( pconParent() )
				pconParent() ->AddContextLightSource( pLS );
		}

		virtual	TqBool	isSolid()
		{
			return ( ( pconParent() ) ? pconParent() ->isSolid() : TqFalse );
		}
		/** Get a pointer to the CSG tree node related to this context.
		 * \return an options reference.
		 */
		virtual	boost::shared_ptr<CqCSGTreeNode>	pCSGNode()
		{
			return ( ( pconParent() ) ? pconParent() ->pCSGNode() : boost::shared_ptr<CqCSGTreeNode>() );
		}

		/** Log invalid context nesting
		 */
		virtual void logInvalidNesting() const;

	public:
		CqAttributes* m_pattrCurrent;		///< The current attributes.
		CqTransformPtr m_ptransCurrent;		///< The current transformation.

	private:
		boost::shared_ptr<CqModeBlock>	m_pconParent;			///< The previous context.
		EqModeBlock		m_modetype;				///< The current type of motionblock in order to double check the delete
}
;


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiBegin/RiEnd.
 */

class CqMainModeBlock : public CqModeBlock
{
	public:
		CqMainModeBlock( const boost::shared_ptr<CqModeBlock>& pconParent );
		virtual	~CqMainModeBlock();

		/** Create a main context.
		 * \warning It is an error to call this within a main context block.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginMainModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}		// Error

		/** Delete the main context.
		 * \attention This is the only valid context deletion from within this block.
		 */
		virtual	void	EndMainModeBlock()
		{}

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
		CqFrameModeBlock( const boost::shared_ptr<CqModeBlock>& pconParent );
		virtual	~CqFrameModeBlock();

		/** Create a main context.
		 * \warning It is an error to call this within a frame context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginMainModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}	// Error
		/** Create a frame context.
		 * \warning It is an error to call this within a frame context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginFrameModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}	// Error
		/** Create a solid context.
		 * \warning It is an error to call this within a frame context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginSolidModeBlock( CqString& type )
		{
			return boost::shared_ptr<CqModeBlock>();
		}	// Error

		/** Delete the frame context.
		 * \attention This is the only valid context deletion from within this block.
		 */
		virtual	void	EndFrameModeBlock()
		{}

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
		CqWorldModeBlock( const boost::shared_ptr<CqModeBlock>& pconParent );
		virtual	~CqWorldModeBlock();

		/** Create a main context.
		 * \warning It is an error to call this within a world context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginMainModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}	// Error
		/** Create a frame context.
		 * \warning It is an error to call this within a world context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginFrameModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}	// Error
		/** Create a world context.
		 * \warning It is an error to call this within a world context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginWorldModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}	// Error

		/** Delete the world context.
		 * \attention This is the only valid context deletion from within this block.
		 */
		virtual	void	EndWorldModeBlock()
		{}

		/** Get a reference to the options at the parent context, as world context doesn't store options.
		 * \return an options reference.
		 */
		virtual	CqOptions&	optCurrent()
		{
			return ( pconParent() ->optCurrent() );
		}

		virtual	void	AddContextLightSource( const CqLightsourcePtr& pLS );

	private:
		std::vector<CqLightsourcePtr>	m_apWorldLights;
};


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiAttributeBegin/RiAttributeEnd.
 */

class CqAttributeModeBlock : public CqModeBlock
{
	public:
		CqAttributeModeBlock( const boost::shared_ptr<CqModeBlock>& pconParent );
		virtual	~CqAttributeModeBlock();

		/** Create a main context.
		 * \warning It is an error to call this within a attribute context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginMainModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}	// Error
		/** Create a frame context.
		 * \warning It is an error to call this within a attribute context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginFrameModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}	// Error
		/** Create a world context.
		 * \warning It is an error to call this within a attribute context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginWorldModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}	// Error

		/** Delete the attribute context.
		 * \attention This is the only valid context deletion from within this block.
		 */
		virtual	void	EndAttributeModeBlock()
		{}

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
		CqTransformModeBlock( const boost::shared_ptr<CqModeBlock>& pconParent );
		virtual	~CqTransformModeBlock();

		/** Create a main context.
		 * \warning It is an error to call this within a transform context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginMainModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}	// Error
		/** Create a frame context.
		 * \warning It is an error to call this within a transform context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginFrameModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}	// Error
		/** Create a world context.
		 * \warning It is an error to call this within a transform context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginWorldModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}	// Error

		/** Delete the transform context.
		 * \attention This is the only valid context deletion from within this block.
		 */
		virtual	void	EndTransformModeBlock()
		{}

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
		CqSolidModeBlock( CqString& type, const boost::shared_ptr<CqModeBlock>& pconParent );
		virtual	~CqSolidModeBlock();

		/** Create a main context.
		 * \warning It is an error to call this within a solid context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginMainModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}	// Error
		/** Create a frame context.
		 * \warning It is an error to call this within a solid context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginFrameModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}	// Error
		/** Create a world context.
		 * \warning It is an error to call this within a solid context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginWorldModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}	// Error

		/** Delete the solid context.
		 * \attention This is the only valid context deletion from within this block.
		 */
		virtual	void	EndSolidModeBlock()
		{}

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
		virtual	boost::shared_ptr<CqCSGTreeNode>	pCSGNode()
		{
			return ( m_pCSGNode );
		}

	private:
		boost::shared_ptr<CqCSGTreeNode>	m_pCSGNode;			///< Pointer to the node in the CSG tree for this level in the solid definition.
		CqString		m_strType;
}
;


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiObjectBegin/RiObjectEnd.
 */

class CqObjectModeBlock : public CqModeBlock
{
	public:
		CqObjectModeBlock( const boost::shared_ptr<CqModeBlock>& pconParent );
		virtual	~CqObjectModeBlock();

		/** Create a main context.
		 * \warning It is an error to call this within an object context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginMainModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}		// Error
		/** Create a frame context.
		 * \warning It is an error to call this within an object context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginFrameModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}		// Error
		/** Create a world context.
		 * \warning It is an error to call this within an object context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginWorldModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}		// Error

		/** Delete the object context.
		 * \attention This is the only valid context deletion from within this block.
		 */
		virtual	void	EndObjectModeBlock()
		{}

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
			assert( TqFalse );
			return ( 0 );
		}	// Illegal to change attributes here.

	private:
};


//----------------------------------------------------------------------
/** Define the context that exists between calls to RiMotionBegin/RiMotionEnd.
 */

class CqMotionModeBlock : public CqModeBlock
{
	public:
		CqMotionModeBlock( TqInt N, TqFloat times[], const boost::shared_ptr<CqModeBlock>& pconParent );
		virtual	~CqMotionModeBlock();

		/** Create a main context.
		 * \warning It is an error to call this within a motion context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginMainModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}		// Error
		/** Create a frame context.
		 * \warning It is an error to call this within a motion context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginFrameModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}		// Error
		/** Create a world context.
		 * \warning It is an error to call this within a motion context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginWorldModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}		// Error
		/** Create a attribute context.
		 * \warning It is an error to call this within a motion context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginAttributeModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}	// Error
		/** Create a transform context.
		 * \warning It is an error to call this within a motion context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginTransformModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}	// Error
		/** Create a solid context.
		 * \warning It is an error to call this within a motion context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginSolidModeBlock( CqString& type )
		{
			return boost::shared_ptr<CqModeBlock>();
		}		// Error
		/** Create a object context.
		 * \warning It is an error to call this within a motion context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginObjectModeBlock()
		{
			return boost::shared_ptr<CqModeBlock>();
		}		// Error
		/** Create a motion context.
		 * \warning It is an error to call this within a motion context.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	BeginMotionModeBlock( TqInt N, TqFloat times[] )
		{
			return boost::shared_ptr<CqModeBlock>();
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
		/// Get the current frame index if in a motion block.
		virtual	TqInt	TimeIndex() const
		{
			return( m_iTime );
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
		virtual boost::shared_ptr<CqDeformingSurface> GetDeformingSurface() const
		{
			return( m_pDeformingSurface );
		}
		/** Set the CqDeformingSurface, if generating a deformation motion blur sequence.
		 */
		virtual void SetDeformingSurface( const boost::shared_ptr<CqDeformingSurface>& pMotionSurface);

	private:
		TqUint	m_iTime;		///< The index of the current frame time.
		std::vector<TqFloat>	m_aTimes;		///< An array of specified frame times.
		boost::shared_ptr<CqDeformingSurface>		m_pDeformingSurface;
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

inline boost::shared_ptr<CqModeBlock> CqModeBlock::BeginMainModeBlock()
{
	return boost::shared_ptr<CqModeBlock>( new CqMainModeBlock( shared_from_this() ) );
}


//----------------------------------------------------------------------
/** Create a new frame context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline boost::shared_ptr<CqModeBlock> CqModeBlock::BeginFrameModeBlock()
{
	return boost::shared_ptr<CqModeBlock>( new CqFrameModeBlock( shared_from_this() ) );
}


//----------------------------------------------------------------------
/** Create a new world context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline boost::shared_ptr<CqModeBlock> CqModeBlock::BeginWorldModeBlock()
{
	return boost::shared_ptr<CqModeBlock>( new CqWorldModeBlock( shared_from_this() ) );
}


//----------------------------------------------------------------------
/** Create a new attribute context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline boost::shared_ptr<CqModeBlock> CqModeBlock::BeginAttributeModeBlock()
{
	return boost::shared_ptr<CqModeBlock>( new CqAttributeModeBlock( shared_from_this() ) );
}


//----------------------------------------------------------------------
/** Create a new transform context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline boost::shared_ptr<CqModeBlock> CqModeBlock::BeginTransformModeBlock()
{
	return boost::shared_ptr<CqModeBlock>( new CqTransformModeBlock( shared_from_this() ) );
}


//----------------------------------------------------------------------
/** Create a new solid context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline boost::shared_ptr<CqModeBlock> CqModeBlock::BeginSolidModeBlock( CqString& type )
{
	return boost::shared_ptr<CqModeBlock>( new CqSolidModeBlock( type, shared_from_this() ) );
}


//----------------------------------------------------------------------
/** Create a new object context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline boost::shared_ptr<CqModeBlock> CqModeBlock::BeginObjectModeBlock()
{
	return boost::shared_ptr<CqModeBlock>( new CqObjectModeBlock( shared_from_this() ) );
}


//----------------------------------------------------------------------
/** Create a new motion context, and link it to the current one.
 * \return a pointer to the new context.
 */

inline boost::shared_ptr<CqModeBlock> CqModeBlock::BeginMotionModeBlock( TqInt N, TqFloat times[] )
{
	return boost::shared_ptr<CqModeBlock>( new CqMotionModeBlock( N, times, shared_from_this() ) );
}

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif
