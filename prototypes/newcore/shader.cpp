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

#include <aqsis/math/noise.h>
#include <aqsis/math/cellnoise.h>

#include "grid.h"
#include "gridstorage.h"
#include "util.h"
#include "varspec.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/// Pattern functions
inline float triangleWave(float x, float w)
{
    return std::abs(x - w*(std::floor(x/w)+0.5));
}

inline float sawWave(float x, float w)
{
    return x - w*std::floor(x/w);
}

inline Vec3 reflect(Vec3 N, Vec3 I)
{
    return I - 2*(N^I) * N;
}

static CqNoise g_noise;
static CqCellNoise g_cellnoise;

inline float f_noise(const Vec3& p)
{
    return g_noise.FGNoise3(CqVector3D(p.x, p.y, p.z));
}

inline Vec3 v_noise(const Vec3& p)
{
    CqVector3D v = g_noise.PGNoise3(CqVector3D(p.x, p.y, p.z));
    return Vec3(v.x(), v.y(), v.z());
}

inline float f_cellnoise(const Vec3& p)
{
    return g_cellnoise.FCellNoise3(CqVector3D(p.x, p.y, p.z));
}

inline Vec3 v_cellnoise(const Vec3& p)
{
    CqVector3D n = g_cellnoise.PCellNoise3(CqVector3D(p.x, p.y, p.z));
    return Vec3(n.x(), n.y(), n.z());
}

inline float smoothstep(float min, float max, float x)
{
    if(x < min)
        return 0;
    if(x > max)
        return 1;
    x = (x - min)/(max - min);
    return x*x*(3 - 2*x);
}

float turbulence(Vec3 pos, int octaves, float lambda, float omega)
{
    float value = 0;
    float l = 1;
    float o = 1;
    for(int i=0; i < octaves; i+=1)
    {
        value += o*(2*f_noise(pos*l)-1);
        l *= lambda;
        o *= omega;
    }
    return value;
}

float crater(Vec3 p, float jitter, float overlap, float sharpness)
{
    Vec3 centre(floor(p.x + 0.5), floor(p.y + 0.5), floor(p.z + 0.5));
    float amp = 0;
    for(int i = -1; i <= 1; i += 1)
    for(int j = -1; j <= 1; j += 1)
    for(int k = -1; k <= 1; k += 1)
    {
        Vec3 cellCentreIjk = centre + Vec3(i,j,k);
        cellCentreIjk += jitter * v_cellnoise(cellCentreIjk) - Vec3(0.5);
        float rad = 0.5*overlap*f_cellnoise(cellCentreIjk);
        float d = (p - cellCentreIjk).length();
        amp = std::min(amp, smoothstep(sharpness*rad, rad, d) - 1);
    }
    return amp;
}

inline float specular(Vec3 N, Vec3 V, Vec3 L, float roughness)
{
    Vec3 H = (L + V).normalized();
    return std::pow(std::max(0.0f, N^H), 8.0f/roughness);
}

inline Vec3 faceforward(Vec3 N, Vec3 I)
{
    if((I^N) < 0)  // Should use Ng here
        return -N;
    else
        return N;
}

