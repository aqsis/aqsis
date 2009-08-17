/*
 * dspyhlpr.h - Set of basic functions for writing display drivers
 *
 * Note: Source code for these functions are in ../etc/dspyhlpr.c
 *
 * RenderDotC (R) is:
 *      Copyright (c) 1996-2001 by Dot C Software, Inc.
 *      All Rights Reserved.
 * RenderDotC (R) is a registered trademark of Dot C Software, Inc.
 */

#ifndef DSPYHLPR_H_INCLUDED
#define DSPYHLPR_H_INCLUDED

#include <aqsis/ri/ndspy.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern PtDspyError
	DspyFindStringInParamList(const char *string, char **result,
	                          int n, const UserParameter *p),
	DspyFindStringsInParamList(const char *string, char ***result,
	                          int n, const UserParameter *p),
	DspyFindMatrixInParamList(const char *string, float *result,
	                          int n, const UserParameter *p),
	DspyFindFloatInParamList(const char *string, float *result,
	                         int n, const UserParameter *p),
	DspyFindFloatsInParamList(const char *string, int *resultCount,
	                          float *result, int n, const UserParameter *p),
	DspyFindIntInParamList(const char *string, int *result, int n,
	                       const UserParameter *p),
	DspyFindIntsInParamList(const char *string, int *resultCount,
	                        int *result, int n, const UserParameter *p),
	DspyReorderFormatting(int formatCount, PtDspyDevFormat *format,
	                      int outFormatCount, const PtDspyDevFormat *outFormat);

extern void
	DspyMemReverseCopy(unsigned char *t, const unsigned char *s, int len);

#ifdef __cplusplus
}
#endif

#endif // DSPYHLPR_H_INCLUDED
