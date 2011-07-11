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

#include "procedural.h"

#include <cstdio>
#include <cstring>
#include <list>

#include <boost/tokenizer.hpp>

#include "renderer.h"
#include <aqsis/util/file.h>
#include <aqsis/util/plugins.h>
#include <aqsis/core/corecontext.h>

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
		CqRiProceduralPlugin(CqString& dsoName)
			: m_ppvfcts(0),
			m_pvfctpvf(0),
			m_pvfctpv(0),
			m_ppriv(0),
			m_handle(0),
			m_bIsValid(false),
			m_Error()
		{
			CqString strConver("ConvertParameters");
			CqString strSubdivide("Subdivide");
			CqString strFree("Free");

			boost::filesystem::path dsoPath = QGetRenderContext()->poptCurrent()
				->findRiFileNothrow(dsoName, "procedural");
			if(dsoPath.empty())
			{
				m_Error = CqString( "Cannot find Procedural DSO for \"" )
				          + dsoName
				          + CqString ("\" in current searchpath");
				return;
			}
			CqString strRealName = native(dsoPath);
			
            try
            {
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
            }
            catch(XqPluginError& e)
            {
                Aqsis::log() << error << e << std::endl;
            }
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
static std::list<boost::shared_ptr<CqRiProceduralPlugin> > ActiveProcDLList;





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
	boost::shared_ptr<CqRiProceduralPlugin> plugin(new CqRiProceduralPlugin(dsoname));

	if( !plugin->IsValid() )
	{
		dsoname = CqString( (( char** ) data)[0] ) + CqString(SHARED_LIBRARY_SUFFIX);
		plugin.reset(new CqRiProceduralPlugin(dsoname));

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


//------------------------------------------------------------------------------
// CqRunProgramRepository implementation
//
std::iostream* CqRunProgramRepository::find(const std::string& command)
{
	TqRunProgramMap::iterator pos = m_activeRunPrograms.find(command);
	if(pos == m_activeRunPrograms.end())
		return startNewRunProgram(command);
	else
	{
		std::iostream* pipe = pos->second.get();
		if(pipe && (pipe->fail() || pipe->eof()))
		{
			pos->second.reset();
			pipe = 0;
		}
		return pipe;
	}
}

/** \brief Split the given command line up into a set of tokens seperated with
 * white space.
 *
 * The splitting isn't very sophisticated, so whitespace cannot currently be
 * escaped.
 */
void CqRunProgramRepository::splitCommandLine(const std::string& command,
		std::vector<std::string>& argv)
{
	typedef boost::tokenizer<boost::char_separator<char> > TqTokenizer;
	TqTokenizer tokens(command, boost::char_separator<char>(" \t\n"));
	for(TqTokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i)
		argv.push_back(*i);
}

/** \brief Start a new child process for the given RunProgram command
 */
TqPopenStream* CqRunProgramRepository::startNewRunProgram(
		const std::string& command)
{
	// Get the program name and command line arguments.
	std::vector<std::string> argv;
	splitCommandLine(command, argv);
	if(argv.empty())
		AQSIS_THROW_XQERROR(XqValidation, EqE_BadToken, "program name not present");
	// Attempt to find the program in the procedural path
	std::string progName = native(QGetRenderContext()->poptCurrent()
		->findRiFileNothrow(argv[0], "procedural"));
	if(progName.empty())
	{
		progName = argv[0];
		Aqsis::log() << info
			<< "RiProcRunProgram: Could not find \"" << progName
			<< "\" in \"procedural\" searchpath, will rely on system path.\n";
	}
	try
	{
		// Attempt to open a pipe to the new procedural.
		TqPopenStreamPtr newPipe(new TqPopenStream(progName, argv));
		newPipe->exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
		m_activeRunPrograms.insert(std::make_pair(command, newPipe));
		return newPipe.get();
	}
	catch(XqEnvironment& e)
	{
		// Install a null pointer to indicate that we shouldn't try to run this
		// procedural again, and rethrow.
		m_activeRunPrograms.insert(std::make_pair(command, TqPopenStreamPtr()));
		AQSIS_THROW_XQERROR(XqEnvironment, e.code(),
			"error starting runprogram [" << command << "] : " << e.what() );
	}
}


//------------------------------------------------------------------------------

// Global runprogram repository.
// TODO: Make this a member of CqRenderer, making sure that runprogram file
// handles and processes get correctly closed at destruction time.
static CqRunProgramRepository g_activeRunPrograms;

extern "C" RtVoid	RiProcRunProgram( RtPointer data, RtFloat detail )
{
	try
	{
		char** args = reinterpret_cast<char**>(data);
		// Get a pipe connected to the procedural
		std::string command = args[0];
		std::iostream* pipe = g_activeRunPrograms.find(command);
		if(!pipe)
			return;
		try
		{
			// Push in the procedural arguments
			(*pipe) << detail << " " << args[1] << "\n" << std::flush;
			// And parse the resulting RIB from the pipe.
			cxxRenderContext()->parseRib(*pipe, ("[" + command + "]").c_str());
			STATS_INC( GEO_prc_created_prp );
		}
		catch(std::ios_base::failure& /*e*/)
		{
			Aqsis::log() << error << "RiProcRunProgram: Broken pipe for RunProgram ["
				<< command << "]  (premature exit?)\n";
		}
	}
	// TODO: Replace the following catches with a catch guard.
	catch(const XqValidation& e)
	{
		QGetRenderContext()->pErrorHandler()(e.code(), RIE_ERROR,
				const_cast<char*>(e.what()));
	}
	catch(const XqException& e)
	{
		QGetRenderContext()->pErrorHandler()(e.code(), RIE_ERROR,
				const_cast<char*>(e.what()));
	}
	catch(const std::exception& e)
	{
		QGetRenderContext()->pErrorHandler()(RIE_BUG, RIE_SEVERE,
				const_cast<char*>(e.what()));
	}
	catch(...)
	{
		QGetRenderContext()->pErrorHandler()(RIE_BUG, RIE_SEVERE,
				const_cast<char*>("unknown exception encountered"));
	}
}

