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
		\brief Declares the base message handling functionality required by display drivers.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is context.h included already?
#ifndef DD_H_INCLUDED
#define DD_H_INCLUDED 1

#include	"aqsis.h"
#include	"displaydriver.h"

START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------

TqInt DDReceiveSome(TqInt s,void* buffer, TqInt len);
TqInt DDReceiveMsg(TqInt s, SqDDMessageBase*& pMsg);
TqInt DDSendSome(TqInt s,void* buffer, TqInt len);
TqInt DDSendMsg(TqInt s, SqDDMessageBase* pMsg);
TqInt DDInitialise(const TqChar* phostname, TqInt port);
TqInt DDProcessMessages();

//----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif // DD_H_INCLUDED