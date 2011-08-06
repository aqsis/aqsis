// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/// \file Tree array storage stuff
///
/// This is left over from earlier experiments, but might still be useful.

#ifndef AQSIS_TREEARRAYSTORAGE_H_INCLUDED
#define AQSIS_TREEARRAYSTORAGE_H_INCLUDED

#include <algorithm>
#include <cassert>

namespace Aqsis {

//------------------------------------------------------------------------------
// Quad tree utilities for indexing array-based node storage.
//
// We can store a balanced quad tree in an array if we put the root node at 0
// and the children of a node at index n at the indices 2*n+1, 2*n+2, 2*n+3,
// 2*n+4.  There's multiple ways to label the children spatially - here's the
// ordering we use:
//
// Numbers indicate node indices.
//
//             level 0            level 1             level 2
//
//        +---------------+   +-------+-------+   +-------+-------+
//        |               |   |       |       |   | 15| 16| 19| 20|
//   ^    |               |   |   3   |   4   |   |-------|-------|
//   |    |               |   |       |       |   | 13| 14| 17| 18|
// y |    |       0       |   +-------+-------+   +-------+-------+
//   |    |               |   |       |       |   | 7 | 8 | 11| 12|
//   |    |               |   |   1   |   2   |   |-------|-------|
//   |    |               |   |       |       |   | 5 | 6 | 9 | 10|
//   |    +---------------+   +-------+-------+   +-------+-------+
//   |
//   |           x
//  -+------------>
//   |
//
//

/// Get total number of nodes in a balanced quadtree
inline int quadTreeNumNodes(int depth)
{
    // This is just the sum of the number of nodes on each level, which
    // happens to be a geometric series.
    return ((1 << 2*(depth+1)) - 1)/3;
}

/// Get array index of parent node from a given node index.
inline int quadTreeParentNode(int index)
{
    return (index - 1) >> 2;
}

/// Get first sibling of a node from the given node index.
inline int quadTreeFirstSibling(int index)
{
    return ((index-1) & 0xFFFFFFFC) + 1;
}

/// Get the array index of a quad tree node at position x,y and given depth.
inline int quadTreeNodeIndex(int x, int y, int depth)
{
    assert(depth >= 0);
    assert(x >= 0 && x < (1<<depth));
    assert(y >= 0 && y < (1<<depth));
    int index = 0;
    // Compute the offset of the node relative to the first node of the level.
    for(int i = 0, iend = 2*depth; i < iend; i+=2)
    {
        index |= ((x & 1) << i) | ((y & 1) << (i+1));
        x >>= 1;
        y >>= 1;
    }
    // Add on the offset of the first node at the level.  This is the sum of
    // the number of nodes on each previous level (which happens to be a
    // geometric series).
    return index + quadTreeNumNodes(depth-1);
}


//------------------------------------------------------------------------------
// 2D BSP tree, packed array storage.
//
// Example: BSP tree of depth 2.
//
// Node numbering
//
//          0       level 0
//         / \            .
//        /   \           .
//       1     2          1
//      / \   / \         .
//     3   4 5   6        2
//
//
// spatial layout
//
//   |         x
//  -+------------>
//   |
//   |   +-------+   +-------+   +-------+
//   |   |       |   |   |   |   | 3 | 5 |
// y |   |   0   |   | 1 | 2 |   |---|---|
//   v   |       |   |   |   |   | 4 | 6 |
//       +-------+   +-------+   +-------+
//
inline int bspTreeNodeIndex(int x, int y, int depth)
{
    assert(depth >= 0);
    int index = 0;
    if(depth % 2 == 0)
        std::swap(x,y);
    // interlace bits of i and j
    for(int i = 0; i < depth; i+=2)
    {
        index |= ((x & 1) << i) | ((y & 1) << (i+1));
        x >>= 1;
        y >>= 1;
    }
    return index + ((1 << depth) - 1);
}


} // namespace Aqsis
#endif // AQSIS_TREEARRAYSTORAGE_H_INCLUDED
