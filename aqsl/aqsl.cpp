// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
		\brief Implements a Renderman Shading Language compiler that generates Aqsis bytecode 
		\author Paul C. Gregory (pgregory@aqsis.com)
		\author Timothy M. Shead (tshead@k-3d.com)
*/

#include	"aqsis.h"

#include	<iostream>
#include	<fstream>

#include	"libslparse.h"
#include	"vmoutput.h"

using namespace Aqsis;

int main(int argc, char* argv[])
{
	std::cout << argv[0] << " V" << VERSION << std::endl;

	if(argc != 1)
		{
			std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
			std::cout << "" << std::endl;
			
			return 1;
		}

	if(Parse(std::cin, "stdin", std::cerr))
		OutputTree(GetParseTree());

	return 0;
}

