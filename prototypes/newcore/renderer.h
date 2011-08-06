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

#ifndef AQSIS_RENDERER_H_INCLUDED
#define AQSIS_RENDERER_H_INCLUDED

#ifdef _WIN32
/* Make sure that including windows.h doesn't define the min and max macros,
 * which conflict with other uses of min and max (Aqsis::min, std::min etc.) */
#ifndef NOMINMAX
#define NOMINMAX
#endif

/* Make sure that the math constants from math.h are defined - that is, M_PI
 * etc.
 */
#ifndef _USE_MATH_DEFINES
#   define _USE_MATH_DEFINES
#endif

/* Faster windows compilation, and less bloat */
#define WIN32_LEAN_AND_MEAN

#endif

#include <cstring>
#include <queue>
#include <vector>

#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

#include <aqsis/riutil/errorhandler.h>

#include "attributes.h"
#include "options.h"
#include "thread.h"
#include "varspec.h"
#include "util.h"

namespace Aqsis {

class BucketSchedulerShared;
class CachedFilter;
class Geometry;
class Grid;
class TessellationContextImpl;
class SampleStorage;
class CircleOfConfusion;
class SplitStore;
class SampleTile;
class FilterProcessor;
class DisplayManager;
class RenderStats;
class DisplayList;
class DofMbTileSet;

class GeomHolder;
typedef boost::intrusive_ptr<GeomHolder> GeomHolderPtr;
class GridHolder;
typedef boost::intrusive_ptr<GridHolder> GridHolderPtr;

typedef boost::intrusive_ptr<Geometry> GeometryPtr;
typedef boost::intrusive_ptr<Grid> GridPtr;


//-----------------------------------------------------------------------------
/// Renderer error codes.
///
/// The relationship between these errors and the RIE_* or corresponding EqE_*
/// versions needs thought.  At the least they should have distinct error
/// codes!
struct ErrorCode
{
    enum Code
    {
        BadOption,
        MaxEyeSplits,
    };
};


//-----------------------------------------------------------------------------
struct OutvarSpec : public VarSpec
{
    int offset;  ///< Offset in output image pixel channels

    OutvarSpec() : VarSpec(), offset(-1) {}
    OutvarSpec(Type type, int arraySize, ustring name, int offset)
        : VarSpec(type, arraySize, name), offset(offset) {}
    OutvarSpec(const VarSpec& spec, int offset)
        : VarSpec(spec), offset(offset) {}
};

class StdOutInd
{
    public:
        enum Id
        {
            z
        };

        StdOutInd() : m_zInd(-1) { }

        void add(int index, const VarSpec& var)
        {
            if(var == Stdvar::z)
                m_zInd = index;
        }

        int get(Id id) const { return m_zInd; }
        int contains(Id id) const { return m_zInd != -1; }

    private:
        int m_zInd;
};

typedef BasicVarSet<OutvarSpec, StdOutInd> OutvarSet;


template<typename T>
struct MotionKey
{
    float time;
    T value;

    MotionKey(float time, const T& value)
        : time(time), value(value) {}
};
typedef MotionKey<GeometryPtr> GeometryKey;
typedef std::vector<GeometryKey> GeometryKeys;

//-----------------------------------------------------------------------------
/// Main renderer interface.
///
/// Renderer is intended to have a minimal, stateless interface, designed so
/// that more convenient interfaces like the RI can be layered on top.
class Renderer
{
    public:
        Renderer(const OptionsPtr& opts, const M44f& camToScreen,
                 const M44f& camToWorld, const DisplayList& displays,
                 ErrorHandler& errorHandler);

        ~Renderer();

        /// Add geometry
        void add(const GeometryPtr& geom, const ConstAttributesPtr& attrs);
        /// Add key frames of deforming geometry
        void add(GeometryKeys& deformingGeom, const ConstAttributesPtr& attrs);

        /// Render all surfaces and save resulting image.
        void render();


    private:
        // TessellationContextImpl is a friend so that it can appropriately
        // push() surfaces and grids into the renderer.
        friend class TessellationContextImpl;

        float micropolyBlurWidth(const GeomHolderPtr& holder,
                                 const CircleOfConfusion* coc) const;

        void sanitizeOptions(Options& opts);

        bool rasterCull(GeomHolder& geom, const Box2i* parentBucketBound);
        bool rasterCull(GridHolder& grid, const Box2i& parentBucketBound);
        void add(const GeomHolderPtr& geom);

        void renderBuckets(BucketSchedulerShared& bucketScheduler,
                           RenderStats& stats);

        void rasterize(SampleTile& tile, GridHolder& holder,
                       RenderStats& stats);

        template<typename GridT, typename PolySamplerT>
        void mbdofRasterize(SampleTile& tile, const GridHolder& holder,
                            RenderStats& stats);

        template<typename GridT, typename PolySamplerT>
        void staticRasterize(SampleTile& tile, const GridHolder& holder,
                             RenderStats& stats);


        OptionsPtr m_opts;             ///< Render options
        boost::scoped_ptr<CircleOfConfusion> m_coc; ///< depth of field info
        boost::scoped_ptr<SplitStore> m_surfaces; ///< Pending surface queue
        OutvarSet m_outVars;           ///< Set of output variables
        boost::scoped_ptr<CachedFilter> m_pixelFilter;
        boost::scoped_ptr<FilterProcessor> m_filterProcessor;
        boost::scoped_ptr<DisplayManager> m_displayManager;
        M44f m_camToSRaster;           ///< Camera -> sample raster transformation
        M44f m_camToWorld;             ///< Camera -> world transformation
        Box2f m_samplingArea;   ///< Area to sample in sraster coords
        std::vector<float> m_defaultFrag; ///< Default fragment samples

        Mutex m_dofMbTileInit;  ///< Mutex for m_dofMbTileSet init
        /// Cached DoF/MB sampling setup data.
        const DofMbTileSet* m_dofMbTileSet;
        /// Handler for renderer error messages.
        ErrorHandler& m_errorHandler;
};


} // namespace Aqsis
#endif // AQSIS_RENDERER_H_INCLUDED
