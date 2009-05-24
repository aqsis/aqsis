// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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
 * \brief Type identifiers for variables attached to geometric primitives.
 * \author Paul Gregory
 * \author Chris Foster
 */

#ifndef PRIMVARTYPE_H_INCLUDED
#define PRIMVARTYPE_H_INCLUDED

#include <aqsis/util/enum.h>

namespace Aqsis {

//----------------------------------------------------------------------
/** \brief Primitive variable interpolation/storage class types
 *
 * Note that this should be kept in sync with the ENUM_INFO block below.
 */
enum EqVariableClass
{
    class_invalid,
    class_constant,
    class_uniform,
    class_varying,
    class_vertex,
    class_facevarying,
    class_facevertex,
};

AQSIS_ENUM_INFO_BEGIN(EqVariableClass, class_invalid)
    "invalid",
    "constant",
    "uniform",
    "varying",
    "vertex",
    "facevarying",
    "facevertex"
AQSIS_ENUM_INFO_END


//----------------------------------------------------------------------
/** \brief Primitive and shader variable type identifiers.
 *
 * Note that this should be kept in sync with the ENUM_INFO block below.
 */
enum EqVariableType
{
    type_invalid,
    type_float,
    type_integer,
    type_point,
    type_string,
    type_color,
    type_triple,
    type_hpoint,
    type_normal,
    type_vector,
    type_void,
    type_matrix,
    type_sixteentuple,
    type_bool,
};

AQSIS_ENUM_INFO_BEGIN(EqVariableType, type_invalid)
    "invalid",
    "float",
    "integer",
    "point",
    "string",
    "color",
    "triple",
    "hpoint",
    "normal",
    "vector",
    "void",
    "matrix",
    "sixteentuple",
    "bool"
AQSIS_ENUM_INFO_END


}

#endif // PRIMVARTYPE_H_INCLUDED
