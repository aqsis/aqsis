// Aqsis
// Copyright � 1997 - 2001, Paul C. Gregory
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
		\brief Sockets based display device manager.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/


#include	"aqsis.h"

#include	<process.h>

#include	"renderer.h"
#include	"render.h"
#include	"displaydriver.h"
#include	"ddmsock.h"
#include	"imagebuffer.h"
#include	"file.h"


START_NAMESPACE(Aqsis)

//---------------------------------------------------------------------
/** Constructor, takes a port no. and prepares the socket to accept clients.
 */

CqDDServer::CqDDServer(TqInt port)
{
	Prepare(port);
}


//---------------------------------------------------------------------
/** Prepare the socket to accept client connections.
 * \param port Integer port number to use.
 */

TqBool	CqDDServer::Prepare(TqInt port)
{
	if(Open())
		if(Bind(port))
			if(Listen())
				return(TqTrue);
	return(TqFalse);
}


//---------------------------------------------------------------------
/** Create the socket.
 */

TqBool CqDDServer::Open()
{
	m_Socket=socket(AF_INET,SOCK_STREAM,0);

	if(m_Socket==INVALID_SOCKET)
	{
		TqInt err=WSAGetLastError();
		CqBasicError(0,0,"Error opening DD server socket");
		return(TqFalse);
	}
	return(TqTrue);
}


//---------------------------------------------------------------------
/** Bind the socket to a specified port.
 */

TqBool CqDDServer::Bind(TqInt port)
{
	SOCKADDR_IN saTemp;
	saTemp.sin_family=AF_INET;
	saTemp.sin_port=htons(port);
	saTemp.sin_addr.s_addr=INADDR_ANY;

	if(bind(m_Socket,(PSOCKADDR)&saTemp,sizeof(saTemp))==SOCKET_ERROR)
	{
		CqBasicError(0,0,"Error binding to DD socket");
		Close();
		return(TqFalse);
	}
	return(TqTrue);
}


//---------------------------------------------------------------------
/** Prepare the socket to listen for client connections.
 */

TqBool CqDDServer::Listen()
{
	if(listen(m_Socket,SOMAXCONN)==SOCKET_ERROR)
	{
		CqBasicError(0,0,"Error listening to DD socket");
		Close();
		return(TqFalse);
	}
	return(TqTrue);
}


//---------------------------------------------------------------------
/** Set ip the thread to wait for client connection requests.
 */

TqBool CqDDServer::Accept(CqDDClient& dd)
{
	SOCKET c;

	if((c=accept(Socket(),NULL,NULL))!=INVALID_SOCKET)
	{
		dd.SetSocket(c);
		// Issue a format request so that we know what data to send to the client.
		SqDDMessageBase msg;
		SqDDMessageFormatResponse frmt;

		msg.m_MessageID=MessageID_FormatQuery;
		msg.m_MessageLength=sizeof(SqDDMessageBase);
		dd.SendMsg(&msg);
		dd.Receive(&frmt,sizeof(frmt));

		// Confirm the message returned is as expected.
		if(frmt.m_MessageID==MessageID_FormatResponse &&
		   frmt.m_MessageLength==sizeof(frmt))
			return(TqTrue);
		else
			dd.Close();
	}
	return(TqFalse);
}


//---------------------------------------------------------------------
/** Destructor, close all connected client sockets.
 */

CqDDServer::~CqDDServer()
{
	Close();
}


//---------------------------------------------------------------------
/** Send some data to the socket.
 * \param buffer Void pointer to the data to send.
 * \param len Integer length of the data in buffer.
 */

void CqDDClient::SendData(void* buffer, TqInt len)
{
	TqInt tot=0,need=len;
	while(need>0)
	{
		TqInt n=send(m_Socket,reinterpret_cast<char*>(buffer)+tot,need,0);
		need-=n;
		tot+=n;
	}
}


//---------------------------------------------------------------------
/** Send a preconstructed message structure to this client.
 * \param pMsg Pointer to a SqDDMessageBase derive structure.
 */

void CqDDClient::SendMsg(SqDDMessageBase* pMsg)
{
	SendData(pMsg,pMsg->m_MessageLength);
}


//---------------------------------------------------------------------
/** Receive some data from the socket.
 * \param buffer Void pointer to the storage area for the data.
 * \param len Integer length of the data required.
 */

void CqDDClient::Receive(void* buffer, TqInt len)
{
	TqInt tot=0,need=len;
	while(need>0)
	{
		TqInt n=recv(m_Socket,reinterpret_cast<char*>(buffer)+tot,need,0);
		need-=n;
		tot+=n;
	}
}



TqInt CqDDManager::Initialise()
{
	if(m_DDServer.Prepare(AQSIS_DD_PORT))
		return(0);
	else
		return(-1);
}

TqInt CqDDManager::Shutdown()
{
	std::vector<CqDDClient>::iterator i;
	for(i=m_aDisplayRequests.begin(); i!=m_aDisplayRequests.end(); i++)
		i->Close();
	m_DDServer.Close();
	return(0);
}


TqInt CqDDManager::AddDisplay(const TqChar* name, const TqChar* type, const TqChar* mode)
{
	m_aDisplayRequests.push_back(CqDDClient(name, type, mode));
	return(0);
}

TqInt CqDDManager::ClearDisplays()
{
	m_aDisplayRequests.clear();
	return(0);
}

TqInt CqDDManager::OpenDisplays()
{
	std::vector<CqDDClient>::iterator i;
	for(i=m_aDisplayRequests.begin(); i!=m_aDisplayRequests.end(); i++)
		LoadDisplayLibrary(*i);
	return(0);
}

TqInt CqDDManager::CloseDisplays()
{
	SqDDMessageClose msg;

	std::vector<CqDDClient>::iterator i;
	for(i=m_aDisplayRequests.begin(); i!=m_aDisplayRequests.end(); i++)
		i->SendMsg(&msg);
	return(0);
}

TqInt CqDDManager::DisplayBucket(IqBucket* pBucket)
{
	// Copy the bucket to the display buffer.
	TqInt		xmin=pBucket->XOrigin();
	TqInt		ymin=pBucket->YOrigin();
	TqInt		xsize=pBucket->XSize();
	TqInt		ysize=pBucket->YSize();
	TqInt		xmaxplus1=xmin+xsize;
	TqInt		ymaxplus1=ymin+ysize;

	for(std::vector<CqDDClient>::iterator i=m_aDisplayRequests.begin(); i!=m_aDisplayRequests.end(); i++)
	{
		RtInt mode=0;
		if(strstr(i->strMode().c_str(), RI_RGB)!=NULL)
			mode|=ModeRGB;
		if(strstr(i->strMode().c_str(), RI_A)!=NULL)
			mode|=ModeA;
		if(strstr(i->strMode().c_str(), RI_Z)!=NULL)
			mode|=ModeZ;

		TqInt		samples=mode&ModeRGB?3:0;
					samples+=mode&ModeA?1:0;
					samples=mode&ModeZ?1:samples;
		TqInt		elementsize=samples*sizeof(TqFloat);
		TqInt		datalen=xsize*ysize*elementsize;

		TqFloat*	pData=new TqFloat[xsize*ysize*samples];

		TqInt		linelen=xsize*samples;

		SqImageValue val;
		TqInt y;
		for(y=0; y<ysize; y++)
		{
			TqInt sy=y+ymin;
			TqInt x;
			for(x=0; x<xsize; x++)
			{
				TqInt sx=x+xmin;
				TqInt so=(y*linelen)+(x*samples);
				// If outputting a zfile, use the midpoint method.
				/// \todo Should really be generalising this section to use specif Filter/Expose/Quantize functions.
				if(mode&ModeZ)
				{
					pData[so]=pBucket->Depth(sx,sy);
				}
				else
				{
					if(samples>=3)
					{
						CqColor col=pBucket->Color(sx,sy);
						pData[so+0]=col.fRed();
						pData[so+1]=col.fGreen();
						pData[so+2]=col.fBlue();
						if(samples==4)
							pData[so+3]=pBucket->Coverage(sx,sy);
					}
					else if(samples==1)
						pData[so+0]=pBucket->Coverage(sx,sy);
				}
			}
		}
		SqDDMessageData* pmsg=SqDDMessageData::Construct(xmin,xmaxplus1,ymin,ymaxplus1,elementsize,pData,datalen);
		delete[](pData);
		i->SendMsg(pmsg);
		pmsg->Destroy();
	}
	return(0);
}


void CqDDManager::LoadDisplayLibrary(CqDDClient& dd)
{
	// Load the requested display library according to the specified mode in the RiDisplay command.
	CqString strDriverFile("framebuffer.exe");

	// Find the display driver in the map loaded from the ini file.
	TqBool	bFound=TqFalse;
	TqInt i;
	for(i=0; i<gaDisplayMap.size(); i++)
	{
		if(dd.strType().compare(gaDisplayMap[i].m_strName)==0)
		{
			strDriverFile=gaDisplayMap[i].m_strLocation;
			bFound=TqTrue;
			break;
		}
	}

	if(!bFound)
	{
		CqString strErr("Cannot find display driver ");
		strErr+=dd.strType().c_str();
		strErr+=" : defaulting to framebuffer";
		//strErr.Format("Cannot find display driver %s : defaulting to framebuffer", QGetRenderContext()->optCurrent().strDisplayType().String());
		CqBasicError(ErrorID_DisplayDriver,Severity_Normal,strErr.c_str());
	}

	CqFile fileDriver(strDriverFile.c_str(), "display");
	if(fileDriver.IsValid())
	{
		TqInt ProcHandle=_spawnl(_P_NOWAITO, fileDriver.strRealName().c_str(), strDriverFile.c_str() ,NULL);
		if(ProcHandle>=0)
		{
			// wait for a connection request from the client
			if(m_DDServer.Accept(dd))
			{
				// Send a filename message
				SqDDMessageFilename* pmsgfname=SqDDMessageFilename::Construct(dd.strName().c_str());
				dd.SendMsg(pmsgfname);
				pmsgfname->Destroy();

				CqMatrix& matWorldToCamera=QGetRenderContext()->matSpaceToSpace("world","camera");
				CqMatrix& matWorldToScreen=QGetRenderContext()->matSpaceToSpace("world","raster");

				SqDDMessageNl msgnl(matWorldToCamera.pElements());
				dd.SendMsg(&msgnl);

				SqDDMessageNP msgnp(matWorldToScreen.pElements());
				dd.SendMsg(&msgnp);
				
				// Send the open message..
				RtInt mode=0;
				if(strstr(dd.strMode().c_str(), RI_RGB)!=NULL)
					mode|=ModeRGB;
				if(strstr(dd.strMode().c_str(), RI_A)!=NULL)
					mode|=ModeA;
				if(strstr(dd.strMode().c_str(), RI_Z)!=NULL)
					mode|=ModeZ;
				TqInt SamplesPerElement=mode&ModeRGB?3:0;
					  SamplesPerElement+=mode&ModeA?1:0;
					  SamplesPerElement=mode&ModeZ?1:SamplesPerElement;
				SqDDMessageOpen msgopen(QGetRenderContext()->pImage()->iXRes(),
										QGetRenderContext()->pImage()->iYRes(),
										SamplesPerElement,
										QGetRenderContext()->pImage()->CropWindowXMin(),
										QGetRenderContext()->pImage()->CropWindowXMax(),
										QGetRenderContext()->pImage()->CropWindowYMin(),
										QGetRenderContext()->pImage()->CropWindowYMax());
				dd.SendMsg(&msgopen);
			}
		}
		else
			CqBasicError(0,0,"Error loading display driver");
	}
}


END_NAMESPACE(Aqsis)
