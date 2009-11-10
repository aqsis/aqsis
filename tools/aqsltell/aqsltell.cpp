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
		\brief A program to test libslxargs.
		\author Douglas Ward (dsward@vidi.com)
*/

#include <aqsis/aqsis.h>
#include <aqsis/util/logging.h>

#ifdef AQSIS_SYSTEM_WIN32
#include <windows.h>
#endif

#ifdef	AQSIS_COMPILER_MSVC6
#pragma warning(disable : 4786 )
#endif //AQSIS_COMPILER_MSVC6

#include <aqsis/util/argparse.h>
#include <aqsis/aqsis.h>
#include <aqsis/util/file.h>
#include <aqsis/ri/slx.h>
#include <aqsis/ri/slo.h>
#include <aqsis/version.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>

#include <cstdlib>
#include <cstring>


namespace Aqsis
{
struct IqRenderer;
extern IqRenderer* QGetRenderContextI();
}

std::string g_shader_path;
bool g_cl_help = 0;
bool g_cl_version = 0;
ArgParse::apstring g_cl_shader_path = "";

int main( int argc, const char** argv )
{

#ifdef	AQSIS_SYSTEM_WIN32
	char acPath[256];
	char rootPath[256];
	if( GetModuleFileName( NULL, acPath, 256 ) != 0)
	{
		// guaranteed file name of at least one character after path
		*( strrchr( acPath, '\\' ) + 1 ) = '\0';
		std::string	 stracPath(acPath);
		stracPath.append("..\\");
		_fullpath(rootPath,&stracPath[0],256);
	}
	g_shader_path = rootPath;
	g_shader_path.append( "shaders" );
#elif defined(AQSIS_SYSTEM_MACOSX)
#else

	g_shader_path = AQSIS_XSTR(DEFAULT_SHADER_PATH);
#endif

	/*Aqsis::QGetRenderContextI();*/
	ArgParse ap;
	ap.usageHeader( ArgParse::apstring( "Usage: " ) + argv[ 0 ] + " <shadername>" );
	ap.argFlag( "help", "\aPrint this help and exit", &g_cl_help );
	ap.alias( "help" , "h" );
	ap.argFlag( "version", "\aPrint version information and exit", &g_cl_version );
	ap.argString( "shaders", "=string\aOverride the default shader searchpath(s) [" + g_shader_path + "]", &g_cl_shader_path );

	if ( argc > 1 && !ap.parse( argc - 1, argv + 1 ) )
	{
		Aqsis::log() << ap.errmsg() << std::endl << ap.usagemsg();
		exit( 1 );
	}

	if ( g_cl_help )
	{
		std::cout << ap.usagemsg();
		exit( 0 );
	}

	if ( g_cl_version )
	{
		std::cout << "aqsltell version " << AQSIS_VERSION_STR_FULL << std::endl << "compiled " << __DATE__ << " " << __TIME__ << std::endl;
		exit( 0 );
	}

	// Apply environment-variable overrides to default paths ...
	if(getenv("AQSIS_SHADER_PATH"))
		g_shader_path = getenv("AQSIS_SHADER_PATH");

	// Apply command-line overrides to default paths ...
	if(!g_cl_shader_path.empty())
		g_shader_path = g_cl_shader_path;

	// Any leftovers are presumed to be shader names.
	if ( ap.leftovers().size() == 0 )     // If no files specified, take input from stdin.
	{
		Aqsis::log() << ap.errmsg() << std::endl << ap.usagemsg();
		exit( 1 );
	}
	else
	{
		for ( ArgParse::apstringvec::const_iterator e = ap.leftovers().begin(); e != ap.leftovers().end(); e++ )
		{
			SLX_SetPath( const_cast<char*>( g_shader_path.c_str() ) );
			SLX_SetDSOPath( const_cast<char*>( g_shader_path.c_str() ) );
			Slo_SetShader( ( char* ) e->c_str() );

			if ( SLX_SetShader( ( char* ) e->c_str() ) == 0 )
			{
				// SLX_SetShader successful
				int	nArgs;
				int i;
				SLX_VISSYMDEF * symPtr;

				std::cout << SLX_TypetoStr( SLX_GetType() ) << " \"" << Slo_GetName() << "\"" << std::endl;
				nArgs = SLX_GetNArgs();

				for ( i = 0; i < nArgs; i++ )
				{
					symPtr = SLX_GetArgById( i );
					if ( symPtr != NULL )
					{
						TqInt arrayLen = 1;
						if ( symPtr->svd_arraylen != 0 )
							arrayLen = symPtr->svd_arraylen;

						std::cout << "    \"" << symPtr->svd_name << "\" \""
							<< SLX_StortoStr( symPtr->svd_storage ) << " "
							<< SLX_DetailtoStr( symPtr->svd_detail ) << " "
							<< SLX_TypetoStr( symPtr->svd_type );

						if ( symPtr->svd_arraylen != 0 )
							std::cout << "[" << arrayLen << "]";

						std::cout << "\"" << std::endl;

						TqInt arrayIndex;
						for ( arrayIndex = 0; arrayIndex < arrayLen; arrayIndex++ )
						{
							std::cout << "\t\tDefault value: ";

							if ( symPtr->svd_spacename != NULL )
							{
								if ( ( symPtr->svd_type == SLX_TYPE_POINT ) ||
								        ( symPtr->svd_type == SLX_TYPE_NORMAL ) ||
								        ( symPtr->svd_type == SLX_TYPE_VECTOR ) ||
								        ( symPtr->svd_type == SLX_TYPE_MATRIX ) ||
								        ( symPtr->svd_type == SLX_TYPE_COLOR ) )
									std::cout << "\"" << symPtr->svd_spacename << "\" ";
							}

							if ( symPtr->svd_default.stringval != NULL )
							{
								switch ( symPtr->svd_type )
								{
										case SLX_TYPE_UNKNOWN:
										std::cout << "unknown" << std::endl;
										break;
										case SLX_TYPE_POINT:
										case SLX_TYPE_NORMAL:
										case SLX_TYPE_VECTOR:
										std::cout << "[" << symPtr->svd_default.pointval[ arrayIndex ].xval << " " <<
										symPtr->svd_default.pointval[ arrayIndex ].yval << " " <<
										symPtr->svd_default.pointval[ arrayIndex ].zval <<
										"]" << std::endl;
										break;
										case SLX_TYPE_COLOR:
										std::cout << "[" << symPtr->svd_default.pointval[ arrayIndex ].xval << " " <<
										symPtr->svd_default.pointval[ arrayIndex ].yval << " " <<
										symPtr->svd_default.pointval[ arrayIndex ].zval <<
										"]" << std::endl;
										break;
										case SLX_TYPE_SCALAR:
										std::cout << symPtr->svd_default.scalarval[ arrayIndex ] << std::endl;
										break;
										case SLX_TYPE_STRING:
										std::cout << "\"" << symPtr->svd_default.stringval[ arrayIndex ] << "\"" << std::endl;
										break;
										case SLX_TYPE_MATRIX:
										std::cout << "[" <<
										symPtr->svd_default.matrixval[ arrayIndex ].val[0][0] << " " <<
										symPtr->svd_default.matrixval[ arrayIndex ].val[0][1] << " " <<
										symPtr->svd_default.matrixval[ arrayIndex ].val[0][2] << " " <<
										symPtr->svd_default.matrixval[ arrayIndex ].val[0][2] << " " <<
										symPtr->svd_default.matrixval[ arrayIndex ].val[1][0] << " " <<
										symPtr->svd_default.matrixval[ arrayIndex ].val[1][1] << " " <<
										symPtr->svd_default.matrixval[ arrayIndex ].val[1][2] << " " <<
										symPtr->svd_default.matrixval[ arrayIndex ].val[1][2] << " " <<
										symPtr->svd_default.matrixval[ arrayIndex ].val[2][0] << " " <<
										symPtr->svd_default.matrixval[ arrayIndex ].val[2][1] << " " <<
										symPtr->svd_default.matrixval[ arrayIndex ].val[2][2] << " " <<
										symPtr->svd_default.matrixval[ arrayIndex ].val[2][3] << " " <<
										symPtr->svd_default.matrixval[ arrayIndex ].val[3][0] << " " <<
										symPtr->svd_default.matrixval[ arrayIndex ].val[3][1] << " " <<
										symPtr->svd_default.matrixval[ arrayIndex ].val[3][2] << " " <<
										symPtr->svd_default.matrixval[ arrayIndex ].val[3][3] <<
										"]" << std::endl;
										break;
										default:
										std::cout << "unknown" << std::endl;
										break;
								}
							}
						}
					}
					else
					{
						printf( "ERROR - null pointer to value\n" );
					}
					//std::cout << std::endl;
				}

				SLX_EndShader();
			}
			else
			{
				printf("ERROR - could not read shader file\n");
			}
		}
	}

	return ( 0 );
}

