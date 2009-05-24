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
		\brief Implements a Renderman Shading Language compiler that generates Aqsis bytecode 
		\author Paul C. Gregory (pgregory@aqsis.org)
		\author Timothy M. Shead (tshead@k-3d.com)
*/

#include	<aqsis/aqsis.h>
#include	<aqsis/util/logging.h>
#include	<aqsis/util/logging_streambufs.h>
#include	<aqsis/util/file.h>

#include	<iostream>
#include	<fstream>
#include	<sstream>
#include	<cstdlib>
#include	<cstring>
#include	<string>
#include	<vector>
#include	<boost/scoped_ptr.hpp>

#ifdef	AQSIS_SYSTEM_WIN32
#include	"io.h"
#else
#include	"unistd.h"
#endif //AQSIS_SYSTEM_WIN32

#include	<aqsis/slcomp/libslparse.h>
#include	<aqsis/slcomp/icodegen.h>
#include	<aqsis/util/argparse.h>

#include	<aqsis/version.h>

//  Setup wave, and the cpp lexer
#include <boost/wave.hpp>
#include <boost/wave/cpplexer/cpp_lex_token.hpp>    // token class
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp> // lexer class

//
using namespace Aqsis;

extern "C" void PreProcess(int argc, char** argv);

ArgParse::apstring	g_stroutname = "";
bool	g_help = 0;
bool	g_version = 0;
ArgParse::apstringvec g_defines; // Filled in with strings to pass to the preprocessor
ArgParse::apstringvec g_includes; // Filled in with strings to pass to the preprocessor
ArgParse::apstringvec g_undefines; // Filled in with strings to pass to the preprocessor
ArgParse::apstring g_backendName = "slx"; /// Name for the comipler backend.

bool g_dumpsl = 0;
bool g_cl_no_color = false;
bool g_cl_syslog = false;
ArgParse::apint g_cl_verbose = 1;

void version( std::ostream& Stream )
{
	Stream << "aqsl version " << AQSIS_VERSION_STR_FULL << std::endl << "compiled " << __DATE__ << " " << __TIME__ << std::endl;
}


/** Process the sl file from stdin and produce an slx bytestream.
 */
