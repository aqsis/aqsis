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
		\brief Implements classes and support functionality for the shader virtual machine.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<aqsis/aqsis.h>
#include	"parameters.h"
#include	<aqsis/math/vector3d.h>
#include	<aqsis/math/vector4d.h>
#include	<aqsis/math/matrix.h>
#include	<aqsis/util/sstring.h>
#include	<aqsis/math/color.h>
#include	"renderer.h"

namespace Aqsis {


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

	assert( Count >= 1 );

	STATS_INC( PRM_created );
	STATS_INC( PRM_current );
	TqInt cPRM = STATS_GETI( PRM_current );
	TqInt cPeak = STATS_GETI( PRM_peak );

	STATS_SETI( PRM_peak, cPRM > cPeak ? cPRM : cPeak );
	m_hash = CqString::hash(strName);
}

/** Copy constructor
 */
CqParameter::CqParameter( const CqParameter& From ) :
		m_strName( From.m_strName ),
		m_Count( From.m_Count ),
		m_hash(From.m_hash)
{
	/// \note Had to remove this as paramters are now created as part of the Renderer construction, so the
	///		  renderer context isn't ready yet.
	//	QGetRenderContext() ->Stats().IncParametersAllocated();
	STATS_INC( PRM_created );
	STATS_INC( PRM_current );
	TqInt cPRM = STATS_GETI( PRM_current );
	TqInt cPeak = STATS_GETI( PRM_peak );

	STATS_SETI( PRM_peak, cPRM > cPeak ? cPRM : cPeak );
}

CqParameter::~CqParameter()
{
	/// \note Had to remove this as paramters are now created as part of the Renderer construction, so the
	///		  renderer context isn't ready yet.
	//	QGetRenderContext() ->Stats().IncParametersDeallocated();
	STATS_DEC( PRM_current );
}

CqParameter* CqParameter::Create(const CqPrimvarToken& tok)
{
	CqParameter* ( *createFunc ) ( const char* strName, TqInt Count ) = 0;
	if(tok.count() <= 1)
	{
		switch(tok.Class())
		{
			case class_constant:
				createFunc = gVariableCreateFuncsConstant[tok.type()];
				break;
			case class_uniform:
				createFunc = gVariableCreateFuncsUniform[tok.type()];
				break;
			case class_varying:
				createFunc = gVariableCreateFuncsVarying[tok.type()];
				break;
			case class_vertex:
				createFunc = gVariableCreateFuncsVertex[tok.type()];
				break;
			case class_facevarying:
				createFunc = gVariableCreateFuncsFaceVarying[tok.type()];
				break;
			case class_facevertex:
				createFunc = gVariableCreateFuncsFaceVertex[tok.type()];
				break;
			default:
				break;
		}
	}
	else
	{
		switch(tok.Class())
		{
			case class_constant:
				createFunc = gVariableCreateFuncsConstantArray[tok.type()];
				break;
			case class_uniform:
				createFunc = gVariableCreateFuncsUniformArray[tok.type()];
				break;
			case class_varying:
				createFunc = gVariableCreateFuncsVaryingArray[tok.type()];
				break;
			case class_vertex:
				createFunc = gVariableCreateFuncsVertexArray[tok.type()];
				break;
			case class_facevarying:
				createFunc = gVariableCreateFuncsFaceVaryingArray[tok.type()];
				break;
			case class_facevertex:
				createFunc = gVariableCreateFuncsFaceVertexArray[tok.type()];
				break;
			default:
				break;
		}
	}
	if(!createFunc)
	{
		AQSIS_THROW_XQERROR(XqInternal, EqE_Bug,
			"Could not create CqParameter for token \"" << tok << "\"");
	}
	return createFunc(tok.name().c_str(), tok.count());
}


