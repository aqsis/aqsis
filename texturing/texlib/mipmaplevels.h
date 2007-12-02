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
 * \brief Declare a class for holding a collection of mipmap levels.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef MIPMAPLEVELS_H_INCLUDED
#define MIPMAPLEVELS_H_INCLUDED

#include "aqsis.h"

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "texturesampleoptions.h"

namespace Aqsis
{

class IqTextureSampler;

//------------------------------------------------------------------------------
/** \brief A container for a set of texture mipmap levels.
 *
 * This class holds texture mipmap level data, in a way which is independent of
 * the sampling options.  This makes sense because our typical usage is for
 * sampling options to change very often, while the underlying data is fixed.
 * 
 * CqMipmapLevels is the class which will be held in the texture cache.
 *
 * In addition to holding mipmap levels, this class also caches the default
 * sampling options, as determined from attributes of the underlying texture
 * file.  When these aren't present, we attempt to choose sensible defaults.
 */
class CqMipmapLevels
{
	public:
		/** \brief Construct a set of mipmap levels from a file
		 *
		 * If the file named by texName doesn't exist, this class is still
		 * constructed sucessfully.  However, the number of levels is then set
		 * to one, and that level is a dummy texture consisting of a single
		 * pixel.  This allows us to gracefully recover from a missing file.
		 *
		 * \param texName - Name of the texture file to open.
		 *
		 * \todo: Consider whether this should really take a pointer to an open
		 * file inestead of a file name...
		 */
		CqMipmapLevels(const std::string& texName);
		/** \brief Get the texture sampler for a given level.
		 *
		 * \param levelNum - mipmap level to grab
		 */
		const IqTextureSampler& level(TqInt levelNum);
		/** \brief Get the number of levels in this mipmap.
		 *
		 */
		inline TqInt numLevels() const;
		/** \brief Get the default sample options associated with the texture file.
		 *
		 * \return The default sample options - these include options which
		 * were used when the texture was created (things like the texutre wrap
		 * mode).
		 */
		inline const CqTextureSampleOptions& defaultSampleOptions();
		/** \brief Get the texture name
		 *
		 * This is the name of the associated texture file, (if the file
		 * doesn't exist, this is still the name which was passed to the
		 * constructor).
		 *
		 * \return The texture name
		 */
		inline const std::string& name() const;
	private:
		std::string m_texName;  ///< Name of texture
		/// Default texture sampling options for the set of mipmap levels.
		CqTextureSampleOptions m_defaultSampleOptions;
		/** \brief List of samplers for mipmap levels.  The pointers to these
		 * may be NULL since they are created only on demand.
		 */
		mutable std::vector<boost::shared_ptr<IqTextureSampler> > m_mipLevels;
};


//==============================================================================
// Implementation details
//==============================================================================
inline TqInt CqMipmapLevels::numLevels() const
{
	return m_mipLevels.size();
}

inline const CqTextureSampleOptions& CqMipmapLevels::defaultSampleOptions()
{
	return m_defaultSampleOptions;
}

inline const std::string& CqMipmapLevels::name() const
{
	return m_texName;
}

} // namespace Aqsis

#endif // MIPMAPLEVELS_H_INCLUDED
