#include <cassert>
#include <cfloat>
#include <cmath>
#include <vector>

#include <tiffio.h>

#include <boost/functional/hash.hpp>
#include <boost/math/special_functions/sinc.hpp>
#include <boost/scoped_array.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include <OpenEXR/ImathVec.h>

#include "../refcount.h"
//#include "omp.h"

// A define to disable inlining so that selected functions show up in the
// profile.
#define noinline __attribute__((noinline))

// Compute ceil(real(n)/d) using integers for positive n and d.
template<typename T>
inline T ceildiv(T n, T d)
{
    return (n-1)/d + 1;
}

template<typename T>
inline T clamp(T x, T low, T high)
{
    return (x < low) ? low : ((x > high) ? high : x);
}


float windowedsinc(float x, float width)
{
    float xscale = x*M_PI;
    float window = 0;
    if(std::abs(x) < width)
        window = boost::math::sinc_pi(xscale/width);
    return boost::math::sinc_pi(xscale)*window;
}

/// Cache filter coefficients
void cacheFilter(std::vector<float>& filter, int filterWidth, int superSamp)
{
    int fw = filterWidth*superSamp;
    filter.resize(fw*fw, FLT_MAX);
    // compute filter coefficients
    for(int j = 0; j < fw; ++j)
    {
        float y = (j+0.5)/superSamp - filterWidth/2.0;
        for(int i = 0; i < fw; ++i)
        {
            float x = (i+0.5)/superSamp - filterWidth/2.0;
            filter[j*fw + i] = windowedsinc(x, filterWidth) *
                               windowedsinc(y, filterWidth);
        }
    }
    // normalize
    float sum = 0;
    for(int i = 0, iend=filter.size(); i < iend; ++i)
        sum += filter[i];
    float renorm = 1/sum;
    for(int i = 0, iend=filter.size(); i < iend; ++i)
        filter[i] *= renorm;

    // Debug; print filter coeffs
//    for(int j = 0; j < fw; ++j)
//    {
//        for(int i = 0; i < fw; ++i)
//            std::cout << filter[fw*j + i] << " ";
//        std::cout << "\n";
//    }
}

//------------------------------------------------------------------------------
// Fractal stuff.
int mandelEscapetime(double cx, double cy, const int maxIter)
{
    int i = 0;
    double x = 0;
    double y = 0;
    while(x*x + y*y < 4 && i < maxIter)
    {
        double xtmp = x*x - y*y + cx;
        y = 2*x*y + cy;
        x = xtmp;
        ++i;
    }
    return i;
}


void colorMap(float* rgb, int i, int maxIter)
{
    if(i == maxIter)
        rgb[0] = rgb[1] = rgb[2] = 0;
    else
    {
        rgb[0] = (i%2048)/2047.0;
        rgb[1] = 0;
        rgb[2] = clamp(i, 0, 255)/255.0;
    }
}

void mandelColor(float* rgb, double x, double y, int maxIter)
{
    int t = mandelEscapetime(x, y, maxIter);
    colorMap(rgb, t, maxIter);
}

struct Transform
{
    double dx;
    double x0;
    double dy;
    double y0;

    Transform offset(int x, int y) const
    {
        Transform t = *this;
        t.x0 += dx*x;
        t.y0 += dy*y;
        return t;
    }
};


noinline
void renderTile(float* rgb, int w, const Transform& trans,
                int maxIter)
{
    for(int j = 0; j < w; ++j)
    {
        double y = trans.y0 + j*trans.dy;
        for(int i = 0; i < w; ++i)
        {
            double x = trans.x0 + i*trans.dx;
            mandelColor(&rgb[3*(w*j + i)], x, y, maxIter);
        }
    }
}


