// Aqsis
// Copyright © 2001, Paul C. Gregory
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
		\brief Implements CqPlugins class.
		\author M. Joron (joron@sympatico.ca)
*/ 


//? Is .h included already?
#ifndef PLUGINS_H_INCLUDED
#define PLUGINS_H_INCLUDED

#include	"aqsis.h"

START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** \class CqPlugins
 * Class encapsulating the functionality of Plugins. It hides the intersect
 * dynamic loading up some function from a .dll on NT or .so on Unix.
 * The macosx works if you download dlopen package from dev apple.
 *  I will later hide its implementation in this class.
 *
 * This class will be used for loading/unloading the bitmap converter 
 * in texturemap.cpp. In ri.cpp to implement RiProcedural and later in
 * the shadervm to loading up any user shared routine in .sl 
 *  
 */

class CqPlugins 
{
	public:

        	CqPlugins(char *searchpath, char *library, char *function);
        	void Close();
        	void *Function();

	private:
	
	protected:
		void *handle;
		void *function;
		TqChar dynamiclibrary[ 1024 ];
		TqChar dynamicfunction[ 1024 ];
		TqChar dynamicsearch[ 1024 ];
}
;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !PLUGINS_H_INCLUDED
