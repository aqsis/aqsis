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

#ifndef FILTERS_H_INCLUDED
#define FILTERS_H_INCLUDED

#include "util.h"  // for Vec2

/// Filter functor class
///
/// Includes information about the width & separability of the filter.
class Filter : public RefCounted
{
    private:
        Vec2 m_width;

    public:
        Filter(const Vec2& width) : m_width(width) {}

        /// Evaluate the filter at position x,y
        ///
        /// TODO: should we pass the widths in here, or expect them to be part
        /// of the filter definition?
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
        const Vec2& width() const { return m_width; }

        virtual ~Filter() {}
};


typedef boost::intrusive_ptr<Filter> FilterPtr;


FilterPtr makeBoxFilter(const Vec2& width);
FilterPtr makeDiscFilter(const Vec2& width);
FilterPtr makeGaussianFilter(const Vec2& width);
FilterPtr makeSincFilter(const Vec2& width);


#endif // FILTERS_H_INCLUDED
