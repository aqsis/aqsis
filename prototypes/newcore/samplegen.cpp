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

#include "util.h"
#include <algorithm>
#include <cstring> // for memcpy

void canonicalTimeLensSamps(std::vector<float>& tuv, int nsamps)
{
    tuv.resize(3*nsamps, FLT_MAX);
    for(int i = 0; i < nsamps; ++i)
    {
        tuv[3*i] = radicalInverse(i, 2);
        // The sqrt here ensures that lens samples are uniformly distributed
        // on the unit disk.
        float r = std::sqrt(radicalInverse(i, 3));
        float theta = 2*M_PI*radicalInverse(i, 5);
        tuv[3*i+1] = r*std::cos(theta);
        tuv[3*i+2] = r*std::sin(theta);
    }
}

// Determine which indices into remain to be placed, and which positions in
// the tile they should go.
void findRemainingInds(const int* tile, int fullwidth, int width,
                       int* toPlace, int& nToPlace, int* remainingInds)
{
    int bWidth = (fullwidth - width)/2;
    assert((fullwidth-width)%2 == 0);
    int nInner = width*width;
    for(int i = 0; i < nInner; ++i)
        remainingInds[i] = i;
    nToPlace = 0;
    for(int j = bWidth; j < fullwidth-bWidth; ++j)
    {
        for(int i = bWidth; i < fullwidth-bWidth; ++i)
        {
            int idx = j*fullwidth + i;
            if(tile[idx] == -1)
            {
                // If tile index is not yet filled, add it to list of tile
                // positions to place.
                toPlace[nToPlace++] = idx;
            }
            else
            {
                // Mark index at tile[idx] as alredy used.
                remainingInds[tile[idx]] = -1;
            }
        }
    }
    std::remove(remainingInds, remainingInds+nInner, -1);
}



// Get index such that tuv[remainingInds[index]] is "well separated" from all
// positions contained tuv[validNeighbours].
//
// nremain is the length of remainingInds,
// nvalid is the length of validNeighbours & validWeights,
// validWeights[i] determines the relative contribution of
//                 tuv[validNeighbours[i]] vs tuv[remainingInds[index]] in
//                 the total cost.
// tuv is the time and lens sample positions.
int getMinCostIdx(const int* remainingInds, int nremain,
                  const int* validNeighbours, const float* validWeights,
                  int nvalid, const float* tuv, float timeStratQuality)
{
    int minCostIdx = 0;
    float minCost = FLT_MAX;
    // Use an extra scaling for tWeight of 4=2*2 here by default since du and
    // dv are in [-1,1] which is 2x the range of dt in [0,1].
    float tWeight = 4*timeStratQuality;
    float uvWeight = 1 - timeStratQuality;
    for(int j = 0; j < nremain; ++j)
    {
        // Compute cost function for j'th remaining sample
        float cost = 0;
        int idx1 = remainingInds[j];
        float t = tuv[3*idx1];
        float u = tuv[3*idx1+1];
        float v = tuv[3*idx1+2];
        for(int i = 0; i < nvalid; ++i)
        {
            int idx2 = validNeighbours[i];
            if(idx2 == idx1)
            {
                // Choosing idx1 == idx2 is really bad, discourage this where
                // possible.
                cost = FLT_MAX;
                break;
            }
            float dt = t - tuv[3*idx2];
            float du = u - tuv[3*idx2+1];
            float dv = v - tuv[3*idx2+2];
            float d = std::sqrt(tWeight*dt*dt + uvWeight*(du*du + dv*dv));
            cost += validWeights[i]/d;
        }
        if(cost < minCost)
        {
            minCost = cost;
            minCostIdx = j;
        }
    }
    return minCostIdx;
}


