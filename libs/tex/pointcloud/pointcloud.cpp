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
		\brief declares an api for points information compatible with prman, 3delight and pixie
		\author Michel Joron  (joron@sympatico.ca)
*/
#include <aqsis/aqsis.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>

// The file structure is equivalent to the following.
// Except it is in binary.
//----------------------------------
// 'Aqsis_PTC' 1
// [Number of Channels] 1 [*optional]
// "Ci" "color"
// [Size of Channels] 3
// [World2Eye] 1/0 *Optional
//   Matrix[0], Matrix[1], ... Matrix[15] *Optional
// [World2NDC] 1/0 *Optional
//   Matrix[0], Matrix[1], ... Matrix[15] *Optional
// [Format] 1/0 *Optinal
//    X, Y, AspectRation] *Optional
// [BoundingBox] 6
// -100.955681 99.044319 -90.713783 109.273087 340.360199 505.259033
// [Number of Data] 26812
// -100.955681 9.275940 440.359985 1.000000 0.000000 -0.000000 1.000000
// 0.052700 0.082552 0.032063
// Point[0], Point[1], Point[2], Normal[0], Normal[1], Normal[2], Radius
// User_Data[0], User_Data[1], ...
// ...
// -------
//

#include	<aqsis/ri/pointcloud.h>

#define PTCNAME    "Aqsis_PTC"
#define PTCVERSION 1

typedef struct
{
	float point[3];
	float normal[3];
	float radius;
	float *user_data;
}
PtcPointCloudKey;

typedef struct
{
	char  signature;
	char  filename[1024];
	char  nvars;
	char **vartypes;
	char **varnames;
	float world2eye[16];
	float world2ndc[16];
	float format[3];
// At runtime the rest is filled by the file content itself
	FILE    *fp;
	int   npoints;
	int   seek;
	float bbox[6];
	int   datasize;
	int   maxpoints;
	PtcPointCloudKey *key;
}
PtcPointCloudHandle;


#define MIN(a, b)  (((a) <= (b)) ? (a) : (b))
#define MAX(a, b)  (((a) >= (b)) ? (a) : (b))

