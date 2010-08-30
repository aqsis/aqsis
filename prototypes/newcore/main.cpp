// Aqsis
// Copyright (C) 1997 - 2010, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
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

#include <iostream>
#include <fstream>
#include <cstdlib>

#include <boost/shared_ptr.hpp>

#include "api.h"

using namespace Aqsis;

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " scene.rib\n";
        return EXIT_SUCCESS;
    }
    const char* fileName = argv[1];
    std::ifstream sceneFile(fileName, std::ios::in | std::ios::binary);
    if(!sceneFile)
    {
        std::cerr << "Could not open scene file\n";
        return EXIT_FAILURE;
    }

    boost::shared_ptr<Ri::RendererServices> renderer(createRenderer());
    renderer->parseRib(sceneFile, fileName);

#if 0
    if(sceneName == "tenpatch")
        renderTenPatchScene();
    else if(sceneName == "simpledeform")
        renderSimpleDeformationScene();
    else if(sceneName == "mbnoisetest")
        renderMbNoiseTestScene();
    else if(sceneName == "dofamounttest")
        renderDofAmountTest();
    else
        renderDefaultScene();
#endif

    return 0;
}

