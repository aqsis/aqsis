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
		\brief Sockets based display device manager.
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
#include	"ddmsock.h"
#include	"imagebuffer.h"
#include	"rifile.h"


START_NAMESPACE( Aqsis )

#ifdef AQSIS_SYSTEM_POSIX

/// Handle cleanup of child processes
void sig_chld( int signo )
{
    pid_t pid;
    int stat;

    while ( ( pid = waitpid( -1, &stat, WNOHANG ) ) > 0 );

    return ;
}

#endif // AQSIS_SYSTEM_POSIX

/// Required function that implements Class Factory design pattern for DDManager libraries
IqDDManager* CreateDisplayDriverManager()
{
    return new CqDDManager;
}

void SendUserParameters( std::map<std::string, void*>& mapParams, CqDDClient* client );

//////////////////////////// CqDDServer ///////////////////////////////

//---------------------------------------------------------------------
/** Default constructor.
 */

CqDDServer::CqDDServer() :
        m_Socket( INVALID_SOCKET )
{}

//---------------------------------------------------------------------
/** Constructor, takes a port no. and prepares the socket to accept clients.
 */

CqDDServer::CqDDServer( TqInt port )
{
    Prepare( port );
}


//---------------------------------------------------------------------
/** Destructor, close all connected client sockets.
 */

CqDDServer::~CqDDServer()
{
    Close();
}

//---------------------------------------------------------------------
/** Close the servers socket, waiting for any connectsion acticity to finish first,
 */

void CqDDServer::Close()
{
#ifdef AQSIS_SYSTEM_WIN32
    int x = 1;
    setsockopt( m_Socket, SOL_SOCKET, SO_DONTLINGER, reinterpret_cast<const char*>( &x ), sizeof( x ) );
    shutdown( m_Socket, SD_BOTH );
    closesocket( m_Socket );
#else // AQSIS_SYSTEM_WIN32
    shutdown( m_Socket, SD_BOTH );
    close( m_Socket );
#endif // !AQSIS_SYSTEM_WIN32

    m_Socket = INVALID_SOCKET;
}


//---------------------------------------------------------------------
/** Prepare the socket to accept client connections.
 * \param port Integer port number to use.
 */

TqBool	CqDDServer::Prepare( TqInt port )
{
    if ( Open() )
        if ( Bind( port ) )
            if ( Listen() )
                return ( TqTrue );
    return ( TqFalse );
}


//---------------------------------------------------------------------
/** Create the socket.
 */

TqBool CqDDServer::Open()
{
    m_Socket = socket( AF_INET, SOCK_STREAM, 0 );

    if ( m_Socket == INVALID_SOCKET )
    {
#ifdef AQSIS_SYSTEM_WIN32
        TqInt err = WSAGetLastError();
#endif // AQSIS_SYSTEM_WIN32
        std::cerr << error << "Error opening DD server socket" << std::endl;
        return ( TqFalse );
    }

    TqInt x = 1;
    setsockopt( m_Socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>( &x ), sizeof( x ) );

#ifdef AQSIS_SYSTEM_WIN32
    BOOL Ret = SetHandleInformation( ( HANDLE ) m_Socket, HANDLE_FLAG_INHERIT, 0 );
#endif // AQSIS_SYSTEM_WIN32

    return ( TqTrue );
}


//---------------------------------------------------------------------
/** Bind the socket to a specified port.
 */

TqBool CqDDServer::Bind( TqInt port )
{
    SOCKADDR_IN saTemp;
    memset( &saTemp, 0, sizeof( saTemp ) );
    saTemp.sin_family = AF_INET;
    saTemp.sin_port = htons( port );
    saTemp.sin_addr.s_addr = INADDR_ANY;

    while ( bind( m_Socket, ( PSOCKADDR ) & saTemp, sizeof( saTemp ) ) == SOCKET_ERROR )
    {
#ifdef AQSIS_SYSTEM_WIN32
        TqInt iE = WSAGetLastError();
        if ( errno == WSAEADDRINUSE )
#else // AQSIS_SYSTEM_WIN32
        if ( errno == EADDRINUSE )
#endif // AQSIS_SYSTEM_WIN32
        {
            port++;
            saTemp.sin_port = htons( port );
            continue;
        }
        else
        {
            std::cerr << error << "Error binding to DD socket" << std::endl;
            Close();
            return ( TqFalse );
        }
    }
    m_Port = port;
    return ( TqTrue );
}


