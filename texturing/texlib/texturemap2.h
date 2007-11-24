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

#include "itexturemap.h"
#include "sstring.h"

namespace Aqsis
{

/* \brief A multi-resolution filtering texture sampler
 *
 * \todo Rename this to CqTextureMap after the old CqTextureMap is removed.
 * \todo Refactor when a new IqTextureMap interface is decided on.
 */
class CqTextureMap2 : public IqTextureMap
{
	public:
		CqTextureMap2(const std::string& fileName);

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
		/// \todo replace this with std::string when the interface is revamped.
		CqString m_fileName;
};


//==============================================================================
// Implementation details
//==============================================================================

inline TqUint CqTextureMap2::XRes() const
{
	// \todo implementation
	return 512;
}

inline TqUint CqTextureMap2::YRes() const
{
	// \todo implementation
	return 512;
}

inline TqInt CqTextureMap2::SamplesPerPixel() const
{
	// \todo implementation
	return 1;
}

inline EqTexFormat CqTextureMap2::Format() const
{
	// \todo implementation
	return TexFormat_MIPMAP;
}

inline EqMapType CqTextureMap2::Type() const
{
	return MapType_Texture;
}

inline const CqString& CqTextureMap2::getName() const
{
	// \todo implementation
	return m_fileName;
}

inline bool CqTextureMap2::IsValid() const
{
	// \todo implementation
	return true;
}

inline void CqTextureMap2::SampleMap(CqVector3D& R, CqVector3D& swidth, CqVector3D& twidth,
		std::valarray<TqFloat>& val, TqInt index,
		TqFloat* average_depth, TqFloat* shadow_depth)
{
	assert(false);
}

inline void CqTextureMap2::SampleMap(CqVector3D& R1, CqVector3D& R2, CqVector3D& R3,
		CqVector3D& R4, std::valarray<TqFloat>& val, TqInt index,
		TqFloat* average_depth, TqFloat* shadow_depth)
{
	assert(false);
}

inline TqInt CqTextureMap2::NumPages() const
{
	return 1;
}

//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // TEXTUREMAP2_H_INCLUDED
