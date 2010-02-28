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

#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED

#include "varspec.h"

#include "util.h"

class Grid;

/// A simplistic shader interface
class Shader : public RefCounted
{
    public:
        /// Get the set of input variables used by the shader.
        virtual const VarSet& inputVars() const = 0;
        /// Get the set of output variables set by the shader.
        virtual const VarSet& outputVars() const = 0;

        /// Execute the shader on the given grid.
        virtual void shade(Grid& grid) = 0;

        virtual ~Shader() {}
};

typedef boost::intrusive_ptr<Shader> ShaderPtr;

// Create one of the builtin shaders.
ShaderPtr createShader(const char* name);


#endif // SHADER_H_INCLUDED
