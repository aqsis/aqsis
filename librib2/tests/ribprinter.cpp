#include "librib.h"
#include "librib2stream.h"

#include <iostream>
#include <fstream>

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
    if ( argc < 2 )
    {
        std::cerr << argv[ 0 ] << " usage: " << argv[ 0 ] << " ribfile1, [ribfile2 ...]" << std::endl;
        return 1;
    }

    for ( int i = 1; i < argc; i++ )
    {
        FILE* file = fopen( argv[ i ], "rb" );
        if ( NULL == file )
        {
            std::cerr << argv[ 0 ] << ": error opening " << argv[ i ] << ", aborting." << std::endl;
            return 2;
        }

        librib2stream::Stream stream( std::cout );
        librib::StandardDeclarations( stream );
        if ( !librib::Parse( file, argv[ i ], stream, std::cerr, NULL ) )
        {
            fclose( file );
            return 3;
        }
        fclose( file );
        librib::CleanupDeclarations( stream );
    }

    return 0;
}