//------------------------------------------------------------------------------
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
        DefaultSurface(const Ri::ParamList& pList)
            : m_Kd(0.7),
            m_Ka(0.2)
        {
            if(Ri::FloatArray Kd = pList.findFloatData(Ri::TypeSpec::Float, "Kd"))
                m_Kd = Kd[0];
            if(Ri::FloatArray Ka = pList.findFloatData(Ri::TypeSpec::Float, "Ka"))
                m_Ka = Ka[0];
            VarSpec inVars[] = {
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
/// Displace along normal with a set of sinusoids.
class LumpySin : public IOVarHolder
{
    public:
        LumpySin(const Ri::ParamList& pList)
            : m_useCameraCoords(false)
        {
            Ri::IntArray useCam = pList.findIntData(Ri::TypeSpec::Int,
                                                    "use_cam_coords");
            m_useCameraCoords = useCam && useCam[0] != 0;
            VarSpec inVars[] = {
                Stdvar::P,
                Stdvar::N,
            };
            setInputVars(inVars);
            VarSpec outVars[] = {
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
            DataView<Vec3> P = stor.P();

            Mat4 shaderCoords = ctx.getTransform("world")
                            * Mat4().setAxisAngle(Vec3(1,0,0), deg2rad(45))
                            * Mat4().setAxisAngle(Vec3(0,0,1), deg2rad(10))
                            * Mat4().setAxisAngle(Vec3(0,1,0), deg2rad(45));

            for(int i = 0; i < nshad; ++i)
            {
                Vec3 p = P[i];
                if(!m_useCameraCoords)
                    p = p*shaderCoords;
                float amp =
                    0.05*(std::sin(10*p.x) +
                          std::sin(30*p.y) +
                          std::sin(20*p.z));
//                    0.1*(triangleWave(1*p.x, 1) +
//                         triangleWave(3*p.y, 1) +
//                         triangleWave(2*p.z, 1));
//                    0.005*(std::sin(100*p.x) +
//                           std::sin(300*p.y) +
//                           std::sin(200*p.z));
                P[i] += amp*N[i].normalized();
            }
            grid.calculateNormals(N, P);
        }

    private:
        bool m_useCameraCoords;
};


//------------------------------------------------------------------------------
// Spirals in the y-direction with given amplitude and frequency.
class Helix : public IOVarHolder
{
    public:
        Helix(const Ri::ParamList& pList)
            : m_frequency(10),
            m_amplitude(0.02)
        {
            if(Ri::FloatArray f = pList.findFloatData(Ri::TypeSpec::Float, "frequency"))
                m_frequency = f[0];
            if(Ri::FloatArray a = pList.findFloatData(Ri::TypeSpec::Float, "amplitude"))
                m_amplitude = a[0];
            VarSpec inVars[] = {
                Stdvar::P,
                Stdvar::N,
            };
            setInputVars(inVars);
            VarSpec outVars[] = {
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
            DataView<Vec3> P = stor.P();

            Mat4 shaderCoords = ctx.getTransform("world");
            Mat4 scinv = shaderCoords.inverse();

            for(int i = 0; i < nshad; ++i)
            {
                Vec3 p = P[i]*shaderCoords;
                p += m_amplitude*Vec3(std::cos(m_frequency*p.y + p.x), 0,
                                      std::sin(m_frequency*p.y + p.x));
                P[i] = p*scinv;
            }
            grid.calculateNormals(N, P);
        }

    private:
        float m_frequency;
        float m_amplitude;
};


//------------------------------------------------------------------------------
/// A lumpy displacement shader with phong lighting.
class Plastic : public IOVarHolder
{
    private:
        float m_Ka;
        float m_Kd;
        float m_Ks;
        float m_roughness;
        Col3 m_lightColor;

    public:
        Plastic(const Ri::ParamList& pList)
            : m_Ka(0.2),
            m_Kd(0.5),
            m_Ks(0.5),
            m_roughness(0.1),
            m_lightColor(Vec3(1))
        {
            if(Ri::FloatArray Kd = pList.findFloatData(Ri::TypeSpec::Float, "Kd"))
                m_Kd = Kd[0];
            if(Ri::FloatArray Ka = pList.findFloatData(Ri::TypeSpec::Float, "Ka"))
                m_Ka = Ka[0];
            if(Ri::FloatArray Ks = pList.findFloatData(Ri::TypeSpec::Float, "Ks"))
                m_Ks = Ks[0];
            if(Ri::FloatArray r = pList.findFloatData(Ri::TypeSpec::Float, "roughness"))
                m_roughness = r[0];
            if(Ri::FloatArray col = pList.findFloatData(Ri::TypeSpec::Color, "lightcolor"))
                m_lightColor = Col3(col[0], col[1], col[2]);
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

            //Vec3 lightPos = Vec3(2,2,2) * ctx.getTransform("world").inverse();

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
                Vec3 nI = I[i].normalized();
                Vec3 nN = faceforward(N[i].normalized(), I[i]);
                //Vec3 nL = (P[i] - lightPos).normalized();
                Vec3 R = reflect(nN, nI);
                Ci[i] = Cs[i] * (m_Ka // ambient
                                 + m_Kd*std::abs(nI^nN)) // diffuse
                        + m_lightColor*m_Ks*specular(nN, nI, nI, m_roughness); // specular
            }
        }
};


//------------------------------------------------------------------------------
/// Shader to show grids.
class ShowGrids : public IOVarHolder
{
    public:
        ShowGrids(const Ri::ParamList& pList)
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

            for(int i = 0; i < nshad; ++i)
            {
                float d = dot(I[i].normalized(), N[i].normalized());
                bool isBoundary = i%nu == 0 || i%nu == nu-1
                                  || i/nu == 0 || i/nu == nv-1;
                Ci[i] = Cs*(0.5f*isBoundary + 0.5)  * (Ka + Kd*d*d);
            }
        }
};


//------------------------------------------------------------------------------
/// Shader to show micro polygons
class ShowPolys : public IOVarHolder
{
    public:
        ShowPolys(const Ri::ParamList& pList)
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
/// Asteroid shader example.
class Asteroid : public IOVarHolder
{
    public:
        Asteroid(const Ri::ParamList& pList)
        {
            VarSpec inVars[] = {
                Stdvar::P,
                Stdvar::N,
            };
            setInputVars(inVars);
            VarSpec outVars[] = {
                Stdvar::P,
                Stdvar::N
            };
            setOutputVars(outVars);
        }

        virtual void shade(ShadingContext& ctx, Grid& grid)
        {
            GridStorage& stor = grid.storage();

            int nshad = stor.nverts();

            Mat4 currentToWorld = ctx.getTransform("world");

            // Bind variables to the storage.  Most of these are guarenteed to
            // be present on the grid.
            DataView<Vec3> N = stor.get(StdIndices::N);
            DataView<Vec3> P = stor.P();

            // Displacement
            for(int i = 0; i < nshad; ++i)
            {
                Vec3 Pobj = P[i]*currentToWorld;
                float amp = 0.2*(
                    0.3*crater(2.0f*Pobj + 0.1f*v_noise(4.0f*Pobj), 0.5, 1, 0.5)
                    + 0.1*crater(5.0f*Pobj + 0.2f*v_noise(7.0f*Pobj), 1, 1, 0.6)
                    + 0.015*crater(20.0f*Pobj, 1, 1, 0.8)
                    ) + 0.1*turbulence(0.5f*Pobj, 8, 2, 0.4);
                P[i] += amp*N[i].normalized();
            }
            grid.calculateNormals(N, P);
        }
};


//------------------------------------------------------------------------------
ShaderPtr createShader(const char* name, const Ri::ParamList& pList)
{
    if(name == std::string("lumpy_sin"))
        return ShaderPtr(new LumpySin(pList));
    if(name == std::string("helix"))
        return ShaderPtr(new Helix(pList));
    else if(name == std::string("plastic"))
        return ShaderPtr(new Plastic(pList));
    else if(name == std::string("asteroid"))
        return ShaderPtr(new Asteroid(pList));
    else if(name == std::string("showgrids"))
        return ShaderPtr(new ShowGrids(pList));
    else if(name == std::string("showpolys"))
        return ShaderPtr(new ShowPolys(pList));
    else if(name == std::string("default"))
        return ShaderPtr(new DefaultSurface(pList));
    else
        return ShaderPtr();
}


//------------------------------------------------------------------------------
// ShadingContext implementation.
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