//------------------------------------------------------------------------------
// Here's a diagram of how the filtering works.
//
//  A--------------------B--------------------+
//  |                    |                    |
//  |                    |                    |
//  |                    |                    |
//  |      .   .   .   . | .   .   .   .      |
//  |                    |                    |
//  |      .  P.....................Q  .      |
//  |         .          |          .         |
//  |      .  .          |          .  .      |
//  |         .          |          .         |
//  |      .  .          |          .  .      |
//  |         .          |          .         |
//  D---------.----------C----------.---------+
//  |         .          |          .         |
//  |      .  .          |          .  .      |
//  |         .          |          .         |
//  |      .  .          |          .  .      |
//  |         .          |          .         |
//  |      .  S.....................R  .      |
//  |                    |                    |
//  |      .   .   .   . | .   .   .   .      |
//  |                    |                    |
//  |                    |                    |
//  |                    |                    |
//  +--------------------+--------------------+
//
//  ----- ABCD is a sample tile
//  ..... PQRS is a filter region
//  .   . is the filter region expanded by the filter radius
//
noinline
void filterAndQuantizeTile(uint8* rgbOut, const float* rgbTiles[2][2],
                           int tileWidth, int superSamp, const float* filter,
                           int filterWidth)
{
    const int fw = filterWidth*superSamp;
    const int widthOn2 = tileWidth/2;
    const int sampTileWidth = superSamp*tileWidth; // Input tile size before filtering
    assert(tileWidth % 2 == 0);
    assert(filterWidth < widthOn2*superSamp);
    // Iterate over output tile
    for(int iy = 0; iy < tileWidth; ++iy)
        for(int ix = 0; ix < tileWidth; ++ix)
        {
            // Filter pixel.  The filter support can lie across any or all of
            // the 2x2 block of input tiles.
            float col[3] = {0,0,0};
            for(int j = 0; j < fw; ++j)
                for(int i = 0; i < fw; ++i)
                {
                    // location x,y of the current pixel in the 2x2 block
                    int x = superSamp*(ix + widthOn2) + i;
                    int y = superSamp*(iy + widthOn2) + j;
                    int tx = x >= sampTileWidth;
                    int ty = y >= sampTileWidth;
                    const float* c = rgbTiles[ty][tx] +
                           3*( sampTileWidth*(y - ty*sampTileWidth) +
                                           (x - tx*sampTileWidth) );
                    float w = filter[fw*j + i];
                    col[0] += w*c[0];
                    col[1] += w*c[1];
                    col[2] += w*c[2];
                }
//            int xb = superSamp*(ix + widthOn2);
//            int xe = xb + fw;
//            int yb = superSamp*(iy + widthOn2);
//            int ye = yb + fw;
//            if(    (xb < sampTileWidth && xe >= sampTileWidth)
//                || (yb < sampTileWidth && ye >= sampTileWidth) )
//            {
//                col[0] = 0;
//            }
//            else
//            {
//                const float* rgb = rgbTiles[yb >= sampTileWidth][xb >= sampTileWidth];
//                if(xb >= sampTileWidth)
//                    xb -= sampTileWidth;
//                if(yb >= sampTileWidth)
//                    yb -= sampTileWidth;
//                for(int j = 0, y = yb; j < fw; ++j, ++y)
//                {
//                    for(int i = 0, x = xb; i < fw; ++i, ++x)
//                    {
//                        const float* c = rgb + 3*(sampTileWidth*y + x);
//                        float w = filter[fw*j + i];
//                        col[0] += w*c[0];
//                        col[1] += w*c[1];
//                        col[2] += w*c[2];
//                    }
//                }
//            }
            // Quantize & store in output Tile
            int idx = 3*(tileWidth*iy + ix);
            rgbOut[idx+0] = uint8(clamp(255*col[0], 0.0f, 255.0f));
            rgbOut[idx+1] = uint8(clamp(255*col[1], 0.0f, 255.0f));
            rgbOut[idx+2] = uint8(clamp(255*col[2], 0.0f, 255.0f));
        }
}

//------------------------------------------------------------------------------
/// Write required TIFF header data
void writeHeader(TIFF* tif, int width, int height, int tileWidth = -1)
{
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, uint32(width));
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, uint32(height));
    TIFFSetField(tif, TIFFTAG_ORIENTATION, uint16(ORIENTATION_TOPLEFT));
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, uint16(PLANARCONFIG_CONTIG));
    TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, uint16(RESUNIT_NONE));
    TIFFSetField(tif, TIFFTAG_XRESOLUTION, 1.0f);
    TIFFSetField(tif, TIFFTAG_YRESOLUTION, 1.0f);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, uint16(COMPRESSION_LZW));
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, uint16(3));
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, uint16(8));
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, 0));

    if(tileWidth > 0)
    {
        TIFFSetField(tif, TIFFTAG_TILEWIDTH, uint32(tileWidth));
        TIFFSetField(tif, TIFFTAG_TILELENGTH, uint32(tileWidth));
    }
}


