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
		\brief Implements the base CqRenderer class which is the central core of the rendering main loop.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<strstream>

#include	<time.h>

#ifdef AQSIS_SYSTEM_WIN32
#include	<process.h>
#endif // AQSIS_SYSTEM_WIN32

#include	"aqsis.h"
#include	"imagebuffer.h"
#include	"lights.h"
#include	"renderer.h"
#include	"shaders.h"
#include	"nurbs.h"
#include	"render.h"
#include	"shadervm.h"
#include	"transform.h"
#include	"file.h"

#include	"ddmsock.h"

START_NAMESPACE(Aqsis)

CqRenderer* pCurrRenderer=0;

//---------------------------------------------------------------------
/** Default constructor for the main renderer class. Initialises current state.
 */

CqRenderer::CqRenderer() :
	m_pImageBuffer(0),
	m_Mode(RenderMode_Image),
	m_fSaveGPrims(TqFalse)
{
	m_pconCurrent=0;
	m_pImageBuffer=new	CqImageBuffer();

	// Initialise the array of coordinate systems.
	m_aCoordSystems.resize(CoordSystem_Last);

	m_aCoordSystems[CoordSystem_Camera]	.m_strName="camera";
	m_aCoordSystems[CoordSystem_Current].m_strName="current";
	m_aCoordSystems[CoordSystem_World]	.m_strName="world";
	m_aCoordSystems[CoordSystem_Screen]	.m_strName="screen";
	m_aCoordSystems[CoordSystem_NDC]	.m_strName="NDC";
	m_aCoordSystems[CoordSystem_Raster]	.m_strName="raster";

#ifdef	AQSIS_SYSTEM_WIN32
	m_pDDManager=new CqDDManager;
	m_pDDManager->Initialise();
#endif
}

//---------------------------------------------------------------------
/** Destructor
 */

CqRenderer::~CqRenderer()
{
	// Delete the current context, should be main, unless render has been aborted.
	while(m_pconCurrent!=0)
	{
		CqContext* pconParent=m_pconCurrent->pconParent();
		delete(m_pconCurrent);
		m_pconCurrent=pconParent;
	}
	if(m_pImageBuffer)
	{
		m_pImageBuffer->Release();
		m_pImageBuffer=0;
	}
	FlushShaders();

	// Close down the Display device manager.
	m_pDDManager->Shutdown();
}


//---------------------------------------------------------------------
/** Create a new main context, called from within RiBegin(), error if not first
 * context created.  If first, create with this as the parent.
 */

CqContext*	CqRenderer::CreateMainContext()
{
	if(m_pconCurrent==0)
	{
		m_pconCurrent=new CqMainContext();
		return(m_pconCurrent);
	}
	else
		return(0);
}


//---------------------------------------------------------------------
/** Create a new Frame context, should only be called when the current 
 * context is a Main context, but the internal context handling deals
 * with it so I don't need to worry.
 */

CqContext*	CqRenderer::CreateFrameContext()
{
	if(m_pconCurrent!=0)
	{
		CqContext* pconNew=m_pconCurrent->CreateFrameContext();
		if(pconNew!=0)
		{
			m_pconCurrent=pconNew;
			return(pconNew);
		}
		else
			return(0);
	}
	else
		return(0);
}


//---------------------------------------------------------------------
/** Create a new world context, again the internal context handling deals
 * with invalid calls.
 */

CqContext*	CqRenderer::CreateWorldContext()
{
	if(m_pconCurrent!=0)
	{
		CqContext* pconNew=m_pconCurrent->CreateWorldContext();
		if(pconNew!=0)
		{
			m_pconCurrent=pconNew;
			return(pconNew);
		}
		else
			return(0);
	}
	else
		return(0);
}


//---------------------------------------------------------------------
/** Create a new attribute context.
 */

