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

#include "renderer.h"

#include <cstring>
#include <functional>

#include "attributes.h"
#include "bucketscheduler.h"
#include "displaymanager.h"
#include "filterprocessor.h"
#include "grid.h"
#include "gridstorage.h"
#include "memdebug.h"
#include "microquadsampler.h"
#include "refcount.h"
#include "sample.h"
#include "samplegen.h"
#include "splitstore.h"
#include "tessellation.h"

#include "stats.h"
#include "timer.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/// Tile of sample positions
class SampleTile
{
    public:
        /// Create empty sample tile.
        ///
        /// The tile is invalid until reset() has been called.
        SampleTile()
            : m_size(-1),
            m_samples(),
            m_samplesSetup(false),
            m_dofMbSetup(false),
            m_fragmentTile(0)
        { }

        /// Set up sample info for next region
        ///
        /// Note that SampleTile does hold a pointer to the fragments, but
        /// doesn't control the fragment tile lifetime.
        void reset(FragmentTile& fragTile)
        {
            m_fragmentTile = &fragTile;
            // Allocate samples if necessary, but defer initialization until
            // we're sure it's necessary.
            if(fragTile.size() != m_size)
            {
                m_size = fragTile.size();
                m_samples.reset(new Sample[prod(m_size)]);
            }
            m_samplesSetup = false;
            m_dofMbSetup = false;
        }

        /// Ensure sample positions are set up
        ///
        /// This initialization step is deferred rather than performed inside
        /// reset(), since some tiles may not have any geometry.  We'd like to
        /// avoid the computational expense of processing such tiles when they
        /// don't even have any samples present.
        void ensureSamplesSetup();

        /// Ensure depth of field/motion blur indices are setup
        ///
        /// This setup needs to be performed after every reset() if you want to
        /// do MB or DoF sampling.  As with ensureSamplesSetup(), the setup is
        /// deferred until necessary so that we can avoid the expense if
        /// possible.
        void ensureDofMbSetup(const DofMbTileSet& tileSet);

        /// Get range of DoF/MB tiles to iterate over for the given bound.
        void dofMbTileRange(const V2f& boundMin, const V2f& boundMax,
                            int& startx, int& endx, int& starty, int& endy)
        {
            V2i bndMin = ifloor((boundMin - m_tileBoundOffset)*m_tileBoundMult);
            V2i bndMax = ifloor((boundMax - m_tileBoundOffset)*m_tileBoundMult);
            startx = clamp(bndMin.x,   0, m_dofMbNumTiles.x);
            endx   = clamp(bndMax.x+1, 0, m_dofMbNumTiles.x);
            starty = clamp(bndMin.y,   0, m_dofMbNumTiles.y);
            endy   = clamp(bndMax.y+1, 0, m_dofMbNumTiles.y);
        }

        int dofMbTileIndex(int tx, int ty, int itime)
        {
            return m_dofMbIndices[(ty*m_dofMbNumTiles.x + tx)*m_sampsPerTile
                                  + itime];
        }

        /// Get sample position relative to (0,0) in upper-left of tile.
        Sample& sample(int x, int y)
        {
            return m_samples[m_size.x*y + x];
        }
        Sample& sample(int index)
        {
            return m_samples[index];
        }

        /// Get a fragment from the associated fragment tile.
        float* fragment(int x, int y)
        {
            return m_fragmentTile->fragment(x,y);
        }
        float* fragment(int index)
        {
            return m_fragmentTile->fragment(index);
        }

        /// Get size of tile in samples
        V2i size() const { return m_size; }

        /// Get position of top-left of tile in sraster coordinates.
        V2i sampleOffset() const { return m_fragmentTile->sampleOffset(); }

        /// Determine whether the bound is entirely occluded by the samples.
        bool occludes(const Box3f& bound, Timer& occlTime)
        {
            if(!m_samplesSetup)
                return false;
            TIME_SCOPE(occlTime);
            // transform bound into integer coords (inclusive end)
            V2i bndMin = ifloor(vec2_cast(bound.min)) - sampleOffset();
            V2i bndMax = ifloor(vec2_cast(bound.max)) - sampleOffset();
            // clamp geometry bound to extent of sample region
            int beginx = clamp(bndMin.x,   0, m_size.x);
            int endx   = clamp(bndMax.x+1, 0, m_size.x);
            int beginy = clamp(bndMin.y,   0, m_size.y);
            int endy   = clamp(bndMax.y+1, 0, m_size.y);
            // Iterate over all the pixels the bound crosses.  This is a
            // pretty braindead way of doing it, but simple things first!
            float zmin = bound.min.z;
            for(int iy = beginy; iy < endy; ++iy)
            for(int ix = beginx; ix < endx; ++ix)
            {
                // If geom bound is closer than the sample, it's not occluded.
                if(zmin < m_samples[m_size.x*iy + ix].z)
                    return false;
            }
            return true;
        }

    private:
        static const SpatialHash m_spatialHash;

        // Sample positions
        V2i m_size;          ///< Number of samples
        boost::scoped_array<Sample> m_samples; ///< Sample positions
        bool m_samplesSetup; ///< True if samples have been setup for bucket

        // Depth of field / Motion blur sample info
        std::vector<int> m_dofMbIndices;
        V2f m_tileBoundMult;
        V2f m_tileBoundOffset;
        int m_sampsPerTile;
        V2i m_dofMbNumTiles;
        bool m_dofMbSetup;

        // Fragments
        FragmentTile* m_fragmentTile; ///< Set of fragments for the bucket
};

const SpatialHash SampleTile::m_spatialHash(3);

