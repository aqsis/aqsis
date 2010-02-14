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

#include "attributes.h"
#include "grid.h"
#include "gridstorage.h"
#include "microquadsampler.h"
#include "sample.h"
#include "simple.h"


/// Container for geometry and geometry metadata
struct Renderer::SurfaceHolder
{
    boost::shared_ptr<Geometry> geom; ///< Pointer to geometry
    int splitCount; ///< Number of times the geometry has been split
    Box bound;      ///< Bound in camera coordinates
    Attributes* attrs; ///< Surface attribute state

    SurfaceHolder(const boost::shared_ptr<Geometry>& geom,
                    int splitCount, Box bound, Attributes* attrs)
        : geom(geom),
        splitCount(splitCount),
        bound(bound),
        attrs(attrs)
    { }
};

/// Ordering functor for surfaces in the render queue
class Renderer::SurfaceOrder
{
    private:
        // desired bucket height in camera coordinates
        float m_bucketHeight;
    public:
        SurfaceOrder() : m_bucketHeight(16) {}

        bool operator()(const SurfaceHolder& a,
                        const SurfaceHolder& b) const
        {
            float ya = a.bound.min.y;
            float yb = b.bound.min.y;
            if(ya < yb - m_bucketHeight)
                return true;
            else if(yb < ya - m_bucketHeight)
                return false;
            else
                return a.bound.min.x < b.bound.min.x;
        }
};


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
            // Fill in any grid data which didn't get filled in by the surface
            // during the dicing stage.
            //
            // TODO: Alias optimization:
            //  - For perspective projections I may be aliased to P rather
            //    than copied
            //  - N may sometimes be aliased to Ng
            //
            GridStorage& stor = grid->storage();
            DataView<Vec3> P = stor.get(StdIndices::P);
            // Deal with normals N & Ng
            DataView<Vec3> Ng = stor.get(StdIndices::Ng);
            DataView<Vec3> N = stor.get(StdIndices::N);
            if(Ng)
                grid->calculateNormals(Ng, P);
            if(N && !m_builder.dicedByGeom(stor, StdIndices::N))
            {
                if(Ng)
                    copy(N, Ng, stor.nverts());
                else
                    grid->calculateNormals(N, P);
            }
            // Deal with view direction.
            if(DataView<Vec3> I = stor.get(StdIndices::I))
            {
                // In shading coordinates, I is just equal to P for
                // perspective projections.  (TODO: orthographic)
                copy(I, P, stor.nverts());
            }
            // Push the grid into the render pipeline
            m_renderer.push(grid, *m_parentSurface);
        }

        virtual GridStorageBuilder& gridStorageBuilder()
        {
            // Add required stdvars for sampling, shader input & output.
            //
            // TODO: Perhaps some of this messy logic can be done once & cached
            // in the surface holder?
            //
            m_builder.clear();
            // TODO: AOV stuff shouldn't be conditional on surfaceShader
            // existing.
            if(m_parentSurface->attrs->surfaceShader)
            {
                // Renderer arbitrary output vars
                const OutvarSet& aoVars = m_renderer.m_outVars;
                const Shader& shader = *m_parentSurface->attrs->surfaceShader;
                const VarSet& inVars = shader.inputVars();
                // P is guaranteed to be dice by the geometry.
                m_builder.add(Stdvar::P,  GridStorage::Varying);
                // Add stdvars computed by the renderer if they're needed in
                // the shader or AOVs.  These are:
                //
                //   I - computed from P
                //   du, dv - compute from u,v
                //   E - eye position is always 0
                //   ncomps - computed from options
                //   time - always 0 (?)
                if(inVars.contains(StdIndices::I) || aoVars.contains(Stdvar::I))
                    m_builder.add(Stdvar::I,  GridStorage::Varying);
                // TODO: du, dv, E, ncomps, time

                // Some geometric stdvars - dPdu, dPdv, Ng - may in principle
                // be filled in by the geometry.  For now we just estimate
                // these in the renderer core using derivatives of P.
                //
                // TODO: dPdu, dPdv
                if(inVars.contains(Stdvar::Ng) || aoVars.contains(Stdvar::Ng))
                    m_builder.add(Stdvar::Ng,  GridStorage::Varying);
                // N is a tricky case; it may inherit a value from Ng if it's
                // not set explicitly, but only if Ng is never assigned to by
                // the shaders (implicitly via P).
                //
                // Thoughts about variable flow for N:
                //
                // How can N differ from Ng??
                // - If it's a primvar
                // - If N is changed in the displacement shader
                // - If P is changed in the displacement shader (implies Ng is
                //   too)
                //
                // 1) Add N if contained in inVars or outVars or AOVs
                // 2) Add if it's in the primvar list
                // 3) Allocate if N can differ from Ng, *and* both N & Ng are
                //    in the list.
                // 4) Dice if it's a primvar
                // 5) Alias to Ng if N & Ng can't differ, else memcpy.
                //
                // Lesson: N and Ng should be the same, unless N is specified
                // by the user (ie, is a primvar) or N & Ng diverge during
                // displacement (ie, N is set or P is set)
                if(inVars.contains(Stdvar::N) || aoVars.contains(Stdvar::N))
                    m_builder.add(Stdvar::N,  GridStorage::Varying);

                // Stdvars which should be attached at geometry creation:
                // Cs, Os - from attributes state
                // u, v
                // s, t - copy u,v ?
                //
                // Stdvars which must be filled in by the geometry:
                // P
                //
                // Stdvars which can be filled in by either geometry or
                // renderer.  Here's how you'd do them with the renderer:
                // N - computed from Ng
                // dPdu, dPdv - computed from P
                // Ng - computed from P

                // Add shader outputs
                const VarSet& outVars = shader.outputVars();
                if(outVars.contains(StdIndices::Ci) && aoVars.contains(Stdvar::Ci))
                    m_builder.add(Stdvar::Ci, GridStorage::Varying);
                if(outVars.contains(StdIndices::Oi) && aoVars.contains(Stdvar::Oi))
                    m_builder.add(Stdvar::Oi, GridStorage::Varying);
                // TODO: Replace the limited stuff above with the following:
                /*
                for(var in outVars)
                {
                    // TODO: signal somehow that these vars are to be retained
                    // after shading.
                    if(aovs.contains(var))
                        m_builder.add(var);
                }
                */
            }
            m_builder.setFromGeom();
            return m_builder;
        }
};


