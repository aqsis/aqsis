#include <stdio.h>
#include <stdlib.h>
#include <ri.h>
char *ri2rib_ascii = "Ascii";
char *ri2rib_binary = "Binary";
char *ri2rib_space = "Space";
char *ri2rib_tab = "Tab";
char *ri2rib_none = "None";
char *ri2rib_gzip = "Gzip";

int main(int argc, char **argv)
{
    RtPoint *P;
    RtInt *nvertices;
    RtFloat constantwidth = 0.01;
    RtFloat detail;
    int i, j, ncurves = 50, nsegs = 2, ncvs;
    char buf[256];

    while (gets(buf) != NULL) {
        sscanf(buf, "%g %d", &detail, &ncurves);

	printf("Hello, World!\n");
	detail = 1;
        ncvs = 3 * nsegs + 1;
        P = (RtPoint *)malloc(ncvs * ncurves * sizeof(RtPoint));
        nvertices = (RtInt *)malloc(ncurves * sizeof(RtInt));
//	RiOption ( "RI2RIB_Output",
//	 ( RtToken ) "Compression", ( RtPointer ) & ri2rib_gzip, RI_NULL );
//	RiOption ( "RI2RIB_Output",
//	 ( RtToken ) "Type", ( RtPointer ) & ri2rib_binary, RI_NULL );
        RiBegin(RI_NULL);
        for (i = 0; i < ncurves; i++) {
            nvertices[i] = ncvs;
            P[i*ncvs][0] = 2 * ((RtFloat)rand() / RAND_MAX) - 1;
            P[i*ncvs][1] = 2 * ((RtFloat)rand() / RAND_MAX) - 1;
            P[i*ncvs][2] = 0.0;
            for (j = 1; j < ncvs; j++) {
                P[i*ncvs+j][0] = 2 * ((RtFloat)rand() / RAND_MAX) - 1;
                P[i*ncvs+j][1] = 2 * ((RtFloat)rand() / RAND_MAX) - 1;
                P[i*ncvs+j][2] = 0.0;
            }
        }
	RiSphere(0.9,-0.9,0.9,360,0);
        RiArchiveRecord(RI_COMMENT, "\377");
        RiEnd();
    }
    return 0;
}
