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
		\brief Display driver message structures.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is context.h included already?
#ifndef DISPLAYDRIVER_H_INCLUDED
#define DISPLAYDRIVER_H_INCLUDED 1

#include	"aqsis.h"

#include	<stdlib.h>

START_NAMESPACE(Aqsis)


enum EqDataFormat
{
	DataFormat_Float32,
	DataFormat_Unsigned32,
	DataFormat_Signed32,
	DataFormat_Unsigned16, 
	DataFormat_Signed16, 
	DataFormat_Unsigned8, 
	DataFormat_Signed8,
};

enum EqDDMessageIDs
{
	MessageID_String=0,
	MessageID_FormatQuery,
	MessageID_Data,
	MessageID_Open,
	MessageID_Close,
	MessageID_Filename,
	MessageID_Nl,
	MessageID_NP,
	
	
	MessageID_FormatResponse=0x8001,
};

//---------------------------------------------------------------------
/** \struct SqDDMessageBase 
 * Base class from which all DD messages are derived.
 */

struct SqDDMessageBase
{
			SqDDMessageBase()	{}
			SqDDMessageBase(TqInt ID, TqInt len) : 
												m_MessageID(ID), 
												m_MessageLength(len)	
												{}
	TqInt	m_MessageID;
	TqInt	m_MessageLength;
};


//---------------------------------------------------------------------
/** \struct SqDDMessageFormatResponse
 * Message containing data format request, sent by client in response to a FormatQuery message.
 */

struct SqDDMessageFormatResponse : public SqDDMessageBase
{
			SqDDMessageFormatResponse()	{}
			SqDDMessageFormatResponse(TqInt DF) : 
												SqDDMessageBase(MessageID_FormatResponse, sizeof(SqDDMessageFormatResponse)), 
												m_DataFormat(DF)
												{}
	TqInt	m_DataFormat;	///< The format to send the data, from EqDDDataFormat
};


//---------------------------------------------------------------------
/** \struct SqDDMessageOpen
 * Message containing data about the image being rendered, used to open the conversation.
 */

struct SqDDMessageOpen : public SqDDMessageBase
{
			SqDDMessageOpen()	{}
			SqDDMessageOpen(TqInt xres, TqInt yres, TqInt samples, TqInt cwxmin, TqInt cwxmax, TqInt cwymin, TqInt cwymax) : 
												SqDDMessageBase(MessageID_Open, sizeof(SqDDMessageOpen)), 
												m_XRes(xres),
												m_YRes(yres),
												m_SamplesPerElement(samples),
												m_CropWindowXMin(cwxmin),
												m_CropWindowXMax(cwxmax),
												m_CropWindowYMin(cwymin),
												m_CropWindowYMax(cwymax)
												{}
	TqInt	m_XRes;
	TqInt	m_YRes;
	TqInt	m_SamplesPerElement;
	TqInt	m_CropWindowXMin;
	TqInt	m_CropWindowXMax;
	TqInt	m_CropWindowYMin;
	TqInt	m_CropWindowYMax;
};


//---------------------------------------------------------------------
/** \struct SqDDMessageClose
 * Message containing data issued when an image is being closed after rendering is complete.
 */

struct SqDDMessageClose : public SqDDMessageBase
{
			SqDDMessageClose() : 
												SqDDMessageBase(MessageID_Close, sizeof(SqDDMessageClose))
												{}
};


//---------------------------------------------------------------------
/** \struct SqDDMessageString
 * Base class from which all DD messages are derived.
 */

struct SqDDMessageString : public SqDDMessageBase
{
	// Specific message data
	TqInt	m_StringLength;
	TqChar	m_String[1];

	static SqDDMessageString*	Construct(const TqChar* string, TqInt ID=MessageID_String);
	void	Destroy()	{free(this);}
};


inline SqDDMessageString* SqDDMessageString::Construct(const TqChar* string, TqInt ID)
{
	SqDDMessageString* pMessage=reinterpret_cast<SqDDMessageString*>(malloc(sizeof(SqDDMessageString)+strlen(string)));
	pMessage->m_MessageID=ID;
	pMessage->m_StringLength=strlen(string);
	pMessage->m_MessageLength=sizeof(SqDDMessageString)+pMessage->m_StringLength;
	memcpy(pMessage->m_String,string,pMessage->m_StringLength+1);

	return(pMessage);
}


//---------------------------------------------------------------------
/** \struct SqDDMessageFilename
 * Message containing image name information.
 */

struct SqDDMessageFilename : public SqDDMessageString
{
	static SqDDMessageFilename*	Construct(const TqChar* string);
};

inline SqDDMessageFilename* SqDDMessageFilename::Construct(const TqChar* name)
{
	return(static_cast<SqDDMessageFilename*>(SqDDMessageString::Construct(name, MessageID_Filename)));
}


//---------------------------------------------------------------------
/** \struct SqDDMessageData
 * Data message, contains bucket data for storage.
 */

struct SqDDMessageData : public SqDDMessageBase
{
	// Specific message data
	TqInt	m_XMin;
	TqInt	m_XMaxPlus1;
	TqInt	m_YMin;
	TqInt	m_YMaxPlus1;
	TqInt	m_ElementSize;
	
	TqInt	m_DataLength;
	TqLong	m_Data[1];

	static SqDDMessageData*	Construct(TqInt xmin, TqInt xmaxplus1, TqInt ymin, TqInt ymaxplus1, TqInt esz, const void* data, TqInt len);
	void	Destroy()	{free(this);}
};


inline SqDDMessageData* SqDDMessageData::Construct(TqInt xmin, TqInt xmaxplus1, TqInt ymin, TqInt ymaxplus1, TqInt esz, const void* data, TqInt len)
{
	SqDDMessageData* pMessage=reinterpret_cast<SqDDMessageData*>(malloc(sizeof(SqDDMessageData)-sizeof(TqLong)+len));
	pMessage->m_MessageID=MessageID_Data;
	pMessage->m_XMin=xmin;
	pMessage->m_XMaxPlus1=xmaxplus1;
	pMessage->m_YMin=ymin;
	pMessage->m_YMaxPlus1=ymaxplus1;
	pMessage->m_ElementSize=esz;
	pMessage->m_DataLength=len;
	pMessage->m_MessageLength=sizeof(SqDDMessageData)-sizeof(TqLong)+len;
	memcpy(&pMessage->m_Data,data,len);

	return(pMessage);
}


//---------------------------------------------------------------------
/** \struct SqDDMessageMatrix
 * Message containing a matrix.
 */

struct SqDDMessageMatrix : public SqDDMessageBase
{
			SqDDMessageMatrix(TqInt ID, TqFloat* mat) : 
												SqDDMessageBase(ID, sizeof(SqDDMessageMatrix))
												{
													memcpy(m_Matrix,mat,sizeof(m_Matrix));
												}

			TqFloat		m_Matrix[4][4];
};


//---------------------------------------------------------------------
/** \struct SqDDMessageNl
 * Message containing the world to camera matrix.
 */

struct SqDDMessageNl : public SqDDMessageMatrix
{
			SqDDMessageNl(TqFloat* mat) : 
												SqDDMessageMatrix(MessageID_Nl, mat)
												{}
};


//---------------------------------------------------------------------
/** \struct SqDDMessageNp
 * Message containing the world to camera matrix.
 */

struct SqDDMessageNP : public SqDDMessageMatrix
{
			SqDDMessageNP(TqFloat* mat) : 
												SqDDMessageMatrix(MessageID_NP, mat)
												{}
};


//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif // DISPLAYDRIVER_H_INCLUDED