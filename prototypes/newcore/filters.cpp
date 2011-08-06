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

#include "filters.h"

#include <cmath>

#include <boost/math/special_functions/sinc.hpp>

namespace Aqsis {


class BoxFilter : public Filter
{
    public:
        BoxFilter(const V2f& width) : Filter(width) {}

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
        DiscFilter(const V2f& width) : Filter(width) {}

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
        GaussianFilter(const V2f& width) : Filter(width) {}

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
        SincFilter(const V2f& width) : Filter(width) {}

        virtual float operator()(float x, float y) const
        {
            return windowedsinc(x, width().x)*windowedsinc(y, width().y);
        }

        virtual bool isSeparable() const { return true; }
};


FilterPtr makeBoxFilter(const V2f& width)
{
    assert(width.x >= 1 && width.y >= 1);
    return FilterPtr(new BoxFilter(width));
}
FilterPtr makeDiscFilter(const V2f& width)
{
    assert(width.x >= 1 && width.y >= 1);
    return FilterPtr(new DiscFilter(width));
}
FilterPtr makeGaussianFilter(const V2f& width)
{
    assert(width.x >= 1 && width.y >= 1);
    return FilterPtr(new GaussianFilter(width));
}
FilterPtr makeSincFilter(const V2f& width)
{
    assert(width.x >= 1 && width.y >= 1);
    return FilterPtr(new SincFilter(width));
}

} // namespace Aqsis
