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
		\brief Implements libslxargs, analogous to Pixar's sloarg library.
		\author Douglas Ward (dsward@vidi.com)
*/

/* TO DO:
 *  1 - Set svd_arraylen and svd_spacename in SLX_VISSYMDEF records.  Currently set to 0 and NULL respectively.
 *  2 - Implement SLX_GetArgByName() and SLX_GetArrayArgElement().
 *  3 - Implement libshadervm and use it instead of libaqsis.  Eliminate other unnecessary libs.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ri.h"
#include "slx.h"

#include "shadervm.h"
#include "renderer.h"
#include "librib2ri.h"
#include "rifile.h"

using namespace Aqsis;

#define kBufferSize 256
#define kModeSeek 0
#define kModeParse 1

// Global variables
RtInt SlxLastError;

static char *shaderSearchPathList = NULL;

static char *currentShaderSearchPath = NULL;
static char *currentShader = NULL;
static char *currentShaderFilePath = NULL;

static SLX_TYPE currentShaderType = SLX_TYPE_UNKNOWN;
static int currentShaderNArgs = 0;
static SLX_VISSYMDEF * currentShaderArgArray = NULL;

static char * SLX_TYPE_UNKNOWN_STR = "unknown";
static char * SLX_TYPE_POINT_STR = "point";
static char * SLX_TYPE_COLOR_STR = "color";
static char * SLX_TYPE_SCALAR_STR = "scalar";
static char * SLX_TYPE_STRING_STR = "string";
static char * SLX_TYPE_SURFACE_STR = "surface";
static char * SLX_TYPE_LIGHT_STR = "light";
static char * SLX_TYPE_DISPLACEMENT_STR = "displacement";
static char * SLX_TYPE_VOLUME_STR = "volume";
static char * SLX_TYPE_TRANSFORMATION_STR = "transformation";
static char * SLX_TYPE_IMAGER_STR = "imager";

static char * SLX_STOR_UNKNOWN_STR = "unknown";
static char * SLX_STOR_CONSTANT_STR = "constant";
static char * SLX_STOR_VARIABLE_STR = "variable";
static char * SLX_STOR_TEMPORARY_STR = "temporary";
static char * SLX_STOR_PARAMETER_STR = "parameter";
static char * SLX_STOR_GSTATE_STR = "gstate";

static char * SLX_DETAIL_UNKNOWN_STR = "unknown";
static char * SLX_DETAIL_VARYING_STR = "varying";
static char * SLX_DETAIL_UNIFORM_STR = "uniform";


/*
 * Open shader file specified in currentShaderFile path, return file pointer
 */
static FILE * OpenCurrentShader()
{
    FILE * shaderInputFile;
    shaderInputFile = NULL;
    if (currentShaderFilePath != NULL)
    {
        shaderInputFile = fopen(currentShaderFilePath, "r");
    }
    return shaderInputFile;
}


/*
 * Close shaderInputFile
 */
static int CloseCurrentShader(FILE * shaderInputFile)
{
    int result;
    result = fclose(shaderInputFile);
    return result;
}


/*
 * Return a pointer to an indexed record in a shader argument array
 */
SLX_VISSYMDEF * GetShaderArgRec(SLX_VISSYMDEF * theShaderArgArray, int argIdx)
{
    long unsigned int arrayPtr;
    long unsigned int arrayOffset;
    SLX_VISSYMDEF * recPtr;
    
    arrayPtr = (long unsigned int)theShaderArgArray;
    arrayOffset = (long unsigned int)argIdx * sizeof(SLX_VISSYMDEF);
    recPtr = (SLX_VISSYMDEF *)(arrayPtr + arrayOffset);
    return recPtr;
}


/*
 * Free storage allocated in shader argument array for default values
 */
static void FreeArgRecStorage(SLX_VISSYMDEF * theShaderArgArray, int theShaderNArgs)
{
    int argIdx;
    for (argIdx = 0; argIdx < currentShaderNArgs; argIdx++)
    {
        SLX_VISSYMDEF * theShaderArgRec;
        theShaderArgRec = GetShaderArgRec(currentShaderArgArray, argIdx);
       
        if (theShaderArgRec->svd_name != NULL)
        {
            free(theShaderArgRec->svd_name);
            theShaderArgRec->svd_name = NULL;
        }
        
        if (theShaderArgRec->svd_spacename != NULL)
        {
            free(theShaderArgRec->svd_spacename);
            theShaderArgRec->svd_spacename = NULL;
        }
        
        if (theShaderArgRec->svd_default.stringval != NULL)
        {
            free(theShaderArgRec->svd_default.stringval);
            theShaderArgRec->svd_default.stringval = NULL;
        }
    }
}