/// Fill a tile with well-distributed sample indices.
///
/// Fill the tile with indices from 0 to width*width-1, such that neighbouring
/// indices i,j from the tile have the property that tuv[i] and tuv[j] are "far
/// away" from each other.
///
/// \param tile - indices to fill
/// \param fullwidth - width of full tile (may include borders which should
///                    remain fixed)
/// \param width - width of the central section to fill
/// \param r - neighbourhood radius
/// \param tuv - time & lens sample positions
void fillTile(int* tile, int fullwidth, int width, int r, const float* tuv,
              float timeStratQuality)
{
    // Determine which indices into tuv remain to be placed, and which
    // positions in the tile they should go.
    int* toPlace = ALLOCA(int, fullwidth*fullwidth);
    int nToPlace = 0;
    int* remainingInds = ALLOCA(int, width*width);
    findRemainingInds(tile, fullwidth, width, toPlace, nToPlace, remainingInds);

    // Randomize the placement order.
    std::random_shuffle(toPlace, toPlace+nToPlace);

    // Compute weights for cost function.
    float falloff = 0.7;
    // Width of neighbourhood
    int nWidth = 2*r + 1;
    int nneighbours = nWidth*nWidth;
    float* weights = FALLOCA(nneighbours);
    for(int j = -r; j <= r; ++j)
        for(int i = -r; i <= r; ++i)
            weights[(j+r)*nWidth + i+r] = std::exp(-falloff*(i*i + j*j));

    // Now, place the "best" possible sample at each position, according to
    // the cost function.
    int* validNeighbours = ALLOCA(int, nneighbours);
    float* validWeights = FALLOCA(nneighbours);
    for(int isamp = 0; isamp < nToPlace; ++isamp)
    {
        // Current sample position inside tile
        int pos = toPlace[isamp];
        int px = pos % fullwidth;
        int py = pos / fullwidth;
        // Find time indices for neighbouring samples which are valid
        int nvalid = 0;
        for(int j = -r; j <= r; ++j)
        {
            int jWrap = j+py;
            if     (jWrap < 0)          jWrap += fullwidth;
            else if(jWrap >= fullwidth) jWrap -= fullwidth;
            for(int i = -r; i <= r; ++i)
            {
                int iWrap = i+px;
                if     (iWrap < 0)          iWrap += fullwidth;
                else if(iWrap >= fullwidth) iWrap -= fullwidth;
                int idx = jWrap*fullwidth + iWrap;
                if(tile[idx] != -1)
                {
                    validNeighbours[nvalid] = tile[idx];
                    validWeights[nvalid] = weights[(j+r)*nWidth + i+r];
                    ++nvalid;
                }
            }
        }
        int nremain = nToPlace - isamp;
        int minCostIdx = 0;
        if(nvalid == 0)
        {
            // If there's no valid neighbouring samples, choose an index at
            // random.
            minCostIdx = rand() % nremain;
        }
        else
        {
            minCostIdx = getMinCostIdx(remainingInds, nremain, validNeighbours,
                                       validWeights, nvalid, tuv,
                                       timeStratQuality);
        }

        // Install selected index into tile
        tile[pos] = remainingInds[minCostIdx];
        // Remove index it from the remaining index list.
        std::copy(remainingInds+minCostIdx+1, remainingInds+nremain,
                  remainingInds+minCostIdx);
    }
}


// Copy a 2D block from one array to another
//
// dest,dwidth - destination array and width
// src,swidth - source array and width
// dx,dy - position of block in dest array
// sx,sy - position of block in src array
// w,h   - width and height of block
static void copyBlock2d(int* dest, int dwidth, const int* src, int swidth,
                        int dx, int dy, int sx, int sy, int w, int h)
{
    assert(dx + w <= dwidth);
    assert(sx + w <= swidth);
    dest += dy*dwidth + dx;
    src  += sy*swidth + sx;
    for(int j = 0; j < h; ++j)
    {
        std::memcpy(dest, src, w*sizeof(int));
        dest += dwidth;
        src += swidth;
    }
}


namespace {
class IndirectLessFunctor
{
    private:
        const int* m_data;
    public:
        IndirectLessFunctor(const int* data) : m_data(data) {}
        bool operator()(int a, int b) const { return m_data[a] < m_data[b]; }
};
}


