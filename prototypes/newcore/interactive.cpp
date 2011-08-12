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

#include <cfloat>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>

#include <QtGui>

#include <aqsis/riutil/ricxxutil.h>
#include <aqsis/util/file.h>

#include "interactive.h"
#include "api.h"
#include "displaymanager.h"

using namespace Aqsis;


//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

    typedef std::vector<std::string> StringVec;
    namespace po = boost::program_options;
    // optional options
    po::options_description optionsDesc("options");
    optionsDesc.add_options()
        ("help,h", "help message")
        ("stats,s", po::value<int>()->default_value(0),
         "frame statistics verbosity")
        ("threads,t", po::value<int>()->default_value(-1),
         "number of threads to use (-1 = auto)")
        ("opts", po::value<std::string>()->default_value(""),
         "frame render options")
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
            << "Usage: " << argv[0] << " [options] model.rib\n\n"
            << optionsDesc;
        return 0;
    }

    boost::shared_ptr<Ri::RendererServices> renderer(createRenderer());

    // Command line options
    Ri::Renderer& ri = renderer->firstFilter();
    ri.Option("statistics", ParamListBuilder()
              ("endofframe", opts["stats"].as<int>()));
    ri.Option("limits", ParamListBuilder()
              ("threads", opts["threads"].as<int>()));

    // Default hard-coded frame options.
    ri.PixelSamples(2, 2);
    ri.PixelFilter(renderer->getFilterFunc("sinc"), 3, 3);
    ri.ShadingRate(1);
    ri.Option("limits", ParamListBuilder()("eyesplits", 6));
    ri.Clipping(0.3, FLT_MAX);

	// Build a list of all ribFiles specified, expanding wildcards where necessary
	StringVec allFiles;
	const StringVec& ribFiles = opts["rib_files"].as<StringVec>();
	for(StringVec::const_iterator rf = ribFiles.begin(), rfEnd = ribFiles.end(); rf != rfEnd; ++rf)
	{
		std::vector<std::string> files = cliGlob(*rf);
		for(std::vector<std::string>::iterator f = files.begin(), fend = files.end(); f != fend; ++f)
			allFiles.push_back(*f);
	}

    RenderWindow win(640, 480, *renderer, opts["opts"].as<std::string>(),
                     allFiles);

	QObject::connect(&win, SIGNAL(exitApplication()), &app, SLOT(quit()));

    win.show();
	win.adjustSize();

    return app.exec();
}