//------------------------------------------------------------------------------
/// Simple sequential method.
///
/// Render a big buffer of samples, then filter those samples.
noinline
void renderImageSimple(TIFF* tif, int width, int superSamp, const float* filter,
                       int filterWidth, const Transform& trans, int maxIter)
{
    writeHeader(tif, width, width);
    int fullWidth = superSamp*(width + filterWidth-1);
    std::vector<float> colVals(3*fullWidth*fullWidth, -1);
    renderTile(&colVals[0], fullWidth, trans, maxIter);

    std::vector<uint8> colFilt(width*3, 0);

    const int fw = filterWidth*superSamp;
    for(int iy = 0; iy < width; ++iy)
    {
        for(int ix = 0; ix < width; ++ix)
        {
            // Filter pixel
            float col[3] = {0,0,0};
            for(int j = 0; j < fw; ++j)
                for(int i = 0; i < fw; ++i)
                {
                    float* c = &colVals[3*(fullWidth*(superSamp*iy+j) + superSamp*ix+i)];
                    float w = filter[fw*j + i];
                    col[0] += w*c[0];
                    col[1] += w*c[1];
                    col[2] += w*c[2];
                }
            // Quantize & store in scanline
            colFilt[3*ix+0] = uint8(clamp(255*col[0], 0.0f, 255.0f));
            colFilt[3*ix+1] = uint8(clamp(255*col[1], 0.0f, 255.0f));
            colFilt[3*ix+2] = uint8(clamp(255*col[2], 0.0f, 255.0f));
        }
        // Save result
        TIFFWriteScanline(tif, &colFilt[0], iy);
    }
}


//------------------------------------------------------------------------------
/// Tiled sequential method.
///
/// Render samples in tiled chunks.  Filtering is interleaved (in time) with
/// sampling, and happens whenever sufficient adjacent tiles have been
/// generated.
///
/// For simplicity full tiles of samples are always generated, though in
/// practise that means there's some sampling wastage at the edges of the
/// image, which means this function will be slightly slower than
/// renderImageSimple().
noinline
void renderImageTiled(TIFF* tif, int width, int superSamp, const float* filter,
                      int filterWidth, const Transform& trans, int maxIter)
{
    const int tileWidth = 16;
    writeHeader(tif, width, width, tileWidth);
    int ntiles = ceildiv(width, tileWidth) + 1;

    int sampTileWidth = superSamp*tileWidth;
    int tileSize = sampTileWidth*sampTileWidth*3;

    // Storage pool for rendered tiles
    int poolSize = ntiles+2;
    std::vector<float> tileStorage(poolSize*tileSize);
    std::vector<float*> tilePool(poolSize);
    for(int i = 0; i < poolSize; ++i)
        tilePool[i] = &tileStorage[i*tileSize];

    // Render tiles left to right, top to bottom.
    int poolPos = 0;
    std::vector<uint8> colFilt(3*tileWidth*tileWidth, 0);
    for(int ty = 0; ty < ntiles; ++ty)
    {
        for(int tx = 0; tx < ntiles; ++tx, ++poolPos)
        {
            float* stor = tilePool[poolPos % poolSize];
            renderTile(stor, sampTileWidth, trans.offset(sampTileWidth*tx,
                       sampTileWidth*ty), maxIter);
            if(ty > 0 && tx > 0)
            {
                // When four adjacent tiles are rendered, filter the interior
                // region, equal in size to one tile.
                const float* toFilter[2][2] = {
                    {tilePool[(poolPos+1) % poolSize], tilePool[(poolPos+2) % poolSize]},
                    {tilePool[(poolPos-1) % poolSize], stor}
                };
                filterAndQuantizeTile(&colFilt[0], toFilter, tileWidth, superSamp,
                                      &filter[0], filterWidth);
                TIFFWriteTile(tif, &colFilt[0], (tx-1)*tileWidth,
                              (ty-1)*tileWidth, 0, 0);
            }
        }
    }
}


