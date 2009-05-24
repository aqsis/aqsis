//
//  Author: Tristan Colgate-McFarlane
//  Author: Derived from d_xpm by Alex Paes
//  
//  Project: This is a debug DD intended for internal linking into aqsis.
//
//  Description: 
//    This DD is intended for internal linking into Aqsis. It is intended as
//    a debugging aide to support a completely static build of Aqsis suitable
//    for debugging in situation where dynamic linking causes problems
//      The intent is to behave as much like a real dd as possible, without doing
//    anything useful


#ifndef ___debugdd_Loaded___
#define ___debugdd_Loaded___

#include <aqsis/ri/ndspy.h>

PtDspyError DebugDspyImageQuery ( PtDspyImageHandle image, PtDspyQueryType type, size_t size, void *data );
PtDspyError DebugDspyImageOpen ( PtDspyImageHandle *image,
                            const char *drivername,
                            const char *filename,
                            int width,
                            int height,
                            int paramcount,
                            const UserParameter *parameters,
                            int formatcount,
                            PtDspyDevFormat *format,
                            PtFlagStuff *flagsstuff
                          );
PtDspyError DebugDspyImageData ( PtDspyImageHandle image,
                            int xmin,
                            int xmax_plus_one,
                            int ymin,
                            int ymax_plus_one,
                            int entrysize,
                            const unsigned char *data
                          );
PtDspyError DebugDspyImageClose ( PtDspyImageHandle image );
PtDspyError DebugDspyDelayImageClose ( PtDspyImageHandle image );

#endif // ___debugdd_Loaded___


