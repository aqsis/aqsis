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
		\brief Implements the base CqRenderer class which is the central core of the rendering main loop.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"

#include	<strstream>

#include	<time.h>

#include	"imagebuffer.h"
#include	"lights.h"
#include	"renderer.h"
#include	"shaders.h"
#include	"nurbs.h"
#include	"render.h"
#include	"transform.h"
#include	"rifile.h"
#include	"texturemap.h"
#include	"shadervm.h"
#include	"log.h"


START_NAMESPACE( Aqsis )

extern IqDDManager* CreateDisplayDriverManager();

static CqShaderRegister * pOShaderRegister = NULL;

CqRenderer* pCurrRenderer = 0;


CqOptions	goptDefault;					///< Default options.

static TqUlong ohash = 0; //< == "object"
static TqUlong shash = 0; //< == "shader"
static TqUlong chash = 0; //< == "camera"
static TqUlong cuhash = 0; //< == "current"

static CqMatrix oldkey[2];  //< to eliminate Inverse(), Transpose() matrix ops.
static CqMatrix oldresult[2];

//---------------------------------------------------------------------
/** Default constructor for the main renderer class. Initialises current state.
 */

CqRenderer::CqRenderer() :
		m_pImageBuffer( 0 ),
		m_Mode( RenderMode_Image ),
		m_fSaveGPrims( TqFalse ),
		m_OutputDataOffset(7),		// Cs, Os, z
		m_OutputDataTotalSize(7)	// Cs, Os, z
{
	m_pconCurrent = 0;
	m_pImageBuffer = new	CqImageBuffer();

	// Initialise the array of coordinate systems.
	m_aCoordSystems.resize( CoordSystem_Last );

	m_aCoordSystems[ CoordSystem_Camera ].m_strName = "__camera__";
	m_aCoordSystems[ CoordSystem_Current ].m_strName = "__current__";
	m_aCoordSystems[ CoordSystem_World ].m_strName = "world";
	m_aCoordSystems[ CoordSystem_Screen ].m_strName = "screen";
	m_aCoordSystems[ CoordSystem_NDC ].m_strName = "NDC";
	m_aCoordSystems[ CoordSystem_Raster ].m_strName = "raster";

	m_aCoordSystems[ CoordSystem_Camera ].m_hash = CqParameter::hash( "__camera__" );
	m_aCoordSystems[ CoordSystem_Current ].m_hash = CqParameter::hash( "__current__" );
	m_aCoordSystems[ CoordSystem_World ].m_hash = CqParameter::hash( "world" );
	m_aCoordSystems[ CoordSystem_Screen ].m_hash = CqParameter::hash( "screen" );
	m_aCoordSystems[ CoordSystem_NDC ].m_hash = CqParameter::hash( "NDC" );
	m_aCoordSystems[ CoordSystem_Raster ].m_hash = CqParameter::hash( "raster" );

	m_pDDManager = CreateDisplayDriverManager();
	m_pDDManager->Initialise();

	// Set up DoF stuff for pinhole lens ( i.e. no DoF )
	m_depthOfFieldData[0] = FLT_MAX;
	m_depthOfFieldData[1] = 0.0f;
	m_depthOfFieldData[2] = 0.0f;

	// Create a transform matrix
	m_depthOfFieldTMatrix = matVSpaceToSpace( "__camera__", "raster" );

	// Get the hash keys for object, shader, camera keywords.
	if ( ohash == 0 ) ohash = CqParameter::hash( "object" );
	if ( shash == 0 ) shash = CqParameter::hash( "shader" );
	if ( chash == 0 ) chash = CqParameter::hash( "camera" );
	if ( cuhash == 0 ) cuhash = CqParameter::hash( "current" );

	// Create the log on the heap, delete it in the destructor
	m_theLog = new CqLog;
	m_theTable = new CqMessageTable;

	// Connect the message table with the log
	m_theLog->setMessageTable( m_theTable );
}

//---------------------------------------------------------------------
/** Destructor
 */

CqRenderer::~CqRenderer()
{
	// Delete the current context, should be main, unless render has been aborted.
	while ( m_pconCurrent != 0 )
	{
		CqModeBlock * pconParent = m_pconCurrent->pconParent();
		m_pconCurrent->Release();
		m_pconCurrent = pconParent;
	}
	if ( m_pImageBuffer )
	{
		m_pImageBuffer->Release();
		m_pImageBuffer = 0;
	}
	FlushShaders();

	// Shutdown the shaderVM.
	CqShaderVM::ShutdownShaderEngine();

	// Close down the Display device manager.
	m_pDDManager->Shutdown();

	// Delete log & message table
	delete m_theLog;
	delete m_theTable;
}


