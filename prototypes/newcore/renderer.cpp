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

#include "renderer.h"

#include <cstring>
#include <tiffio.h>

#include "attributes.h"
#include "grid.h"
#include "gridstorage.h"
#include "microquadsampler.h"
#include "refcount.h"
#include "sample.h"
#include "samplestorage.h"
#include "simple.h"
#include "splitstore.h"
#include "tessellation.h"


//------------------------------------------------------------------------------
/// Circle of confusion class for depth of field
class CircleOfConfusion
{
    private:
        float m_focalDistance;
        float m_cocMult;
        float m_cocOffset;

    public:
        CircleOfConfusion(float fstop, float focalLength, float focalDistance,
                          const Mat4& camToRaster)
        {
            m_focalDistance = focalDistance;
            m_cocMult = 0.5*focalLength/fstop * focalDistance*focalLength
                                              / (focalDistance - focalLength);
            // Get multiplier into raster units.
            m_cocMult *= camToRaster[0][0];
            m_cocOffset = m_cocMult/focalDistance;
        }

        /// Shift the vertex P on the circle of confusion.
        ///
        /// P is updated to position it would have if viewed with a pinhole
        /// camera at the position lensPos.
        ///
        void lensShift(Vec3& P, const Vec2& lensPos) const
        {
            float cocSize = std::fabs(m_cocMult/P.z - m_cocOffset);
            P.x -= lensPos.x * cocSize;
            P.y -= lensPos.y * cocSize;
        }

        /// Compute the minimum lensShift inside the interval [z1,z2]
        float minShiftForBound(float z1, float z2) const
        {
            // First check whether the bound spans the focal plane.
            if((z1 <= m_focalDistance && z2 >= m_focalDistance) ||
               (z1 >= m_focalDistance && z2 <= m_focalDistance))
                return 0;
            // Otherwise, the minimum focal blur is achieved at one of the
            // z-extents of the bound.
            return std::min(std::fabs(m_cocMult/z1 - m_cocOffset),
                            std::fabs(m_cocMult/z2 - m_cocOffset));
        }
};

/// Adjust desired micropolygon width based on focal or motion blurring.
static float micropolyBlurWidth(const GeomHolderPtr& holder,
                                const CircleOfConfusion* coc)
{
    const Attributes& attrs = holder->attrs();
    float polyLength = std::sqrt(attrs.shadingRate);
    if(coc)
    {
        // Adjust shading rate proportionally with the CoC area.  This
        // ensures that we don't waste time rendering high resolution
        // micropolygons in parts of the scene which are very blurry.
        // Depending on the sampling algorithm, it can also make things
        // asymptotically faster.
        //
        // We need a factor which decides the desired ratio of the
        // diameter of the circle of confusion to the width of a
        // micropolygon.  The factor lengthRatio = 0.16 was chosen by
        // some experiments demanding that using focusFactor = 1 yield
        // results almost visually indistingushable from focusFactor = 0.
        //
        // Two main experiments were used to get lengthRatio:
        //   1) Randomly coloured micropolygons: with an input of
        //      ShadingRate = 1 and focusFactor = 1, randomly coloured
        //      micropolys (Ci = random()) should all blend together
        //      with no large regions of colour, even in image regions
        //      with lots of focal blur.
        //   2) A scene with multiple strong specular highlights (a
        //      bilinear patch with displacement shader P +=
        //      0.1*sin(40*v)*cos(20*u) * normalize(N); ): This should
        //      look indistingushable from the result obtained with
        //      focusFactor = 1, regardless of the amount of focal
        //      blur.
        //
        const float minCoC = coc->minShiftForBound(
                holder->bound().min.z, holder->bound().max.z);
        const float lengthRatio = 0.16;
        polyLength *= max(1.0f, lengthRatio*attrs.focusFactor*minCoC);
    }
    return polyLength;
}


//------------------------------------------------------------------------------
// Renderer implementation, utility stuff.

static void writeHeader(TIFF* tif, const Imath::V2i& imageSize,
                        int nchans, bool useFloat)
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
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, uint32(imageSize.x));
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, uint32(imageSize.y));
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
            out[i] = static_cast<uint8>(clamp(255*in[i], 0.0f, 255.0f));
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

