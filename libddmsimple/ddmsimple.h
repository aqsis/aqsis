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
		\brief Simple example display device manager.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef DDMSIMPLE_H_INCLUDED
#define DDMSIMPLE_H_INCLUDED 1

#include	"aqsis.h"

START_NAMESPACE(Aqsis)


//---------------------------------------------------------------------
/** \struct SqDDevice
 * Simple structure storing display device request.
 */

struct SqDDevice
{
	SqDDevice()	{}
	SqDDevice(const TqChar* name, const TqChar* type, const TqChar* mode) :
				m_strName(name),
				m_strType(type),
				m_strMode(mode),
				m_pData(0)
				{}

	CqString	m_strName;
	CqString	m_strType;
	CqString	m_strMode;
	unsigned char*	m_pData;
	TqInt		m_XRes;
	TqInt		m_YRes;
	TqInt		m_SamplesPerElement;
};


//---------------------------------------------------------------------
/** \class CqDDManagerSimple
 * Class providing display device management to the renderer.
 */

class CqDDManagerSimple : public IqDDManager
{
	public:
				CqDDManagerSimple()	{}
				~CqDDManagerSimple()	{}

	// Overridden from IqDDManager
	virtual	TqInt	Initialise();
	virtual	TqInt	Shutdown();
	virtual	TqInt	AddDisplay(const TqChar* name, const TqChar* type, const TqChar* mode);
	virtual	TqInt	ClearDisplays();
	virtual	TqInt	OpenDisplays();
	virtual	TqInt	CloseDisplays();
	virtual	TqInt	DisplayBucket(IqBucket* pBucket);

	private:
		std::vector<SqDDevice>	m_aDisplayRequests;		///< Array of requested display drivers.
};


END_NAMESPACE(Aqsis)

#endif	// DDSERVER_H_INCLUDED