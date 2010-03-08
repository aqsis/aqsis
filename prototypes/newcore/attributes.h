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

#ifndef ATTRIBUTES_H_INCLUDED
#define ATTRIBUTES_H_INCLUDED

#include "shader.h"

/// Surface attribute state.
struct Attributes
{
    float shadingRate;  ///< Desired micropoly area
    float focusFactor;  ///< Control for enlarging focal blurred micropolys
    bool smoothShading; ///< Type of shading interpolation
    float displacementBound;  ///< Maximum bound on displacement from shader
    ShaderPtr surfaceShader;  ///< surface shader

    Attributes()
        : shadingRate(1),
        focusFactor(1),
        smoothShading(true),
        displacementBound(0),
        surfaceShader()
    { }
};

#endif // ATTRIBUTES_H_INCLUDED
