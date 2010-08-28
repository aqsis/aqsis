// Aqsis
// Copyright (C) 1997 - 2010, Paul C. Gregory
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

/// \file Renderer interface implementation
/// \author Chris Foster

#include <vector>

#include <boost/shared_ptr.hpp>

#include <aqsis/riutil/ribparser.h>
#include <aqsis/riutil/ricxx.h>
#include <aqsis/riutil/ricxx_filter.h>
#include <aqsis/riutil/tokendictionary.h>
#include <../../libs/riutil/errorhandlerimpl.h>

#include "renderer.h"
#include "surfaces.h"

namespace Aqsis {


/// An error handler which just sends errors to the Aqsis::log() stream.
class PrintErrorHandler : public Ri::ErrorHandler
{
    public:
        explicit PrintErrorHandler(ErrorCategory verbosity = Warning)
            : ErrorHandler(verbosity)
        { }

    protected:
        virtual void sendError(int code, const std::string& message)
        {
            std::ostream& out = std::cerr;
            switch(errorCategory(code))
            {
                case Debug:   out << "\033[32m"   "DEBUG: "   ; break;
                case Info:    out <<              "INFO: "    ; break;
                case Warning: out << "\033[36m"   "WARNING: " ; break;
                case Error:   out << "\033[1;31m" "ERROR: "   ; break;
                case Severe:  out << "\033[1;31m" "SEVERE: "  ; break;
                default: break;
            }
            out << message << "\033[0m" << std::endl;
        }
};


class ApiServices : public Ri::RendererServices
{
    public:
        ApiServices();

        RtVoid declare(RtConstString name, RtConstString declaration)
        {
            m_tokenDict.declare(name, declaration);
        }

        //--------------------------------------------------
        // from Ri::RenderServices
        virtual Ri::ErrorHandler& errorHandler()
        {
            return m_errorHandler;
        }

        virtual RtFilterFunc getFilterFunc(RtConstToken name) const
        {
            return 0;
            //return getFilterFuncByName(name);
        }
        virtual RtConstBasis* getBasis(RtConstToken name) const
        {
            return 0;
            //return getBasisByName(name);
        }
        virtual RtErrorFunc getErrorFunc(RtConstToken name) const
        {
            return 0;
            //return getErrorFuncByName(name);
        }
        virtual RtProcSubdivFunc getProcSubdivFunc(RtConstToken name) const
        {
            return 0;
            //return getProcSubdivFuncByName(name);
        }

        virtual Ri::TypeSpec getDeclaration(RtConstToken token,
                                        const char** nameBegin = 0,
                                        const char** nameEnd = 0) const
        {
            return m_tokenDict.lookup(token, nameBegin, nameEnd);
        }

        virtual Ri::Renderer& firstFilter()
        {
            if(!m_filterChain.empty())
                return *m_filterChain.back();
            return *m_api;
        }

        virtual void addFilter(const char* name,
                               const Ri::ParamList& filterParams
                               = Ri::ParamList())
        {
            boost::shared_ptr<Ri::Renderer> filter;
            filter.reset(createFilter(name, *this, firstFilter(),
                                      filterParams));
            if(filter)
                m_filterChain.push_back(filter);
            else
            {
                AQSIS_THROW_XQERROR(XqValidation, EqE_BadToken,
                        "filter \"" << name << "\" not found");
            }
        }

        virtual void parseRib(std::istream& ribStream, const char* name,
                              Ri::Renderer& context)
        {
            if(!m_parser)
                m_parser.reset(RibParser::create(*this));
            m_parser->parseStream(ribStream, name, context);
        }

    private:
        /// Renderer API
        boost::shared_ptr<Ri::Renderer> m_api;
        /// Parser for ReadArchive.  May be NULL (created on demand).
        boost::shared_ptr<RibParser> m_parser;
        /// Chain of filters
        std::vector<boost::shared_ptr<Ri::Renderer> > m_filterChain;
        /// Error handler.
        PrintErrorHandler m_errorHandler;
        /// Declared tokens
        TokenDict m_tokenDict;
};


//------------------------------------------------------------------------------
class RenderApi : public Ri::Renderer
{
    private:
        ApiServices& m_services;
        Options m_opts;
        Attributes m_attrs;