//---------------------------------------------------------------------
/** Prepare the socket to listen for client connections.
 */

TqBool CqDDServer::Listen()
{
    if ( listen( m_Socket, SOMAXCONN ) == SOCKET_ERROR )
    {
        std::cerr << error << "Error listening to DD socket" << std::endl;
        Close();
        return ( TqFalse );
    }
    return ( TqTrue );
}


//---------------------------------------------------------------------
/** Set ip the thread to wait for client connection requests.
 */

TqBool CqDDServer::Accept( CqDDClient& dd )
{
    SOCKET c;

    if ( ( c = accept( Socket(), NULL, NULL ) ) != INVALID_SOCKET )
    {
        dd.SetSocket( c );
        return( TqTrue );
    }
    return ( TqFalse );
}

//////////////////////////// CqDDClient ///////////////////////////////


CqDDClient::CqDDClient( const TqChar* name, const TqChar* type, const TqChar* mode, TqInt modeID, TqInt dataOffset, TqInt dataSize ) :
        m_Socket( INVALID_SOCKET ),
        m_strName( name ),
        m_strType( type ),
        m_strMode( mode ),
        m_modeID( modeID ),
        m_dataOffset( dataOffset ),
        m_dataSize( dataSize )
{
    m_hMode = CqParameter::hash( mode );
}

CqDDClient::~CqDDClient()
{}


//---------------------------------------------------------------------
/** Close the socket this client is associated with.
 */
void CqDDClient::Close()
{
#ifdef AQSIS_SYSTEM_WIN32
    int x = 1;
    setsockopt( m_Socket, SOL_SOCKET, SO_DONTLINGER, reinterpret_cast<const char*>( &x ), sizeof( x ) );
    shutdown( m_Socket, SD_BOTH );
    closesocket( m_Socket );
#else // AQSIS_SYSTEM_WIN32
    shutdown( m_Socket, SD_BOTH );
    close( m_Socket );
#endif // !AQSIS_SYSTEM_WIN32

    m_Socket = INVALID_SOCKET;
}



//---------------------------------------------------------------------
/** Send some data to the socket.
 * \param buffer Void pointer to the data to send.
 * \param len Integer length of the data in buffer.
 */

void CqDDClient::SendData( void* buffer, TqInt len )
{
    if ( m_Socket == INVALID_SOCKET )
        return ;

    TqInt tot = 0, need = len;
    while ( need > 0 )
    {
        TqInt n = send( m_Socket, reinterpret_cast<char*>( buffer ) + tot, need, 0 );
        need -= n;
        tot += n;
    }
}


//---------------------------------------------------------------------
/** Send a preconstructed message structure to this client.
 * \param pMsg Pointer to a SqDDMessageBase derive structure.
 */

void CqDDClient::SendMsg( SqDDMessageBase* pMsg )
{
    SendData( pMsg, pMsg->m_MessageLength );
}


//---------------------------------------------------------------------
/** Receive some data from the socket.
 * \param buffer Void pointer to the storage area for the data.
 * \param len Integer length of the data required.
 */

void CqDDClient::Receive( void* buffer, TqInt len )
{
    TqInt tot = 0, need = len;
    while ( need > 0 )
    {
        TqInt n = recv( m_Socket, reinterpret_cast<char*>( buffer ) + tot, need, 0 );
        need -= n;
        tot += n;
    }
}


//////////////////////////// CqDDManager ///////////////////////////////


std::map<std::string, std::string>	g_mapDisplayNames;
TqBool	g_fDisplayMapInitialised = false;

CqDDManager::CqDDManager()
{
#ifdef AQSIS_SYSTEM_WIN32
    WSADATA data;
    WSAStartup( MAKEWORD( 2, 0 ), &data );
#endif // AQSIS_SYSTEM_WIN32
}

CqDDManager::~CqDDManager()
{
#ifdef AQSIS_SYSTEM_WIN32
    WSACleanup();
#endif // AQSIS_SYSTEM_WIN32
}

