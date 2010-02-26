// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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
 *
 * \brief RIB request handler implementation for aqsis.
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 */

#include "ribsema.h"

#include <cstring>  // for strcpy

#include <boost/shared_ptr.hpp>

#include <aqsis/ri/ri.h>

namespace Aqsis
{

//------------------------------------------------------------------------------
// RibSema implementation

RibSema::RibSema()
    : m_requestHandlerMap(),
    m_numColorComps(3),
    m_tokenDict(),
    m_lightMap(),
    m_namedLightMap(),
    m_objectMap(),
    m_namedObjectMap()
{
    typedef HandlerMap::value_type MapValueType;
    MapValueType handlerMapInit[] = {
        /*[[[cog
        from cogutils import *
        riXml = parseXmlTree('ri.xml')

        for p in filter(lambda p: p.haschild('Rib'), riXml.findall('Procedures/Procedure')):
            name = p.findtext('Name')
            cog.outl('MapValueType("%s", &RibSema::handle%s),' % (name, name))
        ]]]*/
        MapValueType("Declare", &RibSema::handleDeclare),
        MapValueType("FrameBegin", &RibSema::handleFrameBegin),
        MapValueType("FrameEnd", &RibSema::handleFrameEnd),
        MapValueType("WorldBegin", &RibSema::handleWorldBegin),
        MapValueType("WorldEnd", &RibSema::handleWorldEnd),
        MapValueType("IfBegin", &RibSema::handleIfBegin),
        MapValueType("ElseIf", &RibSema::handleElseIf),
        MapValueType("Else", &RibSema::handleElse),
        MapValueType("IfEnd", &RibSema::handleIfEnd),
        MapValueType("Format", &RibSema::handleFormat),
        MapValueType("FrameAspectRatio", &RibSema::handleFrameAspectRatio),
        MapValueType("ScreenWindow", &RibSema::handleScreenWindow),
        MapValueType("CropWindow", &RibSema::handleCropWindow),
        MapValueType("Projection", &RibSema::handleProjection),
        MapValueType("Clipping", &RibSema::handleClipping),
        MapValueType("ClippingPlane", &RibSema::handleClippingPlane),
        MapValueType("DepthOfField", &RibSema::handleDepthOfField),
        MapValueType("Shutter", &RibSema::handleShutter),
        MapValueType("PixelVariance", &RibSema::handlePixelVariance),
        MapValueType("PixelSamples", &RibSema::handlePixelSamples),
        MapValueType("PixelFilter", &RibSema::handlePixelFilter),
        MapValueType("Exposure", &RibSema::handleExposure),
        MapValueType("Imager", &RibSema::handleImager),
        MapValueType("Quantize", &RibSema::handleQuantize),
        MapValueType("Display", &RibSema::handleDisplay),
        MapValueType("Hider", &RibSema::handleHider),
        MapValueType("ColorSamples", &RibSema::handleColorSamples),
        MapValueType("RelativeDetail", &RibSema::handleRelativeDetail),
        MapValueType("Option", &RibSema::handleOption),
        MapValueType("AttributeBegin", &RibSema::handleAttributeBegin),
        MapValueType("AttributeEnd", &RibSema::handleAttributeEnd),
        MapValueType("Color", &RibSema::handleColor),
        MapValueType("Opacity", &RibSema::handleOpacity),
        MapValueType("TextureCoordinates", &RibSema::handleTextureCoordinates),
        MapValueType("LightSource", &RibSema::handleLightSource),
        MapValueType("AreaLightSource", &RibSema::handleAreaLightSource),
        MapValueType("Illuminate", &RibSema::handleIlluminate),
        MapValueType("Surface", &RibSema::handleSurface),
        MapValueType("Displacement", &RibSema::handleDisplacement),
        MapValueType("Atmosphere", &RibSema::handleAtmosphere),
        MapValueType("Interior", &RibSema::handleInterior),
        MapValueType("Exterior", &RibSema::handleExterior),
        MapValueType("ShaderLayer", &RibSema::handleShaderLayer),
        MapValueType("ConnectShaderLayers", &RibSema::handleConnectShaderLayers),
        MapValueType("ShadingRate", &RibSema::handleShadingRate),
        MapValueType("ShadingInterpolation", &RibSema::handleShadingInterpolation),
        MapValueType("Matte", &RibSema::handleMatte),
        MapValueType("Bound", &RibSema::handleBound),
        MapValueType("Detail", &RibSema::handleDetail),
        MapValueType("DetailRange", &RibSema::handleDetailRange),
        MapValueType("GeometricApproximation", &RibSema::handleGeometricApproximation),
        MapValueType("Orientation", &RibSema::handleOrientation),
        MapValueType("ReverseOrientation", &RibSema::handleReverseOrientation),
        MapValueType("Sides", &RibSema::handleSides),
        MapValueType("Identity", &RibSema::handleIdentity),
        MapValueType("Transform", &RibSema::handleTransform),
        MapValueType("ConcatTransform", &RibSema::handleConcatTransform),
        MapValueType("Perspective", &RibSema::handlePerspective),
        MapValueType("Translate", &RibSema::handleTranslate),
        MapValueType("Rotate", &RibSema::handleRotate),
        MapValueType("Scale", &RibSema::handleScale),
        MapValueType("Skew", &RibSema::handleSkew),
        MapValueType("CoordinateSystem", &RibSema::handleCoordinateSystem),
        MapValueType("CoordSysTransform", &RibSema::handleCoordSysTransform),
        MapValueType("TransformBegin", &RibSema::handleTransformBegin),
        MapValueType("TransformEnd", &RibSema::handleTransformEnd),
        MapValueType("Resource", &RibSema::handleResource),
        MapValueType("ResourceBegin", &RibSema::handleResourceBegin),
        MapValueType("ResourceEnd", &RibSema::handleResourceEnd),
        MapValueType("Attribute", &RibSema::handleAttribute),
        MapValueType("Polygon", &RibSema::handlePolygon),
        MapValueType("GeneralPolygon", &RibSema::handleGeneralPolygon),
        MapValueType("PointsPolygons", &RibSema::handlePointsPolygons),
        MapValueType("PointsGeneralPolygons", &RibSema::handlePointsGeneralPolygons),
        MapValueType("Basis", &RibSema::handleBasis),
        MapValueType("Patch", &RibSema::handlePatch),
        MapValueType("PatchMesh", &RibSema::handlePatchMesh),
        MapValueType("NuPatch", &RibSema::handleNuPatch),
        MapValueType("TrimCurve", &RibSema::handleTrimCurve),
        MapValueType("SubdivisionMesh", &RibSema::handleSubdivisionMesh),
        MapValueType("Sphere", &RibSema::handleSphere),
        MapValueType("Cone", &RibSema::handleCone),
        MapValueType("Cylinder", &RibSema::handleCylinder),
        MapValueType("Hyperboloid", &RibSema::handleHyperboloid),
        MapValueType("Paraboloid", &RibSema::handleParaboloid),
        MapValueType("Disk", &RibSema::handleDisk),
        MapValueType("Torus", &RibSema::handleTorus),
        MapValueType("Points", &RibSema::handlePoints),
        MapValueType("Curves", &RibSema::handleCurves),
        MapValueType("Blobby", &RibSema::handleBlobby),
        MapValueType("Procedural", &RibSema::handleProcedural),
        MapValueType("Geometry", &RibSema::handleGeometry),
        MapValueType("SolidBegin", &RibSema::handleSolidBegin),
        MapValueType("SolidEnd", &RibSema::handleSolidEnd),
        MapValueType("ObjectBegin", &RibSema::handleObjectBegin),
        MapValueType("ObjectEnd", &RibSema::handleObjectEnd),
        MapValueType("ObjectInstance", &RibSema::handleObjectInstance),
        MapValueType("MotionBegin", &RibSema::handleMotionBegin),
        MapValueType("MotionEnd", &RibSema::handleMotionEnd),
        MapValueType("MakeTexture", &RibSema::handleMakeTexture),
        MapValueType("MakeLatLongEnvironment", &RibSema::handleMakeLatLongEnvironment),
        MapValueType("MakeCubeFaceEnvironment", &RibSema::handleMakeCubeFaceEnvironment),
        MapValueType("MakeShadow", &RibSema::handleMakeShadow),
        MapValueType("MakeOcclusion", &RibSema::handleMakeOcclusion),
        MapValueType("ErrorHandler", &RibSema::handleErrorHandler),
        MapValueType("ReadArchive", &RibSema::handleReadArchive),
        //[[[end]]]
    };
    m_requestHandlerMap.insert(
            handlerMapInit,
            handlerMapInit + sizeof(handlerMapInit)/sizeof(handlerMapInit[0])
    );
    // Add the special RIB-only "version" request to the list.
    m_requestHandlerMap["version"] = &RibSema::handleVersion;
}

void RibSema::handleRequest(const std::string& requestName,
        IqRibParser& parser)
{
    HandlerMap::const_iterator pos = m_requestHandlerMap.find(requestName);
    if(pos == m_requestHandlerMap.end())
        AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken, "unrecognized request");
    RequestHandlerType handler = pos->second;
    (this->*handler)(parser);
}

