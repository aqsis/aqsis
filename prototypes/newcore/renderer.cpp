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
#include "samplestorage.h"
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
        }

        /// Ensure sample positions are set up
        ///
        /// This initialization step is deferred rather than performed inside
        /// reset(), since some tiles may not have any geometry.  We'd like to
        /// avoid the computational expense of processing such tiles when they
        /// don't even have any samples present.
        void ensureSampleInit()
        {
            if(!m_samplesSetup)
            {
                // Initialize sample positions
                Vec2 offset = Vec2(m_fragmentTile->sampleOffset()) + Vec2(0.5f);
                for(int j = 0; j < m_size.y; ++j)
                for(int i = 0; i < m_size.x; ++i)
                    m_samples[m_size.x*j + i] = Sample(Vec2(i,j) + offset);
                m_samplesSetup = true;
            }
        }

        /// Get sample position relative to (0,0) in upper-left of tile.
        Sample& sample(int x, int y)
        {
            return m_samples[m_size.x*y + x];
        }

        /// Get a fragment from the associated fragment tile.
        float* fragment(int x, int y)
        {
            return m_fragmentTile->fragment(x,y);
        }

        /// Get size of tile in samples
        V2i size() const { return m_size; }

        /// Get position of top-left of tile in sraster coordinates.
        V2i sampleOffset() const { return m_fragmentTile->sampleOffset(); }

        /// Determine whether the bound is entirely occluded by the samples.
        bool occludes(const Box& bound, Timer& occlTime)
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
        V2i m_size;
        boost::scoped_array<Sample> m_samples;
        bool m_samplesSetup;
        FragmentTile* m_fragmentTile;
};


//------------------------------------------------------------------------------
/// Circle of confusion class for depth of field
class CircleOfConfusion
{
    private:
        float m_focalDistance;
        Vec2  m_cocMult;
        float m_invFocalDist;

    public:
        CircleOfConfusion(float fstop, float focalLength, float focalDistance,
                          const Mat4& camToRaster)
        {
            m_focalDistance = focalDistance;
            float mult = 0.5*focalLength/fstop * focalDistance*focalLength
                                               / (focalDistance - focalLength);
            // Get multiplier into raster units.
            m_cocMult = mult*Vec2(fabs(camToRaster[0][0]), fabs(camToRaster[1][1]));
            m_invFocalDist = 1/focalDistance;
        }

        /// Shift the vertex P on the circle of confusion.
        ///
        /// P is updated to position it would have if viewed with a pinhole
        /// camera at the position lensPos.
        ///
        void lensShift(Vec3& P, const Vec2& lensPos) const
        {
            Vec2 v = lensPos*m_cocMult*std::fabs(1/P.z - m_invFocalDist);
            P.x -= v.x;
            P.y -= v.y;
        }

        /// Compute the minimum lensShift inside the interval [z1,z2]
        Vec2 minShiftForBound(float z1, float z2) const
        {
            // First check whether the bound spans the focal plane.
            if((z1 <= m_focalDistance && z2 >= m_focalDistance) ||
               (z1 >= m_focalDistance && z2 <= m_focalDistance))
                return Vec2(0);
            // Otherwise, the minimum focal blur is achieved at one of the
            // z-extents of the bound.
            return m_cocMult*std::min(std::fabs(1/z1 - m_invFocalDist),
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
        Vec2 cocScale = coc->minShiftForBound(holder->bound().min.z,
                                              holder->bound().max.z);
        // The CoC shift is in the sraster coordinate system, so divide by
        // superSamp to get it into pixel-based raster coords.  pixel-based
        // raster is the relevant coordinates which determine the size of
        // details which will be visible after filtering.
        cocScale /= Vec2(m_opts->superSamp);
        float minCoC = std::min(cocScale.x, cocScale.y);
        const float lengthRatio = 0.16;
        polyLength *= std::max(1.0f, lengthRatio*attrs.focusFactor*minCoC);
    }
    return polyLength;
}

//------------------------------------------------------------------------------
// Renderer implementation, utility stuff.

namespace {
template<typename T>
void clampOptBelow(T& val, const char* name, const T& minVal,
                   const char* reason = 0)
{
    if(val < minVal)
    {
        std::cerr << "Warning: Option " << name << " = " << val
                  << " is too small";
        if(reason)
            std::cerr << " (" << reason << ")";
        std::cerr << ".  Clamping to " << minVal << ".\n";
        val = minVal;
    }
}
} // anon namespace

void Renderer::sanitizeOptions(Options& opts)
{
#define CLAMP_OPT_BELOW(optname, minVal) \
    clampOptBelow(opts.optname, #optname, minVal)
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
                  "must be greater than filter size");
    clampOptBelow(opts.bucketSize.y, "bucketsize.y", minBucketSize.y,
                  "must be greater than filter size");
    CLAMP_OPT_BELOW(superSamp.x, 1);
    CLAMP_OPT_BELOW(superSamp.y, 1);
    CLAMP_OPT_BELOW(shutterMax, opts.shutterMin);
}


//------------------------------------------------------------------------------
// Guts of the renderer

/// Determine whether the given geometry may be visibility culled.
///
/// This function also transforms the geometry bound into sraster space.
///
/// TODO: Clean this function up - it does several unrelated things in a
/// somewhat confusing manner.
bool Renderer::rasterCull(GeomHolder& holder)
{
    // Get bound in camera space.
    Box& bound = holder.bound();
    if(bound.min.z < FLT_EPSILON && holder.splitCount() > m_opts->eyeSplits)
    {
        std::cerr << "Max eye splits encountered; geometry discarded\n";
        return true;
    }
    // Cull if outside near/far clipping range
    if(bound.max.z < m_opts->clipNear || bound.min.z > m_opts->clipFar)
        return true;
    // Transform bound to raster space.
    //
    // TODO: Support arbitrary coordinate systems for the displacement bound
    bound.min -= Vec3(holder.attrs().displacementBound);
    bound.max += Vec3(holder.attrs().displacementBound);
    float minz = bound.min.z;
    float maxz = bound.max.z;
    bound = transformBound(bound, m_camToSRaster);
    bound.min.z = minz;
    bound.max.z = maxz;
    // Cull if outside xy extent of image
    if(bound.max.x < m_samplingArea.min.x ||
       bound.min.x > m_samplingArea.max.x ||
       bound.max.y < m_samplingArea.min.y ||
       bound.min.y > m_samplingArea.max.y)
        return true;
    // Set initial reference count.
    V2i begin, end;
    m_surfaces->bucketRangeForBound(bound, begin.x, end.x, begin.y, end.y);
    holder.initBucketRefs(begin, end);
    return false;
}

bool Renderer::rasterCull(GridHolder& gridh)
{
    const Box& bound = gridh.bound();
    // Cull if outside clipping planes
    if(bound.max.z < m_opts->clipNear || bound.min.z > m_opts->clipFar)
        return true;
    // Cull if outside xy extent of image
    if(bound.max.x < m_samplingArea.min.x ||
       bound.min.x > m_samplingArea.max.x ||
       bound.max.y < m_samplingArea.min.y ||
       bound.min.y > m_samplingArea.max.y)
        return true;
    // Set initial reference count.
    V2i begin, end;
    m_surfaces->bucketRangeForBound(bound, begin.x, end.x, begin.y, end.y);
    gridh.initBucketRefs(prod(end-begin));
    return false;
}

Renderer::Renderer(const OptionsPtr& opts, const Mat4& camToScreen,
                   const Mat4& camToWorld, const DisplayList& displays)
    : m_opts(opts),
    m_coc(),
    m_surfaces(),
    m_outVars(),
    m_camToSRaster(),
    m_camToWorld(camToWorld)
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
    Imath::Box2i outTileRange(V2i(0), nbuckets - V2i(1));
    m_filterProcessor.reset(
            new FilterProcessor(*m_displayManager, outTileRange,
                                *m_pixelFilter, m_opts->superSamp) );

    // Area to sample, in sraster coords.
    m_samplingArea = Imath::Box2f(Vec2(-m_pixelFilter->offset()),
                                  Vec2(m_opts->resolution*m_opts->superSamp +
                                       m_pixelFilter->offset()));

    V2i sampTileSize = m_opts->superSamp*m_opts->bucketSize;
    Vec2 sampTileOffset(sampTileSize/2);
    Imath::Box2f bucketArea(-sampTileOffset, Vec2(nbuckets * sampTileSize
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
        * Mat4().setScale(Vec3(0.5,-0.5,0))
        * Mat4().setTranslation(Vec3(0.5,0.5,0))
        * Mat4().setScale(Vec3(m_opts->resolution.x*m_opts->superSamp.x,
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
    if(rasterCull(*holder))
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

#   ifdef AQSIS_USE_THREADS
    int nthreads = m_opts->nthreads;
    int ncpus = boost::thread::hardware_concurrency();
    if(nthreads <= 0)
        nthreads = ncpus;

    BucketSchedulerShared scheduler(V2i(m_surfaces->nxBuckets(),
                                        m_surfaces->nyBuckets()), nthreads);
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

    frameStats.printStats(std::cout);
}


void Renderer::renderBuckets(BucketSchedulerShared& schedulerShared,
                             RenderStats& frameStats)
{
    BucketScheduler bucketScheduler(schedulerShared);
    RenderStats stats(frameStats.verbosity);
    stats.averagePolyArea.setScale(1.0/prod(m_opts->superSamp));
    // Coordinate system for tessellation resolution calculation.
    Mat4 tessCoords = m_camToSRaster
        * Mat4().setScale(Vec3(1.0/m_opts->superSamp.x,
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
        // memLog.log(); // FIXME
        V2i sampleOffset = bucketPos*tileSize - tileSize/2;
        // Create new tile for fragment storage
        FragmentTilePtr fragments =
            new FragmentTile(tileSize, sampleOffset,
                             &m_defaultFrag[0], m_defaultFrag.size());
        samples.reset(*fragments);

        // Process all surfaces and grids in the bucket.
        while(GeomHolder* geomh = queue.pop())
        {
            if(!geomh->hasChildren())
            {
                if(samples.occludes(geomh->bound(), stats.occlTime))
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
                    Mat4 scaledTessCoords = tessCoords * Mat4().setScale(
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
                if(queue.boundIntersects(gridh->bound()))
                {
                    if(!samples.occludes(gridh->bound(), stats.occlTime))
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
    tile.ensureSampleInit();
    holder.setRasterized();
    if(holder.isDeforming() || m_opts->fstop != FLT_MAX)
    {
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
    std::cerr << "Warning: MB & DoF are temporarily broken\n";
    /*
    // Determine index of depth output data, if any.
    int zOffset = -1;
    int zIdx = m_outVars.find(StdOutInd::z);
    if(zIdx != OutvarSet::npos)
        zOffset = m_outVars[zIdx].offset;

    bool motionBlur = holder.isDeforming();

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

            stats.averagePolyArea += 0.5*std::abs(vec2_cast(Pa-Pc) %
                                                     vec2_cast(Pb-Pd));

            // Iterate over samples at current time which come from tiles
            // which cross the bound.
            Imath::V2i bndMin = ifloor(vec2_cast(bound.min)*tileBoundMult);
            Imath::V2i bndMax = ifloor(vec2_cast(bound.max)*tileBoundMult);

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
                ++stats.samplesTested;
                if(!hitTest(samp))
                    continue;
                ++stats.samplesHit;
                Vec2 uv = invBilin(samp.p);
                float z = bilerp(Pa.z, Pb.z, Pd.z, Pc.z, uv);
                if(samp.z < z)
                    continue; // Ignore if hit is hidden
                samp.z = z;
                // Generate & store a fragment
                float* samples = &m_sampStorage->m_fragments[shuffIdx*fragSize];
                // Initialize fragment data with the default value.
                for(int i = 0; i < fragSize; ++i)
                    samples[i] = defaultFrag[i];
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
    */
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
    const float* defaultFrag = &m_defaultFrag[0];

    V2i tileSize = tile.size();

    Vec2 bucketMin = Vec2(tile.sampleOffset());
    Vec2 bucketMax = Vec2(tile.sampleOffset() + tileSize);

    // Construct a sampler for the polygons in the grid
    PolySamplerT poly(grid, holder.attrs(), m_outVars);
    // iterate over all micropolys in the grid & render each one.
    for(;poly.valid(); poly.next())
    {
        // TODO: Is it worth precomputing and storing the bound?
        Box bound = poly.bound();
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
