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

/**
        \file
        \brief Implements the classes and support structures for 
                handling RenderMan Procedural primitives.
        \author Jonathan Merritt (j.merritt@pgrad.unimelb.edu.au)
*/

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
#include "share.h"

#ifdef AQSIS_SYSTEM_WIN32
#include <io.h>
#else
#include <unistd.h>
#endif



START_NAMESPACE( Aqsis )


/**
 * CqProcedural constructor.
 */
CqProcedural::CqProcedural() : CqSurface()
{
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
	ADDREF( m_pconStored );
}




TqInt CqProcedural::Split( std::vector<CqBasicSurface*>& aSplits )
{
	// Store current context, set current context to the stored one
	CqModeBlock *pconSave = QGetRenderContext()->pconCurrent( m_pconStored );

	CqBound bound = m_Bound;
	bound.Transform(QGetRenderContext()->matSpaceToSpace("camera", "raster"));
	float detail = ( bound.vecMax().x() - bound.vecMin().x() ) * ( bound.vecMax().y() - bound.vecMin().y() );
	//std::cout << "detail: " << detail << std::endl;

	// Call the procedural secific Split()
	RiAttributeBegin();

	m_pSubdivFunc(m_pData, detail);

	RiAttributeEnd();

	// restore saved context
	QGetRenderContext()->pconCurrent( pconSave );
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
	RELEASEREF( m_pconStored );
	if( m_pFreeFunc ) m_pFreeFunc( m_pData );
}


//----------------------------------------------------------------------
// RiProcFree()
//
extern "C" RtVoid	RiProcFree( RtPointer data )
{
	free(data);
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

        	CqRiFile        fileDSO( strDSOName.c_str(), "procedure" );
		m_bIsValid = false ;

	        if ( !fileDSO.IsValid() )
		{
			m_Error = CqString( "Cannot find Procedural DSO for " )
			 + strDSOName 
			 + CqString (" in current searchpath");

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
//----------------------------------------------------------------------
// RiProcDynamicLoad() subdivide function
//
extern "C" RtVoid	RiProcDynamicLoad( RtPointer data, RtFloat detail )
{
	CqString dsoname = CqString( (( char** ) data)[0] ) + CqString(SHARED_LIBRARY_SUFFIX);
	CqRiProceduralPlugin *plugin = new CqRiProceduralPlugin( dsoname );
	
	if( !plugin->IsValid() )
	{
		QGetRenderContext() ->Logger()->error( CqString("Problem loading Procedural DSO:") );
		QGetRenderContext() ->Logger()->error( CqString(plugin->Error()) );
		return;
	}

	plugin->ConvertParameters( (( char** ) data)[1] );
	plugin->Subdivide( detail );
	plugin->Free();

	ActiveProcDLList.push_back( plugin ); 
} 



//----------------------------------------------------------------------
// RiProcDelayedReadArchive()
//
extern "C" RtVoid	RiProcDelayedReadArchive( RtPointer data, RtFloat detail )
{
	RiReadArchive( (RtToken) ((char**) data)[0], NULL );
}


//----------------------------------------------------------------------
/* RiProcRunProgram()
 * Your program must writes its output to a pipe. Open this
 * pipe with read text attribute so that we can read it 
 * like a text file. 
 */

// TODO: This is far from ideal, we need to parse directly from the popene'd 
// process.

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
#ifdef AQSIS_SYSTEM_WIN32
#else
	std::map<std::string, CqRiProceduralRunProgram*>::iterator it;

	it = ActiveProcRP.find(CqString( ((char**)data)[0] )) ;
	if( it == ActiveProcRP.end() )
	{
		// We don't have an active RunProgram for the specifed
		// program. We need to try and fork a new one
		CqRiProceduralRunProgram *run_proc = new CqRiProceduralRunProgram;
		pipe( run_proc->fd_in ) ;
		pipe( run_proc->fd_out ) ;

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
			// Split up the RunProgram program name string
			int arg_count = 1;
			char *arg_values[32];
			arg_values[0] = ((char**)data)[0] ;
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
			dup( run_proc->fd_out[0] );
//			setvbuf( stdin, NULL, _IONBF, 0 );
			close( STDOUT_FILENO );
			dup( run_proc->fd_in[1] );
//			setvbuf( stdout, NULL, _IONBF, 0 );

			// We should use the procedurals searchpath here
			execvp( arg_values[0], arg_values );
		 };
	};

	FILE *fileout = (it->second)->out;
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
	decoder = new CqRibBinaryDecoder( filein, 1);

	// Parse the resulting block of RIB.
	CqString strRealName( ((char**)data)[0] );
	CqRIBParserState currstate = librib::GetParserState();

	if( currstate.m_pParseCallbackInterface == NULL ) currstate.m_pParseCallbackInterface = new librib2ri::Engine;

	librib::ParseOpenStream( decoder, strRealName.c_str(), *(currstate.m_pParseCallbackInterface), *(currstate.m_pParseErrorStream), (RtArchiveCallback) RI_NULL );

	librib::SetParserState( currstate );

	delete( decoder );

#endif
	return;
}

END_NAMESPACE( Aqsis )