int diff(const void *a, const void *b)
{
	PtcPointCloudKey *A = (PtcPointCloudKey *) a;
	PtcPointCloudKey *B = (PtcPointCloudKey *) b;
	return  memcmp((void *) A->point, (void *) B->point, sizeof(float) * 3);
}
//---------------------------------------------------------------------
/**
* This function opens a given file for reading
* \param filename The complete path to the point cloud file
* \param nvars A pointer to an int that will be filled with the total number of channels
*     in the point cloud file.
* \param varnames A pointer to an array of const char * that will hold the name of each
*variable.
* \param vartypes A pointer to an array of const char * that will hold the type of each
*variable. Types are string representing the type of the variable as declared
*in the shading language (color, point, float, normal, . . . etc).
*
*This function could fail if the file is not accessible or is not a valid point cloud file, in which case null is returned.
*
*note: This API call is badly designed since the caller cannot know the size
*of vartypes and varnames in advance. But for compatibly reasons with other
*software we decided to stick with this API. A simple and secure workaround goes
*as follows:
*1. Call PtcOpenPointCloudFile with vartypes and varnames set to null to
*obtain nvars.
*2. Close the returned file handle.
*3. Allocate arrays big enough for vartypes and varnames since now we know
*their size.
*4. Call PtcOpenPointCloudFile again with the allocated arrays.
* \return a file handle than shall be used in other reading calls.
*/
extern "C" PtcPointCloud PtcOpenPointCloudFile ( const char *filename, int *nvars, const char **vartypes, const char **varnames )
{
	PtcPointCloudHandle * ptc = (PtcPointCloudHandle *) (new PtcPointCloudHandle);
	char exist;

	memset((void*)ptc, 0, sizeof(PtcPointCloudHandle));
	strcpy(ptc->filename, filename);
	ptc->fp = fopen(filename, "rb");
	if (ptc->fp)
	{
		char name[80];
		fread(name, 1, strlen(PTCNAME) + 1, ptc->fp);
		fread(&ptc->signature, 1, 1, ptc->fp);

		if (strcmp(name, PTCNAME) == 0 && ptc->signature == PTCVERSION)
		{
         int i;
			fread(&ptc->nvars, 1, 1, ptc->fp);
			if (ptc->nvars > 0)
			{
				ptc->vartypes = (char **) malloc(ptc->nvars * sizeof(char *));
				ptc->varnames = (char **) malloc(ptc->nvars * sizeof(char *));
				for (int i = 0; i < ptc->nvars; i++)
				{
					unsigned char size[2];
					fread(size, 1, 2, ptc->fp);
					ptc->vartypes[i] = (char *) malloc(size[0]);
					ptc->varnames[i] = (char *) malloc(size[1]);
					fread(ptc->vartypes[i], 1, size[0], ptc->fp);
					fread(ptc->varnames[i], 1, size[1], ptc->fp);
				}
			}

			fread(&ptc->datasize, sizeof(int), 1, ptc->fp);

			fread(&exist, 1, 1, ptc->fp);
			if (exist == 1)
			{
				fread(ptc->world2eye, sizeof(float), 16, ptc->fp);
				exist = 0;
			}

			fread(&exist, 1, 1, ptc->fp);
			if (exist == 1)
			{
				fread(ptc->world2ndc, sizeof(float), 16, ptc->fp);
				exist = 0;
			}

			fread(&exist, 1, 1, ptc->fp);
			if (exist == 1)
			{
				fread(ptc->format, sizeof(float), 3, ptc->fp);
				exist = 0;
			}

			fread(ptc->bbox, sizeof(float), 6, ptc->fp);

			fread(&ptc->npoints, sizeof(int), 1, ptc->fp);

			if (ptc->npoints)
			{
				ptc->maxpoints = ptc->npoints;

				ptc->key = (PtcPointCloudKey *) (malloc(ptc->maxpoints * sizeof(PtcPointCloudKey) ));

				for (i = 0; i < ptc->npoints; i++)
				{
					fread(ptc->key[i].point, sizeof(float), 3, ptc->fp);
					fread(ptc->key[i].normal, sizeof(float), 3, ptc->fp);
					fread(&ptc->key[i].radius, sizeof(float), 1, ptc->fp);
					ptc->key[i].user_data = (float *) (malloc(ptc->datasize * sizeof(float) ));
					fread(ptc->key[i].user_data, sizeof(float), ptc->datasize, ptc->fp);
				}
			}

			if (nvars) *nvars = ptc->nvars;
			if (vartypes) 
         {
            for (i=0; i < ptc->nvars; i++)
            {
               vartypes[i] = ptc->vartypes[i];
            }
         }
			if (varnames) 
         {
            for (i=0; i < ptc->nvars; i++)
            {
               varnames[i] = ptc->varnames[i];
            }
         }
			fclose(ptc->fp);
			ptc->fp = NULL;
		}
		else
		{
			ptc->signature = 0;
			delete ptc;
			ptc = NULL;
		}
	}

	return (PtcPointCloud)(ptc);
}


//---------------------------------------------------------------------
/**
*This function returns informations about a point cloud file. Accepted requests are:
*\param pointcloud  A handle to the point cloud file as returned by PtcOpenPointCloudFile
*\param request     The name of the information needed. The following requests are accepted:
*        'npoints' : Number of points in the point cloud file. C++ data type is an integer
*        'bbox'    : The bounding box of the point cloud. Will return an array of six floats:
*               min x, min y, min z, max x, max y and max z.
*        'datasize': The number of floats needed to store each data point. C++ data type is an integer.
*        'world2eye': The world to eye (world to camera) transformation matrix. Will return an
*               array of 16 floats.
*        'world2ndc': The world to NDC transformation matrix. Will return an array of 16 floats.
*        'format'   : The resolution of the render that generated the point cloud file. Three floats will be returned: x resolution, y resolution
*               and aspect ratio.
*\param result     A pointer to an array large enough to hold the returned informations.
*\return           1 if the request is successful, 0 otherwise.
*note: Some point cloud files generated may not contain the  'format', `world2eye' or 'world2ndc' information.
*/
extern "C" int PtcGetPointCloudInfo ( PtcPointCloud pointcloud, const char *request, void *result )
{
	int error = 1;
	PtcPointCloudHandle * ptc = (PtcPointCloudHandle *)(pointcloud);
	if (!ptc || ptc->signature != PTCVERSION)
	{
		error = 0;
	}
	else
	{
		if (strcmp(request, "npoints") == 0)
		{
			*(int *)(result) = ptc->npoints;
		}
		else if (strcmp(request, "bbox") == 0)
		{
			float *bbox = (float*) ( result );
			for (int i = 0; i < 6; i++)
			{
				bbox[i] = ptc->bbox[i];
			}
		}
		else if (strcmp(request, "datasize") == 0)
		{
			*(int *)(result) = ptc->datasize;
		}
		else if (strcmp(request, "world2eye") == 0)
		{
			float *world2eye = (float*)  ( result );
			for (int i = 0; i < 16; i++)
			{
				world2eye[i] = ptc->world2eye[i];
			}
		}
		else if (strcmp(request, "world2ndc") == 0)
		{
			float *world2ndc = (float*)  ( result );
			for (int i = 0; i < 16; i++)
			{
				world2ndc[i] = ptc->world2ndc[i];
			}
		}
		else if (strcmp(request, "format") == 0)
		{
			float *format = (float*)  ( result );
			for (int i = 0; i < 3; i++)
			{
				format[i] = ptc->format[i];
			}
		}
		else
		{
			error = 0;
		}
	}

// look within the file to see what is about the request information.
	return error;
}

