// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
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

/**
        \file
        \brief Implements the classes and support structures for 
                handling RenderMan Procedural primitives.
        \author Jonathan Merritt (j.merritt@pgrad.unimelb.edu.au)
*/

#if _MSC_VER
#pragma warning(disable : 4786)
#endif

#include <stdio.h>
#include <string.h>
#include <list>
#include <map>

#include "aqsis.h"
#include "rifile.h"
#include "imagebuffer.h"
#include "micropolygon.h"
#include "renderer.h"
#include "procedural.h"
#include "plugins.h"
#include "librib2ri.h"
#include "librib.h"
#include "bdec.h"
#include "libribtypes.h"
#include "parserstate.h"

#ifdef AQSIS_SYSTEM_WIN32
#include <io.h>
#include <iomanip>
#else
#include <unistd.h>
#include <errno.h>
#endif



namespace Aqsis {


/**
 * CqProcedural constructor.
 */
CqProcedural::CqProcedural() : CqSurface()
{
	STATS_INC( GEO_prc_created );
}

/**
 * CqProcedural copy constructor.
 */
CqProcedural::CqProcedural(RtPointer data, CqBound &B, RtProcSubdivFunc subfunc, RtProcFreeFunc freefunc ) : CqSurface()
{
	m_pData = data;
	m_Bound = B;
	m_pSubdivFunc = subfunc;
	m_pFreeFunc = freefunc;

	m_pconStored = QGetRenderContext()->pconCurrent();

	STATS_INC( GEO_prc_created );
}




TqInt CqProcedural::Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
{
	// Store current context, set current context to the stored one
	boost::shared_ptr<CqModeBlock> pconSave = QGetRenderContext()->pconCurrent( m_pconStored );

	m_pconStored->m_pattrCurrent = m_pAttributes;
	ADDREF(m_pAttributes);

	m_pconStored->m_ptransCurrent = m_pTransform;

	/// \note: The bound is in "raster" coordinates by now, as during posting to the imagebuffer
	/// the the Culling routines do the job for us, see CqSurface::CacheRasterBound.
	CqBound bound = m_Bound;
	//    bound.Transform(QGetRenderContext()->matSpaceToSpace("camera", "raster"));
	float detail = ( bound.vecMax().x() - bound.vecMin().x() ) * ( bound.vecMax().y() - bound.vecMin().y() );
	//std::cout << "detail: " << detail << std::endl;

	// Call the procedural secific Split()
	RiAttributeBegin();

	if(m_pSubdivFunc)
		m_pSubdivFunc(m_pData, detail);

	RiAttributeEnd();

	// restore saved context
	QGetRenderContext()->pconCurrent( pconSave );

	STATS_INC( GEO_prc_split );

	return 0;
}


//---------------------------------------------------------------------
/** Transform the quadric primitive by the specified matrix.
 */

void    CqProcedural::Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime )
{
	m_Bound.Transform( matTx );
}


/**
 * CqProcedural destructor.
 */
CqProcedural::~CqProcedural()
{
	if( m_pFreeFunc )
		m_pFreeFunc( m_pData );
}


class CqRiProceduralPlugin : CqPluginBase
{
	private:
		void *( *m_ppvfcts ) ( char * );
		void ( *m_pvfctpvf ) ( void *, float );
		void ( *m_pvfctpv ) ( void * );
		void *m_ppriv;
		void *m_handle;
		bool m_bIsValid;
		CqString m_Error;

	public:
		CqRiProceduralPlugin( CqString& strDSOName )
		{
			CqString strConver("ConvertParameters");
			CqString strSubdivide("Subdivide");
			CqString strFree("Free");

			CqRiFile        fileDSO( strDSOName.c_str(), "procedural" );
			m_bIsValid = false ;

			if ( !fileDSO.IsValid() )
			{
				m_Error = CqString( "Cannot find Procedural DSO for \"" )
				          + strDSOName
				          + CqString ("\" in current searchpath");

				return;
			}

			CqString strRealName( fileDSO.strRealName() );
			fileDSO.Close();
			void *handle = DLOpen( &strRealName );

			if ( ( m_ppvfcts = ( void * ( * ) ( char * ) ) DLSym(handle, &strConver) ) == NULL )
			{
				m_Error = DLError();
				return;
			}

			if ( ( m_pvfctpvf = ( void ( * ) ( void *, float ) ) DLSym(handle, &strSubdivide) ) == NULL )
			{
				m_Error = DLError();
				return;
			}

			if ( ( m_pvfctpv = ( void ( * ) ( void * ) ) DLSym(handle, &strFree) ) == NULL )
			{
				m_Error = DLError();
				return;
			}

			m_bIsValid = true ;
		};

