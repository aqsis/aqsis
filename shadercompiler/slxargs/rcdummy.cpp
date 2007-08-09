// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Declare a dummy render context function, so that libslxargs can be linked
	    		with the libshadervm without needing libaqsis.	
		\author Paul C. Gregory (pgregory@aqsis.org)
*/


/*
 * Define a dummy render context function, needed by the shader VM, if any initialisation code need the
 * render core, it will fail.
 */

typedef	int TqInt;
typedef float TqFloat;
namespace Aqsis
{
struct IqRenderer;
IqRenderer* QGetRenderContextI()
{
	return ( 0 );
}
void gStats_IncI( TqInt index )
{}
void gStats_DecI( TqInt index )
{}
TqInt gStats_getI( TqInt index )
{
	return( 0 );
}
void gStats_setI( TqInt index, TqInt value )
{}
TqFloat gStats_getF( TqInt index )
{
	return( 0.0f );
}
void gStats_setF( TqInt index, TqFloat value )
{}

}
