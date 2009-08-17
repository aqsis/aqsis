/*
 * dspyhlpr.c - Set of basic functions for writing display drivers
 *
 * RenderDotC (R) is:
 *      Copyright (c) 1996-2001 by Dot C Software, Inc.
 *      All Rights Reserved.
 * RenderDotC (R) is a registered trademark of Dot C Software, Inc.
 */

#include <string.h>			/* For strcmp */
#include <aqsis/ri/ndspy.h>

PtDspyError DspyFindStringInParamList(const char *string, char **result,
                                      int n, const UserParameter *p)
{
	int i;

	for (i = 0; i < n; i++, p++)
		if (p->vtype == 's' &&
		        p->name[0] == string[0] &&
		        strcmp(p->name, string) == 0)
		{
			*result = *(char **)p->value;
			return PkDspyErrorNone;
		}

	return PkDspyErrorNoResource;
}

PtDspyError DspyFindStringsInParamList(const char *string, char ***result,
                                      int n, const UserParameter *p)
{
	int i;

	for (i = 0; i < n; i++, p++)
		if (p->vtype == 's' &&
		        p->name[0] == string[0] &&
		        strcmp(p->name, string) == 0)
		{
			*result = (char **)p->value;
			return PkDspyErrorNone;
		}

	return PkDspyErrorNoResource;
}


PtDspyError DspyFindMatrixInParamList(const char *string, float *result,
                                      int n, const UserParameter *p)
{
	int i;

	for (i = 0; i < n; i++, p++)
	{
		if (p->vtype == 'f' &&
		        p->vcount == 16 &&
		        p->name[0] == string[0] &&
		        strcmp(p->name, string) == 0)
		{
			memcpy(result, (float *)p->value, 16 * sizeof(float));
			return PkDspyErrorNone;
		}
	}

	return PkDspyErrorNoResource;
}

PtDspyError DspyFindFloatInParamList(const char *string, float *result,
                                     int n, const UserParameter *p)
{
	int i;

	for (i = 0; i < n; i++, p++)
	{
		if ((p->vtype == 'f' || p->vtype == 'i') &&
		        p->name[0] == string[0] &&
		        strcmp(p->name, string) == 0)
		{
			if (p->vtype == 'f')
			{
				*result = *(float *)p->value;
				return PkDspyErrorNone;
			}
			else
			{
				*result = (float)(*(int *)p->value);
				return PkDspyErrorNone;
			}
		}
	}

	return PkDspyErrorNoResource;
}

PtDspyError DspyFindFloatsInParamList(const char *string, int *resultCount,
                                      float *result, int n, const UserParameter *p)
{
	int i, j, *ip;

	for (i = 0; i < n; i++, p++)
	{
		if ((p->vtype == 'f' || p->vtype == 'i') &&
		        p->name[0] == string[0] &&
		        strcmp(p->name, string) == 0)
		{
			if (p->vcount < *resultCount)
				*resultCount = p->vcount;
			if (p->vtype == 'f')
			{
				memcpy(result, (float *)p->value, *resultCount * sizeof(float));
				return PkDspyErrorNone;
			}
			else
			{
				for (j = 0, ip = (int *)p->value; j < *resultCount; j++)
					*result++ = (float)*ip++;
				return PkDspyErrorNone;
			}
		}
	}

	return PkDspyErrorNoResource;
}

PtDspyError DspyFindIntInParamList(const char *string, int *result,
                                   int n, const UserParameter *p)
{
	int i;

	for (i = 0; i < n; i++, p++)
	{
		if ((p->vtype == 'i' || p->vtype == 'f') &&
		        p->name[0] == string[0] &&
		        strcmp(p->name, string) == 0)
		{
			if (p->vtype == 'i')
			{
				*result = *(int *)p->value;
				return PkDspyErrorNone;
			}
			else
			{
				*result = (int)(*(float *)p->value);
				return PkDspyErrorNone;
			}
		}
	}

	return PkDspyErrorNoResource;
}

PtDspyError DspyFindIntsInParamList(const char *string, int *resultCount,
                                    int *result, int n, const UserParameter *p)
{
	int i, j;
	float *fp;

	for (i = 0; i < n; i++, p++)
	{
		if ((p->vtype == 'i' || p->vtype == 'f') &&
		        p->name[0] == string[0] &&
		        strcmp(p->name, string) == 0)
		{
			if (p->vcount < *resultCount)
				*resultCount = p->vcount;
			if (p->vtype == 'i')
			{
				memcpy(result, (int *)p->value, *resultCount * sizeof(int));
				return PkDspyErrorNone;
			}
			else
			{
				for (j = 0, fp = (float *)p->value; j < *resultCount; j++)
					*result++ = (int)*fp++;
				return PkDspyErrorNone;
			}
		}
	}

	return PkDspyErrorNoResource;
}

PtDspyError DspyReorderFormatting(int formatCount, PtDspyDevFormat *format,
                                  int outFormatCount, const PtDspyDevFormat *outFormat)
{
	PtDspyError ret = PkDspyErrorNone;
	int i, j;

	if (formatCount < outFormatCount)
		outFormatCount = formatCount;

	for (i = 0; i < outFormatCount; i++)
	{
		for (j = i; j < formatCount; j++)
		{
			if (format[j].name[0] == outFormat[i].name[0] &&
			        strcmp(format[j].name, outFormat[i].name) == 0)
			{
				if (i != j)
				{
					PtDspyDevFormat tmpFormat;
					tmpFormat = format[i];
					format[i] = format[j];
					format[j] = tmpFormat;
				}
				if (outFormat[i].type)
					format[i].type = outFormat[i].type;
				break;
			}
		}
		if (j >= formatCount)
			ret = PkDspyErrorBadParams;
	}

	return ret;
}

void DspyMemReverseCopy(unsigned char *t, const unsigned char *s, int n)
{
	int i;

	s += n;
	for (i = 0; i < n; i++)
		*t++ = *--s;
}
