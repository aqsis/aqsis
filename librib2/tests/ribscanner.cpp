#include "librib.h"
#include "bdec.h"
#include "libribtypes.h"
#include "parser.h"

extern int yylex( YYSTYPE* );
namespace librib
{
extern CqRibBinaryDecoder* BinaryDecoder;
};

RtBasis	RiBezierBasis	= {{ -1, 3, -3, 1},
                         {3, -6, 3, 0},
                         { -3, 3, 0, 0},
                         {1, 0, 0, 0}};
RtBasis	RiBSplineBasis	= {{ -1, 3, -3, 1},
                          {3, -6, 3, 0},
                          { -3, 0, 3, 0},
                          {1, 4, 1, 0}};
RtBasis	RiCatmullRomBasis	= {{ -1, 3, -3, 1},
                             {2, -5, 4, -1},
                             { -1, 0, 1, 0},
                             {0, 2, 0, 0}};
RtBasis	RiHermiteBasis	= {{ 2, 1, -2, 1},
                          { -3, -2, 3, -1},
                          {0, 1, 0, 0},
                          {1, 0, 0, 0}};
RtBasis	RiPowerBasis	= {{ 1, 0, 0, 0},
                        {0, 1, 0, 0},
                        {0, 0, 1, 0},
                        {0, 0, 0, 1}};

int main( int argc, char* argv[] )
{
    YYSTYPE data;
    librib::BinaryDecoder = new librib::CqRibBinaryDecoder( stdin );
    while ( yylex( &data ) )
    {}

    delete( librib::BinaryDecoder );
    return 0;
}

