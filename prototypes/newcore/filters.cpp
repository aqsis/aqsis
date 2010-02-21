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

#include "filters.h"

#include <cmath>

#include <boost/math/special_functions/sinc.hpp>


class BoxFilter : public Filter
{
    public:
        BoxFilter(const Vec2& width) : Filter(width) {}

        virtual float operator()(float x, float y) const
        {
            if (   -width().x <= x && x <= width().x
                && -width().y <= y && y <= width().y )
                return 1;
            return 0;
        }

        virtual bool isSeparable() const { return true; }
};


class DiscFilter : public Filter
{
    public:
        DiscFilter(const Vec2& width) : Filter(width) {}

        virtual float operator()(float x, float y) const
        {
            x /= width().x;
            y /= width().y;
            float r2 = x*x + y*y;
            if(r2 <= 1)
                return 1;
            return 0;
        }

        virtual bool isSeparable() const { return false; }
};


class GaussianFilter : public Filter
{
    public:
        GaussianFilter(const Vec2& width) : Filter(width) {}

        virtual float operator()(float x, float y) const
        {
            x /= width().x;
            y /= width().y;
            return std::exp(-8*(x*x + y*y));
        }

        virtual bool isSeparable() const { return true; }
};


/// Sinc filter with Lanczos window
class SincFilter : public Filter
{
    private:
        static float windowedsinc(float x, float width)
        {
            float xscale = x*M_PI;
            float window = 0;
            if(std::abs(x) < width)
                window = boost::math::sinc_pi(xscale/width);
            return boost::math::sinc_pi(xscale)*window;
        }

    public:
        SincFilter(const Vec2& width) : Filter(width) {}

        virtual float operator()(float x, float y) const
        {
            return windowedsinc(x, width().x)*windowedsinc(y, width().y);
        }

        virtual bool isSeparable() const { return true; }
};


FilterPtr makeBoxFilter(const Vec2& width)
{
    assert(width.x >= 1 && width.y >= 1);
    return FilterPtr(new BoxFilter(width));
}
FilterPtr makeDiscFilter(const Vec2& width)
{
    assert(width.x >= 1 && width.y >= 1);
    return FilterPtr(new DiscFilter(width));
}
FilterPtr makeGaussianFilter(const Vec2& width)
{
    assert(width.x >= 1 && width.y >= 1);
    return FilterPtr(new GaussianFilter(width));
}
FilterPtr makeSincFilter(const Vec2& width)
{
    assert(width.x >= 1 && width.y >= 1);
    return FilterPtr(new SincFilter(width));
}

