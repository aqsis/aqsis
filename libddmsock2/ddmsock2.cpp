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
#include	<stdio.h>
#include	<stdlib.h>
#include	<malloc.h>

#ifdef AQSIS_SYSTEM_WIN32

#include	<process.h>

#else // AQSIS_SYSTEM_WIN32

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
#include	"ddmsock2.h"
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

TqInt CqDDManager::Initialise(IqRenderer* renderer)
{
	m_Listener.Prepare(AQSIS_LISTENER_PORT, this);
	return ( 0 );
}

TqInt CqDDManager::Shutdown()
{
	m_Listener.Close();
    return ( 0 );
}

TqInt CqDDManager::AddDisplay( const TqChar* name, const TqChar* type, const TqChar* mode, TqInt modeID, TqInt dataOffset, TqInt dataSize, std::map<std::string, void*> mapOfArguments )
{
	SqDisplayRequest req;
	req.m_name = name;
	req.m_type = type;
	req.m_mode = mode;
	req.m_dataOffset = dataOffset;
	req.m_dataSize = dataSize;

	m_displayRequests.push_back(req);

	return(0);
}

TqInt CqDDManager::ClearDisplays()
{
	m_displayRequests.clear();
    return ( 0 );
}

TqInt CqDDManager::OpenDisplays(IqRenderer* renderer)
{
	std::string name;
	// If the filename has been specified in the RIB file, then use that,, 
	const CqString* optname = renderer->GetStringOption( "bucketcache", "name" );
	if( optname )
		name = *optname;
	else
		// If not, use a temporary filename.
		name = tmpnam(NULL);

	TqBool temp = TqTrue;
	// Find out if the user wants to keep the bucket store.
	const TqInt* opttemp = renderer->GetIntegerOption( "bucketcache", "temp" );
	if( opttemp )
		temp = *opttemp != 0;

	std::cerr << info << "Bucket store filename is " << name.c_str() << std::endl;
	
	if( temp )	std::cerr << info << "Bucket store will be deleted" << std::endl;
	else		std::cerr << info << "Bucket store won't be deleted" << std::endl;

	m_DiskStore.PrepareFile(name, renderer, temp);

	// Now go over any requested displays launching the clients.
	std::vector<SqDisplayRequest>::iterator i;
	for(i = m_displayRequests.begin(); i!=m_displayRequests.end(); i++)
		LoadDisplayLibrary(*i);

    return ( 0 );
}

TqInt CqDDManager::CloseDisplays()
{
    return ( 0 );
}

TqInt CqDDManager::DisplayBucket( IqBucket* pBucket )
{
	TqInt	record_index;
	m_DiskStore.StoreBucket(pBucket, NULL, &record_index);
	// Check if any sender threads are waiting on this bucket.
	std::map<TqInt, boost::condition*>::iterator waiting;
	if((waiting = m_BucketRequestsWaiting.find(record_index)) != m_BucketRequestsWaiting.end() )
	{
		std::cerr << debug << "Hit a waiting bucket " << record_index << std::endl;
		waiting->second->notify_all();
	}
    return ( 0 );
}



TqBool CqDDManager::fDisplayNeeds( const TqChar* var )
{
/*    static TqUlong rgb = CqParameter::hash( "rgb" );
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
    }*/
    return ( TqFalse );
}


TqInt CqDDManager::Uses()
{
    TqInt Uses = 0;
/*    std::vector<CqDDClient>::iterator i;
    for ( i = m_aDisplayRequests.begin(); i != m_aDisplayRequests.end(); i++ )
    {
        TqInt ivar;
        for( ivar = 0; ivar < EnvVars_Last; ivar++ )
        {
            if( i->hMode() == gVariableTokens[ ivar ] )
                Uses |= 1 << ivar;
        }
    }*/
    return ( Uses );
}

/**
  Load the requested display library according to the specified mode in the RiDisplay command.
 
*/
void CqDDManager::LoadDisplayLibrary( SqDisplayRequest& req )
{

    if ( !g_fDisplayMapInitialised )
        InitialiseDisplayNameMap();

    // strDriverFileAndArgs: Second part of the ddmsock.ini line (e.g. "mydriver.exe --foo")
    CqString strDriverFileAndArgs = g_mapDisplayNames[ req.m_type ];
    // strDriverFile: Only the executable without arguments (e.g. "mydriver.exe")
    CqString strDriverFile = GetStringField( strDriverFileAndArgs, 0 );

	// Display type not found.
    if ( strDriverFile.empty() )
    {
        //CqBasicError( ErrorID_DisplayDriver, Severity_Normal, "Invalid display type. \"" + dd.strType + "\"" );
        throw( CqString( "Invalid display type \"" ) + req.m_type + CqString( "\"" ) );
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
//    SetEnvironmentVariable( "AQSIS_DD_PORT", ToString(m_DDServer.getPort()).c_str() );

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
//    putenv( const_cast<char*>(("AQSIS_DD_PORT=" + ToString(m_DDServer.getPort())).c_str()));
    
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

    ddmsock_path.append( "ddmsock2.ini" );

    CqString strConfigFile = ddmsock_path;
#else
    CqString strConfigFile = "ddmsock2.ini";
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


/*void SendUserParameters( std::map<std::string, void*>& mapParams, CqDDClient* client )
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
*/
END_NAMESPACE( Aqsis )