CqContext*	CqRenderer::CreateAttributeContext()
{
	if(m_pconCurrent!=0)
	{
		CqContext* pconNew=m_pconCurrent->CreateAttributeContext();
		if(pconNew!=0)
		{
			m_pconCurrent=pconNew;
			return(pconNew);
		}
		else
			return(0);
	}
	else
		return(0);
}



//---------------------------------------------------------------------
/** Create a new transform context.
 */

CqContext*	CqRenderer::CreateTransformContext()
{
	if(m_pconCurrent!=0)
	{
		CqContext* pconNew=m_pconCurrent->CreateTransformContext();
		if(pconNew!=0)
		{
			m_pconCurrent=pconNew;
			return(pconNew);
		}
		else
			return(0);
	}
	else
		return(0);
}



//---------------------------------------------------------------------
/** Create a new solid context.
 */

CqContext*	CqRenderer::CreateSolidContext()
{
	if(m_pconCurrent!=0)
	{
		CqContext* pconNew=m_pconCurrent->CreateSolidContext();
		if(pconNew!=0)
		{
			m_pconCurrent=pconNew;
			return(pconNew);
		}
		else
			return(0);
	}
	else
		return(0);
}



//---------------------------------------------------------------------
/** Create a new object context.
 */

CqContext*	CqRenderer::CreateObjectContext()
{
	if(m_pconCurrent!=0)
	{
		CqContext* pconNew=m_pconCurrent->CreateObjectContext();
		if(pconNew!=0)
		{
			m_pconCurrent=pconNew;
			return(pconNew);
		}
		else
			return(0);
	}
	else
		return(0);
}



//---------------------------------------------------------------------
/** Create a new motion context.
 */

