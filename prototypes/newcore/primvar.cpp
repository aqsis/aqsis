#include "primvar.h"

namespace StdVar
{
#   define DEFINE_STD_VAR(iclass, type, count, name)              \
        const PrimvarSpec name(PrimvarSpec::iclass, PrimvarSpec::type,  \
                         count, ustring(#name))

    DEFINE_STD_VAR(Vertex, Point, 1, P);
    DEFINE_STD_VAR(Varying, Color, 1, Cs);
    DEFINE_STD_VAR(Varying, Float, 2, st);

#undef DEFINE_STD_VAR
}
