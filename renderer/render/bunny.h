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
		\brief Implements RiGeometry "bunny" option.
		\author Michel Joron (joron@sympatico.ca)
*/
/*    References:
 *
 *       The Stanford 3D Scanning Repository
 *           http://graphics.stanford.edu/data/3Dscanrep/#bunny|Bunny]
 */

//? Is .h included already?
#ifndef BUNNY_H_INCLUDED
#define BUNNY_H_INCLUDED

#include	"aqsis.h"
#include	"ri.h"

START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** \class CqBunny
 * Class encapsulating the information of the Stanford' bunny
 */

class CqBunny
{
	public:
		CqBunny( );
		CqBunny( const CqBunny& From )
		{
			*this = From;
		}
		virtual	~CqBunny()
		{}

		/**
		* Return the Points information 
		              */
		TqFloat *Points( );

		/**
		* Return the Normals information 
		              */
		TqFloat *Normals( );

		/**
		* Return the s,t information 
		              */
		TqFloat *STs( );


		/**
		* Return the list of 3 side polygon information 
		              */
		TqInt *Indexes( );

		/**
		* Return the Vertices information 
		              */
		TqInt *Vertices( );

		/**
		* Return the size of the Indexes
		              */
		TqInt NVertices( );

	private:

	protected:
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !BUNNY_H_INCLUDED

