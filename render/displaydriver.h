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
		\brief Display driver message structures.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is context.h included already?
#ifndef DISPLAYDRIVER_H_INCLUDED
#define DISPLAYDRIVER_H_INCLUDED 1

#include	"aqsis.h"

START_NAMESPACE(Aqsis)


enum EqDDMessageIDs
{
	MessageID_String=0,
};

//---------------------------------------------------------------------
/** \struct SqDDMessageBase 
 * Base class from which all DD messages are derived.
 */

struct SqDDMessageBase
{
	TqInt	m_MessageID;
	TqInt	m_MessageLength;
	TqInt	m_DataLength;

	void	Destroy()	{free(this);}
};


//---------------------------------------------------------------------
/** \struct SqDDMessageString
 * Base class from which all DD messages are derived.
 */

struct SqDDMessageString : public SqDDMessageBase
{
	// Specific message data
	TqChar	m_Data[1];

	static SqDDMessageString*	Construct(const TqChar* string);
};


inline SqDDMessageString* SqDDMessageString::Construct(const TqChar* string)
{
	SqDDMessageString* pMessage=reinterpret_cast<SqDDMessageString*>(malloc(sizeof(SqDDMessageString)+strlen(string)));
	pMessage->m_MessageID=MessageID_String;
	pMessage->m_DataLength=strlen(string);
	pMessage->m_MessageLength=sizeof(SqDDMessageString)+pMessage->m_DataLength;
	memcpy(pMessage->m_Data,string,pMessage->m_DataLength+1);	// Copy the null terminator as well.

	return(pMessage);
}

//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif // DISPLAYDRIVER_H_INCLUDED