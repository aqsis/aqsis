#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <vector>

#include <boost/shared_ptr.hpp>

#include "util.h"
#include "sample.h"
#include "options.h"
#include "grid.h"

#include "tiffio.h"

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
        // dice it & push the resulting grids at the renderer.  If not, split
        // it & push the resulting smaller surfaces at the renderer.
        void splitdice(const Mat4& proj, RenderQueue& renderer) const = 0;
};


class Renderer
{
    private:
        class SurfaceHolder
        {
            boost::shared_ptr<Surface> surface; //< Smart pointer to the surface
            int splitCount; //< Count of the number of times the surface has been split
            Box bound;      //< Bound in camera coordinates

            SurfaceHolder(const boost::shared_ptr<Surface>& surface, int splitCount,
                          Box bound)
                : surface(surface),
                splitCount(splitCount),
                bound(bound),
            { }
        };

        // Ordering functor for surfaces in the render queue
        class surface_order
        {
            private:
                // desired bucket height in camera coordinates
                float m_bucketHeight;
            public:
                surface_order() : m_bucketHeight(0.1) {}

                operator()(const boost::shared_ptr<SurfaceHolder>& a,
                           const boost::shared_ptr<SurfaceHolder>& b) const
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

        Options m_opts;

        SurfaceQueue m_surfaces;
        std::vector<Sample> m_samples;
        std::vector<float> m_image;

        void initSamples()
        {
            // Initialize sample array
            m_samples.resize(m_opts.xRes*m_opts.yRes);
            // Initialize sample positions to contain
            for(int j = 0; j < m_opts.yRes; ++j)
            {
                for(int i = 0; i < m_opts.xRes; ++i)
                    m_samples[j*m_opts.xRes + i] = Sample(Vec2(i+0.5f, j+0.5f), FLT_MAX);
            }
            // Initialize image array
            m_image.resize(m_opts.xRes*m_opts.yRes, FLT_MAX);
        }

        // Save image to a TIFF file.
        void saveImage(std::string& fileName)
        {
            TIFF* tif = TIFFOpen(fileName.c_str(), "r");
            if(!tif)
            {
                std::cerr << "Could not open file!\n";
                return;
            }

            // Write tiff header
            TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, uint32(m_opts.xRes));
            TIFFSetField(tif, TIFFTAG_IMAGELENGTH, uint32(m_opts.yRes));
            TIFFSetField(tif, TIFFTAG_ORIENTATION, uint16(ORIENTATION_TOPLEFT));
            TIFFSetField(tif, TIFFTAG_PLANARCONFIG, uint16(PLANARCONFIG_CONTIG));
            TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, uint16(RESUNIT_NONE));
            TIFFSetField(tif, TIFFTAG_XRESOLUTION, 1.0f);
            TIFFSetField(tif, TIFFTAG_YRESOLUTION, 1.0f);
            TIFFSetField(tif, TIFFTAG_COMPRESSION, uint16(COMPRESSION_LZW));
            TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, uint16(1));
            TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, uint16(8*sizeof(float)));
            TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, uint16(PHOTOMETRIC_MINISBLACK));
            TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, uint16(SAMPLEFORMAT_IEEEFP));
            TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, 0));

            // Write image data
            rowSize = m_opts.xRes*sizeof(float);
            boost::scoped_array<uint8> lineBuf(new uint8[rowStride]);
            for(int line = 0; line < m_opts.yRes; ++line)
            {
                std::memcpy(lineBuf.get(), m_image + line*m_opts.xRes, rowSize);
                TIFFWriteScanline(tif, reinterpret_cast<tdata_t>(lineBuf.get()),
                                  uint32(line));
            }

            TIFFClose(tif);
        }

    public:
        Renderer(const Options& opts)
            : m_opts(opts),
            m_samples()
        { }

        // Push a surface onto the render queue
        void push(boost::shared_ptr<Surface>& surface, int splitCount = 0)
        {
            if(splitCount > m_opts.maxSplits)
                std::cerr << "max splits; surface discarded\n";
            else
                m_surfaces.push(SurfaceHolder(surface, splitCount, surface.bound()));
        }

        void push(boost::shared_ptr<Grid>& grid)
        {
            // Project grid into raster coordinates
            rasterize(*grid);
        }

        void render()
        {
            initSamples();

            while(!m_surfaces.empty())
            {
                SurfaceHolder s = m_surfaces.top();
                m_surfaces.pop();
                RenderQueue queue(*this, s.splitCount);
                s.surface.splitdice(m_camToRas, queue);
            }

            saveImage("test.tif");
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
            // iterate over all micropolys in the grid & render each one.
            for(Grid::Iterator poly = grid.begin(); poly.valid(); ++poly)
            {
                Grid::UPoly poly = *i;

                Box bound = poly.bound();

                // Bounding box for relevant samples, clamped to image extent.
                const int sx = Imath::clamp(Imath::floor(bound.min.x), 0, m_opts.xRes);
                const int ex = Imath::clamp(Imath::floor(bound.max.x)+1, 0, m_opts.yRes);
                const int sy = Imath::clamp(Imath::floor(bound.min.y), 0, m_opts.yRes);
                const int ey = Imath::clamp(Imath::floor(bound.max.y)+1, 0, m_opts.yRes);

                Grid::HitTest hitTest = poly.hitTest();

                // for each sample position in the bound
                for(int ix = sx; ix < ex; ++ix)
                {
                    for(int iy = sx; iy < ex; ++iy)
                    {
                        int idx = m_opts.xRes*iy + ix;
                        Sample& samp = m_samples[idx];
                        // Test whether sample hits the micropoly
                        if(!hitTest(samp))
                            continue;
                        // Generate & store a fragment
                        samp.z = poly.z();
#if 0
                        // Early out if definitely hidden
                        if(samp.z < bound.min.z)
                            continue;
                        float z = 0;
                        // Ignore if sample didn't hit the polygon
                        if(!hitTest(samp, z))
                            continue;
                        // Ignore if sample is hidden
                        if(z < 0 || z > samp.z)
                            continue;
                        // Generate & store a fragment
#endif
                    }
                }
            }
        }
};


#endif // RENDERER_H_INCLUDED