/*
 * Store shader arg record, allocating storage for fields reference by pointer
 */
static RtInt StoreShaderArgDef(int argArrayIdx, char * varName, SLX_TYPE varType, 
        char * spacename, char * defaultVal)
{
    SLX_TYPE theType;
    SLX_VISSYMDEF * theShaderArgRec;

    theShaderArgRec = GetShaderArgRec(currentShaderArgArray, argArrayIdx);

    theShaderArgRec->svd_name = varName;
    
    theShaderArgRec->svd_type = varType;
    
    theShaderArgRec->svd_storage = SLX_STOR_PARAMETER;
    
    theShaderArgRec->svd_detail = SLX_DETAIL_UNIFORM;
    
    theShaderArgRec->svd_spacename = spacename;
    
    theShaderArgRec->svd_default.stringval = defaultVal;
}


/*
 * Allocate storage for array of shader arg records
 */
static RtInt AllocateShaderArgArray(int varCount)
{
    SLX_VISSYMDEF * arrayStorage;
    RtInt result;

    result = RIE_NOERROR;
    
    arrayStorage = (SLX_VISSYMDEF *)malloc(sizeof(SLX_VISSYMDEF) * varCount);
    if (arrayStorage != NULL)
    {
        SLX_VISSYMDEF * theShaderArgRec;
        int i;
        for (i = 0; i < varCount; i++)
        {
            theShaderArgRec = GetShaderArgRec(arrayStorage, i);
            theShaderArgRec->svd_name = NULL;
            theShaderArgRec->svd_type = SLX_TYPE_UNKNOWN;
            theShaderArgRec->svd_storage = SLX_STOR_UNKNOWN;
            theShaderArgRec->svd_detail = SLX_DETAIL_UNKNOWN;
            theShaderArgRec->svd_spacename = NULL;
            theShaderArgRec->svd_arraylen = 0;
            theShaderArgRec->svd_default.stringval = NULL;
        }
        currentShaderArgArray = arrayStorage;
    }
    else
    {
        result = RIE_NOMEM;
    }
    return result;
} 


/*
 * Extract an indexed shader variable from a CqShaderVM object
 */
