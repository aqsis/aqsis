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
		\brief Implements a display driver for saving shadow maps.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<iostream>
#include	<fstream>

#include	"aqsis.h"

#ifdef AQSIS_SYSTEM_WIN32 

#include	<process.h>

#else

typedef int SOCKET;

#endif

#include	"displaydriver.h"
#include	"dd.h"
#include	"tiffio.h"
#include	"sstring.h"

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
#include	"version.h"
#endif

using namespace Aqsis;

SqDDMessageFormatResponse frmt(2);
SqDDMessageCloseAcknowledge closeack;


int main(int argc, char* argv[])
{
	if(DDInitialise(NULL,-1)==0)
	{
		DDProcessMessages();
	}
	return 0;
}

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
#define	ZFILE_HEADER		"Aqsis ZFile" VERSION_STR
#else // AQSIS_SYSTEM_WIN32
#define ZFILE_HEADER		"Aqsis ZFile" VERSION
#endif // !AQSIS_SYSTEM_WIN32


TqInt	XRes,YRes;
TqInt	SamplesPerElement;
TqFloat* pData;
TIFF*	pOut;
TqInt	CWXMin,CWYMin;
std::string	strFilename("output.tif");
TqFloat	matWorldToCamera[4][4];
TqFloat	matWorldToScreen[4][4];

TqInt Query(SOCKET s,SqDDMessageBase* pMsgB)
{
	switch(pMsgB->m_MessageID)
	{
		case MessageID_FormatQuery:
		{
			if(DDSendMsg(s,&frmt)<=0)
				return(-1);
		}
		break;
	}
	return(0);
}

TqInt Open(SOCKET s,SqDDMessageBase* pMsgB)
{
	SqDDMessageOpen* pMsg=static_cast<SqDDMessageOpen*>(pMsgB);

	XRes=(pMsg->m_CropWindowXMax-pMsg->m_CropWindowXMin);
	YRes=(pMsg->m_CropWindowYMax-pMsg->m_CropWindowYMin);
	CWXMin=pMsg->m_CropWindowXMin;
	CWYMin=pMsg->m_CropWindowYMin;
	SamplesPerElement=pMsg->m_SamplesPerElement;

	if(SamplesPerElement>1)	return(-1);

	// Create a buffer big enough to hold a row of buckets.
	pData=new TqFloat[XRes*YRes];
	return(0);
}


TqInt Data(SOCKET s,SqDDMessageBase* pMsgB)
{
	SqDDMessageData* pMsg=static_cast<SqDDMessageData*>(pMsgB);

	TqInt	linelen=XRes*SamplesPerElement;
	char* pBucket=reinterpret_cast<char*>(&pMsg->m_Data);
	
	TqInt y;
	for(y=pMsg->m_YMin; y<pMsg->m_YMaxPlus1; y++)
	{
		TqInt x;
		for(x=pMsg->m_XMin; x<pMsg->m_XMaxPlus1; x++)
		{
			if(x>=0 && y>=0 && x<XRes && y<YRes)
			{
				TqInt so=(y*linelen)+(x*SamplesPerElement);
				
				TqInt i=0;
				while(i<SamplesPerElement)
				{
					pData[so++]=reinterpret_cast<TqFloat*>(pBucket)[i];
					i++;
				}
			}
			pBucket+=pMsg->m_ElementSize;
		}
	}
	return(0);
}


TqInt Close(SOCKET s,SqDDMessageBase* pMsgB)
{
	// Save the shadowmap to a binary file.
	if(strFilename!="")
	{
                std::ofstream ofile(strFilename.c_str(), std::ios::out | std::ios::binary);
		if(ofile.is_open())
		{
			// Save a file type and version marker
			ofile << ZFILE_HEADER;

			// Save the xres and yres.
			ofile.write(reinterpret_cast<char* >(&XRes), sizeof(XRes));
			ofile.write(reinterpret_cast<char* >(&YRes), sizeof(XRes));

			// Save the transformation matrices.
			ofile.write(reinterpret_cast<char*>(matWorldToCamera[0]),sizeof(matWorldToCamera[0][0])*4);
			ofile.write(reinterpret_cast<char*>(matWorldToCamera[1]),sizeof(matWorldToCamera[0][0])*4);
			ofile.write(reinterpret_cast<char*>(matWorldToCamera[2]),sizeof(matWorldToCamera[0][0])*4);
			ofile.write(reinterpret_cast<char*>(matWorldToCamera[3]),sizeof(matWorldToCamera[0][0])*4);

			ofile.write(reinterpret_cast<char*>(matWorldToScreen[0]),sizeof(matWorldToScreen[0][0])*4);
			ofile.write(reinterpret_cast<char*>(matWorldToScreen[1]),sizeof(matWorldToScreen[0][0])*4);
			ofile.write(reinterpret_cast<char*>(matWorldToScreen[2]),sizeof(matWorldToScreen[0][0])*4);
			ofile.write(reinterpret_cast<char*>(matWorldToScreen[3]),sizeof(matWorldToScreen[0][0])*4);

			// Now output the depth values
			ofile.write(reinterpret_cast<char*>(pData),sizeof(TqFloat)*(XRes*YRes));
			ofile.close();
		}	
	}
	if(DDSendMsg(s,&closeack)<=0)
		return(-1);
	else
		return(1);
}


TqInt HandleMessage(SOCKET s,SqDDMessageBase* pMsgB)
{
	switch(pMsgB->m_MessageID)
	{
		case MessageID_Filename:
		{
			SqDDMessageFilename* pMsg=static_cast<SqDDMessageFilename*>(pMsgB);
			strFilename=pMsg->m_String;
		}
		break;

		case MessageID_Nl:
		{
			SqDDMessageNl* pMsg=static_cast<SqDDMessageNl*>(pMsgB);
			memcpy(matWorldToCamera,pMsg->m_Matrix,sizeof(matWorldToCamera));
		}
		break;

		case MessageID_NP:
		{
			SqDDMessageNP* pMsg=static_cast<SqDDMessageNP*>(pMsgB);
			memcpy(matWorldToScreen,pMsg->m_Matrix,sizeof(matWorldToScreen));
		}
		break;
	}
	return(0);
}
