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



//---------------------------------------------------------------------
/** Constructor.
 */

CqTransform::CqTransform() : CqMotionSpec<CqMatrix>( CqMatrix() ), m_cReferences( 0 ), m_StackIndex( 0 ), m_IsMoving(TqFalse)
{
    if ( QGetRenderContext() != 0 )
    {
        QGetRenderContext() ->TransformStack().push_back( this );
        m_StackIndex = QGetRenderContext() ->TransformStack().size() - 1;
    }
    // Get the state of the transformation at the last stack entry, and use this as the default value for new timeslots.
    // if the previous level has motion specification, the value will be interpolated.
    CqMatrix matOtoWLast;
    if ( m_StackIndex > 0 )
        matOtoWLast = QGetRenderContext() ->TransformStack() [ m_StackIndex - 1 ] ->matObjectToWorld();
    SetDefaultObject( matOtoWLast );
}


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqTransform::CqTransform( const CqTransform& From ) : CqMotionSpec<CqMatrix>( From ), m_cReferences( 0 ), m_StackIndex( -1 ), m_IsMoving(TqFalse)
{
    *this = From;

    // Register ourself with the global transform stack.
    QGetRenderContext() ->TransformStack().push_back( this );
    m_StackIndex = QGetRenderContext() ->TransformStack().size() - 1;

    // Get the state of the transformation at the last stack entry, and use this as the default value for new timeslots.
    // if the previous level has motion specification, the value will be interpolated.
    CqMatrix matOtoWLast;
    if ( m_StackIndex > 0 )
        matOtoWLast = QGetRenderContext() ->TransformStack() [ m_StackIndex - 1 ] ->matObjectToWorld();
    SetDefaultObject( matOtoWLast );
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqTransform::~CqTransform()
{
    assert( RefCount() == 0 );

    if ( m_StackIndex >= 0 && m_StackIndex < static_cast<TqInt>( QGetRenderContext() ->TransformStack().size() ) )
    {
        // Remove ourself from the stack
        std::vector<CqTransform*>::iterator p = QGetRenderContext() ->TransformStack().begin();
        p += m_StackIndex;
        std::vector<CqTransform*>::iterator p2 = p;
        while ( p2 != QGetRenderContext() ->TransformStack().end() )
        {
            ( *p2 ) ->m_StackIndex--;
            p2++;
        }
        QGetRenderContext() ->TransformStack().erase( p );
    }
}

//---------------------------------------------------------------------
/** Copy function.
 */

CqTransform& CqTransform::operator=( const CqTransform& From )
{
    CqMotionSpec<CqMatrix>::operator=( From );

	m_IsMoving = From.m_IsMoving;
	m_StaticMatrix = From.m_StaticMatrix;

    return ( *this );
}


//---------------------------------------------------------------------
/** Set the transformation at the specified time.
 */

void CqTransform::SetCurrentTransform( TqFloat time, const CqMatrix& matTrans )
{
    if ( QGetRenderContext() ->pconCurrent() ->fMotionBlock() )
	{
		AddTimeSlot( time, matTrans );
		m_IsMoving = TqTrue;
	}
	else
	{
		if( m_IsMoving )
			AddTimeSlot( time, matTrans );
		else
			m_StaticMatrix = matTrans;
	}
}



void CqTransform::SetTransform( TqFloat time, const CqMatrix& matTrans )
{
    if ( QGetRenderContext() ->pconCurrent() ->fMotionBlock() )
	{
		AddTimeSlot( time, matTrans );
		m_IsMoving = TqTrue;
	}
	else
	{
		// If not in a motion block, but we are moving, apply the transform to all keys.
		if( m_IsMoving )
		{
			CqMatrix mat0 = matObjectToWorld(Time(0));
			//mat0 = mat0.Inverse();
			AddTimeSlot( Time(0), matTrans );
			TqInt i;
			for(i=1; i<cTimes(); i++)
			{
				CqMatrix matOffset = mat0 * matObjectToWorld(Time(i)).Inverse();
				AddTimeSlot( Time(i), matOffset * matTrans);
			}
		}
		else
			m_StaticMatrix = matTrans;
	}
}


//---------------------------------------------------------------------
/** Modify the transformation at the specified time.
 */

void CqTransform::ConcatCurrentTransform( TqFloat time, const CqMatrix& matTrans )
{
    // If we are actually in a motion block, and we already describe a moving transform,
	// concatenate this transform with the existing one at that time slot,
    if ( QGetRenderContext() ->pconCurrent() ->fMotionBlock() )
	{
		if( m_IsMoving )
			ConcatTimeSlot( time, matTrans );
		else
		{
			AddTimeSlot( time, m_StaticMatrix );
			ConcatTimeSlot( time, matTrans );
			m_IsMoving = TqTrue;
		}
	}
    else
    // else, if we are moving, apply this transform at all time slots, otherwise apply to static matrix.
	{
		if( m_IsMoving )
			ConcatAllTimeSlots( matTrans );
		else
			m_StaticMatrix = m_StaticMatrix * matTrans;
	}
}


//---------------------------------------------------------------------
/** Get the transformation at the specified time.
 */

const CqMatrix& CqTransform::matObjectToWorld( TqFloat time ) const
{
	if( m_IsMoving )
		return ( GetMotionObject( time ) );
	else
		return ( m_StaticMatrix );
}


//---------------------------------------------------------------------
/** Reset the transform to the specified matrix, if the flag makeStatic is true
 *  then make the transform static and clear the motion keyframes.
 */

void CqTransform::ResetTransform(const CqMatrix& mat, TqBool makeStatic)
{
	if( makeStatic )
	{
		Reset();
		m_IsMoving=TqFalse;
		m_StaticMatrix = mat;
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

void CqTransform::ClearMotionObject( CqMatrix& A ) const
    {}

CqMatrix CqTransform::ConcatMotionObjects( const CqMatrix& A, const CqMatrix& B ) const
{
    return ( A * B );
}

CqMatrix CqTransform::LinearInterpolateMotionObjects( TqFloat Fraction, const CqMatrix& A, const CqMatrix& B ) const
{
    // TODO: Should we do anything with this???
    return ( A );
}


//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )
