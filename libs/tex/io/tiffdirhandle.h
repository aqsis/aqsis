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
 * \brief A C++ wrapper around tiff file directories
 *
 * \author Chris Foster
 */

#ifndef TIFFDIRHANDLE_H_INCLUDED
#define TIFFDIRHANDLE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <string>

#include <boost/shared_array.hpp>
#include <boost/utility.hpp>
#include <tiffio.h>

#include <aqsis/tex/buffers/channellist.h>
#include <aqsis/util/exception.h>
#include <aqsis/util/file.h>
#include <aqsis/util/logging.h>
#include <aqsis/tex/io/texfileheader.h>

namespace Aqsis
{

class CqTiffFileHandle;

//------------------------------------------------------------------------------
namespace Attr {
	/**
	 * Extra image attribute recording whether a TIFF file has pixels in a
	 * format which are supported for reading natively by aqsistex.  If the
	 * attribute is true, use the generic libtiff RGBA image reading facility
	 * rather than handling the pixel format natively.
	 */
	AQSIS_IMAGE_ATTR_TAG(TiffUseGenericRGBA, bool);
}


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
#ifdef AQSIS_SYSTEM_WIN32
class AQSIS_TEX_SHARE boost::noncopyable_::noncopyable;
#endif
class AQSIS_TEX_SHARE CqTiffDirHandle : boost::noncopyable
{
	public:
		/** \brief Construct a tiff directory handle from a tiff file handle
		 *
		 * \param fileHandle - handle to the underlying tiff file
		 * \param dirIdx - directory index which this handle will point to.
		 *                 Only relevant if the underlying file is open for input.
		 */
		CqTiffDirHandle(const boost::shared_ptr<CqTiffFileHandle>& fileHandle,
				const tdir_t = 0);

		/// Obtain the underlying tiff file pointer
		TIFF* tiffPtr() const;
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
		 *   - channelList: TqChannelList; channel information
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
		 * tag, throw an error, 
		 *
		 * \param tag - tiff tag to write
		 * \param value - value for the tag.
		 */
		template<typename T>
		void setTiffTagValue(const ttag_t tag, const T value);
		/** \brief Write an array tag to the underlying tiff directory.
		 *
		 * Attempt to write an the given array of values to the underlying tiff
		 * directory.  Note that this has the same caveats as
		 * setTiffTagValue().
		 *
		 * \see setTiffTagValue
		 *
		 * \param tag - tiff tag to write
		 * \param values - vector of values for the tag.
		 */
		template<typename T>
		void setTiffTagValue(const ttag_t tag, const std::vector<T>& values);
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
		 * \param channelList - channel object to place the guessed channels in.
		 *                   The object is cleared before being modified.
		 */
		void guessChannels(CqChannelList& channelList) const;
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
#ifdef AQSIS_SYSTEM_WIN32
class AQSIS_TEX_SHARE boost::noncopyable_::noncopyable;
#endif
class AQSIS_TEX_SHARE CqTiffFileHandle : boost::noncopyable
{
	public:
		/** \brief Construct a tiff file handle
		 *
		 * \throw XqInternal if libtiff cannot open the file.
		 *
		 * \param fileName - name of the tiff to open
		 * \param openMode - libtiff file open mode ("r" or "w" for read or write)
		 */
		CqTiffFileHandle(const boostfs::path& fileName, const char* openMode);
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
		inline const boostfs::path& fileName() const;

		/** \brief Write the current directoy to file, and increment the
		 * directory index.
		 *
		 * The tiff file must be open for output - calling this function for an
		 * input file is an error.
		 */
		void writeDirectory();

		/** \brief Determine the number of directories present for this TIFF file.
		 */
		tdir_t numDirectories();

	private:
		friend class CqTiffDirHandle;
		/** \brief Set the current directory for this tiff file.
		 */
		void setDirectory(tdir_t dirIdx);

		const boostfs::path m_fileName;     ///< name of the tiff file
		boost::shared_ptr<TIFF> m_tiffPtr;  ///< underlying TIFF structure
		bool m_isInputFile;                 ///< true if the file is open for input
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


//------------------------------------------------------------------------------
// Utility functions


/** \brief Strided memory copy.
 *
 * Copies numElems data elements from src to dest.  Each data element (eg,
 * contiguous group of pixels) has size given by elemSize bytes.  The stride
 * between one data element and the next is given in bytes.
 *
 * \param dest - destination buffer
 * \param destStride - stride between data elements in the destination buffer in bytes
 * \param src - source buffer
 * \param srcStride - stride between data elements in the source buffe in bytes
 * \param elemSize - size of the data element in bytes
 */
void stridedCopy(TqUint8* dest, TqInt destStride, const TqUint8* src, TqInt srcStride,
		TqInt numElems, TqInt elemSize);


//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

// CqTiffDirHandle implementation
inline TIFF* CqTiffDirHandle::tiffPtr() const
{
	return m_fileHandle->m_tiffPtr.get();
}

template<typename T>
T CqTiffDirHandle::tiffTagValue(const ttag_t tag) const
{
	T temp = 0;
	if(TIFFGetField(tiffPtr(), tag, &temp))
	{
		return temp;
	}
	else
	{
		AQSIS_THROW_XQERROR(XqInternal, EqE_BadFile, "Could not get tiff tag "
				<< tag << " from file \"" << m_fileHandle->fileName() << "\"");
	}
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
void CqTiffDirHandle::setTiffTagValue(const ttag_t tag, const T value)
{
	if(!TIFFSetField(tiffPtr(), tag, value))
	{
		AQSIS_THROW_XQERROR(XqInternal, EqE_System, "Could not set tiff tag "
				<< tag << " to value " << value << " for file \""
				<< m_fileHandle->fileName() << "\"");
	}
}

template<typename T>
void CqTiffDirHandle::setTiffTagValue(const ttag_t tag,
		const std::vector<T>& values)
{
	if(!TIFFSetField(tiffPtr(), tag, static_cast<uint32>(values.size()), &values[0]))
	{
		AQSIS_THROW_XQERROR(XqInternal, EqE_System, "Could not set array tiff tag "
				<< tag << " starting with value " << values[0] << " for file \""
				<< m_fileHandle->fileName() << "\"");
	}
}

//------------------------------------------------------------------------------
// CqTIffFileHandle implementation
inline const boostfs::path& CqTiffFileHandle::fileName() const
{
	return m_fileName;
}

//------------------------------------------------------------------------------
// libtiff wrapper functions
template<typename T>
boost::shared_array<T> tiffMalloc(const tsize_t size)
{
	boost::shared_array<T> buf(reinterpret_cast<T*>(_TIFFmalloc(size)), _TIFFfree);
	if(!buf)
		AQSIS_THROW_XQERROR(XqInternal, EqE_NoMem, "Could not allocate memory with _TIFFmalloc");
	return buf;
}


//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // TIFFDIRHANDLE_H_INCLUDED
