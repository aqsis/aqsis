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
			FILE* file = fopen(argv[i],"rb");
			if(NULL == file)
				{
					std::cerr << argv[0] << ": error opening " << argv[i] << ", aborting." << std::endl;
					return 2;
				}

			librib2stream::Stream stream(std::cout);
			librib::StandardDeclarations(stream);
			if(!librib::Parse(file, argv[i], stream, std::cerr))
			{
				fclose(file);
				return 3;
			}
			fclose(file);
		}

	return 0;
}

