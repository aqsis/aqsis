// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/** \file
 *
 * \brief RIB request handler implementation for aqsis.
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 */

#include "ribparser_impl.h"

#include <cfloat>
#include <cstring>  // for strcpy

#include "riblexer.h"
#include <aqsis/riutil/errorhandler.h>

namespace Aqsis
{

RibParser* RibParser::create(Ri::RendererServices& services)
{
    return new RibParserImpl(services);
}

//------------------------------------------------------------------------------
// RibParserImpl implementation

RibParserImpl::RibParserImpl(Ri::RendererServices& rendererServices)
    : m_services(rendererServices),
    m_lex(RibLexer::create(), &RibLexer::destroy),
    m_requestHandlerMap(),
    m_paramListStorage(),
    m_numColorComps(3)
{
    typedef HandlerMap::value_type MapValueType;
    MapValueType handlerMapInit[] = {
        /*[[[cog
        from codegenutils import *
        riXml = parseXml(riXmlPath)

        for p in filter(lambda p: p.findall('Rib'), riXml.findall('Procedures/Procedure')):
            name = p.findtext('Name')
            cog.outl('MapValueType("%s", &RibParserImpl::handle%s),' % (name, name))
        ]]]*/
        MapValueType("Declare", &RibParserImpl::handleDeclare),
        MapValueType("FrameBegin", &RibParserImpl::handleFrameBegin),
        MapValueType("FrameEnd", &RibParserImpl::handleFrameEnd),
        MapValueType("WorldBegin", &RibParserImpl::handleWorldBegin),
        MapValueType("WorldEnd", &RibParserImpl::handleWorldEnd),
        MapValueType("IfBegin", &RibParserImpl::handleIfBegin),
        MapValueType("ElseIf", &RibParserImpl::handleElseIf),
        MapValueType("Else", &RibParserImpl::handleElse),
        MapValueType("IfEnd", &RibParserImpl::handleIfEnd),
        MapValueType("Format", &RibParserImpl::handleFormat),
        MapValueType("FrameAspectRatio", &RibParserImpl::handleFrameAspectRatio),
        MapValueType("ScreenWindow", &RibParserImpl::handleScreenWindow),
        MapValueType("CropWindow", &RibParserImpl::handleCropWindow),
        MapValueType("Projection", &RibParserImpl::handleProjection),
        MapValueType("Clipping", &RibParserImpl::handleClipping),
        MapValueType("ClippingPlane", &RibParserImpl::handleClippingPlane),
        MapValueType("DepthOfField", &RibParserImpl::handleDepthOfField),
        MapValueType("Shutter", &RibParserImpl::handleShutter),
        MapValueType("PixelVariance", &RibParserImpl::handlePixelVariance),
        MapValueType("PixelSamples", &RibParserImpl::handlePixelSamples),
        MapValueType("PixelFilter", &RibParserImpl::handlePixelFilter),
        MapValueType("Exposure", &RibParserImpl::handleExposure),
        MapValueType("Imager", &RibParserImpl::handleImager),
        MapValueType("Quantize", &RibParserImpl::handleQuantize),
        MapValueType("Display", &RibParserImpl::handleDisplay),
        MapValueType("Hider", &RibParserImpl::handleHider),
        MapValueType("ColorSamples", &RibParserImpl::handleColorSamples),
        MapValueType("RelativeDetail", &RibParserImpl::handleRelativeDetail),
        MapValueType("Option", &RibParserImpl::handleOption),
        MapValueType("AttributeBegin", &RibParserImpl::handleAttributeBegin),
        MapValueType("AttributeEnd", &RibParserImpl::handleAttributeEnd),
        MapValueType("Color", &RibParserImpl::handleColor),
        MapValueType("Opacity", &RibParserImpl::handleOpacity),
        MapValueType("TextureCoordinates", &RibParserImpl::handleTextureCoordinates),
        MapValueType("LightSource", &RibParserImpl::handleLightSource),
        MapValueType("AreaLightSource", &RibParserImpl::handleAreaLightSource),
        MapValueType("Illuminate", &RibParserImpl::handleIlluminate),
        MapValueType("Surface", &RibParserImpl::handleSurface),
        MapValueType("Displacement", &RibParserImpl::handleDisplacement),
        MapValueType("Atmosphere", &RibParserImpl::handleAtmosphere),
        MapValueType("Interior", &RibParserImpl::handleInterior),
        MapValueType("Exterior", &RibParserImpl::handleExterior),
        MapValueType("ShaderLayer", &RibParserImpl::handleShaderLayer),
        MapValueType("ConnectShaderLayers", &RibParserImpl::handleConnectShaderLayers),
        MapValueType("ShadingRate", &RibParserImpl::handleShadingRate),
        MapValueType("ShadingInterpolation", &RibParserImpl::handleShadingInterpolation),
        MapValueType("Matte", &RibParserImpl::handleMatte),
        MapValueType("Bound", &RibParserImpl::handleBound),
        MapValueType("Detail", &RibParserImpl::handleDetail),
        MapValueType("DetailRange", &RibParserImpl::handleDetailRange),
        MapValueType("GeometricApproximation", &RibParserImpl::handleGeometricApproximation),
        MapValueType("Orientation", &RibParserImpl::handleOrientation),
        MapValueType("ReverseOrientation", &RibParserImpl::handleReverseOrientation),
        MapValueType("Sides", &RibParserImpl::handleSides),
        MapValueType("Identity", &RibParserImpl::handleIdentity),
        MapValueType("Transform", &RibParserImpl::handleTransform),
        MapValueType("ConcatTransform", &RibParserImpl::handleConcatTransform),
        MapValueType("Perspective", &RibParserImpl::handlePerspective),
        MapValueType("Translate", &RibParserImpl::handleTranslate),
        MapValueType("Rotate", &RibParserImpl::handleRotate),
        MapValueType("Scale", &RibParserImpl::handleScale),
        MapValueType("Skew", &RibParserImpl::handleSkew),
        MapValueType("CoordinateSystem", &RibParserImpl::handleCoordinateSystem),
        MapValueType("CoordSysTransform", &RibParserImpl::handleCoordSysTransform),
        MapValueType("TransformBegin", &RibParserImpl::handleTransformBegin),
        MapValueType("TransformEnd", &RibParserImpl::handleTransformEnd),
        MapValueType("Resource", &RibParserImpl::handleResource),
        MapValueType("ResourceBegin", &RibParserImpl::handleResourceBegin),
        MapValueType("ResourceEnd", &RibParserImpl::handleResourceEnd),
        MapValueType("Attribute", &RibParserImpl::handleAttribute),
        MapValueType("Polygon", &RibParserImpl::handlePolygon),
        MapValueType("GeneralPolygon", &RibParserImpl::handleGeneralPolygon),
        MapValueType("PointsPolygons", &RibParserImpl::handlePointsPolygons),
        MapValueType("PointsGeneralPolygons", &RibParserImpl::handlePointsGeneralPolygons),
        MapValueType("Basis", &RibParserImpl::handleBasis),
        MapValueType("Patch", &RibParserImpl::handlePatch),
        MapValueType("PatchMesh", &RibParserImpl::handlePatchMesh),
        MapValueType("NuPatch", &RibParserImpl::handleNuPatch),
        MapValueType("TrimCurve", &RibParserImpl::handleTrimCurve),
        MapValueType("SubdivisionMesh", &RibParserImpl::handleSubdivisionMesh),
        MapValueType("Sphere", &RibParserImpl::handleSphere),
        MapValueType("Cone", &RibParserImpl::handleCone),
        MapValueType("Cylinder", &RibParserImpl::handleCylinder),
        MapValueType("Hyperboloid", &RibParserImpl::handleHyperboloid),
        MapValueType("Paraboloid", &RibParserImpl::handleParaboloid),
        MapValueType("Disk", &RibParserImpl::handleDisk),
        MapValueType("Torus", &RibParserImpl::handleTorus),
        MapValueType("Points", &RibParserImpl::handlePoints),
        MapValueType("Curves", &RibParserImpl::handleCurves),
        MapValueType("Blobby", &RibParserImpl::handleBlobby),
        MapValueType("Procedural", &RibParserImpl::handleProcedural),
        MapValueType("Geometry", &RibParserImpl::handleGeometry),
        MapValueType("SolidBegin", &RibParserImpl::handleSolidBegin),
        MapValueType("SolidEnd", &RibParserImpl::handleSolidEnd),
        MapValueType("ObjectBegin", &RibParserImpl::handleObjectBegin),
        MapValueType("ObjectEnd", &RibParserImpl::handleObjectEnd),
        MapValueType("ObjectInstance", &RibParserImpl::handleObjectInstance),
        MapValueType("MotionBegin", &RibParserImpl::handleMotionBegin),
        MapValueType("MotionEnd", &RibParserImpl::handleMotionEnd),
        MapValueType("MakeTexture", &RibParserImpl::handleMakeTexture),
        MapValueType("MakeLatLongEnvironment", &RibParserImpl::handleMakeLatLongEnvironment),
        MapValueType("MakeCubeFaceEnvironment", &RibParserImpl::handleMakeCubeFaceEnvironment),
        MapValueType("MakeShadow", &RibParserImpl::handleMakeShadow),
        MapValueType("MakeOcclusion", &RibParserImpl::handleMakeOcclusion),
        MapValueType("ErrorHandler", &RibParserImpl::handleErrorHandler),
        MapValueType("ReadArchive", &RibParserImpl::handleReadArchive),
        MapValueType("ArchiveBegin", &RibParserImpl::handleArchiveBegin),
        MapValueType("ArchiveEnd", &RibParserImpl::handleArchiveEnd),
        //[[[end]]]
    };
    m_requestHandlerMap.insert(
            handlerMapInit,
            handlerMapInit + sizeof(handlerMapInit)/sizeof(handlerMapInit[0])
    );
    // Add the special RIB-only "version" request to the list.
    m_requestHandlerMap["version"] = &RibParserImpl::handleVersion;
}

namespace {
/// Callback which sends errors to Ri::Renderer::ArchiveRecord
class CommentCallback
{
    private:
        Ri::Renderer& m_renderer;
    public:
        CommentCallback(Ri::Renderer& renderer) : m_renderer(renderer) {}
        void operator()(const std::string& sstr)
        {
            const char* str = sstr.c_str();  // TODO change callback signature to use const char*.
            if(str[0] == '#')
                m_renderer.ArchiveRecord("structure", str+1);
            else
                m_renderer.ArchiveRecord("comment", str);
        }
};
} // anon. namespace

void RibParserImpl::parseStream(std::istream& ribStream,
                            const std::string& streamName,
                            Ri::Renderer& renderer)
{
    m_lex->pushInput(ribStream, streamName, CommentCallback(renderer));
    while(true)
    {
        const char* requestName = 0;
        try
        {
            requestName = m_lex->nextRequest();
            if(!requestName)
                break;  // end of stream
            HandlerMap::const_iterator pos =
                m_requestHandlerMap.find(requestName);
            if(pos == m_requestHandlerMap.end())
                AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken,
                                    "unrecognized request");
            RequestHandlerType handler = pos->second;
            (this->*handler)(renderer);
        }
        catch(XqException& e)
        {
            // TODO: ^^ Think about exception heirarchy, and whether we want to
            // catch something more specific than XqException.

            // Add information on the location (file,line etc) of the problem
            // to the exception message and rethrow.
            if(requestName)
            {
                m_services.errorHandler().error(e.code(),
                                    "Parse error at %s while reading %s: %s",
                                    m_lex->streamPos(), requestName, e.what());
            }
            else
            {
                m_services.errorHandler().error(e.code(),
                    "Parse error at %s: %s", m_lex->streamPos(), e.what());
            }
            m_lex->discardUntilRequest();
        }
        catch(...)
        {
            // Other errors should pass through, but we should make sure to pop
            // the stream first!
            m_lex->popInput();
            throw;
        }
    }
    m_lex->popInput();
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


// Holder of RIB name for a light or object
//
// The RISpec says that lights are identified by a 'sequence number', but
// string identifiers are also allowed in common implementations.  For
// simplicity, we convert int light/object identifiers into strings.  This
// means that 1 is the same as "1".
//
// This class manages the storage, and has an implicit conversion to the
// corresponding type expected by Ri::Renderer (ie, const char*)
class StringOrIntHolder
{
    private:
        std::string m_str;
        const char* m_cstr;
    public:
        StringOrIntHolder(RibLexer& lex)
        {
            if(lex.peekNextType() == RibLexer::Tok_String)
                m_cstr = lex.getString();
            else
            {
                std::ostringstream fmt;
                fmt << lex.getInt();
                m_str = fmt.str();
                m_cstr = m_str.c_str();
            }
        }

