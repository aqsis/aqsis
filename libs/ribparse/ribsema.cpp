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

#include <boost/shared_ptr.hpp>

namespace Aqsis
{

//------------------------------------------------------------------------------
// RibSema implementation

RibSema::RibSema(Ri::Renderer& renderer)
    : m_renderer(renderer),
    m_requestHandlerMap(),
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


//--------------------------------------------------
// Handler helper utilities.
namespace {

template<typename T>
const T* get(const std::vector<T>& v) { return v.empty() ? 0 : &v[0]; }

//------------------------------------------------------------------------------
/// Implementation of IqRibParamListHandler to read in parameter lists
///
/// This implementation reads a parameter list from the parser and translates it
/// into the (count, tokens, values) triple which is accepted by the Ri*V form
/// of the renderman interface functions.
///
class ParamAccumulator : public IqRibParamListHandler
{
    private:
        // Dictionary for looking up RiDeclare'd and standard tokens
        const CqTokenDictionary& m_tokenDict;
        std::vector<Ri::Param> m_params;
        // Storage for vectors of strings
        std::vector<boost::shared_ptr<std::vector<RtConstToken> > > m_stringValues;

        const RtConstToken* cacheStringArray(const IqRibParser::TqStringArray& strings)
        {
            m_stringValues.push_back(
                    boost::shared_ptr<std::vector<RtConstToken> >(
                    new std::vector<RtConstToken>(strings.size(), 0)) );
            std::vector<RtConstToken>& stringsDest = *m_stringValues.back();
            for(TqInt i = 0, end = strings.size(); i < end; ++i)
                stringsDest[i] = strings[i].c_str();
            return get(stringsDest);
        }

    public:
        /// Construct an empty param list
        ParamAccumulator(const CqTokenDictionary& tokenDict) :
            m_tokenDict(tokenDict),
            m_params(),
            m_stringValues()
        { }

        /// Read in the next (token,value) pair from the parser
        virtual void readParameter(const std::string& name, IqRibParser& parser)
        {
            CqPrimvarToken tok;
            try
            {
                tok = m_tokenDict.parseAndLookup(name);
            }
            catch(XqValidation& e)
            {
                AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax, e.what());
            }
            switch(tok.storageType())
            {
                case type_integer:
                    {
                        const IqRibParser::TqIntArray& a = parser.getIntParam();
                        m_params.push_back(Ri::Param(tok, get(a), a.size()));
                    }
                    break;
                case type_float:
                    {
                        const IqRibParser::TqFloatArray& a = parser.getFloatParam();
                        m_params.push_back(Ri::Param(tok, get(a), a.size()));
                    }
                    break;
                case type_string:
                    {
                        const IqRibParser::TqStringArray& a = parser.getStringParam();
                        m_params.push_back(Ri::Param(tok, cacheStringArray(a), a.size()));
                    }
                    break;
                case type_invalid:
                default:
                    assert(0 && "Unknown storage type; we should never get here.");
                    return;
            }
        }

        operator Ri::ParamList()
        {
            return Ri::ParamList(get(m_params), m_params.size());
        }
};

//--------------------------------------------------
// Conversion helpers from the various types coming from the parser into types
// which can be passed into Ri::Renderer
class StringHolder
{
    private:
        std::string m_string;
    public:
        StringHolder(const std::string& string) : m_string(string) {}

        operator RtConstString()
        {
            return m_string.c_str();
        }
};

/// A conversion class, converting IqRibParser::TqStringArray to
/// Ri::TokenArray
class StringArrayHolder
{
    private:
        std::vector<RtConstToken> m_strings;

    public:
        /// Extract a vector of RtToken pointers from a vector of std::strings,
        /// storing them in the storage array.
        StringArrayHolder(const std::vector<std::string>& strings)
        {
            m_strings.reserve(strings.size());
            for(IqRibParser::TqStringArray::const_iterator i = strings.begin(),
                    end = strings.end(); i != end; ++i)
            {
                m_strings.push_back(i->c_str());
            }
        }

        // Conversion to the associated Ri::Array type
        operator Ri::TokenArray()
        {
            return Ri::TokenArray(get(m_strings), m_strings.size());
        }
};

template<typename T>
Ri::Array<T> toRiArray(const std::vector<T>& v)
{
    return Ri::Array<T>(get(v), v.size());
}

template<typename T>
const T& toFloatBasedType(const std::vector<RtFloat>& v,
                          const char* desc = 0, size_t ncomps = 0)
{
    if(desc && ncomps != v.size())
        AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken,
                            "wrong number of components for " << desc);
    return *reinterpret_cast<const T*>(get(v));
}


} // anon. namespace


