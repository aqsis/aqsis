// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\brief A program to test libslxargs.
		\author Douglas Ward (dsward@vidi.com)
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#define testaqsis
//#define testbmrt

#define shaderName "spotlight"

/* 
 *	To test with Aqsis -
 *		Link with  libslxargs, libaqsis, libddmsimple, libaqsistypes, 
 *				librib2ri, libtiff, libstdc++
 */
#define shaderPath "../shaders/"
#include "slx.h"

/*
 *	To compile with BMRT2.6 for Linux -
 *		g++ -Dtestbmrt -L../lib/ -rdynamic -o testslxargs testslxargs.cpp \
 *				-lribout -lstdc++ -ldl -lm -lc
 */
#ifdef testbmrt
#define shaderPath "/home/dsward/BMRT2.6/shaders/"
#include "../include/slc.h"
#define SLX_TYPE SLC_TYPE
#define SLX_TYPE_UNKNOWN SLC_TYPE_UNKNOWN
#define SLX_TYPE_POINT SLC_TYPE_POINT
#define SLX_TYPE_COLOR SLC_TYPE_COLOR
#define SLX_TYPE_SCALAR SLC_TYPE_SCALAR
#define SLX_TYPE_STRING SLC_TYPE_STRING
#define SLX_VISSYMDEF SLC_VISSYMDEF
#define SLX_SetPath SLC_SetPath
#define SLX_GetPath SLC_GetPath
#define SLX_SetShader SLC_SetShader
#define SLX_GetName SLC_GetName
#define SLX_GetType SLC_GetType
#define SLX_GetNArgs SLC_GetNArgs
#define SLX_GetArgById SLC_GetArgById
#define SLX_GetArgByName SLC_GetArgByName
#define SLX_EndShader SLC_EndShader
#define SLX_TypetoStr SLC_TypetoStr
#define SLX_StortoStr SLC_StortoStr
#define SLX_DetailtoStr SLC_DetailtoStr
#endif

namespace Aqsis
{
	struct IqRenderer;
	IqRenderer* QGetRenderContextI()
	{
		return(NULL);
	}
}

/*
 *
 */
int main(int argc, char *argv[])
{    
	printf("slxtestargs\n");
    SLX_SetPath(shaderPath);
    if (SLX_SetShader(shaderName) == 0)
    {
        // SLX_SetShader successful
        char * slxPath;
        char * slxName;
        SLX_TYPE slxType;
        int	nArgs;
        int i;
        SLX_VISSYMDEF * symPtr;
        char * slxTypeStr;
        char * slxStorStr;
        char * slxDetailStr;

        printf("shaderPath: ");
        printf(shaderPath);
        printf("\n");
        printf("shaderName: ");
        printf(shaderName);
        printf("\n");
 
        slxPath = SLX_GetPath();
        printf("GetPath result: %s\n", slxPath);
        
        slxName = SLX_GetName();
        printf("GetName result: %s\n", slxName);
        
        slxType = SLX_GetType();
        printf("GetType result: %s\n", SLX_TypetoStr(slxType));

        nArgs = SLX_GetNArgs();
        printf("GetNArgs result: %d\n\n", nArgs);
        
        for (i=0; i<nArgs; i++)
        {
            symPtr = SLX_GetArgById(i);
            if (symPtr != NULL)
            {
        		printf("Indexed shader argument #%d results from GetArgById:\n", i);
                
                if (symPtr->svd_name != NULL)
                {
                    printf("   Name: %s\n", symPtr->svd_name);
                }
                else
                {
                    printf("   Name: ERROR - svd_name is NULL\n");
                }
        		
                slxTypeStr = SLX_TypetoStr(symPtr->svd_type);
        		printf("   Type: %s\n", slxTypeStr);
        		
                slxStorStr = SLX_StortoStr(symPtr->svd_storage);
        		printf("   Storage: %s\n", slxStorStr);
                
                slxDetailStr = SLX_DetailtoStr(symPtr->svd_detail);
        		printf("   Detail: %s\n", slxDetailStr);

                if (symPtr->svd_spacename != NULL)
                {
                    if ((symPtr->svd_type == SLX_TYPE_POINT) || 
                            (symPtr->svd_type == SLX_TYPE_COLOR))
                    {
                        printf("   Spacename: %s\n", symPtr->svd_spacename);
                    }
                }
                else
                {
                    printf("   Spacename: ERROR - svd_spacename is NULL\n");
                }
                
                printf("   Array length: %d\n", symPtr->svd_arraylen);
                
                if (symPtr->svd_default.stringval != NULL)
                {
                    switch (symPtr->svd_type)
                    {
                        case SLX_TYPE_UNKNOWN:
                            printf("   Data: unknown");
                            break;
                        case SLX_TYPE_POINT:
                            printf("   Data: x=%f\n", symPtr->svd_default.pointval->xval);
                            printf("   Data: y=%f\n", symPtr->svd_default.pointval->yval);
                            printf("   Data: z=%f\n", symPtr->svd_default.pointval->zval);
                            break;
                        case SLX_TYPE_COLOR:
                            printf("   Data: r=%f\n", symPtr->svd_default.pointval->xval);
                            printf("   Data: g=%f\n", symPtr->svd_default.pointval->yval);
                            printf("   Data: b=%f\n", symPtr->svd_default.pointval->zval);
                            break;
                        case SLX_TYPE_SCALAR:
                            printf("   Data: val=%f\n", *(symPtr->svd_default.scalarval));
                            break;
                        case SLX_TYPE_STRING:
                            printf("   Data: val=%s\n", symPtr->svd_default.stringval);
                            break;
                        default:
                            printf("   Data: unknown");
                            break;
                    }
                }
                else
                {
                    printf("   Data: ERROR - null pointer to value");
                }
        		
        		printf("\n");
            }
        }
        
        symPtr = SLX_GetArgByName ("intensity");
        
        // SLX_GetArrayArgElement - Not implemented yet
        // symPtr - SLX_GetArrayArgElement(SLX_VISSYMDEF *array, int index);

        SLX_EndShader();
    }
    else
    {
        printf("Shader not found: \n");
        printf("shaderPath: ");
        printf(shaderPath);
        printf("\n");
        printf("shaderName: ");
        printf(shaderName);
        printf("\n");
    }
    
    exit(0);
}