void RibSema::handleVersion(IqRibParser& parser)
{
	parser.getFloat();
	// Don't do anything with the version number; just blunder on regardless.
	// Probably only worth supporting if Pixar started publishing new versions
	// of the standard again...
}


//--------------------------------------------------
// Implementations for handlers which are hard to autogenerate.

void RibSema::handleDeclare(IqRibParser& parser)
{
}

void RibSema::handleDepthOfField(IqRibParser& parser)
{
}

void RibSema::handleColorSamples(IqRibParser& parser)
{
}

void RibSema::handleLightSource(IqRibParser& parser)
{
}

void RibSema::handleAreaLightSource(IqRibParser& parser)
{
}

void RibSema::handleIlluminate(IqRibParser& parser)
{
}

void RibSema::handleBasis(IqRibParser& parser)
{
}

void RibSema::handleSubdivisionMesh(IqRibParser& parser)
{
}

void RibSema::handleHyperboloid(IqRibParser& parser)
{
}

void RibSema::handleProcedural(IqRibParser& parser)
{
}

void RibSema::handleObjectBegin(IqRibParser& parser)
{
}

void RibSema::handleObjectInstance(IqRibParser& parser)
{
}


//--------------------------------------------------
// Request handlers with autogenerated implementations.
/*[[[cog
import cog
import sys, os
from cogutils import *
from Cheetah.Template import Template

riXml = parseXmlTree('ri.xml')

# Map from RI types to strings which retrieve the value from the parser
getterStatements = {
    'RtBoolean':         'TqInt %s = parser.getInt();',
    'RtInt':             'TqInt %s = parser.getInt();',
    'RtIntArray':        'ArrayHolder<RtInt> %s = parser.getIntArray()',
    'RtFloat':           'TqFloat %s = parser.getFloat()',
    'RtFloatArray':      'ArrayHolder<RtFloat> %s = parser.getFloatArray()',
    'RtString':          'StringHolder %s = parser.getString()',
    'RtStringArray':     'SqRtTokenArrayHolder %s = parser.getStringArray()',
    'RtToken':           'std::string %s = parser.getString()',
    'RtTokenArray':      'SqRtTokenArrayHolder %s = parser.getStringArray()',
    'RtColor':           'ColorHolder %s = parser.getFloatArray(m_numColorComps)',
    'RtPoint':           'PointHolder %s = parser.getFloatArray()',
    'RtMatrix':          'SqRtMatrixHolder %s = parser.getFloatArray()',
    'RtBound':           'BoundHolder %s = parser.getFloatArray(6)',
    'RtFilterFunc':      'CqFilterFuncString %s = parser.getString()',
    'RtArchiveCallback': 'RtArchiveCallback %s = m_interface.lookup()',
    'RtErrorFunc':       'CqErrorHandlerString %s = parser.getString()',
}

customImpl = set(['Declare', 'DepthOfField', 'ColorSamples', 'LightSource',
                  'AreaLightSource', 'Illuminate', 'Basis', 'SubdivisionMesh',
                  'Hyperboloid', 'Procedural', 'ObjectBegin', 'ObjectInstance'])

handlerTemplate = '''
#for $proc in $riXml.findall('Procedures/Procedure')
    ## Ignore procs which have custom implementations.
    #set $procName = $proc.findtext('Name')
    #if not $proc.haschild('Rib') or $procName in $customImpl
            #continue
    #end if
    #set $args = filter(lambda a: not a.haschild('RibValue'), $proc.findall('Arguments/Argument'))
    ## 
void RibSema::handle${procName}(IqRibParser& parser)
{
    #if $proc.haschild('Arguments/RibArgsCanBeArray')
    // Collect all args as an array
    const IqRibParser::TqFloatArray& allArgs = parser.getFloatArray(${len($args)});
        #for $i, $arg in enumerate($args)
    $arg.findtext('Name') = allArgs[$i];
        #end for
    #else
    // Collect individual arguments from parser
        #for $arg in $args
    ${getterStatements[$arg.findtext('Type')] % ($arg.findtext('Name'),)};
        #end for
    #end if
    #if $proc.haschild('Arguments/ParamList')
    // Extract the parameter list
    CqParamListHandler paramList(m_tokenDict);
    parser.getParamList(paramList);
    #end if
    ## Construct the argument list to the C++ interface binding call.
    #set $argList = []
    #for $arg in $args
        #set $argList += ['toRiType(%s)' % ($arg.findtext('Name'),)]
    #end for
    #if $proc.haschild('Arguments/ParamList')
        #set $argList += ['paramList']
    #end if
    // Call through to the C binding
    m_interface.${procName}(${', '.join(argList)});
}

#end for
'''

#cog.out(str(Template(handlerTemplate, searchList=locals())));

for p in filter(lambda p: p.haschild('Rib') and p.findtext('Name') not in customImpl, riXml.findall('Procedures/Procedure')):
    name = p.findtext('Name')
    cog.outl('void RibSema::handle%s(IqRibParser& parser)' % (name,))
    cog.outl('{')
    cog.outl('}')
    cog.outl('')

]]]*/
void RibSema::handleFrameBegin(IqRibParser& parser)
{
}

