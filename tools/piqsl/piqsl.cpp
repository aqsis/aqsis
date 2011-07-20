// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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