		void ConvertParameters(char* opdata)
		{
			if( m_bIsValid )
				m_ppriv = ( *m_ppvfcts ) ( opdata );
		};

		void Subdivide( float detail )
		{
			if( m_bIsValid )
				( *m_pvfctpvf ) ( m_ppriv, detail );
		};

		void Free()
		{
			if( m_bIsValid )
				( *m_pvfctpv ) ( m_ppriv );
		};

		bool IsValid(void)
		{
			return m_bIsValid;
		};

		const CqString Error()
		{
			return m_Error;
		};
};

// We don't want to DLClose until we are finished rendering, since any RiProcedurals
// created from within the dynamic module may re-use the Subdivide and Free pointers so
// they must remain linked in.
static std::list<CqRiProceduralPlugin*> ActiveProcDLList;





} // namespace Aqsis

using namespace Aqsis;

//----------------------------------------------------------------------
// RiProcFree()
//
extern "C" RtVoid	RiProcFree( RtPointer data )
{
	free(data);
}

//----------------------------------------------------------------------
// RiProcDynamicLoad() subdivide function
//
extern "C" RtVoid	RiProcDynamicLoad( RtPointer data, RtFloat detail )
{
	CqString dsoname = CqString( (( char** ) data)[0] );
	CqRiProceduralPlugin *plugin = new CqRiProceduralPlugin( dsoname );

	if( !plugin->IsValid() )
	{
		dsoname = CqString( (( char** ) data)[0] ) + CqString(SHARED_LIBRARY_SUFFIX);
		plugin = new CqRiProceduralPlugin( dsoname );

		if( !plugin->IsValid() )
		{
			Aqsis::log() << error << "Problem loading Procedural DSO: [" << plugin->Error().c_str() << "]" << std::endl;
			return;
		}
	}

	plugin->ConvertParameters( (( char** ) data)[1] );
	plugin->Subdivide( detail );
	plugin->Free();

	ActiveProcDLList.push_back( plugin );

	STATS_INC( GEO_prc_created_dl );
}

//----------------------------------------------------------------------
// RiProcDelayedReadArchive()
//
extern "C" RtVoid	RiProcDelayedReadArchive( RtPointer data, RtFloat detail )
{
	RiReadArchive( (RtToken) ((char**) data)[0], NULL, RI_NULL );
	STATS_INC( GEO_prc_created_dra );
}

//----------------------------------------------------------------------
/* RiProcRunProgram()
 * Your program must writes its output to a pipe. Open this
 * pipe with read text attribute so that we can read it 
 * like a text file. 
 */

// TODO: This is far from ideal, we need to parse directly from the popene'd
// process.

#ifndef	AQSIS_SYSTEM_WIN32

class CqRiProceduralRunProgram
{
	public:
		int fd_out[2]; // aqsis -> servant
		int fd_in[2]; // servant -> aqsis
		pid_t pid;
		FILE *out, *in;
};

static std::map<std::string, CqRiProceduralRunProgram*> ActiveProcRP;

