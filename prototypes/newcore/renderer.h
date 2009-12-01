#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <vector>

#include <OpenEXR/ImathFun.h>

#include "util.h"
#include "sample.h"
#include "options.h"

// Minimal wrapper around a renderer instance to provide control context for
// when surfaces push split/diced objects back into the render queue on the
// renderer.
class RenderQueue
{
	private:
		Renderer& m_renderer;
		int m_splitDepth;
	public:
        RenderQueue(Renderer& renderer, int splitDepth)
            : m_renderer(renderer),
            m_splitDepth(splitDepth)
        { }

		void push(const boost::shared_ptr<Surface>& s)
		{
			m_renderer.push(s, m_splitDepth+1);
		}
		void push(const boost::shared_ptr<Grid>& g)
		{
			m_renderer.render(g);
		}
};


class Surface
{
	public:
		// Return the number of values required to represent a primvar of each
		// storage class for the surface.
		StorageCount storageCount() const = 0;

#if 0
		// Determine whether the surface is diceable from the point of view of
		// the matrix m.  m is not necessarily a projection matrix onto the xy
		// plane, though this is the usual case.
		//
		// Also determines (& stores internally) how the surface
		// should be split if it is not diceable.
		bool diceable(const Mat4& m) const = 0;

		// Split the surface into smaller subsurfaces & add those to the
		// renderer context.
		void split(Renderer& renderer) const = 0;

		// Create a grid of micropolygons from the surface.
		//
		/*?*/ void dice(/*?*/) const = 0;
#endif

		// Split or dice a surface & push it back into the renderer queue.
		//
		// 
		// dice it & push the resulting grids at the renderer.  If not, split
		// it & push the resulting smaller surfaces at the renderer.
		void splitdice(const Mat4& proj, RenderQueue& renderer) const = 0;
};


class Renderer
{
    private:
        class surface_order
        {
            private:
                float m_bucketHeight;
            public:
                surface_order() : m_bucketHeight(16) {}

                operator()(const boost::shared_ptr<Surface>& a,
                           const boost::shared_ptr<Surface>& b) const
                {
                    float ya = a.bound().min.y;
                    float yb = b.bound().min.y;
                    if(ya < yb - m_bucketHeight)
                        return true;
                    else if(yb < ya - m_bucketHeight)
                        return false;
                    else
                        return a.bound().min.x < b.bound().min.x;
                }
        };
        typedef std::priority_queue<boost::shared_ptr<Surface>,
            std::vector<boost::shared_ptr<Surface> >, surface_order> SurfaceQueue;

        Options m_opts;

        SurfaceQueue m_surfaces;
        std::vector<Sample> m_samples;

    public:
        Renderer(Options opts)
            : m_opts(opts)
        {
            m_surfaces
        }

		void push(boost::shared_ptr<Surface>& surf, int splitLevel = 0)
		{
            if(splitLevel > m_opts.maxSplits)
                m_errHandler.warning("max splits; surface discarded");
            else
                m_surfaces.push(surf);
		}

		void push(boost::shared_ptr<Grid>& grid)
		{
			// place grid in render queue, or (perhaps) simply call
			grid->rasterize(Renderer)
		}

        void render()
        {
            while(!m_surfaces.empty())
            {
                const boost::shared_ptr<Surface>& s = m_surfaces.top();
                RenderQueue queue(*this, 0);
                s.splitdice(m_camToRas, queue);
            }
            // Standard reyes split/dice/sample loop
            /*
            Grid grid;
            // For each surface
            while(!m_surfaces.empty())
            {
                const boost::shared_ptr<Surface>& s = m_surfaces.top();
                if(s.diceable())
                {
                    // If the surface is diceable, dice & sample it.
                    s.dice(grid);
                    // <-- shading would go here
                    rasterize(grid);
                }
                else
                {
                    // If not diceable, split the surface & push the results
                    // onto the queue.  The surface puts the results directly
                    // back into the render pipeline via the push() function.
                    s.split(*this);
                    m_surfaces.push()
                }
                m_surfaces.pop();
            }
            */
        }

        // Render a grid by rasterizing each micropolygon.
        void rasterize(const Grid& grid)
        {
            int nxSubSamps = m_opts.xRes*m_opts.nsubSamps.x;
            int nySubSamps = m_opts.yRes*m_opts.nsubSamps.y;
            // iterate over all micropolys in the grid & render each one.
            for(Grid::Iterator i = grid.begin(); i.valid(); ++i)
            {
                Grid::UPoly poly = i.poly();
                Box bound = poly.bound();

                // Bounding box for relevant samples, clamped to image extent.
                const int sx = Imath::clamp(Imath::floor(
                            bound.min.x*m_opts.nsubSamps.x), 0, nxSubSamps);
                const int ex = Imath::clamp(Imath::floor(
                            bound.max.x*m_opts.nsubSamps.x)+1, 0, nxSubSamps);
                const int sy = Imath::clamp(Imath::floor(
                            bound.min.y*m_opts.nsubSamps.y), 0, nySubSamps);
                const int ey = Imath::clamp(Imath::floor(
                            bound.max.y*m_opts.nsubSamps.y)+1, 0, nySubSamps);

                // for each sample position in the bound
                for(int ix = sx; ix < ex; ++ix)
                {
                    for(int iy = sx; iy < ex; ++iy)
                    {
                        int idx = nxSubSamps*iy + ix;
                        Sample& samp = m_samples[idx];
                        // Early out if definitely hidden
                        if(samp.z < bound.min.z)
                            continue;
                        float z = 0;
                        // Ignore if sample didn't hit the polygon
                        if(!poly.contains(m_samples[idx], z))
                            continue;
                        // Ignore if sample is hidden
                        if(z < 0 || z > samp.z)
                            continue;
                        // Generate & store a fragment
                    }
                }
            }
        }

};


#endif // RENDERER_H_INCLUDED
