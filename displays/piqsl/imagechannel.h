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

#ifndef IMAGECHANNEL_H_INCLUDED
#define IMAGECHANNEL_H_INCLUDED

#include "aqsis.h"

#include <vector>
#include <limits>
#include <string>

#include "aqsismath.h"


namespace Aqsis {
//------------------------------------------------------------------------------

/** \brief A more C++ - like version of the PkDspy* macros from ndspy.h
 *
 * Using this enum allows us to explicitly state that functions take a
 * "EqChannelFormat" as an input, rather than just some unnamed integer.
 */
enum EqChannelFormat
{
	Format_Float32,
	Format_Unsigned32,
	Format_Signed32,
	Format_Unsigned16,
	Format_Signed16,
	Format_Unsigned8,
	Format_Signed8
};

/** \brief Convert a dspy format type into the enum EqChannelFormat equivilant.
 * \return the equivilant EqChannelFormat type for the given PkDspy* constant
 */
EqChannelFormat chanFormatFromPkDspy(TqInt dspyFormat);

/** \brief Convert a EqChannelFormat to the PkDspy equivilant
 */
TqInt pkDspyFromChanFormat(EqChannelFormat format);

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
class IqImageChannelSource
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
		virtual void requireSize(TqUint width, TqUint height) const = 0;
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
		//virtual void getRawRow(TqUint row, const TqUchar* &buf, TqInt& stride, EqChannelFormat format) = 0 const;
		inline virtual ~IqImageChannelSource() = 0;
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
		inline virtual ~IqImageChannelSink() = 0;
};


//------------------------------------------------------------------------------
/// Inherit everything from the source and sink interfaces.
class IqImageChannel : public IqImageChannelSource, IqImageChannelSink
{
	public:
		inline virtual ~IqImageChannel() = 0;
};


//------------------------------------------------------------------------------
/** \brief A constant-valued image channel source
 */
class CqImageChannelConstant : public IqImageChannelSource
{
	public:
		/** \brief Construct a constant image channel
		 *
		 * \param value - the constant value which will be generated by the channel.
		 */
		CqImageChannelConstant(TqFloat value = 0);
		virtual void requireSize(TqUint width, TqUint height) const;
		virtual const TqFloatConv* getRow(TqUint row) const;
	private:
		TqFloatConv m_value; ///< The value for the channel
		mutable std::vector<TqFloatConv> m_rowBuf; ///< buffer to hold generated row of data
};


//------------------------------------------------------------------------------
/** \brief A image channel source which produces a checker pattern
 *
 * The checker pattern is suitable for use as a base image to show alpha in
 * other images.
 */
class CqImageChannelCheckered : public IqImageChannelSource
{
	public:
		/** \brief Construct a checker channel
		 *
		 * \param tileSize - the size of the checkered tiles
		 */
		CqImageChannelCheckered(TqUint tileSize = 16);
		virtual void requireSize(TqUint width, TqUint height) const;
		virtual const TqFloatConv* getRow(TqUint row) const;
	private:
		TqUint m_tileSize; ///< The checker tile size
		mutable std::vector<TqFloatConv> m_checkerRow0; ///< pattern for even tiles
		mutable std::vector<TqFloatConv> m_checkerRow1; ///< pattern for odd tiles
};


//------------------------------------------------------------------------------
/** \brief Hold channel information; similar in purpose to PtDspyDevFormat from ndspy.h.
 */
struct SqChannelInfo
{
	std::string name;  ///< name of the channel (eg, "r", "g"...)
	EqChannelFormat type;	///< channel format type
	/// Trivial constructor
	inline SqChannelInfo(const std::string& name, EqChannelFormat type);
	/** \brief Get the number of bytes per pixel for this channel data.
	 *
	 * \return channel size in bytes
	 */
	TqUint bytesPerPixel() const;
};