TqInt CqDDManager::Initialise(IqRenderer*)
{
    if ( !m_DDServer.Prepare( AQSIS_DD_PORT ) )
        return ( -1 );
    else
        return ( 0 );
}

TqInt CqDDManager::Shutdown()
{
    std::vector<CqDDClient>::iterator i;
    for ( i = m_aDisplayRequests.begin(); i != m_aDisplayRequests.end(); i++ )
        i->Close();
    m_DDServer.Close();
    g_mapDisplayNames.clear();
    g_fDisplayMapInitialised = false;
    return ( 0 );
}

TqInt CqDDManager::AddDisplay( const TqChar* name, const TqChar* type, const TqChar* mode, TqInt modeID, TqInt dataOffset, TqInt dataSize, std::map<std::string, void*> mapOfArguments )
{
    CqDDClient New( name, type, mode, modeID, dataOffset, dataSize );
    TqInt result = 0;
    m_aDisplayRequests.push_back( New );
    try
    {
        LoadDisplayLibrary( m_aDisplayRequests.back(), mapOfArguments );
    }
    catch( CqString str )
    {
        std::cerr << error << str.c_str() << std::endl;
        result = 1;
        m_aDisplayRequests.pop_back();
    }
    return ( result );
}

TqInt CqDDManager::ClearDisplays()
{
    SqDDMessageAbandon msg;

    std::vector<CqDDClient>::iterator i;

    for ( i = m_aDisplayRequests.begin(); i != m_aDisplayRequests.end(); i++ )
    {
        if ( i->Socket() != INVALID_SOCKET )
            i->SendMsg( &msg );
    }
    m_aDisplayRequests.clear();
    return ( 0 );
}

TqInt CqDDManager::OpenDisplays(IqRenderer*)
{
    std::vector<CqDDClient>::iterator i;
    for ( i = m_aDisplayRequests.begin(); i != m_aDisplayRequests.end(); i++ )
    {
        CqDDClient& dd = *i;
        CqMatrix matWorldToCamera = QGetRenderContext() ->matSpaceToSpace( "world", "camera", CqMatrix(), CqMatrix(), QGetRenderContextI()->Time() );
        CqMatrix matWorldToScreen = QGetRenderContext() ->matSpaceToSpace( "world", "screen", CqMatrix(), CqMatrix(), QGetRenderContextI()->Time() );

        if ( matWorldToCamera.fIdentity() ) matWorldToCamera.Identity();
        if ( matWorldToScreen.fIdentity() ) matWorldToScreen.Identity();

        SqDDMessageNl msgnl( matWorldToCamera.pElements() );
        dd.SendMsg( &msgnl );

        SqDDMessageNP msgnp( matWorldToScreen.pElements() );
        dd.SendMsg( &msgnp );

        // Send the open message..
        TqInt SamplesPerElement = dd.GetdataSize();
        TqFloat* pQuant = QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "Quantize", "Color" );
        TqInt one = static_cast<TqInt>( pQuant [ 0 ] );
        TqInt min = static_cast<TqInt>( pQuant [ 1 ] );
        TqInt max = static_cast<TqInt>( pQuant [ 2 ] );
        TqBool FloatOutput = ( one == 0 && min == 0 && max == 0 );
        SqDDMessageOpen msgopen( QGetRenderContext() ->pImage() ->iXRes(),
                                 QGetRenderContext() ->pImage() ->iYRes(),
                                 SamplesPerElement,
                                 //BitsPerSample,  	// Bits per sample.
                                 QGetRenderContext() ->pImage() ->CropWindowXMin(),
                                 QGetRenderContext() ->pImage() ->CropWindowXMax(),
                                 QGetRenderContext() ->pImage() ->CropWindowYMin(),
                                 QGetRenderContext() ->pImage() ->CropWindowYMax() );
        dd.SendMsg( &msgopen );

        // Issue a format request so that we know what data to send to the client.
        TqInt formats[2];
        if( FloatOutput )
        {
            formats[0] = DataFormat_Float32;
            formats[1] = DataFormat_Unsigned8;
        }
        else
        {
            formats[0] = DataFormat_Unsigned8;
            formats[1] = DataFormat_Float32;
        }
        SqDDMessageFormatQuery* msg = SqDDMessageFormatQuery::Construct( 2, formats );
        SqDDMessageFormatResponse frmt;

        dd.SendMsg( msg );
        dd.Receive( &frmt, sizeof( frmt ) );
        msg->Destroy();

        // Confirm the message returned is as expected.
        if ( frmt.m_MessageID != MessageID_FormatResponse ||
                frmt.m_MessageLength != sizeof( frmt ) )
            dd.Close();
    }

    return ( 0 );
}

