#include <iostream>
#include <fstream>

#include "ricxx2ri.h"
#include "ribsema.h"

using namespace Aqsis;

int main(int argc, char* argv[])
{
	if(argc < 2)
		return 1;
	std::ifstream inFile(argv[1]);
	if(!inFile)
		return 1;

	boost::shared_ptr<Ri::Renderer> renderer = createRibOut(std::cout);

	RibParser parser(*renderer);

	parser.parseStream(inFile, argv[1]);

	return 0;
}