void SampleTile::ensureSamplesSetup()
{
    if(!m_samplesSetup)
    {
        V2f offset = V2f(m_fragmentTile->sampleOffset()) + V2f(0.5f);
        for(int j = 0; j < m_size.y; ++j)
        for(int i = 0; i < m_size.x; ++i)
            m_samples[m_size.x*j + i] = Sample(V2f(i,j) + offset);
        m_samplesSetup = true;
    }
}

void SampleTile::ensureDofMbSetup(const DofMbTileSet& tileSet)
{
    if(!m_dofMbSetup)
    {
        // TODO: Need some more explanation here, in the event that this
        // sampling method actually survives.
        V2i dofMbSize = tileSet.tileSize();
        m_sampsPerTile = prod(dofMbSize);
        m_tileBoundMult = V2f(1.0)/dofMbSize;
        V2i sampStart = sampleOffset();
        V2i sampEnd = sampStart + m_size;
        V2i tileStart = sampStart/dofMbSize;
        V2i tileEnd   = ceildiv(sampEnd, dofMbSize);
        // Initialize all indices to invalid == -1.
        m_dofMbNumTiles = tileEnd - tileStart;
        m_dofMbIndices.assign(prod(m_dofMbNumTiles)*m_sampsPerTile, -1);
        for(int ty = tileStart.y; ty < tileEnd.y; ++ty)
        for(int tx = tileStart.x; tx < tileEnd.x; ++tx)
        {
            // Compute current tile using spatial hash function.
            // Get the tile from the tile set with the four corner colors
            // calculated using the spatial hash function.
            //
            // TODO: Check somehow that the tiles are being correctly pieced
            // together.
            const int* inTile = tileSet.getTile(
                    m_spatialHash(tx, ty),   m_spatialHash(tx+1, ty),
                    m_spatialHash(tx, ty+1), m_spatialHash(tx+1, ty+1));
            int outTileStart = (m_dofMbNumTiles.x*(ty - tileStart.y)
                              + (tx - tileStart.x)) * m_sampsPerTile;
            assert(outTileStart >= 0);
            assert(outTileStart + prod(dofMbSize) <= (int)m_dofMbIndices.size());
            int* outTile = &m_dofMbIndices[outTileStart];
            // Copy inverse mapping of input tile into output.  This means
            // that when we index outTile at index n, we get the index
            // into the sample position array which corresponds to the n'th
            // time/lens offset.
            V2i tPos = V2i(tx,ty);
            V2i start = max(dofMbSize*tPos, sampStart);
            V2i end   = min(dofMbSize*(tPos + V2i(1)), sampEnd);
            for(int j = start.y; j < end.y; ++j)
            for(int i = start.x; i < end.x; ++i)
            {
                int timeIdx = (j-dofMbSize.y*ty)*dofMbSize.x + i-dofMbSize.x*tx;
                int samplePosIdx = (j-sampStart.y)*m_size.x + (i-sampStart.x);
                outTile[inTile[timeIdx]] = samplePosIdx;
            }
        }
        m_tileBoundOffset = tileStart*dofMbSize;
        m_dofMbSetup = true;
    }
}

//------------------------------------------------------------------------------
/// Circle of confusion class for depth of field
class CircleOfConfusion
{
    private:
        float m_focalDistance;
        V2f  m_cocMult;
        float m_invFocalDist;

    public:
        CircleOfConfusion(float fstop, float focalLength, float focalDistance,
                          const M44f& camToRaster)
        {
            m_focalDistance = focalDistance;
            float mult = 0.5*focalLength/fstop * focalDistance*focalLength
                                               / (focalDistance - focalLength);
            // Get multiplier into raster units.
            m_cocMult = mult*V2f(fabs(camToRaster[0][0]), fabs(camToRaster[1][1]));
            m_invFocalDist = 1/focalDistance;
        }

        /// Shift the vertex P on the circle of confusion.
        ///
        /// P is updated to position it would have if viewed with a pinhole
        /// camera at the position lensPos.
        ///
        void lensShift(V3f& P, const V2f& lensPos) const
        {
            V2f v = lensPos*m_cocMult*std::fabs(1/P.z - m_invFocalDist);
            P.x -= v.x;
            P.y -= v.y;
        }

        /// Compute the minimum lensShift inside the interval [z1,z2]
        V2f minShiftForBound(float z1, float z2) const
        {
            // First check whether the bound spans the focal plane.
            if((z1 <= m_focalDistance && z2 >= m_focalDistance) ||
               (z1 >= m_focalDistance && z2 <= m_focalDistance))
                return V2f(0);
            // Otherwise, the minimum focal blur is achieved at one of the
            // z-extents of the bound.
            return m_cocMult*std::min(std::fabs(1/z1 - m_invFocalDist),
                                      std::fabs(1/z2 - m_invFocalDist));
        }

        /// Compute the maximum lensShift inside the interval [z1,z2]
        V2f maxShiftForBound(float z1, float z2) const
        {
            return m_cocMult*std::max(std::fabs(1/z1 - m_invFocalDist),
                                      std::fabs(1/z2 - m_invFocalDist));
        }
};


//------------------------------------------------------------------------------
/// Renderer statistics
///
/// TODO: Split this stats structure up into managable bits.
struct RenderStats
{
    /// Flag to allow stats to be disabled completely at compile time.
    static const bool useStats=true;
    /// Flag to indicate that expensive statistics should be collected.
    ///
    /// Statistics which have a measurable performance impact should only be
    /// collected if this flag is enabled.
    bool collectExpensiveStats;
    /// Stats verbosity level
    ///
    /// 0 = no stats
    /// 1 = simple things
    /// 2 = include potentially expensive stats
    int verbosity;

    // Geometry stats
    ResourceCounterStat<useStats> geometryInFlight;
    SimpleCounterStat<useStats>   geometryOccluded;
    // Grid stats
    ResourceCounterStat<useStats> gridsInFlight;
    SimpleCounterStat<useStats>   gridsOccluded;
    MinMaxMeanStat<float, useStats> averagePolyArea;

