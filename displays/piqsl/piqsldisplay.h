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
		\brief Implements the default display devices for Aqsis.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#ifndef	PIQSLDISPLAY_H_INCLUDED
#define	PIQSLDISPLAY_H_INCLUDED

#include	"aqsis.h"
#include	<iostream>
#include	<string>
#include	<tinyxml.h>

#include	"ndspy.h"
#include	"socket.h"

START_NAMESPACE( Aqsis )

struct SqDisplayInstance
{
	std::string		m_filename;
	std::string		m_hostname;
	TqInt			m_port;
	CqSocket		m_socket;
	// The number of pixels that have already been rendered (used for progress reporting)
	TqInt		m_pixelsReceived;

	friend std::istream& operator >>(std::istream &is,struct SqDisplayInstance &obj);
	friend std::ostream& operator <<(std::ostream &os,const struct SqDisplayInstance &obj);
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	//	PIQSLDISPLAY_H_INCLUDED
