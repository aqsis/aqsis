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
		\brief Display driver server handler.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is context.h included already?
#ifndef DDMSOCK2_H_INCLUDED
#define DDMSOCK2_H_INCLUDED 1

#include	<vector>
#include	<string>
#include	<map>

#ifdef AQSIS_SYSTEM_WIN32

#include	<winsock2.h>

#else // AQSIS_SYSTEM_WIN32

typedef int SOCKET;

#endif // !AQSIS_SYSTEM_WIN32

#include	"aqsis.h"
#include	"bucketcache.h"
#include	"listener.h"
#include	"sstring.h"
#include	"iddmanager.h"
#include	"logging.h"

#ifdef	AQSIS_SYSTEM_WIN32
#pragma warning(disable : 4275 4251)
#endif

#include	<boost/thread.hpp>

#define		AQSIS_DD_PORT	277472	///< Aqsis display driver port ( AQSIS on phone keypad )

START_NAMESPACE( Aqsis )

//---------------------------------------------------------------------
/** \class CqDDManager
 * Class providing display device management to the renderer.
 */

class CqDDManager : public IqDDManager
{
public:
    CqDDManager();
    virtual ~CqDDManager();

    // Overridden from IqDDManager
    virtual	TqInt	Initialise(IqRenderer*);
    virtual	TqInt	Shutdown();
    virtual	TqInt	AddDisplay( const TqChar* name, const TqChar* type, const TqChar* mode, TqInt modeID, TqInt dataOffset, TqInt dataSize, std::map<std::string, void*> mapOfArguments );
    virtual	TqInt	ClearDisplays();
    virtual	TqInt	OpenDisplays(IqRenderer*);
    virtual	TqInt	CloseDisplays();
    virtual	TqInt	DisplayBucket( IqBucket* pBucket );
    virtual TqBool	fDisplayNeeds( const TqChar* var );
    virtual TqInt	Uses();

    void	InitialiseDisplayNameMap();

	CqBucketDiskStore&	getDiskStore()
			{
				return(m_DiskStore);
			}
	boost::condition* BucketReadyCondition(TqInt index)
			{
				if(m_BucketRequestsWaiting.find(index)==m_BucketRequestsWaiting.end())
					m_BucketRequestsWaiting[index] = new boost::condition();
				return(m_BucketRequestsWaiting[index]);
			}
	boost::mutex& getBucketsLock()
			{
				return(m_BucketsLock);
			}
	boost::mutex& StoreAccess()
			{
				return(m_StoreAccess);
			}

private:
    std::string	GetStringField( const std::string& s, int idx );

	struct SqDisplayRequest
	{
		std::string m_name;
		std::string m_type;
		std::string m_mode;
		TqInt		m_modeHash;
		TqInt		m_modeID;
		TqInt		m_dataOffset;
		TqInt		m_dataSize;
	};

	void	LoadDisplayLibrary( SqDisplayRequest& req );

private:
	CqBucketDiskStore	m_DiskStore;
	CqDisplayListener	m_Listener;
	boost::mutex		m_BucketsLock;
	std::map<TqInt, boost::condition*>	m_BucketRequestsWaiting;
	std::vector<SqDisplayRequest>	m_displayRequests;
	boost::mutex		m_StoreAccess;
}
;



END_NAMESPACE( Aqsis )

#endif	// DDMSOCK2_H_INCLUDED
