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
		\brief Declare display device manager interface.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef IDDMANAGER_H_INCLUDED
#define IDDMANAGER_H_INCLUDED 1

#include	"aqsis.h"


START_NAMESPACE(Aqsis)

struct SqImageValue;

struct IqBucket
{
	virtual	TqInt	XSize() const=0;
	virtual	TqInt	YSize() const=0;
	virtual	TqInt	XOrigin() const=0;
	virtual	TqInt	YOrigin() const=0;
	virtual	TqInt	XFWidth() const=0;
	virtual	TqInt	YFWidth() const=0;

	virtual	TqBool	FilteredElement(TqInt iXPos, TqInt iYPos, SqImageValue& Val)=0;
	virtual	TqBool	Element(TqInt iXPos, TqInt iYPos, SqImageValue& Val)=0;
	virtual	void	ExposeElement(SqImageValue& Val)=0;
	virtual	void	QuantizeElement(SqImageValue& Val)=0;
};

struct IqDDManager
{
	virtual	TqInt	Initialise()=0;
	virtual	TqInt	Shutdown()=0;
	virtual	TqInt	AddDisplay(const TqChar* name, const TqChar* type, const TqChar* mode)=0;
	virtual	TqInt	ClearDisplays()=0;
	virtual	TqInt	OpenDisplays()=0;
	virtual	TqInt	CloseDisplays()=0;
	virtual	TqInt	DisplayBucket(IqBucket* pBucket)=0;
};


END_NAMESPACE(Aqsis)

#endif	// IDDMANAGER_H_INCLUDED