    public:
        RenderApi(ApiServices& services)
            : m_services(services)
        { }

        virtual RtVoid ArchiveRecord(RtConstToken type, const char* string)
        { }

        // Code generator for autogenerated method declarations
        /*[[[cog
        from codegenutils import *

        riXml = parseXml(riXmlPath)

        for proc in riXml.findall('Procedures/Procedure'):
            if proc.findall('Rib'):
                decl = 'virtual %s;' % (riCxxMethodDecl(proc),)
                cog.outl(wrapDecl(decl, 72, wrapIndent=20))
        ]]]*/
        virtual RtVoid Declare(RtConstString name, RtConstString declaration);
        virtual RtVoid FrameBegin(RtInt number);
        virtual RtVoid FrameEnd();
        virtual RtVoid WorldBegin();
        virtual RtVoid WorldEnd();
        virtual RtVoid IfBegin(RtConstString condition);
        virtual RtVoid ElseIf(RtConstString condition);
        virtual RtVoid Else();
        virtual RtVoid IfEnd();
        virtual RtVoid Format(RtInt xresolution, RtInt yresolution,
                            RtFloat pixelaspectratio);
        virtual RtVoid FrameAspectRatio(RtFloat frameratio);
        virtual RtVoid ScreenWindow(RtFloat left, RtFloat right, RtFloat bottom,
                            RtFloat top);
        virtual RtVoid CropWindow(RtFloat xmin, RtFloat xmax, RtFloat ymin,
                            RtFloat ymax);
        virtual RtVoid Projection(RtConstToken name, const ParamList& pList);
        virtual RtVoid Clipping(RtFloat cnear, RtFloat cfar);
        virtual RtVoid ClippingPlane(RtFloat x, RtFloat y, RtFloat z,
                            RtFloat nx, RtFloat ny, RtFloat nz);
        virtual RtVoid DepthOfField(RtFloat fstop, RtFloat focallength,
                            RtFloat focaldistance);
        virtual RtVoid Shutter(RtFloat opentime, RtFloat closetime);
        virtual RtVoid PixelVariance(RtFloat variance);
        virtual RtVoid PixelSamples(RtFloat xsamples, RtFloat ysamples);
        virtual RtVoid PixelFilter(RtFilterFunc function, RtFloat xwidth,
                            RtFloat ywidth);
        virtual RtVoid Exposure(RtFloat gain, RtFloat gamma);
        virtual RtVoid Imager(RtConstToken name, const ParamList& pList);
        virtual RtVoid Quantize(RtConstToken type, RtInt one, RtInt min,
                            RtInt max, RtFloat ditheramplitude);
        virtual RtVoid Display(RtConstToken name, RtConstToken type,
                            RtConstToken mode, const ParamList& pList);
        virtual RtVoid Hider(RtConstToken name, const ParamList& pList);
        virtual RtVoid ColorSamples(const FloatArray& nRGB,
                            const FloatArray& RGBn);
        virtual RtVoid RelativeDetail(RtFloat relativedetail);
        virtual RtVoid Option(RtConstToken name, const ParamList& pList);
        virtual RtVoid AttributeBegin();
        virtual RtVoid AttributeEnd();
        virtual RtVoid Color(RtConstColor Cq);
        virtual RtVoid Opacity(RtConstColor Os);
        virtual RtVoid TextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2,
                            RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4,
                            RtFloat t4);
        virtual RtVoid LightSource(RtConstToken shadername, RtConstToken name,
                            const ParamList& pList);
        virtual RtVoid AreaLightSource(RtConstToken shadername,
                            RtConstToken name, const ParamList& pList);
        virtual RtVoid Illuminate(RtConstToken name, RtBoolean onoff);
        virtual RtVoid Surface(RtConstToken name, const ParamList& pList);
        virtual RtVoid Displacement(RtConstToken name, const ParamList& pList);
        virtual RtVoid Atmosphere(RtConstToken name, const ParamList& pList);
        virtual RtVoid Interior(RtConstToken name, const ParamList& pList);
        virtual RtVoid Exterior(RtConstToken name, const ParamList& pList);
        virtual RtVoid ShaderLayer(RtConstToken type, RtConstToken name,
                            RtConstToken layername, const ParamList& pList);
        virtual RtVoid ConnectShaderLayers(RtConstToken type,
                            RtConstToken layer1, RtConstToken variable1,
                            RtConstToken layer2, RtConstToken variable2);
        virtual RtVoid ShadingRate(RtFloat size);
        virtual RtVoid ShadingInterpolation(RtConstToken type);
        virtual RtVoid Matte(RtBoolean onoff);
        virtual RtVoid Bound(RtConstBound bound);
        virtual RtVoid Detail(RtConstBound bound);
        virtual RtVoid DetailRange(RtFloat offlow, RtFloat onlow,
                            RtFloat onhigh, RtFloat offhigh);
        virtual RtVoid GeometricApproximation(RtConstToken type,
                            RtFloat value);
        virtual RtVoid Orientation(RtConstToken orientation);
        virtual RtVoid ReverseOrientation();
        virtual RtVoid Sides(RtInt nsides);
        virtual RtVoid Identity();
        virtual RtVoid Transform(RtConstMatrix transform);
        virtual RtVoid ConcatTransform(RtConstMatrix transform);
        virtual RtVoid Perspective(RtFloat fov);
        virtual RtVoid Translate(RtFloat dx, RtFloat dy, RtFloat dz);
        virtual RtVoid Rotate(RtFloat angle, RtFloat dx, RtFloat dy,
                            RtFloat dz);
        virtual RtVoid Scale(RtFloat sx, RtFloat sy, RtFloat sz);
        virtual RtVoid Skew(RtFloat angle, RtFloat dx1, RtFloat dy1,
                            RtFloat dz1, RtFloat dx2, RtFloat dy2,
                            RtFloat dz2);
        virtual RtVoid CoordinateSystem(RtConstToken space);
        virtual RtVoid CoordSysTransform(RtConstToken space);
        virtual RtVoid TransformBegin();
        virtual RtVoid TransformEnd();
        virtual RtVoid Resource(RtConstToken handle, RtConstToken type,
                            const ParamList& pList);
        virtual RtVoid ResourceBegin();
        virtual RtVoid ResourceEnd();
        virtual RtVoid Attribute(RtConstToken name, const ParamList& pList);
        virtual RtVoid Polygon(const ParamList& pList);
        virtual RtVoid GeneralPolygon(const IntArray& nverts,
                            const ParamList& pList);
        virtual RtVoid PointsPolygons(const IntArray& nverts,
                            const IntArray& verts, const ParamList& pList);
        virtual RtVoid PointsGeneralPolygons(const IntArray& nloops,
                            const IntArray& nverts, const IntArray& verts,
                            const ParamList& pList);
        virtual RtVoid Basis(RtConstBasis ubasis, RtInt ustep,
                            RtConstBasis vbasis, RtInt vstep);
        virtual RtVoid Patch(RtConstToken type, const ParamList& pList);
        virtual RtVoid PatchMesh(RtConstToken type, RtInt nu,
                            RtConstToken uwrap, RtInt nv, RtConstToken vwrap,
                            const ParamList& pList);
        virtual RtVoid NuPatch(RtInt nu, RtInt uorder, const FloatArray& uknot,
                            RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder,
                            const FloatArray& vknot, RtFloat vmin, RtFloat vmax,
                            const ParamList& pList);
        virtual RtVoid TrimCurve(const IntArray& ncurves, const IntArray& order,
                            const FloatArray& knot, const FloatArray& min,
                            const FloatArray& max, const IntArray& n,
                            const FloatArray& u, const FloatArray& v,
                            const FloatArray& w);
        virtual RtVoid SubdivisionMesh(RtConstToken scheme,
                            const IntArray& nvertices, const IntArray& vertices,
                            const TokenArray& tags, const IntArray& nargs,
                            const IntArray& intargs,
                            const FloatArray& floatargs,
                            const ParamList& pList);
        virtual RtVoid Sphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList);
        virtual RtVoid Cone(RtFloat height, RtFloat radius, RtFloat thetamax,
                            const ParamList& pList);
        virtual RtVoid Cylinder(RtFloat radius, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList);
        virtual RtVoid Hyperboloid(RtConstPoint point1, RtConstPoint point2,
                            RtFloat thetamax, const ParamList& pList);
        virtual RtVoid Paraboloid(RtFloat rmax, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList);
        virtual RtVoid Disk(RtFloat height, RtFloat radius, RtFloat thetamax,
                            const ParamList& pList);
        virtual RtVoid Torus(RtFloat majorrad, RtFloat minorrad, RtFloat phimin,
                            RtFloat phimax, RtFloat thetamax,
                            const ParamList& pList);
        virtual RtVoid Points(const ParamList& pList);
        virtual RtVoid Curves(RtConstToken type, const IntArray& nvertices,
                            RtConstToken wrap, const ParamList& pList);
        virtual RtVoid Blobby(RtInt nleaf, const IntArray& code,
                            const FloatArray& floats, const TokenArray& strings,
                            const ParamList& pList);
        virtual RtVoid Procedural(RtPointer data, RtConstBound bound,
                            RtProcSubdivFunc refineproc,
                            RtProcFreeFunc freeproc);
        virtual RtVoid Geometry(RtConstToken type, const ParamList& pList);
        virtual RtVoid SolidBegin(RtConstToken type);
        virtual RtVoid SolidEnd();
        virtual RtVoid ObjectBegin(RtConstToken name);
        virtual RtVoid ObjectEnd();
        virtual RtVoid ObjectInstance(RtConstToken name);
        virtual RtVoid MotionBegin(const FloatArray& times);
        virtual RtVoid MotionEnd();
        virtual RtVoid MakeTexture(RtConstString imagefile,
                            RtConstString texturefile, RtConstToken swrap,
                            RtConstToken twrap, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList);
        virtual RtVoid MakeLatLongEnvironment(RtConstString imagefile,
                            RtConstString reflfile, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList);
        virtual RtVoid MakeCubeFaceEnvironment(RtConstString px,
                            RtConstString nx, RtConstString py,
                            RtConstString ny, RtConstString pz,
                            RtConstString nz, RtConstString reflfile,
                            RtFloat fov, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList);
        virtual RtVoid MakeShadow(RtConstString picfile,
                            RtConstString shadowfile, const ParamList& pList);
        virtual RtVoid MakeOcclusion(const StringArray& picfiles,
                            RtConstString shadowfile, const ParamList& pList);
        virtual RtVoid ErrorHandler(RtErrorFunc handler);
        virtual RtVoid ReadArchive(RtConstToken name,
                            RtArchiveCallback callback,
                            const ParamList& pList);
        virtual RtVoid ArchiveBegin(RtConstToken name, const ParamList& pList);
        virtual RtVoid ArchiveEnd();
        ///[[[end]]]
};