void RibSema::handleFrameEnd(IqRibParser& parser)
{
}

void RibSema::handleWorldBegin(IqRibParser& parser)
{
}

void RibSema::handleWorldEnd(IqRibParser& parser)
{
}

void RibSema::handleIfBegin(IqRibParser& parser)
{
}

void RibSema::handleElseIf(IqRibParser& parser)
{
}

void RibSema::handleElse(IqRibParser& parser)
{
}

void RibSema::handleIfEnd(IqRibParser& parser)
{
}

void RibSema::handleFormat(IqRibParser& parser)
{
}

void RibSema::handleFrameAspectRatio(IqRibParser& parser)
{
}

void RibSema::handleScreenWindow(IqRibParser& parser)
{
}

void RibSema::handleCropWindow(IqRibParser& parser)
{
}

void RibSema::handleProjection(IqRibParser& parser)
{
}

void RibSema::handleClipping(IqRibParser& parser)
{
}

void RibSema::handleClippingPlane(IqRibParser& parser)
{
}

void RibSema::handleShutter(IqRibParser& parser)
{
}

void RibSema::handlePixelVariance(IqRibParser& parser)
{
}

void RibSema::handlePixelSamples(IqRibParser& parser)
{
}

void RibSema::handlePixelFilter(IqRibParser& parser)
{
}