static void AddShaderVar(CqShaderVM * pShader, int i)
{
    CqShaderVariable * shaderVar;
    EqVariableType	theType;
    SLX_TYPE		slxType;
    CqString		varNameCqStr;
    char *			varNameCStr;
    char *			theVarNameStr;
    int 			nameLength;
    char *			defaultVal;
    int 			defaultValLength;
    char * 			spacename;

    shaderVar = pShader->GetShaderVarAt(i);
    if (shaderVar != NULL) 
    {
        theType = shaderVar->Type();
        
        varNameCqStr = shaderVar->strName();
        varNameCStr = (char *)varNameCqStr.c_str();
        nameLength = strlen(varNameCStr);
        theVarNameStr = (char *)malloc(nameLength + 1);
        strcpy(theVarNameStr, varNameCStr);
        
        spacename = NULL;
        
        switch (theType)
        {
            case Type_UniformFloatVariable:
                {
                    TqFloat			aTqFloat;
                    RtFloat			aRtFloat;
                    slxType = SLX_TYPE_SCALAR;
                    aTqFloat = (*static_cast<CqShaderVariableUniform<Type_Float,TqFloat>*>(shaderVar));
                    aRtFloat = aTqFloat;
                    defaultValLength = sizeof(RtFloat);
                    defaultVal = (char *)malloc(defaultValLength);
                    memcpy(defaultVal, &aRtFloat, defaultValLength);
                    StoreShaderArgDef(i, theVarNameStr, slxType, spacename, defaultVal);
                    StoreShaderArgDef(currentShaderNArgs, theVarNameStr, slxType, 
                            spacename, defaultVal);
                    currentShaderNArgs++;
                }
                break;
            case Type_UniformStringVariable:
                {
                    CqString		aCqString;
                    char *			aCString;
                    slxType = SLX_TYPE_STRING;
                    aCqString = (*static_cast<CqShaderVariableUniform<Type_String,CqString>*>(shaderVar));
                    aCString = (char *)aCqString.c_str();
                    defaultValLength = strlen(aCString) + 1;
                    defaultVal = (char *)malloc(defaultValLength);
                    strcpy(defaultVal, aCString);
                    StoreShaderArgDef(i, theVarNameStr, slxType, spacename, defaultVal);
                    StoreShaderArgDef(currentShaderNArgs, theVarNameStr, slxType, 
                            spacename, defaultVal);
                    currentShaderNArgs++;
                }
                break;
            case Type_UniformPointVariable:
                {
                    CqVector3D		aCqVector3D;
                    RtPoint			aRtPoint;
                    slxType = SLX_TYPE_POINT;
                    aCqVector3D = (*static_cast<CqShaderVariableUniform<Type_Point,CqVector3D>*>(shaderVar));
                    aRtPoint[0] = aCqVector3D[0];
                    aRtPoint[1] = aCqVector3D[1];
                    aRtPoint[2] = aCqVector3D[2];
                    defaultValLength = sizeof(RtPoint);
                    defaultVal = (char *)malloc(defaultValLength);
                    memcpy(defaultVal, &aRtPoint, defaultValLength);
                    StoreShaderArgDef(i, theVarNameStr, slxType, spacename, defaultVal);
                    StoreShaderArgDef(currentShaderNArgs, theVarNameStr, slxType, 
                            spacename, defaultVal);
                    currentShaderNArgs++;
                }
                break;
            case Type_UniformColorVariable:
                {
                    CqColor			aCqColor;
                    RtColor			aRtColor;
                    slxType = SLX_TYPE_COLOR;
                    aCqColor = (*static_cast<CqShaderVariableUniform<Type_Color,CqColor>*>(shaderVar));
                    aRtColor[0] = aCqColor.fRed();
                    aRtColor[1] = aCqColor.fGreen();
                    aRtColor[2] = aCqColor.fBlue();
                    defaultValLength = sizeof(RtColor);
                    defaultVal = (char *)malloc(defaultValLength);
                    memcpy(defaultVal, &aRtColor, defaultValLength);
                    StoreShaderArgDef(currentShaderNArgs, theVarNameStr, slxType, 
                            spacename, defaultVal);
                    currentShaderNArgs++;
                }
                break;
            default:
                break;
        }
    }
}


/*
 * Read shader file, set these global variables -
 *    currentShaderType, currentShaderNArgs, currentShaderArgArray.
 */
static RtInt GetCurrentShaderInfo(char * strName)
{
    RtInt result;
    int i;
    int varCount;
    EqShaderType aShaderType;

    librib2ri::Engine renderengine;
    RiBegin("CRIBBER");
    
    CqString strFilename(strName);
    CqRiFile SLXFile(strFilename.c_str(),"");
    result = RIE_NOERROR;

    if(SLXFile.IsValid())
    {
        CqShaderVM* pShader=new CqShaderVM();
        pShader->LoadProgram(SLXFile);
        pShader->SetstrName(strName);
        pShader->ExecuteInit();
 
        varCount = pShader->GetShaderVarCount();
        
        AllocateShaderArgArray(varCount);
        
        currentShaderType = SLX_TYPE_UNKNOWN;
        aShaderType = pShader->Type();
        switch (aShaderType)
        {
            case Type_Surface:
                currentShaderType = SLX_TYPE_SURFACE;
                break;
            case Type_Lightsource:
                currentShaderType = SLX_TYPE_LIGHT;
                break;
            case Type_Volume:
                currentShaderType = SLX_TYPE_VOLUME;
                break;
            case Type_Displacement:
                currentShaderType = SLX_TYPE_DISPLACEMENT;
                break;
            case Type_Transformation:
                currentShaderType = SLX_TYPE_TRANSFORMATION;
                break;
            case Type_Imager:            
                currentShaderType = SLX_TYPE_IMAGER;
                break;
        }
        
        for(i = 0; i<varCount; i++)
        {
            AddShaderVar(pShader, i);
        }
       
        delete pShader;
    }
    else
    {
        result = RIE_NOFILE;
    }
    RiEnd();
    return result;
}


