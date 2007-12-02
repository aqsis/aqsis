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
 * \brief Declare a filtered texture mapping class
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef TEXTUREMAP2_H_INCLUDED
#define TEXTUREMAP2_H_INCLUDED

#include "aqsis.h"

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "itexturemap.h"
#include "sstring.h"
#include "texfileheader.h"

#include "mipmaplevels.h" /// \todo may be removed when removing wrapper class.

namespace Aqsis
{

class IqTextureSampler;
class CqMipmapLevels;

//------------------------------------------------------------------------------
/* \brief A multi-resolution filtering texture sampler
 *
 * \todo Rename this to CqTextureMap after the old CqTextureMap is removed.
 */
class CqTextureMap2
{
	public:
		CqTextureMap2(const boost::shared_ptr<CqMipmapLevels>& mipmapLevels);
		/** \brief Get the texture attributes of the underlying file.
		 *
		 * This function allows access to the texture file attributes such as
		 * transformation matrices, image resolution etc.
		 *
		 * \return Underlying file attributes, or 0 if there isn't an
		 * underlying file.
		 */
		virtual inline const CqTexFileHeader* attributes() const;
		/// \todo Decide if these two methods are actually needed.
		virtual TqInt numSamples() const;
		virtual const std::string& name() const;

		/** \brief Sample the texture map.
		 *
		 * \param sampleQuad - quadrilateral region to sample over
		 * \param outSamples - the output samples will be placed here.
		 */
		virtual inline void sampleMap(const SqSampleQuad& sampleQuad,
				TqFloat* outSamples) const;

		/** \brief Get the current sample options
		 */
		virtual inline CqTextureSampleOptions& sampleOptions();
		/** \brief Get the current sample options (const version)
		 */
		virtual inline const CqTextureSampleOptions& sampleOptions() const;

		virtual ~CqTextureMap2() {}
	private:
		boost::shared_ptr<CqMipmapLevels> m_mipLevels; ///< Collection of mipmap levels
		CqTextureSampleOptions m_sampleOptions;  ///< sampler options
};

/** Temporary wrapper class for CqTextureMap2 to squash it into the shape of
 * the old IqTextureMap interface.
 *
 * \todo Remove when the new IqTextureMap interface is in place.
 */
class CqTextureMap2Wrapper : public IqTextureMap
{
	public:
		CqTextureMap2Wrapper(const std::string& texName);

		// The following are overridden from IqTextureMap.  It's rather a fat
		// interface and badly needs to be slimmed down.
		virtual TqUint XRes() const;
		virtual TqUint YRes() const;
		virtual TqInt SamplesPerPixel() const;
		virtual EqTexFormat Format() const;
		virtual EqMapType Type() const;
		virtual const CqString& getName() const;
		virtual void Open();
		virtual void Close();
		virtual bool IsValid() const;

		virtual void PrepareSampleOptions(std::map<std::string, IqShaderData*>& paramMap );

		virtual void SampleMap(TqFloat s1, TqFloat t1, TqFloat swidth, TqFloat twidth, std::valarray<TqFloat>& val);
		virtual void SampleMap(TqFloat s1, TqFloat t1, TqFloat s2, TqFloat t2,
				TqFloat s3, TqFloat t3, TqFloat s4, TqFloat t4,
				std::valarray<TqFloat>& val );
		virtual void SampleMap(CqVector3D& R, CqVector3D& swidth, CqVector3D& twidth,
				std::valarray<TqFloat>& val, TqInt index,
				TqFloat* average_depth = NULL, TqFloat* shadow_depth = NULL );
		virtual void SampleMap(CqVector3D& R1, CqVector3D& R2, CqVector3D& R3,
				CqVector3D& R4, std::valarray<TqFloat>& val, TqInt index,
				TqFloat* average_depth = NULL, TqFloat* shadow_depth = NULL );
		virtual CqMatrix& GetMatrix(TqInt which, TqInt index );

		virtual TqInt NumPages() const;
	private:
		mutable CqString m_texName;
		CqMipmapLevels m_levels;
		CqTextureMap2 m_realMap;
		CqTextureSampleOptions m_sampleOptions;
};


//==============================================================================
// Implementation details
//==============================================================================

inline const CqTexFileHeader* CqTextureMap2::attributes() const
{
	/// \todo implementation
	return 0;
}

inline CqTextureSampleOptions& CqTextureMap2::sampleOptions()
{
	return m_sampleOptions;
}

inline const CqTextureSampleOptions& CqTextureMap2::sampleOptions() const
{
	return m_sampleOptions;
}


//------------------------------------------------------------------------------
// CqTextureMap2Wrapper implementation

inline TqUint CqTextureMap2Wrapper::XRes() const
{
	// \todo implementation
	return 512;
}

inline TqUint CqTextureMap2Wrapper::YRes() const
{
	// \todo implementation
	return 512;
}

inline TqInt CqTextureMap2Wrapper::SamplesPerPixel() const
{
	// \todo implementation
	return m_realMap.numSamples();
}

inline EqTexFormat CqTextureMap2Wrapper::Format() const
{
	// \todo implementation
	return TexFormat_MIPMAP;
}

inline EqMapType CqTextureMap2Wrapper::Type() const
{
	return MapType_Texture;
}

inline const CqString& CqTextureMap2Wrapper::getName() const
{
	// \todo implementation
	m_texName = m_realMap.name();
	return m_texName;
}

inline bool CqTextureMap2Wrapper::IsValid() const
{
	// \todo implementation
	return true;
}

inline void CqTextureMap2Wrapper::SampleMap(CqVector3D& R, CqVector3D& swidth, CqVector3D& twidth,
		std::valarray<TqFloat>& val, TqInt index,
		TqFloat* average_depth, TqFloat* shadow_depth)
{
	assert(false);
}

inline void CqTextureMap2Wrapper::SampleMap(CqVector3D& R1, CqVector3D& R2, CqVector3D& R3,
		CqVector3D& R4, std::valarray<TqFloat>& val, TqInt index,
		TqFloat* average_depth, TqFloat* shadow_depth)
{
	assert(false);
}

inline TqInt CqTextureMap2Wrapper::NumPages() const
{
	return 1;
}

//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // TEXTUREMAP2_H_INCLUDED
