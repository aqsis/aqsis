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
		\brief Declares The CqTransform class containing information about the coordinates system for GPrims.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef TRANSFORM_H_INCLUDED
#define TRANSFORM_H_INCLUDED 1

#include	<vector>
#include	<boost/utility.hpp>
#include	<boost/shared_ptr.hpp>

#include	<aqsis/aqsis.h>

#include	"motion.h"
#include	<aqsis/ri/ri.h>
#include	<aqsis/math/matrix.h>
#include	"options.h"
#include	<aqsis/core/itransform.h>

namespace Aqsis {

struct SqTransformation
{
	CqMatrix m_matTransform;
	bool	m_Handedness;
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
		class Set
			{}
		;
		class ConcatCurrent
			{}
		;
		class SetCurrent
			{}
		;

		CqTransform();
		CqTransform( const CqTransform& From );
		CqTransform( const CqTransformPtr& From );
		CqTransform( const CqTransformPtr& From, TqFloat time,
		             const CqMatrix& matTrans, const Set& set
			           );
		CqTransform( const CqTransformPtr& From, TqFloat time,
		             const CqMatrix& matTrans, const ConcatCurrent& concatCurrent );
		CqTransform( const CqTransformPtr& From, TqFloat time,
		             const CqMatrix& matTrans, const SetCurrent& setCurrent );
		virtual	~CqTransform();

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqTransform");
		}
#endif

		// virtual	CqTransform& operator=( const CqTransform& From );

		void	SetCurrentTransform( TqFloat time, const CqMatrix& matTrans );
		void	ResetTransform(const CqMatrix& mat, bool hand, bool makeStatic=true);

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

		virtual bool isMoving() const
		{
			return m_IsMoving;
		}

		virtual	bool GetHandedness(TqFloat time ) const;

		virtual	void	ClearMotionObject( SqTransformation& A ) const;
		virtual	SqTransformation	ConcatMotionObjects( const SqTransformation& A, const SqTransformation& B ) const;
		virtual	SqTransformation	LinearInterpolateMotionObjects( TqFloat Fraction, const SqTransformation& A, const SqTransformation& B ) const;

		CqTransform*	Inverse();

	private:
		void	InitialiseDefaultObject( const CqTransformPtr& From );
		void	SetTransform( TqFloat time, const CqMatrix& matTrans );
		void	ConcatCurrentTransform( TqFloat time, const CqMatrix& matTrans );
		void FlipHandedness(TqFloat /* time */ )
		{
			m_Handedness = !m_Handedness;
		}


		bool	m_IsMoving;			///< Flag indicating this transformation describes a changing transform.
		CqMatrix	m_StaticMatrix;	///< Matrix storing the transformation should there be no motion involved.
		bool	m_Handedness;	///< Current coordinate system orientation.
}
;


/** \brief Collect the key times from two transformations
 *
 * The key frame times are collected from trans1 and trans2, and merged
 * together into a sorted vector of times.  Duplicate key times are eliminated.
 *
 * \param times - output vector of times.
 * \param trans1,trans2 - transformations from which to extract the times.
 */
void mergeKeyTimes(std::vector<TqFloat>& times, const CqTransform& trans1,
				   const CqTransform& trans2);


//-----------------------------------------------------------------------

} // namespace Aqsis


#endif // TRANSFORM_H_INCLUDED