/*
 * Return total entry count of shaderSearchPathList
 */
static int GetSearchPathListCount()
{
    int listCharCount;
    int listCharIdx;
    int listElementCount;
    char * currentChar;
    
    listCharCount = 0;
    listCharIdx = 0;
    listElementCount = 0;
    currentChar = NULL;
    
    listCharCount = strlen(shaderSearchPathList);
    
    if (listCharCount > 0)
    {
        listElementCount = 1;
        currentChar = shaderSearchPathList;
        
        for (listCharIdx = 0; listCharIdx < listCharCount; listCharIdx++)
        {
            if (*currentChar == ':')	// list elements separated by colons
            {
                listElementCount++;
            }
            currentChar++;
        }
    }
    
    return listElementCount;
}


/*
 * Extract indexed entry from search path list
 */
static int GetSearchPathEntryAtIndex(int pathIdx)
{
    bool doLoop;
    bool entryFound;
    int listEntryIdx;
    char * currentChar;
    char * copyOutChar;
    int listCharCount;
    int listCharIdx;
    
    doLoop = true;
    entryFound = false;
    listEntryIdx = 0;
    listCharCount = 0;
    listCharIdx = 0;
    
    if (currentShaderSearchPath != NULL)
    {
        free(currentShaderSearchPath);
        currentShaderSearchPath = NULL;
    }
    
    currentShaderSearchPath = (char *)malloc(strlen(shaderSearchPathList));
    currentChar = shaderSearchPathList;
    copyOutChar = currentShaderSearchPath;
    *copyOutChar = 0x0;
    listCharCount = strlen(shaderSearchPathList);
    while (doLoop == true)
    {
        if (*currentChar == ':')	// path list entries separated by colons
        {
            listEntryIdx++;
            if (listEntryIdx > pathIdx)
                    doLoop = false;
        }
        else
        {
            if (pathIdx == listEntryIdx)
            {
                entryFound = true;
                *copyOutChar = *currentChar;
                copyOutChar++;
                *copyOutChar = 0x0;
            }
        }
        currentChar++;
        listCharIdx++;
        if (listCharIdx >= listCharCount)
                doLoop = false;
    }
    
    return entryFound;
}





/*
 * Attempt to open shader file using search path entries
 */
static bool FindShader (char *name)
{
    bool result;
    int stringLength;
    FILE * shaderInputFile;
    int pathListCount;
    int pathListIdx;
    bool doLoop;
    
    result = false;
    stringLength = 0;
    shaderInputFile = NULL;
    pathListCount = 0;
    pathListIdx = 0;
    doLoop = true;
    
    pathListCount = GetSearchPathListCount();
    if (pathListCount > 0)
    {
        if (GetSearchPathEntryAtIndex(pathListIdx) == false)
                doLoop = false;
    }
    else
    {
        doLoop = false;
    }
    while (doLoop == true)
    {
        // Build a full file path description
        stringLength = strlen(currentShaderSearchPath) + strlen(name) + 1;
        currentShaderFilePath = (char *)malloc(stringLength);
        strcpy(currentShaderFilePath, currentShaderSearchPath);
        strcat(currentShaderFilePath, name);
        
        // attempt to open the shader file
        shaderInputFile = OpenCurrentShader();
        if (shaderInputFile != NULL)
        {
            CloseCurrentShader(shaderInputFile);	// Success

            GetCurrentShaderInfo(currentShaderFilePath);
            
            result = true;
            doLoop = false;
        }
        
        if (result == false)
        {
            pathListIdx++;
            if (GetSearchPathEntryAtIndex(pathListIdx) == false)
                    doLoop = false;
        }
    }
    
    return result;
}




/*
 * Set colon-delimited search path used to locate compiled shaders.
 */
void SLX_SetPath (char *path)
{
    int pathLength;
    
    pathLength = 0;
    
    SlxLastError = RIE_NOERROR;    
    if (shaderSearchPathList != NULL)
    {
        free(shaderSearchPathList);
        shaderSearchPathList = NULL;
    }
    if (path != NULL)
    {
        pathLength = strlen(path);
        shaderSearchPathList = (char *)malloc(pathLength + 1);
        if (shaderSearchPathList != NULL)
        {
            strcpy(shaderSearchPathList, path);
        }
        else
        {
            SlxLastError = RIE_NOMEM;
        }
    }
}


/*
 * Return the type of the current shader, enumarated in SLX_TYPE.
 */
char *SLX_GetPath (void)
{
    SlxLastError = RIE_NOERROR;
    if (currentShaderSearchPath == NULL)
    {
        SlxLastError = RIE_NOFILE;
    }
    return currentShaderSearchPath;
}


/*
 * Attempt to locate and read the specified compiled shader, 
 * searching entries in currentShaderSearchPath, and set current shader info
 * Return 0 on sucess, -1 on error.
 */
int SLX_SetShader (char *name)
{
    int result;
    int stringLength;

    result = -1;
    stringLength = 0;

    SlxLastError = RIE_NOERROR;
    
    SLX_EndShader();	// deallocate currentShader and currentShaderFilePath storage
    
    if (name == NULL)
    {
        SlxLastError = RIE_NOFILE;
    }
    else
    {
        if (strcmp(name, "") == 0)
        {
            SlxLastError = RIE_NOFILE;
        }
    }
    
    if (SlxLastError == RIE_NOERROR)
    {
        if (FindShader(name) == false)
                SlxLastError = RIE_NOFILE;
    }

    if (SlxLastError == RIE_NOERROR)
    {
        stringLength = strlen(name) + 1;
        currentShader = (char *)malloc(stringLength);
        strcpy(currentShader, name);
        result = 0;
    }
    else
    {
        result = -1;
    }

    return result;
}


/*
 * Return pointer to a string containing the name of the current shader.
 */
char *SLX_GetName (void)
{
    SlxLastError = RIE_NOERROR;

    if (strlen(currentShader) <= 0)
    {
        SlxLastError = RIE_NOFILE;
    }

    return currentShader;
}


/*
 * Return type of the current shader, enumerated in SLX_TYPE
 */
SLX_TYPE SLX_GetType (void)
{
    SLX_TYPE slxType;
    FILE * shaderInputFile;
    
    slxType = SLX_TYPE_UNKNOWN;
    shaderInputFile = NULL;

    SlxLastError = RIE_NOERROR;
    
    if (strlen(currentShader) > 0)
    {
        slxType = currentShaderType;
    }
    else
    {
        SlxLastError = RIE_NOFILE;
    }
    
    return slxType;
}


/*
 * Return the number of arguments accepted by the current shader.
 */
int SLX_GetNArgs (void)
{
    SlxLastError = RIE_NOERROR;
    int result;
    
    result = 0;
    
    if (strlen(currentShader) > 0)
    {
        result = currentShaderNArgs;
    }
    else
    {
        SlxLastError = RIE_NOFILE;
    }
   
    return result;
}


/*
 * Return pointer to shader symbol definition for argument specified by symbol ID
 */
SLX_VISSYMDEF *SLX_GetArgById (int id)
{
    SLX_VISSYMDEF * result;
    SlxLastError = RIE_NOERROR;
    result = NULL;
    
    if (currentShaderArgArray != NULL)
    {
        if (id < currentShaderNArgs)
        {
            if (id >= 0)
            {
                result = GetShaderArgRec(currentShaderArgArray, id);
            }
        }
	}
    
    if (result == NULL)
    {
        SlxLastError = RIE_NOMEM;    
    }
    return result;
}


/*
 * Return pointer to shader symbol definition for argument specified by symbol name
 */
SLX_VISSYMDEF *SLX_GetArgByName (char *name)
{
    // Not yet implemented
    SlxLastError = RIE_NOERROR;
}


/*
 *  Return pointer to SLX_VISSYMDEF structure for a single element of an array
 */
SLX_VISSYMDEF *SLX_GetArrayArgElement(SLX_VISSYMDEF *array, int index)
{
    // Not yet implemented
    SlxLastError = RIE_NOERROR;
}


/*
 * Release storage used internally for current shader.
 */
