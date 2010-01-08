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

#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <cstring>
#include <queue>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>

#include "geometry.h"
#include "options.h"
#include "sample.h"
#include "util.h"
#include "varspec.h"

class Renderer;
class Grid;
class QuadGridSimple;

// Minimal wrapper around a renderer instance to provide control context for
// when surfaces push split/diced objects back into the render's queue.
class RenderQueueImpl : public RenderQueue
{
    private:
        Renderer& m_renderer;
        int m_splitDepth;
    public:
        RenderQueueImpl(Renderer& renderer, int splitDepth)
            : m_renderer(renderer),
            m_splitDepth(splitDepth)
        { }

        void push(const boost::shared_ptr<Geometry>& geom);
        void push(const boost::shared_ptr<Grid>& grid);
};

struct OutvarSpec : public VarSpec
{
    int offset;  ///< Offset

    OutvarSpec(Type type, int arraySize, ustring name, int offset)
        : VarSpec(type, arraySize, name), offset(offset) {}
    OutvarSpec(const VarSpec& spec, int offset)
        : VarSpec(spec), offset(offset) {}
};

typedef std::vector<OutvarSpec> OutvarList;

class Renderer
{
    public:
        Renderer(const Options& opts, const Mat4& camToScreen = Mat4(),
                 const VarList& outVars = VarList());

        /// Add geometry
        void add(const boost::shared_ptr<Geometry>& geom);

        /// Render all surfaces and save resulting image.
        void render();


    private:
        // RenderQueueImpl is a friend so that it can appropriately push()
        // surfaces and grids into the renderer.
        friend class RenderQueueImpl;

        // Container for geometry and geometry metadata
        struct SurfaceHolder
        {
            boost::shared_ptr<Geometry> geom; //< Pointer to geometry
            int splitCount; //< Number of times the geometry has been split
            Box bound;      //< Bound in camera coordinates

            SurfaceHolder(const boost::shared_ptr<Geometry>& geom,
                          int splitCount, Box bound)
                : geom(geom),
                splitCount(splitCount),
                bound(bound)
            { }
        };

        // Ordering functor for surfaces in the render queue
        class surface_order
        {
            private:
                // desired bucket height in camera coordinates
                float m_bucketHeight;
            public:
                surface_order() : m_bucketHeight(16) {}

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

        typedef std::priority_queue<SurfaceHolder, std::vector<SurfaceHolder>,
                                    surface_order> SurfaceQueue;

        Options m_opts;                ///< Render options
        SurfaceQueue m_surfaces;       ///< Queue of surface to be rendered
        std::vector<Sample> m_samples; ///< Array of sample info
        OutvarList m_outVars;          ///< Output variable list
        std::vector<float> m_defOutSamps; ///< Default output samples
        std::vector<float> m_image;    ///< Image data
        Mat4 m_camToRas;               ///< Camera -> raster transformation

        void saveImages(const std::string& baseFileName);

        void defaultSamples(float* defaultSamps);
        void initSamples();
        void push(const boost::shared_ptr<Geometry>& geom, int splitCount);
        void push(const boost::shared_ptr<Grid>& grid);

        template<typename GridT, typename PolySamplerT>
        void rasterize(GridT& grid);

        void rasterizeSimple(QuadGridSimple& grid);
};


#endif // RENDERER_H_INCLUDED
