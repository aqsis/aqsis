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
#include "errorhandler.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
// RibParser implementation

RibParser::RibParser(Ri::RendererServices& rendererServices)
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
        MapValueType("ArchiveBegin", &RibParser::handleArchiveBegin),
        MapValueType("ArchiveEnd", &RibParser::handleArchiveEnd),
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

void RibParser::parseStream(std::istream& ribStream,
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
        catch(XqValidation& e)
        {
            // Add information on the location (file,line etc) of the problem
            // to the exception message and rethrow.
            if(requestName)
            {
                AQSIS_LOG_ERROR(m_services.errorHandler(), e.code())
                    << "Parse error at " << m_lex->streamPos()
                    << " while reading " << requestName << ": " << e.what();
            }
            else
            {
                AQSIS_LOG_ERROR(m_services.errorHandler(), e.code())
                    << "Parse error at " << m_lex->streamPos() << ": " << e.what();
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
Ri::ParamList RibParser::readParamList()
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
RtConstBasis& RibParser::getBasis()
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

void RibParser::handleVersion(Ri::Renderer& renderer)
{
    m_lex->getFloat();
    // Don't do anything with the version number; just blunder on regardless.
    // Probably only worth supporting if Pixar started publishing new versions
    // of the standard again...
}

void RibParser::handleDepthOfField(Ri::Renderer& renderer)
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

void RibParser::handleColorSamples(Ri::Renderer& renderer)
{
    // Collect arguments from lex.
    Ri::FloatArray nRGB = m_lex->getFloatArray();
    Ri::FloatArray RGBn = m_lex->getFloatArray();

    renderer.ColorSamples(nRGB, RGBn);
    m_numColorComps = nRGB.size()/3;
}

void RibParser::handleSubdivisionMesh(Ri::Renderer& renderer)
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

void RibParser::handleHyperboloid(Ri::Renderer& renderer)
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

void RibParser::handleProcedural(Ri::Renderer& renderer)
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
void RibParser::handle${procName}(Ri::Renderer& renderer)
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

void RibParser::handleDeclare(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    const char* declaration = m_lex->getString();
    renderer.Declare(name, declaration);
}

void RibParser::handleFrameBegin(Ri::Renderer& renderer)
{
    RtInt number = m_lex->getInt();
    renderer.FrameBegin(number);
}

void RibParser::handleFrameEnd(Ri::Renderer& renderer)
{
    renderer.FrameEnd();
}

void RibParser::handleWorldBegin(Ri::Renderer& renderer)
{
    renderer.WorldBegin();
}

void RibParser::handleWorldEnd(Ri::Renderer& renderer)
{
    renderer.WorldEnd();
}

void RibParser::handleIfBegin(Ri::Renderer& renderer)
{
    const char* condition = m_lex->getString();
    renderer.IfBegin(condition);
}

void RibParser::handleElseIf(Ri::Renderer& renderer)
{
    const char* condition = m_lex->getString();
    renderer.ElseIf(condition);
}

void RibParser::handleElse(Ri::Renderer& renderer)
{
    renderer.Else();
}

void RibParser::handleIfEnd(Ri::Renderer& renderer)
{
    renderer.IfEnd();
}

void RibParser::handleFormat(Ri::Renderer& renderer)
{
    RtInt xresolution = m_lex->getInt();
    RtInt yresolution = m_lex->getInt();
    RtFloat pixelaspectratio = m_lex->getFloat();
    renderer.Format(xresolution, yresolution, pixelaspectratio);
}

void RibParser::handleFrameAspectRatio(Ri::Renderer& renderer)
{
    RtFloat frameratio = m_lex->getFloat();
    renderer.FrameAspectRatio(frameratio);
}

void RibParser::handleScreenWindow(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(4);
    RtFloat left = allArgs[0];
    RtFloat right = allArgs[1];
    RtFloat bottom = allArgs[2];
    RtFloat top = allArgs[3];
    renderer.ScreenWindow(left, right, bottom, top);
}

void RibParser::handleCropWindow(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(4);
    RtFloat xmin = allArgs[0];
    RtFloat xmax = allArgs[1];
    RtFloat ymin = allArgs[2];
    RtFloat ymax = allArgs[3];
    renderer.CropWindow(xmin, xmax, ymin, ymax);
}

void RibParser::handleProjection(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Projection(name, paramList);
}

void RibParser::handleClipping(Ri::Renderer& renderer)
{
    RtFloat cnear = m_lex->getFloat();
    RtFloat cfar = m_lex->getFloat();
    renderer.Clipping(cnear, cfar);
}

void RibParser::handleClippingPlane(Ri::Renderer& renderer)
{
    RtFloat x = m_lex->getFloat();
    RtFloat y = m_lex->getFloat();
    RtFloat z = m_lex->getFloat();
    RtFloat nx = m_lex->getFloat();
    RtFloat ny = m_lex->getFloat();
    RtFloat nz = m_lex->getFloat();
    renderer.ClippingPlane(x, y, z, nx, ny, nz);
}

void RibParser::handleShutter(Ri::Renderer& renderer)
{
    RtFloat opentime = m_lex->getFloat();
    RtFloat closetime = m_lex->getFloat();
    renderer.Shutter(opentime, closetime);
}

void RibParser::handlePixelVariance(Ri::Renderer& renderer)
{
    RtFloat variance = m_lex->getFloat();
    renderer.PixelVariance(variance);
}

void RibParser::handlePixelSamples(Ri::Renderer& renderer)
{
    RtFloat xsamples = m_lex->getFloat();
    RtFloat ysamples = m_lex->getFloat();
    renderer.PixelSamples(xsamples, ysamples);
}

void RibParser::handlePixelFilter(Ri::Renderer& renderer)
{
    RtFilterFunc function = m_services.getFilterFunc(m_lex->getString());
    RtFloat xwidth = m_lex->getFloat();
    RtFloat ywidth = m_lex->getFloat();
    renderer.PixelFilter(function, xwidth, ywidth);
}

void RibParser::handleExposure(Ri::Renderer& renderer)
{
    RtFloat gain = m_lex->getFloat();
    RtFloat gamma = m_lex->getFloat();
    renderer.Exposure(gain, gamma);
}

void RibParser::handleImager(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Imager(name, paramList);
}

void RibParser::handleQuantize(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    RtInt one = m_lex->getInt();
    RtInt min = m_lex->getInt();
    RtInt max = m_lex->getInt();
    RtFloat ditheramplitude = m_lex->getFloat();
    renderer.Quantize(type, one, min, max, ditheramplitude);
}

void RibParser::handleDisplay(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    const char* type = m_lex->getString();
    const char* mode = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Display(name, type, mode, paramList);
}

void RibParser::handleHider(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Hider(name, paramList);
}

void RibParser::handleRelativeDetail(Ri::Renderer& renderer)
{
    RtFloat relativedetail = m_lex->getFloat();
    renderer.RelativeDetail(relativedetail);
}

void RibParser::handleOption(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Option(name, paramList);
}

void RibParser::handleAttributeBegin(Ri::Renderer& renderer)
{
    renderer.AttributeBegin();
}

void RibParser::handleAttributeEnd(Ri::Renderer& renderer)
{
    renderer.AttributeEnd();
}

void RibParser::handleColor(Ri::Renderer& renderer)
{
    RtConstColor& Cq = toFloatBasedType<RtConstColor>(m_lex->getFloatArray(m_numColorComps));
    renderer.Color(Cq);
}

void RibParser::handleOpacity(Ri::Renderer& renderer)
{
    RtConstColor& Os = toFloatBasedType<RtConstColor>(m_lex->getFloatArray(m_numColorComps));
    renderer.Opacity(Os);
}

void RibParser::handleTextureCoordinates(Ri::Renderer& renderer)
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

void RibParser::handleLightSource(Ri::Renderer& renderer)
{
    const char* shadername = m_lex->getString();
    StringOrIntHolder name(*m_lex);
    Ri::ParamList paramList = readParamList();
    renderer.LightSource(shadername, name, paramList);
}

void RibParser::handleAreaLightSource(Ri::Renderer& renderer)
{
    const char* shadername = m_lex->getString();
    StringOrIntHolder name(*m_lex);
    Ri::ParamList paramList = readParamList();
    renderer.AreaLightSource(shadername, name, paramList);
}

void RibParser::handleIlluminate(Ri::Renderer& renderer)
{
    StringOrIntHolder name(*m_lex);
    RtInt onoff = m_lex->getInt();
    renderer.Illuminate(name, onoff);
}

void RibParser::handleSurface(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Surface(name, paramList);
}

void RibParser::handleDisplacement(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Displacement(name, paramList);
}

void RibParser::handleAtmosphere(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Atmosphere(name, paramList);
}

void RibParser::handleInterior(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Interior(name, paramList);
}

void RibParser::handleExterior(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Exterior(name, paramList);
}

void RibParser::handleShaderLayer(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    const char* name = m_lex->getString();
    const char* layername = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.ShaderLayer(type, name, layername, paramList);
}

void RibParser::handleConnectShaderLayers(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    const char* layer1 = m_lex->getString();
    const char* variable1 = m_lex->getString();
    const char* layer2 = m_lex->getString();
    const char* variable2 = m_lex->getString();
    renderer.ConnectShaderLayers(type, layer1, variable1, layer2, variable2);
}

void RibParser::handleShadingRate(Ri::Renderer& renderer)
{
    RtFloat size = m_lex->getFloat();
    renderer.ShadingRate(size);
}

void RibParser::handleShadingInterpolation(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    renderer.ShadingInterpolation(type);
}

void RibParser::handleMatte(Ri::Renderer& renderer)
{
    RtInt onoff = m_lex->getInt();
    renderer.Matte(onoff);
}

void RibParser::handleBound(Ri::Renderer& renderer)
{
    RtConstBound& bound = toFloatBasedType<RtConstBound>(m_lex->getFloatArray(6));
    renderer.Bound(bound);
}

void RibParser::handleDetail(Ri::Renderer& renderer)
{
    RtConstBound& bound = toFloatBasedType<RtConstBound>(m_lex->getFloatArray(6));
    renderer.Detail(bound);
}

void RibParser::handleDetailRange(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(4);
    RtFloat offlow = allArgs[0];
    RtFloat onlow = allArgs[1];
    RtFloat onhigh = allArgs[2];
    RtFloat offhigh = allArgs[3];
    renderer.DetailRange(offlow, onlow, onhigh, offhigh);
}

void RibParser::handleGeometricApproximation(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    RtFloat value = m_lex->getFloat();
    renderer.GeometricApproximation(type, value);
}

void RibParser::handleOrientation(Ri::Renderer& renderer)
{
    const char* orientation = m_lex->getString();
    renderer.Orientation(orientation);
}

void RibParser::handleReverseOrientation(Ri::Renderer& renderer)
{
    renderer.ReverseOrientation();
}

void RibParser::handleSides(Ri::Renderer& renderer)
{
    RtInt nsides = m_lex->getInt();
    renderer.Sides(nsides);
}

void RibParser::handleIdentity(Ri::Renderer& renderer)
{
    renderer.Identity();
}

void RibParser::handleTransform(Ri::Renderer& renderer)
{
    RtConstMatrix& transform = toFloatBasedType<RtConstMatrix>(m_lex->getFloatArray(), "Matrix", 16);
    renderer.Transform(transform);
}

void RibParser::handleConcatTransform(Ri::Renderer& renderer)
{
    RtConstMatrix& transform = toFloatBasedType<RtConstMatrix>(m_lex->getFloatArray(), "Matrix", 16);
    renderer.ConcatTransform(transform);
}

void RibParser::handlePerspective(Ri::Renderer& renderer)
{
    RtFloat fov = m_lex->getFloat();
    renderer.Perspective(fov);
}

void RibParser::handleTranslate(Ri::Renderer& renderer)
{
    RtFloat dx = m_lex->getFloat();
    RtFloat dy = m_lex->getFloat();
    RtFloat dz = m_lex->getFloat();
    renderer.Translate(dx, dy, dz);
}

void RibParser::handleRotate(Ri::Renderer& renderer)
{
    RtFloat angle = m_lex->getFloat();
    RtFloat dx = m_lex->getFloat();
    RtFloat dy = m_lex->getFloat();
    RtFloat dz = m_lex->getFloat();
    renderer.Rotate(angle, dx, dy, dz);
}

void RibParser::handleScale(Ri::Renderer& renderer)
{
    RtFloat sx = m_lex->getFloat();
    RtFloat sy = m_lex->getFloat();
    RtFloat sz = m_lex->getFloat();
    renderer.Scale(sx, sy, sz);
}

void RibParser::handleSkew(Ri::Renderer& renderer)
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

void RibParser::handleCoordinateSystem(Ri::Renderer& renderer)
{
    const char* space = m_lex->getString();
    renderer.CoordinateSystem(space);
}

void RibParser::handleCoordSysTransform(Ri::Renderer& renderer)
{
    const char* space = m_lex->getString();
    renderer.CoordSysTransform(space);
}

void RibParser::handleTransformBegin(Ri::Renderer& renderer)
{
    renderer.TransformBegin();
}

void RibParser::handleTransformEnd(Ri::Renderer& renderer)
{
    renderer.TransformEnd();
}

void RibParser::handleResource(Ri::Renderer& renderer)
{
    const char* handle = m_lex->getString();
    const char* type = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Resource(handle, type, paramList);
}

void RibParser::handleResourceBegin(Ri::Renderer& renderer)
{
    renderer.ResourceBegin();
}

void RibParser::handleResourceEnd(Ri::Renderer& renderer)
{
    renderer.ResourceEnd();
}

void RibParser::handleAttribute(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Attribute(name, paramList);
}

void RibParser::handlePolygon(Ri::Renderer& renderer)
{
    Ri::ParamList paramList = readParamList();
    renderer.Polygon(paramList);
}

void RibParser::handleGeneralPolygon(Ri::Renderer& renderer)
{
    Ri::IntArray nverts = m_lex->getIntArray();
    Ri::ParamList paramList = readParamList();
    renderer.GeneralPolygon(nverts, paramList);
}

void RibParser::handlePointsPolygons(Ri::Renderer& renderer)
{
    Ri::IntArray nverts = m_lex->getIntArray();
    Ri::IntArray verts = m_lex->getIntArray();
    Ri::ParamList paramList = readParamList();
    renderer.PointsPolygons(nverts, verts, paramList);
}

void RibParser::handlePointsGeneralPolygons(Ri::Renderer& renderer)
{
    Ri::IntArray nloops = m_lex->getIntArray();
    Ri::IntArray nverts = m_lex->getIntArray();
    Ri::IntArray verts = m_lex->getIntArray();
    Ri::ParamList paramList = readParamList();
    renderer.PointsGeneralPolygons(nloops, nverts, verts, paramList);
}

void RibParser::handleBasis(Ri::Renderer& renderer)
{
    RtConstBasis& ubasis = getBasis();
    RtInt ustep = m_lex->getInt();
    RtConstBasis& vbasis = getBasis();
    RtInt vstep = m_lex->getInt();
    renderer.Basis(ubasis, ustep, vbasis, vstep);
}

void RibParser::handlePatch(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Patch(type, paramList);
}

void RibParser::handlePatchMesh(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    RtInt nu = m_lex->getInt();
    const char* uwrap = m_lex->getString();
    RtInt nv = m_lex->getInt();
    const char* vwrap = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.PatchMesh(type, nu, uwrap, nv, vwrap, paramList);
}

void RibParser::handleNuPatch(Ri::Renderer& renderer)
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

void RibParser::handleTrimCurve(Ri::Renderer& renderer)
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

void RibParser::handleSphere(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(4);
    RtFloat radius = allArgs[0];
    RtFloat zmin = allArgs[1];
    RtFloat zmax = allArgs[2];
    RtFloat thetamax = allArgs[3];
    Ri::ParamList paramList = readParamList();
    renderer.Sphere(radius, zmin, zmax, thetamax, paramList);
}

void RibParser::handleCone(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(3);
    RtFloat height = allArgs[0];
    RtFloat radius = allArgs[1];
    RtFloat thetamax = allArgs[2];
    Ri::ParamList paramList = readParamList();
    renderer.Cone(height, radius, thetamax, paramList);
}

void RibParser::handleCylinder(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(4);
    RtFloat radius = allArgs[0];
    RtFloat zmin = allArgs[1];
    RtFloat zmax = allArgs[2];
    RtFloat thetamax = allArgs[3];
    Ri::ParamList paramList = readParamList();
    renderer.Cylinder(radius, zmin, zmax, thetamax, paramList);
}

void RibParser::handleParaboloid(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(4);
    RtFloat rmax = allArgs[0];
    RtFloat zmin = allArgs[1];
    RtFloat zmax = allArgs[2];
    RtFloat thetamax = allArgs[3];
    Ri::ParamList paramList = readParamList();
    renderer.Paraboloid(rmax, zmin, zmax, thetamax, paramList);
}

void RibParser::handleDisk(Ri::Renderer& renderer)
{
    Ri::FloatArray allArgs = m_lex->getFloatArray(3);
    RtFloat height = allArgs[0];
    RtFloat radius = allArgs[1];
    RtFloat thetamax = allArgs[2];
    Ri::ParamList paramList = readParamList();
    renderer.Disk(height, radius, thetamax, paramList);
}

void RibParser::handleTorus(Ri::Renderer& renderer)
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

void RibParser::handlePoints(Ri::Renderer& renderer)
{
    Ri::ParamList paramList = readParamList();
    renderer.Points(paramList);
}

void RibParser::handleCurves(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    Ri::IntArray nvertices = m_lex->getIntArray();
    const char* wrap = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Curves(type, nvertices, wrap, paramList);
}

void RibParser::handleBlobby(Ri::Renderer& renderer)
{
    RtInt nleaf = m_lex->getInt();
    Ri::IntArray code = m_lex->getIntArray();
    Ri::FloatArray floats = m_lex->getFloatArray();
    Ri::StringArray strings = m_lex->getStringArray();
    Ri::ParamList paramList = readParamList();
    renderer.Blobby(nleaf, code, floats, strings, paramList);
}

void RibParser::handleGeometry(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.Geometry(type, paramList);
}

void RibParser::handleSolidBegin(Ri::Renderer& renderer)
{
    const char* type = m_lex->getString();
    renderer.SolidBegin(type);
}

void RibParser::handleSolidEnd(Ri::Renderer& renderer)
{
    renderer.SolidEnd();
}

void RibParser::handleObjectBegin(Ri::Renderer& renderer)
{
    StringOrIntHolder name(*m_lex);
    renderer.ObjectBegin(name);
}

void RibParser::handleObjectEnd(Ri::Renderer& renderer)
{
    renderer.ObjectEnd();
}

void RibParser::handleObjectInstance(Ri::Renderer& renderer)
{
    StringOrIntHolder name(*m_lex);
    renderer.ObjectInstance(name);
}

void RibParser::handleMotionBegin(Ri::Renderer& renderer)
{
    Ri::FloatArray times = m_lex->getFloatArray();
    renderer.MotionBegin(times);
}

void RibParser::handleMotionEnd(Ri::Renderer& renderer)
{
    renderer.MotionEnd();
}

void RibParser::handleMakeTexture(Ri::Renderer& renderer)
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

void RibParser::handleMakeLatLongEnvironment(Ri::Renderer& renderer)
{
    const char* imagefile = m_lex->getString();
    const char* reflfile = m_lex->getString();
    RtFilterFunc filterfunc = m_services.getFilterFunc(m_lex->getString());
    RtFloat swidth = m_lex->getFloat();
    RtFloat twidth = m_lex->getFloat();
    Ri::ParamList paramList = readParamList();
    renderer.MakeLatLongEnvironment(imagefile, reflfile, filterfunc, swidth, twidth, paramList);
}

void RibParser::handleMakeCubeFaceEnvironment(Ri::Renderer& renderer)
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

void RibParser::handleMakeShadow(Ri::Renderer& renderer)
{
    const char* picfile = m_lex->getString();
    const char* shadowfile = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.MakeShadow(picfile, shadowfile, paramList);
}

void RibParser::handleMakeOcclusion(Ri::Renderer& renderer)
{
    Ri::StringArray picfiles = m_lex->getStringArray();
    const char* shadowfile = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.MakeOcclusion(picfiles, shadowfile, paramList);
}

void RibParser::handleErrorHandler(Ri::Renderer& renderer)
{
    RtErrorFunc handler = m_services.getErrorFunc(m_lex->getString());
    renderer.ErrorHandler(handler);
}

void RibParser::handleReadArchive(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    RtArchiveCallback callback = 0;
    Ri::ParamList paramList = readParamList();
    renderer.ReadArchive(name, callback, paramList);
}

void RibParser::handleArchiveBegin(Ri::Renderer& renderer)
{
    const char* name = m_lex->getString();
    Ri::ParamList paramList = readParamList();
    renderer.ArchiveBegin(name, paramList);
}

void RibParser::handleArchiveEnd(Ri::Renderer& renderer)
{
    renderer.ArchiveEnd();
}
//[[[end]]]


//--------------------------------------------------

} // namespace Aqsis
// vi: set et:
