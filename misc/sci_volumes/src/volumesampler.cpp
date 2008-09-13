// Volume data shader
// Copyright (C) 2008 Christopher J. Foster
//
// This software is licensed under the GPLv2 - see the file COPYING for details.

#include "volumesampler.h"

VolumeSampler::VolumeSampler(const char* fileName)
	: m_data(fileName)
{ }

inline float lerp(float t, float a, float b)
{
	return (1-t)*a + t*b;
}

template<typename T>
inline T min(T a, T b)
{
	return (a < b) ? a : b;
}

float VolumeSampler::sampleTrilinear(float x, float y, float z) const
{
	if(!inBox(x,y,z))
		return 0;

	// otherwise do trilinear interpolation.
	float xnf = x*m_data.nx() - 0.5f;
	float ynf = y*m_data.ny() - 0.5f;
	float znf = z*m_data.nz() - 0.5f;

	int xn = int(xnf);
	int yn = int(ynf);
	int zn = int(znf);

	int xnp1 = min(xn+1, m_data.nx()-1);
	int ynp1 = min(yn+1, m_data.ny()-1);
	int znp1 = min(zn+1, m_data.nz()-1);

	float xl = xnf - xn;
	float yl = ynf - yn;
	float zl = znf - zn;

	return lerp(zl,
		lerp(yl, lerp(xl, m_data(xn,yn,zn), m_data(xnp1,yn,zn)),
				lerp(xl, m_data(xn,ynp1,zn), m_data(xnp1,ynp1,zn))),
		lerp(yl, lerp(xl, m_data(xn,yn,znp1), m_data(xnp1,yn,znp1)),
				lerp(xl, m_data(xn,ynp1,znp1), m_data(xnp1,ynp1,znp1)))
		);
}

void VolumeSampler::gradient(float x, float y, float z, float& Dx, float& Dy, float& Dz) const
{
	float dx = 1.0f/m_data.nx();
	float dy = 1.0f/m_data.ny();
	float dz = 1.0f/m_data.nz();
	Dx = (sampleTrilinear(x+0.5*dx,y,z) - sampleTrilinear(x-0.5*dx,y,z))/dx;
	Dy = (sampleTrilinear(x,y+0.5*dy,z) - sampleTrilinear(x,y-0.5*dy,z))/dx;
	Dz = (sampleTrilinear(x,y,z+0.5*dz) - sampleTrilinear(x,y,z-0.5*dz))/dx;
}