//------------------------------------------------------------------------------
/*
For reference, the method stubs below were originally generated with the
following cog code generator:

from Cheetah.Template import Template

methodTmpl = '''
$wrapDecl($riCxxMethodDecl($proc, className='RenderApi'), 80)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "$procName not implemented";
}
'''

for proc in riXml.findall('Procedures/'+'*'):
    if proc.tag == 'Procedure':
        if proc.findall('Rib'):
            procName = proc.findtext('Name')
            cog.out(str(Template(methodTmpl, searchList=locals())))
    elif proc.tag == 'Section' or proc.tag == 'SubSection':
        cog.outl('')
        cog.outl('//------------------------------------------------------------')
        cog.outl('// ' + proc.findtext('.'))

*/
//------------------------------------------------------------------------------

RtVoid RenderApi::Declare(RtConstString name, RtConstString declaration)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Declare not implemented";
}

//------------------------------------------------------------
// Graphics State

RtVoid RenderApi::FrameBegin(RtInt number)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "FrameBegin not implemented";
}

RtVoid RenderApi::FrameEnd()
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "FrameEnd not implemented";
}

RtVoid RenderApi::WorldBegin()
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "WorldBegin not implemented";
}

RtVoid RenderApi::WorldEnd()
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "WorldEnd not implemented";
}

//------------------------------------------------------------
// Conditional RIB

RtVoid RenderApi::IfBegin(RtConstString condition)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "IfBegin not implemented";
}

RtVoid RenderApi::ElseIf(RtConstString condition)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ElseIf not implemented";
}

RtVoid RenderApi::Else()
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Else not implemented";
}

RtVoid RenderApi::IfEnd()
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "IfEnd not implemented";
}

//------------------------------------------------------------
// Options

RtVoid RenderApi::Format(RtInt xresolution, RtInt yresolution,
                         RtFloat pixelaspectratio)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Format not implemented";
}

RtVoid RenderApi::FrameAspectRatio(RtFloat frameratio)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "FrameAspectRatio not implemented";
}

RtVoid RenderApi::ScreenWindow(RtFloat left, RtFloat right, RtFloat bottom,
                               RtFloat top)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ScreenWindow not implemented";
}

RtVoid RenderApi::CropWindow(RtFloat xmin, RtFloat xmax, RtFloat ymin,
                             RtFloat ymax)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "CropWindow not implemented";
}

RtVoid RenderApi::Projection(RtConstToken name, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Projection not implemented";
}

RtVoid RenderApi::Clipping(RtFloat cnear, RtFloat cfar)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Clipping not implemented";
}

RtVoid RenderApi::ClippingPlane(RtFloat x, RtFloat y, RtFloat z, RtFloat nx,
                                RtFloat ny, RtFloat nz)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ClippingPlane not implemented";
}

