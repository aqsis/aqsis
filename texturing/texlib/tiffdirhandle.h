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
 * \brief A C++ wrapper around tiff file directories
 *
 * \author Chris Foster
 */

#ifndef TIFFDIRHANDLE_H_INCLUDED
#define TIFFDIRHANDLE_H_INCLUDED

#include "aqsis.h"

#include <string>

#include <boost/utility.hpp>
#include <boost/shared_array.hpp>
#include <boost/format.hpp>
#include <tiffio.h>

#include "exception.h"
#include "channellist.h"
#include "texfileheader.h"
#include "logging.h"

namespace Aqsis
{

class CqTiffFileHandle;

/// Type string for tiff files.

extern const char* tiffFileTypeString;

//------------------------------------------------------------------------------
/** \brief A locked handle to a tiff directory.
 *
 * The handle wraps some libtiff functions in a C++-like manner, but in general
 * is desgined to be simply a thin wrapper with access to the underlying TIFF
 * structure.  This means that users should be prepared to call libtiff
 * functions directly on the TIFF* which is accessible with the tiffPtr()
 * function.
 *
 * Warning! at the moment this isn't thread-safe (but it's easy to make it so).
 *
 * Use this to obtain a handle to a specific directory inside a tiff file.  For
 * threading, the underlying tiff file handle will be locked so that only one
 * directory handle can be obtained at any one time.  This means that an
 * instance of this class blocks access to the underlying tiff file until its
 * destructor is called.
 */
class CqTiffDirHandle : public boost::noncopyable
{
	public:
		/** \brief Construct a tiff directory handle from a tiff file handle
		 *
		 * \param fileHandle - handle to the underlying tiff file
		 * \param dirIdx - directory index
		 */
		CqTiffDirHandle(boost::shared_ptr<CqTiffFileHandle> fileHandle, const tdir_t dirIdx = 0);

		/// Obtain the underlying tiff file pointer
		inline TIFF* tiffPtr() const;
		/// Obtain the index to this directory
		tdir_t dirIndex() const;

		//----------------------------------------------------------------------
		/// \name Functions to read and write header data
		//@{
		/** \brief Fill the given tex file header with relevant data about this
		 * tiff directory.
		 *
		 * "Relevant data" includes the following fields:
		 *   - width, height: TqInt; image dimensions
		 *   - isTiled: bool
		 *     - tileWidth, tileHeight: TqInt; tile size (present only if the
		 *       image is tiled)
		 *   - artist, software, hostname, description, dateTime: std::string;
		 *     descriptive strings
		 *   - channels: TqChannelList; channel information
		 *
		 * \param header - header to fill.
		 */
		void fillHeader(CqTexFileHeader& header) const;
		/** \brief Write relevant header data to the directory.
		 */
		void writeHeader(const CqTexFileHeader& header);
		//@}

		//----------------------------------------------------------------------
		/// \name Access to tags of the underlying tiff file
		//@{
		/** \brief Get the value of a tiff tag
		 *
		 * Note that unfortunately this isn't type-safe: you *must* specify the
		 * correct type, T for the TIFF tag desired, otherwise you'll get
		 * strange results, or a crash!  This is unfortunately due to the
		 * nature of the underlying library and isn't easy to avoid.
		 *
		 * \throw XqInternal if the tag is not defined.
		 *
		 * \param tag - the tiff tag to obtain.
		 *
		 * \return the tag value
		 */
		template<typename T>
		T tiffTagValue(const ttag_t tag) const;
		/** \brief Get the value of a tiff tag with a default value
		 *
		 * Note that unfortunately this isn't type-safe: you *must* specify the
		 * correct type, T for the TIFF tag desired, otherwise you'll get
		 * strange results, or a crash!
		 *
		 * Unlike the one-parameter version of tiffTagValue, this version does
		 * not throw an exception when the tag is not found.  Instead it
		 * quietly returns the supplied default value.
		 *
		 * \param tag - the tiff tag to obtain.
		 * \param defaultVal - value to return if the tag is not present in the file.
		 *
		 * \return the tag value
		 */
		template<typename T>
		T tiffTagValue(const ttag_t tag, const T defaultVal) const;
		/** \brief Write a tag to the underlying tiff dierctory.
		 *
		 * Note that this isn't particularly typesafe - the type T needs to
		 * correspond to the correcty type for the given tag, otherwise weird
		 * stuff might happen.
		 *
		 * If the underlying tiff library reports an error trying to set the
		 * tag, throw an error, but only if throwOnError is true.  Otherwise
		 * the error is reported to the standard aqsis logging facility at
		 * "warning" level.
		 *
		 * \param tag - tiff tag to write
		 * \param value - value for the tag.
		 * \param throwOnError - if true, throw when an error occurrs.
		 */
		template<typename T>
		void setTiffTagValue(const ttag_t tag, const T value,
				bool throwOnError = true);
		//@}

	private:
		//----------------------------------------------------------------------
		/// \name Helper functions for filling CqTexFileHeader
		//@{
		/// Fill the header with required attributes 
		void fillHeaderRequiredAttrs(CqTexFileHeader& header) const;
		/// Search the TIFF for optional header attributes and add them.
		void fillHeaderOptionalAttrs(CqTexFileHeader& header) const;
		/// Fill the header fields dealing with pixel layout
		void fillHeaderPixelLayout(CqTexFileHeader& header) const;
		/** \brief Guess the image channels for the given tiff directory.
		 *
		 * Tiff doesn't have a way to explicitly name the channels contained,
		 * but we can deduce most of what we'd like via various tags.
		 *
		 * \param channels - channel object to place the guessed channels in.
		 *                   The object is cleared before being modified.
		 */
		void guessChannels(CqChannelList& channels) const;
		/** Guess the channel data type
		 *
		 * The channel data type is deduced from reading the tags
		 * TIFFTAG_BITSPERSAMPLE and TIFFTAG_SAMPLEFORMAT.
		 *
		 * \return The data type, or Channel_TypeUnknown if the data type isn't
		 *         understood.
		 */
		EqChannelType guessChannelType() const;
		//@}

		/// \name Helper functions for filling a tiff with header data
		//@{
		/// Write required tiff header attributes
		void writeRequiredAttrs(const CqTexFileHeader& header);
		/// Write channel-related stuff (required attrs.)
		void writeChannelAttrs(const CqTexFileHeader& header);
		/// Write compression-related stuff
		void writeCompressionAttrs(const CqTexFileHeader& header);
		/// Write optional tiff header attributes
		void writeOptionalAttrs(const CqTexFileHeader& header);
		//@}

		//----------------------------------------------------------------------
		boost::shared_ptr<CqTiffFileHandle> m_fileHandle; ///< underlying file handle
		/// \todo: multithreading - add a lock here!
		/// \todo: add a pointer to a TIFFRGBAimage
};


//------------------------------------------------------------------------------
/** \brief A handle for tiff directories
 *
 * The handle takes care of file allocation, deallocation, and keeps
 * track of the current directory.  Do *not* use TIFFOpen, TIFFClose,
 * or TIFFSetDirectory outside this class.
 */
class CqTiffFileHandle : public boost::noncopyable
{
	public:
		/** \brief Construct a tiff file handle
		 *
		 * \throw XqInternal if libtiff cannot open the file.
		 *
		 * \param fileName - name of the tiff to open
		 * \param openMode - libtiff file open mode ("r" or "w" for read or write)
		 */
		CqTiffFileHandle(const std::string& fileName, const char* openMode);
		/** \brief Construct a tiff file handle from a std::istream
		 *
		 * \throw XqInternal if libtiff has a problem with the stream
		 *
		 * \param inputStream - an input stream
		 */
		CqTiffFileHandle(std::istream& inputStream);
		/** \brief Construct a tiff file handle writing to a std::ostream
		 *
		 * \throw XqInternal if libtiff has a problem with the stream
		 *
		 * \param outputStream - the output stream
		 */
		CqTiffFileHandle(std::ostream& outputStream);

		/// Return the file name
		inline const std::string& fileName() const;

	private:
		friend class CqTiffDirHandle;
		/** \brief Set the current directory for this tiff file.
		 */
		void setDirectory(tdir_t dirIdx);

		const std::string m_fileName;       ///< name of the tiff file
		boost::shared_ptr<TIFF> m_tiffPtr;  ///< underlying TIFF structure
		tdir_t m_currDir;                   ///< current directory index
		/// \todo: multithreading - add a mutex!
};

//------------------------------------------------------------------------------
// libtiff wrapper functions

/** \brief Allocate memory with _TIFFmalloc and encapsulate the memory
 * in a boost::shared_array.
 *
 * \throw XqInternal if the allocation fails
 *
 * \param size - number of bytes to allocate.
 * \return 'size' bytes of memory allocated with _TIFFmalloc
 */
template<typename T>
boost::shared_array<T> tiffMalloc(const tsize_t size);



//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

//------------------------------------------------------------------------------
// CqTiffDirHandle
//------------------------------------------------------------------------------

inline TIFF* CqTiffDirHandle::tiffPtr() const
{
	return m_fileHandle->m_tiffPtr.get();
}

template<typename T>
T CqTiffDirHandle::tiffTagValue(const ttag_t tag) const
{
	T temp = 0;
	if(TIFFGetField(tiffPtr(), tag, &temp))
		return temp;
	else
		throw XqInternal((boost::format("Could not get tag with value %d") % tag).str().c_str(), __FILE__, __LINE__);
}

template<typename T>
T CqTiffDirHandle::tiffTagValue(const ttag_t tag, const T defaultVal) const
{
	T temp = 0;
	if(TIFFGetField(tiffPtr(), tag, &temp))
		return temp;
	else
		return defaultVal;
}

template<typename T>
void CqTiffDirHandle::setTiffTagValue(const ttag_t tag, const T value,
		bool throwOnError)
{
	
	if(!TIFFSetField(tiffPtr(), tag, value))
	{
		const std::string errorStr = (boost::format("Could not set tiff tag = %d")
				% tag).str();
		if(throwOnError)
			throw XqInternal(errorStr, __FILE__, __LINE__);
		else
			Aqsis::log() << warning << errorStr;
	}
}

//------------------------------------------------------------------------------
// CqTIffFileHandle
//------------------------------------------------------------------------------
inline const std::string& CqTiffFileHandle::fileName() const
{
	return m_fileName;
}

//------------------------------------------------------------------------------
// libtiff wrapper functions
//------------------------------------------------------------------------------

template<typename T>
boost::shared_array<T> tiffMalloc(const tsize_t size)
{
	boost::shared_array<T> buf(reinterpret_cast<T*>(_TIFFmalloc(size)), _TIFFfree);
	if(!buf)
		throw XqInternal("Could not allocate memory with _TIFFmalloc",
				__FILE__, __LINE__);
	return buf;
}


//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // TIFFDIRHANDLE_H_INCLUDED