int main( int argc, const char** argv )
{
	ArgParse ap;
	boost::scoped_ptr<IqCodeGen> codeGenerator;
	bool error = false; ///! Couldn't compile shader

	ap.usageHeader( ArgParse::apstring( "Usage: " ) + argv[ 0 ] + " [options] <filename>" );
	ap.argString( "o", " %s \aspecify output filename", &g_stroutname );
	ap.argStrings( "i", "%s \aSet path for #include files.", &g_includes );
	ap.argStrings( "I", "%s \aSet path for #include files.", &g_includes );
	ap.argStrings( "D", "Sym[=value] \adefine symbol <string> to have value <value> (default: 1).", &g_defines );
	ap.argStrings( "U", "Sym \aUndefine an initial symbol.", &g_undefines );
	ap.argString( "backend", " %s \aCompiler backend (default %default).  Possibilities include \"slx\" or \"dot\":\a"
			      "slx - produce a compiled shader (in the aqsis shader VM stack language)\a"
				  "dot - make a graphviz visualization of the parse tree (useful for debugging only).", &g_backendName );
	ap.argFlag( "help", "\aPrint this help and exit", &g_help );
	ap.alias("help", "h");
	ap.argFlag( "version", "\aPrint version information and exit", &g_version );
	ap.argFlag( "nocolor", "\aDisable colored output", &g_cl_no_color );
	ap.alias( "nocolor" , "nc" );
	ap.argFlag( "d", "\adump sl data", &g_dumpsl );
	ap.argInt( "verbose", "=integer\aSet log output level\n"
			   "\a0 = errors\n"
			   "\a1 = warnings (default)\n"
			   "\a2 = information\n"
			   "\a3 = debug", &g_cl_verbose );
	ap.alias( "verbose", "v" );

	if ( argc > 1 && !ap.parse( argc - 1, argv + 1 ) )
	{
		Aqsis::log() << ap.errmsg() << std::endl << ap.usagemsg();
		exit( 1 );
	}

	if ( g_version )
	{
		version( std::cout );
		exit( 0 );
	}

	if ( g_help )
	{
		std::cout << ap.usagemsg();
		exit( 0 );
	}

#ifdef	AQSIS_SYSTEM_WIN32
	std::auto_ptr<std::streambuf> ansi( new Aqsis::ansi_buf(Aqsis::log()) );
#endif

	std::auto_ptr<std::streambuf> reset_level( new Aqsis::reset_level_buf(Aqsis::log()) );
	std::auto_ptr<std::streambuf> show_timestamps( new Aqsis::timestamp_buf(Aqsis::log()) );
	std::auto_ptr<std::streambuf> fold_duplicates( new Aqsis::fold_duplicates_buf(Aqsis::log()) );
	std::auto_ptr<std::streambuf> color_level;
	if(!g_cl_no_color)
	{
		std::auto_ptr<std::streambuf> temp_color_level( new Aqsis::color_level_buf(Aqsis::log()) );
		color_level = temp_color_level;
	}
	std::auto_ptr<std::streambuf> show_level( new Aqsis::show_level_buf(Aqsis::log()) );
	Aqsis::log_level_t level = Aqsis::ERROR;
	if( g_cl_verbose > 0 )
		level = Aqsis::WARNING;
	if( g_cl_verbose > 1 )
		level = Aqsis::INFO;
	if( g_cl_verbose > 2 )
		level = Aqsis::DEBUG;
	std::auto_ptr<std::streambuf> filter_level( new Aqsis::filter_by_level_buf(level, Aqsis::log()) );
#ifdef	AQSIS_SYSTEM_POSIX

	if( g_cl_syslog )
		std::auto_ptr<std::streambuf> use_syslog( new Aqsis::syslog_buf(Aqsis::log()) );
#endif	// AQSIS_SYSTEM_POSIX

	if ( ap.leftovers().size() == 0 )
	{
		std::cout << ap.usagemsg();
		exit( 0 );
	}
	else
	{
		for ( ArgParse::apstringvec::const_iterator e = ap.leftovers().begin(); e != ap.leftovers().end(); e++ )
		{
			//Expand filenames
			std::vector<std::string> files = Aqsis::cliGlob(*e);
			std::vector<std::string>::iterator it;
			for(it = files.begin(); it != files.end(); ++it){ 
				ResetParser();
				// Create a code generator for the requested backend.
				if(g_backendName == "slx")
					codeGenerator.reset(new CqCodeGenVM());
				else if(g_backendName == "dot")
					codeGenerator.reset(new CqCodeGenGraphviz());
				else
				{
					std::cout << "Unknown backend type: \"" << g_backendName << "\", assuming slx.";
					codeGenerator.reset(new CqCodeGenVM());
				}
				// current file position is saved for exception handling
				boost::wave::util::file_position_type current_position;

				try
				{
					//  Open and read in the specified input file.
					std::ifstream instream(it->c_str());
					std::string instring;
	
					if (!instream.is_open()) 
					{
						std::cerr << "Could not open input file: " << *it << std::endl;
						continue;
					}
					instream.unsetf(std::ios::skipws);
					instring = std::string(std::istreambuf_iterator<char>(instream.rdbuf()),
					    std::istreambuf_iterator<char>());
	
					typedef boost::wave::cpplexer::lex_token<> token_type;
					typedef boost::wave::cpplexer::lex_iterator<token_type> lex_iterator_type;
					typedef boost::wave::context<std::string::iterator, lex_iterator_type>
					    context_type;
					context_type ctx (instring.begin(), instring.end(), it->c_str());

					// Append the -i arguments passed in to forward them to the preprocessor.
					for ( ArgParse::apstringvec::const_iterator include = g_includes.begin(); include != g_includes.end(); include++ )
					{
						ctx.add_sysinclude_path( include->c_str() );
						ctx.add_include_path( include->c_str() );
					}
  
					// Setup the default defines.
					ctx.add_macro_definition( "AQSIS" );
					ctx.add_macro_definition( "PI=3.141592654" );
  
					// Append the -d arguments passed in to forward them to the preprocessor.
					for ( ArgParse::apstringvec::const_iterator define = g_defines.begin(); define != g_defines.end(); define++ )
					{
						ctx.add_macro_definition( define->c_str() );
					}
  
					// Append the -u arguments passed in to forward them to the preprocessor.
					for ( ArgParse::apstringvec::const_iterator undefine = g_undefines.begin(); undefine != g_undefines.end(); undefine++ )
					{
						ctx.remove_macro_definition( undefine->c_str() );
					}
  
					// analyze the input file
					context_type::iterator_type first = ctx.begin();
					context_type::iterator_type last = ctx.end();

					std::stringstream preprocessed(std::stringstream::in | std::stringstream::out);
					std::ofstream dumpfile;
					if( g_dumpsl )
					{
						std::string dumpfname(*it);
						dumpfname.append(".pp");
						dumpfile.open(dumpfname.c_str());
					};

					while (first != last) 
					{
						current_position = (*first).get_position();
						preprocessed << (*first).get_value();
						dumpfile << (*first).get_value();

						try
						{
							++first	;
						}
						catch (boost::wave::preprocess_exception &e) 
  						{
							Aqsis::log() 
							<< e.file_name() << "(" << e.line_no() << "): "
							<< e.description() << std::endl;
							if (e.get_errorcode() == ::boost::wave::preprocess_exception::last_line_not_terminated )
							{
								preprocessed << std::endl;
								dumpfile << std::endl;
								break;
							};
  						}
						catch (...) 
						{
							throw &e;
						}

					};
  
					if( dumpfile.is_open() )
						dumpfile.close();
  
					if ( Parse( preprocessed, e->c_str(), Aqsis::log() ) )
						codeGenerator->OutputTree( GetParseTree(), g_stroutname );
					else
						error = true;
				}
				catch (boost::wave::cpp_exception &e) 
				{
					// some preprocessing error
					Aqsis::log() 
					<< e.file_name() << "(" << e.line_no() << "): "
					<< e.description() << std::endl;
					continue;
				}
				catch (std::exception &e)
				{
					// use last recognized token to retrieve the error position
					Aqsis::log() << current_position.get_file()
				        << "(" << current_position.get_line() << "): "
				        << "exception caught: " << e.what()
				        << std::endl;
					continue;
				}
				catch (...) {
					// use last recognized token to retrieve the error position
					Aqsis::log() << current_position.get_file()
					<< "(" << current_position.get_line() << "): "
					<< "unexpected exception caught." << std::endl;
					continue;
				}
			}
		};
	}

	return error;
}


