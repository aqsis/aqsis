// Volume data shader
// Copyright (C) 2008 Christopher J. Foster
//
// This software is licensed under the GPLv2 - see the file COPYING for details.

#include <stdexcept>
#include <iostream>

#include <aqsis/ri/shadeop.h>

#include "volumesampler.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Renderman DSO interface stuff

SHADEOP_TABLE (sample_vol) = {
	{ "float vol_dens (point)", "vol_dens_init", "vol_dens_shutdown"},
	{ "", "", "" }
};

SHADEOP_INIT(vol_dens_init)
{
	try
	{
		// Here I've hardcoded the location of the volume data.  The standard
		// DSO shadeop interface doesn't allow for a nice way to store
		// shader-specified state between calls, and passing in a string each
		// time to specify the file is probably rather inefficient.
		//
		// There's some hacks which could work around the problem, but for the
		// moment this is a simple solution...
		return reinterpret_cast<void*>(new VolumeSampler("data/iso_3d_restart.dat"));
	}
	catch(std::runtime_error& e)
	{
		std::cerr << e.what() << "\n";
		return 0;
	}
}

SHADEOP(vol_dens)
{
	float* result = reinterpret_cast<float*>(argv[0]);
	float* P = reinterpret_cast<float*>(argv[1]);
	if(initdata)
	{
		*result = reinterpret_cast<VolumeSampler*>(initdata)->sampleTrilinear(P[0], P[1], P[2]);
		return 0;
	}
	else
		return 1;
}

SHADEOP_SHUTDOWN(vol_dens_shutdown)
{
	delete reinterpret_cast<VolumeSampler*>(initdata);
}

