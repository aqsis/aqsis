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
		\brief Implements the RenderMan context classes.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"

#include "context.h"

START_NAMESPACE(Aqsis)

//---------------------------------------------------------------------
/** Default constructor.
 */

CqContext::CqContext(CqContext* pconParent) : m_pconParent(pconParent), m_pattrCurrent(0), m_ptransCurrent(0)
{
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqMainContext::CqMainContext(CqContext* pconParent) : CqContext(pconParent)
{
	// Copy the current graphics state.
	if(pconParent!=0)
		m_optCurrent=pconParent->optCurrent();

	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent=new CqAttributes();
	m_pattrCurrent->Reference();
	m_ptransCurrent=new CqTransform();
	m_ptransCurrent->Reference();
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqMainContext::~CqMainContext()
{
	m_pattrCurrent->UnReference();
	m_ptransCurrent->UnReference();
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqFrameContext::CqFrameContext(CqContext* pconParent) : CqContext(pconParent)
{
	// Copy the current graphics state.
	m_optCurrent=pconParent->optCurrent();

	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent=new CqAttributes(*pconParent->m_pattrCurrent);
	m_pattrCurrent->Reference();
	m_ptransCurrent=new CqTransform(*pconParent->m_ptransCurrent);
	m_ptransCurrent->Reference();
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqFrameContext::~CqFrameContext()
{
	m_pattrCurrent->UnReference();
	m_ptransCurrent->UnReference();
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqWorldContext::CqWorldContext(CqContext* pconParent) : CqContext(pconParent)
{
	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent=new CqAttributes(*pconParent->m_pattrCurrent);
	m_pattrCurrent->Reference();
	m_ptransCurrent=new CqTransform(*pconParent->m_ptransCurrent);
	m_ptransCurrent->Reference();
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqWorldContext::~CqWorldContext()
{
	m_pattrCurrent->UnReference();
	m_ptransCurrent->UnReference();
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqAttributeContext::CqAttributeContext(CqContext* pconParent) : CqContext(pconParent)
{
	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent=new CqAttributes(*pconParent->m_pattrCurrent);
	m_pattrCurrent->Reference();
	m_ptransCurrent=new CqTransform(*pconParent->m_ptransCurrent);
	m_ptransCurrent->Reference();
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqAttributeContext::~CqAttributeContext()
{
	m_pattrCurrent->UnReference();
	m_ptransCurrent->UnReference();
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqTransformContext::CqTransformContext(CqContext* pconParent) : CqContext(pconParent)
{
	// Copy the parents attributes, as this state change doesn't save attributes.
	if(pconParent!=0)
		m_pattrCurrent=pconParent->m_pattrCurrent;
	else
		m_pattrCurrent=new CqAttributes();

	m_ptransCurrent=new CqTransform(*pconParent->m_ptransCurrent);
	m_ptransCurrent->Reference();
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqTransformContext::~CqTransformContext()
{
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqSolidContext::CqSolidContext(CqContext* pconParent) : CqContext(pconParent)
{
	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent=new CqAttributes(*pconParent->m_pattrCurrent);
	m_pattrCurrent->Reference();
	m_ptransCurrent=new CqTransform(*pconParent->m_ptransCurrent);
	m_ptransCurrent->Reference();
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqSolidContext::~CqSolidContext()
{
	m_pattrCurrent->UnReference();
	m_ptransCurrent->UnReference();
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqObjectContext::CqObjectContext(CqContext* pconParent) : CqContext(pconParent)
{
	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent=new CqAttributes();
	m_pattrCurrent->Reference();
	m_ptransCurrent=new CqTransform(*pconParent->m_ptransCurrent);
	m_ptransCurrent->Reference();
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqObjectContext::~CqObjectContext()
{
	m_pattrCurrent->UnReference();
	m_ptransCurrent->UnReference();
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqMotionContext::CqMotionContext(TqInt N, TqFloat times[], CqContext* pconParent) : CqContext(pconParent)
{
	// Copy the parents attributes, as this state change doesn't save attributes.
	if(pconParent!=0)
		m_pattrCurrent=pconParent->m_pattrCurrent;
	else
		m_pattrCurrent=new CqAttributes();

	if(pconParent!=0)
	{
		m_ptransCurrent=pconParent->m_ptransCurrent;
		// Set the default 'new time slot' matrix to the current 0 time matrix, this
		// takes care of the case of moving from non-Motion to Motion.
		m_ptransCurrent->SetDefaultObject(m_ptransCurrent->GetMotionObject(m_ptransCurrent->Time(0)));
	}
	else
		m_ptransCurrent=new CqTransform();


	// Store the array of times.
	TqInt i;
	for(i=0; i<N; i++)
		m_aTimes.push_back(times[i]);

	m_iTime=0;
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqMotionContext::~CqMotionContext()
{
}


//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)
