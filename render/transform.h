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
		\brief Declares The CqTransform class containing information about the coordinates system for GPrims.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef TRANSFORM_H_INCLUDED
#define TRANSFORM_H_INCLUDED 1

#include	<vector>
#include	<boost/utility.hpp>
#include	<boost/shared_ptr.hpp>

#include	"aqsis.h"

#include	"motion.h"
#include	"ri.h"
#include	"matrix.h"
#include	"options.h"
#include	"itransform.h"

START_NAMESPACE( Aqsis )

struct SqTransformation
{
	CqMatrix m_matTransform;
	TqBool	m_Handedness;
};


class CqTransform;

typedef boost::shared_ptr<CqTransform> CqTransformPtr;

//----------------------------------------------------------------------
/** \class CqTransform
 * Container class for the transform definitions of the graphics state.
 */

class CqTransform : public CqMotionSpec<SqTransformation>, public IqTransform, private boost::noncopyable
{
public:
    class Set {};
    class ConcatCurrent {};
    class SetCurrent {};

    CqTransform();
    CqTransform( const CqTransform& From );
    CqTransform( const CqTransformPtr& From );
    CqTransform( const CqTransformPtr& From, TqFloat time,
	         const CqMatrix& matTrans, const Set& set );
    CqTransform( const CqTransformPtr& From, TqFloat time,
	         const CqMatrix& matTrans, const ConcatCurrent& concatCurrent );
    CqTransform( const CqTransformPtr& From, TqFloat time,
	         const CqMatrix& matTrans, const SetCurrent& setCurrent );
    virtual	~CqTransform();

#ifdef _DEBUG
    CqString className() const { return CqString("CqTransform"); }
#endif

    // virtual	CqTransform& operator=( const CqTransform& From );

    void	SetCurrentTransform( TqFloat time, const CqMatrix& matTrans );
    void	ResetTransform(const CqMatrix& mat, TqBool hand, TqBool makeStatic=TqTrue);

    virtual	const CqMatrix&	matObjectToWorld( TqFloat time ) const;

    virtual	TqFloat	Time( TqInt index ) const
    {
        return( CqMotionSpec<SqTransformation>::Time( index ) );
    }
    virtual	TqInt	cTimes() const
    {
		if(m_IsMoving)
			return( CqMotionSpec<SqTransformation>::cTimes() );
		else
			return( 1 );
    }

	virtual	TqBool GetHandedness(TqFloat time ) const;

    virtual	void	ClearMotionObject( SqTransformation& A ) const;
    virtual	SqTransformation	ConcatMotionObjects( const SqTransformation& A, const SqTransformation& B ) const;
    virtual	SqTransformation	LinearInterpolateMotionObjects( TqFloat Fraction, const SqTransformation& A, const SqTransformation& B ) const;

private:
    void	InitialiseDefaultObject( const CqTransformPtr& From );
    void	SetTransform( TqFloat time, const CqMatrix& matTrans );
    void	ConcatCurrentTransform( TqFloat time, const CqMatrix& matTrans );
	void FlipHandedness(TqFloat time )
	{
		m_Handedness = !m_Handedness;
	}


	TqBool	m_IsMoving;			///< Flag indicating this transformation describes a changing transform.
	CqMatrix	m_StaticMatrix;	///< Matrix storing the transformation should there be no motion involved.
	TqBool	m_Handedness;	///< Current coordinate system orientation.
}
;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )


#endif // TRANSFORM_H_INCLUDED
