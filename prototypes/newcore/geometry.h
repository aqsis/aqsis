#ifndef GEOMETRY_H_INCLUDED
#define GEOMETRY_H_INCLUDED

#include "util.h"

class Grid;
class Geometry;

/// Destination queue for split or diced geometry
///
/// Geometry pushes split/diced subobjects back into the render queue via this
/// interface, from which it gets forwarded to the renderer.
class RenderQueue
{
    public:
        /// Push a surface onto the render pipeline.
        ///
        /// The results from splitting operations go here.
        virtual void push(const boost::shared_ptr<Geometry>& geom) = 0;
        /// Push a grid into the render pipeline
        ///
        /// The results from geometry dicing go here.
        virtual void push(const boost::shared_ptr<Grid>& grid) = 0;

        virtual ~RenderQueue() {};
};


/// Abstract piece of geometry to be rendered
class Geometry
{
    public:
        // Return the number of values required to represent a primvar of each
        // storage class for the surface.
//        virtual StorageCount storageCount() const = 0;

        /// Return a bounding box for the geometry.
        virtual Box bound() const = 0;

        /// Split or tesselate the geometry & push it back into the renderer
        /// pipeline
        ///
        /// This function is an abstraction around the "split" and "dice"
        /// parts of the reyes pipeline.  If the geometry is "small enough" it
        /// should be diced (tesselated) into grids of micropolygons.  If not,
        /// it should be split into smaller pieces and pushed back into the
        /// queue.
        virtual void splitdice(const Mat4& proj, RenderQueue& queue) const = 0;

        // Transform the surface into a new coordinate system.
        virtual void transform(const Mat4& trans) = 0;

        virtual ~Geometry() {}
};

#endif // GEOMETRY_H_INCLUDED
