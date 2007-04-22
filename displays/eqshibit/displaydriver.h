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
#include	<string.h>

START_NAMESPACE( Aqsis )


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

/** \enum EqDDMessageIDs
 *  IDs of the possible messages which can be sent to a display device from the sockets based display manager. 
 */
enum EqDDMessageIDs
{
    MessageID_String = 0,
    MessageID_FormatQuery,
    MessageID_Data,
    MessageID_Open,
    MessageID_Close,
    MessageID_Filename,
    MessageID_Nl,
    MessageID_NP,
    MessageID_DisplayType,
    MessageID_Abandon,
    MessageID_UserParam,


    MessageID_FormatResponse = 0x8001,
    MessageID_CloseAcknowledge = 0x8002,
};

//---------------------------------------------------------------------
/** \struct SqDDMessageBase
 * Base class from which all DD messages are derived.
 */

struct SqDDMessageBase
{
    SqDDMessageBase()
    {}
    SqDDMessageBase( TqInt ID, TqInt len ) :
            m_MessageID( ID ),
            m_MessageLength( len )
    {}
    TqInt	m_MessageID;
    TqInt	m_MessageLength;
	
	TqInt	Send(int sock)
	{
		TqInt tot = 0, need = m_MessageLength;
		while ( need > 0 )
		{
			TqInt n = send( sock, reinterpret_cast<char*>(this) + tot, need, 0 );
			need -= n;
			tot += n;
		}
		Aqsis::log() << info << "Sent: " << m_MessageID << " : " << m_MessageLength << " : " << tot << std::endl;
	}
	
};


//---------------------------------------------------------------------
/** \struct SqDDMessageFormatQuery
 * Format query message, asks the device what format (from a selection) it wants.
 */

struct SqDDMessageFormatQuery : public SqDDMessageBase
{
    // Specific message data
    TqInt	m_FormatCount;
    TqInt	m_Formats[ 1 ];

    static SqDDMessageFormatQuery*	Construct( TqInt formatcount, TqInt* formats );
    void	Destroy()
    {
        free( this );
    }
};


inline SqDDMessageFormatQuery* SqDDMessageFormatQuery::Construct( TqInt formatcount, TqInt* formats )
{
    TqInt len = sizeof(TqInt) * formatcount;
    SqDDMessageFormatQuery * pMessage = reinterpret_cast<SqDDMessageFormatQuery*>( malloc( sizeof( SqDDMessageFormatQuery ) - sizeof( TqInt ) + len ) );
    pMessage->m_MessageID = MessageID_FormatQuery;
    pMessage->m_FormatCount = formatcount;
    pMessage->m_MessageLength = sizeof( SqDDMessageFormatQuery ) - sizeof( TqInt ) + len;
    memcpy( &pMessage->m_Formats, formats, len );

    return ( pMessage );
}


//---------------------------------------------------------------------
/** \struct SqDDMessageFormatResponse
 * Message containing data format request, sent by client in response to a FormatQuery message.
 */

struct SqDDMessageFormatResponse : public SqDDMessageBase
{
    SqDDMessageFormatResponse()
    {}
    SqDDMessageFormatResponse( TqInt DF ) :
            SqDDMessageBase( MessageID_FormatResponse, sizeof( SqDDMessageFormatResponse ) ),
            m_DataFormat( DF )
    {}
    TqInt	m_DataFormat;	///< The format to send the data, from EqDDDataFormat
}
;


//---------------------------------------------------------------------
/** \struct SqDDMessageOpen
 * Message containing data about the image being rendered, used to open the conversation.
 */

struct SqDDMessageOpen : public SqDDMessageBase
{
    SqDDMessageOpen()
    {}
    SqDDMessageOpen( TqInt xres, TqInt yres, TqInt channels, TqInt cwxmin, TqInt cwxmax, TqInt cwymin, TqInt cwymax ) :
            SqDDMessageBase( MessageID_Open, sizeof( SqDDMessageOpen ) ),
            m_XRes( xres ),
            m_YRes( yres ),
            m_Channels( channels ),
            m_CropWindowXMin( cwxmin ),
            m_CropWindowXMax( cwxmax ),
            m_CropWindowYMin( cwymin ),
            m_CropWindowYMax( cwymax )
    {}
    TqInt	m_XRes;
    TqInt	m_YRes;
    TqInt	m_Channels;
    TqInt	m_NotUsed;
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
            SqDDMessageBase( MessageID_Close, sizeof( SqDDMessageClose ) )
    {}
}
;


//---------------------------------------------------------------------
/** \struct SqDDMessageAbandon
 * Message requesting the display device abandon its connection.
 */

struct SqDDMessageAbandon : public SqDDMessageBase
{
    SqDDMessageAbandon() :
            SqDDMessageBase( MessageID_Abandon, sizeof( SqDDMessageAbandon ) )
    {}
}
;


//---------------------------------------------------------------------
/** \struct SqDDMessageAcknowledge
 * Message containing data issued when an image is being closed after rendering is complete.
 */

struct SqDDMessageCloseAcknowledge : public SqDDMessageBase
{
    SqDDMessageCloseAcknowledge() :
            SqDDMessageBase( MessageID_CloseAcknowledge, sizeof( SqDDMessageCloseAcknowledge ) )
    {}
}
;


//---------------------------------------------------------------------
/** \struct SqDDMessageString
 * Base class from which all DD messages are derived.
 */

struct SqDDMessageString : public SqDDMessageBase
{
    // Specific message data
    TqInt	m_StringLength;
    TqChar	m_String[ 1 ];

    static SqDDMessageString*	Construct( const TqChar* string, TqInt ID = MessageID_String );
    void	Destroy()
    {
        free( this );
    }
};


inline SqDDMessageString* SqDDMessageString::Construct( const TqChar* string, TqInt ID )
{
    SqDDMessageString * pMessage = reinterpret_cast<SqDDMessageString*>( malloc( sizeof( SqDDMessageString ) + strlen( string ) ) );
    pMessage->m_MessageID = ID;
    pMessage->m_StringLength = strlen( string );
    pMessage->m_MessageLength = sizeof( SqDDMessageString ) + pMessage->m_StringLength;
    memcpy( pMessage->m_String, string, pMessage->m_StringLength + 1 );

    return ( pMessage );
}


//---------------------------------------------------------------------
/** \struct SqDDMessageFilename
 * Message containing image name information.
 */

struct SqDDMessageFilename : public SqDDMessageString
{
    static SqDDMessageFilename*	Construct( const TqChar* string );
};

inline SqDDMessageFilename* SqDDMessageFilename::Construct( const TqChar* name )
{
    return ( static_cast<SqDDMessageFilename*>( SqDDMessageString::Construct( name, MessageID_Filename ) ) );
}


//---------------------------------------------------------------------
/** \struct SqDDMessageDisplayType
 * Message containing display type name information.
 */

struct SqDDMessageDisplayType : public SqDDMessageString
{
    static SqDDMessageDisplayType*	Construct( const TqChar* string );
};

inline SqDDMessageDisplayType* SqDDMessageDisplayType::Construct( const TqChar* name )
{
    return ( static_cast<SqDDMessageDisplayType*>( SqDDMessageString::Construct( name, MessageID_DisplayType ) ) );
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
    TqLong	m_Data[ 1 ];

    static SqDDMessageData*	Construct( TqInt xmin, TqInt xmaxplus1, TqInt ymin, TqInt ymaxplus1, TqInt esz, const void* data, TqInt len );
    void	Destroy()
    {
        free( this );
    }
};


inline SqDDMessageData* SqDDMessageData::Construct( TqInt xmin, TqInt xmaxplus1, TqInt ymin, TqInt ymaxplus1, TqInt esz, const void* data, TqInt len )
{
    SqDDMessageData * pMessage = reinterpret_cast<SqDDMessageData*>( malloc( sizeof( SqDDMessageData ) - sizeof( TqLong ) + len ) );
    pMessage->m_MessageID = MessageID_Data;
    pMessage->m_XMin = xmin;
    pMessage->m_XMaxPlus1 = xmaxplus1;
    pMessage->m_YMin = ymin;
    pMessage->m_YMaxPlus1 = ymaxplus1;
    pMessage->m_ElementSize = esz;
    pMessage->m_DataLength = len;
    pMessage->m_MessageLength = sizeof( SqDDMessageData ) - sizeof( TqLong ) + len;
    memcpy( &pMessage->m_Data, data, len );

    return ( pMessage );
}


//---------------------------------------------------------------------
/** \struct SqDDMessageMatrix
 * Message containing a matrix.
 */

struct SqDDMessageMatrix : public SqDDMessageBase
{
    SqDDMessageMatrix( TqInt ID, TqFloat* mat ) :
            SqDDMessageBase( ID, sizeof( SqDDMessageMatrix ) )
    {
        memcpy( m_Matrix, mat, sizeof( m_Matrix ) );
    }

    TqFloat	m_Matrix[ 4 ][ 4 ];
};


//---------------------------------------------------------------------
/** \struct SqDDMessageNl
 * Message containing the world to camera matrix.
 */

struct SqDDMessageNl : public SqDDMessageMatrix
{
    SqDDMessageNl( TqFloat* mat ) :
            SqDDMessageMatrix( MessageID_Nl, mat )
    {}
}
;


//---------------------------------------------------------------------
/** \struct SqDDMessageNp
 * Message containing the world to camera matrix.
 */

struct SqDDMessageNP : public SqDDMessageMatrix
{
    SqDDMessageNP( TqFloat* mat ) :
            SqDDMessageMatrix( MessageID_NP, mat )
    {}
}
;


//---------------------------------------------------------------------
/** \struct SqDDMessageUserParam
 * Message delivering a user parameter.
 */

struct SqDDMessageUserParam : public SqDDMessageBase
{
    // Specific message data
    TqInt	m_DataType;
    TqInt	m_NameLength;
    TqInt	m_DataLength;
    TqInt	m_DataCount;
    TqChar	m_NameAndData[ 1 ];

    static SqDDMessageUserParam*	Construct( const TqChar* name, TqInt type, TqInt count, const void* data, TqInt dataLength, TqInt ID = MessageID_UserParam );
    void	Destroy()
    {
        free( this );
    }
};


inline SqDDMessageUserParam* SqDDMessageUserParam::Construct( const TqChar* name, TqInt type, TqInt count, const void* data, TqInt dataLength, TqInt ID )
{
    SqDDMessageUserParam * pMessage = reinterpret_cast<SqDDMessageUserParam*>( malloc( sizeof( SqDDMessageUserParam ) + strlen( name ) + dataLength ) );
    pMessage->m_MessageID = ID;
    pMessage->m_NameLength = strlen( name );
    pMessage->m_DataLength = dataLength;
    pMessage->m_DataType = type;
    pMessage->m_DataCount = count;
    pMessage->m_MessageLength = sizeof( SqDDMessageUserParam ) + pMessage->m_NameLength + pMessage->m_DataLength;
    memcpy( pMessage->m_NameAndData, name, pMessage->m_NameLength + 1 );
    memcpy( pMessage->m_NameAndData + pMessage->m_NameLength + 1, static_cast<const char*>(data), pMessage->m_DataLength );

    return ( pMessage );
}


//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif // DISPLAYDRIVER_H_INCLUDED

