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

#include "renderer.h"

#include <cstring>
#include <tiffio.h>

#include "grid.h"
#include "gridstorage.h"
#include "microquadsampler.h"
#include "simple.h"

//------------------------------------------------------------------------------
// Minimal wrapper around a renderer instance to provide control context for
// when surfaces push split/diced objects back into the render's queue.
class TessellationContextImpl : public TessellationContext
{
    private:
        Renderer& m_renderer;
        GridStorageBuilder m_builder;
        const Renderer::SurfaceHolder* m_parentSurface;

    public:
        TessellationContextImpl(Renderer& renderer)
            : m_renderer(renderer),
            m_builder(),
            m_parentSurface(0)
        { }

        void setParent(Renderer::SurfaceHolder& parent)
        {
            m_parentSurface = &parent;
        }

        virtual const Options& options()
        {
            return m_renderer.m_opts;
        }
        virtual const Attributes& attributes()
        {
            return *(m_parentSurface->attrs);
        }

        virtual void push(const boost::shared_ptr<Geometry>& geom)
        {
            m_renderer.push(geom, *m_parentSurface);
        }
        virtual void push(const boost::shared_ptr<Grid>& grid)
        {
            m_renderer.push(grid, *m_parentSurface);
        }

        virtual GridStorageBuilder& gridStorageBuilder()
        {
            // Add required stdvars for sampling & shader input.  These are the
            // vars which can be deduced from other primvars (eg, Ng is deduced
            // from P), from the attribute state (eg, Cs) or otherwise.
            //
            // Also add storage for required shader output vars, eg N.
            //m_builder.add(m_renderer.);
            //m_builder.setFromGeom();
            //m_builder.add(Stdvar::Cs, 
            m_builder.clear();
//            if(m_parentSurface.attrs.surfaceShader)
//            {
//                const Shader& shader = *m_parentSurface.attrs.surfaceShader;
//                VarSet& inVars = shader->inputVars();
//                for(int i = 0; i < inVars.size(); ++i)
//                {
//                    if(inVars)
//                    m_builder.add
//                }
//            }
            return m_builder;
        }
};


//------------------------------------------------------------------------------
// Renderer implementation, utility stuff.

static void writeHeader(TIFF* tif, Options& opts, int nchans, bool useFloat)
{
    uint16 bitsPerSample = 8;
    uint16 photometric = PHOTOMETRIC_RGB;
    uint16 sampleFormat = SAMPLEFORMAT_UINT;
    if(useFloat)
    {
        bitsPerSample = 8*sizeof(float);
        sampleFormat = SAMPLEFORMAT_IEEEFP;
    }
    if(nchans == 1)
        photometric = PHOTOMETRIC_MINISBLACK;
    // Write TIFF header
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, uint32(opts.xRes));
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, uint32(opts.yRes));
    TIFFSetField(tif, TIFFTAG_ORIENTATION, uint16(ORIENTATION_TOPLEFT));
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, uint16(PLANARCONFIG_CONTIG));
    TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, uint16(RESUNIT_NONE));
    TIFFSetField(tif, TIFFTAG_XRESOLUTION, 1.0f);
    TIFFSetField(tif, TIFFTAG_YRESOLUTION, 1.0f);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, uint16(COMPRESSION_LZW));
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, uint16(nchans));
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric);
    TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, sampleFormat);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, 0));
}

static void quantize(const float* in, int nPix, int nSamps, int pixStride,
                     uint8* out)
{
    for(int j = 0; j < nPix; ++j)
    {
        for(int i = 0; i < nSamps; ++i)
            out[i] = static_cast<uint8>(Imath::clamp(255*in[i], 0.0f, 255.0f));
        in += pixStride;
        out += nSamps;
    }
}

static void strided_memcpy(void* dest, const void* src, size_t nElems,
                           size_t blockSize, size_t srcStride)
{
    char* d = reinterpret_cast<char*>(dest);
    const char* s = reinterpret_cast<const char*>(src);
    for(size_t j = 0; j < nElems; ++j)
    {
        std::memcpy(d, s, blockSize);
        d += blockSize;
        s += srcStride;
    }
}

// TODO: Should probably be a member of OutvarList.
static int numOutChans(const OutvarList& outVars)
{
    int sampsPerPix = 0;
    for(int i = 0, iend = outVars.size(); i < iend; ++i)
        sampsPerPix += outVars[i].scalarSize();
    return sampsPerPix;
}

