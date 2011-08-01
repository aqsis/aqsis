// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/** \file
 *
 * \brief Interfaces for image channels.
 *
 * Basic local manipulations on image channel data form the underpinnings of a
 * simple and fairly limited compositor, which is what these classes are
 * designed for.
 *
 * \author Chris Foster  [ chris42f _at_ gmail.com ]
 */

#ifndef IIMAGECHANNEL_H_INCLUDED
#define IIMAGECHANNEL_H_INCLUDED

#include <aqsis/aqsis.h>

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Floating point type used to do conversions between different image
 * channel data types.
 *
 * We include this as it's slightly possible that 32bit floating point
 * formats won't be entirely ideal - they only inexactly represent
 * 32bit integers for example.
 */
typedef TqFloat TqFloatConv;


//------------------------------------------------------------------------------
/** \brief A source of image channel data.
 *
 * Sources are able to produce image channel data row by row.  This may come
 * from an in-memory buffer, or simply be generated on the fly.
 */
class AQSIS_TEX_SHARE IqImageChannelSource
{
	public:
		/** \brief Require that the buffer return the given size with
		 * subsequent getRow calls.
		 *
		 * If this call suceeds generator returns true, it is assumed that all
		 * subsequent requests to getRow have the desired width.  And that the
		 * "row" parameter to getRow() will be between 0 and height-1
		 *
		 * \param width - requested width of the channel in pixels
		 * \param height - height of the channel in pixels
		 */
		virtual void requireSize(TqInt width, TqInt height) const = 0;
		/** \brief Copy a row of data into the buffer provided
		 *
		 * Performing any necessary type conversions based on the type of this channel.
		 *
		 * \param row - image row to take data from
		 * \return buffer filled with 
		 */
		virtual const TqFloatConv* getRow(TqInt row) const = 0;
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
		//virtual void getRawRow(TqInt row, const TqUint8* &buf, TqInt& stride, EqChannelType format) = 0 const;
		virtual ~IqImageChannelSource() {}
};


//------------------------------------------------------------------------------
/** \brief Interface for channels which can accept data
 *
 * Image channel "sinks" are objects which image channel data can be copied
 * into.  The data comes from an associated IqImageChannelSource.
 */
class AQSIS_TEX_SHARE IqImageChannelSink
{
	public:
		/** \brief Copy data from the source channel, replacing the data in the
		 * current channel.
		 *
		 * \param source - channel which the data should come from.
		 */
		virtual void copyFrom(const IqImageChannelSource& source) = 0;
		/** \brief Composite data from the given source over the top of this
		 * channel.
		 *
		 * I think renderman uses premultiplied alpha (though need to chech the
		 * Display section in the RISpec carefully).
		 *
		 * \param source - source intensity data
		 * \param alpha - alpha channel for the source.
		 */
		virtual void compositeOver(const IqImageChannelSource& source,
				const IqImageChannelSource& sourceAlpha) = 0;
		virtual ~IqImageChannelSink() {}
};


//------------------------------------------------------------------------------
/** \brief An image channel which is both a source of and a sink for data.
 */
class AQSIS_TEX_SHARE IqImageChannel : public IqImageChannelSource, IqImageChannelSink
{ };


} // namespace Aqsis

#endif // IIMAGECHANNEL_H_INCLUDED
