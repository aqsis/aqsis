#include "ri.h"
#include "librib.h"
#include "librib2ri.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

void usage(const std::string Command, std::ostream& Stream)
{
	Stream << "usage: " << Command << " [options] ribfile1 [ribfile2 ...]" << std::endl;
	Stream << std::endl;
	Stream << "  -h, --help               prints this help information and exits" << std::endl;
	Stream << "      --version            prints program version information and exits" << std::endl;
	Stream << "  -n, --nostandard    disables declaration of standard Renderman arguments" << std::endl;
	Stream << std::endl;
}

void version(std::ostream& Stream)
{
	Stream << "aqsis version " << VERSION << std::endl;
}

int main(int argc, char* argv[])
{
	// Create an array of arguments ...
	typedef std::vector<std::string> strings;
	strings arguments;
	for(int i = 1; i < argc; i++)
		arguments.push_back(argv[i]);

	// Default options ...
	bool standard_declarations = true;

	// Look at arguments ...
	for(strings::iterator argument = arguments.begin(); argument != arguments.end();)
		{
			if((*argument == "-h") || (*argument == "--help"))
				{
					argument = arguments.erase(argument);
					usage(argv[0], std::cout);
					return 0;
				}
			else if(*argument == "--version")
				{
					argument = arguments.erase(argument);
					version(std::cout);
					return 0;
				}
			else if((*argument == "-n") || (*argument == "--nostandard"))
				{
					argument = arguments.erase(argument);
					standard_declarations = false;
				}
			else
				{
					argument++;
				}
		}

	// Begin rendering ...
	RiBegin(argv[0]);
	librib2ri::Engine renderengine;

	// Declare standard Renderman arguments ...
	if(standard_declarations)
		librib::StandardDeclarations(renderengine);

	// For each RIB file specified on the command-line ...
	for(strings::iterator argument = arguments.begin(); argument != arguments.end(); argument++)
		{
			std::ifstream file(argument->c_str());
			if(!file.good())
				{
					std::cerr << argv[0] << ": error opening " << *argument << ", aborting." << std::endl;
					return 2;
				}

			if(!librib::Parse(file, *argument, renderengine, std::cerr))
				return 3;
		}
		
	// If there weren't any RIB files, read from stdin ...
	if(0 == arguments.size())
		if(!librib::Parse(std::cin, "stdin", renderengine, std::cerr))
			return 3;
		
	RiEnd();

	return 0;
}