RtVoid RenderApi::DepthOfField(RtFloat fstop, RtFloat focallength,
                               RtFloat focaldistance)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "DepthOfField not implemented";
}

RtVoid RenderApi::Shutter(RtFloat opentime, RtFloat closetime)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Shutter not implemented";
}

RtVoid RenderApi::PixelVariance(RtFloat variance)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "PixelVariance not implemented";
}

RtVoid RenderApi::PixelSamples(RtFloat xsamples, RtFloat ysamples)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "PixelSamples not implemented";
}

RtVoid RenderApi::PixelFilter(RtFilterFunc function, RtFloat xwidth,
                              RtFloat ywidth)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "PixelFilter not implemented";
}

RtVoid RenderApi::Exposure(RtFloat gain, RtFloat gamma)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Exposure not implemented";
}

RtVoid RenderApi::Imager(RtConstToken name, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Imager not implemented";
}

RtVoid RenderApi::Quantize(RtConstToken type, RtInt one, RtInt min, RtInt max,
                           RtFloat ditheramplitude)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Quantize not implemented";
}

RtVoid RenderApi::Display(RtConstToken name, RtConstToken type,
                          RtConstToken mode, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Display not implemented";
}

RtVoid RenderApi::Hider(RtConstToken name, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Hider not implemented";
}

RtVoid RenderApi::ColorSamples(const FloatArray& nRGB, const FloatArray& RGBn)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ColorSamples not implemented";
}

RtVoid RenderApi::RelativeDetail(RtFloat relativedetail)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "RelativeDetail not implemented";
}

RtVoid RenderApi::Option(RtConstToken name, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Option not implemented";
}

//------------------------------------------------------------
// Attributes

RtVoid RenderApi::AttributeBegin()
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "AttributeBegin not implemented";
}

RtVoid RenderApi::AttributeEnd()
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "AttributeEnd not implemented";
}

RtVoid RenderApi::Color(RtConstColor Cq)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Color not implemented";
}

RtVoid RenderApi::Opacity(RtConstColor Os)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Opacity not implemented";
}