TqInt CqDDManager::CloseDisplays()
{
    SqDDMessageClose msg;
    SqDDMessageCloseAcknowledge ack;

    std::vector<CqDDClient>::iterator i;

    for ( i = m_aDisplayRequests.begin(); i != m_aDisplayRequests.end(); i++ )
    {
        if ( i->Socket() != INVALID_SOCKET )
        {
            i->SendMsg( &msg );
            i->Receive( &ack, sizeof( ack ) );

            // Confirm the message returned is as expected.
            if ( ack.m_MessageID == MessageID_CloseAcknowledge &&
                    ack.m_MessageLength == sizeof( ack ) )
                continue;
            else
            {
                std::cerr << error << "Failed to close display device" << std::endl;
            }
        }
    }
    return ( 0 );
}

TqInt CqDDManager::DisplayBucket( IqBucket* pBucket )
{
    // Copy the bucket to the display buffer.
    TqInt	xmin = pBucket->XOrigin();
    TqInt	ymin = pBucket->YOrigin();
    TqInt	xsize = pBucket->Width();
    TqInt	ysize = pBucket->Height();
    TqInt	xmaxplus1 = xmin + xsize;
    TqInt	ymaxplus1 = ymin + ysize;

    for ( std::vector<CqDDClient>::iterator i = m_aDisplayRequests.begin(); i != m_aDisplayRequests.end(); i++ )
    {
        TqInt mode = i->GetmodeID();
        TqInt samples = i->GetdataSize();

        TqInt	elementsize = samples * sizeof( TqFloat );
        TqInt	datalen = xsize * ysize * elementsize;

        TqFloat*	pData = new TqFloat[ xsize * ysize * samples ];

        TqInt	linelen = xsize * samples;

        TqInt y;
        for ( y = 0; y < ysize; y++ )
        {
            TqInt sy = y + ymin;
            TqInt x;
            for ( x = 0; x < xsize; x++ )
            {
                TqInt sx = x + xmin;
                TqInt so = ( y * linelen ) + ( x * samples );
                // If outputting a zfile, use the midpoint method.
                /// \todo Should really be generalising this section to use specif Filter/Expose/Quantize functions.
                if ( mode & ModeZ )
                {
                    pData[ so ] = pBucket->Depth( sx, sy );
                }
                else
                {
                    if ( mode & ModeRGB )
                    {
                        const TqFloat* pSamples = pBucket->Data( sx, sy );
                        pData[ so + 0 ] = pSamples[0];
                        pData[ so + 1 ] = pSamples[1];
                        pData[ so + 2 ] = pSamples[2];
                        if ( mode & ModeA )
                        {
                            TqFloat a = ( pSamples[3] + pSamples[4] + pSamples[5] ) / 3.0f;
                            pData[ so + 3 ] = a * pBucket->Coverage( sx, sy );
                        }
                    }
                    else if ( mode & ModeA )
                    {
                        const TqFloat* pSamples = pBucket->Data( sx, sy );
                        TqFloat a = ( pSamples[3] + pSamples[4] + pSamples[5] ) / 3.0f;
                        pData[ so + 0 ] = a * pBucket->Coverage( sx, sy );
                    }
                    else
                    {
                        // If not any of the standard modes, then check for AOV.
                        TqInt ioffset = i->GetdataOffset();
                        if( i->GetdataSize() != 0 )
                        {
                            const TqFloat* pSamples = pBucket->Data( sx, sy );
                            TqInt idata;
                            for( idata = 0; idata < i->GetdataSize(); idata++ )
                                pData[ so + idata ] = pSamples[ ioffset + idata ];
                        }
                    }
                }
            }
        }
        SqDDMessageData* pmsg = SqDDMessageData::Construct( xmin, xmaxplus1, ymin, ymaxplus1, elementsize, pData, datalen );
        delete[] ( pData );
        i->SendMsg( pmsg );
        pmsg->Destroy();
    }
    return ( 0 );
}



