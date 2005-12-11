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
		\brief Declares the CqRiFile class for handling files with RenderMan searchpath option support.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef RIFILE_H_INCLUDED
#define RIFILE_H_INCLUDED 1

#include	<iostream>

#include	"aqsis.h"
#include	"file.h"
#include	"renderer.h"

START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** \class CqRiFile
 * \brief Standard handling of all file types utilising the searchpath options.
 */

class CqRiFile : public CqFile
{
	public:
		/** Default constructor
		 */
		CqRiFile() : CqFile( 0 )
		{}
		/** Constructor taking an open stream pointer and a name.
		 * \param Stream a pointer to an already opened input stream to attach this object to.
		 * \param strRealName the name of the file associated with this stream.
		 */
		CqRiFile( std::istream* Stream, const char* strRealName ) :
				CqFile( Stream, strRealName )
		{}
		CqRiFile( const char* strFilename, const char* strSearchPathOption = "" )
		{
			//									std::cout << "CqRiFile::CqRiFile() " << strFilename << " - " << strSearchPathOption << std::endl;
			Open( strFilename, strSearchPathOption );
		}
		/** Destructor. Takes care of closing the stream if the constructor opened it.
		 */
		virtual	~CqRiFile()
		{}


		void	Open( const char* strFilename, const char* strSearchPathOption = "", std::ios::openmode mode = std::ios::in )
		{
			CqString SearchPath( "" );
			if ( strSearchPathOption != "" )
			{
				// if not found there, search in the specified option searchpath.
				const CqString * poptShader = QGetRenderContext() ->optCurrent().GetStringOption( "searchpath", strSearchPathOption );
				if ( poptShader != 0 )
					SearchPath = poptShader[ 0 ];

				//std::cout << "\t" << SearchPath.c_str() << std::endl;
			}
			CqFile::Open( strFilename, SearchPath.c_str(), mode );
			// If the file was not found, then try the "resource" searchpath.
			if( !IsValid() )
			{
				// if not found there, search in the specified option searchpath.
				const CqString * poptResource = QGetRenderContext() ->optCurrent().GetStringOption( "searchpath", "resource" );
				if ( poptResource != 0 )
				{
					SearchPath = poptResource[ 0 ];
					CqFile::Open( strFilename, SearchPath.c_str(), mode );
				}
			}
		}

};

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !RIFILE_H_INCLUDED
