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
 *  \brief This class implements the RiContext feature of RiSpec V3.2
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#include "aqsis.h"
#include "context.h"
#include "ascii.h"

using namespace libri2rib;


CqContext::CqContext()
{
    active=(CqASCII *)RI_NULL;
}

void  CqContext::addContext()
{
    active=new CqASCII; 
    chl.push_back(active);
}

RtContextHandle CqContext::getContext()
{
    return (RtContextHandle) active;
}

CqASCII & CqContext::current()
{
    if (active==((CqASCII *) RI_NULL)) {
	throw CqError(RIE_BUG, RIE_SEVERE, "No active context", TqFalse);
    }
    return *active;
}

void CqContext::switchTo(RtContextHandle ch)
{
    list<CqASCII *>::iterator first=chl.begin();
    CqASCII *r=(CqASCII *)ch;
    for (;first!=chl.end();first++) {
	if (*first==r) { 
	    active=r;
	    return;
	}
    }
    throw CqError (RIE_BUG, RIE_SEVERE, "Invalid Context Handle", TqFalse);
}

void CqContext::removeCurrent()
{
    list<CqASCII *>::iterator first=chl.begin();
    for (;first!=chl.end();first++) {
	if (*first==active) {
	    delete *first;
	    chl.erase(first);
	    active = (CqASCII *)RI_NULL;
	    return;
	}
    } 
}