TqBool CqDDManager::fDisplayNeeds( const TqChar* var )
{
    static TqUlong rgb = CqParameter::hash( "rgb" );
    static TqUlong rgba = CqParameter::hash( "rgba" );
    static TqUlong Ci = CqParameter::hash( "Ci" );
    static TqUlong Oi = CqParameter::hash( "Oi" );

    TqUlong htoken = CqParameter::hash( var );

    std::vector<CqDDClient>::iterator i;
    for ( i = m_aDisplayRequests.begin(); i != m_aDisplayRequests.end(); i++ )
    {
        TqBool usage = ( ( i->hMode() == rgba ) || ( i->hMode() == rgb ) );
        if ( ( htoken == Ci ) && usage )
            return ( TqTrue );
        else if ( ( htoken == Oi ) && usage )
            return ( TqTrue );
        else if ( ( i->hMode() == htoken ) )
            return ( TqTrue );
    }
    return ( TqFalse );
}


TqInt CqDDManager::Uses()
{
    TqInt Uses = 0;
    std::vector<CqDDClient>::iterator i;
    for ( i = m_aDisplayRequests.begin(); i != m_aDisplayRequests.end(); i++ )
    {
        TqInt ivar;
        for( ivar = 0; ivar < EnvVars_Last; ivar++ )
        {
            if( i->hMode() == gVariableTokens[ ivar ] )
                Uses |= 1 << ivar;
        }
    }
    return ( Uses );
}

/**
  Load the requested display library according to the specified mode in the RiDisplay command.
 
*/
void CqDDManager::LoadDisplayLibrary( CqDDClient& dd, std::map<std::string, void*>& mapParams )
{
    if ( !g_fDisplayMapInitialised )
        InitialiseDisplayNameMap();

    // strDriverFileAndArgs: Second part of the ddmsock.ini line (e.g. "mydriver.exe --foo")
    CqString strDriverFileAndArgs = g_mapDisplayNames[ dd.strType() ];
    // strDriverFile: Only the executable without arguments (e.g. "mydriver.exe")
    CqString strDriverFile = GetStringField( strDriverFileAndArgs, 0 );

    // Check-ins said temporary way to handle locating display drivers.
    // Until finished provide a helpful message for folks instead of
    //    trapping.
    if ( strDriverFile.empty() )
    {
        //CqBasicError( ErrorID_DisplayDriver, Severity_Normal, "Invalid display type. \"" + dd.strType + "\"" );
        throw( CqString( "Invalid display type \"" ) + dd.strType() + CqString( "\"" ) );
    }

    // Try to open the file to see if it's really there
    CqRiFile fileDriver( strDriverFile.c_str(), "display" );

    if ( !fileDriver.IsValid() )
    {
        //CqBasicError( 0, 0, ( "Error loading display driver [ " + strDriverFile + " ]" ).c_str() );
        throw( CqString( "Error loading display driver [ " ) + strDriverFile + CqString( " ]" ) );
    }

    CqString strDriverPathAndFile = fileDriver.strRealName();

    const int maxargs = 20;
    const char* args[ maxargs ];
    std::string argstrings[ maxargs ];
    int i;

    // Prepare an arry with the arguments
    // (the first argument (the file name) begins at position 2 because
    // the Windows version might have to call cmd.exe which will be
    // prepended to the argument list)
    args[ 2 ] = strDriverFile.c_str();
    args[ 3 ] = NULL;
    args[ maxargs - 1 ] = NULL;
    for ( i = 1; i < maxargs - 3; i++ )
    {
        argstrings[ i ] = GetStringField( strDriverFileAndArgs, i );
        if ( argstrings[ i ].length() == 0 )
        {
            args[ i + 2 ] = NULL;
            break;
        }
        args[ i + 2 ] = argstrings[ i ].c_str();
    }


#ifdef AQSIS_SYSTEM_WIN32

    // Set the AQSIS_DD_PORT environment variable
    SetEnvironmentVariable( "AQSIS_DD_PORT", ToString(m_DDServer.getPort()).c_str() );

    // Spawn the driver (1st try)...
    TqInt ProcHandle = _spawnv( _P_NOWAITO, strDriverPathAndFile.c_str(), &args[ 2 ] );
    // If it didn't work try a second time via "cmd.exe"...
    if ( ProcHandle < 0 )
    {
        args[ 0 ] = "cmd.exe";
        args[ 1 ] = "/C";
        args[ 2 ] = strDriverPathAndFile.c_str();
        ProcHandle = _spawnvp( _P_NOWAITO, "cmd.exe", args );
    }
    if ( ProcHandle < 0 )
    {
        //CqBasicError( 0, 0, "Error spawning display driver process" );
        throw( CqString( "Error spawning display driver process" ) );
    }

#else // AQSIS_SYSTEM_WIN32

    // Set the AQSIS_DD_PORT environment variable
    putenv( const_cast<char*>(("AQSIS_DD_PORT=" + ToString(m_DDServer.getPort())).c_str()));
    signal( SIGCHLD, sig_chld );
    // Spawn the driver
    const int forkresult = fork();
    // Start the driver in the child process
    if ( 0 == forkresult )
    {
        //execlp( strDriverPathAndFile.c_str(), strDriverFile.c_str(), NULL );
        execvp( strDriverPathAndFile.c_str(), ( char * const * ) ( args + 2 ) );
        /* error checking? */
        return ;
    }
    else if ( -1 == forkresult )
    {
        //CqBasicError( 0, 0, "Error forking display driver process" );
        throw( CqString( "Error forking display driver process" ) );
    }
#endif // !AQSIS_SYSTEM_WIN32

    // wait for a connection request from the client
    if ( m_DDServer.Accept( dd ) )
    {
        // Send a filename message
        SqDDMessageFilename * pmsgfname = SqDDMessageFilename::Construct( dd.strName().c_str() );
        dd.SendMsg( pmsgfname );
        pmsgfname->Destroy();
        // Send a display type message
        SqDDMessageDisplayType * pmsgdtype = SqDDMessageDisplayType::Construct( dd.strType().c_str() );
        dd.SendMsg( pmsgdtype );
        pmsgdtype->Destroy();

        SendUserParameters( mapParams, &dd );
    }
}



void CqDDManager::InitialiseDisplayNameMap()
{
    // Read in the configuration file.
    // Find the config file in the same place as the display drivers.
#ifdef AQSIS_SYSTEM_POSIX
    CqString ddmsock_path( "" );
    char* env = NULL;

    env = getenv( "AQSIS_CONFIG_PATH" );
    if ( env == NULL )
    {
#ifndef AQSIS_SYSTEM_MACOSX
        ddmsock_path = CONFIG_PATH;
#else
        ddmsock_path = ".";
#endif

    }
    else
    {
        ddmsock_path = env;
    }

    ddmsock_path.append( "/" );

    ddmsock_path.append( "ddmsock.ini" );

    CqString strConfigFile = ddmsock_path;
#else
    CqString strConfigFile = "ddmsock.ini";
#endif /* AQSIS_SYSTEM_POSIX */

    CqRiFile fileINI( strConfigFile.c_str(), "display" );
    if ( fileINI.IsValid() )
    {
        // On each line, read the first string, then the second and store them in the map.
        std::string strLine;
        std::istream& strmINI = static_cast<std::istream&>( fileINI );

        while ( std::getline( strmINI, strLine ) )
        {
            std::string strName, strDriverName;
            std::string::size_type iStartN = strLine.find_first_not_of( '\t' );
            std::string::size_type iEndN = strLine.find_first_of( '\t', iStartN );
            std::string::size_type iStartD = strLine.find_first_not_of( '\t', iEndN );
            std::string::size_type iEndD = strLine.find_first_of( '\t', iStartD );
            if ( iStartN != std::string::npos && iEndN != std::string::npos &&
                    iStartD != std::string::npos )
            {
                strName = strLine.substr( iStartN, iEndN );
#ifdef AQSIS_SYSTEM_MACOSX
                if ( env == NULL )
                {
                    strDriverName = "./";
                }
                else
                {
                    strDriverName = env;
                    strDriverName.append( "/" );
                }
                strDriverName.append( strLine.substr( iStartD, iEndD ) );
#else
                strDriverName = strLine.substr( iStartD, iEndD );
#endif /* AQSIS_SYSTEM_MACOSX */
                g_mapDisplayNames[ strName ] = strDriverName;
            }
        }
        g_fDisplayMapInitialised = true;
    }
    else
    {
        std::cerr << error << "Could not find ddmsock.ini file." << std::endl;
    }
}

/**
  Return the substring with the given index.
 
  The string \a s is conceptually broken into substrings that are separated by blanks
  or tabs. A continuous sequence of blanks/tabs counts as one individual separator.
  The substring with number \a idx is returned (0-based). If \a idx is higher than the
  number of substrings then an empty string is returned.
 
  \param s Input string.
  \param idx Index (0-based)
  \return Sub string with given index
*/
std::string CqDDManager::GetStringField( const std::string& s, int idx )
{
    int z = 1;   /* state variable  0=skip whitespace  1=skip chars  2=search end  3=end */
    std::string::const_iterator it;
    std::string::size_type start = 0;
    std::string::size_type end = 0;

    for ( it = s.begin(); it != s.end(); it++ )
    {
        char c = *it;

        if ( idx == 0 && z < 2 )
        {
            z = 2;
        }

        switch ( z )
        {
        case 0: if ( c != ' ' && c != '\t' )
            {
                idx--;
                end = start + 1;
                z = 1;
            }
            if ( idx > 0 ) start++;
            break;
        case 1: if ( c == ' ' || c == '\t' )
            {
                z = 0;
            }
            start++;
            break;
        case 2: if ( c == ' ' || c == '\t' )
            {
                z = 3;
            }
            else
            {
                end++;
            }
            break;
        }
    }

    if ( idx == 0 )
        return s.substr( start, end - start );
    else
        return std::string( "" );

}


void SendUserParameters( std::map<std::string, void*>& mapParams, CqDDClient* client )
{
    std::map<std::string, void*>::iterator param;
    for ( param = mapParams.begin(); param != mapParams.end(); param++ )
    {
        SqParameterDeclaration Decl;
        try
        {
            Decl = QGetRenderContext() ->FindParameterDecl( param->first.c_str() );
        }
        catch( XqException e )
        {
            std::cerr << error << e.strReason().c_str() << std::endl;
            return;
        }

        // Check the parameter type is uniform, not valid for non-surface requests otherwise.
        if( Decl.m_Class != class_uniform )
        {
            assert( TqFalse );
            continue;
        }

        // Special case for strings, we need to build an array of strings to pass as we cannot pass pointers across processes.
        if( Decl.m_Type == type_string )
        {
            const char** strings = static_cast<const char**>( param->second );
            TqInt i;
            TqInt len = 0;
            for( i = 0; i < Decl.m_Count; i++ )
                len += strlen( strings[ i ] ) + 1;
            char* data = new char[ len ];
            memset( data, 0, len );
            len = 0;
            for( i = 0; i < Decl.m_Count; i++ )
            {
                strcpy( data + len, strings[ i ] );
                len += strlen( strings[ i ] ) + 1;
            }

            SqDDMessageUserParam* pmsg = SqDDMessageUserParam::Construct(Decl.m_strName.c_str(), Decl.m_Type, Decl.m_Count, data, len );
            client->SendMsg( pmsg );
            pmsg->Destroy();
            delete[]( data );
        }
        else
        {
            TqInt elementsize = 0;
            switch ( Decl.m_Type )
            {
            case type_float:
                elementsize = sizeof(TqFloat);
                break;

            case type_integer:
                elementsize = sizeof(TqInt);
                break;

            case type_point:
            case type_normal:
            case type_vector:
            case type_color:
                elementsize = sizeof(TqFloat) * 3;
                break;

            case type_hpoint:
                elementsize = sizeof(TqFloat) * 4;
                break;

            case type_matrix:
                elementsize = sizeof(TqFloat) * 16;
                break;
            }
            SqDDMessageUserParam* pmsg = SqDDMessageUserParam::Construct(Decl.m_strName.c_str(), Decl.m_Type, Decl.m_Count, param->second, Decl.m_Count * elementsize );
            client->SendMsg( pmsg );
            pmsg->Destroy();
        }
    }
}



END_NAMESPACE( Aqsis )
