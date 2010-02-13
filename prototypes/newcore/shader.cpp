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

#include "shader.h"

#include <string>

#include <boost/range/end.hpp>

#include "grid.h"
#include "gridstorage.h"
#include "util.h"
#include "varspec.h"


//------------------------------------------------------------------------------
// More or less the usual default surface shader, but without opacity.
class DefaultSurface : public Shader
{
    private:
        VarSet m_inputVars;
        VarSet m_outputVars;

        float m_Kd;
        float m_Ka;

    public:
        DefaultSurface(float Kd = 0.8, float Ka = 0.2)
            : m_inputVars(),
            m_outputVars(),
            m_Kd(Kd),
            m_Ka(Ka)
        {
            VarSpec inVars[] = {
                Stdvar::P,
                Stdvar::N,
                Stdvar::I,
                Stdvar::Cs
            };
            m_inputVars.assign(inVars, boost::end(inVars));
            VarSpec outVars[] = {
                Stdvar::Ci
            };
            m_outputVars.assign(outVars, boost::end(outVars));
        }

        const VarSet& inputVars() const { return m_inputVars; }
        const VarSet& outputVars() const { return m_outputVars; }

        virtual void shade(Grid& grid)
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
class TestShader : public Shader
{
    private:
        VarSet m_inputVars;
        VarSet m_outputVars;

    public:
        TestShader()
        {
            VarSpec inVars[] = {
                Stdvar::P,
                Stdvar::N,
                Stdvar::I,
                Stdvar::Cs
            };
            m_inputVars.assign(inVars, boost::end(inVars));
            VarSpec outVars[] = {
                Stdvar::Ci,
                Stdvar::P,
                Stdvar::N
            };
            m_outputVars.assign(outVars, boost::end(outVars));
        }
        const VarSet& inputVars() const { return m_inputVars; }
        const VarSet& outputVars() const { return m_outputVars; }

        virtual void shade(Grid& grid)
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
                float amp = 0.05*(std::sin(10*P[i].x) + std::sin(30*P[i].y) + std::sin(20*P[i].z));
                P[i] += amp*N[i].normalized();
            }
            grid.calculateNormals(N, P);
            for(int i = 0; i < nshad; ++i)
                Ci[i] = Cs[i] * fabs(I[i].normalized().dot(N[i].normalized()));
        }
};


//------------------------------------------------------------------------------
boost::shared_ptr<Shader> createShader(const char* name)
{
    if(name == std::string("test"))
    {
        return boost::shared_ptr<Shader>(new TestShader());
    }
    return boost::shared_ptr<Shader>(new DefaultSurface());
}
