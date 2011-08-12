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

#include <iostream>
#include <fstream>
#include <cstdlib>

#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>

#include <aqsis/riutil/ricxxutil.h>

#include "api.h"

using namespace Aqsis;

int main(int argc, char* argv[])
{
    typedef std::vector<std::string> StringVec;
    namespace po = boost::program_options;
    // optional options
    po::options_description optionsDesc("options");
    optionsDesc.add_options()
        ("help,h", "help message")
        ("stats,s", po::value<int>()->default_value(1),
         "frame statistics verbosity")
        ("threads,t", po::value<int>()->default_value(-1),
         "number of threads to use (-1 = auto)")
    ;
    // options + positional parameters
    po::options_description allArgs("All arguments");
    allArgs.add(optionsDesc);
    allArgs.add_options()
        ("rib_files", po::value<StringVec>(), "RIB files to render")
    ;
    po::positional_options_description params;
    params.add("rib_files", -1);
    // Parse options
    po::variables_map opts;
    po::store(po::command_line_parser(argc, argv)
              .options(allArgs).positional(params).run(), opts);
    po::notify(opts);

    if(opts.count("help") || opts.count("rib_files") == 0)
    {
        std::cout
            << "Usage: " << argv[0] << " [options] scene.rib\n\n"
            << optionsDesc;
        return 0;
    }

    boost::shared_ptr<Ri::RendererServices> renderer(createRenderer());

    renderer->firstFilter().Option("statistics", ParamListBuilder()
                            ("endofframe", opts["stats"].as<int>()));
    renderer->firstFilter().Option("limits", ParamListBuilder()
                            ("threads", opts["threads"].as<int>()));


    const StringVec& ribFiles = opts["rib_files"].as<StringVec>();
    for(int ifile = 0; ifile < (int)ribFiles.size(); ++ifile)
    {
        const char* fileName = ribFiles[ifile].c_str();
        std::ifstream sceneFile(fileName, std::ios::in | std::ios::binary);
        if(!sceneFile)
        {
            std::cerr << "Could not open file \"" << fileName << "\"\n";
            return 1;
        }
        renderer->parseRib(sceneFile, fileName);
    }

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