/// Retrieve a spline basis array from the parser
///
/// A spline basis array can be specified in two ways in a RIB stream: as an
/// array of 16 floats, or as a string indicating one of the standard bases.
/// This function returns the appropriate basis array, translating the string
/// representation into one of the standard bases if necessary.
///
/// \param parser - read input from here.
///
RtConstBasis& RibSema::getBasis(IqRibParser& parser) const
{
    switch(parser.peekNextType())
    {
        case IqRibParser::Tok_Array:
            {
                const IqRibParser::TqFloatArray& basis = parser.getFloatArray();
                if(basis.size() != 16)
                    AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
                        "basis array must be of length 16");
                // Ugly, but should be safe except unless a compiler fails to
                // lay out float[4][4] the same as float[16]
                return *reinterpret_cast<RtConstBasis*>(get(basis));
            }
        case IqRibParser::Tok_String:
            {
                std::string name = parser.getString();
                RtConstBasis* basis = m_renderer.GetBasis(name.c_str());
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


void RibSema::handleVersion(IqRibParser& parser)
{
    parser.getFloat();
    // Don't do anything with the version number; just blunder on regardless.
    // Probably only worth supporting if Pixar started publishing new versions
    // of the standard again...
}


void RibSema::handleDeclare(IqRibParser& parser)
{
    // Collect arguments from parser.
    StringHolder name = parser.getString();
    StringHolder declaration = parser.getString();

    // TODO: Refactor so we don't need m_tokenDict.
    m_tokenDict.insert(CqPrimvarToken(declaration, name));

    m_renderer.Declare(name, declaration);
}

void RibSema::handleDepthOfField(IqRibParser& parser)
{
    if(parser.peekNextType() == IqRibParser::Tok_RequestEnd)
    {
        // If called without arguments, reset to the default pinhole camera.
        m_renderer.DepthOfField(FLT_MAX, FLT_MAX, FLT_MAX);
    }
    else
    {
        // Collect arguments from parser.
        RtFloat fstop = parser.getFloat();
        RtFloat focallength = parser.getFloat();
        RtFloat focaldistance = parser.getFloat();

        m_renderer.DepthOfField(fstop, focallength, focaldistance);
    }
}

void RibSema::handleColorSamples(IqRibParser& parser)
{
    // Collect arguments from parser.
    Ri::FloatArray nRGB = toRiArray(parser.getFloatArray());
    Ri::FloatArray RGBn = toRiArray(parser.getFloatArray());

    m_renderer.ColorSamples(nRGB, RGBn);
    m_numColorComps = nRGB.length/3;
}

/// Handle either LightSource or AreaLightSource
///
/// Both of these share the same RIB arguments and handler requirements, so the
/// code is consolidated in this function.
///
/// \param lightSourceFunc - Interface function to LightSource or AreaLightSource
/// \param parser - parser from which to read the arguments
///
void RibSema::handleLightSourceGeneral(LightSourceFunc lightSourceFunc,
                                       IqRibParser& parser)
{
    // Collect arguments from parser.
    StringHolder name = parser.getString();

    int sequencenumber = 0;
    // The RISpec says that lights are identified by a 'sequence number', but
    // string identifiers are also allowed in common implementations.
    std::string lightName;
    bool useLightName = false;
    if(parser.peekNextType() == IqRibParser::Tok_String)
    {
        lightName = parser.getString();
        useLightName = true;
    }
    else
        sequencenumber = parser.getInt();

    // Extract the parameter list
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);

    // Call through to renderer
    RtLightHandle lightHandle = (m_renderer.*lightSourceFunc)(name, paramList);

    // associate handle with the sequence number/name.
    if(lightHandle)
    {
        if(useLightName)
            m_namedLightMap[lightName] = lightHandle;
        else
            m_lightMap[sequencenumber] = lightHandle;
    }
}

void RibSema::handleLightSource(IqRibParser& parser)
{
    handleLightSourceGeneral(&Ri::Renderer::LightSource, parser);
}

void RibSema::handleAreaLightSource(IqRibParser& parser)
{
    handleLightSourceGeneral(&Ri::Renderer::AreaLightSource, parser);
}

void RibSema::handleIlluminate(IqRibParser& parser)
{
    // Collect arguments from parser.
    RtLightHandle lightHandle = 0;
    if(parser.peekNextType() == IqRibParser::Tok_String)
    {
        // Handle string light names
        std::string name = parser.getString();
        NamedLightMap::const_iterator pos = m_namedLightMap.find(name);
        if(pos == m_namedLightMap.end())
            AQSIS_THROW_XQERROR(XqParseError, EqE_BadHandle,
                                "undeclared light name \"" << name << "\"");
        lightHandle = pos->second;
    }
    else
    {
        // Handle integer sequence numbers
        int sequencenumber = parser.getInt();
        LightMap::const_iterator pos = m_lightMap.find(sequencenumber);
        if(pos == m_lightMap.end())
            AQSIS_THROW_XQERROR(XqParseError, EqE_BadHandle,
                                "undeclared light number " << sequencenumber);
        lightHandle = pos->second;
    }
    RtInt onoff = parser.getInt();

    // Call through to renderer
    m_renderer.Illuminate(lightHandle, onoff);
}

void RibSema::handleSubdivisionMesh(IqRibParser& parser)
{
    // Collect arguments from parser.
    StringHolder scheme = parser.getString();
    Ri::IntArray nvertices = toRiArray(parser.getIntArray());
    Ri::IntArray vertices  = toRiArray(parser.getIntArray());

    if(parser.peekNextType() == IqRibParser::Tok_Array)
    {
        // Handle the four optional arguments.
        StringArrayHolder  tags      = parser.getStringArray();
        Ri::IntArray   nargs     = toRiArray(parser.getIntArray());
        Ri::IntArray   intargs   = toRiArray(parser.getIntArray());
        Ri::FloatArray floatargs = toRiArray(parser.getFloatArray());
        // Extract parameter list
        ParamAccumulator paramList(m_tokenDict);
        parser.getParamList(paramList);
        // Call through to renderer
        m_renderer.SubdivisionMesh(scheme, nvertices, vertices,
                                   tags, nargs, intargs, floatargs,
                                   paramList);
    }
    else
    {
        // Else call version with empty optional args.
        Ri::StringArray  tags;
        Ri::IntArray          nargs;
        Ri::IntArray          intargs;
        Ri::FloatArray        floatargs;
        // Extract parameter list
        ParamAccumulator paramList(m_tokenDict);
        parser.getParamList(paramList);
        // Call through to renderer
        m_renderer.SubdivisionMesh(scheme, nvertices, vertices,
                                   tags, nargs, intargs, floatargs,
                                   paramList);
    }
}

void RibSema::handleHyperboloid(IqRibParser& parser)
{
    // Collect required args as an array
    const IqRibParser::TqFloatArray& allArgs = parser.getFloatArray(7);
    RtConstPoint& point1 = *reinterpret_cast<RtConstPoint*>(&allArgs[0]);
    RtConstPoint& point2 = *reinterpret_cast<RtConstPoint*>(&allArgs[3]);
    RtFloat thetamax = allArgs[6];
    // Extract the parameter list
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    // Call through to renderer
    m_renderer.Hyperboloid(point1, point2, thetamax, paramList);
}

void RibSema::handleProcedural(IqRibParser& parser)
{
    // get procedural subdivision function
    std::string procName = parser.getString();
    RtProcSubdivFunc subdivideFunc = m_renderer.GetProcSubdivFunc(procName.c_str());
    if(!subdivideFunc)
    {
        AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken,
                            "unknown procedural function \"" << procName << "\"");
    }

    // get argument string array.
    const IqRibParser::TqStringArray& args = parser.getStringArray();
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
        dataSize += args[i].size() + 1; // and space for the string
    }
    RtPointer procData = reinterpret_cast<RtPointer>(malloc(dataSize));
    RtString stringstart = reinterpret_cast<RtString>(
            reinterpret_cast<RtString*>(procData) + numArgs);
    for(TqInt i = 0; i < numArgs; ++i)
    {
        reinterpret_cast<RtString*>(procData)[i] = stringstart;
        std::strcpy(stringstart, args[i].c_str());
        stringstart += args[i].size() + 1;
    }

    // get the procedural bound
    RtConstBound& bound = toFloatBasedType<RtConstBound>(parser.getFloatArray(),
                                                         "bound", 6);

    m_renderer.Procedural(procData, bound, subdivideFunc, m_renderer.GetProcFreeFunc());
}

