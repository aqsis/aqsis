// Volume data shader
// Copyright (C) 2008 Christopher J. Foster
//
// This software is licensed under the GPLv2 - see the file COPYING for details.

#ifndef VOLUMESAMPLER_H_INCLUDED
#define VOLUMESAMPLER_H_INCLUDED

#include "array3d.h"

//------------------------------------------------------------------------------
// Class for sampling volume data
class VolumeSampler
{
	public:
		VolumeSampler(const char* fileName);
		// point sample the data by truncating to the nearest sample.
		inline float sample(float x, float y, float z) const;
		float sampleTrilinear(float x, float y, float z) const;
		void gradient(float x, float y, float z, float& Dx, float& Dy, float& Dz) const;
	private:
		// return true if the point lies in the data box [0,1)^3
		static inline bool inBox(float x, float y, float z);

		Array3D<float> m_data;
};


//------------------------------------------------------------------------------
// Implementation of inline functions
inline float VolumeSampler::sample(float x, float y, float z) const
{
	if(!inBox(x,y,z))
		return 0;
	// point sampling
	return m_data(int(x*m_data.nx()), int(y*m_data.ny()), int(z*m_data.nz()));
}

inline bool VolumeSampler::inBox(float x, float y, float z)
{
	return x >= 0 && x < 1
		&& y >= 0 && y < 1
		&& z >= 0 && z < 1;
}

//------------------------------------------------------------------------------
#endif // VOLUMESAMPLER_H_INCLUDED
