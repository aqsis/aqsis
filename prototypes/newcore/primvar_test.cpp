#include "primvar.h"

#define ARRLEN(ar) sizeof(ar)/sizeof(ar[0])

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
    PrimvarStorage stor(bilinPatchCount);

    // Put some stuff into the primvar list.

    float st[] = {
        0, 0,
        1, 0,
        0, 1,
        1, 1
    };
    stor.add(Var::st, st, ARRLEN(st));

    float P[] = {
        0, 0, 0,
        0, 1, 0,
        1, 0, 0,
        1, 1, 0
    };
    stor.add(Var::P, P, ARRLEN(P));

    float asdf[] = {42};
    stor.add(PrimvarSpec(PrimvarSpec::Uniform, PrimvarSpec::Float, 1,
                         ustring("asdf")), asdf, ARRLEN(asdf));

    // Check that we can access the position data using the correct type.
    DataView<Vec3> v = stor.P();
    std::cout << v[0] << " "
        << v[1] << " "
        << v[2] << " "
        << v[3] << "\n";

    // Now simulate the dicing stage of the pipeline.
//    GridvarList gvarList(stor.varList());
//    int nu = 10, nv = 10;
//    Grid grid(nu, nv, gvarList);
//
//    for(int ivar = 0; ivar < grid.nVars(); ++ivar)
//    {
//        const GridvarSpec& gvar = gvarList[ivar];
//        int size = gvar.storageSize();
//        for(int v = 0; v < nv; ++v)
//        {
//            for(int u = 0; u < nu; ++u)
//            {
//            }
//        }
//    }
//
//    DataView<Vec3> P = grid.storage().P();

    return 0;
}