namespace {
template<typename T>
void clampOptBelow(T& val, const char* name, const T& minVal)
{
    if(val < minVal)
    {
        std::cerr << "Warning: Option " << name << " = " << val
                  << " is too small.  Clamping to " << minVal << "\n";
        val = minVal;
    }
}
} // anon namespace

void Renderer::sanitizeOptions(Options& opts)
{
#define CLAMP_OPT_BELOW(optname, minVal) \
    clampOptBelow(opts.optname, #optname, minVal)
    CLAMP_OPT_BELOW(maxSplits, 0);
    CLAMP_OPT_BELOW(gridSize, 1);
    CLAMP_OPT_BELOW(clipNear, FLT_EPSILON);
    CLAMP_OPT_BELOW(clipFar, opts.clipNear);
    CLAMP_OPT_BELOW(xRes, 1);
    CLAMP_OPT_BELOW(yRes, 1);
    CLAMP_OPT_BELOW(bucketSize, 0);
    CLAMP_OPT_BELOW(superSamp.x, 1);
    CLAMP_OPT_BELOW(superSamp.y, 1);
    CLAMP_OPT_BELOW(shutterMax, opts.shutterMin);
}

// Save image to a TIFF file.
void Renderer::saveImages(const std::string& baseFileName)
{
    int pixStride = m_sampStorage->fragmentSize();
    Imath::V2i imageSize = m_sampStorage->outputSize();
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
        writeHeader(tif, imageSize, nSamps, !doQuantize);

        // Write image data
        int rowSize = nSamps*imageSize.x*(doQuantize ? 1 : sizeof(float));
        boost::scoped_array<uint8> lineBuf(new uint8[rowSize]);
        for(int line = 0; line < imageSize.y; ++line)
        {
            const float* src = m_sampStorage->outputScanline(line)
                               + m_outVars[i].offset;
            if(doQuantize)
            {
                quantize(src, imageSize.x, nSamps, pixStride, lineBuf.get());
            }
            else
            {
                strided_memcpy(lineBuf.get(), src, imageSize.x,
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

/// Push geometry into the render queue
void Renderer::push(const GeomHolderPtr& holder)
{
    // Get bound in camera space.
    Box& bound = holder->bound();
    if(bound.min.z < FLT_EPSILON && holder->splitCount() > m_opts.maxSplits)
    {
        std::cerr << "Max eye splits encountered; geometry discarded\n";
        return;
    }
    // Cull if outside near/far clipping range
    if(bound.max.z < m_opts.clipNear || bound.min.z > m_opts.clipFar)
        return;
    // Transform bound to raster space.
    //
    // TODO: Support arbitrary coordinate systems for the displacement bound
    bound.min -= Vec3(holder->attrs().displacementBound);
    bound.max += Vec3(holder->attrs().displacementBound);
    float minz = bound.min.z;
    float maxz = bound.max.z;
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
    m_surfaces->insert(holder);
}


// Push a grid onto the render queue
void Renderer::push(const GridHolderPtr& holder)
{
    holder->shade();
    // Rasterize grids right away, never store them.
    Grid& grid = holder->grid();
    if(holder->isDeforming() || m_opts.fstop != FLT_MAX)
    {
        // Sample with motion blur or depth of field
        switch(grid.type())
        {
            case GridType_QuadSimple:
                assert(0 && "motion blur not implemented for simple grid");
                break;
            case GridType_Quad:
                motionRasterize<QuadGrid, MicroQuadSampler>(*holder);
                break;
        }
    }
    else
    {
        // No motion blur or depth of field
        switch(grid.type())
        {
            case GridType_QuadSimple:
                rasterizeSimple(static_cast<QuadGridSimple&>(grid),
                                holder->attrs());
                break;
            case GridType_Quad:
                rasterize<QuadGrid, MicroQuadSampler>(grid, holder->attrs());
                break;
        }
    }
}

Renderer::Renderer(const Options& opts, const Mat4& camToScreen,
                   const VarList& outVars)
    : m_opts(opts),
    m_coc(),
    m_surfaces(),
    m_outVars(),
    m_sampStorage(),
    m_camToRas()
{
    sanitizeOptions(m_opts);
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

    if(opts.fstop != FLT_MAX)
    {
        m_coc.reset(new CircleOfConfusion(opts.fstop, opts.focalLength,
                                          opts.focalDistance, m_camToRas));
    }

    // Set up storage for samples.
    m_sampStorage.reset(new SampleStorage(m_outVars, m_opts));
    // Set up storage for split surfaces
    //
    // TODO: This doesn't take into account filter width expansion.
    Imath::Box2f storeBound(Vec2(0,0), Vec2(m_opts.xRes, m_opts.yRes));
    int nxbuckets = ceildiv(m_opts.xRes, m_opts.bucketSize);
    int nybuckets = ceildiv(m_opts.yRes, m_opts.bucketSize);
    m_surfaces.reset(new SplitStore(nxbuckets, nybuckets, storeBound));
}

// Trivial destructor.  Only defined here to prevent renderer implementation
// details leaking out of the interface (SurfaceQueue destructor is called
// implicitly)
Renderer::~Renderer() { }

void Renderer::add(const GeometryPtr& geom, Attributes& attrs)
{
    // TODO: Transform to camera space?
    GeomHolderPtr holder(new GeomHolder(geom, &attrs));
    push(holder);
}

void Renderer::add(GeometryKeys& deformingGeom, Attributes& attrs)
{
    GeomHolderPtr holder(new GeomHolder(deformingGeom, &attrs));
    push(holder);
}

// Render all surfaces and save resulting image.
void Renderer::render()
{
    // Splitting transform.  Allowing this to be different from the
    // projection matrix lets us examine a fixed set of grids
    // independently of the viewpoint.
    Mat4 splitTrans = m_camToRas;
    // Make sure that the z-component comes out as zero when splitting based on
    // projected size
    splitTrans[0][2] = 0;
    splitTrans[1][2] = 0;
    splitTrans[2][2] = 0;
    splitTrans[3][2] = 0;
//            Mat4().setScale(Vec3(0.5,-0.5,0))
//        * Mat4().setTranslation(Vec3(0.5,0.5,0))
//        * Mat4().setScale(Vec3(m_opts.xRes, m_opts.yRes, 1));

    TessellationContextImpl tessContext(*this);

    // Loop over all buckets
    for(int j = 0; j < m_surfaces->nyBuckets(); ++j)
    for(int i = 0; i < m_surfaces->nxBuckets(); ++i)
    {
        while(GeomHolderPtr g = m_surfaces->pop(i,j))
        {
            float polyLength = micropolyBlurWidth(g, m_coc.get());
            tessContext.tessellate(splitTrans, polyLength, g);
            g->releaseGeometry();
        }
    }
    saveImages("test");
}


/// TODO: Clean up & abstract the parts of motionRasterize so it can correctly
/// take any GridT, and so that it actually uses PolySamplerT.
struct OutVarInfo
{
    ConstFvecView src;
    int outIdx;

    OutVarInfo(const ConstFvecView& src, int outIdx)
        : src(src), outIdx(outIdx) {}
};


/// Dirty implementation of motion blur sampling.  Won't work unless GridT is
/// a quad grid!
template<typename GridT, typename PolySamplerT>
void Renderer::motionRasterize(GridHolder& holder)
{
    // Project grids into raster coordinates.
    holder.project(m_camToRas);

    // Determine index of depth output data, if any.
    int zOffset = -1;
    int zIdx = m_outVars.find(StdOutInd::z);
    if(zIdx != OutvarSet::npos)
        zOffset = m_outVars[zIdx].offset;

    bool motionBlur = holder.isDeforming();

    // Cache the variables which need to be interpolated into
    // fragment outputs.
    std::vector<OutVarInfo> outVarInfo;
    const GridT& mainGrid = static_cast<GridT&>(holder.grid());
    const GridStorage& mainStor = mainGrid.storage();
    const VarSet& gridVars = mainStor.varSet();
    for(int j = 0, jend = m_outVars.size(); j < jend; ++j)
    {
        // Simplistic linear search through grid variables for now.
        // This only happens once per grid, so maybe it's not too bad?
        for(int i = 0, iend = gridVars.size(); i < iend; ++i)
        {
            if(gridVars[i] == m_outVars[j])
            {
                outVarInfo.push_back(OutVarInfo(
                        mainStor.get(i), m_outVars[j].offset) );
                break;
            }
        }
    }

    int fragSize = m_sampStorage->fragmentSize();
    const float* defaultFrag = m_sampStorage->defaultFragment();

    // Interleaved sampling info
    Imath::V2i tileSize = m_sampStorage->m_tileSize;
    Vec2 tileBoundMult = Vec2(1.0)/Vec2(tileSize);
    Imath::V2i nTiles = Imath::V2i(m_sampStorage->m_xSampRes-1,
                        m_sampStorage->m_ySampRes-1) / tileSize
                        + Imath::V2i(1);
    const int sampsPerTile = tileSize.x*tileSize.y;
    // time/lens position of samples
    const SampleStorage::TimeLens* extraDims = &m_sampStorage->m_extraDims[0];

    // Info for motion interpolation
    int* intervals = 0;
    float* interpWeights = 0;
    if(motionBlur)
    {
        const GridKeys& gridKeys = holder.gridKeys();

        // Pre-compute interpolation info for all time indices
        intervals = ALLOCA(int, sampsPerTile);
        const int maxIntervalIdx = gridKeys.size()-2;
        interpWeights = ALLOCA(float, sampsPerTile);
        for(int i = 0, interval = 0; i < sampsPerTile; ++i)
        {
            // Search forward through grid time intervals to find the interval
            // which contains the i'th sample time.
            while(interval < maxIntervalIdx
                && gridKeys[interval+1].time < extraDims[i].time)
                ++interval;
            intervals[i] = interval;
            interpWeights[i] = (extraDims[i].time - gridKeys[interval].time) /
                        (gridKeys[interval+1].time - gridKeys[interval].time);
        }
    }

    // Helper objects for hit testing
    PointInQuad hitTest;
    InvBilin invBilin;

    // For each micropoly.
    for(int v = 0, nv = mainGrid.nv(); v < nv-1; ++v)
    for(int u = 0, nu = mainGrid.nu(); u < nu-1; ++u)
    {
        MicroQuadInd ind(nu*v + u,        nu*v + u+1,
                         nu*(v+1) + u+1,  nu*(v+1) + u);
        // For each possible sample time
        for(int itime = 0; itime < sampsPerTile; ++itime)
        {
            // Compute vertices of micropolygon
            Vec3 Pa, Pb, Pc, Pd;
            if(motionBlur)
            {
                int interval = intervals[itime];
                const GridKeys& gridKeys = holder.gridKeys();
                const GridT& grid1 = static_cast<GridT&>(*gridKeys[interval].value);
                const GridT& grid2 = static_cast<GridT&>(*gridKeys[interval+1].value);
                // Interpolate micropoly to the current time
                float interp = interpWeights[itime];
                ConstDataView<Vec3> P1 = grid1.storage().P();
                ConstDataView<Vec3> P2 = grid2.storage().P();
                Pa = lerp(P1[ind.a], P2[ind.a], interp);
                Pb = lerp(P1[ind.b], P2[ind.b], interp);
                Pc = lerp(P1[ind.c], P2[ind.c], interp);
                Pd = lerp(P1[ind.d], P2[ind.d], interp);
            }
            else
            {
                ConstDataView<Vec3> P = mainStor.P();
                Pa = P[ind.a];
                Pb = P[ind.b];
                Pc = P[ind.c];
                Pd = P[ind.d];
            }

            // Offset vertices with lens position for depth of field.
            if(m_coc)
            {
                Vec2 lensPos = extraDims[itime].lens;
                m_coc->lensShift(Pa, lensPos);
                m_coc->lensShift(Pb, lensPos);
                m_coc->lensShift(Pc, lensPos);
                m_coc->lensShift(Pd, lensPos);
            }
            hitTest.init(vec2_cast(Pa), vec2_cast(Pb),
                         vec2_cast(Pc), vec2_cast(Pd), (u + v) % 2);
            invBilin.init(vec2_cast(Pa), vec2_cast(Pb),
                          vec2_cast(Pd), vec2_cast(Pc));
            // Compute tight bound
            Box bound(Pa);
            bound.extendBy(Pb);
            bound.extendBy(Pc);
            bound.extendBy(Pd);

            // Iterate over samples at current time which come from tiles
            // which cross the bound.
            Imath::V2i bndMin = ifloor((vec2_cast(bound.min)*m_opts.superSamp
                                + Vec2(m_sampStorage->m_filtExpand))*tileBoundMult);
            Imath::V2i bndMax = ifloor((vec2_cast(bound.max)*m_opts.superSamp
                                + Vec2(m_sampStorage->m_filtExpand))*tileBoundMult);
            int startx = clamp(bndMin.x,   0, nTiles.x);
            int endx   = clamp(bndMax.x+1, 0, nTiles.x);
            int starty = clamp(bndMin.y,   0, nTiles.y);
            int endy   = clamp(bndMax.y+1, 0, nTiles.y);
            // For each tile in the bound
            for(int ty = starty; ty < endy; ++ty)
            for(int tx = startx; tx < endx; ++tx)
            {
                int tileInd = (ty*nTiles.x + tx);
                // Index of which sample in the tile is considered to be at
                // itime.
                int shuffIdx = m_sampStorage->m_tileShuffleIndices[
                                tileInd*sampsPerTile + itime ];
                if(shuffIdx < 0)
                    continue;
                Sample& samp = m_sampStorage->m_samples[shuffIdx];
                if(!hitTest(samp))
                    continue;
                Vec2 uv = invBilin(samp.p);
                float z = bilerp(Pa.z, Pb.z, Pd.z, Pc.z, uv);
                if(samp.z < z)
                    continue; // Ignore if hit is hidden
                samp.z = z;
                // Generate & store a fragment
                float* samples = &m_sampStorage->m_fragments[shuffIdx*fragSize];
                // Initialize fragment data with the default value.
                std::memcpy(samples, defaultFrag, fragSize*sizeof(float));
                // Store interpolated fragment data
                for(int j = 0, jend = outVarInfo.size(); j < jend; ++j)
                {
                    ConstFvecView in = outVarInfo[j].src;
                    const float* in0 = in[ind.a];
                    const float* in1 = in[ind.b];
                    const float* in2 = in[ind.d];
                    const float* in3 = in[ind.c];
                    float w0 = (1-uv.y)*(1-uv.x);
                    float w1 = (1-uv.y)*uv.x;
                    float w2 = uv.y*(1-uv.x);
                    float w3 = uv.y*uv.x;
                    float* out = &samples[outVarInfo[j].outIdx];
                    for(int i = 0, size = in.elSize(); i < size; ++i)
                        out[i] = w0*in0[i] + w1*in1[i] + w2*in2[i] + w3*in3[i];
                }
                if(zOffset >= 0)
                    samples[zOffset] = z;
            }
        }
    }
}


// Render a grid by rasterizing each micropolygon.
template<typename GridT, typename PolySamplerT>
//__attribute__((flatten))
void Renderer::rasterize(Grid& inGrid, const Attributes& attrs)
{
    GridT& grid = static_cast<GridT&>(inGrid);
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

        poly.initHitTest();
        poly.initInterpolator();

        // for each sample position in the bound
        for(SampleStorage::Iterator sampi = m_sampStorage->begin(bound);
            sampi.valid(); ++sampi)
        {
            Sample& samp = sampi.sample();
//           // Early out if definitely hidden
//           if(samp.z < bound.min.z)
//               continue;
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
            float* out = sampi.fragment();
            // Initialize fragment data with the default value.
            std::memcpy(out, defaultFrag, fragSize*sizeof(float));
            // Store interpolated fragment data
            poly.interpolate(out);
            if(zOffset >= 0)
                out[zOffset] = z;
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

        hitTest.init(vec2_cast(a), vec2_cast(b),
                     vec2_cast(c), vec2_cast(d), flipEnd);
        if(smoothShading)
        {
            invBilin.init(vec2_cast(a), vec2_cast(b),
                          vec2_cast(d), vec2_cast(c));
        }

        // for each sample position in the bound
        for(SampleStorage::Iterator sampi = m_sampStorage->begin(bound);
            sampi.valid(); ++sampi)
        {
            Sample& samp = sampi.sample();
//           // Early out if definitely hidden
//           if(samp.z < bound.min.z)
//               continue;
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
            sampi.fragment()[0] = z;
        }
    }
}

