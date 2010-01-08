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

#ifndef SAMPLE_H_INCLUDED
#define SAMPLE_H_INCLUDED

#include <cfloat>

#include "util.h"

struct Sample
{
    Vec2 p;    //< Position of sample in image plane
    float z; //< Things behind this depth are occluded

    Sample() {}
    Sample(const Vec2& p) : p(p), z(FLT_MAX) {}
};

#endif // SAMPLE_H_INCLUDED
