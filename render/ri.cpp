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
		\brief Implement the majority of the RenderMan API functions.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<stdarg.h>
#include	<math.h>

#include	"aqsis.h"
#include	"imagebuffer.h"
#include	"lights.h"
#include	"irenderer.h"
#include	"scene.h"
#include	"patch.h"
#include	"polygon.h"
#include	"nurbs.h"
#include	"subdivision.h"
#include	"symbols.h"
#include	"bilinear.h"
#include	"quadrics.h"
#include	"shaders.h"
#include	"texturemap.h"
#include	"messages.h"

#include	"ri.h"

#define		_qShareName	LIBRARY
#include	"share.h"

using namespace Aqsis;

static RtBoolean ProcessPrimitiveVariables(CqSurface* pSurface, PARAMETERLIST);
template<class T>
RtVoid	CreateGPrim(T* pSurface);


//---------------------------------------------------------------------
// This file contains the interface functions which are published as the
//	Renderman Interface SPECification (C) 1988 Pixar.
//

//---------------------------------------------------------------------
// Interface parameter token strings.


RtToken	RI_FRAMEBUFFER			="framebuffer";
RtToken	RI_FILE					="file";
RtToken	RI_RGB					="rgb";
RtToken	RI_RGBA					="rgba";
RtToken	RI_RGBZ					="rgbz";
RtToken	RI_RGBAZ				="rgbaz";
RtToken	RI_A					="a";
RtToken	RI_Z					="z";
RtToken	RI_AZ					="az";
RtToken	RI_MERGE				="merge";
RtToken	RI_ORIGIN				="origin";
RtToken	RI_PERSPECTIVE			="perspective";
RtToken	RI_ORTHOGRAPHIC			="orthographic";
RtToken	RI_HIDDEN				="hidden";
RtToken	RI_PAINT				="paint";
RtToken	RI_CONSTANT				="constant";
RtToken	RI_SMOOTH				="smooth";
RtToken	RI_FLATNESS				="flatness";
RtToken	RI_FOV					="fov";

RtToken	RI_AMBIENTLIGHT			="ambientlight";
RtToken	RI_POINTLIGHT			="pointlight";
RtToken	RI_DISTANTLIGHT			="distantlight";
RtToken	RI_SPOTLIGHT			="spotlight";
RtToken	RI_INTENSITY			="intensity";	
RtToken	RI_LIGHTCOLOR			="lightcolor";
RtToken	RI_FROM					="from";
RtToken	RI_TO					="to";
RtToken	RI_CONEANGLE			="coneangle";
RtToken	RI_CONEDELTAANGLE		="conedeltaangle";
RtToken	RI_BEAMDISTRIBUTION		="beamdistribution";
RtToken	RI_MATTE				="matte";
RtToken	RI_METAL				="metal";
RtToken	RI_PLASTIC				="plastic";
RtToken	RI_KA					="ka";
RtToken	RI_KD					="kd";
RtToken	RI_KS					="ks";
RtToken	RI_ROUGHNESS			="roughness";
RtToken	RI_SPECULARCOLOR		="specularcolor";
RtToken	RI_DEPTHCUE				="depthcue";
RtToken	RI_FOG					="fog";
RtToken	RI_MINDISTANCE			="mindistance";
RtToken	RI_MAXDISTANCE			="maxdistance";
RtToken	RI_BACKGROUND			="background";
RtToken	RI_DISTANCE				="distance";

RtToken	RI_RASTER				="raster";
RtToken	RI_SCREEN				="screen";
RtToken	RI_CAMERA				="camera";
RtToken	RI_WORLD				="world";
RtToken	RI_OBJECT				="object";
RtToken	RI_INSIDE				="inside";
RtToken	RI_OUTSIDE				="outside";
RtToken	RI_LH					="lh";
RtToken	RI_RH					="rh";
RtToken	RI_P					="P";
RtToken	RI_PZ					="Pz";
RtToken	RI_PW					="Pw";
RtToken	RI_N					="N";
RtToken	RI_NP					="Np";
RtToken	RI_CS					="Cq";
RtToken	RI_OS					="Os";
RtToken	RI_S					="s";
RtToken	RI_T					="t";
RtToken	RI_ST					="st";
RtToken	RI_BILINEAR				="bilinear";
RtToken	RI_BICUBIC				="bicubic";
RtToken	RI_PRIMITIVE			="primitive";
RtToken	RI_INTERSECTION			="intersection";
RtToken	RI_UNION				="union";
RtToken	RI_DIFFERENCE			="difference";
RtToken	RI_WRAP					="wrap";
RtToken	RI_NOWRAP				="nowrap";
RtToken	RI_PERIODIC				="periodic";
RtToken	RI_NONPERIODIC			="nonperiodic";
RtToken	RI_CLAMP				="clamp";
RtToken	RI_BLACK				="black";
RtToken	RI_IGNORE				="ignore";
RtToken	RI_PRINT				="print";
RtToken	RI_ABORT				="abort";
RtToken	RI_HANDLER				="handler";

RtBasis	RiBezierBasis			={-1, 3,-3, 1,
								   3,-6, 3, 0,
								  -3, 3, 0, 0,
								   1, 0, 0, 0};
RtBasis	RiBSplineBasis			={-1, 3,-3, 1,
								   3,-6, 3, 0,
								  -3, 0, 3, 0,
								   1, 4, 1, 0};
RtBasis	RiCatmullRomBasis		={-1, 3,-3, 1,
								   2,-5, 4, -1,
								  -1, 0, 1, 0,
								   0, 2, 0, 0};
RtBasis	RiHermiteBasis			={ 2, 1,-2, 1,
			 					  -3,-2, 3,-1,
								   0, 1, 0, 0,
								   1, 0, 0, 0};
RtBasis	RiPowerBasis			={ 1, 0, 0, 0,
								   0, 1, 0, 0,
								   0, 0, 1, 0,
								   0, 0, 0, 1};

enum RIL_POINTS
{
	RIL_NONE=-1,
	RIL_P,
	RIL_Pz,
	RIL_Pw,
	RIL_N,
	RIL_Np,
	RIL_s,
	RIL_t=RIL_s,
	RIL_st,
};

//----------------------------------------------------------------------
// BuildParameterList
// Helper function to build a parameter list to pass on to the V style functions.
// returns a parameter count.

RtInt BuildParameterList(va_list pArgs, RtToken*& pTokens, RtPointer*& pValues)
{
	// TODO: Check this is thread safe.
	static std::vector<RtToken>		aTokens;
	static std::vector<RtPointer>	aValues;

	RtInt count=0;
	RtToken pToken=va_arg(pArgs, RtToken);
	RtPointer pValue;
	aTokens.clear();
	aValues.clear();
	while(pToken!=0 && pToken!=RI_NULL)	// While not RI_NULL
	{
		aTokens.push_back(pToken);
		pValue=va_arg(pArgs, RtPointer);
		aValues.push_back(pValue);
		pToken=va_arg(pArgs, RtToken);
		count++;
	}

	pTokens=&aTokens[0];
	pValues=&aValues[0];
	return(count);
}

//----------------------------------------------------------------------
// RiDeclare
// Declare a new variable to be recognised by the system.
//
 RtToken	RiDeclare(const char *name, const char *declaration)
{
	CqString strName(name), strDecl(declaration);
	pCurrentRenderer()->AddParameterDecl(strName.c_str(),strDecl.c_str());
	return(0);
}

//----------------------------------------------------------------------
// RiBegin
// Begin a Renderman render phase.
//
RtVoid	RiBegin(RtToken name)
{
	pCurrentRenderer()->Initialise();
	pCurrentRenderer()->CreateMainContext();
	pCurrentRenderer()->ptransWriteCurrent()->SetCurrentTransform(pCurrentRenderer()->Time(),CqMatrix());
	// Clear the scene objects.
	pCurrentRenderer()->Scene().ClearScene();
	// Clear the lightsources stack.
	CqLightsource* pL=Lightsource_stack.pFirst();
	while(pL)
	{
		delete(pL);
		pL=Lightsource_stack.pFirst();
	}

	// Clear any options.
	pCurrentRenderer()->optCurrent().ClearOptions();

	// Set the default search path for shaders to the shaders directory under the executable directory.
	// Read config file name out of the ini file.
	char strExe[255];
	char strDrive[10];
	char strPath[255];
	char strShaderPath[255];
	GetModuleFileName(NULL, strExe, 255);
	_splitpath(strExe,strDrive,strPath,NULL,NULL);
	_makepath(strShaderPath,strDrive,strPath,"shaders","");

	char* strShaderOpt="shader";
	char* pValue=strShaderPath;
	RiOption("searchpath", (RtPointer)strShaderOpt, (RtPointer)&pValue, NULL);

	// Call the renderer callback, which can be overridden for system specific setup.	
	pCurrentRenderer()->OnBegin();

	return(0);
}

//----------------------------------------------------------------------
// RiEnd
// End the rendermam render stage.
//
RtVoid	RiEnd()
{
	pCurrentRenderer()->DeleteMainContext();

	// Flush the shader cache.
	pCurrentRenderer()->FlushShaders();

	// Flush the image cache.
	CqTextureMap::FlushCache();

	// Clear the lightsources stack.
	CqLightsource* pL=Lightsource_stack.pFirst();
	while(pL)
	{
		delete(pL);
		pL=Lightsource_stack.pFirst();
	}

	// Call the renderer callback, which can be overridden for system specific setup.	
	pCurrentRenderer()->OnEnd();
	return(0);
}


//----------------------------------------------------------------------
// RiFrameBegin
// Begin an individual frame, options are saved at this point.
//
RtVoid	RiFrameBegin(RtInt number)
{
	pCurrentRenderer()->CreateFrameContext();
	// Call the renderer callback, which can be overridden for system specific setup.	
	pCurrentRenderer()->OnFrameBegin();
	return(0);
}


//----------------------------------------------------------------------
// RiFrameEnd
// End the rendering of an individual frame, options are restored.
//
RtVoid	RiFrameEnd()
{
	pCurrentRenderer()->DeleteFrameContext();
	// Delete the scene
	pCurrentRenderer()->Scene().ClearScene();

	// Call the renderer callback, which can be overridden for system specific setup.	
	pCurrentRenderer()->OnFrameEnd();

	return(0);
}

//----------------------------------------------------------------------
// RiWorldBegin
// Start the information for the world, options are now frozen.  The world-to-camera
// transformation is set to the current transformation, and current is set to identity.
//
RtVoid	RiWorldBegin()
{
	// Now that the options have all been set, setup any undefined camera parameters.
	if(!pCurrentRenderer()->optCurrent().FrameAspectRatioCalled())
	{
		// Derive the FAR from the resolution and pixel aspect ratio.
		RtFloat PAR=pCurrentRenderer()->optCurrent().fPixelAspectRatio();
		RtFloat resH=pCurrentRenderer()->optCurrent().iXResolution();
		RtFloat resV=pCurrentRenderer()->optCurrent().iYResolution();
		pCurrentRenderer()->optCurrent().SetfFrameAspectRatio((resH*PAR)/resV);
	}
	
	if(!pCurrentRenderer()->optCurrent().ScreenWindowCalled())
	{
		RtFloat fFAR=pCurrentRenderer()->optCurrent().fFrameAspectRatio();

		if(fFAR>=1.0)
		{
			pCurrentRenderer()->optCurrent().SetfScreenWindowLeft(-fFAR);
			pCurrentRenderer()->optCurrent().SetfScreenWindowRight(+fFAR);
			pCurrentRenderer()->optCurrent().SetfScreenWindowBottom(-1);
			pCurrentRenderer()->optCurrent().SetfScreenWindowTop(+1);
		}
		else
		{
			pCurrentRenderer()->optCurrent().SetfScreenWindowLeft(-1);
			pCurrentRenderer()->optCurrent().SetfScreenWindowRight(+1);
			pCurrentRenderer()->optCurrent().SetfScreenWindowBottom(-1.0/fFAR);
			pCurrentRenderer()->optCurrent().SetfScreenWindowTop(+1.0/fFAR);
		}
	}
	
	pCurrentRenderer()->CreateWorldContext();
	// Set the world to camera transformation matrix to the current matrix.
	pCurrentRenderer()->SetmatCamera(pCurrentRenderer()->matCurrent(pCurrentRenderer()->Time()));
	// and then reset the current matrix to identity, ready for object transformations.
	pCurrentRenderer()->ptransWriteCurrent()->SetCurrentTransform(pCurrentRenderer()->Time(),CqMatrix());

	// Call the renderer callback, which can be overridden for system specific setup.	
	pCurrentRenderer()->OnWorldBegin();

	return(0);
}


//----------------------------------------------------------------------
// RiWorldEnd
// End the specifying of world data, options are released.
//

RtVoid	RiWorldEnd()
{
	// Render the world
	pCurrentRenderer()->RenderWorld();
	pCurrentRenderer()->DeleteWorldContext();

	// Call the renderer callback, which can be overridden for system specific setup.	
	pCurrentRenderer()->OnWorldEnd();

	return(0);
}


//----------------------------------------------------------------------
// RiFormat
// Specify the setup of the final image.
//
RtVoid	RiFormat(RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio)
{
	pCurrentRenderer()->optCurrent().SetiXResolution(xresolution);
	pCurrentRenderer()->optCurrent().SetiYResolution(yresolution);
	pCurrentRenderer()->optCurrent().SetfPixelAspectRatio((pixelaspectratio<0.0)?1.0:pixelaspectratio);

	// Inform the system that RiFormat has been called, as this takes priority.
	pCurrentRenderer()->optCurrent().CallFormat();
	
	return(0);
}


//----------------------------------------------------------------------
// RiFrameAspectRatio
// Set the aspect ratio of the frame irrespective of the display setup.
//
RtVoid	RiFrameAspectRatio(RtFloat frameratio)
{
	pCurrentRenderer()->optCurrent().SetfFrameAspectRatio(frameratio);

	// Inform the system that RiFrameAspectRatio has been called, as this takes priority.
	pCurrentRenderer()->optCurrent().CallFrameAspectRatio();

	return(0);
}


//----------------------------------------------------------------------
// RiScreenWindow
// Set the resolution of the screen window in the image plane specified in the screen
// coordinate system.
//
RtVoid	RiScreenWindow(RtFloat left, RtFloat right, RtFloat bottom, RtFloat top)
{
	pCurrentRenderer()->optCurrent().SetfScreenWindowLeft(left);
	pCurrentRenderer()->optCurrent().SetfScreenWindowRight(right);
	pCurrentRenderer()->optCurrent().SetfScreenWindowBottom(bottom);
	pCurrentRenderer()->optCurrent().SetfScreenWindowTop(top);

	// Inform the system that RiScreenWindow has been called, as this takes priority.
	pCurrentRenderer()->optCurrent().CallScreenWindow();

	return(0);
}


