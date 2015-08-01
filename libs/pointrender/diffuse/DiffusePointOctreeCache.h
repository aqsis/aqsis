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

#ifndef DIFFUSEPOINTOCTREECACHE_H_
#define DIFFUSEPOINTOCTREECACHE_H_

#include <map>

#include <boost/shared_ptr.hpp>
#include "DiffusePointOctree.h"

namespace Aqsis {

class DiffusePointOctreeCache {

private:

	typedef std::map<std::string, boost::shared_ptr<DiffusePointOctree> > MapType;
	MapType m_cache; //< The cache

public:

	/**
	 * Find a cached point octree by it's filename.
	 *
	 * TODO: Search path handling.
	 *
	 * @param fileName
	 * 			The filename of the pointcloud file of the octree.
	 * @return
	 * 			The octree of the surfels in the pointcloud file.
	 */
	DiffusePointOctree* find(const std::string& fileName);

	/**
	 * Clear all the octrees of the cache.
	 */
	void clear();

};

}

#endif /* DIFFUSEPOINTOCTREECACHE_H_ */
