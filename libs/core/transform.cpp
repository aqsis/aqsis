// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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
		\brief Implements the CqTransform class for handling GPrim coordinate systems.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<aqsis/aqsis.h>
#include	"transform.h"
#include	"renderer.h"

namespace Aqsis {

//---------------------------------------------------------------------
/** Constructor.
 */

CqTransform::CqTransform() : CqMotionSpec<SqTransformation>(SqTransformation()), m_IsMoving(false), m_Handedness(false)
{}


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqTransform::CqTransform( const CqTransform& From ) : CqMotionSpec<SqTransformation>( From )
{
	m_IsMoving = From.m_IsMoving;
	m_StaticMatrix = From.m_StaticMatrix;
	m_Handedness = From.m_Handedness;
	/*    *this = From;
	 
		// Get the state of the transformation at the last stack entry, and use this as the default value for new timeslots.
		// if the previous level has motion specification, the value will be interpolated.
		CqMatrix matOtoWLast;
		bool handLast = false;
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
	*/
}


//---------------------------------------------------------------------
/** Initialise the default object.
 */
void	CqTransform::InitialiseDefaultObject( const CqTransformPtr& From )
{
	// Get the state of the transformation at the last stack entry, and use this as the default value for new timeslots.
	// if the previous level has motion specification, the value will be interpolated.
	TqFloat time = QGetRenderContext()->Time();
	CqMatrix matOtoWLast =  From->matObjectToWorld( time );
	bool handLast =  From->GetHandedness( time );

	SqTransformation ct;
	ct.m_Handedness = handLast;
	ct.m_matTransform = matOtoWLast;
	SetDefaultObject( ct );
}


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqTransform::CqTransform( const CqTransformPtr& From )
		: CqMotionSpec<SqTransformation>( *From ),
		m_IsMoving(false),
		m_StaticMatrix( From->m_StaticMatrix ),
		m_Handedness( From->m_Handedness )
{
	InitialiseDefaultObject( From );
}


//---------------------------------------------------------------------
/** Concatenation constructors.
 */

CqTransform::CqTransform( const CqTransformPtr& From, TqFloat time,
                          const CqMatrix& matTrans, const Set& /* set */ )
		: CqMotionSpec<SqTransformation>( *From ),
		m_IsMoving( From->m_IsMoving ),
		m_StaticMatrix( From->m_StaticMatrix ),
		m_Handedness( From->m_Handedness )
{
	SetTransform( time, matTrans );
}


CqTransform::CqTransform( const CqTransformPtr& From, TqFloat time,
                          const CqMatrix& matTrans, const ConcatCurrent& /* concatCurrent */ )
		: CqMotionSpec<SqTransformation>( *From ),
		m_IsMoving( From->m_IsMoving ),
		m_StaticMatrix( From->m_StaticMatrix ),
		m_Handedness( From->m_Handedness )
{
	ConcatCurrentTransform( time, matTrans );
}


CqTransform::CqTransform( const CqTransformPtr& From, TqFloat time,
                          const CqMatrix& matTrans, const SetCurrent& /* setCurrent */ )
		: CqMotionSpec<SqTransformation>( *From ),
		m_IsMoving( From->m_IsMoving ),
		m_StaticMatrix( From->m_StaticMatrix ),
		m_Handedness( From->m_Handedness )
{
	SetCurrentTransform( time, matTrans );
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqTransform::~CqTransform()
{}

//---------------------------------------------------------------------
/** Copy function.
 */
#if 0
CqTransform& CqTransform::operator=( const CqTransform& From )
{
	CqMotionSpec<SqTransformation>::operator=( From );

	m_IsMoving = From.m_IsMoving;
	m_StaticMatrix = From.m_StaticMatrix;
	m_Handedness = From.m_Handedness;

	return ( *this );
}
#endif

//---------------------------------------------------------------------
/** Set the transformation at the specified time.
 */

void CqTransform::SetCurrentTransform( TqFloat time, const CqMatrix& matTrans )
{
	TqFloat det = matTrans.Determinant();
	bool flip = ( !matTrans.fIdentity() && det < 0 );

	SqTransformation ct;
	ct.m_matTransform = matTrans;
	ct.m_Handedness = !flip;

	if ( QGetRenderContext() ->pconCurrent() ->fMotionBlock() )
	{
		AddTimeSlot( time, ct );
		m_IsMoving = true;
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
			m_Handedness = (flip)? !m_Handedness : m_Handedness;
			//m_Handedness = flip;
			ct.m_Handedness = flip;
			SetDefaultObject( ct );
		}
	}
}



void CqTransform::SetTransform( TqFloat time, const CqMatrix& matTrans )
{
	TqFloat det = matTrans.Determinant();
	bool flip = ( !matTrans.fIdentity() && det < 0 );
	CqMatrix matCtoW;
	QGetRenderContext()->matSpaceToSpace("world", "camera", NULL, NULL, QGetRenderContext()->Time(), matCtoW);
	TqFloat camdet = matCtoW.Determinant();
	bool camhand = ( !matCtoW.fIdentity() && camdet < 0 );

	if ( QGetRenderContext() ->pconCurrent() ->fMotionBlock() )
	{
		SqTransformation ct;
		ct.m_Handedness = (flip)? !camhand : camhand;
		ct.m_matTransform = matTrans;
		AddTimeSlot( time, ct );
		m_IsMoving = true;
	}
	else
	{
		// If not in a motion block, but we are moving, apply the transform to all keys.
		if( m_IsMoving )
		{
			CqMatrix mat0 = matObjectToWorld(Time(0));

			SqTransformation ct;
			ct.m_Handedness = (flip)? !camhand : camhand;
			bool hand0 = ct.m_Handedness;
			ct.m_matTransform = matTrans;

			AddTimeSlot( Time(0), ct );
			TqInt i;
			for(i=1; i<cTimes(); i++)
			{
				CqMatrix matOffset = mat0 * matObjectToWorld(Time(i)).Inverse();
				ct.m_matTransform = matOffset * matTrans;
				bool flip2 = ( matOffset.Determinant() < 0 );
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
	bool flip = ( !matTrans.fIdentity() && det < 0 );

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
		m_IsMoving = true;
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
	static CqMatrix matInt;
	if( m_IsMoving )
	{
		matInt = GetMotionObjectInterpolated( time ).m_matTransform;
		return ( matInt );
	}
	else
		return ( m_StaticMatrix );
}


//---------------------------------------------------------------------
/** Invert the transformation
 */

CqTransform* CqTransform::Inverse( )
{
	CqTransform* result = new CqTransform();
	if( m_IsMoving )
	{
	}
	else
		result->m_StaticMatrix = m_StaticMatrix.Inverse();
	return(result);
}


//---------------------------------------------------------------------
/** Get the handedness at the specified time.
 */

bool CqTransform::GetHandedness( TqFloat time ) const
{
	if( m_IsMoving )
		return ( GetMotionObjectInterpolated( time ).m_Handedness );
	else
		return ( m_Handedness );
}


//---------------------------------------------------------------------
/** Reset the transform to the specified matrix, if the flag makeStatic is true
 *  then make the transform static and clear the motion keyframes.
 */

void CqTransform::ResetTransform(const CqMatrix& mat, bool hand, bool makeStatic)
{
	if( makeStatic )
	{
		Reset();
		m_IsMoving=false;
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
	bool flip = ( B.m_matTransform.Determinant() < 0 );
	res.m_Handedness = (flip)? !A.m_Handedness : A.m_Handedness;
	return ( res );
}

SqTransformation CqTransform::LinearInterpolateMotionObjects( TqFloat Fraction, const SqTransformation& A, const SqTransformation& B ) const
{
	// TODO: Should we do anything with this???
	return ( A );
}


void mergeKeyTimes(std::vector<TqFloat>& times, const CqTransform& trans1,
				   const CqTransform& trans2)
{
	TqInt n1 = trans1.cTimes();
	TqInt n2 = trans2.cTimes();
	times.clear();
	times.reserve(n1 + n2);
	// Copy out all the times from the two transforms.
	for(TqInt i = 0; i < n1; ++i)
		times.push_back(trans1.Time(i));
	for(TqInt i = 0; i < n2; ++i)
		times.push_back(trans2.Time(i));
	// Sort the times
	std::sort(times.begin(), times.end());
	// uniqueify elements and remove non-unique junk from end.
	std::vector<TqFloat>::iterator endJunk = std::unique(times.begin(), times.end());
	times.erase(endJunk, times.end());
}

//---------------------------------------------------------------------

} // namespace Aqsis
