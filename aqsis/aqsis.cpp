#include "ri.h"
#include "librib.h"
#include "librib2ri.h"

#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
        if(argc < 2)
                {
                        std::cerr << argv[0] << " usage: " << argv[0] << " ribfile1, [ribfile2 ...]" << std::endl;
                        return 1;
                }

	RiBegin(argv[0]);
	librib2ri::Engine renderengine;
        for(int i = 1; i < argc; i++)
                {
                        std::ifstream file(argv[i]);
                        if(!file.good())
                                {
                                        std::cerr << argv[0] << ": error opening " << argv[i] << ", aborting." << std::endl;
                                        return 2;
                                }

                        if(!librib::Parse(file, argv[i], renderengine, std::cerr))
                                return 3;
                }
	RiEnd();

        return 0;
}

