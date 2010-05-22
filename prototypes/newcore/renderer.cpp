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
#include "samplestorage.h"
#include "simple.h"


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


//------------------------------------------------------------------------------
/// Container for geometry and geometry metadata
class GeomHolder : public RefCounted
{
    private:
        GeometryPtr m_geom;  ///< Main geometry (first key for deformation)
        GeometryKeys m_geomKeys; ///< Extra geometry keys
        int m_splitCount;    ///< Number of times the geometry has been split
        Box m_bound;         ///< Bound in camera coordinates
        Attributes* m_attrs; ///< Surface attribute state

        /// Get the bound from a set of geometry keys
        static Box boundFromKeys(const GeometryKeys& keys)
        {
            Box bound = keys[0].value->bound();
            for(int i = 1, iend = keys.size(); i < iend; ++i)
                bound.extendBy(keys[i].value->bound());
            return bound;
        }

    public:
        /// Create initial non-deforming geometry (no parent surface)
        GeomHolder(const GeometryPtr& geom, Attributes* attrs)
            : m_geom(geom),
            m_geomKeys(),
            m_splitCount(0),
            m_bound(geom->bound()),
            m_attrs(attrs)
        { }
        /// Create geometry resulting from splitting (has a parent surface)
        GeomHolder(const GeometryPtr& geom, const GeomHolder& parent)
            : m_geom(geom),
            m_geomKeys(),
            m_splitCount(parent.m_splitCount+1),
            m_bound(geom->bound()),
            m_attrs(parent.m_attrs)
        { }

        /// Create initial deforming geometry (no parent surface)
        GeomHolder(GeometryKeys& keys, Attributes* attrs)
            : m_geom(),
            m_geomKeys(keys),
            m_splitCount(0),
            m_bound(boundFromKeys(m_geomKeys)),
            m_attrs(attrs)
        { }
        /// Create deforming geometry resulting from splitting
        GeomHolder(GeometryPtr* keysBegin, GeometryPtr* keysEnd,
                   int keysStride, const GeomHolder& parent)
            : m_geom(),
            m_geomKeys(),
            m_splitCount(parent.m_splitCount+1),
            m_bound(),
            m_attrs(parent.m_attrs)
        {
            // Init geom keys, taking every keysStride'th geometry
            assert(static_cast<int>(parent.geomKeys().size())
                   == (keysEnd-keysBegin)/keysStride);
            m_geomKeys.reserve(parent.geomKeys().size());
            GeometryKeys::const_iterator oldKey = parent.geomKeys().begin();
            for(; keysBegin < keysEnd; keysBegin += keysStride, ++oldKey)
                m_geomKeys.push_back(GeometryKey(oldKey->time, *keysBegin));
            m_bound = boundFromKeys(m_geomKeys);
        }

        /// Get non-deforming geometry or first key frame
        const Geometry& geom() const
        {
            return m_geom ? *m_geom : *m_geomKeys[0].value;
        }

        /// Get deforming geometry key frames.  Empty if non-moving.
        const GeometryKeys& geomKeys() const { return m_geomKeys; }

        bool isDeforming() const { return !m_geom; }
        int splitCount() const    { return m_splitCount; }
        Box& bound() { return m_bound; }
        Attributes& attrs()       { return *m_attrs; }
        const Attributes& attrs() const { return *m_attrs; }
};



//------------------------------------------------------------------------------
typedef MotionKey<GridPtr> GridKey;
typedef std::vector<GridKey> GridKeys;

class GridHolder : public RefCounted
{
    private:
        GridPtr m_grid;            ///< Non-deforming grid
        GridKeys m_gridKeys;       ///< Grid keys for motion blur
        const Attributes* m_attrs; ///< Attribute state

        void shade(Grid& grid) const
        {
            // Special case: can't shade simple grids.
            if(grid.type() == GridType_QuadSimple)
                return;
            if(m_attrs->surfaceShader)
                m_attrs->surfaceShader->shade(grid);
        }

    public:
        GridHolder(const GridPtr& grid, const Attributes* attrs)
            : m_grid(grid),
            m_gridKeys(),
            m_attrs(attrs)
        { }

        template<typename GridPtrIterT>
        GridHolder(GridPtrIterT begin, GridPtrIterT end,
                   const GeomHolder& parentGeom)
            : m_grid(),
            m_gridKeys(),
            m_attrs(&parentGeom.attrs())
        {
            GeometryKeys::const_iterator oldKey = parentGeom.geomKeys().begin();
            m_gridKeys.reserve(end - begin);
            for(;begin != end; ++begin, ++oldKey)
                m_gridKeys.push_back(GridKey(oldKey->time, *begin));
        }

        bool isDeforming() const { return !m_grid; }
        Grid& grid() { return m_grid ? *m_grid : *m_gridKeys[0].value; }
        GridKeys& gridKeys() { return m_gridKeys; }

        const Attributes& attrs() const { return *m_attrs; }

        /// Shade all grids
        void shade()
        {
            if(isDeforming())
            {
                // TODO: Run displacement shaders only on extra grids
                // TODO: Fill in time variable on the grid before shading
                for(int i = 0, iend = m_gridKeys.size(); i < iend; ++i)
                    shade(*m_gridKeys[i].value);
            }
            else
                shade(*m_grid);
        }