//------------------------------------------------------------------------------
/** \brief A thin wrapper around channel data held in a CqImageBuffer
 *
 * CqImageChannel wraps around a single channel of a subregion of a heterogenous
 * array:
 *
 * A subregion looks like:
 *
 *  [ o o o o ]  
 *  [ o X X o ]  ^
 *  [ o X X o ]  | height
 *  [ o X X o ]  v
 *      <->
 *     width
 *
 *    <----->
 *    full_width
 *
 * where each pixel (o or X) possibly contains hetrogenous data types,
 * eg, X = [0xRR 0xGGGG] for a 8 bit red channel, and a 16 bit green channel.
 *
 * To do this we need to store
 * - A pointer to the start of the data
 * - width and height of the region
 * - the number of bytes to skip between entries ("stride")
 * - the number of pixels which are skipped at the end of each row (rowSkip =
 *   full_width - width)
 *
 * Operations which need access to the type of the channel data (eg,
 * conversions) are performed in the subclass CqImageChannelTyped
 */
class CqImageChannel : public IqImageChannel
{
	public:
		/** \brief Construct an image channel.
		 *
		 * \param chanInfo - channel information (name & type)
		 * \param data - raw pointer to the start of the channel data
		 * \param width - width of the channel in pixels
		 * \param height - height of the channel in pixels
		 * \param stride - stride between pixels in bytes
		 * \param rowSkip - number of pixels which are skipped at the end of
		 *                  each row (allows for support of rectangular subregions).
		 */
		CqImageChannel(const SqChannelInfo& chanInfo, TqUchar* data,
				TqUint width, TqUint height, TqUint stride, TqUint rowSkip = 0);
		virtual inline ~CqImageChannel();
		/** \brief Get descriptive information about the channel
		 */
		inline const SqChannelInfo& channelInfo() const;

		// Inherited
		virtual void requireSize(TqUint width, TqUint height) const;
		virtual void copyFrom(const IqImageChannelSource& source);
		virtual void compositeOver(const IqImageChannelSource& source,
				const IqImageChannelSource& sourceAlpha);

		/// \note Use default copy constructor and assignment ops.
	protected:
		/** \brief Replace a row in the current channel
		 *
		 * Performs the necessary type conversion from TqFloatConv to
		 * the type of this channel.
		 *
		 * \param row - image row to replace, counting from the top (row 0).
		 * \param buf - buffer holding the data to replace the row with.
		 */
		virtual void replaceRow(TqUint row, const TqFloatConv* buf) = 0;
		/** \brief Composite a row onto the current channel
		 *
		 * Performs the necessary type conversion from TqFloatConv to
		 * the type of this channel.  The source is assumed to be
		 * alpha-premultipled.
		 *
		 * \param row - image row to replace, counting from the top (row 0).
		 * \param buf - buffer holding the data to replace the row with.
		 */
		virtual void compositeRow(TqUint row, const TqFloatConv* src,
				const TqFloatConv* srcAlpha) = 0;

		SqChannelInfo m_chanInfo; ///< channel format information
		TqUchar* m_data;	///< raw data
		TqUint m_width;		///< width of raw data in pixels
		TqUint m_height;	///< height of raw data in pixels
		TqUint m_stride;	///< stride between one pixel and the next in bytes
		TqUint m_rowSkip;	///< number of pixels to skip at the end of each row
		mutable std::vector<TqFloatConv> m_copyBuf; ///< temporary row buffer for holding results of a conversion
};


//------------------------------------------------------------------------------
/** \brief Override some methods of CqImageChannel with type-specific
 * conversion capabilities.
 */
template<typename T>
class CqImageChannelTyped : public CqImageChannel
{
	public:
		/** \brief Constructor
		 * \see CqImageChannel::CqImageChannel
		 */
		inline CqImageChannelTyped(const SqChannelInfo& chanInfo, TqUchar* data,
				TqUint width, TqUint height, TqUint stride, TqUint rowSkip = 0);
	private:
		// Inherited
		virtual const TqFloatConv* getRow(TqUint row) const;
		virtual void replaceRow(TqUint row, const TqFloatConv* buf);
		virtual void compositeRow(TqUint row, const TqFloatConv* src,
				const TqFloatConv* srcAlpha);

		/// Convert the type held by this channel into a float.
		static inline TqFloatConv convertToFloat(T t);
		/// Convert from a float to the type held by this channel.
		static inline T convertFromFloat(TqFloatConv f);
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Virtual destructors for interfaces
inline IqImageChannelSource::~IqImageChannelSource()
{ }

inline IqImageChannelSink::~IqImageChannelSink()
{ }

inline IqImageChannel::~IqImageChannel()
{ }

//------------------------------------------------------------------------------
// SqChannelInfo
inline SqChannelInfo::SqChannelInfo(const std::string& name, EqChannelFormat type)
	: name(name),
	type(type)
{ }

//------------------------------------------------------------------------------
// CqImageChannel implementation
inline CqImageChannel::~CqImageChannel()
{ }

inline const SqChannelInfo& CqImageChannel::channelInfo() const
{
	return m_chanInfo;
}


//------------------------------------------------------------------------------
// CqImageChannelTyped implementation
template<typename T>
inline CqImageChannelTyped<T>::CqImageChannelTyped(const SqChannelInfo& chanInfo,
		TqUchar* data, TqUint width, TqUint height, TqUint stride, TqUint rowSkip)
	: CqImageChannel(chanInfo, data, width, height, stride, rowSkip)
{ }

template<typename T>
const TqFloatConv* CqImageChannelTyped<T>::getRow(TqUint row) const
{
	TqUchar* srcBuf = m_data + row*m_stride*(m_width + m_rowSkip);
	std::vector<TqFloatConv>::iterator destBuf = m_copyBuf.begin();
	for(TqUint i = 0; i < m_width; ++i)
	{
		*destBuf = convertToFloat(*reinterpret_cast<T*>(srcBuf));
		srcBuf += m_stride;
		destBuf++;
	}
	return &m_copyBuf[0];
}

template<typename T>
void CqImageChannelTyped<T>::replaceRow(TqUint row, const TqFloatConv* buf)
{
	TqUchar* destBuf = m_data + row*m_stride*(m_width + m_rowSkip);
	for(TqUint i = 0; i < m_width; ++i)
	{
		*reinterpret_cast<T*>(destBuf) = convertFromFloat(*buf);
		destBuf += m_stride;
		buf++;
	}
}

template<typename T>
void CqImageChannelTyped<T>::compositeRow(TqUint row, const TqFloatConv* src,
		const TqFloatConv* srcAlpha)
{
	TqUchar* destBuf = m_data + row*m_stride*(m_width + m_rowSkip);
	for(TqUint i = 0; i < m_width; ++i)
	{
		TqFloatConv oldCol = convertToFloat(*reinterpret_cast<T*>(destBuf));
		*reinterpret_cast<T*>(destBuf) = convertFromFloat(*src + (1 - *srcAlpha)*oldCol);
		destBuf += m_stride;
		src++;
		srcAlpha++;
	}
}

template<typename T>
inline TqFloatConv CqImageChannelTyped<T>::convertToFloat(T src)
{
	if(std::numeric_limits<T>::is_integer)
	{   
		// source = integer
		return ( static_cast<TqFloatConv>(src) -
				static_cast<TqFloatConv>(std::numeric_limits<T>::min()) )
			/ ( static_cast<TqFloatConv>(std::numeric_limits<T>::max())
				- static_cast<TqFloatConv>(std::numeric_limits<T>::min()));
	}
	else
	{
		// source = floating point
		return static_cast<TqFloatConv>(src);
	}
}

template<typename T>
inline T CqImageChannelTyped<T>::convertFromFloat(TqFloatConv src)
{
	if(std::numeric_limits<T>::is_integer)
	{
		// result = integer
		return static_cast<T>(lround(clamp<TqFloatConv>(src, 0, 1) *
					( static_cast<TqFloatConv>(std::numeric_limits<T>::max())
					- static_cast<TqFloatConv>(std::numeric_limits<T>::min()) )
					+ std::numeric_limits<T>::min()
				));
	}
	else
	{
		// result = floating point
		return static_cast<T>(src);
	}
}

#if 0
template<typename T>
void CqImageChannelTyped<T>::copyFromSameType(const CqImageChannel& source)
{
	assert(m_chanInfo.type == source.m_chanInfo.type);
	TqUchar* srcBuf = source.m_data;
	TqUchar* destBuf = m_data;
	for(TqUint row = 0; row < m_height; ++row)
	{
		for(TqUint col = 0; col < m_width; ++col)
		{
			*reinterpret_cast<T*>(destBuf) = *reinterpret_cast<T*>(srcBuf);
			srcBuf += source.m_stride;
			destBuf += m_stride;
		}
		srcBuf += source.m_rowSkip*source.m_stride;
		destBuf += m_rowSkip*m_stride;
	}
}
#endif

//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // IMAGECHANNEL_H_INCLUDED