//----------------------------------------------------------------------
// RiCropWindow
// Set the position and size of the crop window specified in fractions of the raster
// window.
//
RtVoid	RiCropWindow(RtFloat left, RtFloat right, RtFloat top, RtFloat bottom)
{
	pCurrentRenderer()->optCurrent().SetfCropWindowXMin(left);
	pCurrentRenderer()->optCurrent().SetfCropWindowXMax(right);
	pCurrentRenderer()->optCurrent().SetfCropWindowYMin(top);
	pCurrentRenderer()->optCurrent().SetfCropWindowYMax(bottom);

	return(0);
}


//----------------------------------------------------------------------
// RiProjection
// Set the camera projection to be used.
//
RtVoid	RiProjection(const char *name, ...)
{
	va_list	pArgs;
	va_start(pArgs, name);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiProjectionV(name, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiProjectionV
// List mode version of above.
//
RtVoid	RiProjectionV(const char *name, PARAMETERLIST)
{
	if(strcmp(name,RI_PERSPECTIVE)==0)
		pCurrentRenderer()->optCurrent().SeteCameraProjection(ProjectionPerspective);
	else
		pCurrentRenderer()->optCurrent().SeteCameraProjection(ProjectionOrthographic);

	RtInt i;
	for(i=0; i<count; i++)
	{
		RtToken		token=tokens[i];
		RtPointer	value=values[i];

		if(strcmp(token, RI_FOV)==0)
			pCurrentRenderer()->optCurrent().SetfFOV(*(reinterpret_cast<RtFloat*>(value)));
	}
	// TODO: need to get the current transformation so that it can be added to the screen transformation.
	pCurrentRenderer()->ptransWriteCurrent()->SetCurrentTransform(pCurrentRenderer()->Time(),CqMatrix());
	
	return(0);
}


//----------------------------------------------------------------------
// RiClipping
// Set the near and far clipping planes specified as distances from the camera.
//
RtVoid	RiClipping(RtFloat cnear, RtFloat cfar)
{
	pCurrentRenderer()->optCurrent().SetfClippingPlaneNear(cnear);
	pCurrentRenderer()->optCurrent().SetfClippingPlaneFar(cfar);

	return(0);
}


//----------------------------------------------------------------------
// RiDepthOfField
// Specify the parameters which affect focal blur of the camera.
//
RtVoid	RiDepthOfField(RtFloat fstop, RtFloat focallength, RtFloat focaldistance)
{
	pCurrentRenderer()->optCurrent().SetffStop(fstop);
	pCurrentRenderer()->optCurrent().SetfFocalLength(focallength);
	pCurrentRenderer()->optCurrent().SetfFocalDistance(focaldistance);

	return(0);
}


//----------------------------------------------------------------------
// RiShutter
//	Set the times at which the shutter opens and closes, used for motion blur.
//
RtVoid	RiShutter(RtFloat opentime, RtFloat closetime)
{
	pCurrentRenderer()->optCurrent().SetfShutterOpen(opentime);
	pCurrentRenderer()->optCurrent().SetfShutterClose(closetime);

	return(0);
}


//----------------------------------------------------------------------
// RiPixelVariance
// Set the upper bound on the variance from the true pixel color by the pixel filter
// function.
//
RtVoid	RiPixelVariance(RtFloat variance)
{
	pCurrentRenderer()->optCurrent().SetfPixelVariance(variance);

	return(0);
}


//----------------------------------------------------------------------
// RiPixelSamples
// Set the number of samples per pixel for the hidden surface function.
//
RtVoid	RiPixelSamples(RtFloat xsamples, RtFloat ysamples)
{
	pCurrentRenderer()->optCurrent().SetfPixelXSamples(xsamples);
	pCurrentRenderer()->optCurrent().SetfPixelYSamples(ysamples);

	return(0);
}


//----------------------------------------------------------------------
// RiPixelFilter
// Set the function used to generate a final pixel value from supersampled values.
//
RtVoid	RiPixelFilter(RtFilterFunc function, RtFloat xwidth, RtFloat ywidth)
{
	pCurrentRenderer()->optCurrent().SetfuncFilter(function);
	pCurrentRenderer()->optCurrent().SetfFilterXWidth(xwidth);
	pCurrentRenderer()->optCurrent().SetfFilterYWidth(ywidth);

	return(0);
}


//----------------------------------------------------------------------
// RiExposure
//	Set the values of the exposure color modification function.
//
RtVoid	RiExposure(RtFloat gain, RtFloat gamma)
{
	pCurrentRenderer()->optCurrent().SetfExposureGain(gain);
	pCurrentRenderer()->optCurrent().SetfExposureGamma(gamma);

	return(0);
}


//----------------------------------------------------------------------
// RiImager
// Specify a prepocessing imager shader.
//
RtVoid	RiImager(const char *name, ...)
{
	CqBasicError(0,Severity_Normal,"RiImager shaders not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiImagerV 
// List based version of above.
//
RtVoid	RiImagerV(const char *name, PARAMETERLIST)
{
	CqBasicError(0,Severity_Normal,"RiImager shaders not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiQuantize
// Specify the color quantization parameters.
//
RtVoid	RiQuantize(RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ditheramplitude)
{
	if(strcmp(type, "rgba")==0)
	{
		pCurrentRenderer()->optCurrent().SetiColorQuantizeOne(one);
		pCurrentRenderer()->optCurrent().SetiColorQuantizeMin(min);
		pCurrentRenderer()->optCurrent().SetiColorQuantizeMax(max);
		pCurrentRenderer()->optCurrent().SetfColorQuantizeDitherAmplitude(ditheramplitude);
	}
	else
	{
		pCurrentRenderer()->optCurrent().SetiDepthQuantizeOne(one);
		pCurrentRenderer()->optCurrent().SetiDepthQuantizeMin(min);
		pCurrentRenderer()->optCurrent().SetiDepthQuantizeMax(max);
		pCurrentRenderer()->optCurrent().SetfDepthQuantizeDitherAmplitude(ditheramplitude);
	}

	return(0);
}


//----------------------------------------------------------------------
// RiDisplay
// Set the final output name and type.
//
RtVoid	RiDisplay(const char *name, RtToken type, RtToken mode, ...)
{
	va_list	pArgs;
	va_start(pArgs, mode);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiDisplayV(name, type, mode, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiDisplayV
// List based version of above.
//
RtVoid	RiDisplayV(const char *name, RtToken type, RtToken mode, PARAMETERLIST)
{
	CqString strName(name);
	CqString strType(type);

	pCurrentRenderer()->optCurrent().SetstrDisplayName(strName.c_str());
	pCurrentRenderer()->optCurrent().SetstrDisplayType(strType.c_str());
	
	RtInt eValue=0;
	if(strstr(mode, RI_RGB)!=NULL)
		eValue+=ModeRGB;
	if(strstr(mode, RI_A)!=NULL)
		eValue+=ModeA;
	if(strstr(mode, RI_Z)!=NULL)
		eValue+=ModeZ;
	pCurrentRenderer()->optCurrent().SetiDisplayMode(eValue);
	pCurrentRenderer()->LoadDisplayLibrary();

	return(0);
}


//----------------------------------------------------------------------
// RiGaussianFilter
// Gaussian filter used as a possible value passed to RiPixelFilter.
//
RtFloat	RiGaussianFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth)
{
   /* 
    *  d = distance from origin
    *  w = filterwidth ([COOK84a] article used 1.5)
    *      For here use sqrt( (xwidth/2)*(xwidth/2) + (ywidth/2)*(ywidth/2) ).
    *      Simplifying:
    *
    *          w = sqrt( (xwidth*xwidth)/2 + (ywidth*ywidth)/2 )
    *          w = sqrt( (xwidth*xwidth + ywidth*ywidth)/2 )
    *        w*w = (xwidth*xwidth + ywidth*ywidth)/2
    *
    *  if (d < filterwidth) then 0
    *  else  exp(-d*d) - exp(-w*w)
    *
    */
	RtFloat d,d2,w,w2;

	/* d = sqrt(x*x+y*y), d*d = (x*x+y*y)  */
	d2 = (x*x+y*y);
	d = sqrt(d2);

	w2 = 0.5*(xwidth*xwidth + ywidth*ywidth);
	w = sqrt(w2);

	if(d>w) 
		return(0.0);
	else
		return(exp(-d2) - exp(-w2));
}


//----------------------------------------------------------------------
// RiBoxFilter
// Box filter used as a possible value passed to RiPixelFIlter.
//
RtFloat	RiBoxFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth)
{
   /* [UPST89] -- (RC p. 178) says that x and y will be in the
    *    following intervals:
    *           -xwidth/2 <= x <= xwidth/2
    *           -ywidth/2 <= y <= ywidth/2
    *    These constraints on x and y really simplifies the
    *       the following code to just return (1.0).  
    *
    */
   return MIN((fabs(x)<=xwidth/2.0?1.0:0.0), 
			  (fabs(y)<=ywidth/2.0?1.0:0.0));
}


//----------------------------------------------------------------------
// RiTriangleFilter
// Triangle filter used as a possible value passed to RiPixelFilter
//
RtFloat	RiTriangleFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth)
{
	RtFloat	hxw  = xwidth/2.0;
	RtFloat	hyw  = ywidth/2.0;
	RtFloat	absx = fabs(x);
	RtFloat	absy = fabs(y);

   /* This function can be simplified as well by not worrying about
    *    returning zero if the sample is beyond the filter window.
    */
   return MIN( (absx <= hxw ? (hxw - absx)/hxw : 0.0), 
	           (absy <= hyw ? (hyw - absy)/hyw : 0.0) );
}


//----------------------------------------------------------------------
// RiCatmullRomFilter
// Catmull Rom filter used as a possible value passed to RiPixelFilter.
//
RtFloat	RiCatmullRomFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth)
{
   /*
    * From page 223 of [MITC88] 
    *
    * if abs(d) < 1
    *    f(d) = 1/6*(  (12-9*B-9*C)*abs(d*d*d)
    *                + (-18 + 12*B + 6*C)*d*d + (6-2*B) )
    *
    * if 1 <= abs(d) < 2
    *    f(d) = 1/6*(  (-B-6*C)*abs(d*d*d)
    *                + (6*B + 30*C)*d*d
    *                + (-12*B - 48*C)*d
    *                + (8*B + 24*C) )
    *
    * otherwise  f(d)=0
    *
    * -------------------------------------------------------------
    *  When B = 0.0 and C = 0.5 the filter is a Catmull-Rom cubic spline.
    *
    * if abs(d) < 1
    *    f(d) = 1/6*[  (12-3)*abs(d*d*d) + (-18 + 3)*d*d + (6) ]
    *
    * if 1 <= abs(d) < 2
    *    f(d) = 1/6*[  (-3)*abs(d*d*d) + (15)*d*d + (-24)*d + (12) ]
    *
    * otherwise  f(d)=0
    * -------------------------------------------------------------
    * Simplifying:
    *
    * if abs(d) < 1
    *    f(d) = (3/2)*abs(d*d*d) - (5/2)*d*d + 1
    *
    * if 1 <= abs(d) <2
    *    f(d) = (-0.5)*abs(d*d*d) + (5/2)*d*d - 4*abs(d) + 2
    *
    * otherwise  f(d)=0
    *
    */
   
	RtFloat  d,d1,d2,d3;

	d  = sqrt(x*x+y*y); /* distance from origin */
	d1 = fabs(d);		/* was originally abs(d), changed 08/04/99 */
	d2 = d*d;
	d3 =  d2*d1; /* abs(d*d*d) */
   
	if(d1<1.0)
		return((3.0/2.0)*d3-(5.0/2.0)*d2+1.0);
	else 
		if(d1<2.0)
			return((-0.5)*d3+(5.0/2.0)*d2-4.0*d1+2.0);
		else
			return(0.0);
}


//----------------------------------------------------------------------
// RiSincFilter
// Sinc filter used as a possible value passed to RiPixelFilter.
//
RtFloat	RiSincFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth)
{
	RtFloat d;

	d = sqrt(x*x+y*y);

	if(d!=0)
		return(sin(RI_PI*d)/(RI_PI*d));
	else
		return(1.0);
}


//----------------------------------------------------------------------
// RiHider
// Specify a hidden surface calculation mode.
//
RtVoid	RiHider(RtToken type, ...)
{
	CqBasicError(0,Severity_Normal,"RiHider not supported, defaults to hidden");
	return(0);
}


//----------------------------------------------------------------------
// RiHiderV
// List based version of above.
//
RtVoid	RiHiderV(RtToken type, PARAMETERLIST)
{
	CqBasicError(0,Severity_Normal,"RiHider not supported, defaults to hidden");
	return(0);
}


//----------------------------------------------------------------------
// RiColorSamples
// Specify the depth and conversion arrays for color manipulation.
//
RtVoid	RiColorSamples(RtInt N, RtFloat *nRGB, RtFloat *RGBn)
{
	CqBasicError(0,Severity_Normal,"RiColorSamples not supported, defaults to 3");
	return(0);
}


//----------------------------------------------------------------------
// RiRelativeDetail
// Set the scale used for all subsequent level of detail calculations.
//
RtVoid	RiRelativeDetail(RtFloat relativedetail)
{
	CqBasicError(0,Severity_Normal,"RiRelativeDetail not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiOption
// Specify system specific option.
//
RtVoid	RiOption(const char *name, ...)
{
	va_list	pArgs;
	va_start(pArgs, name);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiOptionV(name, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiOptionV
// List based version of above.
//
RtVoid	RiOptionV(const char *name, PARAMETERLIST)
{
	// Find the parameter on the current options.
	CqSystemOption* pOpt=pCurrentRenderer()->optCurrent().pOptionWrite(name);

	RtInt i;
	for(i=0; i<count; i++)
	{
		RtToken		token=tokens[i];
		RtPointer	value=values[i];

		// Search for the parameter in the declarations.
		// Note Options can only be uniform.
		SqParameterDeclaration Decl=pCurrentRenderer()->FindParameterDecl(token);
		RtInt Type=Decl.m_Type;
		CqParameter* pParam=pOpt->pParameter(token);
		if(pParam==0)
		{
			if(Decl.m_strName!="" && (Decl.m_Type&Storage_Mask)==Type_Uniform)
			{
				pParam=Decl.m_pCreate(token, Decl.m_Count);
				pOpt->AddParameter(pParam);
			}
			else
			{
				if(Decl.m_strName=="")
					CqBasicError(ErrorID_UnknownSymbol,Severity_Normal,"Unknown Symbol");
				else
					CqBasicError(ErrorID_InvalidType,Severity_Normal,"Options can only be uniform");
				return(0);
			}
		}
		else
			Type=pParam->Type();

		switch(Type&Type_Mask)
		{
			case Type_Float:
			{
				RtFloat* pf=reinterpret_cast<RtFloat*>(value);
				if(Type&Type_Array)
				{
					RtInt j;
					for(j=0; j<pParam->Count(); j++)
						static_cast<CqParameterTypedUniformArray<RtFloat,Type_Float>*>(pParam)->pValue()[j]=pf[j];
				}
				else
					static_cast<CqParameterTypedUniform<RtFloat,Type_Float>*>(pParam)->pValue()[0]=pf[0];
			}
			break;

			case Type_Integer:
			{
				RtInt* pi=reinterpret_cast<RtInt*>(value);
				if(Type&Type_Array)
				{
					RtInt j;
					for(j=0; j<pParam->Count(); j++)
						static_cast<CqParameterTypedUniformArray<RtInt,Type_Integer>*>(pParam)->pValue()[j]=pi[j];
				}
				else
					static_cast<CqParameterTypedUniform<RtInt,Type_Integer>*>(pParam)->pValue()[0]=pi[0];
			}
			break;

			case Type_String:
			{
				char** ps=reinterpret_cast<char**>(value);
				if(Type&Type_Array)
				{
					RtInt j;
					for(j=0; j<pParam->Count(); j++)
					{
						CqString str("");
						if(strcmp(name,"searchpath")==0)
						{
							// Get the old value for use in escape replacement
							CqString str_old=static_cast<CqParameterTypedUniform<CqString,Type_UniformString>*>(pParam)->pValue()[0];
							// Build the string, checking for & character and replace with old string.
							unsigned int strt=0;
							unsigned int len=0;
							while(1)
							{
								if((len=strcspn(&ps[j][strt],"&"))<strlen(&ps[j][strt]))
								{
									str+=CqString(ps[j]).substr(strt,len);
									str+=str_old;
									strt+=len+1;
								}
								else
								{
									str+=CqString(ps[j]).substr(strt);
									break;
								}
							}
						}
						else
							str=CqString(ps[j]);
								
						static_cast<CqParameterTypedUniformArray<CqString,Type_String>*>(pParam)->pValue()[j]=str;
					}
				}
				else
				{
					CqString str("");
					if(strcmp(name,"searchpath")==0)
					{
						// Get the old value for use in escape replacement
						CqString str_old=static_cast<CqParameterTypedUniform<CqString,Type_UniformString>*>(pParam)->pValue()[0];
						// Build the string, checking for & character and replace with old string.
						unsigned int strt=0;
						unsigned int len=0;
						while(1)
						{
							if((len=strcspn(&ps[0][strt],"&"))<strlen(&ps[0][strt]))
							{
								str+=CqString(ps[0]).substr(strt,len);
								str+=str_old;
								strt+=len+1;
							}
							else
							{
								str+=CqString(ps[0]).substr(strt);
								break;
							}
						}
					}
					else
						str=CqString(ps[0]);
							
					static_cast<CqParameterTypedUniform<CqString,Type_String>*>(pParam)->pValue()[0]=str;
				}
			}
			break;
			// TODO: Rest of parameter types.
		}
	}
	return(0);
}


//----------------------------------------------------------------------
// RiAttributeBegin
// Begin a ne attribute definition, pushes the current attributes.
//
RtVoid	RiAttributeBegin()
{
	pCurrentRenderer()->CreateAttributeContext();
	// Call the renderer callback, which can be overridden for system specific setup.	
	pCurrentRenderer()->OnAttributeBegin();
	return(0);
}


//----------------------------------------------------------------------
// RiAttributeEnd
// End the current attribute defintion, pops the previous attributes.
//
RtVoid	RiAttributeEnd()
{
	pCurrentRenderer()->DeleteAttributeContext();

	// Call the renderer callback, which can be overridden for system specific setup.	
	pCurrentRenderer()->OnAttributeEnd();

	return(0);
}


//----------------------------------------------------------------------
// RiColor
//	Set the current color for use by the geometric primitives.
//
RtVoid	RiColor(RtColor Cq)
{
	pCurrentRenderer()->pattrWriteCurrent()->SetcolColor(CqColor(Cq),pCurrentRenderer()->Time());
	pCurrentRenderer()->AdvanceTime();
	return(0);
}


//----------------------------------------------------------------------
// RiOpacity
// Set the current opacity, for use by the geometric primitives.
//
RtVoid	RiOpacity(RtColor Os)
{
	pCurrentRenderer()->pattrWriteCurrent()->SetcolOpacity(CqColor(Os),pCurrentRenderer()->Time());
	pCurrentRenderer()->AdvanceTime();
	return(0);
}


//----------------------------------------------------------------------
// RiTextureCoordinates
// Set the current texture coordinates used by the parametric geometric primitives.
//
RtVoid	RiTextureCoordinates(RtFloat s1, RtFloat t1, 
							 RtFloat s2, RtFloat t2, 
							 RtFloat s3, RtFloat t3, 
							 RtFloat s4, RtFloat t4)
{
	pCurrentRenderer()->pattrWriteCurrent()->aTextureCoordinates(pCurrentRenderer()->Time())[0]=CqVector2D(s1,t1);
	pCurrentRenderer()->pattrWriteCurrent()->aTextureCoordinates(pCurrentRenderer()->Time())[1]=CqVector2D(s2,t2);
	pCurrentRenderer()->pattrWriteCurrent()->aTextureCoordinates(pCurrentRenderer()->Time())[2]=CqVector2D(s3,t3);
	pCurrentRenderer()->pattrWriteCurrent()->aTextureCoordinates(pCurrentRenderer()->Time())[3]=CqVector2D(s4,t4);
	pCurrentRenderer()->AdvanceTime();

	return(0);
}


//----------------------------------------------------------------------
// RiLightSource
// Create a new light source at the current transformation.
//
RtLightHandle	RiLightSource(const char *name, ...)
{
	va_list	pArgs;
	va_start(pArgs, name);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiLightSourceV(name, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiLightSourceV
// List based version of above.
//
RtLightHandle	RiLightSourceV(const char *name, PARAMETERLIST)
{
	// Find the lightsource shader.
	CqShader* pShader=static_cast<CqShader*>(CqShader::CreateShader(name, Type_Lightsource));

	// TODO: Report error.
	if(pShader==0)	return(0);

	pShader->matCurrent()=pCurrentRenderer()->matCurrent();
	CqLightsource* pNew=new CqLightsource(pShader,RI_TRUE);

	// Execute the intiialisation code here, as we now have our shader context complete.
	pShader->PrepareDefArgs();

	if(pNew!=0)
	{	
		RtInt i;
		for(i=0; i<count; i++)
		{
			RtToken		token=tokens[i];
			RtPointer	value=values[i];

			pShader->SetValue(token,value);
		}
		pCurrentRenderer()->pattrWriteCurrent()->AddLightsource(pNew);
		return(reinterpret_cast<RtLightHandle>(pNew));
	}
	return(0);
}


//----------------------------------------------------------------------
// RiAreaLightSource
// Create a new area light source at the current transformation, all 
// geometric primitives until the next RiAttributeEnd, become part of this 
// area light source.
//
 RtLightHandle	RiAreaLightSource(const char *name, ...)
{
	va_list	pArgs;
	va_start(pArgs, name);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiAreaLightSourceV(name, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiAreaLightSourceV
// List based version of above.
//
 RtLightHandle	RiAreaLightSourceV(const char *name, PARAMETERLIST)
{
	CqBasicError(0,Severity_Normal,"RiAreaLightSource not supported, creating a point lightsource");
	return(RiLightSourceV(name, count, tokens, values));
}


//----------------------------------------------------------------------
// RiIlluminate
// Set the current status of the specified light source.
//
RtVoid	RiIlluminate(RtLightHandle light, RtBoolean onoff)
{
	// Check if we are turning the light on or off.
	if(light==NULL)	return(0);
	if(onoff)
		pCurrentRenderer()->pattrWriteCurrent()->AddLightsource(reinterpret_cast<CqLightsource*>(light));
	else
		pCurrentRenderer()->pattrWriteCurrent()->RemoveLightsource(reinterpret_cast<CqLightsource*>(light));
	return(0);
}


//----------------------------------------------------------------------
// RiSurface
// Set the current surface shader, used by geometric primitives.
//
RtVoid	RiSurface(const char *name, ...)
{
	va_list	pArgs;
	va_start(pArgs, name);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiSurfaceV(name, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiSurfaceV
// List based version of above.
//
RtVoid	RiSurfaceV(const char *name, PARAMETERLIST)
{
	// Find the shader.
	CqShader* pshadSurface=CqShader::CreateShader(name, Type_Surface);

	if(pshadSurface!=0)
	{
		pshadSurface->matCurrent()=pCurrentRenderer()->matCurrent();
		// Execute the intiialisation code here, as we now have our shader context complete.
		pshadSurface->PrepareDefArgs();
		RtInt i;
		for(i=0; i<count; i++)
		{
			RtToken		token=tokens[i];
			RtPointer	value=values[i];

			pshadSurface->SetValue(token,value);
		}
		pCurrentRenderer()->pattrWriteCurrent()->SetpshadSurface(pshadSurface,pCurrentRenderer()->Time());
	}
	pCurrentRenderer()->AdvanceTime();
	return(0);
}


//----------------------------------------------------------------------
// RiAtmosphere
// Set the current atrmospheric shader.
//
RtVoid	RiAtmosphere(const char *name, ...)
{
	va_list	pArgs;
	va_start(pArgs, name);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiAtmosphereV(name, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiAtmosphereV
// List based version of above.
//
RtVoid	RiAtmosphereV(const char *name, PARAMETERLIST)
{
	// Find the shader.
	CqShader* pshadAtmosphere=CqShader::CreateShader(name, Type_Volume);

	if(pshadAtmosphere!=0)
	{
		pshadAtmosphere->matCurrent()=pCurrentRenderer()->matCurrent();
		// Execute the intiialisation code here, as we now have our shader context complete.
		pshadAtmosphere->PrepareDefArgs();
		RtInt i;
		for(i=0; i<count; i++)
		{
			RtToken		token=tokens[i];
			RtPointer	value=values[i];

			pshadAtmosphere->SetValue(token,value);
		}
	}

	pCurrentRenderer()->pattrWriteCurrent()->SetpshadAtmosphere(pshadAtmosphere,pCurrentRenderer()->Time());
	pCurrentRenderer()->AdvanceTime();
	return(0);
}


//----------------------------------------------------------------------
// RiInterior
// Set the current interior volumetric shader.
//
RtVoid	RiInterior(const char *name, ...)
{
	CqBasicError(0,Severity_Normal,"RiInterior shaders not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiInteriorV
// List based version of above.
//
RtVoid	RiInteriorV(const char *name, PARAMETERLIST)
{
	CqBasicError(0,Severity_Normal,"RiInterior shaders not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiExterior
// Set the current exterior volumetric shader.
//
RtVoid	RiExterior(const char *name, ...)
{
	CqBasicError(0,Severity_Normal,"RiExterior shaders not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiExteriorV
// List based version of above.
//
RtVoid	RiExteriorV(const char *name, PARAMETERLIST)
{
	CqBasicError(0,Severity_Normal,"RiExterior shaders not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiShadingRate
// Specify the size of the shading area in pixels.
//
RtVoid	RiShadingRate(RtFloat size)
{
	pCurrentRenderer()->pattrWriteCurrent()->SetfEffectiveShadingRate(size,pCurrentRenderer()->Time());
	pCurrentRenderer()->AdvanceTime();

	return(0);
}


//----------------------------------------------------------------------
// RiShadingInterpolation
// Specify the method of shading interpolation.
//
RtVoid	RiShadingInterpolation(RtToken type)
{
	if(strcmp(type, RI_CONSTANT)==0)
		pCurrentRenderer()->pattrWriteCurrent()->SeteShadingInterpolation(CqShadingAttributes::ShadingConstant,pCurrentRenderer()->Time());
	else
		if(strcmp(type, RI_SMOOTH)==0)
			pCurrentRenderer()->pattrWriteCurrent()->SeteShadingInterpolation(CqShadingAttributes::ShadingSmooth,pCurrentRenderer()->Time());
		else
			CqBasicError(ErrorID_InvalidData,Severity_Normal,"Invald shading interpolation");

	pCurrentRenderer()->AdvanceTime();
	return(0);
}


//----------------------------------------------------------------------
// RiMatte
// Set the matte state of subsequent geometric primitives.
//
RtVoid	RiMatte(RtBoolean onoff)
{
	CqBasicError(0,Severity_Normal,"RiMatte not supported");
	pCurrentRenderer()->pattrWriteCurrent()->SetbMatteSurfaceFlag(onoff,pCurrentRenderer()->Time());
	pCurrentRenderer()->AdvanceTime();
	return(0);
}


//----------------------------------------------------------------------
// RiBound
// Set the bounding cube of the current primitives.
//
RtVoid	RiBound(RtBound bound)
{
	pCurrentRenderer()->pattrWriteCurrent()->SetBound(CqBound(bound));
	pCurrentRenderer()->AdvanceTime();

	return(0);
}


//----------------------------------------------------------------------
// RiDetail
// Set the current bounding cube for use by level of detail calculation.
//
RtVoid	RiDetail(RtBound bound)
{
	CqBasicError(0,Severity_Normal,"RiDetail not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiDetailRange
// Set the visible range of any subsequent geometric primitives.
//
RtVoid	RiDetailRange(RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh)
{
	CqBasicError(0,Severity_Normal,"RiDetailRange not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiGeometricApproximation
// Specify any parameters used by approximation functions during rendering.
//
RtVoid	RiGeometricApproximation(RtToken type, RtFloat value)
{
	CqBasicError(0,Severity_Normal,"RiGeometricApproximation not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiOrientation
// Set the handedness of any subsequent geometric primitives.
//
RtVoid	RiOrientation(RtToken orientation)
{
	if(orientation!=0)
	{
		if(strstr(orientation, RI_LH)!=0)
			pCurrentRenderer()->pattrWriteCurrent()->SeteOrientation(OrientationLH,pCurrentRenderer()->Time());
		if(strstr(orientation, RI_RH)!=0)
			pCurrentRenderer()->pattrWriteCurrent()->SeteOrientation(OrientationRH,pCurrentRenderer()->Time());
		if(strstr(orientation, RI_INSIDE)!=0)
		{
			pCurrentRenderer()->pattrWriteCurrent()->SeteOrientation(pCurrentRenderer()->pattrCurrent()->eCoordsysOrientation(),pCurrentRenderer()->Time());
			pCurrentRenderer()->pattrWriteCurrent()->FlipeOrientation();
		}
		if(strstr(orientation, RI_OUTSIDE)!=0)
			pCurrentRenderer()->pattrWriteCurrent()->SeteOrientation(pCurrentRenderer()->pattrCurrent()->eCoordsysOrientation(),pCurrentRenderer()->Time());
	}
	pCurrentRenderer()->AdvanceTime();
	return(0);
}


//----------------------------------------------------------------------
// RiReverseOrientation
// Reverse the handedness of any subsequent geometric primitives.
//
RtVoid	RiReverseOrientation()
{
	pCurrentRenderer()->pattrWriteCurrent()->FlipeOrientation(pCurrentRenderer()->Time());
	pCurrentRenderer()->AdvanceTime();
	return(0);
}


//----------------------------------------------------------------------
// RiSides
// Set the number of visibles sides for any subsequent geometric primitives.
//
RtVoid	RiSides(RtInt nsides)
{
	pCurrentRenderer()->pattrWriteCurrent()->SetiNumberOfSides(nsides,pCurrentRenderer()->Time());
	pCurrentRenderer()->AdvanceTime();

	return(0);
}


//----------------------------------------------------------------------
// RiIdentity
// Set the current transformation to the identity matrix.
//
RtVoid	RiIdentity()
{
	pCurrentRenderer()->ptransWriteCurrent()->SetCurrentTransform(pCurrentRenderer()->Time(),CqMatrix());
	pCurrentRenderer()->AdvanceTime();
	return(0);
}


//----------------------------------------------------------------------
// Set the current transformation to the specified matrix.
//
RtVoid	RiTransform(RtMatrix transform)
{
	// TODO: Determine if this matrix requires a change in orientation.
	pCurrentRenderer()->ptransWriteCurrent()->SetCurrentTransform(pCurrentRenderer()->Time(),CqMatrix(transform));
	pCurrentRenderer()->AdvanceTime();

	return(0);
}


//----------------------------------------------------------------------
// RoConcatTransform
// Concatenate the specified matrix into the current transformation matrix.
//
RtVoid	RiConcatTransform(RtMatrix transform)
{
	// Check if this transformation results in a change in orientation.
	CqMatrix matTrans(transform);
	if(matTrans.Determinant()<0)
	{
		pCurrentRenderer()->pattrWriteCurrent()->FlipeOrientation(pCurrentRenderer()->Time());
		pCurrentRenderer()->pattrWriteCurrent()->FlipeCoordsysOrientation(pCurrentRenderer()->Time());
	}

	pCurrentRenderer()->ptransWriteCurrent()->ConcatCurrentTransform(pCurrentRenderer()->Time(),CqMatrix(transform));
	pCurrentRenderer()->AdvanceTime();
	return(0);
}


//----------------------------------------------------------------------
// RiPerspective
// Concatenate a perspective transformation into the current transformation.
//
RtVoid	RiPerspective(RtFloat fov)
{
	CqBasicError(0,Severity_Normal,"RiPerspective not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiTranslate
// Concatenate a translation into the current transformation.
//
RtVoid	RiTranslate(RtFloat dx, RtFloat dy, RtFloat dz)
{
	CqMatrix	matTrans(CqVector3D(dx,dy,dz));
	// Check if this transformation results in a change in orientation.
	if(matTrans.Determinant()<0)
	{
		pCurrentRenderer()->pattrWriteCurrent()->FlipeOrientation(pCurrentRenderer()->Time());
		pCurrentRenderer()->pattrWriteCurrent()->FlipeCoordsysOrientation(pCurrentRenderer()->Time());
	}

	pCurrentRenderer()->ptransWriteCurrent()->ConcatCurrentTransform(pCurrentRenderer()->Time(),matTrans);
	pCurrentRenderer()->AdvanceTime();

	return(0);
}


//----------------------------------------------------------------------
// RiRotate
// Concatenate a rotation into the current transformation.
//
RtVoid	RiRotate(RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz)
{
	CqMatrix	matRot(RAD(angle), CqVector4D(dx,dy,dz));
	// Check if this transformation results in a change in orientation.
	if(matRot.Determinant()<0)
	{
		pCurrentRenderer()->pattrWriteCurrent()->FlipeOrientation(pCurrentRenderer()->Time());
		pCurrentRenderer()->pattrWriteCurrent()->FlipeCoordsysOrientation(pCurrentRenderer()->Time());
	}

	pCurrentRenderer()->ptransWriteCurrent()->ConcatCurrentTransform(pCurrentRenderer()->Time(),matRot);
	pCurrentRenderer()->AdvanceTime();
	return(0);
}


//----------------------------------------------------------------------
// RiScale
// Concatenate a scale into the current transformation.
//
RtVoid	RiScale(RtFloat sx, RtFloat sy, RtFloat sz)
{
	CqMatrix	matScale(sx,sy,sz);
	// Check if this transformation results in a change in orientation.
	if(matScale.Determinant()<0)
	{
		pCurrentRenderer()->pattrWriteCurrent()->FlipeOrientation(pCurrentRenderer()->Time());
		pCurrentRenderer()->pattrWriteCurrent()->FlipeCoordsysOrientation(pCurrentRenderer()->Time());
	}

	pCurrentRenderer()->ptransWriteCurrent()->ConcatCurrentTransform(pCurrentRenderer()->Time(),matScale);
	pCurrentRenderer()->AdvanceTime();
	return(0);
}


//----------------------------------------------------------------------
// RiSkew
// Concatenate a skew into the current transformation.
//
RtVoid	RiSkew(RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1,
					   RtFloat dx2, RtFloat dy2, RtFloat dz2)
{
	CqBasicError(0,Severity_Normal,"RiSkew not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiDeformation
// Specify a deformation shader to be included into the current transformation.
//
RtVoid	RiDeformation(const char *name, ...)
{
	CqBasicError(0,Severity_Normal,"RiDeformation shaders not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiDeformationV
// List based version of above.
//
RtVoid	RiDeformationV(const char *name, PARAMETERLIST)
{
	CqBasicError(0,Severity_Normal,"RiDeformation shaders not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiDisplacement
// Specify the current displacement shade used by geometric primitives.
//
RtVoid	RiDisplacement(const char *name, ...)
{
	va_list	pArgs;
	va_start(pArgs, name);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiDisplacementV(name, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiDisplacementV
// List based version of above.
//
RtVoid	RiDisplacementV(const char *name, PARAMETERLIST)
{
	// Find the shader.
	CqShader* pshadDisplacement=CqShader::CreateShader(name, Type_Displacement);

	if(pshadDisplacement!=0)
	{
		pshadDisplacement->matCurrent()=pCurrentRenderer()->matCurrent();
		// Execute the intiialisation code here, as we now have our shader context complete.
		pshadDisplacement->PrepareDefArgs();
		RtInt i;
		for(i=0; i<count; i++)
		{
			RtToken		token=tokens[i];
			RtPointer	value=values[i];

			pshadDisplacement->SetValue(token,value);
		}
	}

	pCurrentRenderer()->pattrWriteCurrent()->SetpshadDisplacement(pshadDisplacement,pCurrentRenderer()->Time());
	pCurrentRenderer()->AdvanceTime();
	return(0);
}


//----------------------------------------------------------------------
// RiCoordinateSystem
// Save the current coordinate system as the specified name.
//
RtVoid	RiCoordinateSystem(RtToken space)
{
	// Insert the named coordinate system into the list help on the renderer.
	pCurrentRenderer()->SetCoordSystem(space,pCurrentRenderer()->matCurrent(pCurrentRenderer()->Time()));
	pCurrentRenderer()->AdvanceTime();

	return(0);
}


//----------------------------------------------------------------------
// ---Additional to spec. v3.1---
// RiCoordSysTransform
// Replace the current transform with the named space.

RtVoid	RiCoordSysTransform(RtToken space)
{
	// Insert the named coordinate system into the list help on the renderer.
	pCurrentRenderer()->ptransWriteCurrent()->SetCurrentTransform(pCurrentRenderer()->Time(),pCurrentRenderer()->matSpaceToSpace(space, "world"));
	pCurrentRenderer()->AdvanceTime();

	return(0);
}


//----------------------------------------------------------------------
// RiTransformPoints
// Transform a list of points from one coordinate system to another.
//
 RtPoint*	RiTransformPoints(RtToken fromspace, RtToken tospace, RtInt npoints, RtPoint points[])
{
	CqBasicError(0,Severity_Normal,"RiTransformPoints not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiTransformBegin
// Push the current transformation state.
//
RtVoid	RiTransformBegin()
{
	pCurrentRenderer()->CreateTransformContext();
	// Call the renderer callback, which can be overridden for system specific setup.	
	pCurrentRenderer()->OnTransformBegin();
	return(0);
}


//----------------------------------------------------------------------
// RiTransformEnd
// Pop the previous transformation state.
//
RtVoid	RiTransformEnd()
{
	pCurrentRenderer()->DeleteTransformContext();

	// Call the renderer callback, which can be overridden for system specific setup.	
	pCurrentRenderer()->OnTransformEnd();

	return(0);
}


//----------------------------------------------------------------------
// RiAttribute
// Set a system specific attribute.
//
RtVoid	RiAttribute(const char *name, ...)
{
	va_list	pArgs;
	va_start(pArgs, name);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiAttributeV(name, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiAttributeV
// List based version of above.
//
RtVoid	RiAttributeV(const char *name, PARAMETERLIST)
{
	// Find the parameter on the current options.
	CqSystemOption* pAttr=pCurrentRenderer()->pattrWriteCurrent()->pAttributeWrite(name);

	RtInt i;
	for(i=0; i<count; i++)
	{
		RtToken		token=tokens[i];
		RtPointer	value=values[i];

		RtInt Type;
		CqParameter* pParam=pAttr->pParameter(token);
		if(pParam==0)
		{
			// Search for the parameter in the declarations.
			// Note attributes can only be uniform.
			SqParameterDeclaration Decl=pCurrentRenderer()->FindParameterDecl(token);
			if(Decl.m_strName!="" && (Decl.m_Type&Storage_Mask)==Type_Uniform)
			{
				pParam=Decl.m_pCreate(Decl.m_strName.c_str(), Decl.m_Count);
				Type=Decl.m_Type;
				pAttr->AddParameter(pParam);
			}
			else
			{
				if(Decl.m_strName=="")
					CqBasicError(ErrorID_UnknownSymbol,Severity_Normal,"Unknown Symbol");
				else
					CqBasicError(ErrorID_InvalidType,Severity_Normal,"Attributes can only be uniform");
				return(0);
			}
		}
		else
			Type=pParam->Type();

		switch(Type&Type_Mask)
		{
			case Type_Float:
			{
				RtFloat* pf=reinterpret_cast<RtFloat*>(value);
				if(Type&Type_Array)
				{
					RtInt j;
					for(j=0; j<pParam->Count(); j++)
						static_cast<CqParameterTypedUniformArray<RtFloat,Type_Float>*>(pParam)->pValue()[j]=pf[j];
				}
				else
					static_cast<CqParameterTypedUniform<RtFloat,Type_Float>*>(pParam)->pValue()[0]=pf[0];
			}
			break;

			case Type_Integer:
			{
				RtInt* pi=reinterpret_cast<RtInt*>(value);
				if(Type&Type_Array)
				{
					RtInt j;
					for(j=0; j<pParam->Count(); j++)
						static_cast<CqParameterTypedUniformArray<RtInt,Type_Integer>*>(pParam)->pValue()[j]=pi[j];
				}
				else
					static_cast<CqParameterTypedUniform<RtInt,Type_Integer>*>(pParam)->pValue()[0]=pi[0];
			}
			break;

			case Type_String:
			{
				char** ps=reinterpret_cast<char**>(value);
				if(Type&Type_Array)
				{
					RtInt j;
					for(j=0; j<pParam->Count(); j++)
					{
						CqString str(ps[j]);
						static_cast<CqParameterTypedUniform<CqString,Type_String>*>(pParam)->pValue()[j]=str;
					}
				}
				else
				{
					CqString str(ps[0]);
					static_cast<CqParameterTypedUniform<CqString,Type_String>*>(pParam)->pValue()[0]=str;
				}
			}
			// TODO: Rest of parameter types.
		}
	}
	return(0);
}


//----------------------------------------------------------------------
// RiPolygon
// Specify a coplanar, convex polygon.
//
RtVoid	RiPolygon(RtInt nvertices, ...)
{
	va_list	pArgs;
	va_start(pArgs, nvertices);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiPolygonV(nvertices, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiPolygonV
// List based version of above.
//
RtVoid	RiPolygonV(RtInt nvertices, PARAMETERLIST)
{
	CqMotionSurface<CqSurfacePolygon*>* pMotionSurface=new CqMotionSurface<CqSurfacePolygon*>(0);
	RtInt i;
	for(i=0; i<pCurrentRenderer()->ptransCurrent()->cTimes(); i++)
	{
		RtFloat time=pCurrentRenderer()->ptransCurrent()->Time(i);
		// Create a new polygon surface primitive.
		CqSurfacePolygon*	pSurface=new CqSurfacePolygon(nvertices);

		// Process any specified primitive variables.
		pSurface->SetDefaultPrimitiveVariables(RI_FALSE); 
		if(ProcessPrimitiveVariables(pSurface,count,tokens,values))
		{
			if(!pSurface->CheckDegenerate())
			{
				// Check if s/t, u/v are needed and not specified, and if so get them from the object space points.
				pSurface->TransferDefaultSurfaceParameters();

				// Transform the points into "current" space,
				pSurface->Transform(pCurrentRenderer()->matSpaceToSpace("object","camera",CqMatrix(),pSurface->pTransform()->matObjectToWorld()),
									pCurrentRenderer()->matNSpaceToSpace("object","camera",CqMatrix(),pSurface->pTransform()->matObjectToWorld()),
									pCurrentRenderer()->matVSpaceToSpace("object","camera",CqMatrix(),pSurface->pTransform()->matObjectToWorld()));
			}
			else
			{
				CqBasicError(ErrorID_InvalidData,Severity_Normal,"Degenerate polygon found");
				delete(pSurface);
			}	
			pMotionSurface->AddTimeSlot(time,pSurface);
		}
		else
			delete(pSurface);
	}
	pCurrentRenderer()->Scene().AddSurface(pMotionSurface);

	return(0);
}


//----------------------------------------------------------------------
// RiGeneralPolygon
// Specify a nonconvex coplanar polygon.
//
RtVoid	RiGeneralPolygon(RtInt nloops, RtInt nverts[], ...)
{
	va_list	pArgs;
	va_start(pArgs, nverts);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiGeneralPolygonV(nloops, nverts, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiGeneralPolygonV
// List based version of above.
//
RtVoid	RiGeneralPolygonV(RtInt nloops, RtInt nverts[], PARAMETERLIST)
{
	CqBasicError(0,Severity_Normal,"RiGeneralPolygon not supported");
	RiPolygonV(nverts[0],count,tokens,values);
	return(0);
}


//----------------------------------------------------------------------
// RiPointsPolygons
// Specify a list of convex coplanar polygons and their shared vertices.
//
RtVoid	RiPointsPolygons(RtInt npolys, RtInt nverts[], RtInt verts[], ...)
{
	va_list	pArgs;
	va_start(pArgs, verts);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiPointsPolygonsV(npolys, nverts, verts, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiPointsPolygonsV
// List based version of above.
//


RtVoid	RiPointsPolygonsV(RtInt npolys, RtInt nverts[], RtInt verts[], PARAMETERLIST)
{
	// Calculate how many vertices there are.
	RtInt cVerts=0;
	RtInt* pVerts=verts;
	RtInt poly;
	for(poly=0; poly<npolys; poly++)
	{
		RtInt v;
		for(v=0; v<nverts[poly]; v++)
		{
			if(((*pVerts)+1)>cVerts)
				cVerts=(*pVerts)+1;
			pVerts++;
		}
	}

	// Create a storage class for all the points.
	CqPolygonPoints* pPointsClass=new CqPolygonPoints(cVerts);
	
	// Process any specified primitive variables
	pPointsClass->SetDefaultPrimitiveVariables(RI_FALSE); 
	if(ProcessPrimitiveVariables(pPointsClass,count,tokens,values))
	{
		// Check if s/t, u/v are needed and not specified, and if so get them from the object space points.
		pPointsClass->TransferDefaultSurfaceParameters();

		// Transform the points into "current" space,
		pPointsClass->Transform(pCurrentRenderer()->matSpaceToSpace("object","camera",CqMatrix(),pPointsClass->pTransform()->matObjectToWorld()),
								pCurrentRenderer()->matNSpaceToSpace("object","camera",CqMatrix(),pPointsClass->pTransform()->matObjectToWorld()),
								pCurrentRenderer()->matVSpaceToSpace("object","camera",CqMatrix(),pPointsClass->pTransform()->matObjectToWorld()));
		
		// For each polygon specified create a primitive.
		RtInt	iP=0;
		for(poly=0; poly<npolys; poly++)
		{
			// Create a surface polygon
			CqSurfacePointsPolygon*	pSurface=new CqSurfacePointsPolygon(pPointsClass);
			RtBoolean fValid=RI_TRUE;

			pSurface->aIndices().resize(nverts[poly]);
			RtInt i;
			for(i=0; i<nverts[poly]; i++)	// Fill in the points
			{
				if(verts[iP]>=cVerts)
				{
					fValid=RI_FALSE;
					CqAttributeError(1, Severity_Normal,"Invalid PointsPolygon index", pSurface->pAttributes());
					break;
				}
				pSurface->aIndices()[i]=verts[iP];
				iP++;
			}
			if(fValid)
				pCurrentRenderer()->Scene().AddSurface(pSurface);
		}
	}
	else
		delete(pPointsClass);

	return(0);
}


//----------------------------------------------------------------------
// RiPointsGeneralPolygons
// Specify a list of coplanar, non-convex polygons and their shared vertices.
//
RtVoid	RiPointsGeneralPolygons(RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], ...)
{
	CqBasicError(0,Severity_Normal,"RiPointsGeneralPolygons not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiPointsGeneralPolygonsV
// List based version of above.
//
RtVoid	RiPointsGeneralPolygonsV(RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], PARAMETERLIST)
{
	CqBasicError(0,Severity_Normal,"RiPointsGeneralPolygons not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiBasis
// Specify the patch basis matrices for the u and v directions, and the knot skip values.
//
RtVoid	RiBasis(RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep)
{
	CqMatrix u;
	CqMatrix v;

	RtInt i;
	for(i=0; i<4; i++)
	{
		RtInt j;
		for(j=0; j<4; j++)
		{
			u.SetElement(i,j,ubasis[i][j]);
			v.SetElement(i,j,vbasis[i][j]);
		}
	}

	pCurrentRenderer()->pattrWriteCurrent()->SetmatuBasis(u,pCurrentRenderer()->Time());
	pCurrentRenderer()->pattrWriteCurrent()->SetmatvBasis(v,pCurrentRenderer()->Time());
	pCurrentRenderer()->pattrWriteCurrent()->SetuSteps(ustep,pCurrentRenderer()->Time());
	pCurrentRenderer()->pattrWriteCurrent()->SetvSteps(vstep,pCurrentRenderer()->Time());
	pCurrentRenderer()->AdvanceTime();
	return(0);
}


//----------------------------------------------------------------------
// RiPatch
// Specify a new patch primitive.
//
RtVoid	RiPatch(RtToken type, ...)
{
	va_list	pArgs;
	va_start(pArgs, type);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiPatchV(type, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiPatchV
// List based version of above.
//
RtVoid	RiPatchV(RtToken type, PARAMETERLIST)
{
	if(strcmp(type, RI_BICUBIC)==0)
	{
		// Create a surface patch
		CqSurfacePatchBicubic*	pSurface=new CqSurfacePatchBicubic();
		// Fill in default values for all primitive variables not explicitly specified.
		pSurface->SetDefaultPrimitiveVariables(); 
		// Fill in primitive variables specified.
		if(ProcessPrimitiveVariables(pSurface, count, tokens, values))
			CreateGPrim(pSurface);
		else
			delete(pSurface);
	}
	else if(strcmp(type, RI_BILINEAR)==0)
	{
		// Create a surface patch
		CqSurfacePatchBilinear*	pSurface=new CqSurfacePatchBilinear();
		// Fill in default values for all primitive variables not explicitly specified.
		pSurface->SetDefaultPrimitiveVariables(); 
		// Fill in primitive variables specified.
		if(ProcessPrimitiveVariables(pSurface, count, tokens, values))
			CreateGPrim(pSurface);
		else
			delete(pSurface);
	}
	return(0);
}


//----------------------------------------------------------------------
// RiPatchMesh
// Specify a quadrilaterla mesh of patches.
//
RtVoid	RiPatchMesh(RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, ...)
{
	va_list	pArgs;
	va_start(pArgs, vwrap);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiPatchMeshV(type, nu, uwrap, nv, vwrap, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiPatchMeshV
// List based version of above.
//

#define	PatchCoord(v,u)	((((v)%nv)*nu)+((u)%nu))
#define	PatchCorner(v,u)	((((v)%nvaryingv)*nvaryingu)+((u)%nvaryingu));

RtVoid	RiPatchMeshV(RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, PARAMETERLIST)
{
	RtInt	fP=RIL_NONE;
	RtInt	fN=RIL_NONE;
	RtInt	fT=RIL_NONE;
	RtBoolean	fCs=RI_FALSE;
	RtBoolean	fOs=RI_FALSE;

	RtFloat*	pPoints=0;
	RtFloat*	pNormals=0;
	RtFloat*	pTextures_s=0,*pTextures_t=0,*pTextures_st=0;
	RtFloat*	pCs=0;
	RtFloat*	pOs=0;

	RtInt i;
	for(i=0; i<count; i++)
	{
		RtToken		token=tokens[i];
		RtPointer	value=values[i];

		if(strcmp(token, RI_P)==0)
		{
			fP=RIL_P;
			pPoints=(RtFloat*)value;
		}
		else if(strcmp(token, RI_PZ)==0)
		{
			fP=RIL_Pz;
			pPoints=(RtFloat*)value;
		}
		else if(strcmp(token, RI_PW)==0)
		{
			fP=RIL_Pw;
			pPoints=(RtFloat*)value;
		}
		else if(strcmp(token, RI_S)==0)
		{
			fT=RIL_s;
			pTextures_s=(RtFloat*)value;
		}
		else if(strcmp(token, RI_T)==0)
		{
			fT=RIL_t;
			pTextures_t=(RtFloat*)value;
		}
		else if(strcmp(token, RI_ST)==0)
		{
			fT=RIL_st;
			pTextures_st=(RtFloat*)value;
		}
		else if(strcmp(token, RI_CS)==0)
		{
			fCs=RI_TRUE;
			pCs=(RtFloat*)value;
		}
		else if(strcmp(token, RI_OS)==0)
		{
			fOs=RI_TRUE;
			pOs=(RtFloat*)value;
		}
	}
		
	// Check that at least points have been defined.
	if(fP==RIL_NONE)
		return(0);

	if(strcmp(type, RI_BICUBIC)==0)
	{
		// Create a surface patch
		RtBoolean	uPeriodic=(strcmp(uwrap,RI_PERIODIC)==0)?RI_TRUE:RI_FALSE;
		RtBoolean	vPeriodic=(strcmp(vwrap,RI_PERIODIC)==0)?RI_TRUE:RI_FALSE;

		CqVector4D vecPoint;
		RtInt iP=0;
		RtInt uStep=pCurrentRenderer()->pattrCurrent()->uSteps();
		RtInt vStep=pCurrentRenderer()->pattrCurrent()->vSteps();
		
		RtInt upatches=(uPeriodic)?nu/uStep:((nu-4)/uStep)+1;
		RtInt vpatches=(vPeriodic)?nv/vStep:((nv-4)/vStep)+1;
		RtInt nvaryingu=(uPeriodic)?upatches:upatches+1;
		RtInt nvaryingv=(vPeriodic)?vpatches:vpatches+1;
		
		RtFloat up,vp;
		RtFloat du=1.0/upatches;
		RtFloat dv=1.0/vpatches;

		CqVector2D st1,st2,st3,st4;
		st1=pCurrentRenderer()->pattrCurrent()->aTextureCoordinates()[0];
		st2=pCurrentRenderer()->pattrCurrent()->aTextureCoordinates()[1];
		st3=pCurrentRenderer()->pattrCurrent()->aTextureCoordinates()[2];
		st4=pCurrentRenderer()->pattrCurrent()->aTextureCoordinates()[3];
		
		vp=0;
		// Fill in the points
		for(i=0; i<vpatches; i++)	
		{
			up=0;
			// vRow is the coordinate row of the mesh.
			RtInt	vRow=i*vStep;
			RtInt j;
			for(j=0; j<upatches; j++)
			{
				// uCol is the coordinate column of the mesh.
				RtInt uCol=j*uStep;
				CqSurfacePatchBicubic*	pSurface=new CqSurfacePatchBicubic();
				pSurface->SetDefaultPrimitiveVariables();
				pSurface->P().SetSize(pSurface->cVertex());
				RtInt v;
				for(v=0; v<4; v++)
				{
					switch(fP)
					{
						case	RIL_P:
						{
							iP=PatchCoord(vRow+v,uCol  )*3;
							pSurface->P()[(v*4)]=CqVector3D(pPoints[iP], pPoints[iP+1], pPoints[iP+2]);
							iP=PatchCoord(vRow+v,uCol+1)*3;
							pSurface->P()[(v*4)+1]=CqVector3D(pPoints[iP], pPoints[iP+1], pPoints[iP+2]);
							iP=PatchCoord(vRow+v,uCol+2)*3;
							pSurface->P()[(v*4)+2]=CqVector3D(pPoints[iP], pPoints[iP+1], pPoints[iP+2]);
							iP=PatchCoord(vRow+v,uCol+3)*3;
							pSurface->P()[(v*4)+3]=CqVector3D(pPoints[iP], pPoints[iP+1], pPoints[iP+2]);
						}
						break;

						case	RIL_Pw:
						{
							iP=PatchCoord(vRow+v,uCol  )*4;
							pSurface->P()[(v*4)]=CqVector4D(pPoints[iP], pPoints[iP+1], pPoints[iP+2], pPoints[iP+3]);
							iP=PatchCoord(vRow+v,uCol+1)*4;
							pSurface->P()[(v*4)+1]=CqVector4D(pPoints[iP+4], pPoints[iP+5], pPoints[iP+6], pPoints[iP+7]);
							iP=PatchCoord(vRow+v,uCol+2)*4;
							pSurface->P()[(v*4)+2]=CqVector4D(pPoints[iP+8], pPoints[iP+9], pPoints[iP+10], pPoints[iP+11]);
							iP=PatchCoord(vRow+v,uCol+3)*4;
							pSurface->P()[(v*4)+3]=CqVector4D(pPoints[iP+12], pPoints[iP+13], pPoints[iP+14], pPoints[iP+15]);
						}
						break;

						case	RIL_Pz:	// TODO: Work out how to parameterize x,y for heightfields.
						default:
						break;
					}
				}
				// Fill in the surface parameters for the mesh.
				if(USES(pSurface->Uses(),EnvVars_u))
				{
					pSurface->u()[0]=up;	
					pSurface->u()[1]=up+du;	
					pSurface->u()[2]=up;	
					pSurface->u()[3]=up+du;	
				}

				if(USES(pSurface->Uses(),EnvVars_v))
				{
					pSurface->v()[0]=vp;
					pSurface->v()[1]=vp;
					pSurface->v()[2]=vp+dv;
					pSurface->v()[3]=vp+dv;
				}

				// Fill in the texture coordinates for the mesh.
				// Use the attribute texture coordinates of none explicitly specified.
				// If any textures coordinates have been sxplicilty specified, they override the
				// attribute level settings.
				RtInt iTa=PatchCorner(i,j);
				RtInt iTb=PatchCorner(i,j+1);
				RtInt iTc=PatchCorner(i+1,j);
				RtInt iTd=PatchCorner(i+1,j+1);
				if(USES(pSurface->Uses(),EnvVars_s))
				{
					pSurface->s()[0]=BilinearEvaluate<RtFloat>(st1.x(),st2.x(),st3.x(),st4.x(),up,vp);
					pSurface->s()[1]=BilinearEvaluate<RtFloat>(st1.x(),st2.x(),st3.x(),st4.x(),up+du,vp);
					pSurface->s()[2]=BilinearEvaluate<RtFloat>(st1.x(),st2.x(),st3.x(),st4.x(),up,vp+dv);
					pSurface->s()[3]=BilinearEvaluate<RtFloat>(st1.x(),st2.x(),st3.x(),st4.x(),up+du,vp+dv);
					switch(fT)
					{
						case RIL_t:
							if(pTextures_s!=0)
							{
								pSurface->s()[0]=pTextures_s[iTa];
								pSurface->s()[1]=pTextures_s[iTb];
								pSurface->s()[2]=pTextures_s[iTc];
								pSurface->s()[3]=pTextures_s[iTd];
							}
							break;

						case RIL_st:
							assert(pTextures_st!=0);
							pSurface->s()[0]=pTextures_st[iTa*2];
							pSurface->s()[1]=pTextures_st[iTb*2];
							pSurface->s()[2]=pTextures_st[iTc*2];
							pSurface->s()[3]=pTextures_st[iTd*2];
							break;
					}
				}

				if(USES(pSurface->Uses(),EnvVars_t))
				{
					pSurface->t()[0]=BilinearEvaluate<RtFloat>(st1.y(),st2.y(),st3.y(),st4.y(),up,vp);
					pSurface->t()[1]=BilinearEvaluate<RtFloat>(st1.y(),st2.y(),st3.y(),st4.y(),up+du,vp);
					pSurface->t()[2]=BilinearEvaluate<RtFloat>(st1.y(),st2.y(),st3.y(),st4.y(),up,vp+dv);
					pSurface->t()[3]=BilinearEvaluate<RtFloat>(st1.y(),st2.y(),st3.y(),st4.y(),up+du,vp+dv);

					switch(fT)
					{
						case RIL_t:
							if(pTextures_t!=0)
							{
								pSurface->t()[0]=pTextures_t[iTa];
								pSurface->t()[1]=pTextures_t[iTb];
								pSurface->t()[2]=pTextures_t[iTc];
								pSurface->t()[3]=pTextures_t[iTd];
							}
							break;

						case RIL_st:
							assert(pTextures_st!=0);
							pSurface->t()[0]=pTextures_st[(iTa*2)+1];
							pSurface->t()[1]=pTextures_st[(iTb*2)+1];
							pSurface->t()[2]=pTextures_st[(iTc*2)+1];
							pSurface->t()[3]=pTextures_st[(iTd*2)+1];
							break;
					}
				}

				if(USES(pSurface->Uses(),EnvVars_Cs) && fCs)
				{
					pSurface->Cs().SetSize(4);
					pSurface->Cs()[0]=CqColor(pCs[(iTa*3)],pCs[(iTa*3)+1],pCs[(iTa*3)+2]);
					pSurface->Cs()[1]=CqColor(pCs[(iTb*3)],pCs[(iTb*3)+1],pCs[(iTb*3)+2]);
					pSurface->Cs()[2]=CqColor(pCs[(iTc*3)],pCs[(iTc*3)+1],pCs[(iTc*3)+2]);
					pSurface->Cs()[3]=CqColor(pCs[(iTd*3)],pCs[(iTd*3)+1],pCs[(iTd*3)+2]);
				}

				if(USES(pSurface->Uses(),EnvVars_Os) && fOs)
				{
					pSurface->Os().SetSize(4);
					pSurface->Os()[0]=CqColor(pOs[(iTa*3)],pOs[(iTa*3)+1],pOs[(iTa*3)+2]);
					pSurface->Os()[1]=CqColor(pOs[(iTb*3)],pOs[(iTb*3)+1],pOs[(iTb*3)+2]);
					pSurface->Os()[2]=CqColor(pOs[(iTc*3)],pOs[(iTc*3)+1],pOs[(iTc*3)+2]);
					pSurface->Os()[3]=CqColor(pOs[(iTd*3)],pOs[(iTd*3)+1],pOs[(iTd*3)+2]);
				}

				up+=du;

				CreateGPrim(pSurface);
			}
			vp+=dv;
		}
	}
	else if(strcmp(type, RI_BILINEAR)==0)
	{
		// Create a surface patch
		RtBoolean	uPeriodic=(strcmp(uwrap,RI_PERIODIC)==0)?RI_TRUE:RI_FALSE;
		RtBoolean	vPeriodic=(strcmp(vwrap,RI_PERIODIC)==0)?RI_TRUE:RI_FALSE;
		CqVector4D vecPoint;
		RtInt iP=0;

		RtInt upatches=(uPeriodic)?nu:nu-1;
		RtInt vpatches=(vPeriodic)?nv:nv-1;
		
		RtFloat up,vp;
		RtFloat du=1.0/upatches;
		RtFloat dv=1.0/vpatches;

		CqVector2D st1,st2,st3,st4;
		st1=pCurrentRenderer()->pattrCurrent()->aTextureCoordinates()[0];
		st2=pCurrentRenderer()->pattrCurrent()->aTextureCoordinates()[1];
		st3=pCurrentRenderer()->pattrCurrent()->aTextureCoordinates()[2];
		st4=pCurrentRenderer()->pattrCurrent()->aTextureCoordinates()[3];
		
		vp=0;
		for(i=0; i<vpatches; i++)	// Fill in the points
		{
			up=0;
			RtInt j;
			for(j=0; j<upatches; j++)
			{
				CqSurfacePatchBilinear*	pSurface=new CqSurfacePatchBilinear();
				pSurface->SetDefaultPrimitiveVariables();
				pSurface->P().SetSize(4);
				switch(fP)
				{
					case	RIL_P:
					{
						// Calculate the position in the point table for u taking into account
						// periodic patches.
						iP=PatchCoord(i,j)*3;
						pSurface->P()[0]=CqVector3D(pPoints[iP   ], pPoints[iP+1 ], pPoints[iP+2 ]);
						iP=PatchCoord(i,j+1)*3;
						pSurface->P()[1]=CqVector3D(pPoints[iP   ], pPoints[iP+1 ], pPoints[iP+2 ]);
						iP=PatchCoord(i+1,j)*3;
						pSurface->P()[2]=CqVector3D(pPoints[iP   ], pPoints[iP+1 ], pPoints[iP+2 ]);
						iP=PatchCoord(i+1,j+1)*3;
						pSurface->P()[3]=CqVector3D(pPoints[iP   ], pPoints[iP+1 ], pPoints[iP+2]);
					}
					break;

					case	RIL_Pw:
					{
						iP=PatchCoord(i,j)*4;
						pSurface->P()[0]=CqVector4D(pPoints[iP   ], pPoints[iP+1 ], pPoints[iP+2 ], pPoints[iP+3 ]);
						iP=PatchCoord(i,j+1)*4;
						pSurface->P()[1]=CqVector4D(pPoints[iP   ], pPoints[iP+1 ], pPoints[iP+2 ], pPoints[iP+3 ]);
						iP=PatchCoord(i+1,j)*4;
						pSurface->P()[2]=CqVector4D(pPoints[iP   ], pPoints[iP+1 ], pPoints[iP+2 ], pPoints[iP+3 ]);
						iP=PatchCoord(i+1,j+1)*4;
						pSurface->P()[3]=CqVector4D(pPoints[iP   ], pPoints[iP+1 ], pPoints[iP+2 ], pPoints[iP+3 ]);
					}
					break;

					case	RIL_Pz:	// TODO: Work out how to parameterize x,y for heightfields.
					default:
					break;
				}
				// Fill in the surface parameters for the mesh.
				if(USES(pSurface->Uses(),EnvVars_u))
				{
					pSurface->u()[0]=up;
					pSurface->u()[1]=up+du;
					pSurface->u()[2]=up;
					pSurface->u()[3]=up+du;
				}

				if(USES(pSurface->Uses(),EnvVars_v))
				{
					pSurface->v()[0]=vp;
					pSurface->v()[1]=vp;
					pSurface->v()[2]=vp+dv;
					pSurface->v()[3]=vp+dv;
				}

				// Fill in the texture coordinates for the mesh.
				// If any textures coordinates have been sxplicilty specified, they override the
				// attribute level settings.
				RtInt iTa=PatchCoord(i,j);
				RtInt iTb=PatchCoord(i,j+1);
				RtInt iTc=PatchCoord(i+1,j);
				RtInt iTd=PatchCoord(i+1,j+1);

				if(USES(pSurface->Uses(),EnvVars_s))
				{
					pSurface->s()[0]=BilinearEvaluate<RtFloat>(st1.x(),st2.x(),st3.x(),st4.x(),up,vp);
					pSurface->s()[1]=BilinearEvaluate<RtFloat>(st1.x(),st2.x(),st3.x(),st4.x(),up+du,vp);
					pSurface->s()[2]=BilinearEvaluate<RtFloat>(st1.x(),st2.x(),st3.x(),st4.x(),up,vp+dv);
					pSurface->s()[3]=BilinearEvaluate<RtFloat>(st1.x(),st2.x(),st3.x(),st4.x(),up+du,vp+dv);

					switch(fT)
					{
						case RIL_t:
							if(pTextures_s!=0)
							{
								pSurface->s()[0]=pTextures_s[iTa];
								pSurface->s()[1]=pTextures_s[iTb];
								pSurface->s()[2]=pTextures_s[iTc];
								pSurface->s()[3]=pTextures_s[iTd];
							}
							break;

						case RIL_st:
							assert(pTextures_st!=0);
							pSurface->s()[0]=pTextures_st[iTa*2];
							pSurface->s()[1]=pTextures_st[iTb*2];
							pSurface->s()[2]=pTextures_st[iTc*2];
							pSurface->s()[3]=pTextures_st[iTd*2];
							break;
					}
				}

				if(USES(pSurface->Uses(),EnvVars_t))
				{
					pSurface->t()[0]=BilinearEvaluate<RtFloat>(st1.y(),st2.y(),st3.y(),st4.y(),up,vp);
					pSurface->t()[1]=BilinearEvaluate<RtFloat>(st1.y(),st2.y(),st3.y(),st4.y(),up+du,vp);
					pSurface->t()[2]=BilinearEvaluate<RtFloat>(st1.y(),st2.y(),st3.y(),st4.y(),up,vp+dv);
					pSurface->t()[3]=BilinearEvaluate<RtFloat>(st1.y(),st2.y(),st3.y(),st4.y(),up+du,vp+dv);

					switch(fT)
					{
						case RIL_t:
							if(pTextures_t!=0)
							{
								pSurface->t()[0]=pTextures_t[iTa];
								pSurface->t()[1]=pTextures_t[iTb];
								pSurface->t()[2]=pTextures_t[iTc];
								pSurface->t()[3]=pTextures_t[iTd];
							}
							break;

						case RIL_st:
							assert(pTextures_st!=0);
							pSurface->t()[0]=pTextures_st[(iTa*2)+1];
							pSurface->t()[1]=pTextures_st[(iTb*2)+1];
							pSurface->t()[2]=pTextures_st[(iTc*2)+1];
							pSurface->t()[3]=pTextures_st[(iTd*2)+1];
							break;
					}
				}

				if(USES(pSurface->Uses(),EnvVars_Cs) && fCs)
				{
					pSurface->Cs().SetSize(4);
					pSurface->Cs()[0]=CqColor(pCs[(iTa*3)],pCs[(iTa*3)+1],pCs[(iTa*3)+2]);
					pSurface->Cs()[1]=CqColor(pCs[(iTb*3)],pCs[(iTb*3)+1],pCs[(iTb*3)+2]);
					pSurface->Cs()[2]=CqColor(pCs[(iTc*3)],pCs[(iTc*3)+1],pCs[(iTc*3)+2]);
					pSurface->Cs()[3]=CqColor(pCs[(iTd*3)],pCs[(iTd*3)+1],pCs[(iTd*3)+2]);
				}

				if(USES(pSurface->Uses(),EnvVars_Os) && fOs)
				{
					pSurface->Os().SetSize(4);
					pSurface->Os()[0]=CqColor(pOs[(iTa*3)],pOs[(iTa*3)+1],pOs[(iTa*3)+2]);
					pSurface->Os()[1]=CqColor(pOs[(iTb*3)],pOs[(iTb*3)+1],pOs[(iTb*3)+2]);
					pSurface->Os()[2]=CqColor(pOs[(iTc*3)],pOs[(iTc*3)+1],pOs[(iTc*3)+2]);
					pSurface->Os()[3]=CqColor(pOs[(iTd*3)],pOs[(iTd*3)+1],pOs[(iTd*3)+2]);
				}

				up+=du;

				CreateGPrim(pSurface);
			}
			vp+=dv;
		}
	}
	return(0);
}


//----------------------------------------------------------------------
// RiNuPatch
// Specify a new non uniform patch.
//
RtVoid	RiNuPatch(RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax, 
				  RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, ...)
{
	va_list	pArgs;
	va_start(pArgs, vmax);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiNuPatchV(nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiNuPatchV
// List based version of above.
//
RtVoid	RiNuPatchV(RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax, 
						   RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, PARAMETERLIST)
{
	// Create a NURBS patch
	CqSurfaceNURBS* pSurface=new CqSurfaceNURBS();
	pSurface->Init(uorder, vorder, nu, nv);

	// Copy the knot vectors.
	RtInt i;
	for(i=0; i<nu+uorder; i++)	pSurface->auKnots()[i]=uknot[i];
	for(i=0; i<nv+vorder; i++)	pSurface->avKnots()[i]=vknot[i];

	// Set up the default primitive variables.
	pSurface->SetDefaultPrimitiveVariables();
	// Process any specified parameters
	if(ProcessPrimitiveVariables(pSurface,count,tokens,values))
	{
		// Clamp the surface to ensure non-periodic.
		pSurface->Clamp();

		// Transform the points into camera space for processing,
		pSurface->Transform(pCurrentRenderer()->matSpaceToSpace("object","camera",CqMatrix(),pSurface->pTransform()->matObjectToWorld()),
							pCurrentRenderer()->matNSpaceToSpace("object","camera",CqMatrix(),pSurface->pTransform()->matObjectToWorld()),
							pCurrentRenderer()->matVSpaceToSpace("object","camera",CqMatrix(),pSurface->pTransform()->matObjectToWorld()));
		pCurrentRenderer()->Scene().AddSurface(pSurface);
	}
	else
		delete(pSurface);

	return(0);
}


//----------------------------------------------------------------------
// RiTrimCurve
// Specify curves which are used to trim NURBS surfaces.
//
RtVoid	RiTrimCurve(RtInt nloops, RtInt ncurves[], RtInt order[], RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[])
{
	CqBasicError(0,Severity_Normal,"RiTrimCurve not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiSphere
// Specify a sphere primitive.
//
RtVoid	RiSphere(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ...)
{
	va_list	pArgs;
	va_start(pArgs, thetamax);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiSphereV(radius, zmin, zmax, thetamax, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiSphereV
// List based version of above.
//
RtVoid	RiSphereV(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST)
{
	// Create a sphere
	CqSphere* pSurface=new CqSphere(radius,zmin,zmax,0,thetamax);
	pSurface->SetDefaultPrimitiveVariables();
	ProcessPrimitiveVariables(pSurface,count,tokens,values);

	CreateGPrim(pSurface);

	return(0);
}


//----------------------------------------------------------------------
// RiCone
// Specify a cone primitive.
//
RtVoid	RiCone(RtFloat height, RtFloat radius, RtFloat thetamax, ...)
{
	va_list	pArgs;
	va_start(pArgs, thetamax);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiConeV(height, radius, thetamax, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiConeV
// List based version of above.
//
RtVoid	RiConeV(RtFloat height, RtFloat radius, RtFloat thetamax, PARAMETERLIST)
{
	// Create a cone
	CqCone* pSurface=new CqCone(height,radius,0,thetamax,0,height);
	pSurface->SetDefaultPrimitiveVariables();
	ProcessPrimitiveVariables(pSurface,count,tokens,values);
	
	CreateGPrim(pSurface);

	return(0);
}


//----------------------------------------------------------------------
// RiCylinder
// Specify a culinder primitive.
//
RtVoid	RiCylinder(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ...)
{
	va_list	pArgs;
	va_start(pArgs, thetamax);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiCylinderV(radius, zmin, zmax, thetamax, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiCylinderV
// List based version of above.
//
RtVoid	RiCylinderV(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST)
{
	// Create a cylinder
	CqCylinder* pSurface=new CqCylinder(radius,zmin,zmax,0,thetamax);
	pSurface->SetDefaultPrimitiveVariables();
	ProcessPrimitiveVariables(pSurface,count,tokens,values);

	CreateGPrim(pSurface);

	return(0);
}


//----------------------------------------------------------------------
// RiHyperboloid
// Specify a hyperboloid primitive.
//
RtVoid	RiHyperboloid(RtPoint point1, RtPoint point2, RtFloat thetamax, ...)
{
	va_list	pArgs;
	va_start(pArgs, thetamax);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiHyperboloidV(point1, point2, thetamax, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiHyperboloidV
// List based version of above.
//
RtVoid	RiHyperboloidV(RtPoint point1, RtPoint point2, RtFloat thetamax,PARAMETERLIST)
{
	// Create a hyperboloid
	CqVector3D v0(point1[0],point1[1],point1[2]);
	CqVector3D v1(point2[0],point2[1],point2[2]);
	CqHyperboloid* pSurface=new CqHyperboloid(v0,v1,0,thetamax);
	pSurface->SetDefaultPrimitiveVariables();
	ProcessPrimitiveVariables(pSurface,count,tokens,values);

	CreateGPrim(pSurface);

	return(0);
}


//----------------------------------------------------------------------
// RiParaboloid
// Specify a paraboloid primitive.
//
RtVoid	RiParaboloid(RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, ...)
{
	va_list	pArgs;
	va_start(pArgs, thetamax);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiParaboloidV(rmax, zmin, zmax, thetamax, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiParaboloidV
// List based version of above.
//
RtVoid	RiParaboloidV(RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, PARAMETERLIST)
{
	// Create a paraboloid
	CqParaboloid* pSurface=new CqParaboloid(rmax,zmin,zmax,0,thetamax);
	pSurface->SetDefaultPrimitiveVariables();
	ProcessPrimitiveVariables(pSurface,count,tokens,values);

	CreateGPrim(pSurface);

	return(0);
}


//----------------------------------------------------------------------
// RiDisk
// Specify a disk primitive.
//
RtVoid	RiDisk(RtFloat height, RtFloat radius, RtFloat thetamax, ...)
{
	va_list	pArgs;
	va_start(pArgs, thetamax);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiDiskV(height, radius, thetamax, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiDiskV
// List based version of above.
//
RtVoid	RiDiskV(RtFloat height, RtFloat radius, RtFloat thetamax, PARAMETERLIST)
{
	// Create a disk
	CqDisk* pSurface=new CqDisk(height,0,radius,0,thetamax);
	pSurface->SetDefaultPrimitiveVariables();
	ProcessPrimitiveVariables(pSurface,count,tokens,values);

	CreateGPrim(pSurface);

	return(0);
}


//----------------------------------------------------------------------
// 
//
RtVoid	RiTorus(RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, ...)
{
	va_list	pArgs;
	va_start(pArgs, thetamax);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiTorusV(majorrad, minorrad, phimin, phimax, thetamax, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiTorus
// Specify a torus primitive.
//
RtVoid	RiTorusV(RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, PARAMETERLIST)
{
	// Create a torus
	CqTorus* pSurface=new CqTorus(majorrad,minorrad,phimin,phimax,0,thetamax);
	pSurface->SetDefaultPrimitiveVariables();
	ProcessPrimitiveVariables(pSurface,count,tokens,values);

	CreateGPrim(pSurface);

	return(0);
}


//----------------------------------------------------------------------
// RiProcedural
// Implement the procedural type primitive.
//
RtVoid	RiProcedural(RtPointer data, RtBound bound, RtFunc refineproc, RtFunc freeproc)
{
	CqBasicError(0,Severity_Normal,"RiProcedural not supported");
	return(0);
}



//	RtVoid				RiGeometry();
//	RtVoid				RiGeometryV();

//----------------------------------------------------------------------
// RiSolidBegin
// Begin the definition of a CSG object.
//
RtVoid	RiSolidBegin(RtToken type)
{
	CqBasicError(0,Severity_Normal,"RiSolidBegin, CSG not supported");
	pCurrentRenderer()->CreateSolidContext();
	// Call the renderer callback, which can be overridden for system specific setup.	
	pCurrentRenderer()->OnSolidBegin();
	return(0);
}


//----------------------------------------------------------------------
// RiSolidEnd
// End the definition of a CSG object.
//
RtVoid	RiSolidEnd()
{
	pCurrentRenderer()->DeleteSolidContext();

	// Call the renderer callback, which can be overridden for system specific setup.	
	pCurrentRenderer()->OnSolidEnd();

	return(0);
}


//----------------------------------------------------------------------
// RiObjectBegin
// Begin the definition of a stored object for use by RiObjectInstance.
//
 RtObjectHandle	RiObjectBegin()
{
	CqBasicError(0,Severity_Normal,"RiObjectBegin, instances not supported");
	pCurrentRenderer()->CreateObjectContext();
	// Call the renderer callback, which can be overridden for system specific setup.	
	pCurrentRenderer()->OnObjectBegin();
	return(0);
}


//----------------------------------------------------------------------
// RiObjectEnd
// End the defintion of a stored object for use by RiObjectInstance.
//
RtVoid	RiObjectEnd()
{
	pCurrentRenderer()->DeleteObjectContext();

	// Call the renderer callback, which can be overridden for system specific setup.	
	pCurrentRenderer()->OnObjectEnd();

	return(0);
}


//----------------------------------------------------------------------
// RiObjectInstance
// Instantiate a copt of a pre-stored geometric object.
//
RtVoid	RiObjectInstance(RtObjectHandle handle)
{
	CqBasicError(0,Severity_Normal,"RiObjectInstance, instances not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiMotionBegin
// Begin the definition of the motion of an object for use by motion blur.
//
RtVoid	RiMotionBegin(RtInt N, ...)
{
	va_list	pArgs;
	va_start(pArgs, N);

	RtFloat* times=new RtFloat[N];
	RtInt i;
	for(i=0; i<N; i++)
		times[i]=va_arg(pArgs, RtFloat);

	RiMotionBeginV(N,times);
	
	delete[](times);
	return(0);
}


//----------------------------------------------------------------------
// RiBeginMotionV
// List based version of above.
//
RtVoid	RiMotionBeginV(RtInt N, RtFloat times[])
{
	pCurrentRenderer()->CreateMotionContext(N,times);
	// Call the renderer callback, which can be overridden for system specific setup.	
	pCurrentRenderer()->OnMotionBegin();
	return(0);
}


//----------------------------------------------------------------------
// RiMotionEnd
// End the definition of the motion of an object.
//
RtVoid	RiMotionEnd()
{
	pCurrentRenderer()->DeleteMotionContext();

	// Call the renderer callback, which can be overridden for system specific setup.	
	pCurrentRenderer()->OnMotionEnd();

	return(0);
}


//----------------------------------------------------------------------
// RiMakeTexture
// Convert a picture to a texture.
//
RtVoid RiMakeTexture (const char *pic, const char *tex, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ...)
{
	va_list	pArgs;
	va_start(pArgs, twidth);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiMakeTextureV(pic, tex, swrap, twrap, filterfunc, swidth, twidth, count, pTokens, pValues));
} 


//----------------------------------------------------------------------
// RiMakeTextureV
// List based version of above.
//
RtVoid	RiMakeTextureV(const char *pic, const char *tex, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST)
{
	assert(pic!=0 && tex!=0 && swrap!=0 && twrap!=0 && filterfunc!=0);

	// Get the wrap modes first.
	enum EqWrapMode smode=WrapMode_Black;
	if(strcmp(swrap,RI_PERIODIC)==0)
		smode=WrapMode_Periodic;
	else if(strcmp(swrap,RI_CLAMP)==0)
		smode=WrapMode_Clamp;

	enum EqWrapMode tmode=WrapMode_Black;
	if(strcmp(twrap,RI_PERIODIC)==0)
		tmode=WrapMode_Periodic;
	else if(strcmp(twrap,RI_CLAMP)==0)
		tmode=WrapMode_Clamp;

	// Now load the original image.
	CqTextureMap Source(pic);
	Source.Open();
	
	if(Source.IsValid() && Source.Format()==TexFormat_Plain)
	{
		// Hopefully CqTextureMap will take care of closing the tiff file after
		// it has SAT mapped it so we can overwrite if needs be.
		// Create a new image.
		TIFF* ptex=TIFFOpen(tex,"w");

		TIFFCreateDirectory(ptex);
		TIFFSetField(ptex,TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		TIFFSetField(ptex,TIFFTAG_PIXAR_TEXTUREFORMAT, SATMAP_HEADER);
		// Write the floating point image to the directory.
		CqTextureMapBuffer* pBuffer=Source.GetBuffer(0,0);
		WriteTileImage(ptex,pBuffer->pBufferData(),Source.XRes(),Source.YRes(),64,64,Source.SamplesPerPixel());
		TIFFClose(ptex);
	}

	Source.Close();
	return(0);
}


//----------------------------------------------------------------------
// RiMakeBump
// Convert a picture to a bump map.
//
RtVoid	RiMakeBump(const char *imagefile, const char *bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ...)
{
	CqBasicError(0,Severity_Normal,"RiMakeBump not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiMakeBumpV
// List based version of above.
//
RtVoid	RiMakeBumpV(const char *imagefile, const char *bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST)
{
	CqBasicError(0,Severity_Normal,"RiMakeBump not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiMakeLatLongEnvironment
// Convert a picture to an environment map.
//
RtVoid	RiMakeLatLongEnvironment(const char *imagefile, const char *reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ...)
{
	CqBasicError(0,Severity_Normal,"RiMakeLatLongEnvironment not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiMakeLatLongEnvironmentV
// List based version of above.
//
RtVoid	RiMakeLatLongEnvironmentV(const char *imagefile, const char *reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST)
{
	CqBasicError(0,Severity_Normal,"RiMakeLatLongEnvironment not supported");
	return(0);
}


//----------------------------------------------------------------------
// RiMakeCubeFaceEnvironment
// Convert a picture to a cubical environment map.
//
RtVoid	RiMakeCubeFaceEnvironment(const char *px, const char *nx, const char *py, const char *ny, const char *pz, const char *nz, const char *reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, ... )
{
	va_list	pArgs;
	va_start(pArgs, twidth);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiMakeCubeFaceEnvironmentV(px, nx, py, ny, pz, nz, reflfile, fov, filterfunc, swidth, twidth, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiMakeCubeFaceEnvironment
// List based version of above.
//
RtVoid	RiMakeCubeFaceEnvironmentV(const char *px, const char *nx, const char *py, const char *ny, const char *pz, const char *nz, const char *reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, PARAMETERLIST)
{
	assert(px!=0 && nx!=0 && py!=0 && ny!=0 && pz!=0 && nz!=0 && 
			 reflfile!=0 && filterfunc!=0);

	// Now load the original image.
	CqTextureMap tpx(px);
	CqTextureMap tnx(nx);
	CqTextureMap tpy(py);
	CqTextureMap tny(ny);
	CqTextureMap tpz(pz);
	CqTextureMap tnz(nz);

	tpx.Open();
	tnx.Open();
	tpy.Open();
	tny.Open();
	tpz.Open();
	tnz.Open();
	if(tpx.Format()!=TexFormat_SAT)	tpx.CreateSATMap();
	if(tnx.Format()!=TexFormat_SAT)	tnx.CreateSATMap();
	if(tpy.Format()!=TexFormat_SAT)	tpy.CreateSATMap();
	if(tny.Format()!=TexFormat_SAT)	tny.CreateSATMap();
	if(tpz.Format()!=TexFormat_SAT)	tpz.CreateSATMap();
	if(tnz.Format()!=TexFormat_SAT)	tnz.CreateSATMap();
	if(tpx.IsValid() && tnx.IsValid() && tpy.IsValid() && tny.IsValid() && tpz.IsValid() && tnz.IsValid())
	{
		// Check all the same size;
		bool fValid=false;
		if(tpx.XRes()==tnx.XRes() && tpx.XRes()==tpy.XRes() && tpx.XRes()==tny.XRes() && tpx.XRes()== tpz.XRes() && tpx.XRes()==tnz.XRes() &&
		   tpx.XRes()==tnx.XRes() && tpx.XRes()==tpy.XRes() && tpx.XRes()==tny.XRes() && tpx.XRes()== tpz.XRes() && tpx.XRes()==tnz.XRes())
			fValid=true;

		if(!fValid)
		{
			CqBasicError(ErrorID_InvalidData,Severity_Normal,"RiMakeCubeFaceEnvironment : Images not the same size");
			return(0);
		}

		// Now copy the images to the big map.
		CqTextureMap* Images[6]=
		{
			{&tpz},
			{&tpx},
			{&tpy},
			{&tnx},
			{&tny},
			{&tnz}
		};

		// Create a new image.
		TIFF* ptex=TIFFOpen(reflfile,"w");

		RtInt ii;
		for(ii=0; ii<6; ii++)
		{
			CqTextureMapBuffer* pBuffer=Images[ii]->GetBuffer(0,0);
			TIFFCreateDirectory(ptex);
			TIFFSetField(ptex,TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
			TIFFSetField(ptex,TIFFTAG_PIXAR_TEXTUREFORMAT, CUBEENVMAP_HEADER);
			WriteTileImage(ptex,pBuffer->pBufferData(),Images[ii]->XRes(),Images[ii]->YRes(),64,64,Images[ii]->SamplesPerPixel());
		}
		TIFFClose(ptex);
	}
	return(0);
}


//----------------------------------------------------------------------
// RiMakeShadow
// Convert a depth map file to a shadow map.
//
RtVoid	RiMakeShadow(const char *picfile, const char *shadowfile, ...)
{
	va_list	pArgs;
	va_start(pArgs, shadowfile);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiMakeShadowV(picfile, shadowfile, count, pTokens, pValues));
}


//----------------------------------------------------------------------
// RiMakeShadowV
// List based version of above.
//
RtVoid	RiMakeShadowV(const char *picfile, const char *shadowfile, PARAMETERLIST)
{
	CqShadowMap ZFile(picfile);
	ZFile.LoadZFile();
	ZFile.SaveShadowMap(shadowfile);
	return(0);
}


//----------------------------------------------------------------------
// RiErrorHandler
// Set the function used to report errors.
//
RtVoid	RiErrorHandler(RtErrorFunc handler)
{
	pCurrentRenderer()->optCurrent().SetpErrorHandler(handler);
	return(0);
}


//----------------------------------------------------------------------
// RiErrorIgnore
// Function used by RiErrorHandler to continue after errors.
//
RtVoid	RiErrorIgnore(RtInt code, RtInt severity, const char* message)
{
	return(0);
}


//----------------------------------------------------------------------
// RiErrorPrint
// Function used by RiErrorHandler to print an error message to stdout and continue.
//
RtVoid	RiErrorPrint(RtInt code, RtInt severity, const char* message)
{
	pCurrentRenderer()->PrintMessage(SqMessage(code, severity, message));
	return(0);
}


//----------------------------------------------------------------------
// RiErrorAbort
// Function used by RiErrorHandler to print and error and stop.
//
RtVoid	RiErrorAbort(RtInt code, RtInt severity, const char* message)
{
	return(0);
}


//----------------------------------------------------------------------
// RiSubdivisionMesh
// Specify a subdivision surface hull with tagging.
//
RtVoid	RiSubdivisionMesh(RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], ...)
{
	va_list	pArgs;
	va_start(pArgs, floatargs);

	RtToken* pTokens;
	RtPointer* pValues;
	RtInt count=BuildParameterList(pArgs, pTokens, pValues);

	return(RiSubdivisionMeshV(scheme, nfaces, nvertices, vertices, ntags, tags, nargs, intargs, floatargs, count, pTokens, pValues));
}

//----------------------------------------------------------------------
// RiSubdivisionMeshV
// List based version of above.
//
RtVoid	RiSubdivisionMeshV(RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], PARAMETERLIST)
{
	// Calculate how many vertices there are.
	RtInt cVerts=0;
	RtInt* pVerts=vertices;
	RtInt face;
	for(face=0; face<nfaces; face++)
	{
		RtInt v;
		for(v=0; v<nvertices[face]; v++)
		{
			if(((*pVerts)+1)>cVerts)
				cVerts=(*pVerts)+1;
			pVerts++;
		}
	}

	// Create a storage class for all the points.
	CqWSurf* pSubdivision=new CqWSurf(cVerts,nfaces);
	
	// Process any specified primitive variables
	pSubdivision->SetDefaultPrimitiveVariables(); 
	if(ProcessPrimitiveVariables(pSubdivision,count,tokens,values))
	{
		// Transform the points into camera space for processing,
		pSubdivision->Transform(pCurrentRenderer()->matSpaceToSpace("object","camera",CqMatrix(),pSubdivision->pTransform()->matObjectToWorld()),
								pCurrentRenderer()->matNSpaceToSpace("object","camera",CqMatrix(),pSubdivision->pTransform()->matObjectToWorld()),
								pCurrentRenderer()->matVSpaceToSpace("object","camera",CqMatrix(),pSubdivision->pTransform()->matObjectToWorld()));
		
		// Intitialise the vertices of the hull
		int i;
		for(i=0; i<cVerts; i++)
			pSubdivision->AddVert(new CqWVert(i));
		
		// Add the faces to the hull.
		std::vector<CqWEdge*> apEdges;
		RtInt	iP=0;
		RtInt	iPStart=0;
		for(face=0; face<nfaces; face++)
		{
			// Create a surface face
			RtBoolean fValid=RI_TRUE;
			RtInt iStartV=vertices[iP];
			RtInt i;
			for(i=0; i<nvertices[face]; i++)	// Fill in the points
			{
				if(vertices[iP]>=cVerts)
				{
					fValid=RI_FALSE;
					CqAttributeError(1, Severity_Normal,"Invalid PointsPolygon index", pSubdivision->pAttributes());
					break;
				}

				if(i<nvertices[face]-1)
				{
					CqWEdge* pE=pSubdivision->AddEdge(pSubdivision->pVert(vertices[iP]),pSubdivision->pVert(vertices[iP+1]));
					if(pE==NULL)
					{
						delete(pSubdivision);
						return(0);
					}
					apEdges.push_back(pE);
				}
				else
				{
					CqWEdge* pE=pSubdivision->AddEdge(pSubdivision->pVert(vertices[iP]),pSubdivision->pVert(vertices[iPStart]));
					if(pE==NULL)
					{
						delete(pSubdivision);
						return(0);
					}
					apEdges.push_back(pE);
				}
				iP++;
			}
			if(fValid)
			{
				// If NULL returned, an error was encountered.
				if(pSubdivision->AddFace(&apEdges[0],nvertices[face])==0)
				{
					delete(pSubdivision);
					return(0);
				}
			}
			apEdges.clear();
			iPStart=iP;
		}
		pCurrentRenderer()->Scene().AddSurface(pSubdivision);
	}
	else
		delete(pSubdivision);

	return(0);
}


//---------------------------------------------------------------------
//---------------------------------------------------------------------
// Helper functions

//----------------------------------------------------------------------
// ProcessPrimitiveVariables
// Process and fill in any primitive variables.
// return	:	RI_TRUE if position specified, RI_FALSE otherwise.

static RtBoolean ProcessPrimitiveVariables(CqSurface* pSurface, PARAMETERLIST)
{
	// Read recognised parameter values.
	RtInt		fP=RIL_NONE;
	RtInt		fN=RIL_NONE;
	RtInt		fT=RIL_NONE;
	RtBoolean	fCs=RI_FALSE;
	RtBoolean	fOs=RI_FALSE;

	RtFloat*	pPoints=0;
	RtFloat*	pNormals=0;
	RtFloat*	pTextures_s=0;
	RtFloat*	pTextures_t=0;
	RtFloat*	pTextures_st=0;
	RtFloat*	pCs=0;
	RtFloat*	pOs=0;
	RtInt i;
	for(i=0; i<count; i++)
	{
		RtToken		token=tokens[i];
		RtPointer	value=values[i];

		if(strcmp(token, RI_S)==0)
		{
			fT=RIL_s;
			pTextures_s=(RtFloat*)value;
		}
		else if(strcmp(token, RI_T)==0)
		{
			fT=RIL_t;
			pTextures_t=(RtFloat*)value;
		}
		else if(strcmp(token, RI_ST)==0)
		{
			fT=RIL_st;
			pTextures_st=(RtFloat*)value;
		}
		else if(strcmp(token, RI_CS)==0)
		{
			fCs=RI_TRUE;
			pCs=(RtFloat*)value;
		}
		else if(strcmp(token, RI_OS)==0)
		{
			fOs=RI_TRUE;
			pOs=(RtFloat*)value;
		}
		else if(strcmp(token, RI_P)==0)
		{
			fP=RIL_P;
			pPoints=(RtFloat*)value;
		}
		else if(strcmp(token, RI_PZ)==0)
		{
			fP=RIL_Pz;
			pPoints=(RtFloat*)value;
		}
		else if(strcmp(token, RI_PW)==0)
		{
			fP=RIL_Pw;
			pPoints=(RtFloat*)value;
		}
		else if(strcmp(token, RI_N)==0)
		{
			fN=RIL_N;
			pNormals=(RtFloat*)value;
		}
		else if(strcmp(token, RI_NP)==0)
		{
			fN=RIL_Np;
			pNormals=(RtFloat*)value;
		}
	}

	// Fill in the position variable according to type.
	if(fP!=RIL_NONE)
	{
		switch(fP)
		{
			case RIL_P:
				pSurface->P().SetSize(pSurface->cVertex());
				for(i=0; i<pSurface->cVertex(); i++)
					pSurface->P()[i]=CqVector3D(pPoints[(i*3)], pPoints[(i*3)+1], pPoints[(i*3)+2]);
				break;

			case RIL_Pz:
				pSurface->P().SetSize(pSurface->cVertex());
				for(i=0; i<pSurface->cVertex(); i++)
					pSurface->P()[i]=CqVector3D(i%1, floor(i/2), pPoints[i]);
				break;

			case RIL_Pw:
				pSurface->P().SetSize(pSurface->cVertex());
				for(i=0; i<pSurface->cVertex(); i++)
					pSurface->P()[i]=CqVector4D(pPoints[(i*4)], pPoints[(i*4)+1], pPoints[(i*4)+2], pPoints[(i*4)+3]);
				break;
		}
	}


	// Fill in the position variable according to type.
	if(fN!=RIL_NONE)
	{
		switch(fN)
		{
			case RIL_N:
				pSurface->N().SetSize(pSurface->cVarying());
				for(i=0; i<pSurface->cVarying(); i++)
					pSurface->N()[i]=CqVector3D(pNormals[(i*3)], pNormals[(i*3)+1], pNormals[(i*3)+2]);
				break;

			case RIL_Np:
				pSurface->N().SetSize(pSurface->cUniform());
				for(i=0; i<pSurface->cUniform(); i++)
					pSurface->N()[i]=CqVector3D(pNormals[(i*3)], pNormals[(i*3)+1], pNormals[(i*3)+2]);
				break;
		}
	}


	// Copy any specified texture coordinates to the surface.
	if(fT!=RIL_NONE)
	{
		switch(fT)
		{
			case RIL_s:
				if(pTextures_s!=0)
				{
					pSurface->s().SetSize(pSurface->cVarying());
					for(i=0; i<pSurface->cVarying(); i++)
						pSurface->s()[i]=pTextures_s[i];
				}

				if(pTextures_t!=0)
				{
					pSurface->t().SetSize(pSurface->cVarying());
					for(i=0; i<pSurface->cVarying(); i++)
						pSurface->t()[i]=pTextures_t[i];
				}
				break;

			case RIL_st:
				assert(pTextures_st!=0);
				pSurface->s().SetSize(pSurface->cVarying());
				pSurface->t().SetSize(pSurface->cVarying());
				for(i=0; i<pSurface->cVarying(); i++)
				{
					pSurface->s()[i]=pTextures_st[(i*2)];
					pSurface->t()[i]=pTextures_st[(i*2)+1];
				}
				break;
		}
	}

	// Copy any specified varying color values to the surface
	if(fCs)
	{
		pSurface->Cs().SetSize(pSurface->cVarying());
		for(i=0; i<pSurface->cVarying(); i++)
			pSurface->Cs()[i]=CqColor(pCs[(i*3)],pCs[(i*3)+1],pCs[(i*3)+2]);
	}

	if(fOs)
	{
		pSurface->Os().SetSize(pSurface->cVarying());
		for(i=0; i<pSurface->cVarying(); i++)
			pSurface->Os()[i]=CqColor(pOs[(i*3)],pOs[(i*3)+1],pOs[(i*3)+2]);
	}
	return(fP!=RIL_NONE);
}


//----------------------------------------------------------------------
// CreateGPrin
// Create and register a GPrim according to the current attributes/transform
//
template<class T>
RtVoid	CreateGPrim(T* pSurface)
{
	if(pCurrentRenderer()->ptransCurrent()->cTimes()<=1)
	{
		// Transform the points into camera space for processing,
		pSurface->Transform(pCurrentRenderer()->matSpaceToSpace("object","camera",CqMatrix(),pSurface->pTransform()->matObjectToWorld()),
							pCurrentRenderer()->matNSpaceToSpace("object","camera",CqMatrix(),pSurface->pTransform()->matObjectToWorld()),
							pCurrentRenderer()->matVSpaceToSpace("object","camera",CqMatrix(),pSurface->pTransform()->matObjectToWorld()));

		pCurrentRenderer()->Scene().AddSurface(pSurface);
	}
	else
	{
		CqMotionSurface<T*>* pMotionSurface=new CqMotionSurface<T*>(0);
		// Add the provided surface as time 0, we will transform it later.
		pMotionSurface->AddTimeSlot(pCurrentRenderer()->ptransCurrent()->Time(0),pSurface);
		RtInt i;
		for(i=1; i<pCurrentRenderer()->ptransCurrent()->cTimes(); i++)
		{
			RtFloat time=pCurrentRenderer()->ptransCurrent()->Time(i);

			T* pTimeSurface=new T(*pSurface);
			pMotionSurface->AddTimeSlot(time,pTimeSurface);

			// Transform the points into camera space for processing,
			pTimeSurface->Transform(pCurrentRenderer()->matSpaceToSpace("object","camera",CqMatrix(),pTimeSurface->pTransform()->matObjectToWorld(time)),
									pCurrentRenderer()->matNSpaceToSpace("object","camera",CqMatrix(),pTimeSurface->pTransform()->matObjectToWorld(time)),
									pCurrentRenderer()->matVSpaceToSpace("object","camera",CqMatrix(),pTimeSurface->pTransform()->matObjectToWorld(time)));
		}
		// Transform the points into camera space for processing,
		pSurface->Transform(pCurrentRenderer()->matSpaceToSpace("object","camera",CqMatrix(),pSurface->pTransform()->matObjectToWorld()),
							pCurrentRenderer()->matNSpaceToSpace("object","camera",CqMatrix(),pSurface->pTransform()->matObjectToWorld()),
							pCurrentRenderer()->matVSpaceToSpace("object","camera",CqMatrix(),pSurface->pTransform()->matObjectToWorld()));
		pCurrentRenderer()->Scene().AddSurface(pMotionSurface);
	}

	return(0);
}


//----------------------------------------------------------------------
/** Get the basis matrix given a standard basis name.
 * \param b Storage for basis matrix.
 * \param strName Name of basis.
 * \return Boolean indicating the basis is valid.
 */

RtBoolean	BasisFromName(RtBasis& b, const char* strName)
{
	RtBasis* pVals=0;
	if(strName=="bezier")
		pVals=&RiBezierBasis;
	else if(strName=="bspline")
		pVals=&RiBSplineBasis;
	else if(strName=="catmull-rom")
		pVals=&RiCatmullRomBasis;
	else if(strName=="hermite")
		pVals=&RiHermiteBasis;
	else if(strName=="power")
		pVals=&RiPowerBasis;

	if(pVals)
	{
		TqInt i,j;
		for(i=0; i<4; i++)
			for(j=0; j<4; j++)
				b[i][j]=(*pVals)[i][j];
		return(TqTrue);
	}
	return(TqFalse);
}


//---------------------------------------------------------------------
 