RtVoid RenderApi::TextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2,
                                     RtFloat t2, RtFloat s3, RtFloat t3,
                                     RtFloat s4, RtFloat t4)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "TextureCoordinates not implemented";
}

RtVoid RenderApi::LightSource(RtConstToken shadername, RtConstToken name,
                              const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "LightSource not implemented";
}

RtVoid RenderApi::AreaLightSource(RtConstToken shadername, RtConstToken name,
                                  const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "AreaLightSource not implemented";
}

RtVoid RenderApi::Illuminate(RtConstToken name, RtBoolean onoff)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Illuminate not implemented";
}

RtVoid RenderApi::Surface(RtConstToken name, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Surface not implemented";
}

RtVoid RenderApi::Displacement(RtConstToken name, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Displacement not implemented";
}

RtVoid RenderApi::Atmosphere(RtConstToken name, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Atmosphere not implemented";
}

RtVoid RenderApi::Interior(RtConstToken name, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Interior not implemented";
}

RtVoid RenderApi::Exterior(RtConstToken name, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Exterior not implemented";
}

RtVoid RenderApi::ShaderLayer(RtConstToken type, RtConstToken name,
                              RtConstToken layername, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ShaderLayer not implemented";
}

RtVoid RenderApi::ConnectShaderLayers(RtConstToken type, RtConstToken layer1,
                                      RtConstToken variable1,
                                      RtConstToken layer2,
                                      RtConstToken variable2)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ConnectShaderLayers not implemented";
}

RtVoid RenderApi::ShadingRate(RtFloat size)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ShadingRate not implemented";
}

RtVoid RenderApi::ShadingInterpolation(RtConstToken type)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ShadingInterpolation not implemented";
}

RtVoid RenderApi::Matte(RtBoolean onoff)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Matte not implemented";
}

RtVoid RenderApi::Bound(RtConstBound bound)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Bound not implemented";
}

RtVoid RenderApi::Detail(RtConstBound bound)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Detail not implemented";
}

RtVoid RenderApi::DetailRange(RtFloat offlow, RtFloat onlow, RtFloat onhigh,
                              RtFloat offhigh)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "DetailRange not implemented";
}

RtVoid RenderApi::GeometricApproximation(RtConstToken type, RtFloat value)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "GeometricApproximation not implemented";
}

RtVoid RenderApi::Orientation(RtConstToken orientation)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Orientation not implemented";
}

RtVoid RenderApi::ReverseOrientation()
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ReverseOrientation not implemented";
}

RtVoid RenderApi::Sides(RtInt nsides)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Sides not implemented";
}

//------------------------------------------------------------
// Transformations

RtVoid RenderApi::Identity()
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Identity not implemented";
}

RtVoid RenderApi::Transform(RtConstMatrix transform)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Transform not implemented";
}

RtVoid RenderApi::ConcatTransform(RtConstMatrix transform)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ConcatTransform not implemented";
}

RtVoid RenderApi::Perspective(RtFloat fov)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Perspective not implemented";
}

RtVoid RenderApi::Translate(RtFloat dx, RtFloat dy, RtFloat dz)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Translate not implemented";
}

RtVoid RenderApi::Rotate(RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Rotate not implemented";
}

RtVoid RenderApi::Scale(RtFloat sx, RtFloat sy, RtFloat sz)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Scale not implemented";
}

RtVoid RenderApi::Skew(RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1,
                       RtFloat dx2, RtFloat dy2, RtFloat dz2)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Skew not implemented";
}

RtVoid RenderApi::CoordinateSystem(RtConstToken space)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "CoordinateSystem not implemented";
}

RtVoid RenderApi::CoordSysTransform(RtConstToken space)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "CoordSysTransform not implemented";
}

RtVoid RenderApi::TransformBegin()
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "TransformBegin not implemented";
}

RtVoid RenderApi::TransformEnd()
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "TransformEnd not implemented";
}

//------------------------------------------------------------
// Resources

RtVoid RenderApi::Resource(RtConstToken handle, RtConstToken type,
                           const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Resource not implemented";
}

