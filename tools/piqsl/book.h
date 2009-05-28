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
		\brief Declare the class controlling book's of images.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is book.h included already?
#ifndef BOOK_H_INCLUDED
#define BOOK_H_INCLUDED 1

#include	<vector>
#include	<string>

#include 	<boost/shared_ptr.hpp>

#include	<aqsis/aqsis.h>
#include	<aqsis/ri/ndspy.h>

namespace Aqsis {

//class CqFramebuffer;
class CqImage;

//---------------------------------------------------------------------
/** \class CqBook
 * Class encapsulating the image book functionality.
 */

class CqBook
{
public:
	CqBook( const std::string& name );
	~CqBook()
	{}

	/** Get the books name.
	 */
	const std::string&	name() const;
	/** Set the name of the book.
     * \param name		The new name to the apply to the book.
     */
	void	setName( const std::string& name );

	/** Get a shared pointer to the framebuffer associated with this book.
	 */
//	boost::shared_ptr<CqFramebuffer> framebuffer();

	/** \typedef TqImageList
	 * Typedef for the locally stored image list.
	 */
	typedef std::vector<boost::shared_ptr<CqImage> >			TqImageList;
	/** \typedef TqImageListIterator
	 * Typedef for a basic iterator over the images contained in the book.
	 */
	typedef std::vector<boost::shared_ptr<CqImage> >::iterator	TqImageListIterator;
	/** Get an iterator to the start of the images this book contains.
	 */
	TqImageListIterator imagesBegin();
	/** Get an iterator to just past the last image this book contains.
 	 * Follows the standard STL iterator conventions, so can be used in a != comparison
 	 * for iterating the images in the book.
 	 */
	TqImageListIterator imagesEnd();

	/** Add an image to the book.
 	 * \param image		Shared pointer to the new image.
 	 * \return			The index of the new image in the image list.
 	 */
	TqImageList::size_type addImage(boost::shared_ptr<CqImage>& image);
	/** Get a shared pointer to the image at the specified index, if one exists.
 	 * Will return a null shared pointer if the index is invalid.
 	 * \param index		The index of the requested image in the list, should be less than numImages().
 	 * \return			A shared pointer to the image, or a null shared pointer if the index is not valid.
 	 */
	boost::shared_ptr<CqImage> image(CqBook::TqImageList::size_type index);
	/** Get the number of images in this book.
 	 * \return			The number of images contained in this book.
 	 */
	TqUlong numImages() const;

	/** Remove an image from the book.
 	 * \param item		Iterator referencing the image to be removed, see imagesBegin and imagesEnd.
 	 */
	void removeImage(TqImageListIterator item);

private:
	std::string	m_name;			///< Book name.
	TqImageList m_images;		///< List of images in the book.
//	boost::shared_ptr<CqFramebuffer> m_framebuffer;	///< Shared pointer to the main framebuffer associated with this book.
};

// Implementations of inline functions.
inline const std::string&	CqBook::name() const
{
	return ( m_name );
}

/*inline boost::shared_ptr<CqFramebuffer> CqBook::framebuffer()
{
	return(m_framebuffer);
}*/

inline CqBook::TqImageListIterator CqBook::imagesBegin()
{
	return(m_images.begin());
}

inline CqBook::TqImageListIterator CqBook::imagesEnd()
{
	return(m_images.end());
}

inline TqUlong CqBook::numImages() const
{
	return(static_cast<TqUlong>(m_images.size()));
}


} // namespace Aqsis

#endif	// BOOK_H_INCLUDED