extern "C" RtVoid	RiProcRunProgram( RtPointer data, RtFloat detail )
{
	std::map<std::string, CqRiProceduralRunProgram*>::iterator it;

	it = ActiveProcRP.find(CqString( ((char**)data)[0] )) ;
	if( it == ActiveProcRP.end() )
	{
		// We don't have an active RunProgram for the specifed
		// program. We need to try and fork a new one
		CqRiProceduralRunProgram *run_proc = new CqRiProceduralRunProgram;
		if( pipe( run_proc->fd_in ) || pipe( run_proc->fd_out ) )
		{
			throw(XqException("Error creating pipes"));
		}

		run_proc->pid = fork() ;

		if (run_proc->pid < 0)
		{
			// problem forking
			return;
		}
		else if( run_proc->pid != 0 )
		{
			// main process
			// fix up the filehandles.

			close(run_proc->fd_out[0]); // we write to fd_out[1]
			close(run_proc->fd_in[1]);  // we read from fd_in[0]

			run_proc->out = fdopen(dup(run_proc->fd_out[1]), "wb");
			//			setvbuf(run_proc->out, NULL, _IONBF, 0);
			run_proc->in = fdopen(dup(run_proc->fd_in[0]), "rb");
			//			setvbuf(run_proc->in, NULL, _IONBF, 0);

			ActiveProcRP[std::string(((char**)data)[0])] = run_proc;
			it = ActiveProcRP.find(std::string(((char**)data)[0]));
		}
		else
		{
			// Find the actual location of the executable, using the "procedural" searchpath.
			CqRiFile searchFile;
			searchFile.Open((( char** ) data)[0], "procedural");
			char* progname;
			if(!searchFile.IsValid())		
			{
				Aqsis::log() << info << "RiProcRunProgram: Could not find \"" << ((char**)data)[0] << "\" in \"procedural\" searchpath, will rely on the PATH." << std::endl;
				progname = ((char**)data)[0];
			}
			else
				progname = strdup(searchFile.strRealName().c_str());

			// Split up the RunProgram program name string
			int arg_count = 1;
			char *arg_values[32];
			arg_values[0] = progname;
			char *i = arg_values[0];
			for( ; *i != '\0' ; i++ )
			{
				if( *i == ' ' )
				{
					arg_count++ ;
					*i = '\0' ;
					arg_values[arg_count - 1] = i + 1 ;
				}
			};
			arg_values[arg_count] = NULL;

			// fix up the filehandles.

			close(run_proc->fd_out[1]);
			close(run_proc->fd_in[0]);

			close( STDIN_FILENO );
			if(dup( run_proc->fd_out[0] )<0)
				throw(XqException("Error preparing stdin for RunProgram"));
			//			setvbuf( stdin, NULL, _IONBF, 0 );
			close( STDOUT_FILENO );
			if(dup( run_proc->fd_in[1] )<0)
				throw(XqException("Error preparing stdout for RunProgram"));
			//			setvbuf( stdout, NULL, _IONBF, 0 );

			execvp( arg_values[0], arg_values );
			free(progname);
		};
	};

	FILE *fileout = (it->second)->out;
	if(!fileout)
	{
		Aqsis::log() << error << "Unable to open output stream for \"RunProgram\" error is " <<  errno << std::endl;
		return;
	}
	// Write out detail and data to the process
	fprintf( fileout, "%g %s\n", detail, ((char**)data)[1] );
	fflush( fileout );
	// should check for a SIGPIPE here incase the RunProgram died.

	// This is not pretty, gzopen attempts to read a gzip header
	//from the stream, if we try and do this before we have sent
	//the request the we get deadlock, so we have to create the decoder
	//after the request, we use a buffer size of one to prevent any
	//potential blocking.
	CqRibBinaryDecoder *decoder;
	FILE *filein = (it->second)->in;
	if(!filein)
	{
		Aqsis::log() << error << "Unable to open input stream for \"RunProgram\" error is " <<  errno << std::endl;
		return;
	}
	decoder = new CqRibBinaryDecoder( filein, 1);

	// Parse the resulting block of RIB.
	CqString strRealName( ((char**)data)[0] );
	CqRIBParserState currstate = librib::GetParserState();

	if( currstate.m_pParseCallbackInterface == NULL )
		currstate.m_pParseCallbackInterface = new librib2ri::Engine;

	librib::ParseOpenStream( decoder, strRealName.c_str(), *(currstate.m_pParseCallbackInterface), *(currstate.m_pParseErrorStream), (RtArchiveCallback) RI_NULL );

	librib::SetParserState( currstate );

	delete( decoder );

	STATS_INC( GEO_prc_created_prp );

	return;
}

#else

#include <windows.h>
#include <fcntl.h>

class CqRiProceduralRunProgram
{
	public:
		HANDLE hChildStdinWrDup;
		HANDLE hChildStdoutRdDup;
		FILE *fChildStdoutRd;
		bool m_valid;
};

static std::map<std::string, CqRiProceduralRunProgram*> ActiveProcRP;


