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
		\brief Implements the CqTransform class for handling GPrim coordinate systems.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"
#include	"transform.h"
#include	"renderer.h"
#include	"imagebuffer.h"

START_NAMESPACE( Aqsis )

std::list<CqTransform*> Transform_stack;

//---------------------------------------------------------------------
/** Constructor.
 */

CqTransform::CqTransform() : CqMotionSpec<SqTransformation>(SqTransformation()), m_cReferences( 0 ), m_IsMoving(TqFalse)
{
	// Get the state of the transformation at the last stack entry, and use this as the default value for new timeslots.
	// if the previous level has motion specification, the value will be interpolated.
	CqMatrix matOtoWLast;
	TqBool handLast = TqFalse;
	if ( !Transform_stack.empty() )
	{
		matOtoWLast =  Transform_stack.front() ->matObjectToWorld(Transform_stack.front()->Time(0));
		handLast =  Transform_stack.front() ->GetHandedness(Transform_stack.front()->Time(0));
	}

	SqTransformation ct;
	ct.m_Handedness = handLast;
	ct.m_matTransform = matOtoWLast;
	SetDefaultObject( ct );

	// Register ourself with the global transform stack.
	Transform_stack.push_front( this );
	m_StackIterator = Transform_stack.begin();
}


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqTransform::CqTransform( const CqTransform& From ) : CqMotionSpec<SqTransformation>( From ), m_cReferences( 0 ), m_IsMoving(TqFalse)
{
    *this = From;

	// Get the state of the transformation at the last stack entry, and use this as the default value for new timeslots.
	// if the previous level has motion specification, the value will be interpolated.
	CqMatrix matOtoWLast;
	TqBool handLast = TqFalse;
	if ( !Transform_stack.empty() )
	{
		TqFloat time = QGetRenderContext()->Time();
		matOtoWLast =  Transform_stack.front() ->matObjectToWorld( time );
		handLast =  Transform_stack.front() ->GetHandedness( time );
	}

	SqTransformation ct;
	ct.m_Handedness = handLast;
	ct.m_matTransform = matOtoWLast;
	SetDefaultObject( ct );

   // Register ourself with the global transform stack.
	Transform_stack.push_front( this );
	m_StackIterator = Transform_stack.begin();
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqTransform::~CqTransform()
{
    assert( RefCount() == 0 );

	// Remove ourself from the stack
	Transform_stack.erase(m_StackIterator);
}

//---------------------------------------------------------------------
/** Copy function.
 */

CqTransform& CqTransform::operator=( const CqTransform& From )
{
    CqMotionSpec<SqTransformation>::operator=( From );

	m_IsMoving = From.m_IsMoving;
	m_StaticMatrix = From.m_StaticMatrix;
	m_Handedness = From.m_Handedness;

    return ( *this );
}


//---------------------------------------------------------------------
/** Set the transformation at the specified time.
 */

void CqTransform::SetCurrentTransform( TqFloat time, const CqMatrix& matTrans )
{
	TqFloat det = matTrans.Determinant();
	TqBool flip = ( !matTrans.fIdentity() && det < 0 );

	SqTransformation ct;
	ct.m_matTransform = matTrans;

    if ( QGetRenderContext() ->pconCurrent() ->fMotionBlock() )
	{
		AddTimeSlot( time, ct );
		m_IsMoving = TqTrue;
	}
	else
	{
		if( m_IsMoving )
		{
			AddTimeSlot( time, ct );
		}
		else
		{
			m_StaticMatrix = matTrans;
			m_Handedness = flip;
			ct.m_Handedness = flip;
			SetDefaultObject( ct );
		}
	}
}



void CqTransform::SetTransform( TqFloat time, const CqMatrix& matTrans )
{
	TqFloat det = matTrans.Determinant();
	TqBool flip = ( !matTrans.fIdentity() && det < 0 );
	CqMatrix matCtoW = QGetRenderContext()->matSpaceToSpace("world", "camera", CqMatrix(), CqMatrix(), QGetRenderContext()->Time());
	TqFloat camdet = matCtoW.Determinant();
	TqBool camhand = ( !matCtoW.fIdentity() && camdet < 0 );

    if ( QGetRenderContext() ->pconCurrent() ->fMotionBlock() )
	{
		SqTransformation ct;
		ct.m_Handedness = (flip)? !camhand : camhand;
		ct.m_matTransform = matTrans;
		AddTimeSlot( time, ct );
		m_IsMoving = TqTrue;
	}
	else
	{
		// If not in a motion block, but we are moving, apply the transform to all keys.
		if( m_IsMoving )
		{
			CqMatrix mat0 = matObjectToWorld(Time(0));
			
			SqTransformation ct;
			ct.m_Handedness = (flip)? !camhand : camhand;
			TqBool hand0 = ct.m_Handedness;
			ct.m_matTransform = matTrans;

			AddTimeSlot( Time(0), ct );
			TqInt i;
			for(i=1; i<cTimes(); i++)
			{
				CqMatrix matOffset = mat0 * matObjectToWorld(Time(i)).Inverse();
				ct.m_matTransform = matOffset * matTrans;
				TqBool flip2 = ( matOffset.Determinant() < 0 );
				ct.m_Handedness = (flip2)? !hand0 : hand0;
				AddTimeSlot( Time(i), ct);
			}
		}
		else
		{
			m_StaticMatrix = matTrans;
			m_Handedness = (flip)? !camhand : camhand;
		}
	}
}


//---------------------------------------------------------------------
/** Modify the transformation at the specified time.
 */

void CqTransform::ConcatCurrentTransform( TqFloat time, const CqMatrix& matTrans )
{
	TqFloat det = matTrans.Determinant();
	TqBool flip = ( !matTrans.fIdentity() && det < 0 );

	SqTransformation ct;
	ct.m_matTransform = matTrans;
	ct.m_Handedness = (flip)? !m_Handedness : m_Handedness;

    // If we are actually in a motion block, and we already describe a moving transform,
	// concatenate this transform with the existing one at that time slot,
	// ConcatTimeSlot will take care of making sure that the matrix is initially set to the 
	// static matrix, as long as we ensure that the default is kept up to date.
    if ( QGetRenderContext() ->pconCurrent() ->fMotionBlock() )
	{
		ConcatTimeSlot( time, ct );
		m_IsMoving = TqTrue;
	}
    else
    // else, if we are moving, apply this transform at all time slots, otherwise apply to static matrix.
	{
		if( m_IsMoving )
			ConcatAllTimeSlots( ct );
		else
		{
			m_StaticMatrix = m_StaticMatrix * matTrans;
			m_Handedness = (flip)? !m_Handedness : m_Handedness;
			ct.m_Handedness = m_Handedness;
			SetDefaultObject( ct );
		}
	}
}


//---------------------------------------------------------------------
/** Get the transformation at the specified time.
 */

const CqMatrix& CqTransform::matObjectToWorld( TqFloat time ) const
{
	if( m_IsMoving )
		return ( GetMotionObject( time ).m_matTransform );
	else
		return ( m_StaticMatrix );
}


//---------------------------------------------------------------------
/** Get the handedness at the specified time.
 */

TqBool CqTransform::GetHandedness( TqFloat time ) const
{
	if( m_IsMoving )
		return ( GetMotionObject( time ).m_Handedness );
	else
		return ( m_Handedness );
}


//---------------------------------------------------------------------
/** Reset the transform to the specified matrix, if the flag makeStatic is true
 *  then make the transform static and clear the motion keyframes.
 */

void CqTransform::ResetTransform(const CqMatrix& mat, TqBool hand, TqBool makeStatic)
{
	if( makeStatic )
	{
		Reset();
		m_IsMoving=TqFalse;
		m_StaticMatrix = mat;
		m_Handedness = hand;
	}
	else
	{
		TqInt i;
		for(i=0; i<cTimes(); i++)
			SetCurrentTransform(Time(i), mat);
	}
}



//---------------------------------------------------------------------
// Overrides from CqMotionSpec reequired for any object subject to motion specification.

void CqTransform::ClearMotionObject( SqTransformation& A ) const
    {}

SqTransformation CqTransform::ConcatMotionObjects( const SqTransformation& A, const SqTransformation& B ) const
{
    SqTransformation res;
	res.m_matTransform = A.m_matTransform * B.m_matTransform;
	TqBool flip = ( B.m_matTransform.Determinant() < 0 );
	res.m_Handedness = (flip)? !A.m_Handedness : A.m_Handedness;
	return ( res );
}

SqTransformation CqTransform::LinearInterpolateMotionObjects( TqFloat Fraction, const SqTransformation& A, const SqTransformation& B ) const
{
    // TODO: Should we do anything with this???
    return ( A );
}


//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )
