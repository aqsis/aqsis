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
		\brief Declares support structires for registering shaders, and any built in shaders.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef SHADERS_H_INCLUDED
#define SHADERS_H_INCLUDED 1

#include "color.h"
#include "exception.h"
#include "matrix.h"
#include "sstring.h"
#include "vector3d.h"
#include "list.h"
#include "shadervm.h"

#include "specific.h"

START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------
/** \class CqShaderRegister
 * Class for registering shaders.
 */

class CqShaderRegister : public CqListEntry<CqShaderRegister>
{
	public:
					CqShaderRegister(const char* strName, EqShaderType type, CqShader* pShader) :
										m_strName(strName),
										m_Type(type),
										m_pShader(pShader)
										{}
		virtual		~CqShaderRegister()	{delete(m_pShader);}

						/** Get the name of the shader.
						 * \return Constant CqString reference containing the name.
						 */
		const CqString& strName()		{return(m_strName);}
						/** Get the shader type.
						 * \return Shader type as a member of EqShaderType.
						 */
		EqShaderType	Type()			{return(m_Type);}
						/** Create an instance of this shader.
						 * \return A pointer to the new instance of the shader.
						 */
		CqShader*		Create()		{return(m_pShader->Clone());}

	private:
		CqString	m_strName;		///< The registered name of the shader.
		EqShaderType m_Type;		///< The type of the shader from EqShaderType.
		CqShader*	m_pShader;		///< Pointer to the shader class.
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
// These are the built in shaders, they will be registered as "builtin_<name>"
// these should be used where speed is an issue.

//----------------------------------------------------------------------
/** \class CqShaderSurfaceConstant
 * Built in constant surface shader.
 */

class CqShaderSurfaceConstant : public CqShader
{
	public:
					CqShaderSurfaceConstant()	{}
	virtual			~CqShaderSurfaceConstant()	{}

	virtual	void	Evaluate(CqShaderExecEnv& Env);
	virtual	void	SetValue(const char* name, TqPchar val);
	virtual CqShader* Clone() const	{return(new CqShaderSurfaceConstant);}

	private:
};



//-----------------------------------------------------------------------

END_NAMESPACE(Sappire)

//}  // End of #ifdef SHADERS_H_INCLUDED
#endif
