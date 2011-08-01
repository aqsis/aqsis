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

#ifndef AQSIS_FILTERS_H_INCLUDED
#define AQSIS_FILTERS_H_INCLUDED

#include "refcount.h"
#include "util.h"  // for V2f

namespace Aqsis {

/// Filter functor class
///
/// Includes information about the width & separability of the filter.
class Filter : public RefCounted
{
    private:
        V2f m_width;

    public:
        Filter(const V2f& width) : m_width(width) {}

        /// Evaluate the filter at position x,y
        virtual float operator()(float x, float y) const = 0;

        /// Return true if the filter is separable
        ///
        /// A separable filter has the property that f(x,y) = f1(x)*f1(y) for
        /// some 1D function f1.  As a result, separable filtering of a region
        /// is O(R) where R is the filter radius, rather than O(R^2) as in the
        /// non-separable case.
        virtual bool isSeparable() const = 0;

        /// Get the filter width
        ///
        /// The filter support is assumed to be nonzero only on the box
        /// with corners -width()/2 and width()/2.
        const V2f& width() const { return m_width; }

        virtual ~Filter() {}
};


typedef boost::intrusive_ptr<Filter> FilterPtr;


FilterPtr makeBoxFilter(const V2f& width);
FilterPtr makeDiscFilter(const V2f& width);
FilterPtr makeGaussianFilter(const V2f& width);
FilterPtr makeSincFilter(const V2f& width);


} // namespace Aqsis

#endif // AQSIS_FILTERS_H_INCLUDED
