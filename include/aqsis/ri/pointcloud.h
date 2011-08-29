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
// foundation, inc., 59 temple place, suite 330, boston, ma  02111-1307  usa


/** \file
	\brief declares an api for points information compatible with prman, 3delight, pixie (at least at the API level under the cover will be different)
	\author Michel Joron  (joron@sympatico.ca)
*/

//? is .h included already?
#ifndef PTC_H_INCLUDE
#define PTC_H_INCLUDE

#include <aqsis/config.h>

typedef void * PtcPointCloud;

extern "C" {
// Create a new Point cloud file
AQSIS_TEX_SHARE PtcPointCloud PtcCreatePointCloudFile ( const char *filename, int nvars,const char **vartypes, const char **varnames, float *world2eye, float
	        *world2ndc, float *format);

// Write a point to the Point cloud file
AQSIS_TEX_SHARE int PtcWriteDataPoint ( PtcPointCloud pointcloud, float *point, float*normal, float radius, float *data);

// Finish and close the Point cloud file
AQSIS_TEX_SHARE void PtcFinishPointCloudFile ( PtcPointCloud pointcloud);

// Open an existant the Point cloud file
AQSIS_TEX_SHARE PtcPointCloud PtcOpenPointCloudFile ( const char *filename, int *nvars,const char **vartypes, const char **varnames );


// Get information from any point from the Point cloud file
AQSIS_TEX_SHARE int PtcGetPointCloudInfo ( PtcPointCloud pointcloud, const char *request,void *result );

// Get one point from the Point clound file
AQSIS_TEX_SHARE int PtcReadDataPoint ( PtcPointCloud pointcloud, float *point, float*normal, float *radius, float *user_data );

// Get one normal, radius, user_data from the location point
AQSIS_TEX_SHARE int PtcFindDataPoint ( PtcPointCloud pointcloud, float *point, float*normal, float *radius, float *user_data );


// Close Point cloud file
AQSIS_TEX_SHARE void PtcClosePointCloudFile ( PtcPointCloud pointcloud );
}

#endif
