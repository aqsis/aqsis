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
		\brief Implements the classes for handling RenderMan lightsources, plus any built in sources.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"
#include	"lights.h"
#include	"file.h"

START_NAMESPACE(Aqsis)

CqList<CqLightsource>	Lightsource_stack;

//---------------------------------------------------------------------
/** Default constructor.
 */

CqLightsource::CqLightsource(CqShader* pShader, TqBool fActive) :
										m_pShader(pShader),
										m_pAttributes(0)
{
	// Set a refernce with the current attributes.
	m_pAttributes=const_cast<CqAttributes*>(pCurrentRenderer()->pattrCurrent());
	m_pAttributes->Reference();
//	m_matLightToWorld=pCurrentRenderer()->matCurrent();
//	m_matWorldToLight=pCurrentRenderer()->matCurrent().Inverse();

	// Link into the lightsource stack.
	Lightsource_stack.LinkFirst(this);
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqLightsource::~CqLightsource()
{
	// Release our reference on the current attributes.
	if(m_pAttributes)
		m_pAttributes->UnReference();
	m_pAttributes=0;

	// Unlink from the stack.
	UnLink();
}


//---------------------------------------------------------------------
/** Initialise the environment for the specified grid size.
 * \param iGridRes Integer grid resolution.
 * \param iGridRes Integer grid resolution.
 */
void CqLightsource::Initialise(TqInt uGridRes, TqInt vGridRes)
{
	if(m_pShader)
		m_pShader->Initialise(uGridRes, vGridRes, *this);

	TqInt Uses=gDefLightUses;
	if(m_pShader)	Uses|=m_pShader->Uses();
	CqShaderExecEnv::Initialise(uGridRes, vGridRes, 0, Uses);
	
	L().Initialise(uGridRes, vGridRes, GridI());
	Cl().Initialise(uGridRes, vGridRes, GridI());

	// Initialise the geometric parameters in the shader exec env.
	P()=pCurrentRenderer()->matSpaceToSpace("shader", "current", m_pShader->matCurrent())*CqVector3D(0.0f,0.0f,0.0f);
	if(USES(Uses,EnvVars_u))	u()=0.0f;
	if(USES(Uses,EnvVars_v))	v()=0.0f;
	if(USES(Uses,EnvVars_du))	du()=0.0f;
	if(USES(Uses,EnvVars_du))	dv()=0.0f;
	if(USES(Uses,EnvVars_s))	s()=0.0f;
	if(USES(Uses,EnvVars_t))	t()=0.0f;
	if(USES(Uses,EnvVars_N))	N()=CqVector3D(0.0f,0.0f,0.0f);
}


//---------------------------------------------------------------------
/** Generate shadow map for this lightsource.
 */

/*void CqLightsource::GenerateShadowMap(const char* strShadowName)
{
	if(m_pShader->fAmbient())	return;

	// Store the current renderer state.
	CqOptions	Options=pCurrentRenderer()->optCurrent();
	CqImageBuffer* pBuffer=pCurrentRenderer()->pImage();
	pCurrentRenderer()->SetImage(0);
	CqMatrix	matScreen(pCurrentRenderer()->matSpaceToSpace("world","screen"));
	CqMatrix	matNDC(pCurrentRenderer()->matSpaceToSpace("world","NDC"));
	CqMatrix	matRaster(pCurrentRenderer()->matSpaceToSpace("world","raster"));
	CqMatrix	matCamera(pCurrentRenderer()->matSpaceToSpace("world","camera"));

	// Get the attributes from the lightsource describing the shadowmap.	
	TqInt ShadowXSize=256;
	TqInt ShadowYSize=256;
	const TqInt* pattrShadowSize=m_pAttributes->GetIntegerAttribute("light","shadowmapsize");
	if(pattrShadowSize!=0)
	{
		ShadowXSize=pattrShadowSize[0];
		ShadowYSize=pattrShadowSize[1];
	}
	
	TqFloat ShadowAngle=90;
	const TqFloat* pattrShadowAngle=m_pAttributes->GetFloatAttribute("light","shadowangle");
	if(pattrShadowAngle!=0)
		ShadowAngle=pattrShadowAngle[0];

	
	// Set up the shadow render options through the Ri interfaces.
	RiFormat(ShadowXSize,ShadowYSize,1);
	RiFrameAspectRatio(1);
	RiPixelSamples(1,1);
	RiPixelFilter(RiBoxFilter,1,1);
	RiScreenWindow(-1,1,-1,1);
	RiProjection("perspective","fov",&ShadowAngle,RI_NULL);
	RiDisplay(strShadowName,"shadowmap",RI_Z,RI_NULL);

	// Equivalent to RiWorldBegin
	pCurrentRenderer()->SetmatCamera(m_matWorldToLight);
	pCurrentRenderer()->optCurrent().InitialiseCamera();

	pCurrentRenderer()->SetfSaveGPrims(TqTrue);
	
	// Equivalent to RiWorldEnd
	pCurrentRenderer()->pImage()->SetImage();
	pCurrentRenderer()->pImage()->InitialiseSurfaces(pCurrentRenderer()->Scene());
	pCurrentRenderer()->pImage()->RenderImage();
	RiMakeShadow(strShadowName, strShadowName);
	pCurrentRenderer()->pImage()->DeleteImage();

	// Restore renderer options.
	pCurrentRenderer()->optCurrent()=Options;
	pCurrentRenderer()->SetImage(pBuffer);
	pCurrentRenderer()->SetmatScreen(matScreen);
	pCurrentRenderer()->SetmatNDC(matNDC);
	pCurrentRenderer()->SetmatRaster(matRaster);
	pCurrentRenderer()->SetmatCamera(matCamera);

	pCurrentRenderer()->SetfSaveGPrims(TqFalse);
}
*/


//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
// These are the built in shaders, they will be registered as "builtin_<name>"
// these should be used where speed is an issue.

void CqShaderLightsourceAmbient::SetValue(const char* name, TqPchar val)
{
	if(strcmp(name,"intensity")==0)	intensity=*reinterpret_cast<TqFloat*>(val);
	else
	if(strcmp(name,"lightcolor")==0)	lightcolor=reinterpret_cast<TqFloat*>(val);
	else
	assert(TqFalse);	// TODO: Report error.
}


void CqShaderLightsourceAmbient::Evaluate(CqShaderExecEnv& Env)
{
	Env.Cl().SetValue(lightcolor*intensity);
}



//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)