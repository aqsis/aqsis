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

#include <iomanip>

#include "primvar.h"
#include "gridstorage.h"

#define ARRLEN(ar) sizeof(ar)/sizeof(ar[0])

using namespace Aqsis;

namespace Var
{
#   define MAKE_STD_VAR(iclass, type, count, name)                 \
        PrimvarSpec name(PrimvarSpec::iclass, PrimvarSpec::type,   \
                         count, ustring(#name))

    MAKE_STD_VAR(Vertex, Point, 1, P);
    MAKE_STD_VAR(Varying, Color, 1, Cs);
    MAKE_STD_VAR(Varying, Float, 2, st);

#   undef MAKE_STD_VAR
}


//class Grid
//{
//    PrimvarList
//    PrimvarStorage
//};


int main()
{
    IclassStorage bilinPatchCount(1, 4, 4, 4, 4);
    PrimvarStorage pvarStorage(bilinPatchCount);

    // Put some stuff into the primvar list.
    {
        float st[] = {
            0, 0,
            1, 0,
            0, 1,
            1, 1
        };
        pvarStorage.add(Var::st, st, ARRLEN(st));

        float P[] = {
            0, 0, 0,
            0, 1, 0,
            1, 0, 0,
            1, 1, 0
        };
        pvarStorage.add(Var::P, P, ARRLEN(P));

        float asdf[] = {42};
        pvarStorage.add(PrimvarSpec(PrimvarSpec::Uniform, PrimvarSpec::Float,
                                    1, ustring("asdf")), asdf, ARRLEN(asdf));
    }


    {
        // Check that we can access the position data using the correct type.
        DataView<V3f> P = pvarStorage.P();
        std::cout << "Primvar P = "
            << P[0] << " "
            << P[1] << " "
            << P[2] << " "
            << P[3] << "\n";
    }

    // Now simulate the dicing stage of the pipeline.
    GridStorageBuilder gridBuilder;
    GridvarList gvarSet(pvarStorage.varList());
    const int nu = 5, nv = 5;
    GridStoragePtr gvarStorage = gridBuilder.build(nu*nv);

    // Create some space to store the variable temporaries.
    int maxAgg = gvarStorage.maxAggregateSize();
    float* aMin = FALLOCA(maxAgg);
    float* aMax = FALLOCA(maxAgg);

    float dv = 1.0/(nv-1);
    float du = 1.0/(nu-1);

    for(int ivar = 0; ivar < gvarList.size(); ++ivar)
    {
        ConstFvecView pvar = pvarStorage.get(ivar);
        FvecView gvar = gvarStorage.get(ivar);
        int size = gvar.elSize();

        if(gvarList[ivar].uniform)
        {
            // Uniform, no interpolation.
            float* out = gvar[0];
            const float* in = pvar[0];
            for(int i = 0; i < size; ++i)
                out[i] = in[i];
        }
        else
        {
            // Varying class, linear interpolation
            const float* a1 = pvar[0];
            const float* a2 = pvar[1];
            const float* a3 = pvar[2];
            const float* a4 = pvar[3];
            for(int v = 0; v < nv; ++v)
            {
                float fv = dv*v;
                // Get endpoints of current segment via linear interpolation
                for(int i = 0; i < size; ++i)
                {
                    aMin[i] = lerp(a1[i], a3[i], fv);
                    aMax[i] = lerp(a2[i], a4[i], fv);
                }
                // Interpolate between endpoints
                for(int u = 0; u < nu; ++u)
                {
                    float fu = du*u;
                    float* out = gvar[u];
                    for(int i = 0; i < size; ++i)
                        out[i] = lerp(aMin[i], aMax[i], fu);
                }
                gvar += nv;
            }
        }
    }

    // Print out the resulting values on the grid.
    for(int ivar = 0; ivar < gvarList.size(); ++ivar)
    {
        std::cout << "\nGridvar "
            << gvarStorage.varList()[ivar].name << " = \n";
        ConstFvecView gvar = gvarStorage.get(ivar);
        if(gvarStorage.varList()[ivar].uniform)
        {
            const float* f = gvar[0];
            for(int i = 0; i < gvar.elSize(); ++i)
                std::cout << std::setprecision(2) << std::fixed << f[i] << " ";
            std::cout << "\n";
        }
        else
        {
            for(int v = 0; v < nv; ++v)
            {
                for(int u = 0; u < nu; ++u)
                {
                    const float* f = gvar[u];
                    for(int i = 0; i < gvar.elSize(); ++i)
                        std::cout << std::setprecision(2) << std::fixed << f[i] << " ";
                    std::cout << "  ";
                }
                std::cout << "\n";
                gvar += nv;
            }
        }
    }

    return 0;
}
