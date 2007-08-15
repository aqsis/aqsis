// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
		\brief Various helper functions for handling daya in the Dspy interface.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include "ndspy.h"
#include <stdlib.h>
#include <string.h>

PtDspyError DspyFindStringInParamList(const char *string, char **result, int n, const UserParameter *p)
{
	int i;

	for (i = 0; i < n; i++, p++)
	{
		if (p->vtype == 's' &&
		        p->name[0] == string[0] &&
		        strcmp(p->name, string) == 0)
		{
			*result = *(char **)p->value;
			return(PkDspyErrorNone);
		}
	}
	return(PkDspyErrorNoResource);
}

PtDspyError DspyFindMatrixInParamList(const char *string, float *result, int n, const UserParameter *p)
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
			return(PkDspyErrorNone);
		}
	}
	return(PkDspyErrorNoResource);
}

PtDspyError DspyFindFloatInParamList(const char *string, float *result, int n, const UserParameter *p)
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
				return(PkDspyErrorNone);
			}
			else
			{
				*result = (float)(*(int *)p->value);
				return(PkDspyErrorNone);
			}
		}
	}
	return(PkDspyErrorNoResource);
}

PtDspyError DspyFindFloatsInParamList(const char *string, int *resultCount, float *result, int n, const UserParameter *p)
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
				return(PkDspyErrorNone);
			}
		}
	}

	return(PkDspyErrorNoResource);
}

PtDspyError DspyFindIntInParamList(const char *string, int *result, int n, const UserParameter *p)
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
				return(PkDspyErrorNone);
			}
		}
	}

	return(PkDspyErrorNoResource);
}

PtDspyError DspyFindIntsInParamList(const char *string, int *resultCount, int *result, int n, const UserParameter *p)
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
				return(PkDspyErrorNone);
			}
		}
	}
	return PkDspyErrorNoResource;
}

PtDspyError DspyReorderFormatting(int formatCount, PtDspyDevFormat *format, int outFormatCount, const PtDspyDevFormat *outFormat)
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

	return(ret);
}

void DspyMemReverseCopy(unsigned char *t, const unsigned char *s, int n)
{
	int i;

	s += n;
	for (i = 0; i < n; i++)
		*t++ = *--s;
}