    // Sampling stats
    SimpleCounterStat<useStats>   samplesTested;
    SimpleCounterStat<useStats>   samplesHit;

    // Timers
    Timer frameTime;
    Timer occlTime;
    Timer splitDiceTime;
    Timer rasterizeTime;
    Timer filteringTime;
    Timer shadingTime;

    /// Merge the given stats into this one.
    void merge(RenderStats& other)
    {
        static Mutex mutex;
        LockGuard lk(mutex);
        // merge counters
        geometryOccluded.merge(other.geometryOccluded);
        gridsOccluded.merge(other.gridsOccluded);
        averagePolyArea.merge(other.averagePolyArea);
        samplesTested.merge(other.samplesTested);
        samplesHit.merge(other.samplesHit);
        // merge timers
        frameTime.merge(other.frameTime);
        occlTime.merge(other.occlTime);
        splitDiceTime.merge(other.splitDiceTime);
        rasterizeTime.merge(other.rasterizeTime);
        filteringTime.merge(other.filteringTime);
        shadingTime.merge(other.shadingTime);
    }

    /// Print the statistics to the given stream.
    void printStats(std::ostream& out);

    RenderStats(int verbosity)
        : collectExpensiveStats(verbosity >= 2),
        verbosity(verbosity),
        frameTime     (false, verbosity >= 1),
        occlTime      (false, verbosity >= 2),
        splitDiceTime (false, verbosity >= 2),
        rasterizeTime (false, verbosity >= 2),
        filteringTime (false, verbosity >= 2),
        shadingTime   (false, verbosity >= 2)
    { }
};

void RenderStats::printStats(std::ostream& out)
{
    // Output stats
    if(verbosity >= 1)
    {
        out << "geometry: allocated            : " << geometryInFlight << "\n"
            << "geometry: occlusion culled     : " << geometryOccluded << "\n"
            << "grids: allocated               : " << gridsInFlight    << "\n"
            << "grids: occlusion culled        : " << gridsOccluded    << "\n";
        if(verbosity >= 2)
            out << "micropolygons: area            : " << averagePolyArea  << "\n";
        out << "sampling: point in poly tests  : " << samplesTested;
        if(samplesTested.value() > 0)
            out << "  (" << 100.0*samplesHit.value()/samplesTested.value()
                << "% hit)";
        out << "\n";

        // Output timings
        double frame = frameTime();
        out << "\n"
            << boost::format("time: frame          :%7.3fs\n") % frame;

        if(verbosity >= 2)
        {
            double inPercent = 100/frame;
#           define FORMAT_TIME(description, timerName) boost::format(  \
                "time: %-14s :%7.3fs  (%4.1f%% of frame)\n")               \
                % description % timerName() % (inPercent*timerName())
            out << FORMAT_TIME("occlusion cull", occlTime)
                << FORMAT_TIME("split/dice", splitDiceTime)
                << FORMAT_TIME("sampling", rasterizeTime)
                << FORMAT_TIME("filtering", filteringTime)
                << FORMAT_TIME("shading", shadingTime);
#           undef FORMAT_TIME
        }
    }
}


namespace {
/// Fill an array with the default no-hit fragment sample values
void fillDefaultFrag(std::vector<float>& defaultFrag, const OutvarSet& outVars)
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
}

/// Adjust desired micropolygon width based on focal or motion blurring.
float Renderer::micropolyBlurWidth(const GeomHolderPtr& holder,
                                   const CircleOfConfusion* coc) const
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
        V2f cocScale = coc->minShiftForBound(holder->rasterBound().min.z,
                                             holder->rasterBound().max.z);
        // The CoC shift is in the sraster coordinate system, so divide by
        // superSamp to get it into pixel-based raster coords.  pixel-based
        // raster is the relevant coordinates which determine the size of
        // details which will be visible after filtering.
        cocScale /= V2f(m_opts->superSamp);
        float minCoC = std::min(cocScale.x, cocScale.y);
        const float lengthRatio = 0.16;
        polyLength *= std::max(1.0f, lengthRatio*attrs.focusFactor*minCoC);
    }
    return polyLength;
}

namespace {
template<typename T>
void clampOptBelow(T& val, const char* name, const T& minVal,
                   ErrorHandler& errorHandler, const char* reason = 0)
{
    if(val < minVal)
    {
        errorHandler.error(ErrorCode::BadOption,
                           "Option %s = %s is to small%s%s%s.  Clamping to %s.",
                           name, val, reason ? " (" : "", reason ? reason : "",
                           reason ? ")" : "", minVal);
        val = minVal;
    }
}
} // anon namespace

void Renderer::sanitizeOptions(Options& opts)
{
#define CLAMP_OPT_BELOW(optname, minVal) \
    clampOptBelow(opts.optname, #optname, minVal, m_errorHandler)
    CLAMP_OPT_BELOW(eyeSplits, 0);
    CLAMP_OPT_BELOW(gridSize, 1);
    CLAMP_OPT_BELOW(clipNear, FLT_EPSILON);
    CLAMP_OPT_BELOW(clipFar, opts.clipNear);
    CLAMP_OPT_BELOW(resolution.x, 1);
    CLAMP_OPT_BELOW(resolution.y, 1);
    CLAMP_OPT_BELOW(bucketSize.x, 1);
    CLAMP_OPT_BELOW(bucketSize.y, 1);
    // Filter width can't be larger than bucket size due to implementation
    // choices.
    V2i minBucketSize = iceil(opts.pixelFilter->width());
    clampOptBelow(opts.bucketSize.x, "bucketsize.x", minBucketSize.x,
                  m_errorHandler, "must be greater than filter size");
    clampOptBelow(opts.bucketSize.y, "bucketsize.y", minBucketSize.y,
                  m_errorHandler, "must be greater than filter size");
    CLAMP_OPT_BELOW(superSamp.x, 1);
    CLAMP_OPT_BELOW(superSamp.y, 1);
    CLAMP_OPT_BELOW(interleaveWidth, 4);
    CLAMP_OPT_BELOW(shutterMax, opts.shutterMin);
}

/// Determine whether the given geometry may be visibility culled.
///
/// This function also transforms the geometry bound into sraster space.
///
/// TODO: Clean this function up - it does several unrelated things in a
/// somewhat confusing manner.
bool Renderer::rasterCull(GeomHolder& holder, const Box2i* parentBucketBound)
{
    // Get bound.  This is actually in camera space to begin with, and we
    // transform it to raster space here.
    Box3f bound = holder.rasterBound();
    // Expand bound for displacement
    // TODO: Support arbitrary coordinate systems for the displacement bound
    bound.min -= V3f(holder.attrs().displacementBound);
    bound.max += V3f(holder.attrs().displacementBound);
    // Cull if outside near/far clipping range
    if(bound.max.z < m_opts->clipNear || bound.min.z > m_opts->clipFar)
        return true;
    if(bound.min.z < FLT_EPSILON)
    {
        // Special case for geometry which spans the epsilon plane.  This is a
        // problem because it's hard to compute a useful raster space bound
        // from the camera space bounding box because of the projective divide.
        // In fact, a piece of geometry spanning the epsilon plane formally
        // projects to a half-infinite or infinite bounding "box".  There's two
        // measures taken to protect against this:
        if(holder.splitCount() > m_opts->eyeSplits)
        {
            // Measure 1:  We have a limit on the number of times an object
            // crossing the epsilon plane can be split to avoid infinite
            // recursion.  Such split events are known as "eye splits", and if
            // the limit is reached, we discard the object.
            m_errorHandler.warning(ErrorCode::MaxEyeSplits,
                            "Max eye splits encountered; geometry discarded");
            return true;
        }
        else
        {
            // Measure 2: Any object with an offending bounding box has the
            // bound adjusted so that it spans all buckets.  This isn't the
            // best we can do for all objects, since some bounds are only
            // half-infinite.  However, getting the special cases right might
            // be tricky, so we do the simple thing for now.
            bound.max.x = m_samplingArea.max.x;
            bound.max.y = m_samplingArea.max.y;
            bound.min.x = m_samplingArea.min.x;
            bound.min.y = m_samplingArea.min.y;
        }
    }
    else
    {
        // Transform bound to raster space.
        float minz = bound.min.z;
        float maxz = bound.max.z;
        bound = transformBound(bound, m_camToSRaster);
        bound.min.z = minz;
        bound.max.z = maxz;
        if(m_coc)
        {
            // Expand bound for depth of field.
            V2f maxLensShift = m_coc->maxShiftForBound(minz, maxz);
            bound.min.x -= maxLensShift.x;
            bound.min.y -= maxLensShift.y;
            bound.max.x += maxLensShift.x;
            bound.max.y += maxLensShift.y;
        }
        // Cull if outside xy extent of image
        if(bound.max.x < m_samplingArea.min.x ||
           bound.min.x > m_samplingArea.max.x ||
           bound.max.y < m_samplingArea.min.y ||
           bound.min.y > m_samplingArea.max.y)
            return true;
    }
    // Set initial reference count.
    V2i begin, end;
    m_surfaces->bucketRangeForBound(bound, begin.x, end.x, begin.y, end.y);
    if(parentBucketBound)
    {
        // If the geometry is the child of a split, we need to make sure its
        // bucket bound is strictly contained within the parent geometry
        // bound.  Sometimes this may not be the case due to floating point
        // precision errors.
        begin = max(begin, parentBucketBound->min);
        end = min(end, parentBucketBound->max);
    }
    holder.setRasterBound(bound);
    holder.initBucketRefs(begin, end);
    return false;
}

bool Renderer::rasterCull(GridHolder& gridh, const Box2i& parentBucketBound)
{
    Box3f bound = gridh.tightRasterBound();
    // Cull if outside clipping planes
    if(bound.max.z < m_opts->clipNear || bound.min.z > m_opts->clipFar)
        return true;
    if(m_coc)
    {
        // Expand bound for depth of field.
        V2f maxLensShift = m_coc->maxShiftForBound(bound.min.z, bound.max.z);
        bound.min.x -= maxLensShift.x;
        bound.min.y -= maxLensShift.y;
        bound.max.x += maxLensShift.x;
        bound.max.y += maxLensShift.y;
    }
    // Cull if outside xy extent of image
    if(bound.max.x < m_samplingArea.min.x ||
       bound.min.x > m_samplingArea.max.x ||
       bound.max.y < m_samplingArea.min.y ||
       bound.min.y > m_samplingArea.max.y)
        return true;
    // Set initial reference count.
    V2i begin, end;
    m_surfaces->bucketRangeForBound(bound, begin.x, end.x, begin.y, end.y);
    // We need to make sure the grid bound is strictly contained within the
    // parent geometry bound.  Sometimes this may not be the case due to
    // floating point errors (even if the geometry is coded correctly!).
    begin = max(begin, parentBucketBound.min);
    end = min(end, parentBucketBound.max);
    gridh.setRasterBound(bound);
    gridh.initBucketRefs(begin, end);
    return false;
}

Renderer::Renderer(const OptionsPtr& opts, const M44f& camToScreen,
                   const M44f& camToWorld, const DisplayList& displays,
                   ErrorHandler& errorHandler)
    : m_opts(opts),
    m_coc(),
    m_surfaces(),
    m_outVars(),
    m_camToSRaster(),
    m_camToWorld(camToWorld),
    m_dofMbTileSet(0),
    m_errorHandler(errorHandler)
{
    sanitizeOptions(*m_opts);
    // Set up output variables.
    std::vector<OutvarSpec> outVarsInit;
    for(int i = 0; i < displays.size(); ++i)
        outVarsInit.push_back(OutvarSpec(displays[i].outputVar, 0));
    std::sort(outVarsInit.begin(), outVarsInit.end());
    // Generate the output offsets after sorting, so that the order of
    // outVars is the same as the order in the output image.  This isn't
    // strictly necessary, but in-order iteration during sampling seems
    // like a good idea.
    int offset = 0;
    for(int i = 0, iend = outVarsInit.size(); i < iend; ++i)
    {
        outVarsInit[i].offset = offset;
        offset += outVarsInit[i].scalarSize();
    }
    m_outVars.assign(outVarsInit.begin(), outVarsInit.end());

    fillDefaultFrag(m_defaultFrag, m_outVars);

    // Cache the pixel filter.
    m_pixelFilter.reset(new CachedFilter(*m_opts->pixelFilter,
                                         m_opts->superSamp));

    // Set up display manager
    m_displayManager.reset(new DisplayManager(m_opts->resolution,
                                              m_opts->bucketSize,
                                              m_outVars, displays));

    V2i nbuckets(ceildiv(m_opts->resolution.x, m_opts->bucketSize.x) + 1,
                 ceildiv(m_opts->resolution.y, m_opts->bucketSize.y) + 1);

    // Set up filtering object
    Box2i outTileRange(V2i(0), nbuckets - V2i(1));
    m_filterProcessor.reset(
            new FilterProcessor(*m_displayManager, outTileRange,
                                *m_pixelFilter, m_opts->superSamp) );

    // Area to sample, in sraster coords.
    m_samplingArea = Box2f(V2f(-m_pixelFilter->offset()),
                                  V2f(m_opts->resolution*m_opts->superSamp +
                                       m_pixelFilter->offset()));

    V2i sampTileSize = m_opts->superSamp*m_opts->bucketSize;
    V2f sampTileOffset(sampTileSize/2);
    Box2f bucketArea(-sampTileOffset, V2f(nbuckets * sampTileSize
                                                  - sampTileSize/2));
    // Set up storage for split surfaces
    m_surfaces.reset(new SplitStore(nbuckets.x, nbuckets.y,
                                    bucketArea));

    // Set up camera -> sample raster matrix.
    //
    // "sraster" is a special raster coordinate system where the unit of length
    // is the distance between adjacent *samples*.  This is different from the
    // usual raster coordinates where the unit of length is the distance
    // between *pixels* after filtering.  Using sraster rather than raster
    // simplifies the sampling code.
    //
    // The origin of the sraster coordinate system is the top left of the
    // filtering region for the top left pixel.
    m_camToSRaster = camToScreen
        * M44f().setScale(V3f(0.5,-0.5,0))
        * M44f().setTranslation(V3f(0.5,0.5,0))
        * M44f().setScale(V3f(m_opts->resolution.x*m_opts->superSamp.x,
                               m_opts->resolution.y*m_opts->superSamp.y, 1));

    if(m_opts->fstop != FLT_MAX)
    {
        m_coc.reset(new CircleOfConfusion(m_opts->fstop, m_opts->focalLength,
                                          m_opts->focalDistance,
                                          m_camToSRaster));
    }

}

Renderer::~Renderer()
{
}

/// Push geometry into the render queue
void Renderer::add(const GeomHolderPtr& holder)
{
    if(rasterCull(*holder, 0))
        return;
    m_surfaces->insert(holder);
}

void Renderer::add(const GeometryPtr& geom, const ConstAttributesPtr& attrs)
{
    add(new GeomHolder(geom, attrs));
}

void Renderer::add(GeometryKeys& deformingGeom,
                   const ConstAttributesPtr& attrs)
{
    add(new GeomHolder(deformingGeom, attrs));
}


// Render all surfaces and save resulting image.
void Renderer::render()
{
    // Statistics for all threads
    RenderStats frameStats(m_opts->statsVerbosity);
    frameStats.averagePolyArea.setScale(1.0/prod(m_opts->superSamp));

    TIME_SCOPE(frameStats.frameTime);
    //MemoryLog memLog;  // FIXME

    BucketSchedulerShared scheduler(V2i(m_surfaces->nxBuckets(),
                                        m_surfaces->nyBuckets()));
#   ifdef AQSIS_USE_THREADS
    int nthreads = m_opts->nthreads;
    int ncpus = boost::thread::hardware_concurrency();
    if(nthreads <= 0)
        nthreads = ncpus;
    if(nthreads > 1)
    {
        boost::thread_group threads;
        for(int i = 0; i < nthreads; ++i)
        {
            boost::thread* thread = new boost::thread(
                boost::mem_fn(&Renderer::renderBuckets),
                this, boost::ref(scheduler),
                boost::ref(frameStats));
            if(nthreads <= ncpus)
            {
                // Fill up the machine from the last core first (core 0 seems
                // to sometimes be used preferentially for OS processes... not
                // sure how common this is?)
                setThreadAffinity(*thread, ncpus-1-i);
            }
            threads.add_thread(thread);
        }
        threads.join_all();
    }
    else
#endif // AQSIS_USE_THREADS
    {
        renderBuckets(scheduler, frameStats);
    }

    // Format stats and send to message log.
    if(frameStats.verbosity > 0)
    {
        std::ostringstream statsStr;
        frameStats.printStats(statsStr);
        m_errorHandler.message(0, "%s", statsStr.str());
    }
}


void Renderer::renderBuckets(BucketSchedulerShared& schedulerShared,
                             RenderStats& frameStats)
{
    BucketScheduler bucketScheduler(schedulerShared);
    RenderStats stats(frameStats.verbosity);
    stats.averagePolyArea.setScale(1.0/prod(m_opts->superSamp));
    // Coordinate system for tessellation resolution calculation.
    M44f tessCoords = m_camToSRaster
        * M44f().setScale(V3f(1.0/m_opts->superSamp.x,
                               1.0/m_opts->superSamp.y, 1));
    // Make sure that the z-component is ignored when tessellating based on the
    // 2D projected object size:
    tessCoords[0][2] = 0;
    tessCoords[1][2] = 0;
    tessCoords[2][2] = 0;
    tessCoords[3][2] = 0;

    V2i tileSize(m_opts->bucketSize*m_opts->superSamp);

    // Per-thread data structures:
    TessellationContextImpl tessContext(*this, frameStats.geometryInFlight,
                                        frameStats.gridsInFlight);
    GeometryQueue queue;
    SampleTile samples;

    // Loop over all buckets
    V2i bucketPos;
    while(bucketScheduler.nextBucket(bucketPos))
    {
        m_surfaces->enqueueGeometry(queue, bucketPos);
        // TODO: Does this work when tileSize is not a multiple of 2?
        V2i sampleOffset = bucketPos*tileSize - tileSize/2;
        // Create new tile for fragment storage
        FragmentTilePtr fragments =
            new FragmentTile(tileSize, sampleOffset,
                             cbegin(m_defaultFrag), m_defaultFrag.size());
        samples.reset(*fragments);

        // Process all surfaces and grids in the bucket.
        while(GeomHolder* geomh = queue.pop())
        {
            if(!geomh->hasChildren())
            {
                if(samples.occludes(geomh->rasterBound(), stats.occlTime))
                {
                    // If the geometry *is* occluded in this bucket, try to set
                    // the occlusion flag.  This will fail if the geometry has
                    // been split by another thread in the meantime.  In that
                    // case, we need to process the children to properly
                    // decrement their bucket reference counts, so we fall
                    // through to the next bit of code.
                    if(geomh->setOccludedInBucket(bucketPos))
                        continue;
                }
                else
                {
                    // If not occluded and not split yet, do the split/dice:
                    TIME_SCOPE(stats.splitDiceTime);
                    // Scale dicing coordinates to account for shading rate.
                    M44f scaledTessCoords = tessCoords * M44f().setScale(
                            1/micropolyBlurWidth(geomh, m_coc.get()));
                    // Note that invoking the tessellator causes surfaces and
                    // grids to be push()ed back to the renderer behind the
                    // scenes.
                    tessContext.tessellate(scaledTessCoords, geomh);
                }
            }
            // If we get here, the surface has children.
            if(GridHolder* gridh = geomh->childGrid().get())
            {
                // It has a grid; render that.
                if(queue.boundIntersects(gridh->bucketBound()))
                {
                    if(!samples.occludes(gridh->rasterBound(), stats.occlTime))
                        rasterize(samples, *gridh, stats);
                    // Release the grid if this was the last bucket it
                    // touches.
                    if(gridh->releaseBucketRef())
                    {
                        --frameStats.gridsInFlight;
                        if(!gridh->rasterized())
                            ++stats.gridsOccluded;
                        geomh->childGrid().reset();
                    }
                }
            }
            else
            {
                // Else it has surface children - push them back into the
                // queue.
                std::vector<GeomHolderPtr>& childGeoms = geomh->childGeoms();
                for(int i = 0, iend = childGeoms.size(); i < iend; ++i)
                    queue.push(childGeoms[i]);
            }
        }
        queue.releaseBucket(frameStats.geometryInFlight,
                            stats.geometryOccluded);
        // Filter the tile
        TIME_SCOPE(stats.filteringTime);
        m_filterProcessor->insert(bucketPos, fragments);
    }
    frameStats.merge(stats);
}


/// Rasterizer driver function for grids.
///
/// Determines which rasterizer function to use, depending on the type of grid
/// and current options.
void Renderer::rasterize(SampleTile& tile, GridHolder& holder,
                         RenderStats& stats)
{
    TIME_SCOPE(stats.rasterizeTime);
    tile.ensureSamplesSetup();
    holder.setRasterized();
    if(holder.isDeforming() || m_opts->fstop != FLT_MAX)
    {
        {
            TIME_SCOPE(stats.shadingTime);
            LockGuard lk(m_dofMbTileInit);
            // Initialize the tile set of DoF/MB indices if it hasn't been
            // initialized yet.  After initialization it's const.
            //
            // TODO: Only do this once if using multiple frames.
            if(!m_dofMbTileSet)
            {
                // Compute quality of time sample stratification relative to
                // lens sample stratification, depending on whether we have
                // motion blur, depth of field, or both in the scene.
                bool hasMotion = m_opts->shutterMin < m_opts->shutterMax;
                bool hasDof = m_coc;
                float timeStratQuality = 0;
                if(hasMotion && hasDof)
                    timeStratQuality = 0.5;
                else if(hasMotion)
                    timeStratQuality = 1;
                else if(hasDof)
                    timeStratQuality = 0;
                m_dofMbTileSet = &DofMbTileSet::create(m_opts->interleaveWidth,
                                                       timeStratQuality,
                                                       m_opts->shutterMin,
                                                       m_opts->shutterMax);
            }
        }
        tile.ensureDofMbSetup(*m_dofMbTileSet);
        // Sample with motion blur or depth of field
        switch(holder.grid().type())
        {
            case GridType_Quad:
                mbdofRasterize<QuadGrid, MicroQuadSampler>(tile, holder, stats);
                break;
        }
    }
    else
    {
        // No motion blur or depth of field
        switch(holder.grid().type())
        {
            case GridType_Quad:
                staticRasterize<QuadGrid, MicroQuadSampler>(tile, holder, stats);
                break;
        }
    }
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

/// Dirty implementation of motion blur / depth of field sampling.  Won't work
/// unless GridT is a quad grid!
template<typename GridT, typename PolySamplerT>
void Renderer::mbdofRasterize(SampleTile& tile, const GridHolder& holder,
                              RenderStats& stats)
{
    // Determine index of depth output data, if any.
    int zOffset = -1;
    int zIdx = m_outVars.find(StdOutInd::z);
    if(zIdx != OutvarSet::npos)
        zOffset = m_outVars[zIdx].offset;

    bool motionBlur = holder.isDeforming();

    V2i tileSize = tile.size();
    V2f bucketMin = V2f(tile.sampleOffset());
    V2f bucketMax = V2f(tile.sampleOffset() + tileSize);

    // Cache the variables which need to be interpolated into
    // fragment outputs.
    std::vector<OutVarInfo> outVarInfo;
    const GridT& mainGrid = static_cast<const GridT&>(holder.grid());
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

    int fragSize = m_defaultFrag.size();
    const float* defaultFrag = cbegin(m_defaultFrag);

    // time/lens position of samples
    const std::vector<TimeLens>& timeLens = m_dofMbTileSet->timeLensPositions();
    const int nTimeLens = timeLens.size();

    // Helper objects for hit testing
    PointInQuad hitTest;
    InvBilin invBilin;

    // Motion interpolation information.  Not used if we've only got depth of
    // field.
    const GridKeys& gridKeys = holder.gridKeys();
    const int maxIntervalIdx = gridKeys.size()-2;

    // For each possible sample time
    for(int itime = 0; itime < nTimeLens; ++itime)
    {
        // First, compute the grid bound at the current time.
        Box3f gbound;
        int interval = 0;
        float interpWeight = 0;
        if(motionBlur)
        {
            // time of current sample
            float time = timeLens[itime].time;
            // If the current sample time is outside of the time interval
            // defined by the grid keys, the grid is invisible for these
            // samples and we can cull it.
            if(time < gridKeys.front().time || time > gridKeys.back().time)
                continue;
            // Search forward through the grid time intervals to find the
            // interval which contains the itime'th sample time.
            while(interval < maxIntervalIdx &&
                  gridKeys[interval+1].time < time)
                ++interval;
            interpWeight = (timeLens[itime].time - gridKeys[interval].time) /
                        (gridKeys[interval+1].time - gridKeys[interval].time);
            // Motion interpolation for grid bound.
            gbound = lerp(gridKeys[interval].bound, gridKeys[interval+1].bound,
                          interpWeight);
        }
        else
           gbound = holder.tightRasterBound();
        // Now, dispalace the grid bound by the lens offset if necessary.
        V2f maxLensShift(0);
        V2f minLensShift(0);
        V2f lensPos(0);
        if(m_coc)
        {
            lensPos = timeLens[itime].lens;
            // Max distance a micropoly inside the bound can move.
            maxLensShift = lensPos*m_coc->maxShiftForBound(gbound.min.z,
                                                           gbound.max.z);
            // Min distance a micropoly inside the bound can move.
            minLensShift = lensPos*m_coc->minShiftForBound(gbound.min.z,
                                                           gbound.max.z);
            if(lensPos.x > 0)
                std::swap(maxLensShift.x, minLensShift.x);
            if(lensPos.y > 0)
                std::swap(maxLensShift.y, minLensShift.y);
            gbound.min.x -= minLensShift.x;
            gbound.min.y -= minLensShift.y;
            gbound.max.x -= maxLensShift.x;
            gbound.max.y -= maxLensShift.y;
        }
        // We now have a bound for the grid at the current time and lens
        // offset.  We test it against the bucket bound to reject the entire
        // grid where possible.
        if(gbound.max.x <  bucketMin.x || gbound.max.y <  bucketMin.y ||
           gbound.min.x >= bucketMax.x || gbound.min.y >= bucketMax.y)
            continue;
        // If we get here, we need to sample each micropolygon.
        for(int v = 0, nv = mainGrid.nv(); v < nv-1; ++v)
        for(int u = 0, nu = mainGrid.nu(); u < nu-1; ++u)
        {
            // First, get a quick guess at the micropoly bound for the current
            // lens/time position.  This allows us to quickly determine whether
            // the polygon could be visible in the current bucket at the
            // current time/lens position; if not we cull it.  The quick bound
            // may be slightly larger than the exact bound, the important thing
            // here is speed.
            Box3f quickBnd;
            int bndIndex = (nu-1)*v + u;
            if(motionBlur)
            {
                quickBnd = lerp(gridKeys[interval].cachedBounds[bndIndex],
                                gridKeys[interval+1].cachedBounds[bndIndex],
                                interpWeight);
            }
            else
                quickBnd = holder.cachedBounds()[bndIndex];
            if(m_coc)
            {
                quickBnd.min.x -= minLensShift.x;
                quickBnd.min.y -= minLensShift.y;
                quickBnd.max.x -= maxLensShift.x;
                quickBnd.max.y -= maxLensShift.y;
            }
            if(quickBnd.max.x <  bucketMin.x || quickBnd.max.y <  bucketMin.y ||
               quickBnd.min.x >= bucketMax.x || quickBnd.min.y >= bucketMax.y)
                continue;
            // If the micropolygon is probably visible according to the quick
            // bound, we need to compute the exact position of its vertices.
            MicroQuadInd ind(nu*v + u,        nu*v + u+1,
                             nu*(v+1) + u+1,  nu*(v+1) + u);
            // Compute vertices of micropolygon
            V3f Pa, Pb, Pc, Pd;
            if(motionBlur)
            {
                // Interpolate micropoly to the current time
                // TODO: Abstract this out of the function and into GridT
                // somehow.
                const GridT& grid1 = static_cast<GridT&>(*gridKeys[interval].grid);
                const GridT& grid2 = static_cast<GridT&>(*gridKeys[interval+1].grid);
                ConstDataView<V3f> P1 = grid1.storage().P();
                ConstDataView<V3f> P2 = grid2.storage().P();
                Pa = lerp(P1[ind.a], P2[ind.a], interpWeight);
                Pb = lerp(P1[ind.b], P2[ind.b], interpWeight);
                Pc = lerp(P1[ind.c], P2[ind.c], interpWeight);
                Pd = lerp(P1[ind.d], P2[ind.d], interpWeight);
            }
            else
            {
                ConstDataView<V3f> P = mainStor.P();
                Pa = P[ind.a];
                Pb = P[ind.b];
                Pc = P[ind.c];
                Pd = P[ind.d];
            }
            if(m_coc)
            {
                // Offset vertices with lens position for depth of field.
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
            Box3f bound(Pa);
            bound.extendBy(Pb);
            bound.extendBy(Pc);
            bound.extendBy(Pd);

            if(stats.collectExpensiveStats)
                stats.averagePolyArea += 0.5*std::abs(vec2_cast(Pa-Pc) %
                                                      vec2_cast(Pb-Pd));

            // Iterate over samples at current time which come from tiles
            // which cross the bound.

            int startx, endx, starty, endy;
            tile.dofMbTileRange(vec2_cast(bound.min), vec2_cast(bound.max),
                                startx, endx, starty, endy);
            // For each tile in the micropolygon bound
            for(int ty = starty; ty < endy; ++ty)
            for(int tx = startx; tx < endx; ++tx)
            {
                // Index of the sample inside the current tile at itime.
                int sampIndex = tile.dofMbTileIndex(tx, ty, itime);
                if(sampIndex < 0)
                    continue;
                Sample& samp = tile.sample(sampIndex);
                ++stats.samplesTested;
                if(!hitTest(samp.p))
                    continue;
                ++stats.samplesHit;
                V2f uv = invBilin(samp.p);
                float z = bilerp(Pa.z, Pb.z, Pd.z, Pc.z, uv);
                if(samp.z < z)
                    continue; // Ignore if hit is hidden
                samp.z = z;
                // Generate & store a fragment
                float* fragment = tile.fragment(sampIndex);
                // Initialize fragment data with the default value.
                for(int i = 0; i < fragSize; ++i)
                    fragment[i] = defaultFrag[i];
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
                    float* out = &fragment[outVarInfo[j].outIdx];
                    for(int i = 0, size = in.elSize(); i < size; ++i)
                        out[i] = w0*in0[i] + w1*in1[i] + w2*in2[i] + w3*in3[i];
                }
                if(zOffset >= 0)
                    fragment[zOffset] = z;
            }
        }
    }
}


/// Rasterize a non-moving grid without depth of field.
template<typename GridT, typename PolySamplerT>
//__attribute__((flatten))
void Renderer::staticRasterize(SampleTile& tile, const GridHolder& holder,
                               RenderStats& stats)
{
    const GridT grid = static_cast<const GridT&>(holder.grid());
    // Determine index of depth output data, if any.
    int zOffset = -1;
    int zIdx = m_outVars.find(StdOutInd::z);
    if(zIdx != OutvarSet::npos)
        zOffset = m_outVars[zIdx].offset;
    int fragSize = m_defaultFrag.size();
    const float* defaultFrag = cbegin(m_defaultFrag);

    // TODO: Rename SampleTile to BucketSamples ?
    V2i tileSize = tile.size();

    V2f bucketMin = V2f(tile.sampleOffset());
    V2f bucketMax = V2f(tile.sampleOffset() + tileSize);

    // Construct a sampler for the polygons in the grid
    PolySamplerT poly(grid, holder, m_outVars);
    // iterate over all micropolys in the grid & render each one.
    for(;poly.valid(); poly.next())
    {
        const Box3f& bound = poly.bound();
        // Go to next micropoly if current is entirely outside the bucket.
        if(bound.max.x <  bucketMin.x || bound.max.y <  bucketMin.y ||
           bound.min.x >= bucketMax.x || bound.min.y >= bucketMax.y)
            continue;

        if(stats.collectExpensiveStats)
            stats.averagePolyArea += poly.area();

        poly.initHitTest();
        poly.initInterpolator();
        // Compute subsample coordinates on the current sample tile.
        V2i bndMin = ifloor(vec2_cast(bound.min)) - tile.sampleOffset();
        V2i bndMax = ifloor(vec2_cast(bound.max)) - tile.sampleOffset();
        int beginx = clamp(bndMin.x,   0, tileSize.x);
        int endx   = clamp(bndMax.x+1, 0, tileSize.x);
        int beginy = clamp(bndMin.y,   0, tileSize.y);
        int endy   = clamp(bndMax.y+1, 0, tileSize.y);
        // Iterate over each sample in the bound
        for(int sy = beginy; sy < endy; ++sy)
        for(int sx = beginx; sx < endx; ++sx)
        {
            Sample& samp = tile.sample(sx,sy);
            // Test whether sample hits the micropoly
            ++stats.samplesTested;
            if(!poly.contains(samp.p))
                continue;
            ++stats.samplesHit;
            // Determine hit depth
            poly.interpolateAt(samp.p);
            float z = poly.interpolateZ();
            if(samp.z < z)
                continue; // Ignore if hit is hidden
            samp.z = z;
            // Generate & store a fragment
            float* out = tile.fragment(sx,sy);
            // Initialize fragment data with the default value.
            for(int i = 0; i < fragSize; ++i)
                out[i] = defaultFrag[i];
            // Store interpolated fragment data
            poly.interpolate(out);
            if(zOffset >= 0)
                out[zOffset] = z;
        }
    }
}

} // namespace Aqsis
