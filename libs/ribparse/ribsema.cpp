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

#include <cfloat>
#include <cstring>  // for strcpy

#include "riblexer.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
// RibParser implementation

RibParser::RibParser(Ri::Renderer& renderer)
    : m_renderer(renderer),
    m_lexer(RibLexer::create(), &RibLexer::destroy),
    m_requestHandlerMap(),
    m_paramListStorage(),
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
            cog.outl('MapValueType("%s", &RibParser::handle%s),' % (name, name))
        ]]]*/
        MapValueType("Declare", &RibParser::handleDeclare),
        MapValueType("FrameBegin", &RibParser::handleFrameBegin),
        MapValueType("FrameEnd", &RibParser::handleFrameEnd),
        MapValueType("WorldBegin", &RibParser::handleWorldBegin),
        MapValueType("WorldEnd", &RibParser::handleWorldEnd),
        MapValueType("IfBegin", &RibParser::handleIfBegin),
        MapValueType("ElseIf", &RibParser::handleElseIf),
        MapValueType("Else", &RibParser::handleElse),
        MapValueType("IfEnd", &RibParser::handleIfEnd),
        MapValueType("Format", &RibParser::handleFormat),
        MapValueType("FrameAspectRatio", &RibParser::handleFrameAspectRatio),
        MapValueType("ScreenWindow", &RibParser::handleScreenWindow),
        MapValueType("CropWindow", &RibParser::handleCropWindow),
        MapValueType("Projection", &RibParser::handleProjection),
        MapValueType("Clipping", &RibParser::handleClipping),
        MapValueType("ClippingPlane", &RibParser::handleClippingPlane),
        MapValueType("DepthOfField", &RibParser::handleDepthOfField),
        MapValueType("Shutter", &RibParser::handleShutter),
        MapValueType("PixelVariance", &RibParser::handlePixelVariance),
        MapValueType("PixelSamples", &RibParser::handlePixelSamples),
        MapValueType("PixelFilter", &RibParser::handlePixelFilter),
        MapValueType("Exposure", &RibParser::handleExposure),
        MapValueType("Imager", &RibParser::handleImager),
        MapValueType("Quantize", &RibParser::handleQuantize),
        MapValueType("Display", &RibParser::handleDisplay),
        MapValueType("Hider", &RibParser::handleHider),
        MapValueType("ColorSamples", &RibParser::handleColorSamples),
        MapValueType("RelativeDetail", &RibParser::handleRelativeDetail),
        MapValueType("Option", &RibParser::handleOption),
        MapValueType("AttributeBegin", &RibParser::handleAttributeBegin),
        MapValueType("AttributeEnd", &RibParser::handleAttributeEnd),
        MapValueType("Color", &RibParser::handleColor),
        MapValueType("Opacity", &RibParser::handleOpacity),
        MapValueType("TextureCoordinates", &RibParser::handleTextureCoordinates),
        MapValueType("LightSource", &RibParser::handleLightSource),
        MapValueType("AreaLightSource", &RibParser::handleAreaLightSource),
        MapValueType("Illuminate", &RibParser::handleIlluminate),
        MapValueType("Surface", &RibParser::handleSurface),
        MapValueType("Displacement", &RibParser::handleDisplacement),
        MapValueType("Atmosphere", &RibParser::handleAtmosphere),
        MapValueType("Interior", &RibParser::handleInterior),
        MapValueType("Exterior", &RibParser::handleExterior),
        MapValueType("ShaderLayer", &RibParser::handleShaderLayer),
        MapValueType("ConnectShaderLayers", &RibParser::handleConnectShaderLayers),
        MapValueType("ShadingRate", &RibParser::handleShadingRate),
        MapValueType("ShadingInterpolation", &RibParser::handleShadingInterpolation),
        MapValueType("Matte", &RibParser::handleMatte),
        MapValueType("Bound", &RibParser::handleBound),
        MapValueType("Detail", &RibParser::handleDetail),
        MapValueType("DetailRange", &RibParser::handleDetailRange),
        MapValueType("GeometricApproximation", &RibParser::handleGeometricApproximation),
        MapValueType("Orientation", &RibParser::handleOrientation),
        MapValueType("ReverseOrientation", &RibParser::handleReverseOrientation),
        MapValueType("Sides", &RibParser::handleSides),
        MapValueType("Identity", &RibParser::handleIdentity),
        MapValueType("Transform", &RibParser::handleTransform),
        MapValueType("ConcatTransform", &RibParser::handleConcatTransform),
        MapValueType("Perspective", &RibParser::handlePerspective),
        MapValueType("Translate", &RibParser::handleTranslate),
        MapValueType("Rotate", &RibParser::handleRotate),
        MapValueType("Scale", &RibParser::handleScale),
        MapValueType("Skew", &RibParser::handleSkew),
        MapValueType("CoordinateSystem", &RibParser::handleCoordinateSystem),
        MapValueType("CoordSysTransform", &RibParser::handleCoordSysTransform),
        MapValueType("TransformBegin", &RibParser::handleTransformBegin),
        MapValueType("TransformEnd", &RibParser::handleTransformEnd),
        MapValueType("Resource", &RibParser::handleResource),
        MapValueType("ResourceBegin", &RibParser::handleResourceBegin),
        MapValueType("ResourceEnd", &RibParser::handleResourceEnd),
        MapValueType("Attribute", &RibParser::handleAttribute),
        MapValueType("Polygon", &RibParser::handlePolygon),
        MapValueType("GeneralPolygon", &RibParser::handleGeneralPolygon),
        MapValueType("PointsPolygons", &RibParser::handlePointsPolygons),
        MapValueType("PointsGeneralPolygons", &RibParser::handlePointsGeneralPolygons),
        MapValueType("Basis", &RibParser::handleBasis),
        MapValueType("Patch", &RibParser::handlePatch),
        MapValueType("PatchMesh", &RibParser::handlePatchMesh),
        MapValueType("NuPatch", &RibParser::handleNuPatch),
        MapValueType("TrimCurve", &RibParser::handleTrimCurve),
        MapValueType("SubdivisionMesh", &RibParser::handleSubdivisionMesh),
        MapValueType("Sphere", &RibParser::handleSphere),
        MapValueType("Cone", &RibParser::handleCone),
        MapValueType("Cylinder", &RibParser::handleCylinder),
        MapValueType("Hyperboloid", &RibParser::handleHyperboloid),
        MapValueType("Paraboloid", &RibParser::handleParaboloid),
        MapValueType("Disk", &RibParser::handleDisk),
        MapValueType("Torus", &RibParser::handleTorus),
        MapValueType("Points", &RibParser::handlePoints),
        MapValueType("Curves", &RibParser::handleCurves),
        MapValueType("Blobby", &RibParser::handleBlobby),
        MapValueType("Procedural", &RibParser::handleProcedural),
        MapValueType("Geometry", &RibParser::handleGeometry),
        MapValueType("SolidBegin", &RibParser::handleSolidBegin),
        MapValueType("SolidEnd", &RibParser::handleSolidEnd),
        MapValueType("ObjectBegin", &RibParser::handleObjectBegin),
        MapValueType("ObjectEnd", &RibParser::handleObjectEnd),
        MapValueType("ObjectInstance", &RibParser::handleObjectInstance),
        MapValueType("MotionBegin", &RibParser::handleMotionBegin),
        MapValueType("MotionEnd", &RibParser::handleMotionEnd),
        MapValueType("MakeTexture", &RibParser::handleMakeTexture),
        MapValueType("MakeLatLongEnvironment", &RibParser::handleMakeLatLongEnvironment),
        MapValueType("MakeCubeFaceEnvironment", &RibParser::handleMakeCubeFaceEnvironment),
        MapValueType("MakeShadow", &RibParser::handleMakeShadow),
        MapValueType("MakeOcclusion", &RibParser::handleMakeOcclusion),
        MapValueType("ErrorHandler", &RibParser::handleErrorHandler),
        MapValueType("ReadArchive", &RibParser::handleReadArchive),
        //[[[end]]]
    };
    m_requestHandlerMap.insert(
            handlerMapInit,
            handlerMapInit + sizeof(handlerMapInit)/sizeof(handlerMapInit[0])
    );
    // Add the special RIB-only "version" request to the list.
    m_requestHandlerMap["version"] = &RibParser::handleVersion;
}