        operator const char*() { return m_cstr; }
};

} // anon. namespace

/// Read in a renderman parameter list from the lexer.
Ri::ParamList RibParserImpl::readParamList()
{
    m_paramListStorage.clear();
    m_paramNameStorage.clear();
    while(m_lex->peekNextType() != RibLexer::Tok_RequestEnd)
    {
        const char* name = 0;
        const char* nameEnd = 0;
        Ri::TypeSpec spec = m_services.getDeclaration(m_lex->getString(),
                                                      &name, &nameEnd);
        if(*nameEnd != 0)
        {
            // Unusual case: name is valid but not properly null-terminated
            // (eg, ends in whitespace).  We need to generate a copy so as to
            // add the correct null termination.
            m_paramNameStorage.push_back(std::string(name, nameEnd));
            name = m_paramNameStorage.back().c_str();
        }
        switch(spec.storageType())
        {
            case Ri::TypeSpec::Integer:
                m_paramListStorage.push_back(Ri::Param(spec, name, m_lex->getIntParam()));
                break;
            case Ri::TypeSpec::Float:
                m_paramListStorage.push_back(Ri::Param(spec, name, m_lex->getFloatParam()));
                break;
            case Ri::TypeSpec::String:
                m_paramListStorage.push_back(Ri::Param(spec, name, m_lex->getStringParam()));
                break;
            case Ri::TypeSpec::Pointer:
                AQSIS_THROW_XQERROR(XqValidation, EqE_BadToken,
                                    "Pointer token \"" << name << "\" "
                                    "invalid in RIB stream.");
                break;
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
RtConstBasis& RibParserImpl::getBasis()
{
    switch(m_lex->peekNextType())
    {
        case RibLexer::Tok_Array:
            {
                Ri::FloatArray basis = m_lex->getFloatArray();
                if(basis.size() != 16)
                    AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
                        "basis array must be of length 16");
                // Ugly, but should be safe except unless a compiler fails to
                // lay out float[4][4] the same as float[16]
                return *reinterpret_cast<RtConstBasis*>(basis.begin());
            }
        case RibLexer::Tok_String:
            {
                const char* name = m_lex->getString();
                RtConstBasis* basis = m_services.getBasis(name);
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

void RibParserImpl::handleVersion(Ri::Renderer& renderer)
{
    m_lex->getFloat();
    // Don't do anything with the version number; just blunder on regardless.
    // Probably only worth supporting if Pixar started publishing new versions
    // of the standard again...
}

void RibParserImpl::handleDepthOfField(Ri::Renderer& renderer)
{
    if(m_lex->peekNextType() == RibLexer::Tok_RequestEnd)
    {
        // If called without arguments, reset to the default pinhole camera.
        renderer.DepthOfField(FLT_MAX, FLT_MAX, FLT_MAX);
    }
    else
    {
        // Collect arguments from lex.
        RtFloat fstop = m_lex->getFloat();
        RtFloat focallength = m_lex->getFloat();
        RtFloat focaldistance = m_lex->getFloat();

        renderer.DepthOfField(fstop, focallength, focaldistance);
    }
}

void RibParserImpl::handleColorSamples(Ri::Renderer& renderer)
{
    // Collect arguments from lex.
    Ri::FloatArray nRGB = m_lex->getFloatArray();
    Ri::FloatArray RGBn = m_lex->getFloatArray();

    renderer.ColorSamples(nRGB, RGBn);
    m_numColorComps = nRGB.size()/3;
}

void RibParserImpl::handleSubdivisionMesh(Ri::Renderer& renderer)
{
    // Collect arguments from lex.
    const char* scheme = m_lex->getString();
    Ri::IntArray nvertices = m_lex->getIntArray();
    Ri::IntArray vertices  = m_lex->getIntArray();

    // Else call version with empty optional args.
    Ri::StringArray  tags;
    Ri::IntArray     nargs;
    Ri::IntArray     intargs;
    Ri::FloatArray   floatargs;
    if(m_lex->peekNextType() == RibLexer::Tok_Array)
    {
        // Handle the four optional arguments.
        tags      = m_lex->getStringArray();
        nargs     = m_lex->getIntArray();
        intargs   = m_lex->getIntArray();
        floatargs = m_lex->getFloatArray();
    }
    // Extract parameter list
    Ri::ParamList paramList = readParamList();
    // Call through to renderer
    renderer.SubdivisionMesh(scheme, nvertices, vertices, tags, nargs,
                             intargs, floatargs, paramList);
}

void RibParserImpl::handleHyperboloid(Ri::Renderer& renderer)
{
    // Collect required args as an array
    Ri::FloatArray allArgs = m_lex->getFloatArray(7);
    RtConstPoint& point1 = *reinterpret_cast<RtConstPoint*>(&allArgs[0]);
    RtConstPoint& point2 = *reinterpret_cast<RtConstPoint*>(&allArgs[3]);
    RtFloat thetamax = allArgs[6];
    // Extract the parameter list
    Ri::ParamList paramList = readParamList();
    // Call through to renderer
    renderer.Hyperboloid(point1, point2, thetamax, paramList);
}

// Equivalent of RiProcFree, but that's not accessible if we haven't linked
// with the RI, so we just define our own version here.
static void ProcFree(RtPointer p)
{
    free(p);
}

void RibParserImpl::handleProcedural(Ri::Renderer& renderer)
{
    // get procedural subdivision function
    const char* procName = m_lex->getString();
    RtProcSubdivFunc subdivideFunc = m_services.getProcSubdivFunc(procName);
    if(!subdivideFunc)
    {
        AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken,
                            "unknown procedural function \"" << procName << "\"");
    }

    // get argument string array.
    Ri::StringArray args = m_lex->getStringArray();
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
    RtConstBound& bound = toFloatBasedType<RtConstBound>(m_lex->getFloatArray(),
                                                         "bound", 6);

    renderer.Procedural(procData, bound, subdivideFunc, &ProcFree);
}


//--------------------------------------------------
// Request handlers with autogenerated implementations.
/*[[[cog
import cog
import sys, os
from codegenutils import *
from Cheetah.Template import Template

riXml = parseXml(riXmlPath)

# Map from RI types to strings which retrieve the value from the lexer
getterStatements = {
    'RtBoolean':     'RtInt %s = m_lex->getInt();',
    'RtInt':         'RtInt %s = m_lex->getInt();',
    'RtIntArray':    'Ri::IntArray %s = m_lex->getIntArray();',
    'RtFloat':       'RtFloat %s = m_lex->getFloat();',
    'RtFloatArray':  'Ri::FloatArray %s = m_lex->getFloatArray();',
    'RtString':      'const char* %s = m_lex->getString();',
    'RtStringArray': 'Ri::StringArray %s = m_lex->getStringArray();',
    'RtToken':       'const char* %s = m_lex->getString();',
    'RtTokenArray':  'Ri::StringArray %s = m_lex->getStringArray();',
    'RtColor':       'RtConstColor& %s = toFloatBasedType<RtConstColor>(m_lex->getFloatArray(m_numColorComps));',
    'RtPoint':       'RtConstPoint& %s = toFloatBasedType<RtConstPoint>(m_lex->getFloatArray(), "Point", 3);',
    'RtMatrix':      'RtConstMatrix& %s = toFloatBasedType<RtConstMatrix>(m_lex->getFloatArray(), "Matrix", 16);',
    'RtBound':       'RtConstBound& %s = toFloatBasedType<RtConstBound>(m_lex->getFloatArray(6));',
    'RtFilterFunc':  'RtFilterFunc %s = m_services.getFilterFunc(m_lex->getString());',
    'RtArchiveCallback': 'RtArchiveCallback %s = 0;',
    'RtErrorFunc':   'RtErrorFunc %s = m_services.getErrorFunc(m_lex->getString());',
    'RtBasis':       'RtConstBasis& %s = getBasis();',
}
def getterStatement(arg):
    name = arg.findtext('Name')
    type = arg.findtext('Type')
    if arg.findall('AltTypeInt') and type == 'RtToken':
        # Special case for an int which should be turned into a string.
        return 'StringOrIntHolder %s(*m_lex);' % name
    return getterStatements[type] % (name,)

customImpl = set(['DepthOfField', 'ColorSamples', 'SubdivisionMesh',
                  'Hyperboloid', 'Procedural'])

handlerTemplate = '''
void RibParserImpl::handle${procName}(Ri::Renderer& renderer)
{
    #if $proc.findall('Arguments/RibArgsCanBeArray')
    ## Collect all args as an array
    Ri::FloatArray allArgs = m_lex->getFloatArray(${len($args)});
        #for $i, $arg in enumerate($args)
    RtFloat $arg.findtext('Name') = allArgs[$i];
        #end for
    #else
    ## Collect individual arguments from lexer
        #for $arg in $args
    $getterStatement($arg)
        #end for
    #end if
    #if $proc.findall('Arguments/ParamList')
    ## Extract parameter list
    Ri::ParamList paramList = readParamList();
    #end if
    ## Construct the argument list to the C++ interface binding call.
    #set $argList = []
    #for $arg in $args
        #set $argList += [$arg.findtext('Name')]
    #end for
    #if $proc.findall('Arguments/ParamList')
        #set $argList += ['paramList']
    #end if
    renderer.${procName}(${', '.join(argList)});
}
'''

for proc in riXml.findall('Procedures/Procedure'):
    if not (proc.findall('Rib') and proc.findtext('Name') not in customImpl):
        continue
    procName = proc.findtext('Name')
    args = ribArgs(proc)
    cog.out(str(Template(handlerTemplate, searchList=locals())));

]]]*/

void RibParserImpl::handleDeclare(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    const char* declaration = m_lex->getString();
    renderer.Declare(name, declaration);
}

void RibParserImpl::handleFrameBegin(Ri::Renderer& renderer)
{
    RtInt number = m_lex->getInt();
    renderer.FrameBegin(number);
}

void RibParserImpl::handleFrameEnd(Ri::Renderer& renderer)
{
    renderer.FrameEnd();
}

void RibParserImpl::handleWorldBegin(Ri::Renderer& renderer)
{
    renderer.WorldBegin();
}

void RibParserImpl::handleWorldEnd(Ri::Renderer& renderer)
{
    renderer.WorldEnd();
}

void RibParserImpl::handleIfBegin(Ri::Renderer& renderer)
{
    const char* condition = m_lex->getString();
    renderer.IfBegin(condition);
}

void RibParserImpl::handleElseIf(Ri::Renderer& renderer)
{
    const char* condition = m_lex->getString();
    renderer.ElseIf(condition);
}

void RibParserImpl::handleElse(Ri::Renderer& renderer)
{
    renderer.Else();
}

void RibParserImpl::handleIfEnd(Ri::Renderer& renderer)
{
    renderer.IfEnd();
}

void RibParserImpl::handleFormat(Ri::Renderer& renderer)
{
    RtInt xresolution = m_lex->getInt();
    RtInt yresolution = m_lex->getInt();
    RtFloat pixelaspectratio = m_lex->getFloat();
    renderer.Format(xresolution, yresolution, pixelaspectratio);
}

void RibParserImpl::handleFrameAspectRatio(Ri::Renderer& renderer)
{
    RtFloat frameratio = m_lex->getFloat();
    renderer.FrameAspectRatio(frameratio);
}

void RibParserImpl::handleScreenWindow(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(4);
    RtFloat left = allArgs[0];
    RtFloat right = allArgs[1];
    RtFloat bottom = allArgs[2];
    RtFloat top = allArgs[3];
    renderer.ScreenWindow(left, right, bottom, top);
}

void RibParserImpl::handleCropWindow(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(4);
    RtFloat xmin = allArgs[0];
    RtFloat xmax = allArgs[1];
    RtFloat ymin = allArgs[2];
    RtFloat ymax = allArgs[3];
    renderer.CropWindow(xmin, xmax, ymin, ymax);
}

void RibParserImpl::handleProjection(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Projection(name, paramList);
}

void RibParserImpl::handleClipping(Ri::Renderer& renderer)
{
    RtFloat cnear = m_lex->getFloat();
    RtFloat cfar = m_lex->getFloat();
    renderer.Clipping(cnear, cfar);
}

void RibParserImpl::handleClippingPlane(Ri::Renderer& renderer)
{
    RtFloat x = m_lex->getFloat();
    RtFloat y = m_lex->getFloat();
    RtFloat z = m_lex->getFloat();
    RtFloat nx = m_lex->getFloat();
    RtFloat ny = m_lex->getFloat();
    RtFloat nz = m_lex->getFloat();
    renderer.ClippingPlane(x, y, z, nx, ny, nz);
}

void RibParserImpl::handleShutter(Ri::Renderer& renderer)
{
    RtFloat opentime = m_lex->getFloat();
    RtFloat closetime = m_lex->getFloat();
    renderer.Shutter(opentime, closetime);
}

void RibParserImpl::handlePixelVariance(Ri::Renderer& renderer)
{
    RtFloat variance = m_lex->getFloat();
    renderer.PixelVariance(variance);
}

void RibParserImpl::handlePixelSamples(Ri::Renderer& renderer)
{
    RtFloat xsamples = m_lex->getFloat();
    RtFloat ysamples = m_lex->getFloat();
    renderer.PixelSamples(xsamples, ysamples);
}

void RibParserImpl::handlePixelFilter(Ri::Renderer& renderer)
{
    RtFilterFunc function = m_services.getFilterFunc(m_lex->getString());
    RtFloat xwidth = m_lex->getFloat();
    RtFloat ywidth = m_lex->getFloat();
    renderer.PixelFilter(function, xwidth, ywidth);
}

void RibParserImpl::handleExposure(Ri::Renderer& renderer)
{
    RtFloat gain = m_lex->getFloat();
    RtFloat gamma = m_lex->getFloat();
    renderer.Exposure(gain, gamma);
}

void RibParserImpl::handleImager(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Imager(name, paramList);
}

void RibParserImpl::handleQuantize(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    RtInt one = m_lex->getInt();
    RtInt min = m_lex->getInt();
    RtInt max = m_lex->getInt();
    RtFloat ditheramplitude = m_lex->getFloat();
    renderer.Quantize(type, one, min, max, ditheramplitude);
}

void RibParserImpl::handleDisplay(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    const char* type = m_lex->getString();
    const char* mode = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Display(name, type, mode, paramList);
}

void RibParserImpl::handleHider(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Hider(name, paramList);
}

void RibParserImpl::handleRelativeDetail(Ri::Renderer& renderer)
{
    RtFloat relativedetail = m_lex->getFloat();
    renderer.RelativeDetail(relativedetail);
}

void RibParserImpl::handleOption(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Option(name, paramList);
}

void RibParserImpl::handleAttributeBegin(Ri::Renderer& renderer)
{
    renderer.AttributeBegin();
}

void RibParserImpl::handleAttributeEnd(Ri::Renderer& renderer)
{
    renderer.AttributeEnd();
}

void RibParserImpl::handleColor(Ri::Renderer& renderer)
{
    RtConstColor& Cq = toFloatBasedType<RtConstColor>(m_lex->getFloatArray(m_numColorComps));
    renderer.Color(Cq);
}

void RibParserImpl::handleOpacity(Ri::Renderer& renderer)
{
    RtConstColor& Os = toFloatBasedType<RtConstColor>(m_lex->getFloatArray(m_numColorComps));
    renderer.Opacity(Os);
}

void RibParserImpl::handleTextureCoordinates(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(8);
    RtFloat s1 = allArgs[0];
    RtFloat t1 = allArgs[1];
    RtFloat s2 = allArgs[2];
    RtFloat t2 = allArgs[3];
    RtFloat s3 = allArgs[4];
    RtFloat t3 = allArgs[5];
    RtFloat s4 = allArgs[6];
    RtFloat t4 = allArgs[7];
    renderer.TextureCoordinates(s1, t1, s2, t2, s3, t3, s4, t4);
}

void RibParserImpl::handleLightSource(Ri::Renderer& renderer)
{
    const char* shadername = m_lex->getString();
    StringOrIntHolder name(*m_lex);
    Ri::ParamList paramList = readParamList();
    renderer.LightSource(shadername, name, paramList);
}

void RibParserImpl::handleAreaLightSource(Ri::Renderer& renderer)
{
    const char* shadername = m_lex->getString();
    StringOrIntHolder name(*m_lex);
    Ri::ParamList paramList = readParamList();
    renderer.AreaLightSource(shadername, name, paramList);
}

void RibParserImpl::handleIlluminate(Ri::Renderer& renderer)
{
    StringOrIntHolder name(*m_lex);
    RtInt onoff = m_lex->getInt();
    renderer.Illuminate(name, onoff);
}

void RibParserImpl::handleSurface(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Surface(name, paramList);
}

void RibParserImpl::handleDisplacement(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Displacement(name, paramList);
}

void RibParserImpl::handleAtmosphere(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Atmosphere(name, paramList);
}

void RibParserImpl::handleInterior(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Interior(name, paramList);
}

void RibParserImpl::handleExterior(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Exterior(name, paramList);
}

void RibParserImpl::handleShaderLayer(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    const char* name = m_lex->getString();
    const char* layername = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.ShaderLayer(type, name, layername, paramList);
}

void RibParserImpl::handleConnectShaderLayers(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    const char* layer1 = m_lex->getString();
    const char* variable1 = m_lex->getString();
    const char* layer2 = m_lex->getString();
    const char* variable2 = m_lex->getString();
    renderer.ConnectShaderLayers(type, layer1, variable1, layer2, variable2);
}

void RibParserImpl::handleShadingRate(Ri::Renderer& renderer)
{
    RtFloat size = m_lex->getFloat();
    renderer.ShadingRate(size);
}

void RibParserImpl::handleShadingInterpolation(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    renderer.ShadingInterpolation(type);
}

void RibParserImpl::handleMatte(Ri::Renderer& renderer)
{
    RtInt onoff = m_lex->getInt();
    renderer.Matte(onoff);
}

void RibParserImpl::handleBound(Ri::Renderer& renderer)
{
    RtConstBound& bound = toFloatBasedType<RtConstBound>(m_lex->getFloatArray(6));
    renderer.Bound(bound);
}

void RibParserImpl::handleDetail(Ri::Renderer& renderer)
{
    RtConstBound& bound = toFloatBasedType<RtConstBound>(m_lex->getFloatArray(6));
    renderer.Detail(bound);
}

void RibParserImpl::handleDetailRange(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(4);
    RtFloat offlow = allArgs[0];
    RtFloat onlow = allArgs[1];
    RtFloat onhigh = allArgs[2];
    RtFloat offhigh = allArgs[3];
    renderer.DetailRange(offlow, onlow, onhigh, offhigh);
}

void RibParserImpl::handleGeometricApproximation(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    RtFloat value = m_lex->getFloat();
    renderer.GeometricApproximation(type, value);
}

void RibParserImpl::handleOrientation(Ri::Renderer& renderer)
{
    const char* orientation = m_lex->getString();
    renderer.Orientation(orientation);
}

void RibParserImpl::handleReverseOrientation(Ri::Renderer& renderer)
{
    renderer.ReverseOrientation();
}

void RibParserImpl::handleSides(Ri::Renderer& renderer)
{
    RtInt nsides = m_lex->getInt();
    renderer.Sides(nsides);
}

void RibParserImpl::handleIdentity(Ri::Renderer& renderer)
{
    renderer.Identity();
}

void RibParserImpl::handleTransform(Ri::Renderer& renderer)
{
    RtConstMatrix& transform = toFloatBasedType<RtConstMatrix>(m_lex->getFloatArray(), "Matrix", 16);
    renderer.Transform(transform);
}

void RibParserImpl::handleConcatTransform(Ri::Renderer& renderer)
{
    RtConstMatrix& transform = toFloatBasedType<RtConstMatrix>(m_lex->getFloatArray(), "Matrix", 16);
    renderer.ConcatTransform(transform);
}

void RibParserImpl::handlePerspective(Ri::Renderer& renderer)
{
    RtFloat fov = m_lex->getFloat();
    renderer.Perspective(fov);
}

void RibParserImpl::handleTranslate(Ri::Renderer& renderer)
{
    RtFloat dx = m_lex->getFloat();
    RtFloat dy = m_lex->getFloat();
    RtFloat dz = m_lex->getFloat();
    renderer.Translate(dx, dy, dz);
}

void RibParserImpl::handleRotate(Ri::Renderer& renderer)
{
    RtFloat angle = m_lex->getFloat();
    RtFloat dx = m_lex->getFloat();
    RtFloat dy = m_lex->getFloat();
    RtFloat dz = m_lex->getFloat();
    renderer.Rotate(angle, dx, dy, dz);
}

void RibParserImpl::handleScale(Ri::Renderer& renderer)
{
    RtFloat sx = m_lex->getFloat();
    RtFloat sy = m_lex->getFloat();
    RtFloat sz = m_lex->getFloat();
    renderer.Scale(sx, sy, sz);
}

void RibParserImpl::handleSkew(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(7);
    RtFloat angle = allArgs[0];
    RtFloat dx1 = allArgs[1];
    RtFloat dy1 = allArgs[2];
    RtFloat dz1 = allArgs[3];
    RtFloat dx2 = allArgs[4];
    RtFloat dy2 = allArgs[5];
    RtFloat dz2 = allArgs[6];
    renderer.Skew(angle, dx1, dy1, dz1, dx2, dy2, dz2);
}

void RibParserImpl::handleCoordinateSystem(Ri::Renderer& renderer)
{
    const char* space = m_lex->getString();
    renderer.CoordinateSystem(space);
}

void RibParserImpl::handleCoordSysTransform(Ri::Renderer& renderer)
{
    const char* space = m_lex->getString();
    renderer.CoordSysTransform(space);
}

void RibParserImpl::handleTransformBegin(Ri::Renderer& renderer)
{
    renderer.TransformBegin();
}

void RibParserImpl::handleTransformEnd(Ri::Renderer& renderer)
{
    renderer.TransformEnd();
}

void RibParserImpl::handleResource(Ri::Renderer& renderer)
{
    const char* handle = m_lex->getString();
    const char* type = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Resource(handle, type, paramList);
}

void RibParserImpl::handleResourceBegin(Ri::Renderer& renderer)
{
    renderer.ResourceBegin();
}

void RibParserImpl::handleResourceEnd(Ri::Renderer& renderer)
{
    renderer.ResourceEnd();
}

void RibParserImpl::handleAttribute(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Attribute(name, paramList);
}

void RibParserImpl::handlePolygon(Ri::Renderer& renderer)
{
    Ri::ParamList paramList = readParamList();
    renderer.Polygon(paramList);
}

void RibParserImpl::handleGeneralPolygon(Ri::Renderer& renderer)
{
    Ri::IntArray nverts = m_lex->getIntArray();
    Ri::ParamList paramList = readParamList();
    renderer.GeneralPolygon(nverts, paramList);
}

void RibParserImpl::handlePointsPolygons(Ri::Renderer& renderer)
{
    Ri::IntArray nverts = m_lex->getIntArray();
    Ri::IntArray verts = m_lex->getIntArray();
    Ri::ParamList paramList = readParamList();
    renderer.PointsPolygons(nverts, verts, paramList);
}

void RibParserImpl::handlePointsGeneralPolygons(Ri::Renderer& renderer)
{
    Ri::IntArray nloops = m_lex->getIntArray();
    Ri::IntArray nverts = m_lex->getIntArray();
    Ri::IntArray verts = m_lex->getIntArray();
    Ri::ParamList paramList = readParamList();
    renderer.PointsGeneralPolygons(nloops, nverts, verts, paramList);
}

void RibParserImpl::handleBasis(Ri::Renderer& renderer)
{
    RtConstBasis& ubasis = getBasis();
    RtInt ustep = m_lex->getInt();
    RtConstBasis& vbasis = getBasis();
    RtInt vstep = m_lex->getInt();
    renderer.Basis(ubasis, ustep, vbasis, vstep);
}

void RibParserImpl::handlePatch(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Patch(type, paramList);
}

void RibParserImpl::handlePatchMesh(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    RtInt nu = m_lex->getInt();
    const char* uwrap = m_lex->getString();
    RtInt nv = m_lex->getInt();
    const char* vwrap = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.PatchMesh(type, nu, uwrap, nv, vwrap, paramList);
}

void RibParserImpl::handleNuPatch(Ri::Renderer& renderer)
{
    RtInt nu = m_lex->getInt();
    RtInt uorder = m_lex->getInt();
    Ri::FloatArray uknot = m_lex->getFloatArray();
    RtFloat umin = m_lex->getFloat();
    RtFloat umax = m_lex->getFloat();
    RtInt nv = m_lex->getInt();
    RtInt vorder = m_lex->getInt();
    Ri::FloatArray vknot = m_lex->getFloatArray();
    RtFloat vmin = m_lex->getFloat();
    RtFloat vmax = m_lex->getFloat();
    Ri::ParamList paramList = readParamList();
    renderer.NuPatch(nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax, paramList);
}

void RibParserImpl::handleTrimCurve(Ri::Renderer& renderer)
{
    Ri::IntArray ncurves = m_lex->getIntArray();
    Ri::IntArray order = m_lex->getIntArray();
    Ri::FloatArray knot = m_lex->getFloatArray();
    Ri::FloatArray min = m_lex->getFloatArray();
    Ri::FloatArray max = m_lex->getFloatArray();
    Ri::IntArray n = m_lex->getIntArray();
    Ri::FloatArray u = m_lex->getFloatArray();
    Ri::FloatArray v = m_lex->getFloatArray();
    Ri::FloatArray w = m_lex->getFloatArray();
    renderer.TrimCurve(ncurves, order, knot, min, max, n, u, v, w);
}

void RibParserImpl::handleSphere(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(4);
    RtFloat radius = allArgs[0];
    RtFloat zmin = allArgs[1];
    RtFloat zmax = allArgs[2];
    RtFloat thetamax = allArgs[3];
    Ri::ParamList paramList = readParamList();
    renderer.Sphere(radius, zmin, zmax, thetamax, paramList);
}

void RibParserImpl::handleCone(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(3);
    RtFloat height = allArgs[0];
    RtFloat radius = allArgs[1];
    RtFloat thetamax = allArgs[2];
    Ri::ParamList paramList = readParamList();
    renderer.Cone(height, radius, thetamax, paramList);
}

void RibParserImpl::handleCylinder(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(4);
    RtFloat radius = allArgs[0];
    RtFloat zmin = allArgs[1];
    RtFloat zmax = allArgs[2];
    RtFloat thetamax = allArgs[3];
    Ri::ParamList paramList = readParamList();
    renderer.Cylinder(radius, zmin, zmax, thetamax, paramList);
}

void RibParserImpl::handleParaboloid(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(4);
    RtFloat rmax = allArgs[0];
    RtFloat zmin = allArgs[1];
    RtFloat zmax = allArgs[2];
    RtFloat thetamax = allArgs[3];
    Ri::ParamList paramList = readParamList();
    renderer.Paraboloid(rmax, zmin, zmax, thetamax, paramList);
}

void RibParserImpl::handleDisk(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(3);
    RtFloat height = allArgs[0];
    RtFloat radius = allArgs[1];
    RtFloat thetamax = allArgs[2];
    Ri::ParamList paramList = readParamList();
    renderer.Disk(height, radius, thetamax, paramList);
}

void RibParserImpl::handleTorus(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(5);
    RtFloat majorrad = allArgs[0];
    RtFloat minorrad = allArgs[1];
    RtFloat phimin = allArgs[2];
    RtFloat phimax = allArgs[3];
    RtFloat thetamax = allArgs[4];
    Ri::ParamList paramList = readParamList();
    renderer.Torus(majorrad, minorrad, phimin, phimax, thetamax, paramList);
}

void RibParserImpl::handlePoints(Ri::Renderer& renderer)
{
    Ri::ParamList paramList = readParamList();
    renderer.Points(paramList);
}

void RibParserImpl::handleCurves(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    Ri::IntArray nvertices = m_lex->getIntArray();
    const char* wrap = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Curves(type, nvertices, wrap, paramList);
}

void RibParserImpl::handleBlobby(Ri::Renderer& renderer)
{
    RtInt nleaf = m_lex->getInt();
    Ri::IntArray code = m_lex->getIntArray();
    Ri::FloatArray floats = m_lex->getFloatArray();
    Ri::StringArray strings = m_lex->getStringArray();
    Ri::ParamList paramList = readParamList();
    renderer.Blobby(nleaf, code, floats, strings, paramList);
}

void RibParserImpl::handleGeometry(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Geometry(type, paramList);
}

void RibParserImpl::handleSolidBegin(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    renderer.SolidBegin(type);
}

void RibParserImpl::handleSolidEnd(Ri::Renderer& renderer)
{
    renderer.SolidEnd();
}

void RibParserImpl::handleObjectBegin(Ri::Renderer& renderer)
{
    StringOrIntHolder name(*m_lex);
    renderer.ObjectBegin(name);
}

void RibParserImpl::handleObjectEnd(Ri::Renderer& renderer)
{
    renderer.ObjectEnd();
}

void RibParserImpl::handleObjectInstance(Ri::Renderer& renderer)
{
    StringOrIntHolder name(*m_lex);
    renderer.ObjectInstance(name);
}

void RibParserImpl::handleMotionBegin(Ri::Renderer& renderer)
{
    Ri::FloatArray times = m_lex->getFloatArray();
    renderer.MotionBegin(times);
}

void RibParserImpl::handleMotionEnd(Ri::Renderer& renderer)
{
    renderer.MotionEnd();
}

void RibParserImpl::handleMakeTexture(Ri::Renderer& renderer)
{
    const char* imagefile = m_lex->getString();
    const char* texturefile = m_lex->getString();
    const char* swrap = m_lex->getString();
    const char* twrap = m_lex->getString();
    RtFilterFunc filterfunc = m_services.getFilterFunc(m_lex->getString());
    RtFloat swidth = m_lex->getFloat();
    RtFloat twidth = m_lex->getFloat();
    Ri::ParamList paramList = readParamList();
    renderer.MakeTexture(imagefile, texturefile, swrap, twrap, filterfunc, swidth, twidth, paramList);
}

void RibParserImpl::handleMakeLatLongEnvironment(Ri::Renderer& renderer)
{
    const char* imagefile = m_lex->getString();
    const char* reflfile = m_lex->getString();
    RtFilterFunc filterfunc = m_services.getFilterFunc(m_lex->getString());
    RtFloat swidth = m_lex->getFloat();
    RtFloat twidth = m_lex->getFloat();
    Ri::ParamList paramList = readParamList();
    renderer.MakeLatLongEnvironment(imagefile, reflfile, filterfunc, swidth, twidth, paramList);
}

void RibParserImpl::handleMakeCubeFaceEnvironment(Ri::Renderer& renderer)
{
    const char* px = m_lex->getString();
    const char* nx = m_lex->getString();
    const char* py = m_lex->getString();
    const char* ny = m_lex->getString();
    const char* pz = m_lex->getString();
    const char* nz = m_lex->getString();
    const char* reflfile = m_lex->getString();
    RtFloat fov = m_lex->getFloat();
    RtFilterFunc filterfunc = m_services.getFilterFunc(m_lex->getString());
    RtFloat swidth = m_lex->getFloat();
    RtFloat twidth = m_lex->getFloat();
    Ri::ParamList paramList = readParamList();
    renderer.MakeCubeFaceEnvironment(px, nx, py, ny, pz, nz, reflfile, fov, filterfunc, swidth, twidth, paramList);
}

void RibParserImpl::handleMakeShadow(Ri::Renderer& renderer)
{
    const char* picfile = m_lex->getString();
    const char* shadowfile = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.MakeShadow(picfile, shadowfile, paramList);
}

void RibParserImpl::handleMakeOcclusion(Ri::Renderer& renderer)
{
    Ri::StringArray picfiles = m_lex->getStringArray();
    const char* shadowfile = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.MakeOcclusion(picfiles, shadowfile, paramList);
}

void RibParserImpl::handleErrorHandler(Ri::Renderer& renderer)
{
    RtErrorFunc handler = m_services.getErrorFunc(m_lex->getString());
    renderer.ErrorHandler(handler);
}

void RibParserImpl::handleReadArchive(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    RtArchiveCallback callback = 0;
    Ri::ParamList paramList = readParamList();
    renderer.ReadArchive(name, callback, paramList);
}

void RibParserImpl::handleArchiveBegin(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.ArchiveBegin(name, paramList);
}

void RibParserImpl::handleArchiveEnd(Ri::Renderer& renderer)
{
    renderer.ArchiveEnd();
}
//[[[end]]]


//--------------------------------------------------

} // namespace Aqsis
// vi: set et:
