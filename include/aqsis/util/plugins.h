// Aqsis
// Copyright (C) 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org //
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
		\brief Implements CseqPlugin class.
		\author M. Joron (joron@sympatico.ca)
		\author T. Colgate (tristan@inuxtech.co.uk)
*/


//? Is .h included already?
#ifndef PLUGINS_H_INCLUDED
#define PLUGINS_H_INCLUDED

#include	<aqsis/aqsis.h>
#include	<aqsis/util/sstring.h>
#include	<aqsis/util/exception.h>
#include	<list>

namespace Aqsis {

//----------------------------------------------------------------------
/** \class CqPluginBase
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

class AQSIS_UTIL_SHARE CqPluginBase
{
	public:
		virtual ~CqPluginBase();
		const CqString DLError();

	private:


	protected:
		void *DLOpen( CqString *library );
		void DLClose( void* );
		void *DLSym( void*, CqString* );

		// we record all the DLOpen'ed handles to close them properly on destruction.
		std::list<void*> m_activeHandles;

} ;


//----------------------------------------------------------------------
/** \class CqPluginBase
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

class CqSimplePlugin : public CqPluginBase
{
	public:
		virtual ~CqSimplePlugin()
		{}

		void *SimpleDLOpen( CqString *library )
		{
			return(DLOpen( library ) );
		}
		void SimpleDLClose( void* handle )
		{
			DLClose( handle );
		}
		void *SimpleDLSym( void* handle, CqString* name )
		{
			return( DLSym( handle, name ) );
		}
	private:
	protected:
} ;


/** \class XqPluginError
 * \brief Errors related to plugin loading and usage
 * Errors which should be signalled by XqPluginError include trying to open
 * non-existant plugin, and trying to use a plugin incorrectly.
 */
AQSIS_DECLARE_XQEXCEPTION(XqPluginError, XqInternal);

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !PLUGINS_H_INCLUDED
