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
 * \brief Forward declarations for short-vector types.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef VECFWD_H_INCLUDED
#define VECFWD_H_INCLUDED

namespace Aqsis
{

// storage policies
class CqVec3Data;
class CqVecRefData;


// forward declarations of vector classes.
template<typename StoreT> class CqBasicVec3;

/// A 3D vector which owns its data storage
typedef CqBasicVec3<CqVec3Data> CqVec3;
/// A 3D vector which holds a reference to external data storage
typedef CqBasicVec3<CqVecRefData> CqRefVec3;

/// Typedef for backward compatibility
typedef CqVec3 CqVector3D;


// forward decls for colors
template<typename StoreT> class CqBasicColor;

/// A 3D color which owns its data storage
typedef CqBasicColor<CqVec3Data> CqColor;
/// A 3D color which holds a reference to external data storage
typedef CqBasicColor<CqVecRefData> CqRefColor;


} // namespace Aqsis

#endif // VECFWD_H_INCLUDED