void RibSema::handleExposure(IqRibParser& parser)
{
}

void RibSema::handleImager(IqRibParser& parser)
{
}

void RibSema::handleQuantize(IqRibParser& parser)
{
}

void RibSema::handleDisplay(IqRibParser& parser)
{
}

void RibSema::handleHider(IqRibParser& parser)
{
}

void RibSema::handleRelativeDetail(IqRibParser& parser)
{
}

void RibSema::handleOption(IqRibParser& parser)
{
}

void RibSema::handleAttributeBegin(IqRibParser& parser)
{
}

void RibSema::handleAttributeEnd(IqRibParser& parser)
{
}

void RibSema::handleColor(IqRibParser& parser)
{
}

void RibSema::handleOpacity(IqRibParser& parser)
{
}

void RibSema::handleTextureCoordinates(IqRibParser& parser)
{
}

void RibSema::handleSurface(IqRibParser& parser)
{
}

void RibSema::handleDisplacement(IqRibParser& parser)
{
}

void RibSema::handleAtmosphere(IqRibParser& parser)
{
}

void RibSema::handleInterior(IqRibParser& parser)
{
}

void RibSema::handleExterior(IqRibParser& parser)
{
}

void RibSema::handleShaderLayer(IqRibParser& parser)
{
}

void RibSema::handleConnectShaderLayers(IqRibParser& parser)
{
}

void RibSema::handleShadingRate(IqRibParser& parser)
{
}

void RibSema::handleShadingInterpolation(IqRibParser& parser)
{
}

void RibSema::handleMatte(IqRibParser& parser)
{
}

void RibSema::handleBound(IqRibParser& parser)
{
}

void RibSema::handleDetail(IqRibParser& parser)
{
}

void RibSema::handleDetailRange(IqRibParser& parser)
{
}

void RibSema::handleGeometricApproximation(IqRibParser& parser)
{
}

void RibSema::handleOrientation(IqRibParser& parser)
{
}

void RibSema::handleReverseOrientation(IqRibParser& parser)
{
}

void RibSema::handleSides(IqRibParser& parser)
{
}

void RibSema::handleIdentity(IqRibParser& parser)
{
}

void RibSema::handleTransform(IqRibParser& parser)
{
}

void RibSema::handleConcatTransform(IqRibParser& parser)
{
}

void RibSema::handlePerspective(IqRibParser& parser)
{
}

void RibSema::handleTranslate(IqRibParser& parser)
{
}

void RibSema::handleRotate(IqRibParser& parser)
{
}

void RibSema::handleScale(IqRibParser& parser)
{
}

void RibSema::handleSkew(IqRibParser& parser)
{
}

void RibSema::handleCoordinateSystem(IqRibParser& parser)
{
}

void RibSema::handleCoordSysTransform(IqRibParser& parser)
{
}

void RibSema::handleTransformBegin(IqRibParser& parser)
{
}

void RibSema::handleTransformEnd(IqRibParser& parser)
{
}

void RibSema::handleResource(IqRibParser& parser)
{
}

void RibSema::handleResourceBegin(IqRibParser& parser)
{
}

void RibSema::handleResourceEnd(IqRibParser& parser)
{
}

void RibSema::handleAttribute(IqRibParser& parser)
{
}

void RibSema::handlePolygon(IqRibParser& parser)
{
}

void RibSema::handleGeneralPolygon(IqRibParser& parser)
{
}

void RibSema::handlePointsPolygons(IqRibParser& parser)
{
}

void RibSema::handlePointsGeneralPolygons(IqRibParser& parser)
{
}

void RibSema::handlePatch(IqRibParser& parser)
{
}

void RibSema::handlePatchMesh(IqRibParser& parser)
{
}

void RibSema::handleNuPatch(IqRibParser& parser)
{
}

void RibSema::handleTrimCurve(IqRibParser& parser)
{
}

void RibSema::handleSphere(IqRibParser& parser)
{
}

void RibSema::handleCone(IqRibParser& parser)
{
}

void RibSema::handleCylinder(IqRibParser& parser)
{
}

void RibSema::handleParaboloid(IqRibParser& parser)
{
}

void RibSema::handleDisk(IqRibParser& parser)
{
}

void RibSema::handleTorus(IqRibParser& parser)
{
}

void RibSema::handlePoints(IqRibParser& parser)
{
}

void RibSema::handleCurves(IqRibParser& parser)
{
}

void RibSema::handleBlobby(IqRibParser& parser)
{
}

void RibSema::handleGeometry(IqRibParser& parser)
{
}

void RibSema::handleSolidBegin(IqRibParser& parser)
{
}

void RibSema::handleSolidEnd(IqRibParser& parser)
{
}

void RibSema::handleObjectEnd(IqRibParser& parser)
{
}

void RibSema::handleMotionBegin(IqRibParser& parser)
{
}

void RibSema::handleMotionEnd(IqRibParser& parser)
{
}

void RibSema::handleMakeTexture(IqRibParser& parser)
{
}

void RibSema::handleMakeLatLongEnvironment(IqRibParser& parser)
{
}

void RibSema::handleMakeCubeFaceEnvironment(IqRibParser& parser)
{
}

void RibSema::handleMakeShadow(IqRibParser& parser)
{
}

void RibSema::handleMakeOcclusion(IqRibParser& parser)
{
}

void RibSema::handleErrorHandler(IqRibParser& parser)
{
}

void RibSema::handleReadArchive(IqRibParser& parser)
{
}

//[[[end]]]


//--------------------------------------------------

} // namespace Aqsis
