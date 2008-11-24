/*
 * bake3d() -- give the possibility to save P,N and multiple channels using the new Ptcapi.
 *
 * This is based from simple example of using shader DSO bbox.
 * It is not very fancy bake3d() but it will save <float, vector, point, color, normal> and p, n, radius in one file (cloudpoint).
*
 * Each time an element is using PtcWriteDataPoint() the value is stored with the handle 'PtcPointCloud'. On the PtcFinishPointCloudFile() the file is effectively saved and closed.
 *
 * texture3d() works the same way it uses P to find the corresponding
 *   N, <color, float, normal, color, point, vector> information.
 *
 * HINTS:
 * see shaders/envlight.sl, bake3d.sl, texture3d.sl to have an idea
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shadeop.h>

extern "C"
{
#include "ptcapi.h"


SHADEOP_TABLE (bake3d) = {
                             { "float bake3f (string, string, point, normal, string, float)", "map_init", "map_cleanup"},
                             { "float bake3v (string, string, point, normal, string, vector)", "map_init", "map_cleanup"},
                             { "float bake3p (string, string, point, normal, string, point)", "map_init", "map_cleanup"},
                             { "float bake3n (string, string, point, normal, string, normal)", "map_init", "map_cleanup"},
                             { "float bake3c (string, string, point, normal, string, color)", "map_init", "map_cleanup"},

                             { "float bake3ft (string, string, point, normal, string, float, string, vector, string,  matrix, string, matrix, string, float)", "map_init", "map_cleanup"},
                             { "float bake3vt (string, string, point, normal, string, vector, string, vector, string,  matrix, string, matrix, string, float)", "map_init", "map_cleanup"},
                             { "float bake3pt (string, string, point, normal, string, point, string, vector, string,  matrix, string, matrix, string, float)", "map_init", "map_cleanup"},
                             { "float bake3nt (string, string, point, normal, string, normal, string, vector, string,  matrix, string, matrix, string, float)", "map_init", "map_cleanup"},
                             { "float bake3ct (string, string, point, normal, string, color, string, vector, string,  matrix, string, matrix, string, float)", "map_init", "map_cleanup"},

                             { "float bake3fr (string, string, point, normal, string, float, string, float)", "map_init", "map_cleanup"},
                             { "float bake3vr (string, string, point, normal, string, vector, string, float)", "map_init", "map_cleanup"},
                             { "float bake3pr (string, string, point, normal, string, point, string, float)", "map_init", "map_cleanup"},
                             { "float bake3nr (string, string, point, normal, string, normal, string, float)", "map_init", "map_cleanup"},
                             { "float bake3cr (string, string, point, normal, string, color, string, float)", "map_init", "map_cleanup"},
                             { "", "", ""}
                         };

SHADEOP_TABLE (texture3d) = {
                                { "void texture3f (string, point, normal, string, float)", "map_init", "map_cleanup"},
                                { "void texture3v (string, point, normal, string, vector)", "map_init", "map_cleanup"},
                                { "void texture3p (string, point, normal, string, point)", "map_init", "map_cleanup"},
                                { "void texture3n (string, point, normal, string, normal)", "map_init", "map_cleanup"},
                                { "void texture3c (string, point, normal, string, color)", "map_init", "map_cleanup"},
                                { "", "", ""}
                            };


typedef struct
{
	char filename[1024];
	PtcPointCloud CloudFile;
	unsigned char ReadWrite;
}
PtcMapEntries;

struct
{
	unsigned short MaxFiles;
	unsigned short CurrentFiles;
	PtcMapEntries *pList;
}
MyList;

PtcPointCloud FindCloudRead(char *s)
{

	//printf("%s MaxFiles %d CurrentFiles %d \n", s, MyList.MaxFiles, MyList.CurrentFiles);
	int i;
	for (i=0; i < MyList.CurrentFiles; i++)
	{
		//printf("name of %s %s\n", s, MyList.pList[i].filename);
		if (MyList.pList[i].ReadWrite == 0 && strcmp(MyList.pList[i].filename, s) == 0)
		{
			return MyList.pList[i].CloudFile;
		}
	}
	return 0;
}

PtcPointCloud FindCloudWrite(char *s)
{
	int i;
	for (i=0; i < MyList.CurrentFiles; i++)
	{
		if (MyList.pList[i].ReadWrite == 1 && strcmp(MyList.pList[i].filename, s) == 0)
		{
			return MyList.pList[i].CloudFile;
		}
	}
	return 0;
}

void SaveCloud(char *s, PtcPointCloud File)
{
	if (MyList.MaxFiles == 0)
	{
		MyList.pList = (PtcMapEntries*) calloc( 3, sizeof(PtcMapEntries));
		MyList.MaxFiles = 3;
		MyList.CurrentFiles = 0;
	}
	else if (MyList.CurrentFiles >= MyList.MaxFiles)
	{

		MyList.MaxFiles += 3;
		MyList.pList =(PtcMapEntries*) realloc(MyList.pList, MyList.MaxFiles  * sizeof(PtcMapEntries));
	}

	strcpy(MyList.pList[MyList.CurrentFiles].filename, s);
	MyList.pList[MyList.CurrentFiles].CloudFile = File;
}

void SaveCloudRead(char *s, PtcPointCloud File)
{
	//printf("name of %s\n", s);
	SaveCloud(s, File);
	MyList.pList[MyList.CurrentFiles].ReadWrite = 0;
	MyList.CurrentFiles ++;
}

void SaveCloudWrite(char *s, PtcPointCloud File)
{
	SaveCloud(s, File);
	MyList.pList[MyList.CurrentFiles].ReadWrite = 1;
	MyList.CurrentFiles ++;
}

/* bake3d one parameter of 'type */
static void bake3d_one(void *argv[], char *type)
{
	char *varnames[2];
	char *vartypes[2];

	STRING_DESC *ptc = (STRING_DESC *) argv[1];

	float *point = (float*) argv[3];
	float *normal = (float*) argv[4];
	STRING_DESC *name = (STRING_DESC*) argv[5];
	float *f = (float*) argv[6];


	varnames[0] = name->s;
	varnames[1] = 0;
	vartypes[0] = type;
	vartypes[1] = 0;


	PtcPointCloud MyCloudWrite = FindCloudWrite(ptc->s);
	if (!MyCloudWrite)
	{
		MyCloudWrite = (PtcPointCloud *) PtcCreatePointCloudFile((const char*) ptc->s, 1, (const char **) vartypes, (const char **) varnames, NULL, NULL, NULL);
		SaveCloudWrite(ptc->s, MyCloudWrite);
	}
	//printf("MyCloudWrite %p", MyCloudWrite);
	PtcWriteDataPoint(MyCloudWrite, point, normal, 0.01, f);
}

/* bake3d one parameter of 'type with radius */
static void bake3d_radius(void *argv[], char *type)
{
	char *varnames[2];
	char *vartypes[2];

	STRING_DESC *ptc = (STRING_DESC *) argv[1];

	float *point = (float*) argv[3];
	float *normal = (float*) argv[4];
	STRING_DESC *name = (STRING_DESC*) argv[5];
	float *f = (float*) argv[6];
	float *radius = (float*) argv[8]; // assume it will be radius

	varnames[0] = name->s;
	varnames[1] = 0;
	vartypes[0] = type;
	vartypes[1] = 0;

	PtcPointCloud MyCloudWrite = FindCloudWrite(ptc->s);
	if (!MyCloudWrite)
	{
		MyCloudWrite = (PtcPointCloud *) PtcCreatePointCloudFile((const char*) ptc->s, 1, (const char **) vartypes, (const char **) varnames, NULL, NULL, NULL);
		SaveCloudWrite(ptc->s, MyCloudWrite);
	}
	PtcWriteDataPoint(MyCloudWrite, point, normal, *radius, f);

}

/* bake3d one parameter of 'type with radius + eye2world, eye2screen, format */
static void bake3d_all(void *argv[], char *type)
{
	char *varnames[2];
	char *vartypes[2];

	STRING_DESC *ptc = (STRING_DESC *) argv[1];

	float *point = (float*) argv[3];
	float *normal = (float*) argv[4];
	STRING_DESC *name = (STRING_DESC*) argv[5];
	float *f = (float*) argv[6];
	float *format = (float*) argv[8];
	float *world2eye = (float*) argv[10];
	float *world2ndc = (float*) argv[12];
	float *radius = (float*) argv[14];

	varnames[0] = name->s;
	varnames[1] = 0;
	vartypes[0] = type;
	vartypes[1] = 0;

	PtcPointCloud MyCloudWrite = FindCloudWrite(ptc->s);
	if (!MyCloudWrite)
	{
		MyCloudWrite = PtcCreatePointCloudFile((const char*) ptc->s, 1, (const char **) vartypes, (const char **) varnames, world2eye, world2ndc, format);
		SaveCloudWrite(ptc->s, MyCloudWrite);
	}
	PtcWriteDataPoint(MyCloudWrite, point, normal, *radius, f);
}



SHADEOP (bake3f)
{
	bake3d_one(argv, "float");
	return 0;
}

SHADEOP (bake3v)
{
	bake3d_one(argv, "vector");
	return 0;
}

SHADEOP (bake3p)
{
	bake3d_one(argv, "point");
	return 0;
}
SHADEOP (bake3n)
{
	bake3d_one(argv, "normal");
	return 0;
}
SHADEOP (bake3c)
{
	bake3d_one(argv, "color");
	return 0;
}

SHADEOP (bake3ft)
{
	bake3d_all(argv, "float");
	return 0;
}

SHADEOP (bake3vt)
{
	bake3d_all(argv, "vector");
	return 0;
}

SHADEOP (bake3pt)
{
	bake3d_all(argv, "point");
	return 0;
}

SHADEOP (bake3nt)
{
	bake3d_all(argv, "normal");
	return 0;
}

SHADEOP (bake3ct)
{
	bake3d_all(argv, "color");
	return 0;
}

SHADEOP (bake3fr)
{
	bake3d_radius(argv, "float");
	return 0;
}

SHADEOP (bake3vr)
{
	bake3d_radius(argv, "vector");
	return 0;
}

SHADEOP (bake3pr)
{
	bake3d_radius(argv, "point");
	return 0;
}

SHADEOP (bake3nr)
{
	bake3d_radius(argv, "normal");
	return 0;
}

SHADEOP (bake3cr)
{
	bake3d_radius(argv, "color");
	return 0;
}

// Read one parameter from a point cloud file
void texture3d_one(void *argv[])
{
	int okay = 0;
	char *varnames[2];

	STRING_DESC *ptc = (STRING_DESC *) argv[1];
	float *point = (float*) argv[2];
	float *normal = (float*) argv[3];
	STRING_DESC *name = (STRING_DESC*) argv[4];
	float *v = (float*) argv[5];

	varnames[0] = name->s;
	varnames[1] = 0;


	PtcPointCloud MyCloudRead = FindCloudRead(ptc->s);
	if (!MyCloudRead )
	{
		int n;
		int i;

		MyCloudRead = PtcOpenPointCloudFile((const char*) ptc->s, &n, NULL, (const char **) varnames);

		// Double check if this variable exist in this PTC...
		for (i=0; i < n && !okay; i++)
		{
			if (strcmp(varnames[i], name->s) == 0)
			{
				okay = 1;
			}
		}
		if (!okay)
		{
			PtcClosePointCloudFile(MyCloudRead);
			MyCloudRead = 0;
		}
		else
		{
			SaveCloudRead(ptc->s, MyCloudRead);
		}
	}

	if (MyCloudRead != 0)
	{
		float radius;


		/*
		        find will do a qsort/bsearch based on point
		        okay = PtcFindDataPoint(MyCloudRead, point, normal, &radius, v);
		*/
		okay = PtcReadDataPoint(MyCloudRead, point, normal, &radius, v);
	}
}

SHADEOP (texture3f)
{
	texture3d_one(argv);
	return 0;
}
SHADEOP (texture3v)
{
	texture3d_one(argv);
	return 0;
}

SHADEOP (texture3n)
{
	texture3d_one(argv);
	return 0;
}

SHADEOP (texture3p)
{
	texture3d_one(argv);
	return 0;
}

SHADEOP (texture3c)
{
	texture3d_one(argv);
	return 0;
}

SHADEOP_SHUTDOWN (map_cleanup)
{
	//printf("map_cleanup");
        int i;
	for ( i = 0; i < MyList.CurrentFiles; i++)
	{
		if (MyList.pList[i].CloudFile)
		{
			if (MyList.pList[i].ReadWrite)
				PtcFinishPointCloudFile(MyList.pList[i].CloudFile);
			else
				PtcClosePointCloudFile( MyList.pList[i].CloudFile);
		}
	}
	MyList.CurrentFiles = 0;
	MyList.MaxFiles = 0;
	free(MyList.pList);
	MyList.pList = 0;
}

SHADEOP_INIT (map_init)
{
	MyList.MaxFiles = 0;
	MyList.CurrentFiles = 0;
	MyList.pList = NULL;

	return (void *) &MyList;
}
}
