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


#include <iostream>

#undef DSPY_INTERNAL
#include <aqsis/ri/ndspy.h>
#include "debugdd.h"
#include <aqsis/util/logging.h>

PtDspyError DebugDspyImageQuery ( PtDspyImageHandle image, PtDspyQueryType type, size_t size, void *data )
{
	Aqsis::log() << Aqsis::debug << "Entering DspyImageQuery\n";
	if( size<=0 || !data )
	{
		return PkDspyErrorBadParams;
	}

	switch( type )
	{
		case PkSizeQuery:
			Aqsis::log() << Aqsis::debug << "DspyImageQuery: type = PkSizeQuery\n";
			break;

		case PkOverwriteQuery:
			Aqsis::log() << Aqsis::debug << "DspyImageQuery: type = PkOverwriteQuery\n";
			break;

		default:
			return PkDspyErrorUnsupported;
	}

	Aqsis::log() << Aqsis::debug << "DspyImageQuery: size = " << size << "\n";

	return PkDspyErrorNone;
}

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
                               )
{
	// SHould do more useful things in here, output channels and so on that are
	// being rendered.

	Aqsis::log() << Aqsis::debug << "Entering DspyImageOpen\n";

	flagsstuff->flags |= PkDspyFlagsWantsEmptyBuckets;

	return PkDspyErrorNone;
}


PtDspyError DebugDspyImageData ( PtDspyImageHandle image,
                                 int xmin,
                                 int xmax_plus_one,
                                 int ymin,
                                 int ymax_plus_one,
                                 int entrysize,
                                 const unsigned char *data
                               )
{
	Aqsis::log() << Aqsis::debug << "Entering DspyImageData\n";

	// Should do something useful here with the bucket data.

	return PkDspyErrorNone;
}


PtDspyError DebugDspyImageClose ( PtDspyImageHandle image )
{
	Aqsis::log() << Aqsis::debug << "Entering DspyImageClose\n";

	return PkDspyErrorNone;
}

PtDspyError DebugDspyDelayImageClose ( PtDspyImageHandle image )
{
	Aqsis::log() << Aqsis::debug << "Entering DspyDelayImageClose\n";

	return PkDspyErrorNone;
}