extern "C" RtVoid	RiProcRunProgram( RtPointer data, RtFloat detail )
{
	HANDLE hChildStdinRd, hChildStdinWr, hChildStdoutRd, hChildStdoutWr;
	HANDLE hParentStderrWr, hDupStderrWr;
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	SECURITY_ATTRIBUTES saAttr;
	BOOL bFuncRetn = FALSE;

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	std::map<std::string, CqRiProceduralRunProgram*>::iterator it;

	it = ActiveProcRP.find(CqString( ((char**)data)[0] )) ;
	if( it == ActiveProcRP.end() )
	{
		// We don't have an active RunProgram for the specifed
		// program. We need to try and fork a new one
		CqRiProceduralRunProgram *run_proc = new CqRiProceduralRunProgram;
		// Proc is invalid until fully resolved.
		run_proc->m_valid = false;

		ActiveProcRP[std::string(((char**)data)[0])] = run_proc;
		it = ActiveProcRP.find(std::string(((char**)data)[0]));

		// Create a pipe for the child process's STDOUT.

		if (! CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0) ||
		    ! CreatePipe(&hChildStdinRd,  &hChildStdinWr,  &saAttr, 0) )
		{
			Aqsis::log() << error << "RiProcRunProgram: Stdout pipe creation failed" << std::endl;
			return;
		}
		hParentStderrWr = GetStdHandle(STD_ERROR_HANDLE);
		if( ! DuplicateHandle(GetCurrentProcess(), hParentStderrWr, GetCurrentProcess(), &hDupStderrWr, 0, TRUE, DUPLICATE_SAME_ACCESS) )
		{
			Aqsis::log() << error << "RiProcRunProgram: Stderr handle duplication failed" << std::endl;
			return;
		}

		SetHandleInformation(hChildStdinWr, HANDLE_FLAG_INHERIT, 0);
		SetHandleInformation(hChildStdoutRd,  HANDLE_FLAG_INHERIT, 0);

		ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
		siStartInfo.cb = sizeof(STARTUPINFO);
		siStartInfo.hStdOutput = hChildStdoutWr;
		siStartInfo.hStdError = hDupStderrWr;
		siStartInfo.hStdInput = hChildStdinRd;
		siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
		ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );

		// Find the actual location of the executable, using the "procedural" searchpath.
		CqRiFile searchFile;
		searchFile.Open((( char** ) data)[0], "procedural");
		char* progname;
		if(!searchFile.IsValid())		
		{
			searchFile.Open((CqString( (( char** ) data)[0] ) + CqString(".exe")).c_str(), "procedural");
			if(!searchFile.IsValid())		
			{
				Aqsis::log() << info << "RiProcRunProgram: Could not find \"" << ((char**)data)[0] << "\" in \"procedural\" searchpath, will rely on the PATH." << std::endl;
				progname = _strdup(((char**)data)[0]);
			}
			else
				progname = _strdup(searchFile.strRealName().c_str());
		}
		else
			progname = _strdup(searchFile.strRealName().c_str());

		// Create the child process.
		bFuncRetn = CreateProcess(NULL,
		                          progname, // command line
		                          NULL,          // process security attributes
		                          NULL,          // primary thread security attributes
		                          TRUE,          // handles are inherited
		                          0,             // creation flags
		                          NULL,          // use parent's environment
		                          NULL,          // use parent's current directory
		                          &siStartInfo,  // STARTUPINFO pointer
		                          &piProcInfo);  // receives PROCESS_INFORMATION
		free(progname);

		if (bFuncRetn == 0)
		{
			Aqsis::log() << error << "RiProcRunProgram: CreateProcess failed" << std::endl;
			return;
		}

		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);

		CloseHandle(hChildStdoutWr);
		CloseHandle(hChildStdinRd);

		int fd_hChildStdoutRdDup = _open_osfhandle((long)(hChildStdoutRd), O_RDONLY);
		FILE *filein = _fdopen( fd_hChildStdoutRdDup, "r" );

		if(!filein)
		{
			errno_t err;
			_get_errno( &err );
			Aqsis::log() << error << "Unable to open input stream for \"RunProgram\" error is " <<  err << std::endl;
			return;
		}

		// Store the handles.
		(it->second)->hChildStdinWrDup = hChildStdinWr;
		(it->second)->hChildStdoutRdDup = hChildStdoutRd;
		(it->second)->fChildStdoutRd = filein;

		// Proc seems to be valid.
		run_proc->m_valid = true;
	}
	else
	{
		if( !(it->second)->m_valid )
			return;
	}

	// Build the procedural arguments into a string and write them out to the 
	// stdin pipe for the procedural to read.
	std::stringstream args;
	args << std::setiosflags( std::ios_base::fixed ) << detail << " " << ((char**)data)[1] << std::endl;
	DWORD bytesWritten;
	WriteFile( it->second->hChildStdinWrDup, args.str().c_str(), args.str().size(), &bytesWritten, NULL);
	//BOOL result = FlushFileBuffers( it->second->hChildStdinWrDup );

	CqRibBinaryDecoder *decoder;
	decoder = new CqRibBinaryDecoder( it->second->fChildStdoutRd, 1);

	// Parse the resulting block of RIB.
	CqString strRealName( ((char**)data)[0] );
	CqRIBParserState currstate = librib::GetParserState();

	if( currstate.m_pParseCallbackInterface == NULL )
		currstate.m_pParseCallbackInterface = new librib2ri::Engine;

	librib::ParseOpenStream( decoder, strRealName.c_str(), *(currstate.m_pParseCallbackInterface), *(currstate.m_pParseErrorStream), (RtArchiveCallback) RI_NULL );

	librib::SetParserState( currstate );

	delete( decoder );

	STATS_INC( GEO_prc_created_prp );

	return;
}


#endif
