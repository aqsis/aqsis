#include "librib.h"
#include "librib2stream.h"

#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
	if(argc < 2)
		{
			std::cerr << argv[0] << " usage: " << argv[0] << " ribfile1, [ribfile2 ...]" << std::endl;
			return 1;
		}

	for(int i = 1; i < argc; i++)
		{
			std::ifstream file(argv[i]);
			if(!file.good())
				{
					std::cerr << argv[0] << ": error opening " << argv[i] << ", aborting." << std::endl;
					return 2;
				}

			librib2stream::Stream stream(std::cout);
			if(!librib::Parse(file, argv[i], stream, std::cerr))
				return 3;
		}

	return 0;
}

