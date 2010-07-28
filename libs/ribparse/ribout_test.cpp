#include <iostream>

#include "ricxx2ri.h"
#include "ribsema.h"

using namespace Aqsis;

int main(int argc, char* argv[])
{
    bool useGzip = false;
    bool useBinary = false;
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

    boost::shared_ptr<Ri::Renderer> renderer =
        createRibOut(std::cout, useBinary, useGzip);
    RibParser parser(*renderer);
    std::ios_base::sync_with_stdio(false);
    parser.parseStream(std::cin, "stdin");

    return 0;
}
// vi: set et:
