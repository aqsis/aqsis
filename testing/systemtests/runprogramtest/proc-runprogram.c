#include <stdio.h>
#include <stdlib.h>
#include <ri.h>
#include <math.h>

char *ri2rib_ascii = "Ascii";
char *ri2rib_binary = "Binary";
char *ri2rib_space = "Space";
char *ri2rib_tab = "Tab";
char *ri2rib_none = "None";
char *ri2rib_gzip = "Gzip";
char *curve_type = "cubic";

rands(RtFloat x[])
{

	RtFloat s,xx,yy;

	do
	{
		xx = 2.0f * ((RtFloat)rand() / RAND_MAX) - 1.0f;
		yy = 2.0f * ((RtFloat)rand() / RAND_MAX) - 1.0f;
		s = xx*xx + yy*yy;
	} while(s > 1.0f || s == 0.0f );

	x[2] = 2.0f*s - 1.0f;
	s = (RtFloat)sqrt((1.0f-x[2]*x[2])/s);
	x[0] = s*xx;
	x[1] = s*yy;
}

int main(int argc, char **argv)
{
	RtPoint *P;
	RtInt *nvertices;
	RtFloat constantwidth = 0.01f;
	RtFloat detail;
	int i, j, ncurves = 50, nsegs = 2, ncvs;
	char buf[256];
	RtFloat width = 0.01f;
	RtFloat point[3];
	RtFloat scale = 2;

	while (gets(buf) != NULL)
	{
		sscanf(buf, "%g %d", &detail, &ncurves);

		printf("Hello, World!\n");
		detail = 1;
		ncvs = 3 * nsegs + 1;
		P = (RtPoint *)malloc(ncvs * ncurves * sizeof(RtPoint));
		nvertices = (RtInt *)malloc(ncurves * sizeof(RtInt));
	//	RiOption ( "RI2RIB_Output", ( RtToken ) "Compression", ( RtPointer ) & ri2rib_gzip, RI_NULL );
	//	RiOption ( "RI2RIB_Output", ( RtToken ) "Type", ( RtPointer ) & ri2rib_binary, RI_NULL );
		RiBegin(RI_NULL);
		for (i = 0; i < ncurves; i++)
		{
			nvertices[i] = ncvs;
			rands(point);
			P[i*ncvs][0] = 
			P[i*ncvs][1] = 
			P[i*ncvs][2] = 0.0f;
			for (j = 1; j < ncvs; j++) 
			{
				P[i*ncvs+j][0] = point[0] * ((RtFloat)j/ncvs) * scale;
				P[i*ncvs+j][1] = point[1] * ((RtFloat)j/ncvs) * scale;
				P[i*ncvs+j][2] = point[2] * ((RtFloat)j/ncvs) * scale;
			}
		}
		RiCurves(curve_type, ncurves, nvertices, "nonperiodic", "P", P, "constantwidth", &width, RI_NULL);
		RiSphere(0.9f,-0.9f,0.9f,360.0f, RI_NULL);
		RiArchiveRecord(RI_COMMENT, "\377");
		RiEnd();
    }
    return 0;
}