        /// Project all grids held by the holder
        void project(const Mat4& camToRas)
        {
            if(isDeforming())
            {
                for(int i = 0, iend = m_gridKeys.size(); i < iend; ++i)
                    m_gridKeys[i].value->project(camToRas);
            }
            else
                m_grid->project(camToRas);
        }
};


//------------------------------------------------------------------------------
/// Ordering functor for surfaces in the render queue
class Renderer::SurfaceOrder
{
    private:
        // desired bucket height in camera coordinates
        float m_bucketHeight;
    public:
        SurfaceOrder() : m_bucketHeight(16) {}

        bool operator()(const GeomHolderPtr& a, const GeomHolderPtr& b) const
        {
            float ya = a->bound().min.y;
            float yb = b->bound().min.y;
            if(ya < yb - m_bucketHeight)
                return true;
            else if(yb < ya - m_bucketHeight)
                return false;
            else
                return a->bound().min.x < b->bound().min.x;
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
        const GeomHolder* m_parentGeom;

        /// Collecter for splits of deforming surfaces
        class DeformCollector
        {
            private:
                std::vector<GeometryPtr> m_splits;
                int m_splitsPerKey;
                std::vector<GridPtr> m_grids;

            public:
                void reset()
                {
                    m_splits.clear();
                    m_splitsPerKey = 0;
                    m_grids.clear();
                }

                void firstKeyDone()
                {
                    m_splitsPerKey = m_splits.size();
                }

                void push(const GeometryPtr& geom)
                {
                    m_splits.push_back(geom);
                }

                void push(const GridPtr& grid)
                {
                    m_grids.push_back(grid);
                }

                void pushResults(const GeomHolder& parentGeom,
                                 Renderer& renderer)
                {
                    if(!m_splits.empty())
                    {
                        // Construct holder for each deforming child surface &
                        // push it back to the renderer.
                        assert((m_splits.size()/m_splitsPerKey)*m_splitsPerKey
                                == m_splits.size());
                        for(int i = 0; i < m_splitsPerKey; ++i)
                        {
                            GeomHolderPtr holder(
                                new GeomHolder(
                                    &*m_splits.begin() + i,
                                    &*m_splits.end() + i,
                                    m_splitsPerKey, parentGeom
                                )
                            );
                            renderer.push(holder);
                        }
                    }
                    if(!m_grids.empty())
                    {
                        // Construct deforming child grids
                        renderer.push( GridHolderPtr(
                                new GridHolder(
                                    m_grids.begin(), m_grids.end(),
                                    parentGeom
                                )
                        ));
                    }
                }
        };
        DeformCollector m_deformCollector;

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

    public:
        TessellationContextImpl(Renderer& renderer)
            : m_renderer(renderer),
            m_builder(),
            m_parentGeom(0)
        { }

        void tessellate(const Mat4& splitTrans, const GeomHolderPtr& holder)
        {
            m_parentGeom = holder.get();
            float polyLength = micropolyBlurWidth(holder, m_renderer.m_coc.get());
            holder->geom().tessellate(splitTrans, polyLength, *this);
        }

        virtual void invokeTessellator(TessControl& tessControl)
        {
            if(m_parentGeom->isDeforming())
            {
                // Collect the split/dice results in m_deformCollector if the
                // surface is involved in deformation motion blur.
                m_deformCollector.reset();
                const GeometryKeys& keys = m_parentGeom->geomKeys();
                tessControl.tessellate(*keys[0].value, *this);
                m_deformCollector.firstKeyDone();
                for(int i = 1, nkeys = keys.size(); i < nkeys; ++i)
                    tessControl.tessellate(*keys[i].value, *this);
                m_deformCollector.pushResults(*m_parentGeom, m_renderer);
            }
            else
                tessControl.tessellate(m_parentGeom->geom(), *this);
        }

        virtual void push(const GeometryPtr& geom)
        {
            if(m_parentGeom->isDeforming())
                m_deformCollector.push(geom);
            else
            {
                GeomHolderPtr holder(new GeomHolder(geom, *m_parentGeom));
                m_renderer.push(holder);
            }
        }

        virtual void push(const GridPtr& grid)
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
            if(m_parentGeom->isDeforming())
                m_deformCollector.push(grid);
            else
            {
                m_renderer.push(GridHolderPtr(
                    new GridHolder(grid, &m_parentGeom->attrs()) ));
            }
        }

        virtual const Options& options()
        {
            return m_renderer.m_opts;
        }
        virtual const Attributes& attributes()
        {
            return m_parentGeom->attrs();
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
            if(m_parentGeom->attrs().surfaceShader)
            {
                // Renderer arbitrary output vars
                const OutvarSet& aoVars = m_renderer.m_outVars;
                const Shader& shader = *m_parentGeom->attrs().surfaceShader;
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
    m_surfaces->push(holder);
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
    m_surfaces(new SurfaceQueue()),
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
    m_sampStorage.reset(new SampleStorage(m_outVars, m_opts));
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

    while(!m_surfaces->empty())
    {
        GeomHolderPtr g = m_surfaces->top();
        m_surfaces->pop();
        tessContext.tessellate(splitTrans, g);
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