//------------------------------------------------------------------------------
// Storage for samples positions and output fragments
class SampleStorage
{
    private:
        int m_fragRowStride; ///< length of a row of samples.
        int m_fragSize;      ///< number of floats in a fragment
        int m_xRes;

        std::vector<Sample> m_samples;        ///< sample positions

        std::vector<float> m_defaultFragment; ///< Default fragment channels
        std::vector<float> m_fragments;       ///< array of fragments

        /// Fill an array with the default no-hit fragment sample values
        static void fillDefault(std::vector<float>& defaultFrag,
                                const OutvarSet& outVars)
        {
            int nchans = 0;
            for(int i = 0, iend = outVars.size(); i < iend; ++i)
                nchans += outVars[i].scalarSize();
            // Set up default values for samples.
            defaultFrag.assign(nchans, 0.0f);
            // Fill in default depth if relevant
            int zIdx = outVars.find(StdOutInd::z);
            if(zIdx != OutvarSet::npos)
            {
                int zOffset = outVars[zIdx].offset;
                defaultFrag[zOffset] = FLT_MAX;
            }
        }

    public:
        SampleStorage(const OutvarSet& outVars, const Options& opts)
            : m_fragRowStride(0),
            m_fragSize(0),
            m_xRes(opts.xRes),
            m_samples(),
            m_defaultFragment(),
            m_fragments()
        {
            int npixels = opts.xRes*opts.yRes;
            // Initialize sample array
            m_samples.resize(npixels);
            for(int j = 0; j < opts.yRes; ++j)
            {
                for(int i = 0; i < opts.xRes; ++i)
                    m_samples[j*opts.xRes + i] = Sample(Vec2(i+0.5f, j+0.5f));
            }
            // Initialize default fragment
            fillDefault(m_defaultFragment, outVars);
            m_fragSize = m_defaultFragment.size();
            m_fragRowStride = opts.xRes*m_fragSize;
            // Initialize fragment array
            m_fragments.resize(m_fragSize*npixels);
            const float* defFrag = &m_defaultFragment[0];
            for(int i = 0; i < npixels; ++i)
            {
                std::memcpy(&m_fragments[m_fragSize*i], defFrag,
                            m_fragSize*sizeof(float));
            }
        }

//        class Iterator
//        {
//            private:
//                int m_sx;
//                int m_ex;
//                int m_ey;
//                int m_x;
//                int m_y;
//
//                Sample* m_sample;
//                float* m_fragment;
//            public:
//                Iterator(int sx, int ex, int sy, int ey, SampleStorage)
//                    : m_sx(sx),
//                    m_ex(ex),
//                    m_ey(ey),
//                    m_x(sx),
//                    m_y(sx < ex ? sy : ey)
//                { }
//
//                Iterator& operator++()
//                {
//                    ++m_x;
//                    ++m_sample;
//                    m_fragment += m_storage.m_fragSize;
//                    if(m_x == m_ex)
//                    {
//                        // Advance to next row.
//                        m_x = m_sx;
//                        ++m_y;
//                        m_sample +=
//                    }
//                    return *this;
//                }
//
//                bool valid()
//                {
//                    return m_y < m_ey;
//                }
//
//                Sample& sample() { return *m_sample; }
//                float* fragment() { return *m_fragment; }
//        };
//
//        Iterator begin(int startx, int endx, int starty, int endy)
//        {
//            return Iterator(startx, endx, starty, endy, *this);
//        }