// Save image to a TIFF file.
void Renderer::saveImages(const std::string& baseFileName)
{
    int pixStride = numOutChans(m_outVars);
    for(int i = 0, nFiles = m_outVars.size(); i < nFiles; ++i)
    {
        int nSamps = m_outVars[i].scalarSize();

        std::string fileName = baseFileName;
        fileName += "_";
        fileName += m_outVars[i].name.c_str();
        fileName += ".tif";

        TIFF* tif = TIFFOpen(fileName.c_str(), "w");
        if(!tif)
        {
            std::cerr << "Could not open file " << fileName << "\n";
            continue;
        }

        // Don't quatize if we've got depth data.
        bool doQuantize = m_outVars[i] != Stdvar::z;
        writeHeader(tif, m_opts, nSamps, !doQuantize);

        // Write RGB image data
        int rowSize = nSamps*m_opts.xRes*(doQuantize ? 1 : sizeof(float));
        boost::scoped_array<uint8> lineBuf(new uint8[rowSize]);
        for(int line = 0; line < m_opts.yRes; ++line)
        {
            const float* src = &m_image[0] + pixStride*line*m_opts.xRes
                               + m_outVars[i].offset;
            if(doQuantize)
            {
                quantize(src, m_opts.xRes, nSamps, pixStride, lineBuf.get());
            }
            else
            {
                strided_memcpy(lineBuf.get(), src, m_opts.xRes,
                               nSamps*sizeof(float), pixStride*sizeof(float));
            }
            TIFFWriteScanline(tif, reinterpret_cast<tdata_t>(lineBuf.get()),
                            uint32(line));
        }

        TIFFClose(tif);
    }
}


//------------------------------------------------------------------------------
// Guts of the renderer

/// Fill a vector with the default no-hit fragment sample values
void Renderer::defaultSamples(float* defaultSamps)
{
    // Determine index of depth output data, if any.
    int zOffset = -1;
    OutvarList::const_iterator zIter = std::find(m_outVars.begin(),
                                                 m_outVars.end(), Stdvar::z);
    if(zIter != m_outVars.end())
        zOffset = zIter->offset;
    int pixStride = numOutChans(m_outVars);
    // Set up default values for samples.
    std::memset(defaultSamps, 0, pixStride*sizeof(float));
    if(zOffset >= 0)
        defaultSamps[zOffset] = FLT_MAX;
}

/// Initialize the sample and image arrays.
void Renderer::initSamples()
{
    // Initialize sample array
    m_samples.resize(m_opts.xRes*m_opts.yRes);
    // Initialize sample positions to contain
    for(int j = 0; j < m_opts.yRes; ++j)
    {
        for(int i = 0; i < m_opts.xRes; ++i)
            m_samples[j*m_opts.xRes + i] = Sample(Vec2(i+0.5f, j+0.5f));
    }
    // Initialize image array.
    int nOutChans = numOutChans(m_outVars);
    m_defOutSamps.resize(nOutChans);
    defaultSamples(&m_defOutSamps[0]);
    m_image.resize(nOutChans*m_opts.xRes*m_opts.yRes);
    for(int i = 0, nPix = m_opts.xRes*m_opts.yRes; i < nPix; ++i)
    {
        std::memcpy(&m_image[nOutChans*i], &m_defOutSamps[0],
                    nOutChans*sizeof(float));
    }
}


/// Push geometry into the render queue
void Renderer::push(const boost::shared_ptr<Geometry>& geom,
                    const SurfaceHolder& parentSurface)
{
    int splitCount = parentSurface.splitCount + 1;
    // Get bound in camera space.
    Box bound = geom->bound();
    if(bound.min.z < FLT_EPSILON && splitCount > m_opts.maxSplits)
    {
        std::cerr << "Max eye splits encountered; geometry discarded\n";
        return;
    }
    // Cull if outside near/far clipping range
    if(bound.max.z < m_opts.clipNear || bound.min.z > m_opts.clipFar)
        return;
    // Transform bound to raster space.  TODO: The need to do this
    // here seems undesirable somehow; perhaps use objects in world
    // space + arbitrary spatial bounding computation?
    float minz = bound.min.z;
    float maxz = bound.min.z;
    bound = transformBound(bound, m_camToRas);
    bound.min.z = minz;
    bound.max.z = maxz;
    // Cull if outside xy extent of image
    if(   bound.max.x < 0 || bound.min.x > m_opts.xRes
       || bound.max.y < 0 || bound.min.y > m_opts.yRes )
    {
        return;
    }
    // If we get to here the surface should be rendered, so push it
    // onto the queue.
    m_surfaces.push(SurfaceHolder(geom, splitCount, bound,
                                  parentSurface.attrs));
}


