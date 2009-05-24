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
		\brief Interface classes for variable definitions
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef IVARDEF_H_INCLUDED
#define IVARDEF_H_INCLUDED 1

#include	<aqsis/aqsis.h>

namespace Aqsis {

struct IqParseNode;

/** \todo <b>Code Review</b>: The types declared here are mostly a duplicate of
 * those declared in aqsistypes/primvartype.h and should be merged.  Note a
 * slight inconsistency wrt the hextuple type.  If this gets redone, should
 * also consider whether we really want the types and storage classes in a
 * single enum or in multiple enums (which would make life easier).
 */
enum EqVariableType
{
    Type_Nil = 0,

    Type_Float = 1,
    Type_Integer,
    Type_Point,
    Type_String,
    Type_Color,
    Type_Triple,
    Type_hPoint,
    Type_Normal,
    Type_Vector,
    Type_Void,
    Type_Matrix,
    Type_HexTuple,
    Type_Last,

    Type_Uniform = 0x8000,
    Type_Varying = 0x4000,
    Type_Vertex = 0x2000,

    Type_Output = 0x0800,
    Type_Variable = 0x0400,
    Type_Param = 0x0200,
    Type_Array = 0x0100,

    Type_VaryingFloat = Type_Float | Type_Varying,
    Type_VaryingInteger = Type_Integer | Type_Varying,
    Type_VaryingPoint = Type_Point | Type_Varying,
    Type_VaryingString = Type_String | Type_Varying,
    Type_VaryingColor = Type_Color | Type_Varying,
    Type_VaryinghPoint = Type_hPoint | Type_Varying,
    Type_VaryingNormal = Type_Normal | Type_Varying,
    Type_VaryingVector = Type_Vector | Type_Varying,
    Type_VaryingMatrix = Type_Matrix | Type_Varying,

    Type_UniformFloat = Type_Float | Type_Uniform,
    Type_UniformInteger = Type_Integer | Type_Uniform,
    Type_UniformPoint = Type_Point | Type_Uniform,
    Type_UniformString = Type_String | Type_Uniform,
    Type_UniformColor = Type_Color | Type_Uniform,
    Type_UniformhPoint = Type_hPoint | Type_Uniform,
    Type_UniformNormal = Type_Normal | Type_Uniform,
    Type_UniformVector = Type_Vector | Type_Uniform,
    Type_UniformMatrix = Type_Matrix | Type_Uniform,

    Type_VertexFloat = Type_Float | Type_Vertex,
    Type_VertexInteger = Type_Integer | Type_Vertex,
    Type_VertexPoint = Type_Point | Type_Vertex,
    Type_VertexString = Type_String | Type_Vertex,
    Type_VertexColor = Type_Color | Type_Vertex,
    Type_VertexhPoint = Type_hPoint | Type_Vertex,
    Type_VertexNormal = Type_hPoint | Type_Vertex,
    Type_VertexVector = Type_hPoint | Type_Vertex,
    Type_VertexMatrix = Type_Matrix | Type_Vertex,

    Type_UniformFloatVariable = Type_Float | Type_Variable | Type_Uniform,
    Type_UniformPointVariable = Type_Point | Type_Variable | Type_Uniform,
    Type_UniformStringVariable = Type_String | Type_Variable | Type_Uniform,
    Type_UniformColorVariable = Type_Color | Type_Variable | Type_Uniform,
    Type_UniformNormalVariable = Type_Normal | Type_Variable | Type_Uniform,
    Type_UniformVectorVariable = Type_Vector | Type_Variable | Type_Uniform,
    Type_UniformMatrixVariable = Type_Matrix | Type_Variable | Type_Uniform,

    Type_VaryingFloatVariable = Type_Float | Type_Variable | Type_Varying,
    Type_VaryingPointVariable = Type_Point | Type_Variable | Type_Varying,
    Type_VaryingStringVariable = Type_String | Type_Variable | Type_Varying,
    Type_VaryingColorVariable = Type_Color | Type_Variable | Type_Varying,
    Type_VaryingNormalVariable = Type_Normal | Type_Variable | Type_Varying,
    Type_VaryingVectorVariable = Type_Vector | Type_Variable | Type_Varying,
    Type_VaryingMatrixVariable = Type_Matrix | Type_Variable | Type_Varying,

    Type_VertexFloatVariable = Type_Float | Type_Variable | Type_Vertex,
    Type_VertexPointVariable = Type_Point | Type_Variable | Type_Vertex,
    Type_VertexStringVariable = Type_String | Type_Variable | Type_Vertex,
    Type_VertexColorVariable = Type_Color | Type_Variable | Type_Vertex,
    Type_VertexNormalVariable = Type_Normal | Type_Variable | Type_Vertex,
    Type_VertexVectorVariable = Type_Vector | Type_Variable | Type_Vertex,
    Type_VertexMatrixVariable = Type_Matrix | Type_Variable | Type_Vertex,

    Type_VaryingFloatArray = Type_Float | Type_Varying | Type_Array,
    Type_VaryingIntegerArray = Type_Integer | Type_Varying | Type_Array,
    Type_VaryingPointArray = Type_Point | Type_Varying | Type_Array,
    Type_VaryingStringArray = Type_String | Type_Varying | Type_Array,
    Type_VaryingColorArray = Type_Color | Type_Varying | Type_Array,
    Type_VaryinghPointArray = Type_hPoint | Type_Varying | Type_Array,
    Type_VaryingNormalArray = Type_Normal | Type_Varying | Type_Array,
    Type_VaryingVectorArray = Type_Vector | Type_Varying | Type_Array,
    Type_VaryingMatrixArray = Type_Matrix | Type_Varying | Type_Array,

