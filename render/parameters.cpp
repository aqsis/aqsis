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
		\brief Implements classes and support functionality for the shader virtual machine.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"
#include	"shaderstack.h"
#include	"parameters.h"
#include	"vector3d.h"
#include	"vector4d.h"
#include	"matrix.h"
#include	"sstring.h"
#include	"color.h"

START_NAMESPACE(Aqsis)


CqParameter* (*gVariableCreateFuncsUniform[])(const char* strName, TqInt Count)=
{
	0,
	CqParameterTypedUniform<TqFloat,Type_UniformFloat>::Create,
	CqParameterTypedUniform<TqInt,Type_UniformInteger>::Create,
	CqParameterTypedUniform<CqVector3D,Type_UniformPoint>::Create,
	CqParameterTypedUniform<CqString,Type_UniformString>::Create,
	CqParameterTypedUniform<CqColor,Type_UniformColor>::Create,
	0,
	CqParameterTypedUniform<CqVector4D,Type_UniformhPoint>::Create,
	CqParameterTypedUniform<CqVector3D,Type_UniformNormal>::Create,
	CqParameterTypedUniform<CqVector3D,Type_UniformVector>::Create,
	CqParameterTypedUniform<CqMatrix,Type_UniformMatrix>::Create,
};

CqParameter* (*gVariableCreateFuncsVarying[])(const char* strName, TqInt Count)=
{
	0,
	CqParameterTypedVarying<TqFloat,Type_VaryingFloat>::Create,
	CqParameterTypedVarying<TqInt,Type_VaryingInteger>::Create,
	CqParameterTypedVarying<CqVector3D,Type_VaryingPoint>::Create,
	CqParameterTypedVarying<CqString,Type_VaryingString>::Create,
	CqParameterTypedVarying<CqColor,Type_VaryingColor>::Create,
	0,
	CqParameterTypedVarying<CqVector4D,Type_VaryinghPoint>::Create,
	CqParameterTypedVarying<CqVector3D,Type_VaryingNormal>::Create,
	CqParameterTypedVarying<CqVector3D,Type_VaryingVector>::Create,
	CqParameterTypedVarying<CqMatrix,Type_VaryingMatrix>::Create,
};

CqParameter* (*gVariableCreateFuncsVertex[])(const char* strName, TqInt Count)=
{
	0,
	CqParameterTypedVertex<TqFloat,Type_VertexFloat>::Create,
	CqParameterTypedVertex<TqInt,Type_VertexInteger>::Create,
	CqParameterTypedVertex<CqVector3D,Type_VertexPoint>::Create,
	CqParameterTypedVertex<CqString,Type_VertexString>::Create,
	CqParameterTypedVertex<CqColor,Type_VertexColor>::Create,
	0,
	CqParameterTypedVertex<CqVector4D,Type_VertexhPoint>::Create,
	CqParameterTypedVertex<CqVector3D,Type_VertexNormal>::Create,
	CqParameterTypedVertex<CqVector3D,Type_VertexVector>::Create,
	CqParameterTypedVertex<CqMatrix,Type_VertexMatrix>::Create,
};


CqParameter* (*gVariableCreateFuncsUniformArray[])(const char* strName, TqInt Count)=
{
	0,
	CqParameterTypedUniformArray<TqFloat,Type_UniformFloatArray>::Create,
	CqParameterTypedUniformArray<TqInt,Type_UniformIntegerArray>::Create,
	CqParameterTypedUniformArray<CqVector3D,Type_UniformPointArray>::Create,
	CqParameterTypedUniformArray<CqString,Type_UniformStringArray>::Create,
	CqParameterTypedUniformArray<CqColor,Type_UniformColorArray>::Create,
	0,
	CqParameterTypedUniformArray<CqVector4D,Type_UniformhPointArray>::Create,
	CqParameterTypedUniformArray<CqVector3D,Type_UniformNormalArray>::Create,
	CqParameterTypedUniformArray<CqVector3D,Type_UniformVectorArray>::Create,
	CqParameterTypedUniformArray<CqMatrix,Type_UniformMatrixArray>::Create,
};

CqParameter* (*gVariableCreateFuncsVaryingArray[])(const char* strName, TqInt Count)=
{
	0,
	CqParameterTypedVaryingArray<TqFloat,Type_VaryingFloatArray>::Create,
	CqParameterTypedVaryingArray<TqInt,Type_VaryingIntegerArray>::Create,
	CqParameterTypedVaryingArray<CqVector3D,Type_VaryingPointArray>::Create,
	CqParameterTypedVaryingArray<CqString,Type_VaryingStringArray>::Create,
	CqParameterTypedVaryingArray<CqColor,Type_VaryingColorArray>::Create,
	0,
	CqParameterTypedVaryingArray<CqVector4D,Type_VaryinghPointArray>::Create,
	CqParameterTypedVaryingArray<CqVector3D,Type_VaryingNormalArray>::Create,
	CqParameterTypedVaryingArray<CqVector3D,Type_VaryingVectorArray>::Create,
	CqParameterTypedVaryingArray<CqMatrix,Type_VaryingMatrixArray>::Create,
};

CqParameter* (*gVariableCreateFuncsVertexArray[])(const char* strName, TqInt Count)=
{
	0,
	CqParameterTypedVertexArray<TqFloat,Type_VertexFloat>::Create,
	CqParameterTypedVertexArray<TqInt,Type_VertexInteger>::Create,
	CqParameterTypedVertexArray<CqVector3D,Type_VertexPoint>::Create,
	CqParameterTypedVertexArray<CqString,Type_VertexString>::Create,
	CqParameterTypedVertexArray<CqColor,Type_VertexColor>::Create,
	0,
	CqParameterTypedVertexArray<CqVector4D,Type_VertexhPointArray>::Create,
	CqParameterTypedVertexArray<CqVector3D,Type_VertexNormalArray>::Create,
	CqParameterTypedVertexArray<CqVector3D,Type_VertexVectorArray>::Create,
	CqParameterTypedVertexArray<CqMatrix,Type_VertexMatrixArray>::Create,
};


END_NAMESPACE(Aqsis)
//---------------------------------------------------------------------
