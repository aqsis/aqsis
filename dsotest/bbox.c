/*
 * bbox() -- give the bounding box for the object(s) that use
 *   this function.
 *
 * This is a simple example of using shader DSO.  It tracks the min/max
 * xyz values.  When the render is done, the bounding box area containing all 
 * of the objects that use this function is given.
 *
 * HINTS:
 *   This function makes no assumptions as to what space the point data is
 *   in.  So the bounding box will be given in whatever space that src 
 *   point is in.  A good use of this function is to give back the raster
 *   space to for use with "Level of Detail" (LOD).
 *
 * AUTHOR: Tal Lancaster
 * HISTORY:
 *    created: Oct. 2, 1998
 *
 */


#include <stdio.h>
#include <shadeop.h>

SHADEOP_TABLE (bbox) = {
    { "float bbox (point)", "", "bbox_cleanup"},
    { "", "", ""}
};

typedef struct {
    float xmm [2];
    float ymm [2];
    float zmm [2];
} BBOX_DATA;

static BBOX_DATA bbox_data = {{1000, -1000},
			      {1000, -1000},
			      {1000, -1000}
};


SHADEOP (bbox) {
    float *result = (float *)argv[0];
    float *pos = (float*) argv[1];
   
    if (*pos < bbox_data.xmm[0])
	 bbox_data.xmm[0] = *pos;
    if (*pos > bbox_data.xmm[1])
	 bbox_data.xmm[1] = *pos;
    if (*(pos+1) < bbox_data.ymm[0])
	 bbox_data.ymm[0] = *(pos+1);
    if (*(pos+1) > bbox_data.ymm[1])
	 bbox_data.ymm[1] = *(pos+1);
    if (*(pos+2) < bbox_data.zmm[0])
	 bbox_data.zmm[0] = *(pos+2);
    if (*(pos+2) > bbox_data.zmm[1])
	 bbox_data.zmm[1] = *(pos+2);
    *result = bbox_data.xmm[1];
    return 0;
}

SHADEOP_SHUTDOWN (bbox_cleanup) {
    fprintf (stderr, 
	     "BBOX(xyz min/max): %.1f %.1f %.1f %.1f %.1f %.1f\n",
	     bbox_data.xmm[0], bbox_data.xmm[1],  
	     bbox_data.ymm[0], bbox_data.ymm[1],  
	     bbox_data.zmm[0], bbox_data.zmm[1]);
    fprintf (stderr, "BBOX(xyz): %.1f %.1f %.1f\n",
	      bbox_data.xmm[1] - bbox_data.xmm[0],
	      bbox_data.ymm[1] - bbox_data.ymm[0],
	      bbox_data.zmm[1] - bbox_data.zmm[0]);
}