//------------------------------------------------------------------------------
// Parallel tiled implementation.

/// Tile containing sampled image color data
struct SampTile : public RefCounted
{
    private:
        Imath::V2i m_pos;
        boost::scoped_array<float> m_samps;
    public:
        SampTile(int x, int y, int width)
            : m_pos(x,y),
            m_samps(new float[3*width*width])
        { }

        float* samps() { return m_samps.get(); }
        const float* samps() const { return m_samps.get(); }

        const Imath::V2i& pos() const { return m_pos; }
};
typedef boost::intrusive_ptr<SampTile> SampTilePtr;


/// Holder for a 2x2 block of tiles waiting to be filtered
struct TileFilterBlock
{
    SampTilePtr tiles[2][2];

    bool readyForFilter() { return tiles[0][0] && tiles[0][1]
                                && tiles[1][0] && tiles[1][1]; }
};


namespace Imath // namespace needed to allow ADL to find this :-(
{
/// Hash function for 2D points, copied from boost docs.
std::size_t hash_value(Imath::V2i const& p)
{
    std::size_t seed = 0;
    boost::hash_combine(seed, p.x);
    boost::hash_combine(seed, p.y);
    return seed;
}
}


/// Parallel tiled method using OpenMP.
noinline
void renderImageParallelOmp(TIFF* tif, int width, int superSamp,
                            const float* filter, int filterWidth,
                            const Transform& trans, int maxIter)
{
    const int tileWidth = 16;
    writeHeader(tif, width, width, tileWidth);
    // number of filtered tiles.
    int nftiles = ceildiv(width, tileWidth);
    // number of sample tiles
    int ntiles = nftiles + 1;

    int sampTileWidth = superSamp*tileWidth;

    typedef boost::unordered_map<Imath::V2i, TileFilterBlock> FilterBlockContainer;
    FilterBlockContainer waitingBlocks(2*nftiles);

    // Render tiles left to right, top to bottom.
#   pragma omp parallel
    {
    std::vector<uint8> colFilt(3*tileWidth*tileWidth, 0);
    for(int ty = 0; ty < ntiles; ++ty)
    {
        // The options here are very important for correct load balancing,
        // since the amount of work for a given tx,ty varies unpredictably.
        //
        // schedule(dynamic,1) tells the scheduler to give each thread
        // _single_ values of tx (chunks of length 1) to work on at a time.
        //
        // nowait tells the scheduler that no thread synchronization is
        // required at the end of the tx loop.
#       pragma omp for schedule(dynamic,1) nowait
        for(int tx = 0; tx < ntiles; ++tx)
        {
            SampTilePtr tile = new SampTile(tx, ty, sampTileWidth);
            float* stor = tile->samps();
            renderTile(stor, sampTileWidth, trans.offset(sampTileWidth*tx,
                       sampTileWidth*ty), maxIter);
            // The sample tile with coordinates (tx,ty) overlaps the
            // four filtering tiles with coordinates:
            //
            //   (tx-1, ty-1)   (tx,ty-1)
            //   (tx-1, ty)     (tx,ty)
            //
            // we insert the sample tile into each of these filtering tiles,
            // ignoring filtering tile coordinates which lie outside the image
            // boundaries.
            for(int j = 0; j < 2; ++j)
            for(int i = 0; i < 2; ++i)
            {
                Imath::V2i p(tx+i-1, ty+j-1);
                // Ignore filtering tiles outside image boundaries
                if(p.x >= 0 && p.y >= 0 && p.x < nftiles && p.y < nftiles)
                {
                    TileFilterBlock block;
#                   pragma omp critical
                    {
                        FilterBlockContainer::iterator blockIt = waitingBlocks.find(p);
                        if(blockIt == waitingBlocks.end())
                        {
                            // Create blank block if it didn't exist
                            std::pair<FilterBlockContainer::iterator, bool> insRes =
                                waitingBlocks.insert(std::make_pair(p, TileFilterBlock()));
                            assert(insRes.second);
                            blockIt = insRes.first;
                        }
                        blockIt->second.tiles[1-j][1-i] = tile;
                        if(blockIt->second.readyForFilter())
                        {
                            block = blockIt->second;
                            waitingBlocks.erase(blockIt);
                        }
                    }
                    if(block.readyForFilter())
                    {
                        // Filter, quantize & save result.  This can be done in parallel.
                        const float* toFilter[2][2] = {
                            {block.tiles[0][0]->samps(), block.tiles[0][1]->samps()},
                            {block.tiles[1][0]->samps(), block.tiles[1][1]->samps()},
                        };
                        filterAndQuantizeTile(&colFilt[0], toFilter, tileWidth,
                                              superSamp, filter, filterWidth);
#                       pragma omp critical
                        TIFFWriteTile(tif, &colFilt[0], p.x*tileWidth,
                                      p.y*tileWidth, 0, 0);
                    }
                }
            }
        }
    }
    }
    assert(waitingBlocks.size() == 0);
}


