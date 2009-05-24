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
 *
 * \brief Data storage policies for small vectors
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef VECTORSTORAGE_H_INCLUDED
#define VECTORSTORAGE_H_INCLUDED

#include <aqsis/aqsis.h>

namespace Aqsis
{

/** \brief Container for 3d vector data
 *
 * This class holds the three components for a 3D vector by value.  For use as
 * a storage policy for 3D vector types.
 */
class CqVec3Data
{
	private:
		TqFloat m_v[3];
	public:
		CqVec3Data(const TqFloat* v)
		{
			m_v[0] = v[0];
			m_v[1] = v[1];
			m_v[2] = v[2];
		}
		CqVec3Data(TqFloat x, TqFloat y, TqFloat z)
		{
			m_v[0] = x;
			m_v[1] = y;
			m_v[2] = z;
		}
		/// Indexed component access
		TqFloat operator[](TqInt i) const
		{
			return m_v[i];
		}
		TqFloat& operator[](TqInt i)
		{
			return m_v[i];
		}
};

/** \brief Reference to vector data.
 *
 * This class holds a pointer to data representing the components of a vector.
 * For use as a storage policy for CqVec3 and friends.
 */
class CqVecRefData
{
	private:
		TqFloat* m_v;
	public:
		CqVecRefData(TqFloat* v)
			: m_v(v)
		{ }
		/// Indexed component access
		TqFloat operator[](TqInt i) const
		{
			return m_v[i];
		}
		TqFloat& operator[](TqInt i)
		{
			return m_v[i];
		}
};

} // namespace Aqsis

#endif // VECTORSTORAGE_H_INCLUDED