CqContext*	CqRenderer::CreateMotionContext(TqInt N, TqFloat times[])
{
	if(m_pconCurrent!=0)
	{
		CqContext* pconNew=m_pconCurrent->CreateMotionContext(N, times);
		if(pconNew!=0)
		{
			m_pconCurrent=pconNew;
			return(pconNew);
		}
		else
			return(0);
	}
	else
		return(0);
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a main context.
 */

void	CqRenderer::DeleteMainContext()
{
	if(m_pconCurrent!=0)
	{
		CqContext*	pconParent=m_pconCurrent->pconParent();
		m_pconCurrent->DeleteMainContext();
		m_pconCurrent=pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a frame context.
 */

void	CqRenderer::DeleteFrameContext()
{
	if(m_pconCurrent!=0)
	{
		CqContext*	pconParent=m_pconCurrent->pconParent();
		m_pconCurrent->DeleteFrameContext();
		m_pconCurrent=pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a world context.
 */

void	CqRenderer::DeleteWorldContext()
{
	if(m_pconCurrent!=0)
	{
		CqContext*	pconParent=m_pconCurrent->pconParent();
		m_pconCurrent->DeleteWorldContext();
		m_pconCurrent=pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a attribute context.
 */

void	CqRenderer::DeleteAttributeContext()
{
	if(m_pconCurrent!=0)
	{
		CqContext*	pconParent=m_pconCurrent->pconParent();
		m_pconCurrent->DeleteAttributeContext();
		m_pconCurrent=pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a transform context.
 */

void	CqRenderer::DeleteTransformContext()
{
	if(m_pconCurrent!=0)
	{
		CqContext*	pconParent=m_pconCurrent->pconParent();
		// Copy the current state of the attributes UP the stack as a TransformBegin/End doesn't store them
		pconParent->m_pattrCurrent=m_pconCurrent->m_pattrCurrent;
		m_pconCurrent->DeleteTransformContext();
		m_pconCurrent=pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a solid context.
 */

void	CqRenderer::DeleteSolidContext()
{
	if(m_pconCurrent!=0)
	{
		CqContext*	pconParent=m_pconCurrent->pconParent();
		m_pconCurrent->DeleteSolidContext();
		m_pconCurrent=pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a object context.
 */

void	CqRenderer::DeleteObjectContext()
{
	if(m_pconCurrent!=0)
	{
		CqContext*	pconParent=m_pconCurrent->pconParent();
		m_pconCurrent->DeleteObjectContext();
		m_pconCurrent=pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a motion context.
 */

void	CqRenderer::DeleteMotionContext()
{
	if(m_pconCurrent!=0)
	{
		CqContext*	pconParent=m_pconCurrent->pconParent();
		// Copy the current state of the attributes UP the stack as a TransformBegin/End doesn't store them
		pconParent->m_pattrCurrent=m_pconCurrent->m_pattrCurrent;
		pconParent->m_ptransCurrent=m_pconCurrent->m_ptransCurrent;
		m_pconCurrent->DeleteMotionContext();
		m_pconCurrent=pconParent;
	}
}


//----------------------------------------------------------------------
/** Get the current shutter time, always returns 0.0 unless within a motion block,
 * when it returns the appropriate shutter time.
 */

TqFloat	CqRenderer::Time() const
{
	if(m_pconCurrent!=0)
		return(m_pconCurrent->Time());
	else
		return(0);
}

//----------------------------------------------------------------------
/** Advance the current shutter time, only valid within motion blocks.
 */

void CqRenderer::AdvanceTime()
{
	if(m_pconCurrent!=0)
		m_pconCurrent->AdvanceTime();
}


//----------------------------------------------------------------------
/** Return a reference to the current options.
 */

CqOptions& CqRenderer::optCurrent()
{
	if(m_pconCurrent!=0)
		return(m_pconCurrent->optCurrent());
	else
		return(m_optDefault);
}


//----------------------------------------------------------------------
/** Return a pointer to the current attributes.
 */

const CqAttributes* CqRenderer::pattrCurrent()
{
	if(m_pconCurrent!=0)
		return(m_pconCurrent->pattrCurrent());
	else
		return(&m_attrDefault);
}


//----------------------------------------------------------------------
/** Return a writable pointer to the current attributes.
 */

CqAttributes* CqRenderer::pattrWriteCurrent()
{
	if(m_pconCurrent!=0)
		return(m_pconCurrent->pattrWriteCurrent());
	else
		return(&m_attrDefault);
}


//----------------------------------------------------------------------
/** Return a pointer to the current transform.
 */

const CqTransform* CqRenderer::ptransCurrent()
{
	if(m_pconCurrent!=0)
		return(m_pconCurrent->ptransCurrent());
	else
		return(&m_transDefault);
}


//----------------------------------------------------------------------
/** Return a writable pointer to the current transform.
 */

CqTransform* CqRenderer::ptransWriteCurrent()
{
	if(m_pconCurrent!=0)
		return(m_pconCurrent->ptransWriteCurrent());
	else
		return(&m_transDefault);
}


//----------------------------------------------------------------------
/** Render all surface in the current list to the image buffer.
 */

void CqRenderer::RenderWorld()
{
	// Check we have a valid Image buffer
	if(pImage()==0)
		SetImage(new CqImageBuffer);
	
	// Store the time at start.
	m_timeTaken=time(0);

	// Print to the defined output what we are rendering.
	CqString strMsg("Rendering - ");
	strMsg+=optCurrent().strDisplayName();
	CqBasicError(0,Severity_Normal,strMsg.c_str());

	m_pDDManager->OpenDisplays();

	pImage()->RenderImage();

	// Calculate the time taken.
	m_timeTaken=time(0)-m_timeTaken;

	TqInt verbosity=0;
	const TqInt* poptEndofframe=optCurrent().GetIntegerOption("statistics","endofframe");
	if(poptEndofframe!=0)
		verbosity=poptEndofframe[0];

	PrintStats(verbosity);

	m_pDDManager->CloseDisplays();
}


//----------------------------------------------------------------------
/** Output rendering stats if required.
 */

void CqRenderer::PrintStats(TqInt level)
{
	if(level>0)
	{
		std::strstream MSG;

		TqFloat h=m_timeTaken/(60*60);
		TqFloat m=(m_timeTaken/60)-(h*60);
		TqFloat s=(m_timeTaken)-(h*60*60)-(m*60);
		MSG << "Total render time: ";
		if(h>0.0)
			MSG << static_cast<TqInt>(h) << "hrs ";
		if(m>0.0)
			MSG << static_cast<TqInt>(m) << "mins ";
		if(s>0.0)
			MSG << static_cast<TqInt>(s) << "secs ";
		MSG << std::endl << std::endl;

		MSG << "Grids:    \t";
		MSG << Stats().cGridsAllocated() << " created / ";
		MSG << Stats().cGridsAllocated()-Stats().cGridsDeallocated() << " remaining" << std::endl;

		MSG << "Micropolygons: \t";
		MSG << Stats().cMPGsAllocated() << " created / ";
		MSG << Stats().cMPGsAllocated()-Stats().cMPGsDeallocated() << " remaining" << std::endl;

		MSG << "Sampling: \t";
		MSG << Stats().cSamples() << " samples" << std::endl;
		MSG << "          \t" << Stats().cSampleBoundHits() << " bound hits (";
		MSG << (100.0f/Stats().cSamples())*Stats().cSampleBoundHits() << "% of samples)" << std::endl;
		MSG << "          \t" << Stats().cSampleHits() << " hits (";
		MSG << (100.0f/Stats().cSamples())*Stats().cSampleHits() << "% of samples)" << std::endl;

		MSG << "GPrims: \t";
		MSG << Stats().cGPrims() << std::endl;

		MSG << "Attributes: \t";
		MSG << (TqInt)Attribute_stack.size() << " created" << std::endl;

		MSG << "Transforms: \t";
		MSG << (TqInt)TransformStack().size() << " created" << std::endl;

		MSG << "Variables: \t";
		MSG << Stats().cVariablesAllocated() << " created / ";
		MSG << Stats().cVariablesAllocated()-Stats().cVariablesDeallocated() << " remaining" << std::endl;

		MSG << "Parameters: \t";
		MSG << Stats().cParametersAllocated() << " created / ";
		MSG << Stats().cParametersAllocated()-Stats().cParametersDeallocated() << " remaining" << std::endl;
		MSG << std::ends;

		CqString strMSG(MSG.str());
		CqBasicError(0,Severity_Normal,strMSG.c_str());
	}
}

//----------------------------------------------------------------------
/** Quit rendering at the next opportunity.
 */

void CqRenderer::Quit()
{
	if(m_pImageBuffer)
	{
		// Ask the image buffer to quit.
		m_pImageBuffer->Quit();
	}
}


//----------------------------------------------------------------------
/** Initialise the renderer.
 */

void CqRenderer::Initialise()
{
	ClearSymbolTable();
	FlushShaders();

	RiDeclare("Ka","uniform float");
	RiDeclare("Kd","uniform float");
	RiDeclare("Ks","uniform float");
	RiDeclare("Kr","uniform float");
	RiDeclare("roughness","uniform float");
	RiDeclare("texturename","uniform string");
	RiDeclare("specularcolor","uniform color");
	RiDeclare("intensity","uniform float");
	RiDeclare("lightcolor","uniform color");
	RiDeclare("from","uniform point");
	RiDeclare("to","uniform point");
	RiDeclare("coneangle","uniform float");
	RiDeclare("conedeltaangle","uniform float");
	RiDeclare("beamdistribution","uniform float");
	RiDeclare("mindistance","uniform float");
	RiDeclare("maxdistance","uniform float");
	RiDeclare("distance","uniform float");
	RiDeclare("background","uniform color");
	RiDeclare("fov","uniform float");
	RiDeclare("P","vertex point");
	RiDeclare("Pz","vertex point");
	RiDeclare("Pw","vertex hpoint");
	RiDeclare("N","varying normal");
	RiDeclare("Np","uniform normal");
	RiDeclare("Cs","varying color");
	RiDeclare("Os","varying color");
	RiDeclare("s","varying float");
	RiDeclare("t","varying float");
	RiDeclare("st","varying float");
	RiDeclare("bucketsize","uniform integer[2]");
	RiDeclare("eyesplits","uniform integer");
	RiDeclare("shader","uniform string");
	RiDeclare("archive","uniform string");
	RiDeclare("texture","uniform string");
	RiDeclare("display","uniform string");
	RiDeclare("auto_shadows","uniform string");
	RiDeclare("endofframe","uniform integer");
	RiDeclare("sphere","uniform float");
	RiDeclare("coordinatesystem","uniform string");
	RiDeclare("shadows","uniform string");
	RiDeclare("shadowmapsize","uniform integer[2]");
	RiDeclare("shadowangle","uniform float");
	RiDeclare("shadowmapname","uniform string");
	RiDeclare("shadow_shadingrate","uniform float");
	RiDeclare("name","uniform string");
	
	// Register built in shaders.
	RegisterShader("builtin_constant", Type_Surface, new CqShaderSurfaceConstant());

	// Register standard lightsource shaders.
	RegisterShader("ambientlight", Type_Lightsource, new CqShaderLightsourceAmbient());

	CqShaderRegister* pFirst=m_Shaders.pFirst();
	pFirst->Create();

	// Initialise the matrices for this camera according to the 
	// status of the camera attributes.
	optCurrent().InitialiseCamera();

	// Truncate the array of named coordinate systems to just the standard ones.
	m_aCoordSystems.resize(CoordSystem_Last);
}


//----------------------------------------------------------------------
/** Get the matrix to convert between the specified coordinate systems.
 */

CqMatrix	CqRenderer::matSpaceToSpace(const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld, const CqMatrix& matObjectToWorld)
{
	CqMatrix	matResult, matA, matB;
	// Get the two component matrices.
	// First check for special cases.
	if(strcmp(strFrom,"object")==0)	matA=matObjectToWorld;
	else if(strcmp(strFrom,"shader")==0)	matA=matShaderToWorld;
	if(strcmp(strTo,  "object")==0)	matB=matObjectToWorld.Inverse();
	else if(strcmp(strTo,  "shader")==0)	matB=matShaderToWorld.Inverse();
	TqInt i;
	for(i=m_aCoordSystems.size()-1; i>=0; i--)
	{
		if(m_aCoordSystems[i].m_strName==strFrom)	matA=m_aCoordSystems[i].m_matToWorld;
		if(m_aCoordSystems[i].m_strName==strTo  )	matB=m_aCoordSystems[i].m_matWorldTo;
	}
	matResult=matB*matA;

	return(matResult);
}


//----------------------------------------------------------------------
/** Get the matrix to convert vectors between the specified coordinate systems.
 */

CqMatrix	CqRenderer::matVSpaceToSpace(const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld, const CqMatrix& matObjectToWorld)
{
	CqMatrix	matResult, matA, matB;
	// Get the two component matrices.
	// First check for special cases.
	if(strcmp(strFrom,"object")==0)	matA=matObjectToWorld;
	else if(strcmp(strFrom,"shader")==0)	matA=matShaderToWorld;
	if(strcmp(strTo,  "object")==0)	matB=matObjectToWorld.Inverse();
	else if(strcmp(strTo,  "shader")==0)	matB=matShaderToWorld.Inverse();
	TqInt i;
	for(i=m_aCoordSystems.size()-1; i>=0; i--)
	{
		if(m_aCoordSystems[i].m_strName==strFrom)	matA=m_aCoordSystems[i].m_matToWorld;
		if(m_aCoordSystems[i].m_strName==strTo  )	matB=m_aCoordSystems[i].m_matWorldTo;
	}
	matResult=matB*matA;

	matResult[3][0]=matResult[3][1]=matResult[3][2]=matResult[0][3]=matResult[1][3]=matResult[2][3]=0.0;
	matResult[3][3]=1.0;

	return(matResult);
}


//----------------------------------------------------------------------
/** Get the matrix to convert normals between the specified coordinate systems.
 */

CqMatrix	CqRenderer::matNSpaceToSpace(const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld, const CqMatrix& matObjectToWorld)
{
	CqMatrix	matResult, matA, matB;
	// Get the two component matrices.
	// First check for special cases.
	if(strcmp(strFrom,"object")==0)	matA=matObjectToWorld;
	else if(strcmp(strFrom,"shader")==0)	matA=matShaderToWorld;
	if(strcmp(strTo,  "object")==0)	matB=matObjectToWorld.Inverse();
	else if(strcmp(strTo,  "shader")==0)	matB=matShaderToWorld.Inverse();
	TqInt i;
	for(i=m_aCoordSystems.size()-1; i>=0; i--)
	{
		if(m_aCoordSystems[i].m_strName==strFrom)	matA=m_aCoordSystems[i].m_matToWorld;
		if(m_aCoordSystems[i].m_strName==strTo  )	matB=m_aCoordSystems[i].m_matWorldTo;
	}
	matResult=matB*matA;

	matResult[3][0]=matResult[3][1]=matResult[3][2]=matResult[0][3]=matResult[1][3]=matResult[2][3]=0.0;
	matResult[3][3]=1.0;
	matResult.Inverse();	
	matResult.Transpose();

	return(matResult);
}


//----------------------------------------------------------------------
/** Store the named coordinate system in the array of named coordinate systems, overwrite any existing
 * with the same name. Returns TqTrue if system already exists.
 */

TqBool	CqRenderer::SetCoordSystem(const char* strName, const CqMatrix& matToWorld)
{
	// Search for the same named system in the current list.
	for(TqUint i=0; i<m_aCoordSystems.size(); i++)
	{
		if(m_aCoordSystems[i].m_strName==strName)
		{
			m_aCoordSystems[i].m_matToWorld=matToWorld;
			m_aCoordSystems[i].m_matWorldTo=matToWorld.Inverse();
			return(TqTrue);
		}
	}

	// If we got here, it didn't exists.
	m_aCoordSystems.push_back(SqCoordSys(strName, matToWorld, matToWorld.Inverse()));
	return(TqFalse);
}


//----------------------------------------------------------------------
/** Find a parameter type declaration and return it.
 * \param strDecl Character pointer to the name of the declaration to find.
 */

SqParameterDeclaration CqRenderer::FindParameterDecl(const char* strDecl)
{
	TqInt Count=1;
	CqString strName("");
	EqVariableType ILType=Type_Nil;
	
	// First check if the declaration has embedded type information.
	CqString strLocalDecl(strDecl);
	TqInt i;
	for(i=0; i<gcVariableStorageNames; i++)
	{
		if(strLocalDecl.find(gVariableStorageNames[i])!=CqString::npos)
		{
			ILType=(EqVariableType)(ILType|(0x01<<(Storage_Shift+i))&Storage_Mask);
			break;
		}
	}

	for(i=0; i<gcVariableTypeNames; i++)
	{
		if(strLocalDecl.find(gVariableTypeNames[i])!=CqString::npos)
		{
			ILType=(EqVariableType)(ILType|(i&Type_Mask));
			break;
		}
	}

	// Now search for an array specifier.
	TqInt s,e;
	if((s=strLocalDecl.find('['))!=CqString::npos)
	{
		if((e=strLocalDecl.find(']'))!=CqString::npos && e>s)
		{
			Count=static_cast<TqInt>(atoi(strLocalDecl.substr(s+1,e-(s+1)).c_str()));
			ILType=(EqVariableType)(ILType|Type_Array);
		}
	}
	
	// Copy the token to the name.
	s=strLocalDecl.find_last_of(' ');
	if(s!=CqString::npos)	strName=strLocalDecl.substr(s+1);
	else						strName=strLocalDecl;

	if((ILType&Type_Mask)!=Type_Nil)
	{
		// Default to uniform if no storage specified
		if((ILType&Storage_Mask)==Type_Nil)
			ILType=(EqVariableType)(ILType|Type_Uniform);

		SqParameterDeclaration Decl;
		Decl.m_strName=strName;
		Decl.m_Count=Count;
		Decl.m_Type=ILType;
		Decl.m_strSpace="";
		
		// Get the creation function.
		switch(ILType&Storage_Mask)
		{
			case Type_Uniform:
			{
				if(ILType&Type_Array)
					Decl.m_pCreate=gVariableCreateFuncsUniformArray[ILType&Type_Mask];
				else
					Decl.m_pCreate=gVariableCreateFuncsUniform[ILType&Type_Mask];
			}
			break;

			case Type_Varying:
			{
				if(ILType&Type_Array)
					Decl.m_pCreate=gVariableCreateFuncsVaryingArray[ILType&Type_Mask];
				else				
					Decl.m_pCreate=gVariableCreateFuncsVarying[ILType&Type_Mask];
			}	
			break;

			case Type_Vertex:
			{
				if(ILType&Type_Array)
					Decl.m_pCreate=gVariableCreateFuncsVertexArray[ILType&Type_Mask];
				else
					Decl.m_pCreate=gVariableCreateFuncsVertex[ILType&Type_Mask];
			}	
			break;
		}
		return(Decl);
	}
	
	strName=strDecl;
	// Search the local parameter declaration list.
	for(i=0; i<m_Symbols.size(); i++)
	{
		if(strName==m_Symbols[i].m_strName)
			return(m_Symbols[i]);
	}
	return(SqParameterDeclaration("",Type_Nil,0,0,""));
}


//----------------------------------------------------------------------
/** Add a parameter type declaration to the local declarations.
 * \param strName Character pointer to parameter name.
 * \param strType Character pointer to string containing the type identifier.
 */

void CqRenderer::AddParameterDecl(const char* strName, const char* strType)
{
	CqString strDecl(strType);
	strDecl+=" ";
	strDecl+=strName;
	SqParameterDeclaration Decl=FindParameterDecl(strDecl.c_str());

	// Put new declaration at the top to make it take priority over pervious
	m_Symbols.insert(m_Symbols.begin(),Decl);
}


//---------------------------------------------------------------------
/** Register a shader of the specified type with the specified name.
 */

void CqRenderer::RegisterShader(const char* strName, EqShaderType type, CqShader* pShader)
{ 
	assert(pShader);
	m_Shaders.LinkLast(new CqShaderRegister(strName, type, pShader));
}


//---------------------------------------------------------------------
/** Find a shader of the specified type with the specified name.
 */

CqShaderRegister* CqRenderer::FindShader(const char* strName, EqShaderType type)
{
	// Search the register list.
	CqShaderRegister* pShaderRegister=m_Shaders.pFirst();
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

CqShader* CqRenderer::CreateShader(const char* strName, EqShaderType type)
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
/** Add a new requested display driver to the list.
 */

void CqRenderer::AddDisplayRequest(const TqChar* name, const TqChar* type, const TqChar* mode)
{
	m_pDDManager->AddDisplay(name, type, mode);
}



//---------------------------------------------------------------------
/** Clear the list of requested display drivers.
 */

void CqRenderer::ClearDisplayRequests()
{
	m_pDDManager->ClearDisplays();
}


void QSetRenderContext(CqRenderer* pRend)
{
	pCurrRenderer=pRend;
}

//---------------------------------------------------------------------
 
END_NAMESPACE(Aqsis)