//------------------------------------------------------------------------------

/// Collate sample tiles and filter them when ready
class TileCollator
{
    public:
        class SharedData
        {
            private:
                typedef boost::unordered_map<Imath::V2i, TileFilterBlock> BlockContainer;
                BlockContainer waitingBlocks;
                boost::mutex waitingBlocksMutex;
                int nfilterTiles;
                int tileWidth;
                int superSamp;
                // Filter stuff
                const float* filter;
                int filterWidth;
                // Output
                TIFF* outFile;
                boost::mutex tifMutex;
                friend class TileCollator;
            public:
                SharedData(int ntiles, int tileWidth, int superSamp,
                        const float* filter, int filterWidth, TIFF* outFile)
                    : waitingBlocks(2*ntiles),
                    nfilterTiles(ntiles-1),
                    tileWidth(tileWidth),
                    superSamp(superSamp),
                    filter(filter),
                    filterWidth(filterWidth),
                    outFile(outFile)
                { }
        };

    private:
        SharedData& m_;
        std::vector<uint8> m_colFilt;

    public:
        TileCollator(SharedData& sharedData)
            : m_(sharedData),
            m_colFilt(3*m_.tileWidth*m_.tileWidth)
        { }

        void push(const SampTilePtr& tile)
        {
            // The _sample_ tile with coordinates (tx,ty) overlaps the four
            // _filtering_ tiles with coordinates:
            //
            //   (tx-1, ty-1)   (tx,ty-1)
            //   (tx-1, ty)     (tx,ty)
            //
            // we insert the sample tile into each of these filtering tiles,
            // ignoring filtering tile coordinates which lie outside the image
            // boundaries.
            for(int j = 0; j < 2; ++j)
            for(int i = 0; i < 2; ++i)
            {
                // Position of filter tile
                Imath::V2i p = tile->pos() + Imath::V2i(i-1,j-1);
                // Ignore filtering tiles outside image boundaries
                if(p.x >= 0 && p.y >= 0 && p.x < m_.nfilterTiles &&
                                           p.y < m_.nfilterTiles)
                {
                    TileFilterBlock block;
                    {
                        boost::lock_guard<boost::mutex> lock(m_.waitingBlocksMutex);
                        SharedData::BlockContainer::iterator blockIt =
                                m_.waitingBlocks.find(p);
                        if(blockIt == m_.waitingBlocks.end())
                        {
                            // Create blank block if it didn't exist
                            std::pair<SharedData::BlockContainer::iterator, bool> insRes =
                                m_.waitingBlocks.insert(std::make_pair(p, TileFilterBlock()));
                            assert(insRes.second);
                            blockIt = insRes.first;
                        }
                        blockIt->second.tiles[1-j][1-i] = tile;
                        if(blockIt->second.readyForFilter())
                        {
                            block = blockIt->second;
                            m_.waitingBlocks.erase(blockIt);
                        }
                    }
                    if(block.readyForFilter())
                    {
                        // Filter, quantize & save result.  This can be done in parallel.
                        const float* toFilter[2][2] = {
                            {block.tiles[0][0]->samps(), block.tiles[0][1]->samps()},
                            {block.tiles[1][0]->samps(), block.tiles[1][1]->samps()},
                        };
                        uint8* output = &m_colFilt[0];
                        filterAndQuantizeTile(output, toFilter, m_.tileWidth,
                                              m_.superSamp, m_.filter, m_.filterWidth);
                        boost::lock_guard<boost::mutex> lock(m_.tifMutex);
                        TIFFWriteTile(m_.outFile, output, p.x*m_.tileWidth,
                                      p.y*m_.tileWidth, 0, 0);
                    }
                }
            }
        }
};

