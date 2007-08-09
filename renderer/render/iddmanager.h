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
		\brief Declare display device manager interface.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef IDDMANAGER_H_INCLUDED
#define IDDMANAGER_H_INCLUDED 1

#include	"aqsis.h"
#include	"color.h"
#include	<map>
#include	<boost/shared_ptr.hpp>


START_NAMESPACE( Aqsis )


struct IqBucket
{
	virtual ~IqBucket()
	{
	};
	/** Get the bucket size in X
	 */
	virtual	TqInt	Width() const = 0;
	/** Get the bucket size in Y
	 */
	virtual	TqInt	Height() const = 0;
	/** Get the bucket size in X including filtering extra
	 */
	virtual	TqInt	RealWidth() const = 0;
	/** Get the bucket size in Y including filtering extra
	 */
	virtual	TqInt	RealHeight() const = 0;
	/** Get the position of this bucket in X
	 */
	virtual	TqInt	XOrigin() const = 0;
	/** Get the position of this bucket in Y
	 */
	virtual	TqInt	YOrigin() const = 0;

	/** Get an element color from this bucket. If the requested address is not within this bucket, returns black.
	 * \param iXPos Screen position of the requested element.
	 * \param iYPos Screen position of the requested element.
	 */
	virtual	CqColor Color( TqInt iXPos, TqInt iYPos ) const = 0;
	/** Get an element opacity from this bucket. If the requested address is not within this bucket, returns transparent.
	 * \param iXPos Screen position of the requested element.
	 * \param iYPos Screen position of the requested element.
	 */
	virtual	CqColor Opacity( TqInt iXPos, TqInt iYPos ) const = 0;
	/** Get an element coverage from this bucket. If the requested address is not within this bucket, returns 0.
	 * \param iXPos Screen position of the requested element.
	 * \param iYPos Screen position of the requested element.
	 */
	virtual	TqFloat Coverage( TqInt iXPos, TqInt iYPos ) const = 0;
	/** Get an element depth from this bucket. If the requested address is not within this bucket, returns FLT_MAX.
	 * \param iXPos Screen position of the requested element.
	 * \param iYPos Screen position of the requested element.
	 */
	virtual	TqFloat Depth( TqInt iXPos, TqInt iYPos ) const = 0;
	/** Get a pointer to the sample array
	 * \param iXPos Screen position of the requested element.
	 * \param iYPos Screen position of the requested element.
	 */
	virtual const TqFloat* Data( TqInt iXPos, TqInt iYPos ) const = 0;
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
	virtual	TqInt	OpenDisplays() = 0;
	/** Close all displays in the managers list, rendering is finished.
	 */
	virtual	TqInt	CloseDisplays() = 0;
	/** Display a bucket.
	 */
	virtual	TqInt	DisplayBucket( const IqBucket* pBucket ) = 0;
	/** Determine if any of the displays need the named shader variable.
	 */
	virtual bool	fDisplayNeeds( const TqChar* var) = 0;
	/** Determine if any of the displays need the named shader variable.
	 */
	virtual TqInt	Uses( ) = 0;
};

END_NAMESPACE( Aqsis )

#endif	// IDDMANAGER_H_INCLUDED

