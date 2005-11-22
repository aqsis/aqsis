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
		\brief Implements CqPluginBase plugin loader/deloader for textures/riprocedural/shared objects for shaders
		\author Michel Joron (joron@sympatico.ca)
*/


#include <stdio.h>
#include <string.h>

#include "aqsis.h"
#include "converter.h"


START_NAMESPACE( Aqsis )


//---------------------------------------------------------------------
/** Constructor.
 * Set up the search path, library (.dll, .dso, .so) name, and function name
 */
CqConverter::CqConverter( char *searchpath, char *library, char *function )
{
    m_errorlog = "" ;
    m_dynamicsearch = searchpath ;
    m_dynamiclibrary =  library ;
    m_dynamicfunction = function ;
    m_handle = NULL;
}

//---------------------------------------------------------------------
/** Main function to call!
 * return the function pointer based on the construction information 
 * if possible otherwise it returns NULL;
 * If the return value is NULL you could call ErrorLog() to get a string
 * explaining why it failed earlier.
 */
void *CqConverter::Function()
{
    void * function_pt = NULL;

    if ( !m_handle )
        m_handle = ( void* ) DLOpen ( &m_dynamiclibrary ) ;

    if ( m_handle )
    {
        function_pt = ( void * ) DLSym(m_handle, &m_dynamicfunction );
        if ( function_pt == NULL )
        {
            m_errorlog = m_dynamicfunction + "(): " +  DLError() ;
        }
    }
    else
    {
        m_errorlog = m_dynamiclibrary + DLError() ;
    };

    return function_pt;
}

//---------------------------------------------------------------------
/** Return the current log in case of error occurred in Function();
 *  the string comes from the OS.
 */
const char * CqConverter::ErrorLog()
{
    return m_errorlog.c_str();
}
//---------------------------------------------------------------------
/** Close and unload the .dll, .dso, .so
 */
void CqConverter::Close()
{

    if ( m_handle )
        DLClose( m_handle );

    m_handle = NULL;

}

END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------