//---------------------------------------------------------------------
/**
* Reads next point from the point cloud file. The parameters are:
* \param pointcloud  The handle to the point cloud file as returned by
*                  PtcOpenPointCloudFile.
* \param point       A pointer to a point (three floats) that will be filled with current 'point' position.
* \param normal      A pointer to a point (three floats) that will be filled with current 'point' normal.
* \param radius      A pointer to float that will be filled with point's radius. The area of the micro-polygon that generated this sample is radius * radius * 4.
* \param user_data   A pointer to a user buffer of a size big enough to hold all the variables attached to a point.
* The size of the buffer, in floats, can be obtained
* by calling PtcGetPointCloudInfo with request set to 'datasize'.
*
* \return 1 if the operation is successful, 0 otherwise.
* note: normal, radius and user data can be null if their value is not needed.
*       point is read to know which data to read
*/
extern "C" int PtcReadDataPoint ( PtcPointCloud pointcloud, float *point, float*normal, float *radius, float *user_data )
{
	int error = 1;
	PtcPointCloudHandle * ptc = (PtcPointCloudHandle *)(pointcloud);

	if (!ptc || (ptc->signature != PTCVERSION || ptc->seek >= ptc->npoints))
	{
		error = 0;
	}
	else
	{
		// reject points outside the bounding box
		int seek = ptc->seek;

		ptc->seek++;

		// Try the previous value
		if (seek < ptc->npoints)
		{
			if (point != NULL)
			{
				memcpy(point,  ptc->key[seek].point, 3 * sizeof(float));
			}
			if (normal != NULL)
			{
				memcpy(normal,  ptc->key[seek].normal, 3 * sizeof(float));
			}
			if (user_data != NULL)
			{
				memcpy(user_data, ptc->key[seek].user_data, ptc->datasize * sizeof(float));
			}
			if (radius != NULL)
			{
				*radius =  ptc->key[seek].radius;
			}
		}

	}

	return error;
}

extern "C" int PtcFindDataPoint ( PtcPointCloud pointcloud, float *point, float*normal, float *radius, float *user_data )
{
	int error = 1;
	PtcPointCloudHandle * ptc = (PtcPointCloudHandle *)(pointcloud);

	if (!ptc || (ptc->signature != PTCVERSION || ptc->seek >= ptc->npoints))
	{
		error = 0;
	}
	else
	{
		// reject points outside the bounding box
		if (point[0] < ptc->bbox[0] || point[1] < ptc->bbox[2]  || point[2] < ptc->bbox[4]  ||
		        point[0] > ptc->bbox[1] || point[1] > ptc->bbox[3]  || point[2] > ptc->bbox[5] )
			return 1;

		int seek = ptc->seek;

		// order all the read keys (once)
		if (ptc->seek == 0)
		{
			qsort(ptc->key, ptc->npoints, sizeof(PtcPointCloudKey), diff);
			ptc->seek = 1;
		}


		PtcPointCloudKey key;
		memcpy((void *) key.point, point, 3 * sizeof(float));
		PtcPointCloudKey *found = NULL;
		if ((found = (PtcPointCloudKey *) bsearch(&key, ptc->key, ptc->npoints, sizeof(PtcPointCloudKey), diff)))
		{
			seek = found - ptc->key;
		}
		else
		{
			// Not found !!!
			return 0;
		}

		// Try the previous value
		if (seek != -1)
		{
			if (normal != NULL)
			{
				memcpy(normal,  ptc->key[seek].normal, 3 * sizeof(float));
			}
			if (user_data != NULL)
			{
				memcpy(user_data, ptc->key[seek].user_data, ptc->datasize * sizeof(float));
			}
			if (radius != NULL)
			{
				*radius =  ptc->key[seek].radius;
			}
		}

	}

	return error;
}
//---------------------------------------------------------------------
/**
* Closes a file opened with PtcOpenPointCloudFile.
*/
extern "C" void PtcClosePointCloudFile ( PtcPointCloud pointcloud )
{
	int error = 0;
	PtcPointCloudHandle * ptc = (PtcPointCloudHandle *)(pointcloud);
	if (!ptc || ptc->signature != PTCVERSION)
	{
		error = 1;
	}
	else
	{
		if (ptc->fp != NULL)
		{
			fclose(ptc->fp);
			ptc->fp = NULL;
		}
		ptc->seek = 0;
	}
}