// Push a grid onto the render queue
void Renderer::push(const boost::shared_ptr<Grid>& grid,
                    const SurfaceHolder& parentSurface)
{
    // For now, just rasterize it directly.
    switch(grid->type())
    {
        case GridType_QuadSimple:
            rasterizeSimple(static_cast<QuadGridSimple&>(*grid),
                            *parentSurface.attrs);
            break;
        case GridType_Quad:
            rasterize<QuadGrid, MicroQuadSampler>(
                    static_cast<QuadGrid&>(*grid), *parentSurface.attrs);
            break;
    }
}

Renderer::Renderer(const Options& opts, const Mat4& camToScreen,
                   const VarList& outVars)
    : m_opts(opts),
    m_surfaces(),
    m_samples(),
    m_outVars(),
    m_image(),
    m_camToRas()
{
    // Set up output variables.  Default is to use Cs.
    if(outVars.size() == 0)
    {
        m_outVars.push_back(OutvarSpec(Stdvar::Cs, 0));
    }
    else
    {
        int offset = 0;
        for(int i = 0, iend = outVars.size(); i < iend; ++i)
        {
            m_outVars.push_back(OutvarSpec(outVars[i], offset));
            offset += outVars[i].scalarSize();
        }
    }
    // Set up camera -> raster matrix
    m_camToRas = camToScreen
        * Mat4().setScale(Vec3(0.5,-0.5,0))
        * Mat4().setTranslation(Vec3(0.5,0.5,0))
        * Mat4().setScale(Vec3(m_opts.xRes, m_opts.yRes, 1));
}

void Renderer::add(const boost::shared_ptr<Geometry>& geom,
                   Attributes& attrs)
{
    // TODO: Transform to camera space?
    //
    // We need a dummy holder here for the "parent" surface; it's only
    // really required to pass the attributes and split count on.
    SurfaceHolder fakeParent(boost::shared_ptr<Geometry>(), -1, Box(), &attrs);
    push(geom, fakeParent);
}

// Render all surfaces and save resulting image.
void Renderer::render()
{
    // Splitting transform.  Allowing this to be different from the
    // projection matrix lets us examine a fixed set of grids
    // independently of the viewpoint.
    Mat4 splitTrans = m_camToRas;
//            Mat4().setScale(Vec3(0.5,-0.5,0))
//        * Mat4().setTranslation(Vec3(0.5,0.5,0))
//        * Mat4().setScale(Vec3(m_opts.xRes, m_opts.yRes, 1));

    // Use the same tessellation context for all split/dice events
    TessellationContextImpl tessContext(*this);

    initSamples();
    while(!m_surfaces.empty())
    {
        SurfaceHolder s = m_surfaces.top();
        m_surfaces.pop();
        tessContext.setParent(s);
        s.geom->splitdice(splitTrans, tessContext);
    }
    saveImages("test");
}