namespace {
/// Callback which sends errors to Ri::Renderer::ArchiveRecord
class CommentCallback
{
    private:
        Ri::Renderer& m_renderer;
    public:
        CommentCallback(Ri::Renderer& renderer) : m_renderer(renderer) {}
        void operator()(const std::string& error)
        {
            m_renderer.ArchiveRecord("comment", error.c_str());
        }
};
} // anon. namespace

void RibParser::parseStream(std::istream& ribStream,
                            const std::string& streamName)
{
    m_lexer->pushInput(ribStream, streamName, CommentCallback(m_renderer));
    while(true)
    {
        const char* requestName = 0;
        try
        {
            requestName = m_lexer->nextRequest();
            if(!requestName)
                break;  // end of stream
            HandlerMap::const_iterator pos =
                m_requestHandlerMap.find(requestName);
            if(pos == m_requestHandlerMap.end())
                AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken,
                                    "unrecognized request");
            RequestHandlerType handler = pos->second;
            (this->*handler)(*m_lexer);
        }
        catch(XqValidation& e)
        {
            // Add information on the location (file,line etc) of the problem
            // to the exception message and rethrow.
            std::ostringstream msg;
            msg << "Parse error at " << m_lexer->streamPos();
            if(requestName)
                msg << " while reading " << requestName;
            msg << ": " << e.what();
            m_renderer.Error(msg.str().c_str());
            m_lexer->discardUntilRequest();
        }
        catch(...)
        {
            // Other errors should pass through, but we should make sure to pop
            // the stream first!
            m_lexer->popInput();
            throw;
        }
    }
    m_lexer->popInput();
}

namespace { // Handler helpers

template<typename T>
inline const T& toFloatBasedType(Ri::FloatArray a,
                          const char* desc = 0, size_t ncomps = 0)
{
    if(desc && ncomps != a.size())
        AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken,
                            "wrong number of components for " << desc);
    return *reinterpret_cast<const T*>(a.begin());
}

} // anon. namespace

/// Read in a renderman parameter list from the lexer.
Ri::ParamList RibParser::readParamList(RibLexer& lex)
{
    m_paramListStorage.clear();
    while(lex.peekNextType() != RibLexer::Tok_RequestEnd)
    {
        CqPrimvarToken tok;
        tok = m_tokenDict.parseAndLookup(lex.getString());
        switch(tok.storageType())
        {
            case type_integer:
                m_paramListStorage.push_back(Ri::Param(tok, lex.getIntParam()));
                break;
            case type_float:
                m_paramListStorage.push_back(Ri::Param(tok, lex.getFloatParam()));
                break;
            case type_string:
                m_paramListStorage.push_back(Ri::Param(tok, lex.getStringParam()));
                break;
            case type_invalid:
            default:
                assert(0 && "Unknown storage type; we should never get here.");
                break;
        }
    }
    if(m_paramListStorage.empty())
        return Ri::ParamList();
    else
        return Ri::ParamList(&m_paramListStorage[0], m_paramListStorage.size());
}


