#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <cstring>
#include <queue>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>

#include "grid.h"
#include "invbilin.h"
#include "options.h"
#include "sample.h"
#include "geometry.h"
#include "util.h"

#include "tiffio.h"

class Renderer;

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


class Renderer
{
    private:
        // RenderQueueImpl is a friend so that it can appropriately push() surfaces
        // and grids into the renderer.
        friend class RenderQueueImpl;

        // Standard container for geometry metadata
        struct SurfaceHolder
        {
            boost::shared_ptr<Geometry> geom; //< Pointer to geometry
            int splitCount; //< Number of times the geometry has been split
            Box bound;      //< Bound in camera coordinates

            SurfaceHolder(const boost::shared_ptr<Geometry>& geom, int splitCount,
                          Box bound)
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
                surface_order() : m_bucketHeight(0.1) {}

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

        Options m_opts;
        SurfaceQueue m_surfaces;
        std::vector<Sample> m_samples;
        std::vector<float> m_image;
        Mat4 m_camToRas;

        void initSamples()
        {
            // Initialize sample array
            m_samples.resize(m_opts.xRes*m_opts.yRes);
            // Initialize sample positions to contain
            for(int j = 0; j < m_opts.yRes; ++j)
            {
                for(int i = 0; i < m_opts.xRes; ++i)
                    m_samples[j*m_opts.xRes + i] = Sample(Vec2(i+0.5f, j+0.5f));
            }
            // Initialize image array
            m_image.resize(m_opts.xRes*m_opts.yRes, FLT_MAX);
        }

        // Save image to a TIFF file.
        void saveImage(const std::string& fileName)
        {
            TIFF* tif = TIFFOpen(fileName.c_str(), "w");
            if(!tif)
            {
                std::cerr << "Could not open file!\n";
                return;
            }

            // Write header
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
            int rowSize = m_opts.xRes*sizeof(float);
            boost::scoped_array<uint8> lineBuf(new uint8[rowSize]);
            for(int line = 0; line < m_opts.yRes; ++line)
            {
                std::memcpy(lineBuf.get(), &m_image[0] + line*m_opts.xRes,
                            rowSize);
                TIFFWriteScanline(tif, reinterpret_cast<tdata_t>(lineBuf.get()),
                                  uint32(line));
            }

            TIFFClose(tif);
        }

        // Push geometry into the render queue
        void push(const boost::shared_ptr<Geometry>& geom, int splitCount)
        {
            Box bound = geom->bound();
            if(bound.min.z < FLT_EPSILON && splitCount > m_opts.maxSplits)
            {
                std::cerr << "Max eye splits encountered; geometry discarded\n";
                return;
            }
            if(bound.max.z < m_opts.clipNear || bound.min.z > m_opts.clipFar)
                return;
            // TODO: Discard geometry if outside of image.
            m_surfaces.push(SurfaceHolder(geom, splitCount, bound));
        }

        // Push a grid onto the render queue
        void push(const boost::shared_ptr<Grid>& grid)
        {
            rasterize(*grid);
        }

    public:
        Renderer(const Options& opts, const Mat4& camToScreen = Mat4())
            : m_opts(opts),
            m_surfaces(),
            m_samples(),
            m_image(),
            m_camToRas()
        {
            // Set up camera -> raster matrix
            m_camToRas = camToScreen
                * Mat4().setScale(Vec3(0.5,-0.5,0))
                * Mat4().setTranslation(Vec3(0.5,0.5,0))
                * Mat4().setScale(Vec3(m_opts.xRes, m_opts.yRes, 1));
        }

        // Add geometry
        void add(const boost::shared_ptr<Geometry>& geom)
        {
            // TODO: Transform to camera space?
            push(geom, 0);
        }

        // Render all surfaces and save resulting image.
        void render()
        {
            // Splitting transform.  Allowing this to be different from the
            // projection matrix lets us examine a fixed set of grids
            // independently of the viewpoint.
            Mat4 splitTrans = m_camToRas;
//                  Mat4().setScale(Vec3(0.5,-0.5,0))
//                * Mat4().setTranslation(Vec3(0.5,0.5,0))
//                * Mat4().setScale(Vec3(m_opts.xRes, m_opts.yRes, 1));

            initSamples();
            while(!m_surfaces.empty())
            {
                SurfaceHolder s = m_surfaces.top();
                m_surfaces.pop();
                RenderQueueImpl queue(*this, s.splitCount);
                s.geom->splitdice(splitTrans, queue);
            }
            saveImage("test.tif");
        }

        // Render a grid by rasterizing each micropolygon.
//        __attribute__((flatten))
        void rasterize(Grid& grid)
        {
            // Project grid into raster coordinates.
            grid.project(m_camToRas);
            // iterate over all micropolys in the grid & render each one.
            for(Grid::Iterator i = grid.begin(); i.valid(); ++i)
            {
//                if(i.u() != 0 && i.v() != 0)
//                    continue;
                Grid::UPoly poly = *i;

                Box bound = poly.bound();

                // Bounding box for relevant samples, clamped to image extent.
                const int sx = Imath::clamp(Imath::floor(bound.min.x), 0, m_opts.xRes);
                const int ex = Imath::clamp(Imath::floor(bound.max.x)+1, 0, m_opts.xRes);
                const int sy = Imath::clamp(Imath::floor(bound.min.y), 0, m_opts.yRes);
                const int ey = Imath::clamp(Imath::floor(bound.max.y)+1, 0, m_opts.yRes);

                Grid::HitTest hitTest = poly.hitTest();
                InvBilin invBilin;
                if(m_opts.smoothShading)
                {
                    invBilin.init(vec2_cast(poly.a()), vec2_cast(poly.b()),
                                  vec2_cast(poly.d()), vec2_cast(poly.c()));
                }

                // for each sample position in the bound
                for(int ix = sx; ix < ex; ++ix)
                {
                    for(int iy = sy; iy < ey; ++iy)
                    {
                        int idx = m_opts.xRes*iy + ix;
                        Sample& samp = m_samples[idx];
//                        // Early out if definitely hidden
//                        if(samp.z < bound.min.z)
//                            continue;
                        // Test whether sample hits the micropoly
                        if(!hitTest(samp))
                            continue;
                        // Determine hit depth
                        // Generate & store a fragment
                        //
                        // TODO: Abstract the smooth/constant shading handling
                        // out of here & onto the grid or micropoly.
                        float z;
                        if(m_opts.smoothShading)
                        {
                            Vec2 uv = invBilin(samp.p);
                            z = bilerp(poly.a().z, poly.b().z,
                                       poly.d().z, poly.c().z, uv);
                        }
                        else
                        {
                            // constant shading
                            z = poly.a().z;
                        }
                        if(samp.z < z)
                        {
                            // Ignore if hit is hidden
                            continue;
                        }
                        samp.z = z;
                        m_image[idx] = z;
                    }
                }
            }
        }
};


//==============================================================================

void RenderQueueImpl::push(const boost::shared_ptr<Geometry>& geom)
{
    m_renderer.push(geom, m_splitDepth+1);
}
void RenderQueueImpl::push(const boost::shared_ptr<Grid>& grid)
{
    m_renderer.push(grid);
}

#endif // RENDERER_H_INCLUDED
