#include <iostream>

#include <aqsis/util/logging.h>
#include <aqsis/util/logging_streambufs.h>

#include "ricxx2ri.h"
#include "ribsema.h"

using namespace Aqsis;

int main(int argc, char* argv[])
{
    bool useGzip = false;
    bool useBinary = false;
    bool interpolate = false;
    if(argc > 1)
    {
        char* c = argv[1];
        if(*c == '-')
        {
            ++c;
            while(*c)
            {
                switch(*c)
                {
                    case 'a':
                        interpolate = true;
                        break;
                    case 'b':
                        useBinary = true;
                        break;
                    case 'z':
                        useGzip = true;
                        break;
                }
                ++c;
            }
        }
    }

    // Colourization of errors:
    Aqsis::color_level_buf colorLevel(Aqsis::log());
    // Faster buffering, but log colourization then fails:
    //std::ios_base::sync_with_stdio(false);

    boost::shared_ptr<Ri::RendererServices> services =
        createRibWriter(std::cout, interpolate, useBinary, useGzip);
    services->addFilter("validate");
    services->parseRib(std::cin, "stdin", services->firstFilter());

    return 0;
}

// vi: set et:
