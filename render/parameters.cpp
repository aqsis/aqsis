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
#include	"parameters.h"
#include	"vector3d.h"
#include	"vector4d.h"
#include	"matrix.h"
#include	"sstring.h"
#include	"color.h"
#include	"renderer.h"

START_NAMESPACE( Aqsis )


/** Default constructor
 * \param strName Character pointer to parameter name.
 * \param Count Integer value count, for arrays.
 */
CqParameter::CqParameter( const char* strName, TqInt Count ) :
		m_strName( strName ),
		m_Count( Count )
{
	/// \note Had to remove this as paramters are now created as part of the Renderer construction, so the 
	///		  renderer context isn't ready yet.
//	QGetRenderContext() ->Stats().IncParametersAllocated();
}

/** Copy constructor
 */
CqParameter::CqParameter( const CqParameter& From ) :
		m_strName( From.m_strName ),
		m_Count( From.m_Count )
{
	/// \note Had to remove this as paramters are now created as part of the Renderer construction, so the 
	///		  renderer context isn't ready yet.
//	QGetRenderContext() ->Stats().IncParametersAllocated();
}

CqParameter::~CqParameter()
{
	/// \note Had to remove this as paramters are now created as part of the Renderer construction, so the 
	///		  renderer context isn't ready yet.
//	QGetRenderContext() ->Stats().IncParametersDeallocated();
}


CqParameter* ( *gVariableCreateFuncsUniform[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedUniform<TqFloat, type_float>::Create,
        CqParameterTypedUniform<TqInt, type_integer>::Create,
        CqParameterTypedUniform<CqVector3D, type_point>::Create,
        CqParameterTypedUniform<CqString, type_string>::Create,
        CqParameterTypedUniform<CqColor, type_color>::Create,
        0,
        CqParameterTypedUniform<CqVector4D, type_hpoint>::Create,
        CqParameterTypedUniform<CqVector3D, type_normal>::Create,
        CqParameterTypedUniform<CqVector3D, type_vector>::Create,
        CqParameterTypedUniform<CqMatrix, type_matrix>::Create,
    };

CqParameter* ( *gVariableCreateFuncsVarying[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedVarying<TqFloat, type_float>::Create,
        CqParameterTypedVarying<TqInt, type_integer>::Create,
        CqParameterTypedVarying<CqVector3D, type_point>::Create,
        CqParameterTypedVarying<CqString, type_string>::Create,
        CqParameterTypedVarying<CqColor, type_color>::Create,
        0,
        CqParameterTypedVarying<CqVector4D, type_hpoint>::Create,
        CqParameterTypedVarying<CqVector3D, type_normal>::Create,
        CqParameterTypedVarying<CqVector3D, type_vector>::Create,
        CqParameterTypedVarying<CqMatrix, type_matrix>::Create,
    };

CqParameter* ( *gVariableCreateFuncsVertex[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedVertex<TqFloat, type_float>::Create,
        CqParameterTypedVertex<TqInt, type_integer>::Create,
        CqParameterTypedVertex<CqVector3D, type_point>::Create,
        CqParameterTypedVertex<CqString, type_string>::Create,
        CqParameterTypedVertex<CqColor, type_color>::Create,
        0,
        CqParameterTypedVertex<CqVector4D, type_hpoint>::Create,
        CqParameterTypedVertex<CqVector3D, type_normal>::Create,
        CqParameterTypedVertex<CqVector3D, type_vector>::Create,
        CqParameterTypedVertex<CqMatrix, type_matrix>::Create,
    };


CqParameter* ( *gVariableCreateFuncsUniformArray[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedUniformArray<TqFloat, type_float>::Create,
        CqParameterTypedUniformArray<TqInt, type_integer>::Create,
        CqParameterTypedUniformArray<CqVector3D, type_point>::Create,
        CqParameterTypedUniformArray<CqString, type_string>::Create,
        CqParameterTypedUniformArray<CqColor, type_color>::Create,
        0,
        CqParameterTypedUniformArray<CqVector4D, type_hpoint>::Create,
        CqParameterTypedUniformArray<CqVector3D, type_normal>::Create,
        CqParameterTypedUniformArray<CqVector3D, type_vector>::Create,
        CqParameterTypedUniformArray<CqMatrix, type_matrix>::Create,
    };

CqParameter* ( *gVariableCreateFuncsVaryingArray[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedVaryingArray<TqFloat, type_float>::Create,
        CqParameterTypedVaryingArray<TqInt, type_integer>::Create,
        CqParameterTypedVaryingArray<CqVector3D, type_point>::Create,
        CqParameterTypedVaryingArray<CqString, type_string>::Create,
        CqParameterTypedVaryingArray<CqColor, type_color>::Create,
        0,
        CqParameterTypedVaryingArray<CqVector4D, type_hpoint>::Create,
        CqParameterTypedVaryingArray<CqVector3D, type_normal>::Create,
        CqParameterTypedVaryingArray<CqVector3D, type_vector>::Create,
        CqParameterTypedVaryingArray<CqMatrix, type_matrix>::Create,
    };

CqParameter* ( *gVariableCreateFuncsVertexArray[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedVertexArray<TqFloat, type_float>::Create,
        CqParameterTypedVertexArray<TqInt, type_integer>::Create,
        CqParameterTypedVertexArray<CqVector3D, type_point>::Create,
        CqParameterTypedVertexArray<CqString, type_string>::Create,
        CqParameterTypedVertexArray<CqColor, type_color>::Create,
        0,
        CqParameterTypedVertexArray<CqVector4D, type_hpoint>::Create,
        CqParameterTypedVertexArray<CqVector3D, type_normal>::Create,
        CqParameterTypedVertexArray<CqVector3D, type_vector>::Create,
        CqParameterTypedVertexArray<CqMatrix, type_matrix>::Create,
    };


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