//---------------------------------------------------------------------
/** Create a new main context, called from within RiBegin(), error if not first
 * context created.  If first, create with this as the parent.
 */

CqModeBlock*	CqRenderer::BeginMainModeBlock()
{
	if ( m_pconCurrent == 0 )
	{
		m_pconCurrent = new CqMainModeBlock();
		m_pconCurrent->AddRef();
		return ( m_pconCurrent );
	}
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Create a new Frame context, should only be called when the current
 * context is a Main context, but the internal context handling deals
 * with it so I don't need to worry.
 */

CqModeBlock*	CqRenderer::BeginFrameModeBlock()
{
	if ( m_pconCurrent != 0 )
	{
		CqModeBlock * pconNew = m_pconCurrent->BeginFrameModeBlock();
		if ( pconNew != 0 )
		{
			pconNew->AddRef();
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return ( 0 );
	}
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Create a new world context, again the internal context handling deals
 * with invalid calls.
 */

CqModeBlock*	CqRenderer::BeginWorldModeBlock()
{
	if ( m_pconCurrent != 0 )
	{
		CqModeBlock * pconNew = m_pconCurrent->BeginWorldModeBlock();
		if ( pconNew != 0 )
		{
			pconNew->AddRef();
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return ( 0 );
	}
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Create a new attribute context.
 */

CqModeBlock*	CqRenderer::BeginAttributeModeBlock()
{
	if ( m_pconCurrent != 0 )
	{
		CqModeBlock * pconNew = m_pconCurrent->BeginAttributeModeBlock();
		if ( pconNew != 0 )
		{
			pconNew->AddRef();
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return ( 0 );
	}
	else
		return ( 0 );
}



//---------------------------------------------------------------------
/** Create a new transform context.
 */

CqModeBlock*	CqRenderer::BeginTransformModeBlock()
{
	if ( m_pconCurrent != 0 )
	{
		CqModeBlock * pconNew = m_pconCurrent->BeginTransformModeBlock();
		if ( pconNew != 0 )
		{
			pconNew->AddRef();
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return ( 0 );
	}
	else
		return ( 0 );
}



//---------------------------------------------------------------------
/** Create a new solid context.
 */

CqModeBlock*	CqRenderer::BeginSolidModeBlock( CqString& type )
{
	if ( m_pconCurrent != 0 )
	{
		CqModeBlock * pconNew = m_pconCurrent->BeginSolidModeBlock( type );
		if ( pconNew != 0 )
		{
			pconNew->AddRef();
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return ( 0 );
	}
	else
		return ( 0 );
}



//---------------------------------------------------------------------
/** Create a new object context.
 */

CqModeBlock*	CqRenderer::BeginObjectModeBlock()
{
	if ( m_pconCurrent != 0 )
	{
		CqModeBlock * pconNew = m_pconCurrent->BeginObjectModeBlock();
		if ( pconNew != 0 )
		{
			pconNew->AddRef();
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return ( 0 );
	}
	else
		return ( 0 );
}



//---------------------------------------------------------------------
/** Create a new motion context.
 */

CqModeBlock*	CqRenderer::BeginMotionModeBlock( TqInt N, TqFloat times[] )
{
	if ( m_pconCurrent != 0 )
	{
		CqModeBlock * pconNew = m_pconCurrent->BeginMotionModeBlock( N, times );
		if ( pconNew != 0 )
		{
			pconNew->AddRef();
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return ( 0 );
	}
	else
		return ( 0 );
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a main context.
 */

void	CqRenderer::EndMainModeBlock()
{
	if ( m_pconCurrent != 0 )
	{
		CqModeBlock * pconParent = m_pconCurrent->pconParent();
		m_pconCurrent->EndMainModeBlock();
		m_pconCurrent->Release();
		m_pconCurrent = pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a frame context.
 */

void	CqRenderer::EndFrameModeBlock()
{
	if ( m_pconCurrent != 0 )
	{
		CqModeBlock * pconParent = m_pconCurrent->pconParent();
		m_pconCurrent->EndFrameModeBlock();
		m_pconCurrent->Release();
		m_pconCurrent = pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a world context.
 */

void	CqRenderer::EndWorldModeBlock()
{
	if ( m_pconCurrent != 0 )
	{
		CqModeBlock * pconParent = m_pconCurrent->pconParent();
		m_pconCurrent->EndWorldModeBlock();
		m_pconCurrent->Release();
		m_pconCurrent = pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a attribute context.
 */

void	CqRenderer::EndAttributeModeBlock()
{
	if ( m_pconCurrent != 0 )
	{
		CqModeBlock * pconParent = m_pconCurrent->pconParent();
		m_pconCurrent->EndAttributeModeBlock();
		m_pconCurrent->Release();
		m_pconCurrent = pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a transform context.
 */

void	CqRenderer::EndTransformModeBlock()
{
	if ( m_pconCurrent != 0 )
	{
		CqModeBlock * pconParent = m_pconCurrent->pconParent();
		// Copy the current state of the attributes UP the stack as a TransformBegin/End doesn't store them
		pconParent->m_pattrCurrent = m_pconCurrent->m_pattrCurrent;
		m_pconCurrent->EndTransformModeBlock();
		m_pconCurrent->Release();
		m_pconCurrent = pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a solid context.
 */

void	CqRenderer::EndSolidModeBlock()
{
	if ( m_pconCurrent != 0 )
	{
		CqModeBlock * pconParent = m_pconCurrent->pconParent();
		m_pconCurrent->EndSolidModeBlock();
		m_pconCurrent->Release();
		m_pconCurrent = pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a object context.
 */

void	CqRenderer::EndObjectModeBlock()
{
	if ( m_pconCurrent != 0 )
	{
		CqModeBlock * pconParent = m_pconCurrent->pconParent();
		m_pconCurrent->EndObjectModeBlock();
		m_pconCurrent->Release();
		m_pconCurrent = pconParent;
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a motion context.
 */

void	CqRenderer::EndMotionModeBlock()
{
	if ( m_pconCurrent != 0 )
	{
		CqModeBlock * pconParent = m_pconCurrent->pconParent();
		// Copy the current state of the attributes UP the stack as a TransformBegin/End doesn't store them
		pconParent->m_pattrCurrent = m_pconCurrent->m_pattrCurrent;
		pconParent->m_ptransCurrent = m_pconCurrent->m_ptransCurrent;
		m_pconCurrent->EndMotionModeBlock();
		m_pconCurrent->Release();
		m_pconCurrent = pconParent;
	}
}


//----------------------------------------------------------------------
/** Get the current shutter time, always returns 0.0 unless within a motion block,
 * when it returns the appropriate shutter time.
 */

TqFloat	CqRenderer::Time() const
{
	if ( m_pconCurrent != 0 )
		return ( m_pconCurrent->Time() );
	else
		return ( 0 );
}

//----------------------------------------------------------------------
/** Advance the current shutter time, only valid within motion blocks.
 */

void CqRenderer::AdvanceTime()
{
	if ( m_pconCurrent != 0 )
		m_pconCurrent->AdvanceTime();
}


//----------------------------------------------------------------------
/** Return a reference to the current options.
 */

CqOptions& CqRenderer::optCurrent() const
{
	if ( m_pconCurrent != 0 )
		return ( m_pconCurrent->optCurrent() );
	else
		return ( goptDefault );
}


//----------------------------------------------------------------------
/** Return a pointer to the current attributes.
 */

const CqAttributes* CqRenderer::pattrCurrent()
{
	if ( m_pconCurrent != 0 )
		return ( m_pconCurrent->pattrCurrent() );
	else
		return ( &m_attrDefault );
}


//----------------------------------------------------------------------
/** Return a writable pointer to the current attributes.
 */

CqAttributes* CqRenderer::pattrWriteCurrent()
{
	if ( m_pconCurrent != 0 )
		return ( m_pconCurrent->pattrWriteCurrent() );
	else
		return ( &m_attrDefault );
}


//----------------------------------------------------------------------
/** Return a pointer to the current transform.
 */

const CqTransform* CqRenderer::ptransCurrent()
{
	if ( m_pconCurrent != 0 )
		return ( m_pconCurrent->ptransCurrent() );
	else
		return ( &m_transDefault );
}


//----------------------------------------------------------------------
/** Return a writable pointer to the current transform.
 */

CqTransform* CqRenderer::ptransWriteCurrent()
{
	if ( m_pconCurrent != 0 )
		return ( m_pconCurrent->ptransWriteCurrent() );
	else
		return ( &m_transDefault );
}


//----------------------------------------------------------------------
/** Render all surface in the current list to the image buffer.
 */

void CqRenderer::RenderWorld()
{
	// Check we have a valid Image buffer
	if ( pImage() == 0 )
		SetImage( new CqImageBuffer );

	// \debug:
	std::map<std::string, SqOutputDataEntry>::iterator entry;
	for( entry = m_OutputDataEntries.begin(); entry != m_OutputDataEntries.end(); entry++ )
	{
		std::cout << entry->first.c_str() << " - " << entry->second.m_Offset << "," << entry->second.m_NumSamples << std::endl;
	}

	m_pDDManager->OpenDisplays();

	pImage() ->RenderImage();

	m_pDDManager->CloseDisplays();
}



//----------------------------------------------------------------------
/** Quit rendering at the next opportunity.
 */

void CqRenderer::Quit()
{
	if ( m_pImageBuffer )
	{
		// Ask the image buffer to quit.
		m_pImageBuffer->Quit();
	}
}


//----------------------------------------------------------------------
/** Initialise the renderer.
 */

void CqRenderer::Initialise()
{
	ClearSymbolTable();
	FlushShaders();

	// Truncate the array of named coordinate systems to just the standard ones.
	m_aCoordSystems.resize( CoordSystem_Last );

	// Clear the output data entries
	m_OutputDataEntries.clear();
	m_OutputDataOffset = 7;		// Cs, Os, z
	m_OutputDataTotalSize = 7;	// Cs, Os, z
}


//----------------------------------------------------------------------
/** Get the matrix to convert between the specified coordinate systems.
 */


CqMatrix	CqRenderer::matSpaceToSpace( const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld, const CqMatrix& matObjectToWorld, TqFloat time )
{
	CqMatrix	matResult, matA, matB;
	TqUlong fhash, thash;


	// Get the hash keys for From,To spaces
	fhash = CqParameter::hash( strFrom );
	thash = CqParameter::hash( strTo );

	// Get the two component matrices.
	// First check for special cases.
	if ( fhash == ohash ) matA = matObjectToWorld;
	else if ( fhash == shash ) matA = matShaderToWorld;
	else if ( ( fhash == chash ) || ( fhash == cuhash ) )
		matA = m_transCamera.GetMotionObjectInterpolated( time ).Inverse();
	else
	{
		WhichMatToWorld( matA, fhash );
	}


	if ( thash == ohash ) matB = matObjectToWorld.Inverse();
	else if ( thash == shash ) matB = matShaderToWorld.Inverse();
	else if ( ( thash == chash ) || ( thash == cuhash ) )
		matB = m_transCamera.GetMotionObjectInterpolated( time );
	else
	{
		WhichMatWorldTo( matB, thash );
	}

	matResult = matB * matA;

	return ( matResult );
}



//----------------------------------------------------------------------
/** Get the matrix to convert vectors between the specified coordinate systems.
 */

CqMatrix	CqRenderer::matVSpaceToSpace( const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld, const CqMatrix& matObjectToWorld, TqFloat time )
{
	CqMatrix	matResult, matA, matB;

	TqUlong fhash, thash;

	// Get the hash keys for From,To spaces
	fhash = CqParameter::hash( strFrom );
	thash = CqParameter::hash( strTo );

	// Get the two component matrices.
	// First check for special cases.
	if ( fhash == ohash ) matA = matObjectToWorld;
	else if ( fhash == shash ) matA = matShaderToWorld;
	else if ( ( fhash == chash ) || ( fhash == cuhash ) )
		matA = m_transCamera.GetMotionObjectInterpolated( time ).Inverse();
	else
	{
		WhichMatToWorld ( matA, fhash );
	}

	if ( thash == ohash ) matB = matObjectToWorld.Inverse();
	else if ( thash == shash ) matB = matShaderToWorld.Inverse();
	else if ( ( thash == chash ) || ( thash == cuhash ) )
		matB = m_transCamera.GetMotionObjectInterpolated( time );
	else
	{
		WhichMatWorldTo ( matB, thash );
	}

	matResult = matB * matA;

	

	if (memcmp((void *) oldkey[0].pElements(), (void *) matResult.pElements(), sizeof(TqFloat) * 16) != 0)
	{
		oldkey[0] = matResult;
		matResult[ 3 ][ 0 ] = matResult[ 3 ][ 1 ] = matResult[ 3 ][ 2 ] = matResult[ 0 ][ 3 ] = matResult[ 1 ][ 3 ] = matResult[ 2 ][ 3 ] = 0.0;
		matResult[ 3 ][ 3 ] = 1.0;
		oldresult[0] = matResult;

	} else
	{
		return oldresult[0];
	}
	return ( matResult );
}


//----------------------------------------------------------------------
/** Get the matrix to convert normals between the specified coordinate systems.
 */

CqMatrix	CqRenderer::matNSpaceToSpace( const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld, const CqMatrix& matObjectToWorld, TqFloat time )
{
	CqMatrix	matResult, matA, matB;
	
	TqUlong fhash, thash;

	// Get the hash keys for From,To spaces
	fhash = CqParameter::hash( strFrom );
	thash = CqParameter::hash( strTo );

	// Get the two component matrices.
	// First check for special cases.
	if ( fhash == ohash ) matA = matObjectToWorld;
	else if ( fhash == shash ) matA = matShaderToWorld;
	else if ( ( fhash == chash ) || ( fhash == cuhash ) )
		matA = m_transCamera.GetMotionObjectInterpolated( time ).Inverse();
	else
	{
		WhichMatToWorld ( matA, fhash );
	}

	if ( thash == ohash ) matB = matObjectToWorld.Inverse();
	else if ( thash == shash ) matB = matShaderToWorld.Inverse();
	else if ( ( thash == chash ) || ( thash == cuhash ) )
		matB = m_transCamera.GetMotionObjectInterpolated( time );
	else
	{
		WhichMatWorldTo ( matB, thash );
	}


	matResult = matB * matA;
	if (memcmp((void *) oldkey[1].pElements(), (void *) matResult.pElements(), sizeof(TqFloat) * 16) != 0)
	{
		oldkey[1] = matResult;
		matResult[ 3 ][ 0 ] = matResult[ 3 ][ 1 ] = matResult[ 3 ][ 2 ] = matResult[ 0 ][ 3 ] = matResult[ 1 ][ 3 ] = matResult[ 2 ][ 3 ] = 0.0;
		matResult[ 3 ][ 3 ] = 1.0;
		matResult.Inverse();
		matResult.Transpose();
		oldresult[1] = matResult;

	} else
	{
		return oldresult[1];
	}

	return ( matResult );
}


const	TqFloat*	CqRenderer::GetFloatOption( const char* strName, const char* strParam ) const
{
	return ( optCurrent().GetFloatOption( strName, strParam ) );
}

const	TqInt*	CqRenderer::GetIntegerOption( const char* strName, const char* strParam ) const
{
	return ( optCurrent().GetIntegerOption( strName, strParam ) );
}

const	CqString*	CqRenderer::GetStringOption( const char* strName, const char* strParam ) const
{
	return ( optCurrent().GetStringOption( strName, strParam ) );
}

const	CqVector3D*	CqRenderer::GetPointOption( const char* strName, const char* strParam ) const
{
	return ( optCurrent().GetPointOption( strName, strParam ) );
}

const	CqColor*	CqRenderer::GetColorOption( const char* strName, const char* strParam ) const
{
	return ( optCurrent().GetColorOption( strName, strParam ) );
}


TqFloat*	CqRenderer::GetFloatOptionWrite( const char* strName, const char* strParam )
{
	return ( optCurrent().GetFloatOptionWrite( strName, strParam ) );
}

TqInt*	CqRenderer::GetIntegerOptionWrite( const char* strName, const char* strParam )
{
	return ( optCurrent().GetIntegerOptionWrite( strName, strParam ) );
}

CqString*	CqRenderer::GetStringOptionWrite( const char* strName, const char* strParam )
{
	return ( optCurrent().GetStringOptionWrite( strName, strParam ) );
}

CqVector3D*	CqRenderer::GetPointOptionWrite( const char* strName, const char* strParam )
{
	return ( optCurrent().GetPointOptionWrite( strName, strParam ) );
}

CqColor*	CqRenderer::GetColorOptionWrite( const char* strName, const char* strParam )
{
	return ( optCurrent().GetColorOptionWrite( strName, strParam ) );
}


//----------------------------------------------------------------------
/** Store the named coordinate system in the array of named coordinate systems, overwrite any existing
 * with the same name. Returns TqTrue if system already exists.
 */

TqBool	CqRenderer::SetCoordSystem( const char* strName, const CqMatrix& matToWorld )
{
	// Search for the same named system in the current list.
	TqLong hash = CqParameter::hash( strName );
	for ( TqUint i = 0; i < m_aCoordSystems.size(); i++ )
	{
		if ( m_aCoordSystems[ i ].m_hash == hash )
		{
			m_aCoordSystems[ i ].m_matToWorld = matToWorld;
			m_aCoordSystems[ i ].m_matWorldTo = matToWorld.Inverse();
			return ( TqTrue );
		}
	}

	// If we got here, it didn't exists.
	m_aCoordSystems.push_back( SqCoordSys( strName, matToWorld, matToWorld.Inverse() ) );
	return ( TqFalse );
}


//----------------------------------------------------------------------
/** Find a parameter type declaration and return it.
 * \param strDecl Character pointer to the name of the declaration to find.
 */

SqParameterDeclaration CqRenderer::FindParameterDecl( const char* strDecl )
{
	TqInt Count = 1;
	CqString strName( "" );
	EqVariableType ILType = type_invalid;
	EqVariableClass ILClass = class_invalid;
	TqBool bArray = TqFalse;

	// First check if the declaration has embedded type information.
	CqString strLocalDecl( strDecl );
	TqInt i;
	/// \note Go backwards through the type names to make sure facevarying is matched before varying.
	for ( i = gcVariableClassNames - 1; i >= 0; i-- )
	{
		if ( strLocalDecl.find( gVariableClassNames[ i ] ) != CqString::npos )
		{
			ILClass = static_cast< EqVariableClass > ( i );
			break;
		}
	}

	/// \note Go backwards through the type names to make sure hpoint is matched before point.
	for ( i = gcVariableTypeNames - 1; i >= 0; i-- )
	{
		if ( strLocalDecl.find( gVariableTypeNames[ i ] ) != CqString::npos )
		{
			ILType = static_cast< EqVariableType > ( i );
			break;
		}
	}

	// Now search for an array specifier.
	TqUint s, e;
	if ( ( s = strLocalDecl.find( '[' ) ) != CqString::npos )
	{
		if ( ( e = strLocalDecl.find( ']' ) ) != CqString::npos && e > s )
		{
			Count = static_cast<TqInt>( atoi( strLocalDecl.substr( s + 1, e - ( s + 1 ) ).c_str() ) );
			bArray = TqTrue ;
		}
	}

	// Copy the token to the name.
	s = strLocalDecl.find_last_of( ' ' );
	if ( s != CqString::npos ) strName = strLocalDecl.substr( s + 1 );
	else	strName = strLocalDecl;

	if ( ILType != type_invalid )
	{
		// Default to uniform if no class specified
		if ( ILClass == class_invalid )
			ILClass = class_uniform ;

		SqParameterDeclaration Decl;
		Decl.m_strName = strName;
		Decl.m_Count = Count;
		Decl.m_Type = ILType;
		Decl.m_Class = ILClass;
		Decl.m_strSpace = "";

		// Get the creation function.
		switch ( ILClass )
		{
				case class_constant:
				{
					if ( bArray )
						Decl.m_pCreate = gVariableCreateFuncsConstantArray[ ILType ];
					else
						Decl.m_pCreate = gVariableCreateFuncsConstant[ ILType ];
				}
				break;

				case class_uniform:
				{
					if ( bArray )
						Decl.m_pCreate = gVariableCreateFuncsUniformArray[ ILType ];
					else
						Decl.m_pCreate = gVariableCreateFuncsUniform[ ILType ];
				}
				break;

				case class_varying:
				{
					if ( bArray )
						Decl.m_pCreate = gVariableCreateFuncsVaryingArray[ ILType ];
					else
						Decl.m_pCreate = gVariableCreateFuncsVarying[ ILType ];
				}
				break;

				case class_vertex:
				{
					if ( bArray )
						Decl.m_pCreate = gVariableCreateFuncsVertexArray[ ILType ];
					else
						Decl.m_pCreate = gVariableCreateFuncsVertex[ ILType ];
				}
				break;

				case class_facevarying:
				{
					if ( bArray )
						Decl.m_pCreate = gVariableCreateFuncsFaceVaryingArray[ ILType ];
					else
						Decl.m_pCreate = gVariableCreateFuncsFaceVarying[ ILType ];
				}
				break;
		}
		return ( Decl );
	}

	strName = strDecl;
	// Search the local parameter declaration list.
	std::vector<SqParameterDeclaration>::const_iterator is;
	std::vector<SqParameterDeclaration>::const_iterator end = m_Symbols.end();
	TqLong hash = CqParameter::hash( strDecl );
	for ( is = m_Symbols.begin(); is != end ; is++ )
	{
		TqLong hash2 = CqParameter::hash( is->m_strName.c_str() );
		if ( hash == hash2 )
			return ( *is );
	}
	return ( SqParameterDeclaration( "", type_invalid, class_invalid, 0, 0, "" ) );
}


//----------------------------------------------------------------------
/** Add a parameter type declaration to the local declarations.
 * \param strName Character pointer to parameter name.
 * \param strType Character pointer to string containing the type identifier.
 */

void CqRenderer::AddParameterDecl( const char* strName, const char* strType )
{
	CqString strDecl( strType );
	strDecl += " ";
	strDecl += strName;
	SqParameterDeclaration Decl = FindParameterDecl( strDecl.c_str() );

	// Put new declaration at the top to make it take priority over pervious
	m_Symbols.insert( m_Symbols.begin(), Decl );
}


//---------------------------------------------------------------------
/** Register a shader of the specified type with the specified name.
 */

void CqRenderer::RegisterShader( const char* strName, EqShaderType type, IqShader* pShader )
{
	assert( pShader );
	m_Shaders.LinkLast( new CqShaderRegister( strName, type, pShader ) );
}


//---------------------------------------------------------------------
/** Find a shader of the specified type with the specified name.
 */

CqShaderRegister* CqRenderer::FindShader( const char* strName, EqShaderType type )
{
	// Search the register list.
	CqShaderRegister * pShaderRegister = m_Shaders.pFirst();



	while ( pShaderRegister )
	{
		if ( pShaderRegister->strName() == strName && pShaderRegister->Type() == type )
		{
			pOShaderRegister = pShaderRegister ;
			return ( pShaderRegister );
		}

		pShaderRegister = pShaderRegister->pNext();
	}
	return ( 0 );
}


//---------------------------------------------------------------------
/** Find a shader of the specified type with the specified name.
 * If not found, try and load one.
 */

IqShader* CqRenderer::CreateShader( const char* strName, EqShaderType type )
{
	CqShaderRegister * pReg = NULL;

	if ( pOShaderRegister && pOShaderRegister->strName() == strName && pOShaderRegister->Type() == type )
		pReg = pOShaderRegister;
	else
		pReg = FindShader( strName, type );
	if ( pReg != 0 )
	{
		IqShader * pShader = pReg->Create();
		RegisterShader( strName, type, pShader );
		return ( pShader );
	}
	else
	{
		// Search in the current directory first.
		CqString strFilename( strName );
		strFilename += RI_SHADER_EXTENSION;
		CqRiFile SLXFile( strFilename.c_str(), "shader" );
		if ( SLXFile.IsValid() )
		{
			CqShaderVM * pShader = new CqShaderVM();
			const CqString *poptDSOPath = QGetRenderContext()->optCurrent().GetStringOption( "searchpath","dsolibs" );
			pShader->SetDSOPath( poptDSOPath );
			pShader->SetLogger( m_theLog );
			pShader->SetstrName( strName );
			pShader->LoadProgram( SLXFile );
			RegisterShader( strName, type, pShader );
			return ( pShader );
		}
		else
		{
			if ( strcmp( strName, "null" ) != 0 )
			{
				CqString strError( "Shader \"" );
				strError += strName;
				strError += "\" not found";
				//strError.Format("Shader \"%s\" not found",strName.String());
				CqBasicError( ErrorID_FileNotFound, Severity_Normal, strError.c_str() );
			}
			if( type == Type_Surface )
			{
				CqShaderVM * pShader = new CqShaderVM();
				pShader->SetstrName( "null" );
				pShader->DefaultSurface();
				RegisterShader( strName, type, pShader );
				return ( pShader );
			}
			else
				return ( NULL );
		}
	}
}



//---------------------------------------------------------------------
/** Add a new requested display driver to the list.
 */

void CqRenderer::AddDisplayRequest( const TqChar* name, const TqChar* type, const TqChar* mode, TqInt compression, TqInt quality, TqInt modeID, TqInt dataOffset, TqInt dataSize )
{
	m_pDDManager->AddDisplay( name, type, mode, compression, quality, modeID, dataOffset, dataSize );
}



//---------------------------------------------------------------------
/** Clear the list of requested display drivers.
 */

void CqRenderer::ClearDisplayRequests()
{
	m_pDDManager->ClearDisplays();
}


void QSetRenderContext( CqRenderer* pRend )
{
	pCurrRenderer = pRend;
}

IqRenderer* QGetRenderContextI()
{
	return ( pCurrRenderer );
}


IqTextureMap* CqRenderer::GetTextureMap( const CqString& strFileName )
{
	return ( CqTextureMap::GetTextureMap( strFileName ) );
}

IqTextureMap* CqRenderer::GetEnvironmentMap( const CqString& strFileName )
{
	return ( CqTextureMap::GetEnvironmentMap( strFileName ) );
}

IqTextureMap* CqRenderer::GetShadowMap( const CqString& strFileName )
{
	return ( CqTextureMap::GetShadowMap( strFileName ) );
}

IqTextureMap* CqRenderer::GetLatLongMap( const CqString& strFileName )
{
	return ( CqTextureMap::GetLatLongMap( strFileName ) );
}


TqBool	CqRenderer::GetBasisMatrix( CqMatrix& matBasis, const CqString& name )
{
	RtBasis basis;
	if ( BasisFromName( &basis, name.c_str() ) )
	{
		matBasis = basis;
		return ( TqTrue );
	}
	else
		return ( TqFalse );
}

//---------------------------------------------------------------------
/** Which matrix will be used in ToWorld
 */
void CqRenderer::WhichMatToWorld( CqMatrix &matA, TqUlong thash )
{
	static TqInt awhich = 0;
	TqInt tmp = awhich;


	for ( ; awhich >= 0; awhich-- )
	{
		if ( m_aCoordSystems[ awhich ].m_hash == thash )
		{
			matA = m_aCoordSystems[ awhich ].m_matToWorld;
			return ;
		}
	}

	TqInt size = m_aCoordSystems.size() - 1;
	for ( awhich = size; awhich > tmp; awhich-- )
	{
		if ( m_aCoordSystems[ awhich ].m_hash == thash )
		{
			matA = m_aCoordSystems[ awhich ].m_matToWorld;
			break;
		}
	}
}

//---------------------------------------------------------------------
/** Which matrix will be used in WorldTo
 */

void CqRenderer::WhichMatWorldTo( CqMatrix &matB, TqUlong thash )
{
	static TqInt bwhich = 0;
	TqInt tmp = bwhich;


	for ( ; bwhich >= 0; bwhich-- )
	{
		if ( m_aCoordSystems[ bwhich ].m_hash == thash )
		{
			matB = m_aCoordSystems[ bwhich ].m_matWorldTo;
			return ;
		}
	}

	TqInt size = m_aCoordSystems.size() - 1;
	for ( bwhich = size; bwhich > tmp; bwhich-- )
	{
		if ( m_aCoordSystems[ bwhich ].m_hash == thash )
		{
			matB = m_aCoordSystems[ bwhich ].m_matWorldTo;
			break;
		}
	}
}


TqInt CqRenderer::RegisterOutputData( const char* name )
{
	TqInt offset;
	if( ( offset = OutputDataIndex( name ) ) != -1 )
		return(offset);

	SqParameterDeclaration Decl = FindParameterDecl( name );
	if( Decl.m_Type != type_invalid )
	{
		if( Decl.m_Count != 1 )
			throw("Error: Cannot use array as an output type");

		SqOutputDataEntry DataEntry;
		TqInt NumSamples = 0;
		switch( Decl.m_Type )
		{
			case type_float:
			case type_integer:
				NumSamples = 1;
				break;
			case type_point:
			case type_normal:
			case type_vector:
			case type_hpoint:
				NumSamples = 3;
				break;
			case type_color:
				// \note: Color is handled separately in case we ever support RiColorSamples
				NumSamples = 3;
				break;
			case type_matrix:
				NumSamples = 16;
				break;
			case type_string:
				throw("Error: String not valid as an output type");
				break;
		}
		
		DataEntry.m_Offset = m_OutputDataOffset;
		DataEntry.m_NumSamples = NumSamples;
		m_OutputDataOffset += NumSamples;
		m_OutputDataTotalSize += NumSamples;

		// Add the new entry to the map, using the Decl name as the key.
		m_OutputDataEntries[Decl.m_strName] = DataEntry;

		return( DataEntry.m_Offset );
	}	

	return( -1 );
}

TqInt CqRenderer::OutputDataIndex( const char* name )
{
	SqParameterDeclaration Decl = FindParameterDecl( name );
	if( Decl.m_Type != type_invalid )
	{
		std::map<std::string, SqOutputDataEntry>::iterator entry = m_OutputDataEntries.find( Decl.m_strName );
		if( entry != m_OutputDataEntries.end() )
			return( entry->second.m_Offset );
	}
	return( -1 );
}

TqInt CqRenderer::OutputDataSamples( const char* name )
{
	SqParameterDeclaration Decl = FindParameterDecl( name );
	if( Decl.m_Type != type_invalid )
	{
		std::map<std::string, SqOutputDataEntry>::iterator entry = m_OutputDataEntries.find( Decl.m_strName );
		if( entry != m_OutputDataEntries.end() )
			return( entry->second.m_NumSamples );
	}
	return( 0 );
}


//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )
