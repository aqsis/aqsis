
#include	<process.h>

#include	"renderer.h"
#include	"displaydriver.h"
#include	"ddserver.h"


START_NAMESPACE(Aqsis)

static void AcceptConnections(void* pvServer);


//---------------------------------------------------------------------
/** Constructor, takes a port no. and prepares the socket to accept clients.
 */

CqDDServer::CqDDServer(TqInt port) : m_bHasQuit(TqFalse)
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
			{
				Accept();
				return(TqTrue);
			}
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

void CqDDServer::Accept()
{
	// Start the accept thread
	m_AcceptThreadID=_beginthread(&AcceptConnections,0,this);
}


//---------------------------------------------------------------------
/** Destructor, close all connected client sockets.
 */

CqDDServer::~CqDDServer()
{
	for(std::vector<CqDDClient>::iterator i=m_aClients.begin(); i!=m_aClients.end(); i++)
		i->Close();

	Close();
}


//---------------------------------------------------------------------
/** Send some data to all listening clients.
 * \param buffer Void pointer to the data to send.
 * \param len Integer length of the data in buffer.
 */

void CqDDServer::SendData(void* buffer, TqInt len)
{
	std::vector<CqDDClient>::iterator i;
	for(i=m_aClients.begin(); i!=m_aClients.end(); i++)
		i->SendData(buffer,len);
}


//---------------------------------------------------------------------
/** Send a preconstructed message structure to all clients.
 * \param pMsg Pointer to a SqDDMessageBase derive structure.
 */

void CqDDServer::SendMessage(SqDDMessageBase* pMsg)
{
	SendData(pMsg,pMsg->m_MessageLength);
}


//---------------------------------------------------------------------
/** Send some data to the socket.
 * \param buffer Void pointer to the data to send.
 * \param len Integer length of the data in buffer.
 */

void CqDDClient::SendData(void* buffer, TqInt len)
{
	send(m_Socket,reinterpret_cast<char*>(buffer),len,0);
}


//---------------------------------------------------------------------
/** Thread function, just sits listening to the port for connection requests.
 * Upon recieving a valid request the client socket is added to the list of clients.
 */

static void AcceptConnections(void* pvServer)
{
	CqDDServer* pServer=reinterpret_cast<CqDDServer*>(pvServer);

	if(pServer)
	{
		while(1)
		{
			// Check if the server has quit.
			if(pServer->bHasQuit())
			{
				QGetRenderContext()->SignalDDThreadFinished();
				_endthread();
			}

			SOCKET c;

			if((c=accept(pServer->Socket(),NULL,NULL))==INVALID_SOCKET)
			{
				TqInt err=WSAGetLastError();
			}
			else
			{
				unsigned long argp=1;
				// Set nonblocking.
				ioctlsocket(c,FIONBIO,&argp);
				pServer->AddClient(c);
			}
		}
	}
}


END_NAMESPACE(Aqsis)