void makeTileSet(std::vector<int>& tiles, int width, std::vector<float>& tuv,
                 float timeStratQuality)
{
    assert(timeStratQuality >= 0 && timeStratQuality <= 1);
    assert(width >= 4);
    assert((int)tuv.size() == 3*width*width);
    // Radius of region over which to evenly distribute samples
    int r = 2;
    if(width < 8)
        r = 1;
    // Number of tile colors
    const int ncol = 3;

    // Edge tile width
    int eWidth = r + r%2;
    // Corner tile width
    int cWidth = 2*r + eWidth;

    int fullwidth = width + eWidth;
    // Working space for computing tiles.
    std::vector<int> tileStor(fullwidth*fullwidth, -1);
    int* tile = &tileStor[0];

    // First, create corner tiles
    // --------------------------
    int cSize = cWidth*cWidth;
    std::vector<int> cornerStor(cSize*3,-1);
    int* corners[] = { &cornerStor[0],
                       &cornerStor[cSize],
                       &cornerStor[2*cSize] };
    fillTile(tile, width, width, r, get(tuv), timeStratQuality);
    copyBlock2d(corners[0], cWidth, tile, width, 0,0, 0,0, cWidth,cWidth);
    if(2*cWidth <= width)
    {
        // cWidth is small enough that we can cut out two more corner tiles
        // from the original tile
        copyBlock2d(corners[1], cWidth, tile, width, 0,0, width/2,0, cWidth,cWidth);
        copyBlock2d(corners[2], cWidth, tile, width, 0,0, width/2,width/2, cWidth,cWidth);
    }
    else
    {
        // else need to optimize a new tile to cut out each corner, to avoid
        // duplication of sample patterns
        tileStor.assign(tileStor.size(), -1);
        fillTile(tile, width, width, r, get(tuv), timeStratQuality);
        copyBlock2d(corners[1], cWidth, tile, width, 0,0, 0,0, cWidth,cWidth);
        tileStor.assign(tileStor.size(), -1);
        fillTile(tile, width, width, r, get(tuv), timeStratQuality);
        copyBlock2d(corners[2], cWidth, tile, width, 0,0, 0,0, cWidth,cWidth);
    }
    // Overwrite sections of the corner tiles with -1 (the invalid index) This
    // makes sure the corner tiles take up the minimum number of tuv indices
    // possible.
    for(int j = 0; j < cWidth; ++j)
    {
        for(int i = 0; i < cWidth; ++i)
        {
            if((i < r || i >= cWidth-r) && (j < r || j >= cWidth-r))
            {
                corners[0][j*cWidth + i] = -1;
                corners[1][j*cWidth + i] = -1;
                corners[2][j*cWidth + i] = -1;
            }
        }
    }

    // Second, create edge tiles
    // -------------------------
    int eLen = width - cWidth;
    int eSize = eLen*eWidth;
    int nEdges = ncol*ncol; // number of horizontal or vertical edges
    std::vector<int> edgeStor(eSize*2*nEdges,-1);
    std::vector<int*> horizEdges(nEdges);
    std::vector<int*> vertEdges(nEdges);
    for(int iedge = 0; iedge < nEdges; ++iedge)
    {
        horizEdges[iedge] = &edgeStor[eSize*iedge];
        vertEdges[iedge] = &edgeStor[eSize*(iedge+nEdges)];
        // Corner indices for current edges
        int c1 = iedge % ncol;
        int c2 = iedge / ncol;
        // Insert corner samples into top left and top right of tile:
        //
        // +--------------+
        // |C            C|  C = corner
        // |CCeeeeeeeeeeCC|  e = edge to be extracted
        // |CCeeeeeeeeeeCC|
        // |C            C|
        // |              |
        // .              .
        int cw2 = cWidth/2;
        tileStor.assign(tileStor.size(), -1);
        copyBlock2d(tile, width, corners[c1], cWidth,
                    0,0, cw2,0, cw2,cWidth);
        copyBlock2d(tile, width, corners[c2], cWidth,
                    width-cw2,0, 0,0, cw2,cWidth);
        // optimize tile & extract newly optimized edge
        fillTile(tile, width, width, r, get(tuv), timeStratQuality);
        copyBlock2d(horizEdges[iedge], eLen, tile, width,
                    0,0, cw2,r, eLen,eWidth);

        // Now do the same thing for the vertical edges.
        tileStor.assign(tileStor.size(), -1);
        copyBlock2d(tile, width, corners[c1], cWidth,
                    0,0, 0,cw2, cWidth,cw2);
        copyBlock2d(tile, width, corners[c2], cWidth,
                    0,width-cw2, 0,0, cWidth,cw2);
        fillTile(tile, width, width, r, get(tuv), timeStratQuality);
        copyBlock2d(vertEdges[iedge], eWidth, tile, width,
                    0,0, r,cw2, eWidth,eLen);
    }

    // Third, create the full tiles
    // ----------------------------
    // We now have a consistent set of coloured corners and all possible
    // associated edges.  Fill in the tiles themselves using these corners &
    // edges as the boundaries

    // Record tile positions which need to have the indices de-duplicated.
    std::vector<int> deDupInd;
    deDupInd.reserve(2*width*eWidth - eWidth*eWidth);
    int ew2 = eWidth/2;
    for(int j = 0; j < width; ++j)
        for(int i = 0; i < width; ++i)
        {
            if((i < ew2 || i >= width-ew2) || (j < ew2 || j >= width-ew2))
                deDupInd.push_back((j+ew2)*fullwidth + i+ew2);
        }

    int nsamps = width*width;
    int ntiles = ncol*ncol*ncol*ncol;
    tiles.assign(nsamps*ntiles,-1);
    for(int itile = 0; itile < ntiles; ++itile)
    {
        // Fill in tile boundaries to look something like
        //
        // c1              c2
        //    CCCeeeeeeCCC
        //    CCCeeeeeeCCC
        //    CC--------CC
        //    ee--------ee
        //    ee--------ee
        //    CC--------CC
        //    CCCeeeeeeCCC
        //    CCCeeeeeeCCC
        // c3              c4
        //
        // corner indices
        int c1 = itile % ncol;
        int c2 = (itile/ncol) % ncol;
        int c3 = (itile/(ncol*ncol)) % ncol;
        int c4 = itile/(ncol*ncol*ncol);
        // Fill in corners
        tileStor.assign(tileStor.size(), -1);
        int cw = cWidth-r;
        copyBlock2d(tile, fullwidth, corners[c1], cWidth,
                    0,0, r,r, cw,cw);
        copyBlock2d(tile, fullwidth, corners[c2], cWidth,
                    fullwidth-cw,0, 0,r, cw,cw);
        copyBlock2d(tile, fullwidth, corners[c2], cWidth,
                    0,fullwidth-cw, r,0, cw,cw);
        copyBlock2d(tile, fullwidth, corners[c2], cWidth,
                    fullwidth-cw,fullwidth-cw, 0,0, cw,cw);
        // Fill in edges
        copyBlock2d(tile, fullwidth, horizEdges[ncol*c2 + c1], eLen,
                    cw,0, 0,0, eLen,eWidth);
        copyBlock2d(tile, fullwidth, horizEdges[ncol*c4 + c3], eLen,
                    cw,fullwidth-eWidth, 0,0, eLen,eWidth);
        copyBlock2d(tile, fullwidth, vertEdges[ncol*c3 + c1], eWidth,
                    0,cw, 0,0, eWidth,eLen);
        copyBlock2d(tile, fullwidth, vertEdges[ncol*c4 + c2], eWidth,
                    fullwidth-eWidth,cw, 0,0, eWidth,eLen);

        // Remove duplicate indices inside the central width x width region
        std::sort(deDupInd.begin(), deDupInd.end(), IndirectLessFunctor(tile));
        for(int k = 0, kend=deDupInd.size()-1; k < kend; ++k)
        {
            int currSamp = tile[deDupInd[k]];
            if(tile[deDupInd[k+1]] == currSamp)
            {
                // currSamp is a duplicate of the next sample index, choose
                // the next closest sample instead, as measured by the
                // distance in time-lens space.
                float t = tuv[3*currSamp];
                float u = tuv[3*currSamp+1];
                float v = tuv[3*currSamp+2];
                float tWeight = 4*timeStratQuality;
                float uvWeight = 1 - timeStratQuality;
                float minDist2 = FLT_MAX;
                int bestReplacement = -1;
                for(int isamp = 0; isamp < nsamps; ++isamp)
                {
                    if(isamp == currSamp)
                        continue;
                    float dt = t - tuv[3*isamp];
                    float du = u - tuv[3*isamp+1];
                    float dv = v - tuv[3*isamp+2];
                    float dist2 = tWeight*dt*dt + uvWeight*(du*du + dv*dv);
                    if(dist2 < minDist2)
                    {
                        // isamp looks promising out of the ones we've tried
                        // so far, but check that it's not already used before
                        // storing it.
                        bool valid = true;
                        // simple linear search.  This shouldn't be called very
                        // many times.
                        for(int i = 0, iend = deDupInd.size(); i < iend; ++i)
                        {
                            if(tile[deDupInd[i]] == isamp)
                            {
                                valid = false;
                                break;
                            }
                        }
                        if(valid)
                        {
                            bestReplacement = isamp;
                            minDist2 = dist2;
                        }
                    }
                }
                assert(bestReplacement != -1);
                tile[deDupInd[k]] = bestReplacement;
            }
        }

        // Optimize tile & save the result
        fillTile(tile, fullwidth, width, r, get(tuv), timeStratQuality);
        copyBlock2d(&tiles[nsamps*itile], width, tile, fullwidth,
                    0,0, eWidth/2,eWidth/2, width,width);
    }
}