RtVoid RenderApi::ResourceBegin()
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ResourceBegin not implemented";
}

RtVoid RenderApi::ResourceEnd()
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ResourceEnd not implemented";
}

//------------------------------------------------------------
// Implementation-specific Attributes

RtVoid RenderApi::Attribute(RtConstToken name, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Attribute not implemented";
}

//------------------------------------------------------------
// Geometric Primitives

//------------------------------------------------------------
// Polygons

RtVoid RenderApi::Polygon(const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Polygon not implemented";
}

RtVoid RenderApi::GeneralPolygon(const IntArray& nverts,
                                 const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "GeneralPolygon not implemented";
}

RtVoid RenderApi::PointsPolygons(const IntArray& nverts, const IntArray& verts,
                                 const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "PointsPolygons not implemented";
}

RtVoid RenderApi::PointsGeneralPolygons(const IntArray& nloops,
                                        const IntArray& nverts,
                                        const IntArray& verts,
                                        const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "PointsGeneralPolygons not implemented";
}

//------------------------------------------------------------
// Patches

RtVoid RenderApi::Basis(RtConstBasis ubasis, RtInt ustep, RtConstBasis vbasis,
                        RtInt vstep)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Basis not implemented";
}

RtVoid RenderApi::Patch(RtConstToken type, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Patch not implemented";
}

RtVoid RenderApi::PatchMesh(RtConstToken type, RtInt nu, RtConstToken uwrap,
                            RtInt nv, RtConstToken vwrap,
                            const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "PatchMesh not implemented";
}

RtVoid RenderApi::NuPatch(RtInt nu, RtInt uorder, const FloatArray& uknot,
                          RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder,
                          const FloatArray& vknot, RtFloat vmin, RtFloat vmax,
                          const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "NuPatch not implemented";
}

RtVoid RenderApi::TrimCurve(const IntArray& ncurves, const IntArray& order,
                            const FloatArray& knot, const FloatArray& min,
                            const FloatArray& max, const IntArray& n,
                            const FloatArray& u, const FloatArray& v,
                            const FloatArray& w)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "TrimCurve not implemented";
}

//------------------------------------------------------------
// Subdivision Surfaces

RtVoid RenderApi::SubdivisionMesh(RtConstToken scheme,
                                  const IntArray& nvertices,
                                  const IntArray& vertices,
                                  const TokenArray& tags, const IntArray& nargs,
                                  const IntArray& intargs,
                                  const FloatArray& floatargs,
                                  const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "SubdivisionMesh not implemented";
}

//------------------------------------------------------------
// Quadrics

RtVoid RenderApi::Sphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
                         RtFloat thetamax, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Sphere not implemented";
}

RtVoid RenderApi::Cone(RtFloat height, RtFloat radius, RtFloat thetamax,
                       const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Cone not implemented";
}

RtVoid RenderApi::Cylinder(RtFloat radius, RtFloat zmin, RtFloat zmax,
                           RtFloat thetamax, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Cylinder not implemented";
}

RtVoid RenderApi::Hyperboloid(RtConstPoint point1, RtConstPoint point2,
                              RtFloat thetamax, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Hyperboloid not implemented";
}

RtVoid RenderApi::Paraboloid(RtFloat rmax, RtFloat zmin, RtFloat zmax,
                             RtFloat thetamax, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Paraboloid not implemented";
}

RtVoid RenderApi::Disk(RtFloat height, RtFloat radius, RtFloat thetamax,
                       const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Disk not implemented";
}

RtVoid RenderApi::Torus(RtFloat majorrad, RtFloat minorrad, RtFloat phimin,
                        RtFloat phimax, RtFloat thetamax,
                        const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Torus not implemented";
}

//------------------------------------------------------------
// Points and Curve Primitives

RtVoid RenderApi::Points(const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Points not implemented";
}

RtVoid RenderApi::Curves(RtConstToken type, const IntArray& nvertices,
                         RtConstToken wrap, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Curves not implemented";
}

//------------------------------------------------------------
// Blobby Implicit Surfaces

RtVoid RenderApi::Blobby(RtInt nleaf, const IntArray& code,
                         const FloatArray& floats, const TokenArray& strings,
                         const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Blobby not implemented";
}

//------------------------------------------------------------
// Procedural Primitives

RtVoid RenderApi::Procedural(RtPointer data, RtConstBound bound,
                             RtProcSubdivFunc refineproc,
                             RtProcFreeFunc freeproc)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Procedural not implemented";
}

//------------------------------------------------------------
// Implementation-specific Geometric Primitives

RtVoid RenderApi::Geometry(RtConstToken type, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "Geometry not implemented";
}

//------------------------------------------------------------
// Soids and Spatial Set Operations

RtVoid RenderApi::SolidBegin(RtConstToken type)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "SolidBegin not implemented";
}

RtVoid RenderApi::SolidEnd()
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "SolidEnd not implemented";
}

//------------------------------------------------------------
// Retained Geometry

RtVoid RenderApi::ObjectBegin(RtConstToken name)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ObjectBegin not implemented";
}

RtVoid RenderApi::ObjectEnd()
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ObjectEnd not implemented";
}

RtVoid RenderApi::ObjectInstance(RtConstToken name)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ObjectInstance not implemented";
}

//------------------------------------------------------------
// Motion

RtVoid RenderApi::MotionBegin(const FloatArray& times)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "MotionBegin not implemented";
}

RtVoid RenderApi::MotionEnd()
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "MotionEnd not implemented";
}

//------------------------------------------------------------
// External Resources

//------------------------------------------------------------
// Texture Map Utilities

RtVoid RenderApi::MakeTexture(RtConstString imagefile,
                              RtConstString texturefile, RtConstToken swrap,
                              RtConstToken twrap, RtFilterFunc filterfunc,
                              RtFloat swidth, RtFloat twidth,
                              const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "MakeTexture not implemented";
}

RtVoid RenderApi::MakeLatLongEnvironment(RtConstString imagefile,
                                         RtConstString reflfile,
                                         RtFilterFunc filterfunc,
                                         RtFloat swidth, RtFloat twidth,
                                         const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "MakeLatLongEnvironment not implemented";
}

RtVoid RenderApi::MakeCubeFaceEnvironment(RtConstString px, RtConstString nx,
                                          RtConstString py, RtConstString ny,
                                          RtConstString pz, RtConstString nz,
                                          RtConstString reflfile, RtFloat fov,
                                          RtFilterFunc filterfunc,
                                          RtFloat swidth, RtFloat twidth,
                                          const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "MakeCubeFaceEnvironment not implemented";
}

RtVoid RenderApi::MakeShadow(RtConstString picfile, RtConstString shadowfile,
                             const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "MakeShadow not implemented";
}

RtVoid RenderApi::MakeOcclusion(const StringArray& picfiles,
                                RtConstString shadowfile,
                                const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "MakeOcclusion not implemented";
}

//------------------------------------------------------------
// Errors

RtVoid RenderApi::ErrorHandler(RtErrorFunc handler)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ErrorHandler not implemented";
}

//------------------------------------------------------------
// Archive Files

RtVoid RenderApi::ReadArchive(RtConstToken name, RtArchiveCallback callback,
                              const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ReadArchive not implemented";
}

RtVoid RenderApi::ArchiveBegin(RtConstToken name, const ParamList& pList)
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ArchiveBegin not implemented";
}

RtVoid RenderApi::ArchiveEnd()
{
    AQSIS_LOG_WARNING(m_services.errorHandler(), EqE_Unimplement)
        << "ArchiveEnd not implemented";
}


//------------------------------------------------------------------------------
ApiServices::ApiServices()
    : m_api(),
    m_parser(),
    m_filterChain(),
    m_errorHandler()
{
    m_api.reset(new RenderApi(*this));
    addFilter("inlinearchive");
    addFilter("validate");
}


//--------------------------------------------------
Ri::RendererServices* createRenderer()
{
    return new ApiServices();
}

} // namespace Aqsis
