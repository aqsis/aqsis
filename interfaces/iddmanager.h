// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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
		\brief Declare display device manager interface.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef IDDMANAGER_H_INCLUDED
#define IDDMANAGER_H_INCLUDED 1

#include	"aqsis.h"
#include	"color.h"
#include	"region.h"
#include	<map>
#include	<boost/shared_ptr.hpp>


namespace Aqsis {

struct SqImageSample;
struct IqRenderer;
class CqParameter;

struct IqChannelBuffer
{
	public:
		virtual ~IqChannelBuffer() {}	

		virtual TqUint width() const = 0;
		virtual TqUint height() const = 0;
		virtual TqUint getChannelIndex(const std::string& name) const = 0;
		virtual std::vector<TqFloat>::const_iterator operator()(TqUint x, TqUint y, TqUint index) const = 0;
};

struct IqDDManager
{
	virtual ~IqDDManager()
	{
	};
	/** Initialise the device manager.
	 */
	virtual	TqInt	Initialise() = 0;
	/** Shutdown the device manager.
	 */
	virtual	TqInt	Shutdown() = 0;
	/** Add a display request to the managers list.
	 */
	virtual	TqInt	AddDisplay( const TqChar* name, const TqChar* type, const TqChar* mode, TqInt modeID, TqInt dataOffset, TqInt dataSize, std::map<std::string, void*> mapOfArguments ) = 0;
	/** Clear all display requests from the managers list.
	 */
	virtual	TqInt	ClearDisplays() = 0;
	/** Open all displays in the managers list.
	 */
	virtual	TqInt	OpenDisplays(TqInt width, TqInt height) = 0;
	/** Close all displays in the managers list, rendering is finished.
	 */
	virtual	TqInt	CloseDisplays() = 0;
	/** Display a bucket.
	 */
	virtual	TqInt	DisplayBucket( const CqRegion& DRegion, const IqChannelBuffer* pBuffer ) = 0;
	/** Determine if any of the displays need the named shader variable.
	 */
	virtual bool	fDisplayNeeds( const TqChar* var) = 0;
	/** Determine if any of the displays need the named shader variable.
	 */
	virtual TqInt	Uses( ) = 0;
};

} // namespace Aqsis

#endif	// IDDMANAGER_H_INCLUDED