// Render a grid by rasterizing each micropolygon.
template<typename GridT, typename PolySamplerT>
//__attribute__((flatten))
void Renderer::rasterize(GridT& grid, const Attributes& attrs)
{
    // Determine index of depth output data, if any.
    int zOffset = -1;
    OutvarList::const_iterator zIter = std::find(m_outVars.begin(),
                                                 m_outVars.end(), Stdvar::z);
    if(zIter != m_outVars.end())
        zOffset = zIter->offset;
    int pixStride = numOutChans(m_outVars);

    // Project grid into raster coordinates.
    grid.project(m_camToRas);

    // Construct a sampler for the polygons in the grid
    PolySamplerT poly(grid, attrs, m_outVars);
    // iterate over all micropolys in the grid & render each one.
    while(poly.valid())
    {
        Box bound = poly.bound();

        // Bounding box for relevant samples, clamped to image extent.
        const int sx = Imath::clamp(Imath::floor(bound.min.x), 0, m_opts.xRes);
        const int ex = Imath::clamp(Imath::floor(bound.max.x)+1, 0, m_opts.xRes);
        const int sy = Imath::clamp(Imath::floor(bound.min.y), 0, m_opts.yRes);
        const int ey = Imath::clamp(Imath::floor(bound.max.y)+1, 0, m_opts.yRes);
        poly.initHitTest();
        poly.initInterpolator();

        // for each sample position in the bound
        for(int ix = sx; ix < ex; ++ix)
        {
            for(int iy = sy; iy < ey; ++iy)
            {
                int idx = m_opts.xRes*iy + ix;
                Sample& samp = m_samples[idx];
//                // Early out if definitely hidden
//                if(samp.z < bound.min.z)
//                    continue;
                // Test whether sample hits the micropoly
                if(!poly.contains(samp))
                    continue;
                // Determine hit depth
                poly.interpolateAt(samp);
                float z = poly.interpolateZ();
                if(samp.z < z)
                    continue; // Ignore if hit is hidden
                samp.z = z;
                // Generate & store a fragment
                float* out = &m_image[pixStride*idx];
                // Initialize fragment data with the default value.
                std::memcpy(out, &m_defOutSamps[0], pixStride*sizeof(float));
                // Store interpolated fragment data
                poly.interpolate(out);
                if(zOffset >= 0)
                    out[zOffset] = z;
            }
        }
        poly.next();
    }
}


// Simple rasterizer function for simple quad grids only!
//
// The purpose of this version rasterizer function is to provide a simple
// version to benchmark against.
void Renderer::rasterizeSimple(QuadGridSimple& grid, const Attributes& attrs)
{
    int pixStride = numOutChans(m_outVars);

    // Project grid into raster coordinates.
    grid.project(m_camToRas);
    const Vec3* P = grid.P(0);

    // Point-in-polygon tester
    PointInQuad hitTest;

    // Shading interpolation
    InvBilin invBilin;
    // Whether to use smooth shading or not.
    bool smoothShading = attrs.smoothShading;
    // Micropoly uv coordinates for smooth shading
    Vec2 uv(0.0f);

    // iterate over all micropolys in the grid & render each one.
    for(int v = 0, nv = grid.nv(); v < nv-1; ++v)
    for(int u = 0, nu = grid.nu(); u < nu-1; ++u)
    {
        Vec3 a = P[nu*v + u];
        Vec3 b = P[nu*v + u+1];
        Vec3 c = P[nu*(v+1) + u+1];
        Vec3 d = P[nu*(v+1) + u];
        bool flipEnd = (u+v)%2;

        Box bound(a);
        bound.extendBy(b);
        bound.extendBy(c);
        bound.extendBy(d);

        // Bounding box for relevant samples, clamped to image extent.
        const int sx = Imath::clamp(Imath::floor(bound.min.x), 0, m_opts.xRes);
        const int ex = Imath::clamp(Imath::floor(bound.max.x)+1, 0, m_opts.xRes);
        const int sy = Imath::clamp(Imath::floor(bound.min.y), 0, m_opts.yRes);
        const int ey = Imath::clamp(Imath::floor(bound.max.y)+1, 0, m_opts.yRes);

        hitTest.init(vec2_cast(a), vec2_cast(b),
                     vec2_cast(c), vec2_cast(d), flipEnd);
        if(smoothShading)
        {
            invBilin.init(vec2_cast(a), vec2_cast(b),
                          vec2_cast(d), vec2_cast(c));
        }

        // for each sample position in the bound
        for(int ix = sx; ix < ex; ++ix)
        {
            for(int iy = sy; iy < ey; ++iy)
            {
                int idx = m_opts.xRes*iy + ix;
                Sample& samp = m_samples[idx];
//                // Early out if definitely hidden
//                if(samp.z < bound.min.z)
//                    continue;
                // Test whether sample hits the micropoly
                if(!hitTest(samp))
                    continue;
                // Determine hit depth
                float z;
                if(smoothShading)
                {
                    uv = invBilin(samp.p);
                    z = bilerp(a.z, b.z, d.z, c.z, uv);
                }
                else
                {
                    z = a.z;
                }
                if(samp.z < z)
                    continue; // Ignore if hit is hidden
                samp.z = z;
                // Store z value.
                m_image[pixStride*idx] = z;
            }
        }
    }
}

