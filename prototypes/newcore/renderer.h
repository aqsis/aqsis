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

#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <cstring>
#include <queue>
#include <vector>

#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

#include "attributes.h"
#include "options.h"
#include "varspec.h"
#include "util.h"

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

class GeomHolder;
typedef boost::intrusive_ptr<GeomHolder> GeomHolderPtr;
class GridHolder;
typedef boost::intrusive_ptr<GridHolder> GridHolderPtr;

typedef boost::intrusive_ptr<Geometry> GeometryPtr;
typedef boost::intrusive_ptr<Grid> GridPtr;


//-----------------------------------------------------------------------------
struct OutvarSpec : public VarSpec
{
    int offset;  ///< Offset in output image pixel channels

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
        Renderer(const OptionsPtr& opts, const Mat4& camToScreen = Mat4(),
                 const VarList& outVars = VarList());

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
        class Stats;

        float micropolyBlurWidth(const GeomHolderPtr& holder,
                                 const CircleOfConfusion* coc) const;

        static void sanitizeOptions(Options& opts);

        bool rasterCull(GeomHolder& holder);
        void add(const GeomHolderPtr& geom);

        void renderBuckets(BucketSchedulerShared& bucketScheduler);

        void rasterize(SampleTile& tile, GridHolder& holder);

        template<typename GridT, typename PolySamplerT>
        void mbdofRasterize(SampleTile& tile, const GridHolder& holder);

        template<typename GridT, typename PolySamplerT>
        void staticRasterize(SampleTile& tile, const GridHolder& holder);


        OptionsPtr m_opts;             ///< Render options
        boost::scoped_ptr<CircleOfConfusion> m_coc; ///< depth of field info
        boost::scoped_ptr<SplitStore> m_surfaces; ///< Pending surface queue
        OutvarSet m_outVars;           ///< Set of output variables
        boost::scoped_ptr<CachedFilter> m_pixelFilter;
        boost::scoped_ptr<FilterProcessor> m_filterProcessor;
        boost::scoped_ptr<DisplayManager> m_displayManager;
        Mat4 m_camToSRaster;           ///< Camera -> sample raster transformation
        Imath::Box2f m_samplingArea;   ///< Area to sample in sraster coords
        std::vector<float> m_defaultFrag; ///< Default fragment samples
        mutable boost::scoped_ptr<Stats> m_stats; ///< Render statistics
};


#endif // RENDERER_H_INCLUDED
