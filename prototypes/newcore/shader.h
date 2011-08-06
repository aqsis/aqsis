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

#ifndef AQSIS_SHADER_H_INCLUDED
#define AQSIS_SHADER_H_INCLUDED

#include "refcount.h"
#include "util.h"
#include "varspec.h"

#include <aqsis/riutil/ricxx.h>

#include <boost/random/linear_congruential.hpp>

namespace Aqsis {

class Grid;
class ShadingContext;

//------------------------------------------------------------------------------
/// A simplistic shader interface
class Shader : public RefCounted
{
    public:
        /// Get the set of input variables used by the shader.
        virtual const VarSet& inputVars() const = 0;
        /// Get the set of output variables set by the shader.
        virtual const VarSet& outputVars() const = 0;

        /// Execute the shader on the given grid.
        virtual void shade(ShadingContext& ctx, Grid& grid) = 0;

        virtual ~Shader() {}
};

typedef boost::intrusive_ptr<Shader> ShaderPtr;

// Create one of the builtin shaders.
ShaderPtr createShader(const char* name,
                       const Ri::ParamList& pList = Ri::ParamList());


//------------------------------------------------------------------------------
/// Context for shader execution.
class ShadingContext
{
    public:
        ShadingContext(const M44f& camToWorld);

        /// Transformation handling
        M44f getTransform(const char* toSpace);
        //M44f getTransform(const char* fromSpace, const char* toSpace);

        /// Get a uniform random number between 0 and 1
        float rand();

    private:
        typedef boost::rand48 RngType;

        RngType m_rand;
        float m_randScale;

        M44f m_camToWorld;
};


} // namespace Aqsis
#endif // AQSIS_SHADER_H_INCLUDED