//---------------------------------------------------------------------
/**
* Creates the specified point cloud file. If the point cloud file already exists, it will be
*  overwritten.
* \param filename Complete path to point cloud file.
* \param nvars Number of variables to save in the point cloud.
* \param vartype A type for each variable. Types are the same as in the shading language:
*           point, normal, color, float, matrix ... Etc.
* \param varname A name for each variable.
* \param world2eye A world to camera transformation matrix. (optional)
* \param world2ndc A world to NDC transformation matrix. optional)
* \param format The X resolution, Y resolution and aspect ratio of the image. (optional)
*/
extern "C" PtcPointCloud PtcCreatePointCloudFile ( const char *filename, int nvars, const char **vartypes, const char **varnames, float *world2eye, float *world2ndc, float *format)
{
	PtcPointCloudHandle *  ptc = new PtcPointCloudHandle;
	unsigned char exist;
	memset((void*)ptc, 0, sizeof(PtcPointCloudHandle));
	ptc->fp = fopen(filename, "wb");

	ptc->signature = PTCVERSION;
	strcpy(ptc->filename, filename);
	ptc->bbox[0] = FLT_MAX;
	ptc->bbox[2] = FLT_MAX;
	ptc->bbox[4] = FLT_MAX;
	ptc->bbox[1] = -FLT_MAX;
	ptc->bbox[3] = -FLT_MAX;
	ptc->bbox[5] = -FLT_MAX;
	int datasize = 0;
	fwrite(PTCNAME, 1, strlen(PTCNAME) + 1, ptc->fp);
	fwrite(&ptc->signature, 1, 1, ptc->fp);
	exist = nvars; // allow a maximum of 255 variables per ptc
	fwrite(&exist, 1, 1, ptc->fp);
	for (int i = 0; i < nvars; i++)
	{
		unsigned char size[2];

		size[0] = strlen(vartypes[i]) + 1;
		size[1] = strlen(varnames[i]) + 1;
		fwrite(size, 1, 2, ptc->fp);

		fwrite(vartypes[i], 1, size[0], ptc->fp);
		fwrite(varnames[i], 1, size[1], ptc->fp);
		if (strcmp(vartypes[i],"float") == 0) datasize ++;
		else if (strcmp(vartypes[i],"color") == 0) datasize += 3;
		else if (strcmp(vartypes[i],"normal") == 0) datasize += 3;
		else if (strcmp(vartypes[i],"vector") == 0) datasize += 3;
		else if (strcmp(vartypes[i],"point") == 0) datasize += 3;
		else if (strcmp(vartypes[i],"matrix") == 0) datasize += 16;
		else
		{
			// Not sure it should abort()!!!
			datasize ++;
		}
	}
	ptc->nvars = nvars;
	ptc->datasize = datasize;
	fwrite(&datasize, sizeof(int), 1, ptc->fp);


	if (world2eye != NULL)
	{
		exist = 1;
		fwrite(&exist, 1, 1, ptc->fp);
		fwrite(world2eye,sizeof(float), 16, ptc->fp);
	}
	else
	{
		exist = 0;
		fwrite(&exist, 1, 1, ptc->fp);
	}

	if (world2ndc != NULL)
	{
		exist = 1;
		fwrite(&exist, 1, 1, ptc->fp);
		fwrite(world2ndc,sizeof(float), 16, ptc->fp);
	}
	else
	{
		exist = 0;
		fwrite(&exist, 1, 1, ptc->fp);
	}

	if (format != NULL)
	{
		exist = 1;
		fwrite(&exist, 1, 1, ptc->fp);
		fwrite(&format, sizeof(float), 3, ptc->fp);
	}
	else
	{
		exist = 0;
		fwrite(&exist, 1, 1, ptc->fp);
	}


	return (PtcPointCloud)(ptc);
}



//---------------------------------------------------------------------
/**
*
* Adds a point, along with its data, to a point cloud file.
* \param pointcloud
*             A handle to a point cloud file as returned by PtcCreatePointCloudFile.
*        point normal radius Position, orientation and radius of the point and data. point and normal
*        cannot be null.
* \param data Array of floats containing data for all variables, continuously in memory.
*            The data must be of the same size as the sum of sizes of the variables
*        passed to PtcCreatePointCloudFile.
* \return 1 if the operation is successful, 0 otherwise.
*/
extern "C" int PtcWriteDataPoint ( PtcPointCloud pointcloud, float *point, float *normal, float radius, float *data)
{
	int error = 0;
	PtcPointCloudHandle * ptc = (PtcPointCloudHandle *)(pointcloud);
	if (!ptc || ptc->signature != PTCVERSION)
	{
		error = 1;
	}
	else
	{
		// update the bounding box
		ptc->bbox[0] = MIN(ptc->bbox[0], point[0]);
		ptc->bbox[1] = MAX(ptc->bbox[1], point[0]);
		ptc->bbox[2] = MIN(ptc->bbox[2], point[1]);
		ptc->bbox[3] = MAX(ptc->bbox[3], point[1]);
		ptc->bbox[4] = MIN(ptc->bbox[4], point[2]);
		ptc->bbox[5] = MAX(ptc->bbox[5], point[2]);


		if (ptc->maxpoints == 0)
		{
			ptc->maxpoints = 1024;
			ptc->key = (PtcPointCloudKey *) (malloc(ptc->maxpoints * sizeof(PtcPointCloudKey) ));
			for (int i =0; i < 1024; i++)
				ptc->key[i].user_data = (float *) (malloc(ptc->datasize * sizeof(float) ));
		}
		else if (ptc->npoints >= ptc->maxpoints)
		{
			ptc->maxpoints += 1024;
			ptc->key = (PtcPointCloudKey *) (realloc(ptc->key, ptc->maxpoints * sizeof(PtcPointCloudKey) ));
			for (int i =ptc->npoints; i < ptc->maxpoints; i++)
				ptc->key[i].user_data = (float *) (malloc(ptc->datasize * sizeof(float) ));
		}

		for (int i = 0; i < 3; i ++)
		{
			ptc->key[ptc->npoints].point[i] = point[i];
			ptc->key[ptc->npoints].normal[i] = normal[i];
		}
		ptc->key[ptc->npoints].radius = radius;
		for (int i = 0; i < ptc->datasize; i ++)
		{
			ptc->key[ptc->npoints].user_data[i] = data[i];
		}
		ptc->npoints ++;
	}
	return error;
}


//---------------------------------------------------------------------
/**
* Writes out all data to disk and closes the file.
*/
extern "C" void PtcFinishPointCloudFile ( PtcPointCloud pointcloud)
{
	int error = 0;
	PtcPointCloudHandle * ptc = (PtcPointCloudHandle *)(pointcloud);
	if (!ptc || ptc->signature != PTCVERSION)
	{
		error = 1;
	}
	else if (ptc->fp != NULL)
	{
		fwrite(&ptc->bbox, sizeof(float), 6, ptc->fp);

		fwrite(&ptc->npoints, sizeof(int), 1, ptc->fp);

		if (ptc->npoints)
		{
			for (int i = 0; i < ptc->npoints; i++)
			{
				fwrite(ptc->key[i].point, sizeof(float), 3, ptc->fp);
				fwrite(ptc->key[i].normal, sizeof(float), 3, ptc->fp);
				fwrite(&ptc->key[i].radius, sizeof(float), 1, ptc->fp);
				fwrite(ptc->key[i].user_data, sizeof(float), ptc->datasize, ptc->fp);

			}
		}

		PtcClosePointCloudFile(pointcloud);
	}
}