/// Retrieve a spline basis array from the lexer
///
/// A spline basis array can be specified in two ways in a RIB stream: as an
/// array of 16 floats, or as a string indicating one of the standard bases.
/// This function returns the appropriate basis array, translating the string
/// representation into one of the standard bases if necessary.
///
/// \param lex - read input from here.
///
RtConstBasis& RibParser::getBasis(RibLexer& lex) const
{
    switch(lex.peekNextType())
    {
        case RibLexer::Tok_Array:
            {
                Ri::FloatArray basis = lex.getFloatArray();
                if(basis.size() != 16)
                    AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
                        "basis array must be of length 16");
                // Ugly, but should be safe except unless a compiler fails to
                // lay out float[4][4] the same as float[16]
                return *reinterpret_cast<RtConstBasis*>(basis.begin());
            }
        case RibLexer::Tok_String:
            {
                const char* name = lex.getString();
                RtConstBasis* basis = m_renderer.GetBasis(name);
                if(!basis)
                {
                    AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken,
                        "unknown basis \"" << name << "\"");
                }
                return *basis;
            }
        default:
            AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
                "expected string or float array for basis");
            static RtConstBasis dummy = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
            return dummy; // Kill compiler warnings
    }
}

//--------------------------------------------------
// Hand-written handler implementations.  Mostly these are hard to autogenerate.

void RibParser::handleVersion(RibLexer& lex)
{
    lex.getFloat();
    // Don't do anything with the version number; just blunder on regardless.
    // Probably only worth supporting if Pixar started publishing new versions
    // of the standard again...
}

void RibParser::handleDeclare(RibLexer& lex)
{
    // Collect arguments from lex.
    const char* name = lex.getString();
    const char* declaration = lex.getString();

    // TODO: Refactor so we don't need m_tokenDict.
    m_tokenDict.insert(CqPrimvarToken(declaration, name));

    m_renderer.Declare(name, declaration);
}

void RibParser::handleDepthOfField(RibLexer& lex)
{
    if(lex.peekNextType() == RibLexer::Tok_RequestEnd)
    {
        // If called without arguments, reset to the default pinhole camera.
        m_renderer.DepthOfField(FLT_MAX, FLT_MAX, FLT_MAX);
    }
    else
    {
        // Collect arguments from lex.
        RtFloat fstop = lex.getFloat();
        RtFloat focallength = lex.getFloat();
        RtFloat focaldistance = lex.getFloat();

        m_renderer.DepthOfField(fstop, focallength, focaldistance);
    }
}

void RibParser::handleColorSamples(RibLexer& lex)
{
    // Collect arguments from lex.
    Ri::FloatArray nRGB = lex.getFloatArray();
    Ri::FloatArray RGBn = lex.getFloatArray();

    m_renderer.ColorSamples(nRGB, RGBn);
    m_numColorComps = nRGB.size()/3;
}

/// Handle either LightSource or AreaLightSource
///
/// Both of these share the same RIB arguments and handler requirements, so the
/// code is consolidated in this function.
///
/// \param lightSourceFunc - Interface function to LightSource or AreaLightSource
/// \param lex - lexer from which to read the arguments
///
void RibParser::handleLightSourceGeneral(LightSourceFunc lightSourceFunc,
                                         RibLexer& lex)
{
    // Collect arguments from lex.
    const char* name = lex.getString();

    int sequencenumber = 0;
    // The RISpec says that lights are identified by a 'sequence number', but
    // string identifiers are also allowed in common implementations.
    const char* lightName = 0;
    if(lex.peekNextType() == RibLexer::Tok_String)
        lightName = lex.getString();
    else
        sequencenumber = lex.getInt();

    // Extract the parameter list
    Ri::ParamList paramList = readParamList(lex);

    // Call through to renderer
    RtLightHandle lightHandle = (m_renderer.*lightSourceFunc)(name, paramList);

    // associate handle with the sequence number/name.
    if(lightHandle)
    {
        if(lightName)
            m_namedLightMap[lightName] = lightHandle;
        else
            m_lightMap[sequencenumber] = lightHandle;
    }
}

void RibParser::handleLightSource(RibLexer& lex)
{
    handleLightSourceGeneral(&Ri::Renderer::LightSource, lex);
}

void RibParser::handleAreaLightSource(RibLexer& lex)
{
    handleLightSourceGeneral(&Ri::Renderer::AreaLightSource, lex);
}

void RibParser::handleIlluminate(RibLexer& lex)
{
    // Collect arguments from lex.
    RtLightHandle lightHandle = 0;
    if(lex.peekNextType() == RibLexer::Tok_String)
    {
        // Handle string light names
        const char* name = lex.getString();
        NamedLightMap::const_iterator pos = m_namedLightMap.find(name);
        if(pos == m_namedLightMap.end())
            AQSIS_THROW_XQERROR(XqParseError, EqE_BadHandle,
                                "undeclared light name \"" << name << "\"");
        lightHandle = pos->second;
    }
    else
    {
        // Handle integer sequence numbers
        int sequencenumber = lex.getInt();
        LightMap::const_iterator pos = m_lightMap.find(sequencenumber);
        if(pos == m_lightMap.end())
            AQSIS_THROW_XQERROR(XqParseError, EqE_BadHandle,
                                "undeclared light number " << sequencenumber);
        lightHandle = pos->second;
    }
    RtInt onoff = lex.getInt();

    // Call through to renderer
    m_renderer.Illuminate(lightHandle, onoff);
}

void RibParser::handleSubdivisionMesh(RibLexer& lex)
{
    // Collect arguments from lex.
    const char* scheme = lex.getString();
    Ri::IntArray nvertices = lex.getIntArray();
    Ri::IntArray vertices  = lex.getIntArray();

    if(lex.peekNextType() == RibLexer::Tok_Array)
    {
        // Handle the four optional arguments.
        Ri::StringArray tags      = lex.getStringArray();
        Ri::IntArray    nargs     = lex.getIntArray();
        Ri::IntArray    intargs   = lex.getIntArray();
        Ri::FloatArray  floatargs = lex.getFloatArray();
        // Extract parameter list
        Ri::ParamList paramList = readParamList(lex);
        // Call through to renderer
        m_renderer.SubdivisionMesh(scheme, nvertices, vertices,
                                   tags, nargs, intargs, floatargs,
                                   paramList);
    }
    else
    {
        // Else call version with empty optional args.
        Ri::StringArray  tags;
        Ri::IntArray     nargs;
        Ri::IntArray     intargs;
        Ri::FloatArray   floatargs;
        // Extract parameter list
        Ri::ParamList paramList = readParamList(lex);
        // Call through to renderer
        m_renderer.SubdivisionMesh(scheme, nvertices, vertices,
                                   tags, nargs, intargs, floatargs,
                                   paramList);
    }
}

