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

#include "shader.h"

#include <string>

#include <boost/range/end.hpp>

#include "grid.h"
#include "gridstorage.h"
#include "util.h"
#include "varspec.h"

namespace Aqsis {


// Helper mixin class to hold shader input/output variables.
class IOVarHolder : public Shader
{
    private:
        VarSet m_inputVars;
        VarSet m_outputVars;

    protected:
        IOVarHolder() : m_inputVars(), m_outputVars() {}

        template<typename T, size_t n>
        void setInputVars(const T (&inVars)[n]) { m_inputVars.assign(inVars, inVars+n); }

        template<typename T, size_t n>
        void setOutputVars(const T (&outVars)[n]) { m_outputVars.assign(outVars, outVars+n); }

    public:
        virtual const VarSet& inputVars() const { return m_inputVars; }
        virtual const VarSet& outputVars() const { return m_outputVars; }
};


//------------------------------------------------------------------------------
// More or less the usual default surface shader, but without opacity.
class DefaultSurface : public IOVarHolder
{
    private:
        float m_Kd;
        float m_Ka;

    public:
        DefaultSurface(float Kd = 0.8, float Ka = 0.2)
            : m_Kd(Kd),
            m_Ka(Ka)
        {
            VarSpec inVars[] = {
                Stdvar::P,
                Stdvar::N,
                Stdvar::I,
                Stdvar::Cs
            };
            setInputVars(inVars);
            VarSpec outVars[] = {
                Stdvar::Ci
            };
            setOutputVars(outVars);
        }

        virtual void shade(ShadingContext& ctx, Grid& grid)
        {
            GridStorage& stor = grid.storage();

            DataView<Vec3> N = stor.get(StdIndices::N);
            ConstDataView<Vec3> I = stor.get(StdIndices::I);
            ConstDataView<Col3> Cs = stor.get(StdIndices::Cs);
            DataView<Col3> Ci = stor.get(StdIndices::Ci);

            int nshad = stor.nverts();

            for(int i = 0; i < nshad; ++i)
            {
                float d = dot(I[i].normalized(), N[i].normalized());
                Ci[i] = Cs[i] * (m_Ka + m_Kd*d*d);
            }
        }
};


//------------------------------------------------------------------------------
/// A crazy test shader (combined surface + displacement for now)
class LumpySin : public IOVarHolder
{
    public:
        LumpySin()
        {
            VarSpec inVars[] = {
                Stdvar::P,
                Stdvar::N,
                Stdvar::I,
                Stdvar::Cs
            };
            setInputVars(inVars);
            VarSpec outVars[] = {
                Stdvar::Ci,
                Stdvar::P,
                Stdvar::N
            };
            setOutputVars(outVars);
        }

