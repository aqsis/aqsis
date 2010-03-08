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

#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

#include "options.h"
#include "util.h"
#include "varspec.h"

class Attributes;
class Geometry;
class Grid;
class QuadGridSimple;
class TessellationContextImpl;
class SampleStorage;
class CircleOfConfusion;

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
        Renderer(const Options& opts, const Mat4& camToScreen = Mat4(),
                 const VarList& outVars = VarList());

        ~Renderer();

        /// Add geometry
        void add(const GeometryPtr& geom, Attributes& attrs);
        /// Add key frames of deforming geometry
        void add(GeometryKeys& deformingGeom, Attributes& attrs);

        /// Render all surfaces and save resulting image.
        void render();


    private:
        // TessellationContextImpl is a friend so that it can appropriately
        // push() surfaces and grids into the renderer.
        friend class TessellationContextImpl;

        class SurfaceOrder;
        typedef std::priority_queue<GeomHolder, std::vector<GeomHolderPtr>,
                                    SurfaceOrder> SurfaceQueue;

        static void sanitizeOptions(Options& opts);

        void saveImages(const std::string& baseFileName);

        void push(const GeomHolderPtr& geom);
        void push(const GridHolderPtr& grid);

        template<typename GridT, typename PolySamplerT>
        void motionRasterize(GridHolder& holder);

        template<typename GridT, typename PolySamplerT>
        void rasterize(Grid& inGrid, const Attributes& attrs);

        void rasterizeSimple(QuadGridSimple& grid, const Attributes& attrs);


        Options m_opts;                ///< Render options
        boost::scoped_ptr<CircleOfConfusion> m_coc; ///< depth of field info
        boost::scoped_ptr<SurfaceQueue> m_surfaces; ///< Pending surface queue
        OutvarSet m_outVars;           ///< Set of output variables
        boost::scoped_ptr<SampleStorage> m_sampStorage; ///< Samples & fragments
        Mat4 m_camToRas;               ///< Camera -> raster transformation
};


#endif // RENDERER_H_INCLUDED
