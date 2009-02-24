#include <string>
#include <iostream>

// Simple helper program for the popoen unit tests.
//
// The arguments and stdin are echoed to stdout in the following format:
//
// argv[0]
// argv[1]
// ...
// argv[n]
// end-of-args
// stdin_line_1
// stdin_line_2
// stdin_line_3
// ...
//
// The stream is flushed after the arguments and after each line of stdin to
// ensure that the reading program has something to work with immediately and
// doesn't block.
//
int main(int argc, char* argv[])
{
	for(int i = 0; i < argc; ++i)
		std::cout << argv[i] << "\r";
	std::cout << "end-of-args\r";
	std::cout << std::flush;

	while(true)
	{
		std::string line;
		std::getline(std::cin, line);
		if(line.size() == 0)
			break;
		std::cout << line << "\r";
		std::cout << std::flush;
	}

	return 0;
}
