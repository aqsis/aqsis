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
#include	<list>

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


//----------------------------------------------------------------------
/** \class CqTransform
 * Container class for the transform definitions of the graphics state.
 */

class CqTransform : public CqMotionSpec<SqTransformation>, public CqRefCount, public IqTransform
{
public:
    CqTransform();
    CqTransform( const CqTransform& From );
    virtual	~CqTransform();

#ifdef _DEBUG
    CqString className() const { return CqString("CqTransform"); }
#endif

    /** Get a writable copy of this, if the reference count is greater than 1
     * create a new copy and retirn that.
     */
    CqTransform* Write()
    {
        // We are about to write to this attribute,so clone if references exist.
        if ( RefCount() > 1 )
        {
            CqTransform * pWrite = Clone();
            ADDREF( pWrite );
            RELEASEREF( this );
            return ( pWrite );
        }
        else
            return ( this );
    }

    virtual	CqTransform& operator=( const CqTransform& From );

    /** Get a duplicate of this transform.
     */
    CqTransform*	Clone() const
    {
        return ( new CqTransform( *this ) );
    }

    virtual	void	SetTransform( TqFloat time, const CqMatrix& matTrans );
    virtual	void	SetCurrentTransform( TqFloat time, const CqMatrix& matTrans );
    virtual	void	ConcatCurrentTransform( TqFloat time, const CqMatrix& matTrans );

    virtual	const CqMatrix&	matObjectToWorld( TqFloat time = 0.0f ) const;

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

	virtual void ResetTransform(const CqMatrix& mat, TqBool hand, TqBool makeStatic=TqTrue);
	virtual	TqBool GetHandedness(TqFloat time = 0.0f) const;
	virtual void FlipHandedness(TqFloat time = 0.0f)
	{
		m_Handedness = !m_Handedness;
	}

#ifndef _DEBUG
    virtual	void	Release()
    {
        CqRefCount::Release();
    }
    virtual	void	AddRef()
    {
        CqRefCount::AddRef();
    }
#else
    virtual void AddRef(const TqChar* file, TqInt line)
    {
        CqRefCount::AddRef(file, line);
    }
    virtual void Release(const TqChar* file, TqInt line)
    {
        CqRefCount::Release(file, line);
    }
#endif

    virtual	void	ClearMotionObject( SqTransformation& A ) const;
    virtual	SqTransformation	ConcatMotionObjects( const SqTransformation& A, const SqTransformation& B ) const;
    virtual	SqTransformation	LinearInterpolateMotionObjects( TqFloat Fraction, const SqTransformation& A, const SqTransformation& B ) const;

private:
    TqInt	m_cReferences;		///< Number of references to this transform.
	TqBool	m_IsMoving;			///< Flag indicating this transformation describes a changing transform.
	CqMatrix	m_StaticMatrix;	///< Matrix storing the transformation should there be no motion involved.
    std::list<CqTransform*>::iterator	m_StackIterator;		///< Iterator in the transform stack for this transform.
	TqBool	m_Handedness;	///< Current coordinate system orientation.
}
;

/// Global transform
extern std::list<CqTransform*>	Transform_stack;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )


#endif // TRANSFORM_H_INCLUDED
