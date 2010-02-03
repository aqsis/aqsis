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
        /// Push some geometry into the render pipeline.
        /// The results from splitting operations go here.
        virtual void push(const boost::shared_ptr<Geometry>& geom) = 0;

        /// Push a grid into the render pipeline.
        /// The results from geometry dicing go here.
        virtual void push(const boost::shared_ptr<Grid>& grid) = 0;

        /// Return the renderer option state
        virtual const Options& options() = 0;
        /// Return the current surface options state
        virtual const Attributes& attributes() = 0;

        /// Get a builder for storage which will hold dicing results.
        virtual GridStorageBuilder& gridStorageBuilder() = 0;

        virtual ~TessellationContext() {};
};


/// Abstract piece of geometry to be rendered
class Geometry
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
        virtual Box bound() const = 0;

        /// Split/dice geometry & push it back at the tessellation context
        ///
        /// This function is an abstraction around the "split" and "dice"
        /// parts of the reyes pipeline.  If the geometry is "small enough" it
        /// should be diced (tesselated) into grids of micropolygons.  If not,
        /// it should be split into smaller pieces and pushed back into the
        /// queue.
        virtual void splitdice(const Mat4& proj, TessellationContext& tessCtx) const = 0;

        // Transform the surface into a new coordinate system.
        virtual void transform(const Mat4& trans) = 0;

        virtual ~Geometry() {}
};


#endif // GEOMETRY_H_INCLUDED
