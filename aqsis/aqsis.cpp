//#include "ddmsimple.h"
#include "renderer.h"

#include "ri.h"

int main(int argc, char* argv[])
{
	RtPoint square[4] = {{.7,.5,.5}, {.5, -.5, .5}, {-.5, -.5, .5}, {-.5, .5, .5}};

	RiBegin("aqsis");
//		Aqsis::QGetRenderContext()->SetImage(new Aqsis::CqFileBuffer("test.tif"));

		RiFormat(160, 120, 1.0);
		RiWorldBegin();
			RiDisplay("test.tif", RI_FILE, RI_RGBA, RI_NULL);
			RiPolygon(4, RI_P, RtPointer(square), RI_NULL);
		RiWorldEnd();
	RiEnd();
	
	return 0;
}