void RibParser::handleHyperboloid(RibLexer& lex)
{
    // Collect required args as an array
    Ri::FloatArray allArgs = lex.getFloatArray(7);
    RtConstPoint& point1 = *reinterpret_cast<RtConstPoint*>(&allArgs[0]);
    RtConstPoint& point2 = *reinterpret_cast<RtConstPoint*>(&allArgs[3]);
    RtFloat thetamax = allArgs[6];
    // Extract the parameter list
    Ri::ParamList paramList = readParamList(lex);
    // Call through to renderer
    m_renderer.Hyperboloid(point1, point2, thetamax, paramList);
}

void RibParser::handleProcedural(RibLexer& lex)
{
    // get procedural subdivision function
    const char* procName = lex.getString();
    RtProcSubdivFunc subdivideFunc = m_renderer.GetProcSubdivFunc(procName);
    if(!subdivideFunc)
    {
        AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken,
                            "unknown procedural function \"" << procName << "\"");
    }

    // get argument string array.
    Ri::StringArray args = lex.getStringArray();
    // Convert the string array to something passable as data arguments to the
    // builtin procedurals.
    //
    // We jump through a few hoops to meet the spec here.  The data argument to
    // the builtin procedurals should be interpretable as an array of RtString,
    // which we somehow also want to be free()'able.  If we choose to use
    // RiProcFree(), we must allocate it in one big lump.  Ugh.
    size_t dataSize = 0;
    TqInt numArgs = args.size();
    for(TqInt i = 0; i < numArgs; ++i)
    {
        dataSize += sizeof(RtString);   // one pointer for this entry
        dataSize += std::strlen(args[i]) + 1; // and space for the string
    }
    RtPointer procData = reinterpret_cast<RtPointer>(malloc(dataSize));
    RtString stringstart = reinterpret_cast<RtString>(
            reinterpret_cast<RtString*>(procData) + numArgs);
    for(TqInt i = 0; i < numArgs; ++i)
    {
        reinterpret_cast<RtString*>(procData)[i] = stringstart;
        std::strcpy(stringstart, args[i]);
        stringstart += std::strlen(args[i]) + 1;
    }

    // get the procedural bound
    RtConstBound& bound = toFloatBasedType<RtConstBound>(lex.getFloatArray(),
                                                         "bound", 6);

    m_renderer.Procedural(procData, bound, subdivideFunc, m_renderer.GetProcFreeFunc());
}

void RibParser::handleObjectBegin(RibLexer& lex)
{
    // The RIB identifier is an integer according to the RISpec, but it's
    // common to also allow string identifiers, hence the branch here.
    if(lex.peekNextType() == RibLexer::Tok_String)
    {
        const char* lightName = lex.getString();
        if(RtObjectHandle handle = m_renderer.ObjectBegin())
            m_namedObjectMap[lightName] = handle;
    }
    else
    {
        int sequenceNumber = lex.getInt();
        if(RtObjectHandle handle = m_renderer.ObjectBegin())
            m_objectMap[sequenceNumber] = handle;
    }
}

void RibParser::handleObjectInstance(RibLexer& lex)
{
    if(lex.peekNextType() == RibLexer::Tok_String)
    {
        const char* name = lex.getString();
        NamedObjectMap::const_iterator pos = m_namedObjectMap.find(name);
        if(pos == m_namedObjectMap.end())
            AQSIS_THROW_XQERROR(XqParseError, EqE_BadHandle,
                    "undeclared object name \"" << name << "\"");
        m_renderer.ObjectInstance(pos->second);
    }
    else
    {
        int sequencenumber = lex.getInt();
        ObjectMap::const_iterator pos = m_objectMap.find(sequencenumber);
        if(pos == m_objectMap.end())
            AQSIS_THROW_XQERROR(XqParseError, EqE_BadHandle,
                    "undeclared object number " << sequencenumber);
        m_renderer.ObjectInstance(pos->second);
    }
}


//--------------------------------------------------
// Request handlers with autogenerated implementations.
/*[[[cog
import cog
import sys, os
from cogutils import *
from Cheetah.Template import Template

riXml = parseXmlTree('ri.xml')

# Map from RI types to strings which retrieve the value from the lexer
getterStatements = {
    'RtBoolean':     'RtInt %s = lex.getInt();',
    'RtInt':         'RtInt %s = lex.getInt();',
    'RtIntArray':    'Ri::IntArray %s = lex.getIntArray();',
    'RtFloat':       'RtFloat %s = lex.getFloat();',
    'RtFloatArray':  'Ri::FloatArray %s = lex.getFloatArray();',
    'RtString':      'const char* %s = lex.getString();',
    'RtStringArray': 'Ri::StringArray %s = lex.getStringArray();',
    'RtToken':       'const char* %s = lex.getString();',
    'RtTokenArray':  'Ri::StringArray %s = lex.getStringArray();',
    'RtColor':       'RtConstColor& %s = toFloatBasedType<RtConstColor>(lex.getFloatArray(m_numColorComps));',
    'RtPoint':       'RtConstPoint& %s = toFloatBasedType<RtConstPoint>(lex.getFloatArray(), "Point", 3);',
    'RtMatrix':      'RtConstMatrix& %s = toFloatBasedType<RtConstMatrix>(lex.getFloatArray(), "Matrix", 16);',
    'RtBound':       'RtConstBound& %s = toFloatBasedType<RtConstBound>(lex.getFloatArray(6));',
    'RtFilterFunc':  'RtFilterFunc %s = m_renderer.GetFilterFunc(lex.getString());',
    'RtArchiveCallback': 'RtArchiveCallback %s = 0;',
    'RtErrorFunc':   'RtErrorFunc %s = m_renderer.GetErrorFunc(lex.getString());',
    'RtBasis':       'RtConstBasis& %s = getBasis(lex);',
}

customImpl = set(['Declare', 'DepthOfField', 'ColorSamples', 'LightSource',
                  'AreaLightSource', 'Illuminate', 'SubdivisionMesh',
                  'Hyperboloid', 'Procedural', 'ObjectBegin', 'ObjectInstance'])

# Ignore procs which have custom implementations.
procs = filter(lambda p: p.haschild('Rib') and p.findtext('Name') not in customImpl,
               riXml.findall('Procedures/Procedure'))

handlerTemplate = '''
#for $proc in $procs
    #set $procName = $proc.findtext('Name')
    #set $args = filter(lambda a: not a.haschild('RibValue'), $proc.findall('Arguments/Argument'))
    ## 
void RibParser::handle${procName}(RibLexer& lex)
{
    #if $proc.haschild('Arguments/RibArgsCanBeArray')
    ## Collect all args as an array
    Ri::FloatArray allArgs = lex.getFloatArray(${len($args)});
        #for $i, $arg in enumerate($args)
    RtFloat $arg.findtext('Name') = allArgs[$i];
        #end for
    #else
    ## Collect individual arguments from lexer
        #for $arg in $args
    ${getterStatements[$arg.findtext('Type')] % ($arg.findtext('Name'),)}
        #end for
    #end if
    #if $proc.haschild('Arguments/ParamList')
    ## Extract parameter list
    Ri::ParamList paramList = readParamList(lex);
    #end if
    ## Construct the argument list to the C++ interface binding call.
    #set $argList = []
    #for $arg in $args
        #set $argList += [$arg.findtext('Name')]
    #end for
    #if $proc.haschild('Arguments/ParamList')
        #set $argList += ['paramList']
    #end if
    m_renderer.${procName}(${', '.join(argList)});
}

#end for
'''

cog.out(str(Template(handlerTemplate, searchList=locals())));

]]]*/

void RibParser::handleFrameBegin(RibLexer& lex)
{
    RtInt number = lex.getInt();
    m_renderer.FrameBegin(number);
}

void RibParser::handleFrameEnd(RibLexer& lex)
{
    m_renderer.FrameEnd();
}

void RibParser::handleWorldBegin(RibLexer& lex)
{
    m_renderer.WorldBegin();
}

void RibParser::handleWorldEnd(RibLexer& lex)
{
    m_renderer.WorldEnd();
}

void RibParser::handleIfBegin(RibLexer& lex)
{
    const char* condition = lex.getString();
    m_renderer.IfBegin(condition);
}

void RibParser::handleElseIf(RibLexer& lex)
{
    const char* condition = lex.getString();
    m_renderer.ElseIf(condition);
}

void RibParser::handleElse(RibLexer& lex)
{
    m_renderer.Else();
}

void RibParser::handleIfEnd(RibLexer& lex)
{
    m_renderer.IfEnd();
}

void RibParser::handleFormat(RibLexer& lex)
{
    RtInt xresolution = lex.getInt();
    RtInt yresolution = lex.getInt();
    RtFloat pixelaspectratio = lex.getFloat();
    m_renderer.Format(xresolution, yresolution, pixelaspectratio);
}

void RibParser::handleFrameAspectRatio(RibLexer& lex)
{
    RtFloat frameratio = lex.getFloat();
    m_renderer.FrameAspectRatio(frameratio);
}

void RibParser::handleScreenWindow(RibLexer& lex)
{
    Ri::FloatArray allArgs = lex.getFloatArray(4);
    RtFloat left = allArgs[0];
    RtFloat right = allArgs[1];
    RtFloat bottom = allArgs[2];
    RtFloat top = allArgs[3];
    m_renderer.ScreenWindow(left, right, bottom, top);
}

void RibParser::handleCropWindow(RibLexer& lex)
{
    Ri::FloatArray allArgs = lex.getFloatArray(4);
    RtFloat xmin = allArgs[0];
    RtFloat xmax = allArgs[1];
    RtFloat ymin = allArgs[2];
    RtFloat ymax = allArgs[3];
    m_renderer.CropWindow(xmin, xmax, ymin, ymax);
}

void RibParser::handleProjection(RibLexer& lex)
{
    const char* name = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Projection(name, paramList);
}

void RibParser::handleClipping(RibLexer& lex)
{
    RtFloat cnear = lex.getFloat();
    RtFloat cfar = lex.getFloat();
    m_renderer.Clipping(cnear, cfar);
}

void RibParser::handleClippingPlane(RibLexer& lex)
{
    RtFloat x = lex.getFloat();
    RtFloat y = lex.getFloat();
    RtFloat z = lex.getFloat();
    RtFloat nx = lex.getFloat();
    RtFloat ny = lex.getFloat();
    RtFloat nz = lex.getFloat();
    m_renderer.ClippingPlane(x, y, z, nx, ny, nz);
}

void RibParser::handleShutter(RibLexer& lex)
{
    RtFloat opentime = lex.getFloat();
    RtFloat closetime = lex.getFloat();
    m_renderer.Shutter(opentime, closetime);
}

void RibParser::handlePixelVariance(RibLexer& lex)
{
    RtFloat variance = lex.getFloat();
    m_renderer.PixelVariance(variance);
}

void RibParser::handlePixelSamples(RibLexer& lex)
{
    RtFloat xsamples = lex.getFloat();
    RtFloat ysamples = lex.getFloat();
    m_renderer.PixelSamples(xsamples, ysamples);
}

void RibParser::handlePixelFilter(RibLexer& lex)
{
    RtFilterFunc function = m_renderer.GetFilterFunc(lex.getString());
    RtFloat xwidth = lex.getFloat();
    RtFloat ywidth = lex.getFloat();
    m_renderer.PixelFilter(function, xwidth, ywidth);
}

void RibParser::handleExposure(RibLexer& lex)
{
    RtFloat gain = lex.getFloat();
    RtFloat gamma = lex.getFloat();
    m_renderer.Exposure(gain, gamma);
}

void RibParser::handleImager(RibLexer& lex)
{
    const char* name = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Imager(name, paramList);
}

void RibParser::handleQuantize(RibLexer& lex)
{
    const char* type = lex.getString();
    RtInt one = lex.getInt();
    RtInt min = lex.getInt();
    RtInt max = lex.getInt();
    RtFloat ditheramplitude = lex.getFloat();
    m_renderer.Quantize(type, one, min, max, ditheramplitude);
}

void RibParser::handleDisplay(RibLexer& lex)
{
    const char* name = lex.getString();
    const char* type = lex.getString();
    const char* mode = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Display(name, type, mode, paramList);
}

void RibParser::handleHider(RibLexer& lex)
{
    const char* name = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Hider(name, paramList);
}

void RibParser::handleRelativeDetail(RibLexer& lex)
{
    RtFloat relativedetail = lex.getFloat();
    m_renderer.RelativeDetail(relativedetail);
}

void RibParser::handleOption(RibLexer& lex)
{
    const char* name = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Option(name, paramList);
}

void RibParser::handleAttributeBegin(RibLexer& lex)
{
    m_renderer.AttributeBegin();
}

void RibParser::handleAttributeEnd(RibLexer& lex)
{
    m_renderer.AttributeEnd();
}

void RibParser::handleColor(RibLexer& lex)
{
    RtConstColor& Cq = toFloatBasedType<RtConstColor>(lex.getFloatArray(m_numColorComps));
    m_renderer.Color(Cq);
}

void RibParser::handleOpacity(RibLexer& lex)
{
    RtConstColor& Os = toFloatBasedType<RtConstColor>(lex.getFloatArray(m_numColorComps));
    m_renderer.Opacity(Os);
}

void RibParser::handleTextureCoordinates(RibLexer& lex)
{
    Ri::FloatArray allArgs = lex.getFloatArray(8);
    RtFloat s1 = allArgs[0];
    RtFloat t1 = allArgs[1];
    RtFloat s2 = allArgs[2];
    RtFloat t2 = allArgs[3];
    RtFloat s3 = allArgs[4];
    RtFloat t3 = allArgs[5];
    RtFloat s4 = allArgs[6];
    RtFloat t4 = allArgs[7];
    m_renderer.TextureCoordinates(s1, t1, s2, t2, s3, t3, s4, t4);
}

void RibParser::handleSurface(RibLexer& lex)
{
    const char* name = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Surface(name, paramList);
}

void RibParser::handleDisplacement(RibLexer& lex)
{
    const char* name = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Displacement(name, paramList);
}

void RibParser::handleAtmosphere(RibLexer& lex)
{
    const char* name = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Atmosphere(name, paramList);
}

void RibParser::handleInterior(RibLexer& lex)
{
    const char* name = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Interior(name, paramList);
}

void RibParser::handleExterior(RibLexer& lex)
{
    const char* name = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Exterior(name, paramList);
}

void RibParser::handleShaderLayer(RibLexer& lex)
{
    const char* type = lex.getString();
    const char* name = lex.getString();
    const char* layername = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.ShaderLayer(type, name, layername, paramList);
}

void RibParser::handleConnectShaderLayers(RibLexer& lex)
{
    const char* type = lex.getString();
    const char* layer1 = lex.getString();
    const char* variable1 = lex.getString();
    const char* layer2 = lex.getString();
    const char* variable2 = lex.getString();
    m_renderer.ConnectShaderLayers(type, layer1, variable1, layer2, variable2);
}

void RibParser::handleShadingRate(RibLexer& lex)
{
    RtFloat size = lex.getFloat();
    m_renderer.ShadingRate(size);
}

void RibParser::handleShadingInterpolation(RibLexer& lex)
{
    const char* type = lex.getString();
    m_renderer.ShadingInterpolation(type);
}

void RibParser::handleMatte(RibLexer& lex)
{
    RtInt onoff = lex.getInt();
    m_renderer.Matte(onoff);
}

void RibParser::handleBound(RibLexer& lex)
{
    RtConstBound& bound = toFloatBasedType<RtConstBound>(lex.getFloatArray(6));
    m_renderer.Bound(bound);
}

void RibParser::handleDetail(RibLexer& lex)
{
    RtConstBound& bound = toFloatBasedType<RtConstBound>(lex.getFloatArray(6));
    m_renderer.Detail(bound);
}

void RibParser::handleDetailRange(RibLexer& lex)
{
    Ri::FloatArray allArgs = lex.getFloatArray(4);
    RtFloat offlow = allArgs[0];
    RtFloat onlow = allArgs[1];
    RtFloat onhigh = allArgs[2];
    RtFloat offhigh = allArgs[3];
    m_renderer.DetailRange(offlow, onlow, onhigh, offhigh);
}

void RibParser::handleGeometricApproximation(RibLexer& lex)
{
    const char* type = lex.getString();
    RtFloat value = lex.getFloat();
    m_renderer.GeometricApproximation(type, value);
}

void RibParser::handleOrientation(RibLexer& lex)
{
    const char* orientation = lex.getString();
    m_renderer.Orientation(orientation);
}

void RibParser::handleReverseOrientation(RibLexer& lex)
{
    m_renderer.ReverseOrientation();
}

void RibParser::handleSides(RibLexer& lex)
{
    RtInt nsides = lex.getInt();
    m_renderer.Sides(nsides);
}

void RibParser::handleIdentity(RibLexer& lex)
{
    m_renderer.Identity();
}

void RibParser::handleTransform(RibLexer& lex)
{
    RtConstMatrix& transform = toFloatBasedType<RtConstMatrix>(lex.getFloatArray(), "Matrix", 16);
    m_renderer.Transform(transform);
}

void RibParser::handleConcatTransform(RibLexer& lex)
{
    RtConstMatrix& transform = toFloatBasedType<RtConstMatrix>(lex.getFloatArray(), "Matrix", 16);
    m_renderer.ConcatTransform(transform);
}

void RibParser::handlePerspective(RibLexer& lex)
{
    RtFloat fov = lex.getFloat();
    m_renderer.Perspective(fov);
}

void RibParser::handleTranslate(RibLexer& lex)
{
    RtFloat dx = lex.getFloat();
    RtFloat dy = lex.getFloat();
    RtFloat dz = lex.getFloat();
    m_renderer.Translate(dx, dy, dz);
}

void RibParser::handleRotate(RibLexer& lex)
{
    RtFloat angle = lex.getFloat();
    RtFloat dx = lex.getFloat();
    RtFloat dy = lex.getFloat();
    RtFloat dz = lex.getFloat();
    m_renderer.Rotate(angle, dx, dy, dz);
}

void RibParser::handleScale(RibLexer& lex)
{
    RtFloat sx = lex.getFloat();
    RtFloat sy = lex.getFloat();
    RtFloat sz = lex.getFloat();
    m_renderer.Scale(sx, sy, sz);
}

void RibParser::handleSkew(RibLexer& lex)
{
    Ri::FloatArray allArgs = lex.getFloatArray(7);
    RtFloat angle = allArgs[0];
    RtFloat dx1 = allArgs[1];
    RtFloat dy1 = allArgs[2];
    RtFloat dz1 = allArgs[3];
    RtFloat dx2 = allArgs[4];
    RtFloat dy2 = allArgs[5];
    RtFloat dz2 = allArgs[6];
    m_renderer.Skew(angle, dx1, dy1, dz1, dx2, dy2, dz2);
}

void RibParser::handleCoordinateSystem(RibLexer& lex)
{
    const char* space = lex.getString();
    m_renderer.CoordinateSystem(space);
}

void RibParser::handleCoordSysTransform(RibLexer& lex)
{
    const char* space = lex.getString();
    m_renderer.CoordSysTransform(space);
}

void RibParser::handleTransformBegin(RibLexer& lex)
{
    m_renderer.TransformBegin();
}

void RibParser::handleTransformEnd(RibLexer& lex)
{
    m_renderer.TransformEnd();
}

void RibParser::handleResource(RibLexer& lex)
{
    const char* handle = lex.getString();
    const char* type = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Resource(handle, type, paramList);
}

void RibParser::handleResourceBegin(RibLexer& lex)
{
    m_renderer.ResourceBegin();
}

void RibParser::handleResourceEnd(RibLexer& lex)
{
    m_renderer.ResourceEnd();
}

void RibParser::handleAttribute(RibLexer& lex)
{
    const char* name = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Attribute(name, paramList);
}

void RibParser::handlePolygon(RibLexer& lex)
{
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Polygon(paramList);
}

void RibParser::handleGeneralPolygon(RibLexer& lex)
{
    Ri::IntArray nverts = lex.getIntArray();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.GeneralPolygon(nverts, paramList);
}

void RibParser::handlePointsPolygons(RibLexer& lex)
{
    Ri::IntArray nverts = lex.getIntArray();
    Ri::IntArray verts = lex.getIntArray();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.PointsPolygons(nverts, verts, paramList);
}

void RibParser::handlePointsGeneralPolygons(RibLexer& lex)
{
    Ri::IntArray nloops = lex.getIntArray();
    Ri::IntArray nverts = lex.getIntArray();
    Ri::IntArray verts = lex.getIntArray();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.PointsGeneralPolygons(nloops, nverts, verts, paramList);
}

void RibParser::handleBasis(RibLexer& lex)
{
    RtConstBasis& ubasis = getBasis(lex);
    RtInt ustep = lex.getInt();
    RtConstBasis& vbasis = getBasis(lex);
    RtInt vstep = lex.getInt();
    m_renderer.Basis(ubasis, ustep, vbasis, vstep);
}

void RibParser::handlePatch(RibLexer& lex)
{
    const char* type = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Patch(type, paramList);
}

void RibParser::handlePatchMesh(RibLexer& lex)
{
    const char* type = lex.getString();
    RtInt nu = lex.getInt();
    const char* uwrap = lex.getString();
    RtInt nv = lex.getInt();
    const char* vwrap = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.PatchMesh(type, nu, uwrap, nv, vwrap, paramList);
}

void RibParser::handleNuPatch(RibLexer& lex)
{
    RtInt nu = lex.getInt();
    RtInt uorder = lex.getInt();
    Ri::FloatArray uknot = lex.getFloatArray();
    RtFloat umin = lex.getFloat();
    RtFloat umax = lex.getFloat();
    RtInt nv = lex.getInt();
    RtInt vorder = lex.getInt();
    Ri::FloatArray vknot = lex.getFloatArray();
    RtFloat vmin = lex.getFloat();
    RtFloat vmax = lex.getFloat();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.NuPatch(nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax, paramList);
}

void RibParser::handleTrimCurve(RibLexer& lex)
{
    Ri::IntArray ncurves = lex.getIntArray();
    Ri::IntArray order = lex.getIntArray();
    Ri::FloatArray knot = lex.getFloatArray();
    Ri::FloatArray min = lex.getFloatArray();
    Ri::FloatArray max = lex.getFloatArray();
    Ri::IntArray n = lex.getIntArray();
    Ri::FloatArray u = lex.getFloatArray();
    Ri::FloatArray v = lex.getFloatArray();
    Ri::FloatArray w = lex.getFloatArray();
    m_renderer.TrimCurve(ncurves, order, knot, min, max, n, u, v, w);
}

void RibParser::handleSphere(RibLexer& lex)
{
    Ri::FloatArray allArgs = lex.getFloatArray(4);
    RtFloat radius = allArgs[0];
    RtFloat zmin = allArgs[1];
    RtFloat zmax = allArgs[2];
    RtFloat thetamax = allArgs[3];
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Sphere(radius, zmin, zmax, thetamax, paramList);
}

