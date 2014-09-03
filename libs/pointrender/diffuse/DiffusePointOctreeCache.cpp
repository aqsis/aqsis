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



#include "DiffusePointOctreeCache.h"

#include <cassert>
#include <cmath>
#include <cstring>

#include <Partio.h>

#include <aqsis/util/logging.h>

#include "DiffusePointOctree.h"

namespace Aqsis {


DiffusePointOctree* DiffusePointOctreeCache::find(const std::string& fileName) {

	// Try to get octree from the cache ...
    MapType::const_iterator i = m_cache.find(fileName);
    if(i == m_cache.end()) {

        // Not in the cache, open the file ...
        // TODO: Path handling
        PointArray points;

        // Convert to octree
        boost::shared_ptr<DiffusePointOctree> tree;
        if(loadDiffusePointFile(points, fileName)) {
            tree.reset(new DiffusePointOctree(points));
        } else {
            Aqsis::log() << error << "Point cloud file \"" << fileName
                         << "\" not found\n";
        }

        // Insert into map.  If we couldn't load the file, we insert
        // a null pointer to record the failure.
        m_cache.insert(MapType::value_type(fileName, tree));
        return tree.get();
    }

    // Return the octree.
    return i->second.get();
}


void DiffusePointOctreeCache::clear() {
    m_cache.clear();
}


}
