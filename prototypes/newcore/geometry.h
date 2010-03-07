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

#ifndef GEOMETRY_H_INCLUDED
#define GEOMETRY_H_INCLUDED

#include "util.h"

class Grid;
class GridStorageBuilder;
class Geometry;
class Options;
class Attributes;
class TessControl;

typedef boost::intrusive_ptr<Geometry> GeometryPtr;
typedef boost::intrusive_ptr<Grid> GridPtr;

/// Tessellation context for geometric split/dice
///
/// The split/dice part of an reyes pipeline is an exercise in generating a
/// fine, uniform, adaptive tesselation of the geometry to be rendered.  The
/// uniformity of the tesselation is usually measured with respect to raster
/// space.
///
/// During tessellation, geometry needs to know how finely to split itself up,
/// and the results have to be placed somewhere.  TessellationContext fulfils
/// these needs.
class TessellationContext
{
    public:
        /// Invoke the tessellator object with the current geometry being
        /// processed as the argument.
        virtual void invokeTessellator(TessControl& tessControl) = 0;

        /// Push some geometry into the render pipeline.
        /// The results from splitting operations go here.
        virtual void push(const GeometryPtr& geom) = 0;

        /// Push a grid into the render pipeline.
        /// The results from geometry dicing go here.
        virtual void push(const GridPtr& grid) = 0;

        /// Return the renderer option state
        virtual const Options& options() = 0;
        /// Return the current surface options state
        virtual const Attributes& attributes() = 0;

        /// Get a builder for storage which will hold dicing results.
        virtual GridStorageBuilder& gridStorageBuilder() = 0;

        virtual ~TessellationContext() {};
};


/// Abstract piece of geometry to be rendered
///
/// TODO: Need really good docs on how this works
class Geometry : public RefCounted
{
    public:
        // Return the number of values required to represent a primvar of each
        // storage class for the surface.
//        virtual StorageCount storageCount() const = 0;

        /// Return a bounding box for the geometry.
        ///
        /// This will only be called once by the renderer for each piece of
        /// geometry - there's no need to cache the results internally or
        /// anything.
        ///
        virtual Box bound() const = 0;

        /// Tessellation driver function; the "split/dice decision"
        ///
        /// In reyes language, this function determines whether the surface
        /// should be split or diced.  It should then submit the resulting
        /// decision object to the tessellation context via the
        /// invokeTessellator() function.  (Avoiding doing the actual
        /// splitting and dicing directly inside this function allows the
        /// renderer to do the hard work when keeping track of deformation
        /// motion blur.)
        ///
        /// \param trans - Transform into "dice coordinates".  The geometry
        ///                should transform vertices into this space to
        ///                determine how large the object will be in raster
        ///                coordinates.  Note that in general trans is not a
        ///                projection, and the geometry shouldn't neglect the
        ///                z-coordinate when measuring the size.
        ///
        /// \param polyLength - The desired linear size ("edge length") of
        ///                micropolygon pieces resulting from dicing, as
        ///                measured in the trans coordinate system.
        ///                diceLength is the square root of the reyes shading
        ///                rate.
        ///
        virtual void tessellate(const Mat4& trans, float polyLength,
                                TessellationContext& tessCtx) const = 0;

        /// Transform the surface into a new coordinate system.
        ///
        /// TODO: Remove this; geometry should be immutable in the main part
        /// of the pipeline.
        virtual void transform(const Mat4& trans) = 0;

        virtual ~Geometry() {}
};


/// Control geometric tessellation (eg, know whether to split or dice)
///
/// Each Geometry class should have an associated TessControl class which can
/// hold the tessellation decision determined by Geometry::tessellate().
/// Normally the "tessellation decision" will be a split/dice decision, along
/// with associated split direction or dice resolution.
class TessControl
{
    public:
        /// Tessellate the geometry & push the results back to the context.
        virtual void tessellate(const Geometry& geom,
                                TessellationContext& tessContext) const = 0;

        virtual ~TessControl() {};
};


//------------------------------------------------------------------------------
// Tessellator control objects for the usual reyes-style split/dice
// tessellation of 2D surfaces:

/// Tessellator control for dicing 2D surfaces.
template<typename GeomT>
class SurfaceDicer : public TessControl
{
    private:
        int m_nu;
        int m_nv;
    public:
        SurfaceDicer(int nu, int nv) : m_nu(nu), m_nv(nv) {}

        virtual void tessellate(const Geometry& geom,
                                TessellationContext& tessContext) const
        {
            const GeomT& g = static_cast<const GeomT&>(geom);
            g.dice(m_nu, m_nv, tessContext);
        }
};


/// Tessellator control for splitting 2D surfaces.
template<typename GeomT>
class SurfaceSplitter : public TessControl
{
    private:
        bool m_splitDirectionU;
    public:
        SurfaceSplitter(bool splitDirectionU)
            : m_splitDirectionU(splitDirectionU) {}

        virtual void tessellate(const Geometry& geom,
                                TessellationContext& tessContext) const
        {
            const GeomT& g = static_cast<const GeomT&>(geom);
            g.split(m_splitDirectionU, tessContext);
        }
};


#endif // GEOMETRY_H_INCLUDED