        virtual void shade(ShadingContext& ctx, Grid& grid)
        {
            GridStorage& stor = grid.storage();

            int nshad = stor.nverts();

            // Bind variables to the storage.  Most of these are guarenteed to
            // be present on the grid.
            DataView<Vec3> N = stor.get(StdIndices::N);
            ConstDataView<Vec3> I = stor.get(StdIndices::I);
            ConstDataView<Col3> Cs = stor.get(StdIndices::Cs);
            DataView<Col3> Ci = stor.get(StdIndices::Ci);
            if(!Ci)
                Ci = DataView<Col3>(FALLOCA(3*nshad));
            DataView<Vec3> P = stor.P();

            for(int i = 0; i < nshad; ++i)
            {
                float amp = 0.05*(std::sin(10*P[i].x) + std::sin(30*P[i].y)
                                  + std::sin(20*P[i].z));
                P[i] += amp*N[i].normalized();
            }
            grid.calculateNormals(N, P);
            for(int i = 0; i < nshad; ++i)
                Ci[i] = Cs[i] * fabs(I[i].normalized().dot(N[i].normalized()));
        }
};


//------------------------------------------------------------------------------
/// Shader to show grids.
class ShowGrids : public IOVarHolder
{
    public:
        ShowGrids()
        {
            VarSpec inVars[] = {
                Stdvar::P,
                Stdvar::N,
                Stdvar::I,
            };
            setInputVars(inVars);
            VarSpec outVars[] = {
                Stdvar::Ci
            };
            setOutputVars(outVars);
        }
        virtual void shade(ShadingContext& ctx, Grid& grid)
        {
            GridStorage& stor = grid.storage();

            DataView<Vec3> N = stor.get(StdIndices::N);
            ConstDataView<Vec3> I = stor.get(StdIndices::I);
            DataView<Col3> Ci = stor.get(StdIndices::Ci);

            Col3 Cs(ctx.rand(), ctx.rand(), ctx.rand());

            float Kd = 0.8;
            float Ka = 0.2;
            int nshad = stor.nverts();

            int nu = 1;
            int nv = 1;
            if(grid.type() == GridType_Quad)
            {
                // Ugh!  Would be nice if there was a better way to extract
                // nu & nv generically...
                nu = static_cast<QuadGrid&>(grid).nu();
                nv = static_cast<QuadGrid&>(grid).nv();
            }

            /*
            // Debug: Some displacement stuff.
            DataView<Vec3> P = stor.P();
            for(int i = 0; i < nshad; ++i)
            {
                float amp = 0.05*(std::sin(10*P[i].x) + std::sin(30*P[i].y)
                                  + std::sin(20*P[i].z));
                P[i] += amp*N[i].normalized();
            }
            grid.calculateNormals(N, P);
            */

            for(int i = 0; i < nshad; ++i)
            {
                float d = dot(I[i].normalized(), N[i].normalized());
                bool isBoundary = i%nu == 0 || i%nu == nu-1
                                  || i/nu == 0 || i/nu == nv-1;
                Ci[i] = Cs*(0.5f*isBoundary + 0.5)  * (Ka + Kd*d*d);
            }
        }
};

/// Shader to show micro polygons
class ShowPolys : public IOVarHolder
{
    public:
        ShowPolys()
        {
            VarSpec inVars[] = {
                Stdvar::P,
                Stdvar::N,
                Stdvar::I,
            };
            setInputVars(inVars);
            VarSpec outVars[] = {
                Stdvar::Ci
            };
            setOutputVars(outVars);
        }
        virtual void shade(ShadingContext& ctx, Grid& grid)
        {
            GridStorage& stor = grid.storage();

            DataView<Vec3> N = stor.get(StdIndices::N);
            ConstDataView<Vec3> I = stor.get(StdIndices::I);
            DataView<Col3> Ci = stor.get(StdIndices::Ci);

            float Kd = 0.8;
            float Ka = 0.2;
            int nshad = stor.nverts();

            for(int i = 0; i < nshad; ++i)
            {
                Col3 Cs(ctx.rand(), ctx.rand(), ctx.rand());
                float d = dot(I[i].normalized(), N[i].normalized());
                Ci[i] = Cs * (Ka + Kd*d*d);
            }
        }
};


//------------------------------------------------------------------------------
ShaderPtr createShader(const char* name)
{
    if(name == std::string("lumpy_sin"))
        return ShaderPtr(new LumpySin());
    else if(name == std::string("showgrids"))
        return ShaderPtr(new ShowGrids());
    else if(name == std::string("showpolys"))
        return ShaderPtr(new ShowPolys());
    else if(name == std::string("default"))
        return ShaderPtr(new DefaultSurface());
    else
        return ShaderPtr();
}


//------------------------------------------------------------------------------
ShadingContext::ShadingContext(const Mat4& camToWorld)
    : m_camToWorld(camToWorld)
{
    m_randScale = 1.0f/(m_rand.max() - m_rand.min());
}

Mat4 ShadingContext::getTransform(const char* toSpace)
{
    if(strcmp(toSpace, "world") == 0)
    {
        return m_camToWorld;
    }
    else
    {
        // TODO
        assert(0);
        return Mat4();
    }
}

float ShadingContext::rand()
{
    return m_randScale*(m_rand() - m_rand.min());
}

} // namespace Aqsis
