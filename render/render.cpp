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
		\brief Implements the structures and functions for system specific renderer initialisation.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<windows.h>
#include	<io.h>
#include	<fcntl.h>

#include	"aqsis.h"
#include	"sstring.h"
#include	"render.h"

using namespace Aqsis;

void InitialiseINIData();

BOOL APIENTRY DllMain( HINSTANCE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			InitialiseINIData();
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
    }
   	return TRUE;
}


void InitialiseINIData()
{
	// Read in and store information from the ini file.
	char strExe[255];
	char strDrive[10];
	char strPath[255];
	char strINIFile[255];
	GetModuleFileName(NULL, strExe, 255);
	_splitpath(strExe,strDrive,strPath,NULL,NULL);
	_makepath(strINIFile,strDrive,strPath,"aqsis",".ini");

	char* pTable;

	// Open the ini file, if one exists in the current directory, use it else, look in the executable directory.
	int fh=_open(".\\aqsis.ini",_O_RDONLY);
	TqInt maxLen=0;
	if(fh>=0)
	{
		strcpy(strINIFile,".\\aqsis.ini");
		maxLen=_filelength(fh);
		_close(fh);
	}
	else
	{
		fh=_open(strINIFile,_O_RDONLY);
		if(fh>=0)
		{
			maxLen=_filelength(fh);
			_close(fh);
		}
	}

	// Now read the data in and store it in global data for later access.
	if(maxLen>0)
	{
		// Truncate the display map to 0 length.
		gaDisplayMap.clear();

		pTable=new char[maxLen];

		// Read in the  entire value table for the [DISPLAY_DRIVERS] section.
		TqInt totLen=GetPrivateProfileString("DISPLAY_DRIVERS", NULL, "", pTable, maxLen, strINIFile);
		char* pEnt=pTable;
		while(totLen>1)
		{
			TqInt entLen=strlen(pEnt);

			// Get the value for the next key
			char strValue[255];
			GetPrivateProfileString("DISPLAY_DRIVERS", pEnt, "framebuffer.dll", strValue, 255, strINIFile);
			gaDisplayMap.push_back(SqDisplayMapEntry(pEnt, strValue));

			pEnt+=entLen+1;
			totLen-=entLen+1;
		}
		delete[](pTable);
	}
}



START_NAMESPACE(Aqsis)

std::vector<SqDisplayMapEntry>	gaDisplayMap;

//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)
