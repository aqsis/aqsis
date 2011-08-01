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

/** \file
		\brief Implements the default display devices for Aqsis.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include <aqsis/aqsis.h>

#include <QtGui/QApplication>

#include <tinyxml.h>

#include <aqsis/util/argparse.h>
#include <aqsis/util/logging.h>
#include <aqsis/util/logging_streambufs.h>
#include <aqsis/version.h>

#include "piqsl_ui.h"

using namespace Aqsis;


int main( int argc, char** argv )
{
	ArgParse ap;

	// Set up the options
	ArgParse::apstring strInterface = "127.0.0.1";
	ArgParse::apstring strPort = "49515";
	bool displayHelp = 0;
	bool displayVersion = 0;
	bool cl_no_color = false;
	bool cl_syslog = false;
	ArgParse::apint cl_verbose = 1;

	ap.usageHeader( ArgParse::apstring( "Usage: " ) + argv[ 0 ] + " [options] [BKS file...] [Image file...]" );
	ap.argString( "i", "\aSpecify the IP address to listen on (default: %default)", &strInterface );
	ap.argString( "p", "\aSpecify the TCP port to listen on (default: %default)", &strPort );
	ap.argFlag( "help", "\aPrint this help and exit", &displayHelp );
	ap.alias( "help" , "h" );
	ap.argFlag( "version", "\aPrint version information and exit", &displayVersion );
	ap.argFlag( "nocolor", "\aDisable colored output", &cl_no_color );
	ap.alias( "nocolor", "nc" );
	ap.argInt( "verbose", "=integer\aSet log output level\n"
		"\a0 = errors\n"
		"\a1 = warnings (default)\n"
		"\a2 = information\n"
		"\a3 = debug", &cl_verbose );
	ap.alias( "verbose", "v" );

#ifdef  AQSIS_SYSTEM_POSIX
	ap.argFlag( "syslog", "\aLog messages to syslog", &cl_syslog );
#endif  // AQSIS_SYSTEM_POSIX

	QApplication app(argc, argv);

	const char **c_argv = const_cast<const char**>(argv);
	if ( argc > 1 && !ap.parse( argc - 1, c_argv + 1 ) )
	{
		std::cerr << ap.errmsg() << std::endl << ap.usagemsg();
		exit( 1 );
	}

	if ( displayHelp )
	{
		std::cerr << ap.usagemsg();
		exit( 0 );
	}

	if ( displayVersion )
	{
		std::cout << "piqsl version " << AQSIS_VERSION_STR_FULL << std::endl
				  << "compiled " << __DATE__ << " " << __TIME__ << std::endl;
		exit( 0 );
	}

#ifdef  AQSIS_SYSTEM_WIN32
	std::auto_ptr<std::streambuf> ansi( new Aqsis::ansi_buf(std::cerr) );
#endif
	std::auto_ptr<std::streambuf> reset_level( new Aqsis::reset_level_buf(std::cerr) );
	std::auto_ptr<std::streambuf> show_timestamps( new Aqsis::timestamp_buf(std::cerr) );
	std::auto_ptr<std::streambuf> fold_duplicates( new Aqsis::fold_duplicates_buf(std::cerr) );
	std::auto_ptr<std::streambuf> color_level;
	if(!cl_no_color)
	{
		std::auto_ptr<std::streambuf> temp_color_level( new Aqsis::color_level_buf(std::cerr) );
		color_level = temp_color_level;
	}
	std::auto_ptr<std::streambuf> show_level( new Aqsis::show_level_buf(std::cerr) );
	Aqsis::log_level_t level = Aqsis::ERROR;
	if( cl_verbose > 0 )
		level = Aqsis::WARNING;
	if( cl_verbose > 1 )
		level = Aqsis::INFO;
	if( cl_verbose > 2 )
		level = Aqsis::DEBUG;
	std::auto_ptr<std::streambuf> filter_level( new Aqsis::filter_by_level_buf(level, Aqsis::log()) );

#ifdef  AQSIS_SYSTEM_POSIX
	if( cl_syslog )
		std::auto_ptr<std::streambuf> use_syslog( new Aqsis::syslog_buf(std::cerr) );
#endif  // AQSIS_SYSTEM_POSIX

	int portno = atoi(strPort.c_str());

	QStringList filesToOpen;
	for(int i = 0; i < (int)ap.leftovers().size(); ++i)
		filesToOpen.push_back(QString::fromStdString(ap.leftovers()[i]));

	Aqsis::PiqslMainWindow window(QString::fromStdString(strInterface),
								  portno, filesToOpen);
	window.show();

	return app.exec();
}


