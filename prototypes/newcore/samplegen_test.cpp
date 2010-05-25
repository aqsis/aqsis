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

#include "samplegen.h"
#include "util.h"

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

BOOST_AUTO_TEST_CASE(findRemainingInds_test)
{
    const int width = 20;
    const int nsamps = width*width;
    std::vector<int> tile(nsamps, -1);
    tile[0] = 1;
    tile[10] = 2;
    std::vector<int> toPlace(nsamps, -1);
    std::vector<int> remainingInds(nsamps, -1);
    int nToPlace = 0;
    findRemainingInds(&tile[0], width, width, &toPlace[0],
                      nToPlace, &remainingInds[0]);
    BOOST_CHECK_EQUAL(nToPlace, nsamps-2);

    toPlace.resize(nToPlace);
    remainingInds.resize(nToPlace);
    // Check that we the positions 0 & 10 are marked as taken.
    BOOST_CHECK(std::find(toPlace.begin(), toPlace.end(), 0) == toPlace.end());
    BOOST_CHECK(std::find(toPlace.begin(), toPlace.end(), 10) == toPlace.end());
    // Check that position 1 & last is present
    BOOST_CHECK(std::find(toPlace.begin(), toPlace.end(), 1) != toPlace.end());
    BOOST_CHECK(std::find(toPlace.begin(), toPlace.end(), nsamps-1) != toPlace.end());

    // Check that remaining inds doesn't contain indices 1 or 2
    BOOST_CHECK(std::find(remainingInds.begin(), remainingInds.end(), 1)
                == remainingInds.end());
    BOOST_CHECK(std::find(remainingInds.begin(), remainingInds.end(), 2)
                == remainingInds.end());
    // Check that remaining inds contains some other indices...
    BOOST_CHECK(std::find(remainingInds.begin(), remainingInds.end(), nsamps-1)
                != remainingInds.end());
}


BOOST_AUTO_TEST_CASE(findRemainingIndsWithBdry_test)
{
    const int width = 19;
    const int r = 2;
    const int nsamps = width*width;
    std::vector<int> tile(nsamps, -1);
    tile[0] = 1;  // in boundary layer, should be ignored.
    tile[2*width+4] = 2; // in interior
    std::vector<int> toPlace(nsamps, -1);
    std::vector<int> remainingInds(nsamps, -1);
    int nToPlace = 0;
    findRemainingInds(&tile[0], width, width-2*r, &toPlace[0],
                      nToPlace, &remainingInds[0]);
    BOOST_CHECK_EQUAL(nToPlace, (width-2*r)*(width-2*r) - 1);
    // Boundary shouldn't be present
    BOOST_CHECK(std::find(toPlace.begin(), toPlace.end(), width/2) ==
                toPlace.end());
    // Interior points should be present
    BOOST_CHECK(std::find(toPlace.begin(), toPlace.end(), 4*width+4) !=
                toPlace.end());
    // Except those which are specially filled.
    BOOST_CHECK(std::find(toPlace.begin(), toPlace.end(), 2*width+4) ==
                toPlace.end());
    // Ensure pre-filled boundary stuff is ignored.
    BOOST_CHECK(std::find(remainingInds.begin(), remainingInds.end(), 1)
                != remainingInds.end());
    BOOST_CHECK(std::find(remainingInds.begin(), remainingInds.end(), 2)
                == remainingInds.end());
}


BOOST_AUTO_TEST_CASE(getMinCostIdx_test)
{
    float tuv[] = {0,0,0,  1,0,0,  2,0,0,  3,0,0,  4,0,0,  5,0,0,  6,0,0};
    int remainingInds[] = {2,3,4};
    int nremain = array_len(remainingInds);
    int validNeighbours[] = {0,1,2,6};
    float validWeights[] = {1,1,1,1};
    int nvalid = array_len(validNeighbours);
    BOOST_CHECK_EQUAL(2, 
        getMinCostIdx(remainingInds, nremain, validNeighbours,
                      validWeights, nvalid, tuv, 0.5));

    validWeights[3] = 1000;
    BOOST_CHECK_EQUAL(1, 
        getMinCostIdx(remainingInds, nremain, validNeighbours,
                      validWeights, nvalid, tuv, 0.5));
}


// Check that the tile contains all sample indices from 1 to width*width.
bool hasAllIndices(const int* tileIn, int width)
{
    const int nsamps = width*width;
    std::vector<int> tile(tileIn, tileIn+nsamps);
    std::sort(tile.begin(), tile.end());
    for(int i = 0; i < nsamps; ++i)
        if(tile[i] != i)
            return false;
    return true;
}


BOOST_AUTO_TEST_CASE(makeTileSet_test)
{
    int width = 14;
    std::vector<float> tuv;
    canonicalTimeLensSamps(tuv, width*width);

    std::vector<int> tiles;
    makeTileSet(tiles, width, tuv);

    // Check that all indices are positive.  Negative indices will occur if
    // the tile generation has forgotten to fill them in.
    for(int i = 0; i < (int)tiles.size(); ++i)
        BOOST_CHECK_GE(tiles[i], 0);

    const int nsamps = width*width;
    for(int itile = 0; itile < 81; ++itile)
        BOOST_CHECK(hasAllIndices(&tiles[itile*nsamps], width));
}

bool makeTileSetHasAllIndices(int width)
{
    std::vector<float> tuv;
    canonicalTimeLensSamps(tuv, width*width);

    std::vector<int> tiles;
    makeTileSet(tiles, width, tuv);

    const int nsamps = width*width;
    for(int itile = 0; itile < 81; ++itile)
        if(!hasAllIndices(&tiles[itile*nsamps], width))
            return false;
    return true;
}

BOOST_AUTO_TEST_CASE(makeTileSet_small_width_test)
{
    // Check that generation of small tiles works ok.
    BOOST_CHECK(makeTileSetHasAllIndices(4));
    BOOST_CHECK(makeTileSetHasAllIndices(5));
    BOOST_CHECK(makeTileSetHasAllIndices(7));
    BOOST_CHECK(makeTileSetHasAllIndices(8));
    BOOST_CHECK(makeTileSetHasAllIndices(9));
}
