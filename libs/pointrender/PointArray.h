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

#ifndef POINTARRAY_H_
#define POINTARRAY_H_

#include <vector>

#include <Imath/ImathVec.h>
#include <Imath/ImathBox.h>
#include <Imath/ImathColor.h>

namespace Aqsis {

/**
 * This is an array of surface elements.
 *
 * Point data is stored in a flat array as:
 *
 * <pre>
 * [P1 N1 r1 data1  P2 N2 r2 data2  ... ]
 * </pre>
 *
 * where data1 data2... is extra "user data" appended after the position
 * normal, and radius data.
 */
struct PointArray {

	int stride; //< The size of a point in floats.
	std::vector<float> data; //< The points stored in a float array.

	/**
	 * Get the number of points in the array.
	 *
	 * @return The number of points in the array.
	 */
	size_t size() const {
		return data.size() / stride;
	}

	/**
	 * Get the geometric center of all the points in the array.
	 *
	 * @return The geametric center of all the points.
	 */
	Imath::V3f centroid() const {
		Imath::V3f sum(0);
		for (std::vector<float>::const_iterator p = data.begin(); p
				< data.end(); p += stride) {
			sum += Imath::V3f(p[0], p[1], p[2]);
		}
		return (1.0f / data.size() * stride) * sum;
	}
};

}
#endif /* POINTARRAY_H_ */