        /// Get a sample at the given index
        Sample& sample(int ix, int iy)
        {
            return m_samples[iy*m_xRes + ix];
        }

        float* fragment(int ix, int iy)
        {
            return &m_fragments[(iy*m_xRes + ix)*m_fragSize];
        }

        /// Get a scanline of the output image.
        const float* outputScanline(int line) const
        {
            return &m_fragments[0] + line*m_fragRowStride;
        }

        int fragmentSize() const { return m_defaultFragment.size(); }
        const float* defaultFragment() const { return &m_defaultFragment[0]; }
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

// Save image to a TIFF file.
void Renderer::saveImages(const std::string& baseFileName)
{
    int pixStride = m_sampStorage->fragmentSize();
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

        // Write image data
        int rowSize = nSamps*m_opts.xRes*(doQuantize ? 1 : sizeof(float));
        boost::scoped_array<uint8> lineBuf(new uint8[rowSize]);
        for(int line = 0; line < m_opts.yRes; ++line)
        {
            const float* src = m_sampStorage->outputScanline(line)
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

/// Initialize the sample and image arrays.
void Renderer::initSamples()
{
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
    m_surfaces->push(SurfaceHolder(geom, splitCount, bound,
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
            if(parentSurface.attrs->surfaceShader)
                parentSurface.attrs->surfaceShader->shade(*grid);
            rasterize<QuadGrid, MicroQuadSampler>(
                    static_cast<QuadGrid&>(*grid), *parentSurface.attrs);
            break;
    }
}

Renderer::Renderer(const Options& opts, const Mat4& camToScreen,
                   const VarList& outVars)
    : m_opts(opts),
    m_surfaces(new SurfaceQueue()),
    m_outVars(),
    m_sampStorage(),
    m_camToRas()
{
    // Set up output variables.  Default is to use Cs.
    std::vector<OutvarSpec> outVarsInit;
    if(outVars.size() == 0)
    {
        outVarsInit.push_back(OutvarSpec(Stdvar::Cs, 0));
    }
    else
    {
        for(int i = 0, iend = outVars.size(); i < iend; ++i)
            outVarsInit.push_back(OutvarSpec(outVars[i], 0));
        std::sort(outVarsInit.begin(), outVarsInit.end());
        // Generate the output offsets after sorting, so that the order of
        // outVars is the same as the order in the output image.  This isn't
        // strictly necessary, but (1) in-order iteration during sampling seems
        // like a good idea, and (2) it's less confusing outside this init step.
        int offset = 0;
        for(int i = 0, iend = outVars.size(); i < iend; ++i)
        {
            outVarsInit[i].offset = offset;
            offset += outVarsInit[i].scalarSize();
        }
    }
    m_outVars.assign(outVarsInit.begin(), outVarsInit.end());
    // Set up camera -> raster matrix
    m_camToRas = camToScreen
        * Mat4().setScale(Vec3(0.5,-0.5,0))
        * Mat4().setTranslation(Vec3(0.5,0.5,0))
        * Mat4().setScale(Vec3(m_opts.xRes, m_opts.yRes, 1));
}

// Trivial destructor.  Only here to prevent renderer implementation details
// leaking (SurfaceQueue destructor is called implicitly)
Renderer::~Renderer() { }

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
    m_sampStorage.reset(new SampleStorage(m_outVars, m_opts));
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
    while(!m_surfaces->empty())
    {
        SurfaceHolder s = m_surfaces->top();
        m_surfaces->pop();
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
    int zIdx = m_outVars.find(StdOutInd::z);
    if(zIdx != OutvarSet::npos)
        zOffset = m_outVars[zIdx].offset;
    int fragSize = m_sampStorage->fragmentSize();
    const float* defaultFrag = m_sampStorage->defaultFragment();

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
        for(int iy = sy; iy < ey; ++iy)
        {
            for(int ix = sx; ix < ex; ++ix)
            {
                Sample& samp = m_sampStorage->sample(ix, iy);
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
                float* out = m_sampStorage->fragment(ix, iy);
                // Initialize fragment data with the default value.
                std::memcpy(out, defaultFrag, fragSize*sizeof(float));
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
        for(int iy = sy; iy < ey; ++iy)
        {
            for(int ix = sx; ix < ex; ++ix)
            {
                Sample& samp = m_sampStorage->sample(ix, iy);
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
                m_sampStorage->fragment(ix, iy)[0] = z;
            }
        }
    }
}