void RibParser::handleCone(RibLexer& lex)
{
    Ri::FloatArray allArgs = lex.getFloatArray(3);
    RtFloat height = allArgs[0];
    RtFloat radius = allArgs[1];
    RtFloat thetamax = allArgs[2];
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Cone(height, radius, thetamax, paramList);
}

void RibParser::handleCylinder(RibLexer& lex)
{
    Ri::FloatArray allArgs = lex.getFloatArray(4);
    RtFloat radius = allArgs[0];
    RtFloat zmin = allArgs[1];
    RtFloat zmax = allArgs[2];
    RtFloat thetamax = allArgs[3];
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Cylinder(radius, zmin, zmax, thetamax, paramList);
}

void RibParser::handleParaboloid(RibLexer& lex)
{
    Ri::FloatArray allArgs = lex.getFloatArray(4);
    RtFloat rmax = allArgs[0];
    RtFloat zmin = allArgs[1];
    RtFloat zmax = allArgs[2];
    RtFloat thetamax = allArgs[3];
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Paraboloid(rmax, zmin, zmax, thetamax, paramList);
}

void RibParser::handleDisk(RibLexer& lex)
{
    Ri::FloatArray allArgs = lex.getFloatArray(3);
    RtFloat height = allArgs[0];
    RtFloat radius = allArgs[1];
    RtFloat thetamax = allArgs[2];
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Disk(height, radius, thetamax, paramList);
}

void RibParser::handleTorus(RibLexer& lex)
{
    Ri::FloatArray allArgs = lex.getFloatArray(5);
    RtFloat majorrad = allArgs[0];
    RtFloat minorrad = allArgs[1];
    RtFloat phimin = allArgs[2];
    RtFloat phimax = allArgs[3];
    RtFloat thetamax = allArgs[4];
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Torus(majorrad, minorrad, phimin, phimax, thetamax, paramList);
}

void RibParser::handlePoints(RibLexer& lex)
{
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Points(paramList);
}

void RibParser::handleCurves(RibLexer& lex)
{
    const char* type = lex.getString();
    Ri::IntArray nvertices = lex.getIntArray();
    const char* wrap = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Curves(type, nvertices, wrap, paramList);
}

void RibParser::handleBlobby(RibLexer& lex)
{
    RtInt nleaf = lex.getInt();
    Ri::IntArray code = lex.getIntArray();
    Ri::FloatArray flt = lex.getFloatArray();
    Ri::StringArray str = lex.getStringArray();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Blobby(nleaf, code, flt, str, paramList);
}

void RibParser::handleGeometry(RibLexer& lex)
{
    const char* type = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.Geometry(type, paramList);
}

void RibParser::handleSolidBegin(RibLexer& lex)
{
    const char* type = lex.getString();
    m_renderer.SolidBegin(type);
}

void RibParser::handleSolidEnd(RibLexer& lex)
{
    m_renderer.SolidEnd();
}

void RibParser::handleObjectEnd(RibLexer& lex)
{
    m_renderer.ObjectEnd();
}

void RibParser::handleMotionBegin(RibLexer& lex)
{
    Ri::FloatArray times = lex.getFloatArray();
    m_renderer.MotionBegin(times);
}

void RibParser::handleMotionEnd(RibLexer& lex)
{
    m_renderer.MotionEnd();
}

void RibParser::handleMakeTexture(RibLexer& lex)
{
    const char* imagefile = lex.getString();
    const char* texturefile = lex.getString();
    const char* swrap = lex.getString();
    const char* twrap = lex.getString();
    RtFilterFunc filterfunc = m_renderer.GetFilterFunc(lex.getString());
    RtFloat swidth = lex.getFloat();
    RtFloat twidth = lex.getFloat();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.MakeTexture(imagefile, texturefile, swrap, twrap, filterfunc, swidth, twidth, paramList);
}

void RibParser::handleMakeLatLongEnvironment(RibLexer& lex)
{
    const char* imagefile = lex.getString();
    const char* reflfile = lex.getString();
    RtFilterFunc filterfunc = m_renderer.GetFilterFunc(lex.getString());
    RtFloat swidth = lex.getFloat();
    RtFloat twidth = lex.getFloat();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.MakeLatLongEnvironment(imagefile, reflfile, filterfunc, swidth, twidth, paramList);
}

void RibParser::handleMakeCubeFaceEnvironment(RibLexer& lex)
{
    const char* px = lex.getString();
    const char* nx = lex.getString();
    const char* py = lex.getString();
    const char* ny = lex.getString();
    const char* pz = lex.getString();
    const char* nz = lex.getString();
    const char* reflfile = lex.getString();
    RtFloat fov = lex.getFloat();
    RtFilterFunc filterfunc = m_renderer.GetFilterFunc(lex.getString());
    RtFloat swidth = lex.getFloat();
    RtFloat twidth = lex.getFloat();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.MakeCubeFaceEnvironment(px, nx, py, ny, pz, nz, reflfile, fov, filterfunc, swidth, twidth, paramList);
}

void RibParser::handleMakeShadow(RibLexer& lex)
{
    const char* picfile = lex.getString();
    const char* shadowfile = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.MakeShadow(picfile, shadowfile, paramList);
}

void RibParser::handleMakeOcclusion(RibLexer& lex)
{
    Ri::StringArray picfiles = lex.getStringArray();
    const char* shadowfile = lex.getString();
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.MakeOcclusion(picfiles, shadowfile, paramList);
}

void RibParser::handleErrorHandler(RibLexer& lex)
{
    RtErrorFunc handler = m_renderer.GetErrorFunc(lex.getString());
    m_renderer.ErrorHandler(handler);
}

void RibParser::handleReadArchive(RibLexer& lex)
{
    const char* name = lex.getString();
    RtArchiveCallback callback = 0;
    Ri::ParamList paramList = readParamList(lex);
    m_renderer.ReadArchive(name, callback, paramList);
}

//[[[end]]]


//--------------------------------------------------

} // namespace Aqsis
// vi: set et:
