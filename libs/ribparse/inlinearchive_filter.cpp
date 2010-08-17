// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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

/// \file Filter to save and interpolate InlineArchive calls.
///
/// \author Chris Foster

#include "ricxx_filter.h"

#include <stack>
#include <vector>

#include <aqsis/util/exception.h>
#include "ricxx_cache.h"
#include "errorhandler.h"

namespace Aqsis {

/// Filter to save and interpolate InlineArchive calls and Object instances.
///
/// This filter caches all the calls between ArchiveBegin and ArchiveEnd into
/// memory under the given archive name.  Whenever a ReadArchive call with the
/// same name is processed, we insert the contents of the cached stream back
/// into the first filter.
///
/// Some PRMan docs found online suggest that inline archives may be
/// arbitrarily nested, so we allow this behaviour - we keeping a stack of
/// currently active archives, with commands being cached into the innermost
/// archive.
///
/// Because the object instancing mechanism is so similar to inline archive
/// handling, we include that here as well.
///
/// TODO: Should we flush archives/objects at end of frame scope ??
class InlineArchiveFilter : public Ri::Filter
{
    private:
        // Named cached stream.
        struct CachedArchive
        {
            std::string name;
            CachedRiStream stream;

            CachedArchive(RtConstString name) : name(name) {}
        };
        std::vector<CachedArchive*> m_archives;
        std::stack<CachedRiStream*> m_activeArchives;
        std::vector<CachedRiStream*> m_objectInstances;
        CachedRiStream* m_currCache;
        bool m_inObject;

    public:
        InlineArchiveFilter(Ri::RendererServices& services, Ri::Renderer& out)
            : Ri::Filter(services, out),
            m_archives(),
            m_activeArchives(),
            m_objectInstances(),
            m_currCache(0),
            m_inObject(false)
        { }

        ~InlineArchiveFilter()
        {
            for(size_t i = 0; i < m_archives.size(); ++i)
                delete m_archives[i];
            for(size_t i = 0; i < m_objectInstances.size(); ++i)
                delete m_objectInstances[i];
        }

        // TODO - remove this!
        typedef RtPointer RtArchiveHandle;

        virtual RtArchiveHandle ArchiveBegin(RtConstToken name, const ParamList& pList)
        {
            if(m_currCache)
                m_activeArchives.push(m_currCache);
            m_archives.push_back(new CachedArchive(name));
            m_currCache = &m_archives.back()->stream;
            return const_cast<RtToken>(name);
        }

        virtual RtVoid ArchiveEnd()
        {
            if(m_activeArchives.empty())
                m_currCache = 0;
            else
            {
                m_currCache = m_activeArchives.top();
                m_activeArchives.pop();
            }
        }

        virtual RtVoid ReadArchive(RtConstToken name, RtArchiveCallback callback,
                            const ParamList& pList)
        {
            if(m_currCache)
            {
                m_currCache->push_back(new RiCache::ReadArchive(name, callback, pList));
                return;
            }
            // Search for the archive name in the cached archives.
            for(int i = 0, iend = m_archives.size(); i < iend; ++i)
            {
                if(m_archives[i]->name == name)
                {
                    // If we find it, replay the archive into the start of the
                    // filter chain.
                    m_archives[i]->stream.replay(services().firstFilter());
                    return;
                }
            }
            // If not found in our archive list it's probably on-disk, so we
            // let subsequent layers handle it.
            return nextFilter().ReadArchive(name, callback, pList);
        }

        virtual RtObjectHandle ObjectBegin()
        {
            if(m_currCache)
            {
                // If we're in an inline archive, always just cache the object
                // call, don't instantiate it.
                m_currCache->push_back(new RiCache::ObjectBegin());
                return 0;
            }
            else
            {
                // If not currently in an archive, instantiate the object.
                m_objectInstances.push_back(new CachedRiStream());
                m_currCache = m_objectInstances.back();
                m_inObject = true;
                return m_currCache;
            }
        }

        virtual RtVoid ObjectEnd()
        {
            if(m_currCache && !m_inObject)
            {
                // If we're not in an object, but are in an archive, cache the
                // call.
                m_currCache->push_back(new RiCache::ObjectEnd());
            }
            else if(m_currCache)
            {
                // Else if we're currently making an object instance, terminate
                // it.
                m_inObject = false;
                m_currCache = 0;
                assert(m_activeArchives.empty());
            }
            // Else it's a scoping error; just ignore the ObjectEnd.
        }

        virtual RtVoid ObjectInstance(RtObjectHandle handle)
        {
            if(m_currCache)
            {
                m_currCache->push_back(new RiCache::ObjectInstance(handle));
                return;
            }
            const CachedRiStream* cache = static_cast<CachedRiStream*>(handle);
            // Search for handle to make sure it's a valid instance.
            bool found = false;
            for(int i = 0, iend = m_objectInstances.size(); i < iend; ++i)
            {
                if(m_objectInstances[i] == cache)
                {
                    found = true;
                    break;
                }
            }
            if(found)
                cache->replay(services().firstFilter());
            else
                AQSIS_LOG_ERROR(services().errorHandler(), EqE_BadHandle)
                    << "Bad object handle " << handle;
        }

        virtual RtVoid ArchiveRecord(RtConstToken type, const char* string)
        {
            if(!m_currCache)
                nextFilter().ArchiveRecord(type, string);
        }

        // Code generator for autogenerated method declarations
        /*[[[cog
        from codegenutils import *
        riXml = parseXmlTree(riXmlPath)
        from Cheetah.Template import Template

        exclude = set(('ArchiveBegin', 'ArchiveEnd', 'ReadArchive',
                       'ObjectBegin', 'ObjectEnd', 'ObjectInstance'))

        methodTemplate = r'''
        virtual $wrapDecl($riCxxMethodDecl($proc), 72, wrapIndent=20)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::${procName}($callArgs));
            #if $proc.findtext('ReturnType') != 'RtVoid'
                return 0;
            #else
                return;
            #end if
            }
            return nextFilter().${procName}($callArgs);
        }
        '''

        for proc in riXml.findall('Procedures/Procedure'):
            procName = proc.findtext('Name')
            if proc.haschild('Rib') and procName not in exclude:
                callArgs = ', '.join(wrapperCallArgList(proc))
                cog.out(str(Template(methodTemplate, searchList=locals())));

        ]]]*/

        virtual RtToken Declare(RtConstString name, RtConstString declaration)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Declare(name, declaration));
                return 0;
            }
            return nextFilter().Declare(name, declaration);
        }

        virtual RtVoid FrameBegin(RtInt number)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::FrameBegin(number));
                return;
            }
            return nextFilter().FrameBegin(number);
        }

        virtual RtVoid FrameEnd()
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::FrameEnd());
                return;
            }
            return nextFilter().FrameEnd();
        }

        virtual RtVoid WorldBegin()
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::WorldBegin());
                return;
            }
            return nextFilter().WorldBegin();
        }

        virtual RtVoid WorldEnd()
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::WorldEnd());
                return;
            }
            return nextFilter().WorldEnd();
        }

        virtual RtVoid IfBegin(RtConstString condition)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::IfBegin(condition));
                return;
            }
            return nextFilter().IfBegin(condition);
        }

        virtual RtVoid ElseIf(RtConstString condition)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::ElseIf(condition));
                return;
            }
            return nextFilter().ElseIf(condition);
        }

        virtual RtVoid Else()
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Else());
                return;
            }
            return nextFilter().Else();
        }

        virtual RtVoid IfEnd()
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::IfEnd());
                return;
            }
            return nextFilter().IfEnd();
        }

        virtual RtVoid Format(RtInt xresolution, RtInt yresolution,
                            RtFloat pixelaspectratio)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Format(xresolution, yresolution, pixelaspectratio));
                return;
            }
            return nextFilter().Format(xresolution, yresolution, pixelaspectratio);
        }

        virtual RtVoid FrameAspectRatio(RtFloat frameratio)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::FrameAspectRatio(frameratio));
                return;
            }
            return nextFilter().FrameAspectRatio(frameratio);
        }

        virtual RtVoid ScreenWindow(RtFloat left, RtFloat right, RtFloat bottom,
                            RtFloat top)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::ScreenWindow(left, right, bottom, top));
                return;
            }
            return nextFilter().ScreenWindow(left, right, bottom, top);
        }

        virtual RtVoid CropWindow(RtFloat xmin, RtFloat xmax, RtFloat ymin,
                            RtFloat ymax)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::CropWindow(xmin, xmax, ymin, ymax));
                return;
            }
            return nextFilter().CropWindow(xmin, xmax, ymin, ymax);
        }

        virtual RtVoid Projection(RtConstToken name, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Projection(name, pList));
                return;
            }
            return nextFilter().Projection(name, pList);
        }

        virtual RtVoid Clipping(RtFloat cnear, RtFloat cfar)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Clipping(cnear, cfar));
                return;
            }
            return nextFilter().Clipping(cnear, cfar);
        }

        virtual RtVoid ClippingPlane(RtFloat x, RtFloat y, RtFloat z, RtFloat nx,
                            RtFloat ny, RtFloat nz)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::ClippingPlane(x, y, z, nx, ny, nz));
                return;
            }
            return nextFilter().ClippingPlane(x, y, z, nx, ny, nz);
        }

        virtual RtVoid DepthOfField(RtFloat fstop, RtFloat focallength,
                            RtFloat focaldistance)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::DepthOfField(fstop, focallength, focaldistance));
                return;
            }
            return nextFilter().DepthOfField(fstop, focallength, focaldistance);
        }

        virtual RtVoid Shutter(RtFloat opentime, RtFloat closetime)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Shutter(opentime, closetime));
                return;
            }
            return nextFilter().Shutter(opentime, closetime);
        }

        virtual RtVoid PixelVariance(RtFloat variance)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::PixelVariance(variance));
                return;
            }
            return nextFilter().PixelVariance(variance);
        }

        virtual RtVoid PixelSamples(RtFloat xsamples, RtFloat ysamples)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::PixelSamples(xsamples, ysamples));
                return;
            }
            return nextFilter().PixelSamples(xsamples, ysamples);
        }

        virtual RtVoid PixelFilter(RtFilterFunc function, RtFloat xwidth,
                            RtFloat ywidth)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::PixelFilter(function, xwidth, ywidth));
                return;
            }
            return nextFilter().PixelFilter(function, xwidth, ywidth);
        }

        virtual RtVoid Exposure(RtFloat gain, RtFloat gamma)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Exposure(gain, gamma));
                return;
            }
            return nextFilter().Exposure(gain, gamma);
        }

        virtual RtVoid Imager(RtConstToken name, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Imager(name, pList));
                return;
            }
            return nextFilter().Imager(name, pList);
        }

        virtual RtVoid Quantize(RtConstToken type, RtInt one, RtInt min, RtInt max,
                            RtFloat ditheramplitude)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Quantize(type, one, min, max, ditheramplitude));
                return;
            }
            return nextFilter().Quantize(type, one, min, max, ditheramplitude);
        }

        virtual RtVoid Display(RtConstToken name, RtConstToken type, RtConstToken mode,
                            const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Display(name, type, mode, pList));
                return;
            }
            return nextFilter().Display(name, type, mode, pList);
        }

        virtual RtVoid Hider(RtConstToken name, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Hider(name, pList));
                return;
            }
            return nextFilter().Hider(name, pList);
        }

        virtual RtVoid ColorSamples(const FloatArray& nRGB, const FloatArray& RGBn)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::ColorSamples(nRGB, RGBn));
                return;
            }
            return nextFilter().ColorSamples(nRGB, RGBn);
        }

        virtual RtVoid RelativeDetail(RtFloat relativedetail)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::RelativeDetail(relativedetail));
                return;
            }
            return nextFilter().RelativeDetail(relativedetail);
        }

        virtual RtVoid Option(RtConstToken name, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Option(name, pList));
                return;
            }
            return nextFilter().Option(name, pList);
        }

        virtual RtVoid AttributeBegin()
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::AttributeBegin());
                return;
            }
            return nextFilter().AttributeBegin();
        }

        virtual RtVoid AttributeEnd()
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::AttributeEnd());
                return;
            }
            return nextFilter().AttributeEnd();
        }

        virtual RtVoid Color(RtConstColor Cq)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Color(Cq));
                return;
            }
            return nextFilter().Color(Cq);
        }

        virtual RtVoid Opacity(RtConstColor Os)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Opacity(Os));
                return;
            }
            return nextFilter().Opacity(Os);
        }

        virtual RtVoid TextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2,
                            RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4,
                            RtFloat t4)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::TextureCoordinates(s1, t1, s2, t2, s3, t3, s4, t4));
                return;
            }
            return nextFilter().TextureCoordinates(s1, t1, s2, t2, s3, t3, s4, t4);
        }

        virtual RtLightHandle LightSource(RtConstToken name, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::LightSource(name, pList));
                return 0;
            }
            return nextFilter().LightSource(name, pList);
        }

        virtual RtLightHandle AreaLightSource(RtConstToken name,
                            const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::AreaLightSource(name, pList));
                return 0;
            }
            return nextFilter().AreaLightSource(name, pList);
        }

        virtual RtVoid Illuminate(RtLightHandle light, RtBoolean onoff)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Illuminate(light, onoff));
                return;
            }
            return nextFilter().Illuminate(light, onoff);
        }

        virtual RtVoid Surface(RtConstToken name, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Surface(name, pList));
                return;
            }
            return nextFilter().Surface(name, pList);
        }

        virtual RtVoid Displacement(RtConstToken name, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Displacement(name, pList));
                return;
            }
            return nextFilter().Displacement(name, pList);
        }

        virtual RtVoid Atmosphere(RtConstToken name, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Atmosphere(name, pList));
                return;
            }
            return nextFilter().Atmosphere(name, pList);
        }

        virtual RtVoid Interior(RtConstToken name, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Interior(name, pList));
                return;
            }
            return nextFilter().Interior(name, pList);
        }

        virtual RtVoid Exterior(RtConstToken name, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Exterior(name, pList));
                return;
            }
            return nextFilter().Exterior(name, pList);
        }

        virtual RtVoid ShaderLayer(RtConstToken type, RtConstToken name,
                            RtConstToken layername, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::ShaderLayer(type, name, layername, pList));
                return;
            }
            return nextFilter().ShaderLayer(type, name, layername, pList);
        }

        virtual RtVoid ConnectShaderLayers(RtConstToken type, RtConstToken layer1,
                            RtConstToken variable1, RtConstToken layer2,
                            RtConstToken variable2)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::ConnectShaderLayers(type, layer1, variable1, layer2, variable2));
                return;
            }
            return nextFilter().ConnectShaderLayers(type, layer1, variable1, layer2, variable2);
        }

        virtual RtVoid ShadingRate(RtFloat size)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::ShadingRate(size));
                return;
            }
            return nextFilter().ShadingRate(size);
        }

        virtual RtVoid ShadingInterpolation(RtConstToken type)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::ShadingInterpolation(type));
                return;
            }
            return nextFilter().ShadingInterpolation(type);
        }

        virtual RtVoid Matte(RtBoolean onoff)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Matte(onoff));
                return;
            }
            return nextFilter().Matte(onoff);
        }

        virtual RtVoid Bound(RtConstBound bound)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Bound(bound));
                return;
            }
            return nextFilter().Bound(bound);
        }

        virtual RtVoid Detail(RtConstBound bound)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Detail(bound));
                return;
            }
            return nextFilter().Detail(bound);
        }

        virtual RtVoid DetailRange(RtFloat offlow, RtFloat onlow, RtFloat onhigh,
                            RtFloat offhigh)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::DetailRange(offlow, onlow, onhigh, offhigh));
                return;
            }
            return nextFilter().DetailRange(offlow, onlow, onhigh, offhigh);
        }

        virtual RtVoid GeometricApproximation(RtConstToken type, RtFloat value)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::GeometricApproximation(type, value));
                return;
            }
            return nextFilter().GeometricApproximation(type, value);
        }

        virtual RtVoid Orientation(RtConstToken orientation)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Orientation(orientation));
                return;
            }
            return nextFilter().Orientation(orientation);
        }

        virtual RtVoid ReverseOrientation()
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::ReverseOrientation());
                return;
            }
            return nextFilter().ReverseOrientation();
        }

        virtual RtVoid Sides(RtInt nsides)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Sides(nsides));
                return;
            }
            return nextFilter().Sides(nsides);
        }

        virtual RtVoid Identity()
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Identity());
                return;
            }
            return nextFilter().Identity();
        }

        virtual RtVoid Transform(RtConstMatrix transform)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Transform(transform));
                return;
            }
            return nextFilter().Transform(transform);
        }

        virtual RtVoid ConcatTransform(RtConstMatrix transform)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::ConcatTransform(transform));
                return;
            }
            return nextFilter().ConcatTransform(transform);
        }

        virtual RtVoid Perspective(RtFloat fov)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Perspective(fov));
                return;
            }
            return nextFilter().Perspective(fov);
        }

        virtual RtVoid Translate(RtFloat dx, RtFloat dy, RtFloat dz)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Translate(dx, dy, dz));
                return;
            }
            return nextFilter().Translate(dx, dy, dz);
        }

        virtual RtVoid Rotate(RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Rotate(angle, dx, dy, dz));
                return;
            }
            return nextFilter().Rotate(angle, dx, dy, dz);
        }

        virtual RtVoid Scale(RtFloat sx, RtFloat sy, RtFloat sz)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Scale(sx, sy, sz));
                return;
            }
            return nextFilter().Scale(sx, sy, sz);
        }

        virtual RtVoid Skew(RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1,
                            RtFloat dx2, RtFloat dy2, RtFloat dz2)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Skew(angle, dx1, dy1, dz1, dx2, dy2, dz2));
                return;
            }
            return nextFilter().Skew(angle, dx1, dy1, dz1, dx2, dy2, dz2);
        }

        virtual RtVoid CoordinateSystem(RtConstToken space)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::CoordinateSystem(space));
                return;
            }
            return nextFilter().CoordinateSystem(space);
        }

        virtual RtVoid CoordSysTransform(RtConstToken space)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::CoordSysTransform(space));
                return;
            }
            return nextFilter().CoordSysTransform(space);
        }

        virtual RtVoid TransformBegin()
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::TransformBegin());
                return;
            }
            return nextFilter().TransformBegin();
        }

        virtual RtVoid TransformEnd()
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::TransformEnd());
                return;
            }
            return nextFilter().TransformEnd();
        }

        virtual RtVoid Resource(RtConstToken handle, RtConstToken type,
                            const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Resource(handle, type, pList));
                return;
            }
            return nextFilter().Resource(handle, type, pList);
        }

        virtual RtVoid ResourceBegin()
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::ResourceBegin());
                return;
            }
            return nextFilter().ResourceBegin();
        }

        virtual RtVoid ResourceEnd()
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::ResourceEnd());
                return;
            }
            return nextFilter().ResourceEnd();
        }

        virtual RtVoid Attribute(RtConstToken name, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Attribute(name, pList));
                return;
            }
            return nextFilter().Attribute(name, pList);
        }

        virtual RtVoid Polygon(const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Polygon(pList));
                return;
            }
            return nextFilter().Polygon(pList);
        }

        virtual RtVoid GeneralPolygon(const IntArray& nverts, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::GeneralPolygon(nverts, pList));
                return;
            }
            return nextFilter().GeneralPolygon(nverts, pList);
        }

        virtual RtVoid PointsPolygons(const IntArray& nverts, const IntArray& verts,
                            const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::PointsPolygons(nverts, verts, pList));
                return;
            }
            return nextFilter().PointsPolygons(nverts, verts, pList);
        }

        virtual RtVoid PointsGeneralPolygons(const IntArray& nloops,
                            const IntArray& nverts, const IntArray& verts,
                            const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::PointsGeneralPolygons(nloops, nverts, verts, pList));
                return;
            }
            return nextFilter().PointsGeneralPolygons(nloops, nverts, verts, pList);
        }

        virtual RtVoid Basis(RtConstBasis ubasis, RtInt ustep, RtConstBasis vbasis,
                            RtInt vstep)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Basis(ubasis, ustep, vbasis, vstep));
                return;
            }
            return nextFilter().Basis(ubasis, ustep, vbasis, vstep);
        }

        virtual RtVoid Patch(RtConstToken type, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Patch(type, pList));
                return;
            }
            return nextFilter().Patch(type, pList);
        }

        virtual RtVoid PatchMesh(RtConstToken type, RtInt nu, RtConstToken uwrap,
                            RtInt nv, RtConstToken vwrap,
                            const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::PatchMesh(type, nu, uwrap, nv, vwrap, pList));
                return;
            }
            return nextFilter().PatchMesh(type, nu, uwrap, nv, vwrap, pList);
        }

        virtual RtVoid NuPatch(RtInt nu, RtInt uorder, const FloatArray& uknot,
                            RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder,
                            const FloatArray& vknot, RtFloat vmin, RtFloat vmax,
                            const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::NuPatch(nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax, pList));
                return;
            }
            return nextFilter().NuPatch(nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax, pList);
        }

        virtual RtVoid TrimCurve(const IntArray& ncurves, const IntArray& order,
                            const FloatArray& knot, const FloatArray& min,
                            const FloatArray& max, const IntArray& n,
                            const FloatArray& u, const FloatArray& v,
                            const FloatArray& w)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::TrimCurve(ncurves, order, knot, min, max, n, u, v, w));
                return;
            }
            return nextFilter().TrimCurve(ncurves, order, knot, min, max, n, u, v, w);
        }

        virtual RtVoid SubdivisionMesh(RtConstToken scheme, const IntArray& nvertices,
                            const IntArray& vertices, const TokenArray& tags,
                            const IntArray& nargs, const IntArray& intargs,
                            const FloatArray& floatargs,
                            const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::SubdivisionMesh(scheme, nvertices, vertices, tags, nargs, intargs, floatargs, pList));
                return;
            }
            return nextFilter().SubdivisionMesh(scheme, nvertices, vertices, tags, nargs, intargs, floatargs, pList);
        }

        virtual RtVoid Sphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Sphere(radius, zmin, zmax, thetamax, pList));
                return;
            }
            return nextFilter().Sphere(radius, zmin, zmax, thetamax, pList);
        }

        virtual RtVoid Cone(RtFloat height, RtFloat radius, RtFloat thetamax,
                            const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Cone(height, radius, thetamax, pList));
                return;
            }
            return nextFilter().Cone(height, radius, thetamax, pList);
        }

        virtual RtVoid Cylinder(RtFloat radius, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Cylinder(radius, zmin, zmax, thetamax, pList));
                return;
            }
            return nextFilter().Cylinder(radius, zmin, zmax, thetamax, pList);
        }

        virtual RtVoid Hyperboloid(RtConstPoint point1, RtConstPoint point2,
                            RtFloat thetamax, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Hyperboloid(point1, point2, thetamax, pList));
                return;
            }
            return nextFilter().Hyperboloid(point1, point2, thetamax, pList);
        }

        virtual RtVoid Paraboloid(RtFloat rmax, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Paraboloid(rmax, zmin, zmax, thetamax, pList));
                return;
            }
            return nextFilter().Paraboloid(rmax, zmin, zmax, thetamax, pList);
        }

        virtual RtVoid Disk(RtFloat height, RtFloat radius, RtFloat thetamax,
                            const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Disk(height, radius, thetamax, pList));
                return;
            }
            return nextFilter().Disk(height, radius, thetamax, pList);
        }

        virtual RtVoid Torus(RtFloat majorrad, RtFloat minorrad, RtFloat phimin,
                            RtFloat phimax, RtFloat thetamax,
                            const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Torus(majorrad, minorrad, phimin, phimax, thetamax, pList));
                return;
            }
            return nextFilter().Torus(majorrad, minorrad, phimin, phimax, thetamax, pList);
        }

        virtual RtVoid Points(const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Points(pList));
                return;
            }
            return nextFilter().Points(pList);
        }

        virtual RtVoid Curves(RtConstToken type, const IntArray& nvertices,
                            RtConstToken wrap, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Curves(type, nvertices, wrap, pList));
                return;
            }
            return nextFilter().Curves(type, nvertices, wrap, pList);
        }

        virtual RtVoid Blobby(RtInt nleaf, const IntArray& code,
                            const FloatArray& floats, const TokenArray& strings,
                            const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Blobby(nleaf, code, floats, strings, pList));
                return;
            }
            return nextFilter().Blobby(nleaf, code, floats, strings, pList);
        }

        virtual RtVoid Procedural(RtPointer data, RtConstBound bound,
                            RtProcSubdivFunc refineproc,
                            RtProcFreeFunc freeproc)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Procedural(data, bound, refineproc, freeproc));
                return;
            }
            return nextFilter().Procedural(data, bound, refineproc, freeproc);
        }

        virtual RtVoid Geometry(RtConstToken type, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::Geometry(type, pList));
                return;
            }
            return nextFilter().Geometry(type, pList);
        }

        virtual RtVoid SolidBegin(RtConstToken type)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::SolidBegin(type));
                return;
            }
            return nextFilter().SolidBegin(type);
        }

        virtual RtVoid SolidEnd()
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::SolidEnd());
                return;
            }
            return nextFilter().SolidEnd();
        }

        virtual RtVoid MotionBegin(const FloatArray& times)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::MotionBegin(times));
                return;
            }
            return nextFilter().MotionBegin(times);
        }

        virtual RtVoid MotionEnd()
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::MotionEnd());
                return;
            }
            return nextFilter().MotionEnd();
        }

        virtual RtVoid MakeTexture(RtConstString imagefile, RtConstString texturefile,
                            RtConstToken swrap, RtConstToken twrap,
                            RtFilterFunc filterfunc, RtFloat swidth,
                            RtFloat twidth, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::MakeTexture(imagefile, texturefile, swrap, twrap, filterfunc, swidth, twidth, pList));
                return;
            }
            return nextFilter().MakeTexture(imagefile, texturefile, swrap, twrap, filterfunc, swidth, twidth, pList);
        }

        virtual RtVoid MakeLatLongEnvironment(RtConstString imagefile,
                            RtConstString reflfile, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::MakeLatLongEnvironment(imagefile, reflfile, filterfunc, swidth, twidth, pList));
                return;
            }
            return nextFilter().MakeLatLongEnvironment(imagefile, reflfile, filterfunc, swidth, twidth, pList);
        }

        virtual RtVoid MakeCubeFaceEnvironment(RtConstString px, RtConstString nx,
                            RtConstString py, RtConstString ny,
                            RtConstString pz, RtConstString nz,
                            RtConstString reflfile, RtFloat fov,
                            RtFilterFunc filterfunc, RtFloat swidth,
                            RtFloat twidth, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::MakeCubeFaceEnvironment(px, nx, py, ny, pz, nz, reflfile, fov, filterfunc, swidth, twidth, pList));
                return;
            }
            return nextFilter().MakeCubeFaceEnvironment(px, nx, py, ny, pz, nz, reflfile, fov, filterfunc, swidth, twidth, pList);
        }

        virtual RtVoid MakeShadow(RtConstString picfile, RtConstString shadowfile,
                            const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::MakeShadow(picfile, shadowfile, pList));
                return;
            }
            return nextFilter().MakeShadow(picfile, shadowfile, pList);
        }

        virtual RtVoid MakeOcclusion(const StringArray& picfiles,
                            RtConstString shadowfile, const ParamList& pList)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::MakeOcclusion(picfiles, shadowfile, pList));
                return;
            }
            return nextFilter().MakeOcclusion(picfiles, shadowfile, pList);
        }

        virtual RtVoid ErrorHandler(RtErrorFunc handler)
        {
            if(m_currCache) {
                m_currCache->push_back(new RiCache::ErrorHandler(handler));
                return;
            }
            return nextFilter().ErrorHandler(handler);
        }
        ///[[[end]]]
};

Ri::Renderer* createInlineArchiveFilter(Ri::RendererServices& services,
                        Ri::Renderer& out, const Ri::ParamList& pList)
{
    return new InlineArchiveFilter(services, out);
}

} // namespace Aqsis
// vi: set et:
