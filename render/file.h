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
		\brief Declares the CqFile class for handling files with RenderMan searchpath option support.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED 1

#include	<iostream>

#include	"ri.h"
#include	"sstring.h"

#include	"specific.h"	// Needed for namespace macros.

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------
/** \class CqFile 
 * Standard handling of all file types utilising the searchpath options.
 */

class _qShareC CqFile
{
	public:
								/** Default constructor
								 */
		_qShareM				CqFile() : m_pStream(0), m_bInternal(TqFalse)	{}
								/** Constructor taking an open stream pointer and a name.
								 * \param Stream a pointer to an already opened input stream to attach this object to.
								 * \param strRealName the name of the file associated with this stream.
								 */
		_qShareM				CqFile(std::istream* Stream, const char* strRealName) :
														m_pStream(Stream), m_strRealName(strRealName), m_bInternal(TqFalse)	{}
		_qShareM				CqFile(const char* strFilename, const char* strSearchPathOption="");
								/** Dectructor. Takes care of closing the stream if the constructor opened it.
								 */
		_qShareM	virtual		~CqFile()				{if(m_pStream!=NULL && m_bInternal)	delete(m_pStream);}

		_qShareM	void		Open(const char* strFilename, const char* strSearchPathOption="");
								/** Close any opened stream associated with this object.
								 */
		_qShareM	void		Close()					{if(m_pStream!=NULL)	delete(m_pStream); m_pStream=NULL;}
								/** Find out if the stream associated with this object is valid.
								 * \return boolean indicating validity.
								 */
		_qShareM	TqBool		IsValid() const			{return(m_pStream!=NULL);}
								/** Get the name asociated with this file object.
								 * \return a read only reference to the string object.
								 */
		_qShareM	const CqString&	strRealName() const		{return(m_strRealName);}

								/** Cast to a stream reference.
								 */
		_qShareM				operator std::istream&()		{return(*m_pStream);}
								/** Cast to a stream pointer.
								 */
		_qShareM				operator std::istream*()		{return(m_pStream);}
	
								/** Get the current position within the stream if appropriate.
								 * \return long integer indicating the offest from the start.
								 */
		_qShareM	TqLong		Position()				{return(m_pStream->tellg());}
								/** Get the length of the stream if a file.
								 * \return the lenght as a long integer.
								 */
		_qShareM	TqLong		Length()				{
															/// \todo Should check if it is a file here.
															long pos=Position();
															m_pStream->seekg(0,std::ios::end);
															long len=Position();
															m_pStream->seekg(pos,std::ios::beg);
															return(len);
														}

	private:
		std::istream*	m_pStream;		///< a poimter to the stream associated with this file object.
		CqString		m_strRealName;	///< the name of this file object, usually the filename.
		TqBool			m_bInternal;	///< a flag indicating whether the stream originated internally, or was externally created and passed in.
};

//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !FILE_H_INCLUDED
