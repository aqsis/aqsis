// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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
 *
 * \brief Define classes for manipulating image channels.
 *
 * Basic local manipulations on image channel data form the underpinnings of a
 * simple compositor, which is what these classes are designed for.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 *
 */

#ifndef IMAGEBUFFER_H_INCLUDED
#define IMAGEBUFFER_H_INCLUDED

#include "aqsis.h"

#include "ndspy.h"


namespace Aqsis {
//------------------------------------------------------------------------------

/** \brief A more C++ - like version of the PkDspy* macros from ndspy.h
 *
 * Using this enum allows us to explicitly state that functions take a
 * "ChannelFormat" as an input, rather than just some unnamed integer.
 */
enum ChannelFormat
{
	Format_Float32 = PkDspyFloat32,
	Format_Unsigned32 = PkDspyUnsigned32,
	Format_Signed32 = PkDspySigned32,
	Format_Unsigned16 = PkDspyUnsigned16,
	Format_Signed16 = PkDspySigned16,
	Format_Unsigned8 = PkDspyUnsigned8,
	Format_Signed8 = PkDspySigned8,
};

/** \brief Floating point type used to do conversions.
 *
 * We include this as it's slightly possible that 32bit floating point
 * formats won't be entirely ideal - they only inexactly represent
 * 32bit integers for example.
 */
typedef TqFloat TqFloatConv;


//------------------------------------------------------------------------------
class IqImageChannelSource
{
	public:
		/** \brief Request whether the source can produce a given channel width and
		 * height (in pixels).
		 *
		 * If the generator returns true, it is assumed that all subsequent
		 * requests to getRow have the desired width.  And that the "row"
		 * parameter to getRow() will be between 0 and height-1
		 *
		 * \param width - requested width of the channel in pixels
		 * \param height - height of the channel in pixels
		 * \return true if the source can output the requested width and
		 *         heights, false otherwise.
		 */
		bool requestSize(TqUint width, TqUint height) = 0;
		/** \brief Copy a row of data into the buffer provided
		 *
		 * Performing any necessary type conversions based on the type of this channel.
		 *
		 * \param row - image row to take data from
		 * \return buffer filled with 
		 */
		virtual const TqFloatConv* getRow(TqUint row) const = 0;
		/** \brief Get a "raw" row.  This allows some sort of access to the
		 * underlying data held in the channel.
		 *
		 * Implement this one later if useful for efficiency...  It's intended
		 * to replace the messy implementation of copyFromSameType() in
		 * CqImageChannel.
		 *
		 * \param row - channel row number to take the data from
		 * \param buf - output pointer to the beginning of the buffer
		 * \param stride - stride for buf in bytes
		 * \param format - desired format for the row
		 */
		//virtual void getRawRow(TqUint row, const TqUchar* &buf, TqInt& stride, ChannelFormat format) = 0 const;
};


//------------------------------------------------------------------------------
/** \brief Interface for channels which can accept data
 *
 * Image channel "sinks" are able to gather 
 */
class IqImageChannelSink
{
	public:
		/** \brief Copy data from the source channel, replacing the data in the
		 * current channel.
		 *
		 * \param source - channel which the data should come from.
		 */
		void copyFrom(const IqImageChannelSource& source) = 0;
		/** \brief Composite data from the given source over the top of this
		 * channel.
		 *
		 * I think renderman uses premultiplied alpha (though need to chech the
		 * Display section in the RISpec carefully).
		 *
		 * \param source - source intensity data
		 * \param alpha - alpha channel for the source.
		 */
		//void compositeOver(const IqImageChannelSource& source, const IqImageChannelSource& alpha);
};


//------------------------------------------------------------------------------
/// Simply inherit everything from the source and sink interfaces.
class IqImageChannel : public IqImageChannelSource, IqImageChannelSink
{
};


//------------------------------------------------------------------------------
} // namespace Aqsis