CqParameter* ( *gVariableCreateFuncsConstant[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedConstant<TqFloat, type_float, TqFloat>::Create,
        CqParameterTypedConstant<TqInt, type_integer, TqFloat>::Create,
        CqParameterTypedConstant<CqVector3D, type_point, CqVector3D>::Create,
        CqParameterTypedConstant<CqString, type_string, CqString>::Create,
        CqParameterTypedConstant<CqColor, type_color, CqColor>::Create,
        0,
        CqParameterTypedConstant<CqVector4D, type_hpoint, CqVector3D>::Create,
        CqParameterTypedConstant<CqVector3D, type_normal, CqVector3D>::Create,
        CqParameterTypedConstant<CqVector3D, type_vector, CqVector3D>::Create,
        0,
        CqParameterTypedConstant<CqMatrix, type_matrix, CqMatrix>::Create,
        0,
        0,
    };


CqParameter* ( *gVariableCreateFuncsUniform[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedUniform<TqFloat, type_float, TqFloat>::Create,
        CqParameterTypedUniform<TqInt, type_integer, TqFloat>::Create,
        CqParameterTypedUniform<CqVector3D, type_point, CqVector3D>::Create,
        CqParameterTypedUniform<CqString, type_string, CqString>::Create,
        CqParameterTypedUniform<CqColor, type_color, CqColor>::Create,
        0,
        CqParameterTypedUniform<CqVector4D, type_hpoint, CqVector3D>::Create,
        CqParameterTypedUniform<CqVector3D, type_normal, CqVector3D>::Create,
        CqParameterTypedUniform<CqVector3D, type_vector, CqVector3D>::Create,
        0,
        CqParameterTypedUniform<CqMatrix, type_matrix, CqMatrix>::Create,
        0,
        0,
    };

CqParameter* ( *gVariableCreateFuncsVarying[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedVarying<TqFloat, type_float, TqFloat>::Create,
        CqParameterTypedVarying<TqInt, type_integer, TqFloat>::Create,
        CqParameterTypedVarying<CqVector3D, type_point, CqVector3D>::Create,
        CqParameterTypedVarying<CqString, type_string, CqString>::Create,
        CqParameterTypedVarying<CqColor, type_color, CqColor>::Create,
        0,
        CqParameterTypedVarying<CqVector4D, type_hpoint, CqVector3D>::Create,
        CqParameterTypedVarying<CqVector3D, type_normal, CqVector3D>::Create,
        CqParameterTypedVarying<CqVector3D, type_vector, CqVector3D>::Create,
        0,
        CqParameterTypedVarying<CqMatrix, type_matrix, CqMatrix>::Create,
        0,
        0,
    };

CqParameter* ( *gVariableCreateFuncsVertex[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedVertex<TqFloat, type_float, TqFloat>::Create,
        CqParameterTypedVertex<TqInt, type_integer, TqFloat>::Create,
        CqParameterTypedVertex<CqVector3D, type_point, CqVector3D>::Create,
        CqParameterTypedVertex<CqString, type_string, CqString>::Create,
        CqParameterTypedVertex<CqColor, type_color, CqColor>::Create,
        0,
        CqParameterTypedVertex<CqVector4D, type_hpoint, CqVector3D>::Create,
        CqParameterTypedVertex<CqVector3D, type_normal, CqVector3D>::Create,
        CqParameterTypedVertex<CqVector3D, type_vector, CqVector3D>::Create,
        0,
        CqParameterTypedVertex<CqMatrix, type_matrix, CqMatrix>::Create,
        0,
        0,
    };


CqParameter* ( *gVariableCreateFuncsFaceVarying[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedFaceVarying<TqFloat, type_float, TqFloat>::Create,
        CqParameterTypedFaceVarying<TqInt, type_integer, TqFloat>::Create,
        CqParameterTypedFaceVarying<CqVector3D, type_point, CqVector3D>::Create,
        CqParameterTypedFaceVarying<CqString, type_string, CqString>::Create,
        CqParameterTypedFaceVarying<CqColor, type_color, CqColor>::Create,
        0,
        CqParameterTypedFaceVarying<CqVector4D, type_hpoint, CqVector3D>::Create,
        CqParameterTypedFaceVarying<CqVector3D, type_normal, CqVector3D>::Create,
        CqParameterTypedFaceVarying<CqVector3D, type_vector, CqVector3D>::Create,
        0,
        CqParameterTypedFaceVarying<CqMatrix, type_matrix, CqMatrix>::Create,
        0,
        0,
    };


CqParameter* ( *gVariableCreateFuncsFaceVertex[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedFaceVertex<TqFloat, type_float, TqFloat>::Create,
        CqParameterTypedFaceVertex<TqInt, type_integer, TqFloat>::Create,
        CqParameterTypedFaceVertex<CqVector3D, type_point, CqVector3D>::Create,
        CqParameterTypedFaceVertex<CqString, type_string, CqString>::Create,
        CqParameterTypedFaceVertex<CqColor, type_color, CqColor>::Create,
        0,
        CqParameterTypedFaceVertex<CqVector4D, type_hpoint, CqVector3D>::Create,
        CqParameterTypedFaceVertex<CqVector3D, type_normal, CqVector3D>::Create,
        CqParameterTypedFaceVertex<CqVector3D, type_vector, CqVector3D>::Create,
        0,
        CqParameterTypedFaceVertex<CqMatrix, type_matrix, CqMatrix>::Create,
        0,
        0,
    };

CqParameter* ( *gVariableCreateFuncsConstantArray[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedConstantArray<TqFloat, type_float, TqFloat>::Create,
        CqParameterTypedConstantArray<TqInt, type_integer, TqFloat>::Create,
        CqParameterTypedConstantArray<CqVector3D, type_point, CqVector3D>::Create,
        CqParameterTypedConstantArray<CqString, type_string, CqString>::Create,
        CqParameterTypedConstantArray<CqColor, type_color, CqColor>::Create,
        0,
        CqParameterTypedConstantArray<CqVector4D, type_hpoint, CqVector3D>::Create,
        CqParameterTypedConstantArray<CqVector3D, type_normal, CqVector3D>::Create,
        CqParameterTypedConstantArray<CqVector3D, type_vector, CqVector3D>::Create,
        0,
        CqParameterTypedConstantArray<CqMatrix, type_matrix, CqMatrix>::Create,
        0,
        0,
    };


CqParameter* ( *gVariableCreateFuncsUniformArray[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedUniformArray<TqFloat, type_float, TqFloat>::Create,
        CqParameterTypedUniformArray<TqInt, type_integer, TqFloat>::Create,
        CqParameterTypedUniformArray<CqVector3D, type_point, CqVector3D>::Create,
        CqParameterTypedUniformArray<CqString, type_string, CqString>::Create,
        CqParameterTypedUniformArray<CqColor, type_color, CqColor>::Create,
        0,
        CqParameterTypedUniformArray<CqVector4D, type_hpoint, CqVector3D>::Create,
        CqParameterTypedUniformArray<CqVector3D, type_normal, CqVector3D>::Create,
        CqParameterTypedUniformArray<CqVector3D, type_vector, CqVector3D>::Create,
        0,
        CqParameterTypedUniformArray<CqMatrix, type_matrix, CqMatrix>::Create,
        0,
        0,
    };

CqParameter* ( *gVariableCreateFuncsVaryingArray[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedVaryingArray<TqFloat, type_float, TqFloat>::Create,
        CqParameterTypedVaryingArray<TqInt, type_integer, TqFloat>::Create,
        CqParameterTypedVaryingArray<CqVector3D, type_point, CqVector3D>::Create,
        CqParameterTypedVaryingArray<CqString, type_string, CqString>::Create,
        CqParameterTypedVaryingArray<CqColor, type_color, CqColor>::Create,
        0,
        CqParameterTypedVaryingArray<CqVector4D, type_hpoint, CqVector3D>::Create,
        CqParameterTypedVaryingArray<CqVector3D, type_normal, CqVector3D>::Create,
        CqParameterTypedVaryingArray<CqVector3D, type_vector, CqVector3D>::Create,
        0,
        CqParameterTypedVaryingArray<CqMatrix, type_matrix, CqMatrix>::Create,
        0,
        0,
    };

CqParameter* ( *gVariableCreateFuncsVertexArray[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedVertexArray<TqFloat, type_float, TqFloat>::Create,
        CqParameterTypedVertexArray<TqInt, type_integer, TqFloat>::Create,
        CqParameterTypedVertexArray<CqVector3D, type_point, CqVector3D>::Create,
        CqParameterTypedVertexArray<CqString, type_string, CqString>::Create,
        CqParameterTypedVertexArray<CqColor, type_color, CqColor>::Create,
        0,
        CqParameterTypedVertexArray<CqVector4D, type_hpoint, CqVector3D>::Create,
        CqParameterTypedVertexArray<CqVector3D, type_normal, CqVector3D>::Create,
        CqParameterTypedVertexArray<CqVector3D, type_vector, CqVector3D>::Create,
        0,
        CqParameterTypedVertexArray<CqMatrix, type_matrix, CqMatrix>::Create,
        0,
        0,
    };

CqParameter* ( *gVariableCreateFuncsFaceVaryingArray[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedFaceVaryingArray<TqFloat, type_float, TqFloat>::Create,
        CqParameterTypedFaceVaryingArray<TqInt, type_integer, TqFloat>::Create,
        CqParameterTypedFaceVaryingArray<CqVector3D, type_point, CqVector3D>::Create,
        CqParameterTypedFaceVaryingArray<CqString, type_string, CqString>::Create,
        CqParameterTypedFaceVaryingArray<CqColor, type_color, CqColor>::Create,
        0,
        CqParameterTypedFaceVaryingArray<CqVector4D, type_hpoint, CqVector3D>::Create,
        CqParameterTypedFaceVaryingArray<CqVector3D, type_normal, CqVector3D>::Create,
        CqParameterTypedFaceVaryingArray<CqVector3D, type_vector, CqVector3D>::Create,
        0,
        CqParameterTypedFaceVaryingArray<CqMatrix, type_matrix, CqMatrix>::Create,
        0,
        0,
    };

CqParameter* ( *gVariableCreateFuncsFaceVertexArray[] ) ( const char* strName, TqInt Count ) =
    {
        0,
        CqParameterTypedFaceVertexArray<TqFloat, type_float, TqFloat>::Create,
        CqParameterTypedFaceVertexArray<TqInt, type_integer, TqFloat>::Create,
        CqParameterTypedFaceVertexArray<CqVector3D, type_point, CqVector3D>::Create,
        CqParameterTypedFaceVertexArray<CqString, type_string, CqString>::Create,
        CqParameterTypedFaceVertexArray<CqColor, type_color, CqColor>::Create,
        0,
        CqParameterTypedFaceVertexArray<CqVector4D, type_hpoint, CqVector3D>::Create,
        CqParameterTypedFaceVertexArray<CqVector3D, type_normal, CqVector3D>::Create,
        CqParameterTypedFaceVertexArray<CqVector3D, type_vector, CqVector3D>::Create,
        0,
        CqParameterTypedFaceVertexArray<CqMatrix, type_matrix, CqMatrix>::Create,
        0,
        0,
    };

//---------------------------------------------------------------------
/** Copy constructor.
 */

CqNamedParameterList::CqNamedParameterList( const CqNamedParameterList& From ) :
		m_strName( From.m_strName ),
		m_hash( From.m_hash)
{
	TqInt i = From.m_aParameters.size();
	while ( i-- > 0 )
	{
		m_aParameters.push_back( From.m_aParameters[ i ] ->Clone() );
	}
}


} // namespace Aqsis
//---------------------------------------------------------------------
