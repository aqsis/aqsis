#include "varspec.h"
#include "fixedstrings.h"

namespace Stdvar
{
#   define DEFINE_STD_VAR(type, count, name)                          \
        const VarSpec name(VarSpec::type, count, ustring(#name))

    DEFINE_STD_VAR(Point, 1, P);
    DEFINE_STD_VAR(Color, 1, Cs);
    DEFINE_STD_VAR(Color, 1, Ci);
    DEFINE_STD_VAR(Color, 1, Os);
    DEFINE_STD_VAR(Color, 1, Oi);
    DEFINE_STD_VAR(Float, 2, st);
    DEFINE_STD_VAR(Float, 2, I);
    DEFINE_STD_VAR(Float, 1, N);

    DEFINE_STD_VAR(Float, 1, z);

#undef DEFINE_STD_VAR
}
