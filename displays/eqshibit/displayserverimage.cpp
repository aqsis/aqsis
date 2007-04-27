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
		\brief Implements an image class getting it's data from the Dspy server.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/


#include	"aqsis.h"

#include	<fstream>
#include	<map>
#include	"signal.h"

#ifdef AQSIS_SYSTEM_WIN32

#include	<process.h>

#else // AQSIS_SYSTEM_WIN32

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>

static const int INVALID_SOCKET = -1;
static const int SD_BOTH = 2;
static const int SOCKET_ERROR = -1;

typedef sockaddr_in SOCKADDR_IN;
typedef sockaddr* PSOCKADDR;

#endif // !AQSIS_SYSTEM_WIN32

#include	"renderer.h"
#include	"render.h"
#include	"displaydriver.h"
#include	"displayserverimage.h"


START_NAMESPACE( Aqsis )


//---------------------------------------------------------------------
/** Close the socket this client is associated with.
 */
void CqDisplayServerImage::close()
{
#ifdef AQSIS_SYSTEM_WIN32
    int x = 1;
    setsockopt( m_socket, SOL_SOCKET, SO_DONTLINGER, reinterpret_cast<const char*>( &x ), sizeof( x ) );
    shutdown( m_socket, SD_BOTH );
    closesocket( m_socket );
#else // AQSIS_SYSTEM_WIN32
    shutdown( m_socket, SD_BOTH );
    ::close( m_socket );
#endif // !AQSIS_SYSTEM_WIN32

    m_socket = INVALID_SOCKET;
}



//---------------------------------------------------------------------
/** Send some data to the socket.
 * \param buffer Void pointer to the data to send.
 * \param len Integer length of the data in buffer.
 */

void CqDisplayServerImage::sendData( void* buffer, TqInt len )
{
    if ( m_socket == INVALID_SOCKET )
        return ;

    TqInt tot = 0, need = len;
    while ( need > 0 )
    {
        TqInt n = send( m_socket, reinterpret_cast<char*>( buffer ) + tot, need, 0 );
        need -= n;
        tot += n;
    }
}


//---------------------------------------------------------------------
/** Send a preconstructed message structure to this client.
 * \param pMsg Pointer to a SqDDMessageBase derive structure.
 */

void CqDisplayServerImage::sendMsg( SqDDMessageBase* pMsg )
{
    sendData( pMsg, pMsg->m_MessageLength );
}


//---------------------------------------------------------------------
/** Receive some data from the socket.
 * \param buffer Void pointer to the storage area for the data.
 * \param len Integer length of the data required.
 */

void CqDisplayServerImage::receive( void* buffer, TqInt len )
{
    TqInt tot = 0, need = len;
    while ( need > 0 )
    {
        TqInt n = recv( m_socket, reinterpret_cast<char*>( buffer ) + tot, need, 0 );
        need -= n;
        tot += n;
    }
}


//----------------------------------------------------------------------
/** CompositeAlpha() Composite with the alpha the end result RGB
*
*/

#define INT_MULT(a,b,t) ( (t) = (a) * (b) + 0x80, ( ( ( (t)>>8 ) + (t) )>>8 ) )
#define INT_PRELERP(p, q, a, t) ( (p) + (q) - INT_MULT( a, p, t) )

void CompositeAlpha(TqInt r, TqInt g, TqInt b, unsigned char &R, unsigned char &G, unsigned char &B, 
		    unsigned char alpha )
{ 
	TqInt t;
	// C’ = INT_PRELERP( A’, B’, b, t )
	TqInt R1 = static_cast<TqInt>(INT_PRELERP( R, r, alpha, t ));
	TqInt G1 = static_cast<TqInt>(INT_PRELERP( G, g, alpha, t ));
	TqInt B1 = static_cast<TqInt>(INT_PRELERP( B, b, alpha, t ));
	R = CLAMP( R1, 0, 255 );
	G = CLAMP( G1, 0, 255 );
	B = CLAMP( B1, 0, 255 );
}


void CqDisplayServerImage::acceptData(SqDDMessageData* dataMsg)
{
	TqUlong xmin__ = MAX((dataMsg->m_XMin - originX()), 0);
	TqUlong ymin__ = MAX((dataMsg->m_YMin - originY()), 0);
	TqUlong xmaxplus1__ = MIN((dataMsg->m_XMaxPlus1 - originX()), imageWidth());
	TqUlong ymaxplus1__ = MIN((dataMsg->m_YMaxPlus1 - originY()), imageWidth());
	TqUlong bucketlinelen = dataMsg->m_ElementSize * (dataMsg->m_XMaxPlus1 - dataMsg->m_XMin); 
	std::cout << "Render: " << xmin__ << ", " << ymin__ << " --> " << xmaxplus1__ << ", " << ymaxplus1__ << " [dlen: " << dataMsg->m_DataLength << "]" << std::endl;
	
	char* pdatarow = (char*)(dataMsg->m_Data);
	// Calculate where in the bucket we are starting from if the window is cropped.
	TqInt row = 0;
	if(originY() > static_cast<TqUlong>(dataMsg->m_YMin))
		row = originY() - dataMsg->m_YMin;
	TqInt col = 0;
	if(originX() > static_cast<TqUlong>(dataMsg->m_XMin))
		col = originX() - dataMsg->m_XMin;
	pdatarow += (row * bucketlinelen) + (col * dataMsg->m_ElementSize);

	if( data() && xmin__ >= 0 && ymin__ >= 0 && xmaxplus1__ <= imageWidth() && ymaxplus1__ <= imageHeight() )
	{
		TqUint comp = dataMsg->m_ElementSize/channels();
		TqUlong y;
		unsigned char *unrolled = data();

		for ( y = ymin__; y < ymaxplus1__; y++ )
		{
			TqUlong x;
			char* _pdatarow = pdatarow;
			for ( x = xmin__; x < xmaxplus1__; x++ )
			{
				TqInt so = channels() * (( y * imageWidth() ) +  x );
				switch (comp)
				{
					case 2 :
					{
						TqUshort *svalue = reinterpret_cast<TqUshort *>(_pdatarow);
						TqUchar alpha = 255;
						if (channels() == 4)
						{
							alpha = (svalue[3]/256);
						}
						CompositeAlpha((TqInt) svalue[0]/256, (TqInt) svalue[1]/256, (TqInt) svalue[2]/256, 
										unrolled[so + 0], unrolled[so + 1], unrolled[so + 2], 
										alpha);
						if (channels() == 4)
							unrolled[ so + 3 ] = alpha;
					}
					break;
					case 4:
					{

						TqUlong *lvalue = reinterpret_cast<TqUlong *>(_pdatarow);
						TqUchar alpha = 255;
						if (channels() == 4)
						{
							alpha = (TqUchar) (lvalue[3]/256);
						}
						CompositeAlpha((TqInt) lvalue[0]/256, (TqInt) lvalue[1]/256, (TqInt) lvalue[2]/256, 
										unrolled[so + 0], unrolled[so + 1], unrolled[so + 2], 
										alpha);
						if (channels() == 4)
							unrolled[ so + 3 ] = alpha;
					}
					break;

					case 1:
					default:
					{
						TqUchar *cvalue = reinterpret_cast<TqUchar *>(_pdatarow);
						TqUchar alpha = 255;
						if (channels() == 4)
						{
							alpha = (TqUchar) (cvalue[3]);
						}
						CompositeAlpha((TqInt) cvalue[0], (TqInt) cvalue[1], (TqInt) cvalue[2], 
										unrolled[so + 0], unrolled[so + 1], unrolled[so + 2], 
										alpha);
						if (channels() == 4)
							unrolled[ so + 3 ] = alpha;
					}
					break;
				}
				_pdatarow += dataMsg->m_ElementSize;

			}
			pdatarow += bucketlinelen;
		}
		if(m_updateCallback)
			m_updateCallback(xmin__, ymin__, xmaxplus1__-xmin__, ymaxplus1__-ymin__);
	}
}

END_NAMESPACE( Aqsis )
