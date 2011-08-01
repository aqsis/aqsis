// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

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
