/******************************************************************************/
/* COPYRIGHT                                                                  */
/*                                                                            */
/* Copyright 2000 by Schroff Development Corporation, Shawnee-Mission,        */
/* Kansas, United States of America. All rights reserved.                     */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* This Display Driver is distributed as "freeware". There are no             */
/* restrictions on its' usage.                                                */
/*                                                                            */
/******************************************************************************/

#include <windows.h>
#include "d_sdcWin32.h"
#include <aqsis/ri/ndspy.h>



/******************************************************************************/
/* DspyImageQuery                                                             */
/*                                                                            */
/* Query the display driver for image size (if not specified in the open call)*/
/* and aspect ratio.                                                          */
/******************************************************************************/

PtDspyError DspyImageQuery(PtDspyImageHandle image,
                           PtDspyQueryType   type,
                           size_t	         size,
                           void              *data)
{
#if SHOW_CALLSTACK
	fprintf(stderr, "sdcWin32_DspyImageQuery called, type: %d.\n", type);
#endif

	PtDspyError          ret = PkDspyErrorNone;
	PtDspyOverwriteInfo  owi;
	PtDspySizeInfo       si;

	if ( size > 0 && data )
	{
		switch ( type )
		{
				case PkOverwriteQuery:

				if ( size > sizeof(owi) )
					size = sizeof(owi);

				owi.overwrite   = 1;
				owi.interactive = 0;

				memcpy(data, &owi, size);
				break;

				case PkSizeQuery:

				if ( size > sizeof(si) )
					size = sizeof(si);

				if ( image )
				{
					si.width       = ((AppData *)image)->bmi.bmiHeader.biWidth;
					si.height      = ((AppData *)image)->bmi.bmiHeader.biHeight;
					si.aspectRatio = DEFAULT_PIXELASPECTRATIO;
				}
				else
				{
					si.width       = DEFAULT_IMAGEWIDTH;
					si.height      = DEFAULT_IMAGEHEIGHT;
					si.aspectRatio = DEFAULT_PIXELASPECTRATIO;
				}

				memcpy(data, &si, size);
				break;

				default:
				ret = PkDspyErrorUnsupported;
				break;
		}
	}
	else
		ret = PkDspyErrorBadParams;

	return ret;
}