void RibSema::handleObjectBegin(IqRibParser& parser)
{
    // The RIB identifier is an integer according to the RISpec, but it's
    // common to also allow string identifiers, hence the branch here.
    if(parser.peekNextType() == IqRibParser::Tok_String)
    {
        std::string lightName = parser.getString();
        if(RtObjectHandle handle = m_renderer.ObjectBegin())
            m_namedObjectMap[lightName] = handle;
    }
    else
    {
        int sequenceNumber = parser.getInt();
        if(RtObjectHandle handle = m_renderer.ObjectBegin())
            m_objectMap[sequenceNumber] = handle;
    }
}

void RibSema::handleObjectInstance(IqRibParser& parser)
{
    if(parser.peekNextType() == IqRibParser::Tok_String)
    {
        std::string name = parser.getString();
        NamedObjectMap::const_iterator pos = m_namedObjectMap.find(name);
        if(pos == m_namedObjectMap.end())
            AQSIS_THROW_XQERROR(XqParseError, EqE_BadHandle,
                    "undeclared object name \"" << name << "\"");
        m_renderer.ObjectInstance(pos->second);
    }
    else
    {
        int sequencenumber = parser.getInt();
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

# Map from RI types to strings which retrieve the value from the parser
getterStatements = {
    'RtBoolean':     'RtInt %s = parser.getInt();',
    'RtInt':         'RtInt %s = parser.getInt();',
    'RtIntArray':    'Ri::IntArray %s = toRiArray(parser.getIntArray());',
    'RtFloat':       'RtFloat %s = parser.getFloat();',
    'RtFloatArray':  'Ri::FloatArray %s = toRiArray(parser.getFloatArray());',
    'RtString':      'StringHolder %s = parser.getString();',
    'RtStringArray': 'StringArrayHolder %s = parser.getStringArray();',
    'RtToken':       'StringHolder %s = parser.getString();',
    'RtTokenArray':  'StringArrayHolder %s = parser.getStringArray();',
    'RtColor':       'RtConstColor& %s = toFloatBasedType<RtConstColor>(parser.getFloatArray(m_numColorComps));',
    'RtPoint':       'RtConstPoint& %s = toFloatBasedType<RtConstPoint>(parser.getFloatArray(), "Point", 3);',
    'RtMatrix':      'RtConstMatrix& %s = toFloatBasedType<RtConstMatrix>(parser.getFloatArray(), "Matrix", 16);',
    'RtBound':       'RtConstBound& %s = toFloatBasedType<RtConstBound>(parser.getFloatArray(6));',
    'RtFilterFunc':  'RtFilterFunc %s = m_renderer.GetFilterFunc(parser.getString().c_str());',
    'RtArchiveCallback': 'RtArchiveCallback %s = 0;',
    'RtErrorFunc':   'RtErrorFunc %s = m_renderer.GetErrorFunc(parser.getString().c_str());',
    'RtBasis':       'RtConstBasis& %s = getBasis(parser);',
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
void RibSema::handle${procName}(IqRibParser& parser)
{
    #if $proc.haschild('Arguments/RibArgsCanBeArray')
    ## Collect all args as an array
    const IqRibParser::TqFloatArray& allArgs = parser.getFloatArray(${len($args)});
        #for $i, $arg in enumerate($args)
    RtFloat $arg.findtext('Name') = allArgs[$i];
        #end for
    #else
    ## Collect individual arguments from parser
        #for $arg in $args
    ${getterStatements[$arg.findtext('Type')] % ($arg.findtext('Name'),)}
        #end for
    #end if
    #if $proc.haschild('Arguments/ParamList')
    ## Extract parameter list
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
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

void RibSema::handleFrameBegin(IqRibParser& parser)
{
    RtInt number = parser.getInt();
    m_renderer.FrameBegin(number);
}

void RibSema::handleFrameEnd(IqRibParser& parser)
{
    m_renderer.FrameEnd();
}

void RibSema::handleWorldBegin(IqRibParser& parser)
{
    m_renderer.WorldBegin();
}

void RibSema::handleWorldEnd(IqRibParser& parser)
{
    m_renderer.WorldEnd();
}

void RibSema::handleIfBegin(IqRibParser& parser)
{
    StringHolder condition = parser.getString();
    m_renderer.IfBegin(condition);
}

void RibSema::handleElseIf(IqRibParser& parser)
{
    StringHolder condition = parser.getString();
    m_renderer.ElseIf(condition);
}

void RibSema::handleElse(IqRibParser& parser)
{
    m_renderer.Else();
}

void RibSema::handleIfEnd(IqRibParser& parser)
{
    m_renderer.IfEnd();
}

void RibSema::handleFormat(IqRibParser& parser)
{
    RtInt xresolution = parser.getInt();
    RtInt yresolution = parser.getInt();
    RtFloat pixelaspectratio = parser.getFloat();
    m_renderer.Format(xresolution, yresolution, pixelaspectratio);
}

void RibSema::handleFrameAspectRatio(IqRibParser& parser)
{
    RtFloat frameratio = parser.getFloat();
    m_renderer.FrameAspectRatio(frameratio);
}

void RibSema::handleScreenWindow(IqRibParser& parser)
{
    const IqRibParser::TqFloatArray& allArgs = parser.getFloatArray(4);
    RtFloat left = allArgs[0];
    RtFloat right = allArgs[1];
    RtFloat bottom = allArgs[2];
    RtFloat top = allArgs[3];
    m_renderer.ScreenWindow(left, right, bottom, top);
}

void RibSema::handleCropWindow(IqRibParser& parser)
{
    const IqRibParser::TqFloatArray& allArgs = parser.getFloatArray(4);
    RtFloat xmin = allArgs[0];
    RtFloat xmax = allArgs[1];
    RtFloat ymin = allArgs[2];
    RtFloat ymax = allArgs[3];
    m_renderer.CropWindow(xmin, xmax, ymin, ymax);
}

void RibSema::handleProjection(IqRibParser& parser)
{
    StringHolder name = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Projection(name, paramList);
}

void RibSema::handleClipping(IqRibParser& parser)
{
    RtFloat cnear = parser.getFloat();
    RtFloat cfar = parser.getFloat();
    m_renderer.Clipping(cnear, cfar);
}

void RibSema::handleClippingPlane(IqRibParser& parser)
{
    RtFloat x = parser.getFloat();
    RtFloat y = parser.getFloat();
    RtFloat z = parser.getFloat();
    RtFloat nx = parser.getFloat();
    RtFloat ny = parser.getFloat();
    RtFloat nz = parser.getFloat();
    m_renderer.ClippingPlane(x, y, z, nx, ny, nz);
}

void RibSema::handleShutter(IqRibParser& parser)
{
    RtFloat opentime = parser.getFloat();
    RtFloat closetime = parser.getFloat();
    m_renderer.Shutter(opentime, closetime);
}

void RibSema::handlePixelVariance(IqRibParser& parser)
{
    RtFloat variance = parser.getFloat();
    m_renderer.PixelVariance(variance);
}

void RibSema::handlePixelSamples(IqRibParser& parser)
{
    RtFloat xsamples = parser.getFloat();
    RtFloat ysamples = parser.getFloat();
    m_renderer.PixelSamples(xsamples, ysamples);
}

void RibSema::handlePixelFilter(IqRibParser& parser)
{
    RtFilterFunc function = m_renderer.GetFilterFunc(parser.getString().c_str());
    RtFloat xwidth = parser.getFloat();
    RtFloat ywidth = parser.getFloat();
    m_renderer.PixelFilter(function, xwidth, ywidth);
}

void RibSema::handleExposure(IqRibParser& parser)
{
    RtFloat gain = parser.getFloat();
    RtFloat gamma = parser.getFloat();
    m_renderer.Exposure(gain, gamma);
}

void RibSema::handleImager(IqRibParser& parser)
{
    StringHolder name = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Imager(name, paramList);
}

void RibSema::handleQuantize(IqRibParser& parser)
{
    StringHolder type = parser.getString();
    RtInt one = parser.getInt();
    RtInt min = parser.getInt();
    RtInt max = parser.getInt();
    RtFloat ditheramplitude = parser.getFloat();
    m_renderer.Quantize(type, one, min, max, ditheramplitude);
}

void RibSema::handleDisplay(IqRibParser& parser)
{
    StringHolder name = parser.getString();
    StringHolder type = parser.getString();
    StringHolder mode = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Display(name, type, mode, paramList);
}

void RibSema::handleHider(IqRibParser& parser)
{
    StringHolder name = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Hider(name, paramList);
}

void RibSema::handleRelativeDetail(IqRibParser& parser)
{
    RtFloat relativedetail = parser.getFloat();
    m_renderer.RelativeDetail(relativedetail);
}

void RibSema::handleOption(IqRibParser& parser)
{
    StringHolder name = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Option(name, paramList);
}

void RibSema::handleAttributeBegin(IqRibParser& parser)
{
    m_renderer.AttributeBegin();
}

void RibSema::handleAttributeEnd(IqRibParser& parser)
{
    m_renderer.AttributeEnd();
}

void RibSema::handleColor(IqRibParser& parser)
{
    RtConstColor& Cq = toFloatBasedType<RtConstColor>(parser.getFloatArray(m_numColorComps));
    m_renderer.Color(Cq);
}

void RibSema::handleOpacity(IqRibParser& parser)
{
    RtConstColor& Os = toFloatBasedType<RtConstColor>(parser.getFloatArray(m_numColorComps));
    m_renderer.Opacity(Os);
}

void RibSema::handleTextureCoordinates(IqRibParser& parser)
{
    const IqRibParser::TqFloatArray& allArgs = parser.getFloatArray(8);
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

void RibSema::handleSurface(IqRibParser& parser)
{
    StringHolder name = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Surface(name, paramList);
}

void RibSema::handleDisplacement(IqRibParser& parser)
{
    StringHolder name = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Displacement(name, paramList);
}

void RibSema::handleAtmosphere(IqRibParser& parser)
{
    StringHolder name = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Atmosphere(name, paramList);
}

void RibSema::handleInterior(IqRibParser& parser)
{
    StringHolder name = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Interior(name, paramList);
}

void RibSema::handleExterior(IqRibParser& parser)
{
    StringHolder name = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Exterior(name, paramList);
}

void RibSema::handleShaderLayer(IqRibParser& parser)
{
    StringHolder type = parser.getString();
    StringHolder name = parser.getString();
    StringHolder layername = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.ShaderLayer(type, name, layername, paramList);
}

void RibSema::handleConnectShaderLayers(IqRibParser& parser)
{
    StringHolder type = parser.getString();
    StringHolder layer1 = parser.getString();
    StringHolder variable1 = parser.getString();
    StringHolder layer2 = parser.getString();
    StringHolder variable2 = parser.getString();
    m_renderer.ConnectShaderLayers(type, layer1, variable1, layer2, variable2);
}

void RibSema::handleShadingRate(IqRibParser& parser)
{
    RtFloat size = parser.getFloat();
    m_renderer.ShadingRate(size);
}

void RibSema::handleShadingInterpolation(IqRibParser& parser)
{
    StringHolder type = parser.getString();
    m_renderer.ShadingInterpolation(type);
}

void RibSema::handleMatte(IqRibParser& parser)
{
    RtInt onoff = parser.getInt();
    m_renderer.Matte(onoff);
}

void RibSema::handleBound(IqRibParser& parser)
{
    RtConstBound& bound = toFloatBasedType<RtConstBound>(parser.getFloatArray(6));
    m_renderer.Bound(bound);
}

void RibSema::handleDetail(IqRibParser& parser)
{
    RtConstBound& bound = toFloatBasedType<RtConstBound>(parser.getFloatArray(6));
    m_renderer.Detail(bound);
}

void RibSema::handleDetailRange(IqRibParser& parser)
{
    const IqRibParser::TqFloatArray& allArgs = parser.getFloatArray(4);
    RtFloat offlow = allArgs[0];
    RtFloat onlow = allArgs[1];
    RtFloat onhigh = allArgs[2];
    RtFloat offhigh = allArgs[3];
    m_renderer.DetailRange(offlow, onlow, onhigh, offhigh);
}

void RibSema::handleGeometricApproximation(IqRibParser& parser)
{
    StringHolder type = parser.getString();
    RtFloat value = parser.getFloat();
    m_renderer.GeometricApproximation(type, value);
}

void RibSema::handleOrientation(IqRibParser& parser)
{
    StringHolder orientation = parser.getString();
    m_renderer.Orientation(orientation);
}

void RibSema::handleReverseOrientation(IqRibParser& parser)
{
    m_renderer.ReverseOrientation();
}

void RibSema::handleSides(IqRibParser& parser)
{
    RtInt nsides = parser.getInt();
    m_renderer.Sides(nsides);
}

void RibSema::handleIdentity(IqRibParser& parser)
{
    m_renderer.Identity();
}

void RibSema::handleTransform(IqRibParser& parser)
{
    RtConstMatrix& transform = toFloatBasedType<RtConstMatrix>(parser.getFloatArray(), "Matrix", 16);
    m_renderer.Transform(transform);
}

void RibSema::handleConcatTransform(IqRibParser& parser)
{
    RtConstMatrix& transform = toFloatBasedType<RtConstMatrix>(parser.getFloatArray(), "Matrix", 16);
    m_renderer.ConcatTransform(transform);
}

void RibSema::handlePerspective(IqRibParser& parser)
{
    RtFloat fov = parser.getFloat();
    m_renderer.Perspective(fov);
}

void RibSema::handleTranslate(IqRibParser& parser)
{
    RtFloat dx = parser.getFloat();
    RtFloat dy = parser.getFloat();
    RtFloat dz = parser.getFloat();
    m_renderer.Translate(dx, dy, dz);
}

void RibSema::handleRotate(IqRibParser& parser)
{
    RtFloat angle = parser.getFloat();
    RtFloat dx = parser.getFloat();
    RtFloat dy = parser.getFloat();
    RtFloat dz = parser.getFloat();
    m_renderer.Rotate(angle, dx, dy, dz);
}

void RibSema::handleScale(IqRibParser& parser)
{
    RtFloat sx = parser.getFloat();
    RtFloat sy = parser.getFloat();
    RtFloat sz = parser.getFloat();
    m_renderer.Scale(sx, sy, sz);
}

void RibSema::handleSkew(IqRibParser& parser)
{
    const IqRibParser::TqFloatArray& allArgs = parser.getFloatArray(7);
    RtFloat angle = allArgs[0];
    RtFloat dx1 = allArgs[1];
    RtFloat dy1 = allArgs[2];
    RtFloat dz1 = allArgs[3];
    RtFloat dx2 = allArgs[4];
    RtFloat dy2 = allArgs[5];
    RtFloat dz2 = allArgs[6];
    m_renderer.Skew(angle, dx1, dy1, dz1, dx2, dy2, dz2);
}

void RibSema::handleCoordinateSystem(IqRibParser& parser)
{
    StringHolder space = parser.getString();
    m_renderer.CoordinateSystem(space);
}

void RibSema::handleCoordSysTransform(IqRibParser& parser)
{
    StringHolder space = parser.getString();
    m_renderer.CoordSysTransform(space);
}

void RibSema::handleTransformBegin(IqRibParser& parser)
{
    m_renderer.TransformBegin();
}

void RibSema::handleTransformEnd(IqRibParser& parser)
{
    m_renderer.TransformEnd();
}

void RibSema::handleResource(IqRibParser& parser)
{
    StringHolder handle = parser.getString();
    StringHolder type = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Resource(handle, type, paramList);
}

void RibSema::handleResourceBegin(IqRibParser& parser)
{
    m_renderer.ResourceBegin();
}

void RibSema::handleResourceEnd(IqRibParser& parser)
{
    m_renderer.ResourceEnd();
}

void RibSema::handleAttribute(IqRibParser& parser)
{
    StringHolder name = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Attribute(name, paramList);
}

void RibSema::handlePolygon(IqRibParser& parser)
{
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Polygon(paramList);
}

void RibSema::handleGeneralPolygon(IqRibParser& parser)
{
    Ri::IntArray nverts = toRiArray(parser.getIntArray());
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.GeneralPolygon(nverts, paramList);
}

void RibSema::handlePointsPolygons(IqRibParser& parser)
{
    Ri::IntArray nverts = toRiArray(parser.getIntArray());
    Ri::IntArray verts = toRiArray(parser.getIntArray());
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.PointsPolygons(nverts, verts, paramList);
}

void RibSema::handlePointsGeneralPolygons(IqRibParser& parser)
{
    Ri::IntArray nloops = toRiArray(parser.getIntArray());
    Ri::IntArray nverts = toRiArray(parser.getIntArray());
    Ri::IntArray verts = toRiArray(parser.getIntArray());
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.PointsGeneralPolygons(nloops, nverts, verts, paramList);
}

void RibSema::handleBasis(IqRibParser& parser)
{
    RtConstBasis& ubasis = getBasis(parser);
    RtInt ustep = parser.getInt();
    RtConstBasis& vbasis = getBasis(parser);
    RtInt vstep = parser.getInt();
    m_renderer.Basis(ubasis, ustep, vbasis, vstep);
}

void RibSema::handlePatch(IqRibParser& parser)
{
    StringHolder type = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Patch(type, paramList);
}

void RibSema::handlePatchMesh(IqRibParser& parser)
{
    StringHolder type = parser.getString();
    RtInt nu = parser.getInt();
    StringHolder uwrap = parser.getString();
    RtInt nv = parser.getInt();
    StringHolder vwrap = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.PatchMesh(type, nu, uwrap, nv, vwrap, paramList);
}

void RibSema::handleNuPatch(IqRibParser& parser)
{
    RtInt nu = parser.getInt();
    RtInt uorder = parser.getInt();
    Ri::FloatArray uknot = toRiArray(parser.getFloatArray());
    RtFloat umin = parser.getFloat();
    RtFloat umax = parser.getFloat();
    RtInt nv = parser.getInt();
    RtInt vorder = parser.getInt();
    Ri::FloatArray vknot = toRiArray(parser.getFloatArray());
    RtFloat vmin = parser.getFloat();
    RtFloat vmax = parser.getFloat();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.NuPatch(nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax, paramList);
}

void RibSema::handleTrimCurve(IqRibParser& parser)
{
    Ri::IntArray ncurves = toRiArray(parser.getIntArray());
    Ri::IntArray order = toRiArray(parser.getIntArray());
    Ri::FloatArray knot = toRiArray(parser.getFloatArray());
    Ri::FloatArray min = toRiArray(parser.getFloatArray());
    Ri::FloatArray max = toRiArray(parser.getFloatArray());
    Ri::IntArray n = toRiArray(parser.getIntArray());
    Ri::FloatArray u = toRiArray(parser.getFloatArray());
    Ri::FloatArray v = toRiArray(parser.getFloatArray());
    Ri::FloatArray w = toRiArray(parser.getFloatArray());
    m_renderer.TrimCurve(ncurves, order, knot, min, max, n, u, v, w);
}

void RibSema::handleSphere(IqRibParser& parser)
{
    const IqRibParser::TqFloatArray& allArgs = parser.getFloatArray(4);
    RtFloat radius = allArgs[0];
    RtFloat zmin = allArgs[1];
    RtFloat zmax = allArgs[2];
    RtFloat thetamax = allArgs[3];
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Sphere(radius, zmin, zmax, thetamax, paramList);
}

void RibSema::handleCone(IqRibParser& parser)
{
    const IqRibParser::TqFloatArray& allArgs = parser.getFloatArray(3);
    RtFloat height = allArgs[0];
    RtFloat radius = allArgs[1];
    RtFloat thetamax = allArgs[2];
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Cone(height, radius, thetamax, paramList);
}

void RibSema::handleCylinder(IqRibParser& parser)
{
    const IqRibParser::TqFloatArray& allArgs = parser.getFloatArray(4);
    RtFloat radius = allArgs[0];
    RtFloat zmin = allArgs[1];
    RtFloat zmax = allArgs[2];
    RtFloat thetamax = allArgs[3];
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Cylinder(radius, zmin, zmax, thetamax, paramList);
}

void RibSema::handleParaboloid(IqRibParser& parser)
{
    const IqRibParser::TqFloatArray& allArgs = parser.getFloatArray(4);
    RtFloat rmax = allArgs[0];
    RtFloat zmin = allArgs[1];
    RtFloat zmax = allArgs[2];
    RtFloat thetamax = allArgs[3];
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Paraboloid(rmax, zmin, zmax, thetamax, paramList);
}

void RibSema::handleDisk(IqRibParser& parser)
{
    const IqRibParser::TqFloatArray& allArgs = parser.getFloatArray(3);
    RtFloat height = allArgs[0];
    RtFloat radius = allArgs[1];
    RtFloat thetamax = allArgs[2];
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Disk(height, radius, thetamax, paramList);
}

void RibSema::handleTorus(IqRibParser& parser)
{
    const IqRibParser::TqFloatArray& allArgs = parser.getFloatArray(5);
    RtFloat majorrad = allArgs[0];
    RtFloat minorrad = allArgs[1];
    RtFloat phimin = allArgs[2];
    RtFloat phimax = allArgs[3];
    RtFloat thetamax = allArgs[4];
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Torus(majorrad, minorrad, phimin, phimax, thetamax, paramList);
}

void RibSema::handlePoints(IqRibParser& parser)
{
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Points(paramList);
}

void RibSema::handleCurves(IqRibParser& parser)
{
    StringHolder type = parser.getString();
    Ri::IntArray nvertices = toRiArray(parser.getIntArray());
    StringHolder wrap = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Curves(type, nvertices, wrap, paramList);
}

void RibSema::handleBlobby(IqRibParser& parser)
{
    RtInt nleaf = parser.getInt();
    Ri::IntArray code = toRiArray(parser.getIntArray());
    Ri::FloatArray flt = toRiArray(parser.getFloatArray());
    StringArrayHolder str = parser.getStringArray();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Blobby(nleaf, code, flt, str, paramList);
}

void RibSema::handleGeometry(IqRibParser& parser)
{
    StringHolder type = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.Geometry(type, paramList);
}

void RibSema::handleSolidBegin(IqRibParser& parser)
{
    StringHolder type = parser.getString();
    m_renderer.SolidBegin(type);
}

void RibSema::handleSolidEnd(IqRibParser& parser)
{
    m_renderer.SolidEnd();
}

void RibSema::handleObjectEnd(IqRibParser& parser)
{
    m_renderer.ObjectEnd();
}

void RibSema::handleMotionBegin(IqRibParser& parser)
{
    Ri::FloatArray times = toRiArray(parser.getFloatArray());
    m_renderer.MotionBegin(times);
}

void RibSema::handleMotionEnd(IqRibParser& parser)
{
    m_renderer.MotionEnd();
}

void RibSema::handleMakeTexture(IqRibParser& parser)
{
    StringHolder imagefile = parser.getString();
    StringHolder texturefile = parser.getString();
    StringHolder swrap = parser.getString();
    StringHolder twrap = parser.getString();
    RtFilterFunc filterfunc = m_renderer.GetFilterFunc(parser.getString().c_str());
    RtFloat swidth = parser.getFloat();
    RtFloat twidth = parser.getFloat();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.MakeTexture(imagefile, texturefile, swrap, twrap, filterfunc, swidth, twidth, paramList);
}

void RibSema::handleMakeLatLongEnvironment(IqRibParser& parser)
{
    StringHolder imagefile = parser.getString();
    StringHolder reflfile = parser.getString();
    RtFilterFunc filterfunc = m_renderer.GetFilterFunc(parser.getString().c_str());
    RtFloat swidth = parser.getFloat();
    RtFloat twidth = parser.getFloat();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.MakeLatLongEnvironment(imagefile, reflfile, filterfunc, swidth, twidth, paramList);
}

void RibSema::handleMakeCubeFaceEnvironment(IqRibParser& parser)
{
    StringHolder px = parser.getString();
    StringHolder nx = parser.getString();
    StringHolder py = parser.getString();
    StringHolder ny = parser.getString();
    StringHolder pz = parser.getString();
    StringHolder nz = parser.getString();
    StringHolder reflfile = parser.getString();
    RtFloat fov = parser.getFloat();
    RtFilterFunc filterfunc = m_renderer.GetFilterFunc(parser.getString().c_str());
    RtFloat swidth = parser.getFloat();
    RtFloat twidth = parser.getFloat();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.MakeCubeFaceEnvironment(px, nx, py, ny, pz, nz, reflfile, fov, filterfunc, swidth, twidth, paramList);
}

void RibSema::handleMakeShadow(IqRibParser& parser)
{
    StringHolder picfile = parser.getString();
    StringHolder shadowfile = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.MakeShadow(picfile, shadowfile, paramList);
}

void RibSema::handleMakeOcclusion(IqRibParser& parser)
{
    StringArrayHolder picfiles = parser.getStringArray();
    StringHolder shadowfile = parser.getString();
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.MakeOcclusion(picfiles, shadowfile, paramList);
}

void RibSema::handleErrorHandler(IqRibParser& parser)
{
    RtErrorFunc handler = m_renderer.GetErrorFunc(parser.getString().c_str());
    m_renderer.ErrorHandler(handler);
}

void RibSema::handleReadArchive(IqRibParser& parser)
{
    StringHolder name = parser.getString();
    RtArchiveCallback callback = 0;
    ParamAccumulator paramList(m_tokenDict);
    parser.getParamList(paramList);
    m_renderer.ReadArchive(name, callback, paramList);
}

//[[[end]]]


//--------------------------------------------------

} // namespace Aqsis