void SLX_EndShader (void)
{
    SlxLastError = RIE_NOERROR;
    
    if (currentShader != NULL)
    {
        free(currentShader);
        currentShader = NULL;
    }

    if (currentShaderFilePath != NULL)
    {
        free(currentShaderFilePath);
        currentShaderFilePath = NULL;
    }

    if (currentShaderSearchPath != NULL)
    {
        free(currentShaderSearchPath);
        currentShaderSearchPath = NULL;
    }

    FreeArgRecStorage(currentShaderArgArray, currentShaderNArgs);

    if (currentShaderArgArray != NULL)
    {
        free(currentShaderArgArray);
        currentShaderArgArray = NULL;
    }

    currentShaderNArgs = 0;

    currentShaderType = SLX_TYPE_UNKNOWN;
}


/*
 * Return ASCII representation of SLX_TYPE
 */
char *SLX_TypetoStr (SLX_TYPE type)
{
    char * slxTypeStr;
    
    SlxLastError = RIE_NOERROR;
    slxTypeStr = SLX_TYPE_UNKNOWN_STR;
    
    switch (type)
    {
        case SLX_TYPE_UNKNOWN:
            slxTypeStr = SLX_TYPE_UNKNOWN_STR;
            break;
        case SLX_TYPE_POINT:
            slxTypeStr = SLX_TYPE_POINT_STR;
            break;
        case SLX_TYPE_COLOR:
            slxTypeStr = SLX_TYPE_COLOR_STR;
            break;
        case SLX_TYPE_SCALAR:
            slxTypeStr = SLX_TYPE_SCALAR_STR;
            break;
        case SLX_TYPE_STRING:
            slxTypeStr = SLX_TYPE_STRING_STR;
            break;
        case SLX_TYPE_SURFACE:
            slxTypeStr = SLX_TYPE_SURFACE_STR;
            break;
        case SLX_TYPE_LIGHT:
            slxTypeStr = SLX_TYPE_LIGHT_STR;
            break;
        case SLX_TYPE_DISPLACEMENT:
            slxTypeStr = SLX_TYPE_DISPLACEMENT_STR;
            break;
        case SLX_TYPE_VOLUME:
            slxTypeStr = SLX_TYPE_VOLUME_STR;
            break;
        case SLX_TYPE_TRANSFORMATION:
            slxTypeStr = SLX_TYPE_TRANSFORMATION_STR;
            break;
        case SLX_TYPE_IMAGER:
            slxTypeStr = SLX_TYPE_IMAGER_STR;
            break;
    }
    return slxTypeStr;
}


/*
 * Return ASCII representation of SLX_STORAGE
 */
char *SLX_StortoStr (SLX_STORAGE storage)
{
    char * slxStorageStr;
    
    SlxLastError = RIE_NOERROR;
    slxStorageStr = SLX_STOR_UNKNOWN_STR;

    switch (storage)
    {
        case SLX_STOR_UNKNOWN:
            slxStorageStr = SLX_STOR_UNKNOWN_STR;
            break;
        case SLX_STOR_CONSTANT:
            slxStorageStr = SLX_STOR_CONSTANT_STR;
            break;
        case SLX_STOR_VARIABLE:
            slxStorageStr = SLX_STOR_VARIABLE_STR;
            break;
        case SLX_STOR_TEMPORARY:
            slxStorageStr = SLX_STOR_TEMPORARY_STR;
            break;
        case SLX_STOR_PARAMETER:
            slxStorageStr = SLX_STOR_PARAMETER_STR;
            break;
        case SLX_STOR_GSTATE:
            slxStorageStr = SLX_STOR_GSTATE_STR;
            break;
    }
    return slxStorageStr;
}


/*
 * Return ASCII representation of SLX_DETAIL
 */
char *SLX_DetailtoStr (SLX_DETAIL detail)
{
    char * slxDetailStr;

    SlxLastError = RIE_NOERROR;
    slxDetailStr = SLX_DETAIL_UNKNOWN_STR;

    switch (detail)
    {
        case SLX_DETAIL_UNKNOWN:
            slxDetailStr = SLX_DETAIL_UNKNOWN_STR;
            break;
        case SLX_DETAIL_VARYING:
            slxDetailStr = SLX_DETAIL_VARYING_STR;
            break;
        case SLX_DETAIL_UNIFORM:
            slxDetailStr = SLX_DETAIL_UNIFORM_STR;
            break;
    }
    return slxDetailStr;
}

