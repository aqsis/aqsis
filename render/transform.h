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

#include	"aqsis.h"

#include	"motion.h"
#include	"ri.h"
#include	"matrix.h"
#include	"options.h"
#include	"itransform.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE( Aqsis )


//----------------------------------------------------------------------
/** \class CqTransform
 * Container class for the transform definitions of the graphics state.
 */

class _qShareC	CqTransform : public CqMotionSpec<CqMatrix>, public CqRefCount, public IqTransform
{
	public:
		_qShareM	CqTransform();
		_qShareM	CqTransform( const CqTransform& From );
		_qShareM	virtual	~CqTransform();

		/** Get a writable copy of this, if the reference count is greater than 1
		 * create a new copy and retirn that.
		 */
		_qShareM CqTransform* Write()
		{
			// We are about to write to this attribute,so clone if references exist.
			if ( RefCount() > 1 )
			{
				CqTransform * pWrite = Clone();
				pWrite->AddRef();
				Release();
				return ( pWrite );
			}
			else
				return ( this );
		}

		_qShareM virtual	CqTransform& operator=( const CqTransform& From );

		/** Get a duplicate of this transform.
		 */
		_qShareM CqTransform*	Clone() const
		{
			return ( new CqTransform( *this ) );
		}

		_qShareM virtual	void	SetCurrentTransform( TqFloat time, const CqMatrix& matTrans );
		_qShareM virtual	void	ConcatCurrentTransform( TqFloat time, const CqMatrix& matTrans );

		_qShareM virtual	const CqMatrix&	matObjectToWorld( TqFloat time = 0.0f ) const;

		_qShareM virtual	TqFloat	Time( TqInt index ) const
		{
			return( CqMotionSpec<CqMatrix>::Time( index ) );
		}
		_qShareM virtual	TqInt	cTimes() const
		{
			return( CqMotionSpec<CqMatrix>::cTimes() );
		}

		virtual	void	Release()
		{
			CqRefCount::Release();
		}
		virtual	void	AddRef()
		{
			CqRefCount::AddRef();
		}

		_qShareM	virtual	void	ClearMotionObject( CqMatrix& A ) const;
		_qShareM	virtual	CqMatrix	ConcatMotionObjects( const CqMatrix& A, const CqMatrix& B ) const;
		_qShareM	virtual	CqMatrix	LinearInterpolateMotionObjects( TqFloat Fraction, const CqMatrix& A, const CqMatrix& B ) const;

	private:
		TqInt	m_cReferences;		///< Number of references to this transform.
		TqInt	m_StackIndex;		///< Index in the transform stack of this transform.
}
;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )


#endif // TRANSFORM_H_INCLUDED
