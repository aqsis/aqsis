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
		\brief Implements support structires for registering shaders, and any built in shaders.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<string.h>

#include	"aqsis.h"
#include	"lights.h"
#include	"shaders.h"
#include	"file.h"

START_NAMESPACE(Aqsis)


//---------------------------------------------------------------------
/** Register a shader of the specified type with the specified name.
 */

void CqShader::RegisterShader(const char* strName, EqShaderType type, CqShader* pShader)
{ 
	assert(pShader);
	pCurrentRenderer()->Shaders().LinkLast(new CqShaderRegister(strName, type, pShader));
}


//---------------------------------------------------------------------
/** Find a shader of the specified type with the specified name.
 */

CqShaderRegister* CqShader::FindShader(const char* strName, EqShaderType type)
{
	// Search the register list.
	CqShaderRegister* pShaderRegister=pCurrentRenderer()->Shaders().pFirst();
	while(pShaderRegister)
	{
		if(pShaderRegister->strName()==strName && pShaderRegister->Type()==type)
			return(pShaderRegister);

		pShaderRegister=pShaderRegister->pNext();
	}
	return(0);
}


//---------------------------------------------------------------------
/** Find a shader of the specified type with the specified name.
 * If not found, try and load one.
 */

CqShader* CqShader::CreateShader(const char* strName, EqShaderType type)
{
	CqShaderRegister* pReg=FindShader(strName, type);
	if(pReg!=0)
	{
		CqShader* pShader=pReg->Create();
		RegisterShader(strName,type,pShader);
		return(pShader);
	}
	else
	{
		// Search in the current directory first.
		CqString strFilename(strName);
		strFilename+=RI_SHADER_EXTENSION;
		CqFile SLXFile(strFilename.c_str(),"shader");
		if(SLXFile.IsValid())
		{
			CqShaderVM* pShader=new CqShaderVM();
			pShader->LoadProgram(SLXFile);
			RegisterShader(strName,type,pShader);
			return(pShader);
		}
		else
		{
			CqString strError("Shader \"");
			strError+=strName;
			strError+="\" not found";
			//strError.Format("Shader \"%s\" not found",strName.String());
			CqBasicError(ErrorID_FileNotFound,Severity_Normal,strError.c_str());
			return(0);
		}
	}
}


//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
// These are the built in shaders, they will be registered as "builtin_<name>"
// these should be used where speed is an issue.

//---------------------------------------------------------------------
/** Set the instance variables on this shader.
 */

void CqShaderSurfaceConstant::SetValue(const char* name, TqPchar val)
{
}

//---------------------------------------------------------------------
/** Evaluate the shader function based on the values in the variables.
 */

void CqShaderSurfaceConstant::Evaluate(CqShaderExecEnv& Env)
{
	Env.Reset();
	do
	{
		Env.Ci()=Env.Os()*Env.Cs();
		Env.Oi()=Env.Os();
	}while(Env.Advance());
}



//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)