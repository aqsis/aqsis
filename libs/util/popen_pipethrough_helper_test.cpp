#include <string>
#include <iostream>

// Simple helper program for the popoen unit tests.
//
// The arguments and stdin are echoed to stdout in the following format:
//
// argv[0]\t
// argv[1]\t
// ...
// argv[n]\t
// end-of-args\t
// stdin_line_1\t
// stdin_line_2\t
// stdin_line_3\t
// ...
//
// We use the tab character '\t' here instead of \n to avoid differences
// between the newline formats on unix and windows - the parent test process
// writes to the pipe in raw binary mode, while stdin and stdout in this file
// are in text mode by default.
//
// The stream is flushed after the arguments and after each line of stdin to
// ensure that the reading program has something to work with immediately and
// doesn't block.
//
int main(int argc, char* argv[])
{
	bool earlyExit = false;
	for(int i = 0; i < argc; ++i)
	{
		std::cout << argv[i] << "\t";
		earlyExit |= (argv[i] == std::string("-earlyexit"));
	}
	std::cout << "end-of-args\t";
	std::cout << std::flush;
	if(earlyExit)
		return 0;

	while(true)
	{
		std::string line;
		std::getline(std::cin, line, '\t');
		if(!std::cin || std::cin.eof())
			break;
		std::cout << line << "\t";
		std::cout << std::flush;
	}

	return 0;
}
