// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

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