    Type_UniformFloatArray = Type_Float | Type_Uniform | Type_Array,
    Type_UniformIntegerArray = Type_Integer | Type_Uniform | Type_Array,
    Type_UniformPointArray = Type_Point | Type_Uniform | Type_Array,
    Type_UniformStringArray = Type_String | Type_Uniform | Type_Array,
    Type_UniformColorArray = Type_Color | Type_Uniform | Type_Array,
    Type_UniformhPointArray = Type_hPoint | Type_Uniform | Type_Array,
    Type_UniformNormalArray = Type_Normal | Type_Uniform | Type_Array,
    Type_UniformVectorArray = Type_Vector | Type_Uniform | Type_Array,
    Type_UniformMatrixArray = Type_Matrix | Type_Uniform | Type_Array,

    Type_VertexFloatArray = Type_Float | Type_Vertex | Type_Array,
    Type_VertexIntegerArray = Type_Integer | Type_Vertex | Type_Array,
    Type_VertexPointArray = Type_Point | Type_Vertex | Type_Array,
    Type_VertexStringArray = Type_String | Type_Vertex | Type_Array,
    Type_VertexColorArray = Type_Color | Type_Vertex | Type_Array,
    Type_VertexhPointArray = Type_hPoint | Type_Vertex | Type_Array,
    Type_VertexNormalArray = Type_hPoint | Type_Vertex | Type_Array,
    Type_VertexVectorArray = Type_hPoint | Type_Vertex | Type_Array,
    Type_VertexMatrixArray = Type_Matrix | Type_Vertex | Type_Array,

    Type_Mask = 0x00FF,
    Storage_Mask = 0xF000,
    Usage_Mask = 0x0F00,

    Storage_Shift = 12,
    Usage_Shift = 8,
};

/// \todo <b>Code review</b> This is an almost-duplicate from elsewhere (see primvartype.h)
extern const char* const gVariableTypeNames[];

enum EqShaderType
{
    Type_Surface,   			///< Surface shader
    Type_Lightsource,   		///< Lightsource shader.
    Type_Volume,   			///< Volume shader.
    Type_Displacement,   		///< Displacement shader.
    Type_Transformation,   	///< Transformation shader.
    Type_Imager,   			///< Image shader.
};


enum EqEnvVars
{
    EnvVars_Cs,   		///< Surface color.
    EnvVars_Os,   		///< Surface opacity.
    EnvVars_Ng,   		///< Geometric normal.
    EnvVars_du,   		///< First derivative in u.
    EnvVars_dv,   		///< First derivative in v.
    EnvVars_L,   		///< Incoming light direction.
    EnvVars_Cl,   		///< Light color.
    EnvVars_Ol,   		///< Light opacity.
    EnvVars_P,   		///< Point being shaded.
    EnvVars_dPdu,   	///< Change in P with respect to change in u.
    EnvVars_dPdv,   	///< Change in P with respect to change in v.
    EnvVars_N,   		///< Surface normal.
    EnvVars_u,   		///< Surface u coordinate.
    EnvVars_v,   		///< Surface v coordinate.
    EnvVars_s,   		///< Texture s coordinate.
    EnvVars_t,   		///< Texture t coordinate.
    EnvVars_I,   		///< Incident ray direction.
    EnvVars_Ci,   		///< Incident color.
    EnvVars_Oi,   		///< Incident opacity.
    EnvVars_Ps,   		///< Point being lit.
    EnvVars_E,   		///< Viewpoint position.
    EnvVars_ncomps,   	///< Number of color components.
    EnvVars_time,   	///< Frame time.
    EnvVars_alpha,   	///< Fractional pixel coverage.

    EnvVars_Ns,   		///< Normal at point being lit.

    EnvVars_Last
};


///----------------------------------------------------------------------
/// EqVarType
/// Type of variable

enum EqVarType
{
    VarTypeStandard = 0,
    VarTypeLocal,
};


///----------------------------------------------------------------------
/// SqVarRef
/// Structure storing a variable reference.

struct SqVarRef
{
	EqVarType	m_Type;
	TqUint	m_Index;

	bool	operator==( const SqVarRef& From ) const
	{
		return ( From.m_Type == m_Type && From.m_Index == m_Index );
	}
};


struct IqVarDef
{
	virtual	const IqParseNode*	pInitialiser() const = 0;
	virtual	IqParseNode*	pInitialiser() = 0;
	virtual	TqInt	Type() const = 0;
	virtual const char*	strName() const = 0;
	virtual	void	IncUseCount() = 0;
	virtual	TqInt	UseCount() const = 0;
	virtual	TqInt	ArrayLength() const = 0;
	virtual	bool	fExtern() const = 0;
	virtual	SqVarRef	vrExtern() const = 0;
	virtual	void	SetParam( bool fParam = true ) = 0;
	virtual	void	SetOutput( bool fOutput = true ) = 0;
	virtual	void	SetDefaultStorage( TqInt Storage ) = 0;

	static	IqVarDef*	GetVariablePtr( const SqVarRef& Ref );

	virtual ~IqVarDef()
	{
	};
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !IVARDEF_H_INCLUDED