/// Define the tile ordering for sample tile rendering
class TileScheduler
{
    private:
        boost::mutex m_mutex;
        int m_ntiles;
        int m_tx;
        int m_ty;

    public:
        TileScheduler(int ntiles)
            : m_ntiles(ntiles),
            m_tx(0),
            m_ty(0)
        {}

        bool nextTile(int& tx, int& ty)
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);
            if(m_ty >= m_ntiles)
                return false;
            tx = m_tx;
            ty = m_ty;
            ++m_tx;
            if(m_tx >= m_ntiles)
            {
                ++m_ty;
                m_tx = 0;
            }
            return true;
        }
};


/// Render thread functor for use with boost.thread
class RenderThreadFunc
{
    private:
        TileScheduler& m_scheduler;
        TileCollator::SharedData& m_collatorShared;
        int m_sampTileWidth;
        Transform m_trans;
        int m_maxIter;

    public:
        RenderThreadFunc(TileScheduler& scheduler,
                         TileCollator::SharedData& collatorShared,
                         int sampTileWidth, const Transform& trans,
                         int maxIter)
            : m_scheduler(scheduler),
            m_collatorShared(collatorShared),
            m_sampTileWidth(sampTileWidth),
            m_trans(trans),
            m_maxIter(maxIter)
        { }

        void operator()()
        {
            int tx = 0, ty = 0;
            TileCollator collator(m_collatorShared);
            while(m_scheduler.nextTile(tx, ty))
            {
                SampTilePtr tile = new SampTile(tx, ty, m_sampTileWidth);
                renderTile(tile->samps(), m_sampTileWidth,
                           m_trans.offset(m_sampTileWidth*tx,
                                          m_sampTileWidth*ty), m_maxIter);
                collator.push(tile);
            }
        }
};


/// Parallel tiled method using boost.thread
noinline
void renderImageParallel(TIFF* tif, int width, int superSamp, const float* filter,
                         int filterWidth, const Transform& trans, int maxIter)
{
    const int tileWidth = 16;
    writeHeader(tif, width, width, tileWidth);
    // number of sample tiles over width of image
    int ntiles = ceildiv(width, tileWidth) + 1;

    TileScheduler scheduler(ntiles);
    TileCollator::SharedData collatorShared(ntiles, tileWidth, superSamp,
                                            filter, filterWidth, tif);
    RenderThreadFunc threadFunc(scheduler, collatorShared,
                                superSamp*tileWidth, trans, maxIter);

    int nthreads = boost::thread::hardware_concurrency();
    if(nthreads > 1)
    {
        boost::thread_group threads;
        for(int i = 0; i < nthreads; ++i)
            threads.create_thread(threadFunc);
        threads.join_all();
    }
    else
        threadFunc();
}



//------------------------------------------------------------------------------
int main()
{
    const int width = 800;
    const int maxIter = 2000;

    const int superSamp = 3;

    double scale = 0.01;
    Transform trans;
    trans.dx = scale/superSamp * 2.0/width;
    trans.x0 =  -0.79 + scale * -1.0;
    trans.dy = -scale/superSamp * 2.0/width;
    trans.y0 =  0.15 + scale * 1.0;

    const int filterWidth = 3;
    std::vector<float> filter;
    cacheFilter(filter, filterWidth, superSamp);

    TIFF* tif = TIFFOpen("mandel.tif", "w");

//    renderImageSimple(tif, width, superSamp, &filter[0], filterWidth,
//                      trans, maxIter);
//    renderImageTiled(tif, width, superSamp, &filter[0], filterWidth,
//                     trans, maxIter);
//    renderImageParallelOmp(tif, width, superSamp, &filter[0], filterWidth,
//                           trans, maxIter);
    renderImageParallel(tif, width, superSamp, &filter[0], filterWidth,
                        trans, maxIter);

    TIFFClose(tif);

    return 0;
}

// Link with -ltiff
