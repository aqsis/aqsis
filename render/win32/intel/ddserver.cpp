#include	"aqsis.h"

#include	<process.h>

#include	"renderer.h"
#include	"displaydriver.h"
#include	"ddserver.h"
#include	"imagebuffer.h"


START_NAMESPACE(Aqsis)

static void AcceptConnections(void* pvServer);


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

END_NAMESPACE(Aqsis)
