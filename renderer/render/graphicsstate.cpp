// Aqsis
// Copyright 1997 - 2001, Paul C. Gregory
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
		\brief Implements the RenderMan context classes.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include "lights.h"
#include "csgtree.h"
#include "renderer.h"
#include "surface.h"
#include "graphicsstate.h"

namespace Aqsis {



//---------------------------------------------------------------------
/** Default constructor.
 */

CqModeBlock::CqModeBlock( const boost::shared_ptr<CqModeBlock>& pconParent, EqModeBlock modetype ) :
		m_pattrCurrent(),
		m_ptransCurrent(),
		m_poptCurrent(),
		m_pconParent( pconParent ),
		m_modetype( modetype)
{}

//---------------------------------------------------------------------
/** Default Destructor.
 */

CqModeBlock::~CqModeBlock()
{}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqMainModeBlock::CqMainModeBlock( const boost::shared_ptr<CqModeBlock>& pconParent ) : CqModeBlock( pconParent, BeginEnd )
{
	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent.reset(new CqAttributes);
	m_ptransCurrent.reset(new CqTransform);
	m_poptCurrent.reset(new CqOptions);
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqMainModeBlock::~CqMainModeBlock()
{
	// Make sure any options pushed on the stack are cleared.
	while(!m_optionsStack.empty())
	{
		m_optionsStack.pop();
	}
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqFrameModeBlock::CqFrameModeBlock( const boost::shared_ptr<CqModeBlock>& pconParent ) : CqModeBlock( pconParent , Frame)
{
	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent.reset(new CqAttributes( *pconParent->m_pattrCurrent ));
	m_ptransCurrent.reset( new CqTransform(*pconParent->m_ptransCurrent.get() ) );
	m_poptCurrent.reset( new CqOptions(*pconParent->m_poptCurrent.get() ) );
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqFrameModeBlock::~CqFrameModeBlock()
{
	// Make sure any options pushed on the stack are cleared.
	while(!m_optionsStack.empty())
	{
		m_optionsStack.pop();
	}
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqWorldModeBlock::CqWorldModeBlock( const boost::shared_ptr<CqModeBlock>& pconParent ) : CqModeBlock( pconParent, World )
{
	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent.reset(new CqAttributes( *pconParent->m_pattrCurrent ));
	m_ptransCurrent.reset( new CqTransform( pconParent->m_ptransCurrent ) );
	m_poptCurrent.reset( new CqOptions(*pconParent->m_poptCurrent.get() ) );
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqWorldModeBlock::~CqWorldModeBlock()
{
}


//---------------------------------------------------------------------
/** Hold a reference to the specified lightsource in this context.
 */

void CqWorldModeBlock::AddContextLightSource( const CqLightsourcePtr& pLS )
{
	// Add the light to the context stack.
	m_apWorldLights.push_back( pLS );
}

//---------------------------------------------------------------------
/** Default constructor.
 */

CqAttributeModeBlock::CqAttributeModeBlock( const boost::shared_ptr<CqModeBlock>& pconParent ) : CqModeBlock( pconParent, Attribute )
{
	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent.reset(new CqAttributes( *pconParent->m_pattrCurrent ));
	m_ptransCurrent.reset( new CqTransform(*pconParent->m_ptransCurrent.get() ) );
	m_poptCurrent.reset( new CqOptions(*pconParent->m_poptCurrent.get() ) );
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqAttributeModeBlock::~CqAttributeModeBlock()
{
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqTransformModeBlock::CqTransformModeBlock( const boost::shared_ptr<CqModeBlock>& pconParent ) : CqModeBlock( pconParent, Transform )
{
	// Copy the parents attributes, as this state change doesn't save attributes.
	if ( pconParent )
		m_pattrCurrent = pconParent->m_pattrCurrent;
	else
	{
		m_pattrCurrent.reset(new CqAttributes);
	}
	m_ptransCurrent.reset( new CqTransform(*pconParent->m_ptransCurrent.get() ) );
	m_poptCurrent.reset( new CqOptions(*pconParent->m_poptCurrent.get() ) );
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqTransformModeBlock::~CqTransformModeBlock()
{}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqSolidModeBlock::CqSolidModeBlock( CqString& type, const boost::shared_ptr<CqModeBlock>& pconParent ) : CqModeBlock( pconParent, Solid ), m_strType( type )
{
	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent.reset(new CqAttributes( *pconParent->m_pattrCurrent ));
	m_ptransCurrent.reset( new CqTransform(*pconParent->m_ptransCurrent.get() ) );
	m_poptCurrent.reset( new CqOptions(*pconParent->m_poptCurrent.get() ) );

	// Create a new CSG tree node of the appropriate type.
	m_pCSGNode = CqCSGTreeNode::CreateNode( type );

	if ( pconParent && pconParent->isSolid() )
	{
		CqSolidModeBlock * pParentSolid = static_cast<CqSolidModeBlock*>( pconParent.get() );
		// Check if we are linking under a Primitive node, if so warn, and link at the top.
		if ( pParentSolid->pCSGNode() ->NodeType() == CqCSGTreeNode::CSGNodeType_Primitive )
		{
			CqString objname( "unnamed" );
			const CqString* pattrName = m_pattrCurrent->GetStringAttribute( "identifier", "name" );
			if ( pattrName != 0 )
				objname = pattrName[ 0 ];
			Aqsis::log() << warning << "Cannot add solid block under 'Primitive' \"" << objname.c_str() << "\" solid block" << std::endl;

		}
		else
			pParentSolid->pCSGNode() ->AddChild( m_pCSGNode );
	}
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqSolidModeBlock::~CqSolidModeBlock()
{
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqObjectModeBlock::CqObjectModeBlock( const boost::shared_ptr<CqModeBlock>& pconParent ) : CqModeBlock( pconParent, Object )
{
	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent.reset(new CqAttributes());
	m_ptransCurrent.reset( new CqTransform(*pconParent->m_ptransCurrent.get() ) );
	m_poptCurrent.reset( new CqOptions(*pconParent->m_poptCurrent.get() ) );
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqObjectModeBlock::~CqObjectModeBlock()
{
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqMotionModeBlock::CqMotionModeBlock( TqInt N, TqFloat times[], const boost::shared_ptr<CqModeBlock>& pconParent ) : CqModeBlock( pconParent, Motion )
{
	// Copy the parents attributes, as this state change doesn't save attributes.
	if ( pconParent )
		m_pattrCurrent = pconParent->m_pattrCurrent;
	else
	{
		m_pattrCurrent.reset(new CqAttributes);
	}

	if ( pconParent )
	{
		m_ptransCurrent.reset( new CqTransform(*pconParent->m_ptransCurrent.get() ) );
		m_poptCurrent.reset( new CqOptions(*pconParent->m_poptCurrent.get() ) );
		// Set the default 'new time slot' matrix to the current 0 time matrix, this
		// takes care of the case of moving from non-Motion to Motion.
		//m_ptransCurrent->SetDefaultObject( m_ptransCurrent->GetMotionObject( m_ptransCurrent->Time( 0 ) ) );
		SqTransformation ct;
		ct.m_matTransform = m_ptransCurrent->matObjectToWorld( m_ptransCurrent->Time( 0 ) );
		ct.m_Handedness = m_ptransCurrent->GetHandedness( m_ptransCurrent->Time( 0 ) );
		m_ptransCurrent->SetDefaultObject( ct );
	}
	else
	{
		m_ptransCurrent.reset(new CqTransform);
		m_poptCurrent.reset(new CqOptions);
	}


	// Store the array of times.
	TqInt i;
	for ( i = 0; i < N; i++ )
		m_aTimes.push_back( times[ i ] );

	m_iTime = 0;
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqMotionModeBlock::~CqMotionModeBlock()
{}

/** Delete the object context.
 * \attention This is the only valid context deletion from within this block.
 */
void CqMotionModeBlock::EndMotionModeBlock()
{
	if( m_pDeformingSurface )
	{
		QGetRenderContext()->StorePrimitive( m_pDeformingSurface );
		STATS_INC( GPR_created );
	}
}

/** Set the CqDeformingSurface, if generating a deformation motion blur sequence.
 */
void CqMotionModeBlock::SetDeformingSurface( const boost::shared_ptr<CqDeformingSurface>& pDeformingSurface)
{
	m_pDeformingSurface = pDeformingSurface;
}

//---------------------------------------------------------------------
/** Default constructor.
 */

CqResourceModeBlock::CqResourceModeBlock( const boost::shared_ptr<CqModeBlock>& pconParent ) : CqModeBlock( pconParent, Attribute )
{
	// Create new Attributes as they must be pushed/popped by the state change.
/*	m_pattrCurrent = new CqAttributes( *pconParent->m_pattrCurrent );
	ADDREF( m_pattrCurrent );
	m_ptransCurrent = CqTransformPtr( new CqTransform(*pconParent->m_ptransCurrent.get() ) );
	m_poptCurrent = CqOptionsPtr( new CqOptions(*pconParent->m_poptCurrent.get() ) );*/
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqResourceModeBlock::~CqResourceModeBlock()
{
}

void CqModeBlock::logInvalidNesting() const
{
	Aqsis::log() << critical << "Invalid context nesting" << std::endl;
}


} // namespace Aqsis
