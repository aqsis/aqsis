// Aqsis
// Copyright © 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com //
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
		\brief Implements CqConverter class.
		\author M. Joron (joron@sympatico.ca)
*/


//? Is .h included already?
#ifndef CONVERTER_H_INCLUDED
#define CONVERTER_H_INCLUDED

#include	"aqsis.h"
#include	"sstring.h"
#include	"plugins.h"
#include	<list>

START_NAMESPACE( Aqsis )


//----------------------------------------------------------------------
/** \class CqConverter
  *  A class for dynamically loading routines for on-the-fly conversion
  * of files for texture maps.
  */
class CqConverter : public CqPluginBase
{
	public:

		CqConverter( char *searchpath, char *library, char *function );

		void Close();
		void *Function();
		const TqChar * ErrorLog();

	protected:
		void *m_handle;
		void *m_function;
		CqString m_dynamiclibrary;
		CqString m_dynamicfunction;
		CqString m_dynamicsearch;
		CqString m_errorlog;
} ;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !PLUGINS_H_INCLUDED
