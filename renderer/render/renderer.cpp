// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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


/** \file
		\brief Implements the base CqRenderer class which is the central core of the rendering main loop.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	"aqsis.h"

#include	<time.h>

#include	"imagebuffer.h"
#include	"lights.h"
#include	"renderer.h"
#include	"shaders.h"
#include	"nurbs.h"
#include	"points.h"
#include	"lath.h"
#include	"render.h"
#include	"transform.h"
#include	"rifile.h"
#include	"texturemap.h"
#include	"shadervm.h"
#include	"inlineparse.h"
#include	"tiffio.h"
#include	"objectinstance.h"


START_NAMESPACE( Aqsis )

extern IqDDManager* CreateDisplayDriverManager();
extern IqRaytrace* CreateRaytracer();

//static CqShaderRegister * pOShaderRegister = NULL;

CqRenderer* pCurrRenderer = 0;

// Forward declaration
//-------------------------------- Tiff error handlers
void TIFF_ErrorHandler(const char*, const char*, va_list);
void TIFF_WarnHandler(const char*, const char*, va_list);


static TqUlong ohash = CqString::hash( "object" ); //< == "object"
static TqUlong shash = CqString::hash( "shader" ); //< == "shader"
static TqUlong chash = CqString::hash( "camera" ); //< == "camera"
static TqUlong cuhash = CqString::hash( "current" ); //< == "current"

static CqMatrix oldkey[2];  //< to eliminate Inverse(), Transpose() matrix ops.
static CqMatrix oldresult[2];

//---------------------------------------------------------------------
/** Default constructor for the main renderer class. Initialises current state.
 */

CqRenderer::CqRenderer() :
		m_pImageBuffer( 0 ),
		m_Mode( RenderMode_Image ),
		m_fSaveGPrims( false ),
		m_OutputDataOffset(9),		// Cs, Os, z, coverage, a
		m_OutputDataTotalSize(9),	// Cs, Os, z, coverage, a
		m_FrameNo( 0 ),
		m_bObjectOpen(false),
		m_pErrorHandler( &RiErrorPrint ),
		m_pProgressHandler( NULL ),
		m_pPreRenderFunction( NULL ),
		m_pPreWorldFunction( NULL ),
		m_pRaytracer( NULL )
{
	m_pImageBuffer = new	CqImageBuffer();

	// Initialize the default attributes, transform and camera transform
	m_pAttrDefault  = new CqAttributes;
	ADDREF( m_pAttrDefault );
	m_pTransDefault = CqTransformPtr( new CqTransform );
	m_pTransCamera  = CqTransformPtr( new CqTransform );
	m_pTransDefObj  = CqTransformPtr( new CqTransform );
	m_fWorldBegin = false;

	m_poptDefault = CqOptionsPtr(new CqOptions );

	// Initialise the array of coordinate systems.
	m_aCoordSystems.resize( CoordSystem_Last );

	m_aCoordSystems[ CoordSystem_Camera ].m_strName = "__camera__";
	m_aCoordSystems[ CoordSystem_Current ].m_strName = "__current__";
	m_aCoordSystems[ CoordSystem_World ].m_strName = "world";
	m_aCoordSystems[ CoordSystem_Screen ].m_strName = "screen";
	m_aCoordSystems[ CoordSystem_NDC ].m_strName = "NDC";
	m_aCoordSystems[ CoordSystem_Raster ].m_strName = "raster";

	m_aCoordSystems[ CoordSystem_Camera ].m_hash = CqString::hash( "__camera__" );
	m_aCoordSystems[ CoordSystem_Current ].m_hash = CqString::hash( "__current__" );
	m_aCoordSystems[ CoordSystem_World ].m_hash = CqString::hash( "world" );
	m_aCoordSystems[ CoordSystem_Screen ].m_hash = CqString::hash( "screen" );
	m_aCoordSystems[ CoordSystem_NDC ].m_hash = CqString::hash( "NDC" );
	m_aCoordSystems[ CoordSystem_Raster ].m_hash = CqString::hash( "raster" );

	m_pDDManager = CreateDisplayDriverManager();
	m_pDDManager->Initialise();

	m_pRaytracer = CreateRaytracer();
	m_pRaytracer->Initialise();

	// Set up DoF stuff for pinhole lens ( i.e. no DoF )
	m_UsingDepthOfField = false;

	// Get the hash keys for object, shader, camera keywords.

	// Set the TIFF Error/Warn handler
	TIFFSetErrorHandler( &TIFF_ErrorHandler );
	TIFFSetWarningHandler( &TIFF_WarnHandler );
}

//---------------------------------------------------------------------
/** Destructor
 */

CqRenderer::~CqRenderer()
{
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
	delete(m_pDDManager);

	// Delete the default attributes, transform and camera transform
	if ( m_pAttrDefault )
	{
		RELEASEREF( m_pAttrDefault );
		m_pAttrDefault = NULL;
	}

	if( m_pRaytracer )	// MGC: MEMLEAK_FIX
	{
		delete m_pRaytracer;
		m_pRaytracer = 0;	// MGC: or better NULL?
	}

	// Clear the ObjectInstance buffer
	std::vector<CqObjectInstance*>::iterator i;
	for(i=m_ObjectInstances.begin(); i!=m_ObjectInstances.end(); i++)
		delete((*i));
	m_ObjectInstances.clear();

#ifdef _DEBUG
	// Print information about any un-released CqRefCount objects
	//report_refcounts();
#endif

}


//---------------------------------------------------------------------
/** Create a new main context, called from within RiBegin(), error if not first
 * context created.  If first, create with this as the parent.
 */

boost::shared_ptr<CqModeBlock>	CqRenderer::BeginMainModeBlock()
{
	// XXX: Error checking may eventually be unnecessary.  - ajb
	if ( !m_pconCurrent )
	{
		m_pconCurrent = boost::shared_ptr<CqModeBlock>( new CqMainModeBlock( m_pconCurrent ) );
		return ( m_pconCurrent );
	}
	else
		return boost::shared_ptr<CqModeBlock>( );
}


//---------------------------------------------------------------------
/** Create a new Frame context, should only be called when the current
 * context is a Main context, but the internal context handling deals
 * with it so I don't need to worry.
 */

boost::shared_ptr<CqModeBlock>	CqRenderer::BeginFrameModeBlock()
{
	// XXX: Error checking may eventually be unnecessary.  - ajb
	if ( m_pconCurrent )
	{
		boost::shared_ptr<CqModeBlock> pconNew = m_pconCurrent->BeginFrameModeBlock();
		if ( pconNew )
		{
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return boost::shared_ptr<CqModeBlock>( );
	}
	else
		return boost::shared_ptr<CqModeBlock>( );
}


//---------------------------------------------------------------------
/** Create a new world context, again the internal context handling deals
 * with invalid calls.
 */

boost::shared_ptr<CqModeBlock>	CqRenderer::BeginWorldModeBlock()
{
	// XXX: Error checking may eventually be unnecessary.  - ajb
	if ( m_pconCurrent )
	{
		boost::shared_ptr<CqModeBlock> pconNew = m_pconCurrent->BeginWorldModeBlock();
		if ( pconNew )
		{
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return boost::shared_ptr<CqModeBlock>( );
	}
	else
		return boost::shared_ptr<CqModeBlock>( );
}


//---------------------------------------------------------------------
/** Create a new attribute context.
 */

boost::shared_ptr<CqModeBlock>	CqRenderer::BeginAttributeModeBlock()
{
	// XXX: Error checking may eventually be unnecessary.  - ajb
	if ( m_pconCurrent )
	{
		boost::shared_ptr<CqModeBlock> pconNew = m_pconCurrent->BeginAttributeModeBlock();
		if ( pconNew )
		{
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return boost::shared_ptr<CqModeBlock>( );
	}
	else
		return boost::shared_ptr<CqModeBlock>( );
}



//---------------------------------------------------------------------
/** Create a new transform context.
 */

boost::shared_ptr<CqModeBlock>	CqRenderer::BeginTransformModeBlock()
{
	// XXX: Error checking may eventually be unnecessary.  - ajb
	if ( m_pconCurrent )
	{
		boost::shared_ptr<CqModeBlock> pconNew = m_pconCurrent->BeginTransformModeBlock();
		if ( pconNew )
		{
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return boost::shared_ptr<CqModeBlock>( );
	}
	else
		return boost::shared_ptr<CqModeBlock>( );
}



//---------------------------------------------------------------------
/** Create a new solid context.
 */

boost::shared_ptr<CqModeBlock>	CqRenderer::BeginSolidModeBlock( CqString& type )
{
	// XXX: Error checking may eventually be unnecessary.  - ajb
	if ( m_pconCurrent )
	{
		boost::shared_ptr<CqModeBlock> pconNew = m_pconCurrent->BeginSolidModeBlock( type );
		if ( pconNew )
		{
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return boost::shared_ptr<CqModeBlock>( );
	}
	else
		return boost::shared_ptr<CqModeBlock>( );
}



//---------------------------------------------------------------------
/** Create a new object context.
 */

boost::shared_ptr<CqModeBlock>	CqRenderer::BeginObjectModeBlock()
{
	// XXX: Error checking may eventually be unnecessary.  - ajb
	if ( m_pconCurrent )
	{
		boost::shared_ptr<CqModeBlock> pconNew = m_pconCurrent->BeginObjectModeBlock();
		if ( pconNew )
		{
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return boost::shared_ptr<CqModeBlock>( );
	}
	else
		return boost::shared_ptr<CqModeBlock>( );
}



//---------------------------------------------------------------------
/** Create a new motion context.
 */

boost::shared_ptr<CqModeBlock>	CqRenderer::BeginMotionModeBlock( TqInt N, TqFloat times[] )
{
	// XXX: Error checking may eventually be unnecessary.  - ajb
	if ( m_pconCurrent )
	{
		boost::shared_ptr<CqModeBlock> pconNew = m_pconCurrent->BeginMotionModeBlock( N, times );
		if ( pconNew )
		{
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return boost::shared_ptr<CqModeBlock>( );
	}
	else
		return boost::shared_ptr<CqModeBlock>( );
}

//---------------------------------------------------------------------
/** Create a new resource context.
 */

boost::shared_ptr<CqModeBlock>	CqRenderer::BeginResourceModeBlock()
{
	// XXX: Error checking may eventually be unnecessary.  - ajb
	if ( m_pconCurrent )
	{
		boost::shared_ptr<CqModeBlock> pconNew = m_pconCurrent->BeginResourceModeBlock();
		if ( pconNew )
		{
			m_pconCurrent = pconNew;
			return ( pconNew );
		}
		else
			return boost::shared_ptr<CqModeBlock>( );
	}
	else
		return boost::shared_ptr<CqModeBlock>( );
}

//----------------------------------------------------------------------
/** Delete the current context presuming it is a main context.
 */

void	CqRenderer::EndMainModeBlock()
{
	if ( m_pconCurrent && (m_pconCurrent->Type() == BeginEnd))
	{
		m_pconCurrent->EndMainModeBlock();
		m_pconCurrent = m_pconCurrent->pconParent();
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a frame context.
 */

void	CqRenderer::EndFrameModeBlock()
{
	if ( m_pconCurrent && (m_pconCurrent->Type() == Frame ))
	{
		m_pconCurrent->EndFrameModeBlock();
		m_pconCurrent = m_pconCurrent->pconParent();
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a world context.
 */

void	CqRenderer::EndWorldModeBlock()
{
	if ( m_pconCurrent && (m_pconCurrent->Type() == World))
	{
		m_pconCurrent->EndWorldModeBlock();
		m_pconCurrent = m_pconCurrent->pconParent();
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a attribute context.
 */

void	CqRenderer::EndAttributeModeBlock()
{
	if ( m_pconCurrent && (m_pconCurrent->Type() == Attribute))
	{
		m_pconCurrent->EndAttributeModeBlock();
		m_pconCurrent = m_pconCurrent->pconParent();
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a transform context.
 */

void	CqRenderer::EndTransformModeBlock()
{
	if ( m_pconCurrent && (m_pconCurrent->Type() == Transform))
	{
		// Copy the current state of the attributes UP the stack as a TransformBegin/End doesn't store them
		m_pconCurrent->pconParent()->m_pattrCurrent = m_pconCurrent->m_pattrCurrent;
		m_pconCurrent->EndTransformModeBlock();
		m_pconCurrent = m_pconCurrent->pconParent();
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a solid context.
 */

void	CqRenderer::EndSolidModeBlock()
{
	if ( m_pconCurrent && (m_pconCurrent->Type() == Solid ) )
	{
		m_pconCurrent->EndSolidModeBlock();
		m_pconCurrent = m_pconCurrent->pconParent();
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a object context.
 */

void	CqRenderer::EndObjectModeBlock()
{
	if ( m_pconCurrent && (m_pconCurrent->Type() == Object ) )
	{
		m_pconCurrent->EndObjectModeBlock();
		m_pconCurrent = m_pconCurrent->pconParent();
	}
}


//----------------------------------------------------------------------
/** Delete the current context presuming it is a motion context.
 */

void	CqRenderer::EndMotionModeBlock()
{
	if ( m_pconCurrent && (m_pconCurrent->Type() == Motion) )
	{
		boost::shared_ptr<CqModeBlock> pconParent = m_pconCurrent->pconParent();
		// Copy the current state of the attributes UP the stack as a TransformBegin/End doesn't store them
		pconParent->m_pattrCurrent = m_pconCurrent->m_pattrCurrent;
		pconParent->m_ptransCurrent = m_pconCurrent->m_ptransCurrent;
		m_pconCurrent->EndMotionModeBlock();
		m_pconCurrent = pconParent;
	}
}

//
//----------------------------------------------------------------------
/** Delete the current context presuming it is a resource context.
 */

void	CqRenderer::EndResourceModeBlock()
{
	if ( m_pconCurrent && (m_pconCurrent->Type() == Resource))
	{
		m_pconCurrent->EndResourceModeBlock();
		m_pconCurrent = m_pconCurrent->pconParent();
	}
}

//----------------------------------------------------------------------
/** Get the current shutter time, always returns 0.0 unless within a motion block,
 * when it returns the appropriate shutter time.
 */

TqFloat	CqRenderer::Time() const
{
	if ( m_pconCurrent && m_pconCurrent->Type() == Motion)
		return ( m_pconCurrent->Time() );
	else
		return ( QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Shutter" ) [ 0 ] );
}


TqInt CqRenderer::bucketCount()
{
	return(pImage()->cXBuckets() * pImage()->cYBuckets() );
}


//----------------------------------------------------------------------
/** Advance the current shutter time, only valid within motion blocks.
 */

void CqRenderer::AdvanceTime()
{
	if ( m_pconCurrent )
		m_pconCurrent->AdvanceTime();
}


//----------------------------------------------------------------------
/** Return a reference to the current options.
 */

const IqOptionsPtr CqRenderer::poptCurrent() const
{
	if ( m_pconCurrent )
		return ( m_pconCurrent->poptCurrent() );
	else
	{
		return ( m_poptDefault );
	}
}

//----------------------------------------------------------------------
/** Return a reference to the current options.
 */

IqOptionsPtr CqRenderer::poptWriteCurrent()
{
	if ( m_pconCurrent )
		return ( m_pconCurrent->poptWriteCurrent() );
	else
	{
		return ( m_poptDefault );
	}
}
//----------------------------------------------------------------------
/** Push the current options allowing modification.
 */

IqOptionsPtr CqRenderer::pushOptions()
{
	if ( m_pconCurrent )
		return ( m_pconCurrent->pushOptions() );
	else
	{
		// \note: cannot push/pop options outside the Main block.
		return ( m_poptDefault );
	}
}

//----------------------------------------------------------------------
/** Pop the last stored options.
 */

IqOptionsPtr CqRenderer::popOptions()
{
	if ( m_pconCurrent )
		return ( m_pconCurrent->popOptions() );
	else
	{
		// \note: cannot push/pop options outside the Main block.
		return ( m_poptDefault );
	}
}


//----------------------------------------------------------------------
/** Return a pointer to the current attributes.
 */

const CqAttributes* CqRenderer::pattrCurrent()
{
	if ( m_pconCurrent )
		return ( m_pconCurrent->pattrCurrent() );
	else
		return ( m_pAttrDefault );
}


//----------------------------------------------------------------------
/** Return a writable pointer to the current attributes.
 */

CqAttributes* CqRenderer::pattrWriteCurrent()
{
	if ( m_pconCurrent )
		return ( m_pconCurrent->pattrWriteCurrent() );
	else
		return ( m_pAttrDefault );
}


//----------------------------------------------------------------------
/** Return a pointer to the current transform.
 */

CqTransformPtr CqRenderer::ptransCurrent()
{
	if ( m_pconCurrent )
		return ( m_pconCurrent->ptransCurrent() );
	else
		return ( m_pTransDefault );
}


//----------------------------------------------------------------------
/** Modify the current transformation.
 */

#if 0
CqTransformPtr CqRenderer::ptransWriteCurrent()
{
	if ( m_pconCurrent )
		return ( m_pconCurrent->ptransWriteCurrent() );
	else
		return ( m_pTransDefault );
}
#endif

void	CqRenderer::ptransSetTime( const CqMatrix& matTrans )
{
	if ( !m_pconCurrent )
	{
		throw 0;
	}

	CqTransformPtr newTrans( new CqTransform( m_pconCurrent->ptransCurrent(), Time(), matTrans, CqTransform::Set() ) );
	m_pconCurrent->ptransSetCurrent( newTrans );
}

void	CqRenderer::ptransSetCurrentTime( const CqMatrix& matTrans )
{
	if ( !m_pconCurrent )
	{
		throw 0;
	}

	CqTransformPtr newTrans( new CqTransform( m_pconCurrent->ptransCurrent(), Time(), matTrans, CqTransform::SetCurrent() ) );
	m_pconCurrent->ptransSetCurrent( newTrans );
}

void	CqRenderer::ptransConcatCurrentTime( const CqMatrix& matTrans )
{
	if ( !m_pconCurrent )
	{
		throw 0;
	}

	CqTransformPtr newTrans( new CqTransform( m_pconCurrent->ptransCurrent(), Time(), matTrans, CqTransform::ConcatCurrent() ) );
	m_pconCurrent->ptransSetCurrent( newTrans );
}


//----------------------------------------------------------------------
/** Render all surface in the current list to the image buffer.
 */

void CqRenderer::RenderWorld(bool clone)
{
	// While rendering, all primitives should fasttrack straight into the pipeline, the easiest way to ensure this
	// is to switch into 'non' multipass mode.
	TqInt multiPass = 0;
	TqInt* pMultipass = GetIntegerOptionWrite("Render", "multipass");
	if(pMultipass)
	{
		multiPass = pMultipass[0];
		pMultipass[0] = 0;
	}
	
	// Ensure that the camera and projection matrices are initialised.
	poptCurrent()->InitialiseCamera();
	pImage()->SetImage();

	PrepareShaders();

	if(clone)
		PostCloneOfWorld();
	else
		PostWorld();

	m_pDDManager->OpenDisplays();
	pImage() ->RenderImage();
	m_pDDManager->CloseDisplays();

	if(NULL != pMultipass)
		pMultipass[0] = multiPass;
}


//----------------------------------------------------------------------
/** Render any automatic shadow passes.
 */

void CqRenderer::RenderAutoShadows()
{
	// Check if multipass rendering is switched on.
	const TqInt* pMultipass = GetIntegerOption("Render", "multipass");
	if(pMultipass && pMultipass[0])
	{
		// Check all the lightsources for any with an attribute indicating autoshadows.
		TqUint ilight;
		for(ilight=0; ilight<Lightsource_stack.size(); ilight++)
		{
			CqLightsourcePtr light = Lightsource_stack[ilight];
			const CqString* pMapName = light->pAttributes()->GetStringAttribute("autoshadows", "shadowmapname");
			const CqString* pattrName = light->pAttributes()->GetStringAttribute( "identifier", "name" );
			if(NULL != pMapName)
			{
				if(NULL != pattrName)
					Aqsis::log() << info << "Rendering automatic shadow pass for lightsource : \"" << pattrName[0].c_str() << "\" to shadow map file \"" << pMapName[0].c_str() << "\"" << std::endl;
				else
					Aqsis::log() << info << "Rendering automatic shadow pass for lightsource : \"unnamed\" to shadow map file \"" << pMapName[0].c_str() << "\"" << std::endl;

				const TqInt* pRes = light->pAttributes()->GetIntegerAttribute("autoshadows", "res");
				TqInt res = 300;
				if(NULL != pRes)
					res = pRes[0];
				// Setup a new set of options based on the current ones.
				IqOptionsPtr opts = pushOptions();
				opts->GetIntegerOptionWrite( "System", "Resolution" ) [ 0 ] = res;
				opts->GetIntegerOptionWrite( "System", "Resolution" ) [ 1 ] = res;
				opts->GetFloatOptionWrite( "System", "PixelAspectRatio" ) [ 0 ] = 1.0f;

				// Now that the options have all been set, setup any undefined camera parameters.
				opts->GetFloatOptionWrite( "System", "FrameAspectRatio" ) [ 0 ] = 1.0;
				opts->GetFloatOptionWrite( "System", "ScreenWindow" ) [ 0 ] = -1.0 ;
				opts->GetFloatOptionWrite( "System", "ScreenWindow" ) [ 1 ] = 1.0;
				opts->GetFloatOptionWrite( "System", "ScreenWindow" ) [ 2 ] = 1.0;
				opts->GetFloatOptionWrite( "System", "ScreenWindow" ) [ 3 ] = -1.0;
				opts->GetIntegerOptionWrite( "System", "DisplayMode" ) [ 0 ] = ModeZ;

				// Set the pixel samples to 1,1 for shadow rendering.
				opts->GetIntegerOptionWrite( "System", "PixelSamples" ) [ 0 ] = 1;
				opts->GetIntegerOptionWrite( "System", "PixelSamples" ) [ 1 ] = 1;

				// Set the pixel filter to box, 1,1 for shadow rendering.
				opts->SetfuncFilter( RiBoxFilter );
				opts->GetFloatOptionWrite( "System", "FilterWidth" ) [ 0 ] = 1;
				opts->GetFloatOptionWrite( "System", "FilterWidth" ) [ 1 ] = 1;

				// Make sure the depthFilter is set to "midpoint".
				opts->GetStringOptionWrite( "Hider", "depthfilter" ) [ 0 ] = CqString("midpoint");

				// Don't bother doing lighting calcualations.
				opts->GetIntegerOptionWrite( "EnableShaders", "lighting" ) [ 0 ] = 0;

				// Now set the camera transform the to light transform (inverse because the camera transform is transforming the world into camera space).
				CqTransformPtr lightTrans(light->pTransform()->Inverse());

				// Cache the current DDManager, and replace it for the purposes of our shadow render.
				IqDDManager* realDDManager = m_pDDManager;
				m_pDDManager = CreateDisplayDriverManager();
				m_pDDManager->Initialise();
				std::map<std::string, void*> args;
				AddDisplayRequest(pMapName[0].c_str(), "shadow", "z", ModeZ, 0, 1, args);

				// Store the current camera transform for later.
				CqTransformPtr defaultCamera;
				defaultCamera = GetCameraTransform();
				SetCameraTransform(lightTrans);
				
				// Render the world
				RenderWorld(true);

				popOptions();
				SetCameraTransform(defaultCamera);

				m_pDDManager->Shutdown();
				delete(m_pDDManager);
				m_pDDManager = realDDManager;

				CqTextureMap::FlushCache();
				CqOcclusionBox::DeleteHierarchy();
				clippingVolume().clear();
			}
		}
	}
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
	m_OutputDataOffset = 9;		// Cs, Os, depth, coverage, a
	m_OutputDataTotalSize = 9;	// Cs, Os, depth, coverage, a

	m_clippingVolume.clear();
}


//----------------------------------------------------------------------
/** Get the matrix to convert between the specified coordinate systems.
 */


CqMatrix	CqRenderer::matSpaceToSpace( const char* strFrom, const char* strTo, const IqTransform* transShaderToWorld, const IqTransform* transObjectToWorld, TqFloat time )
{
	CqMatrix	matResult, matA, matB;
	TqUlong fhash, thash;

	// Get the hash keys for From,To spaces
	fhash = CqString::hash( strFrom );
	thash = CqString::hash( strTo );

	// Get the two component matrices.
	// First check for special cases.
	if ( fhash == ohash )
	{
		if (transObjectToWorld) 
			matA = transObjectToWorld->matObjectToWorld(time);
	} else if ( fhash == shash )
	{
		if (transShaderToWorld) 
			matA = transShaderToWorld->matObjectToWorld(time);
	} else if ( ( fhash == chash ) || ( fhash == cuhash ) )
	{
		if (m_pTransCamera) 
			matA = m_pTransCamera->matObjectToWorld( time ).Inverse();
	}
	else
	{
		WhichMatToWorld( matA, fhash );
	}


	if ( thash == ohash )
	{
		if (transObjectToWorld)
			matB = transObjectToWorld->matObjectToWorld(time).Inverse();
	} else if ( thash == shash )
	{
		if (transShaderToWorld)
			matB = transShaderToWorld->matObjectToWorld(time).Inverse();
	} else if ( ( thash == chash ) || ( thash == cuhash ) )
	{
		if (m_pTransCamera)
			matB = m_pTransCamera->matObjectToWorld( time );
	} else
	{
		WhichMatWorldTo( matB, thash );
	}

	matResult = matB * matA;

	return ( matResult );
}



//----------------------------------------------------------------------
/** Get the matrix to convert vectors between the specified coordinate systems.
 */

CqMatrix	CqRenderer::matVSpaceToSpace( const char* strFrom, const char* strTo, const IqTransform* transShaderToWorld, const IqTransform* transObjectToWorld, TqFloat time )
{
	CqMatrix	matResult, matA, matB;

	TqUlong fhash, thash;

	// Get the hash keys for From,To spaces
	fhash = CqString::hash( strFrom );
	thash = CqString::hash( strTo );

	// Get the two component matrices.
	// First check for special cases.
	if ( fhash == ohash )
	{
		if (transObjectToWorld)
			matA = transObjectToWorld->matObjectToWorld(time);
	} else if ( fhash == shash )
	{
		if (transShaderToWorld)
			matA = transShaderToWorld->matObjectToWorld(time);
	} else if ( ( fhash == chash ) || ( fhash == cuhash ) )
	{
		if (m_pTransCamera)
			matA = m_pTransCamera->matObjectToWorld( time ).Inverse();
	} else
	{
		WhichMatToWorld ( matA, fhash );
	}

	if ( thash == ohash )
	{
		if (transObjectToWorld)
			matB = transObjectToWorld->matObjectToWorld(time).Inverse();
	} else if ( thash == shash )
	{
		if (transShaderToWorld)
			matB = transShaderToWorld->matObjectToWorld(time).Inverse();
	}
	else if ( ( thash == chash ) || ( thash == cuhash ) )
	{
		if (m_pTransCamera)
			matB = m_pTransCamera->matObjectToWorld( time );
	} else
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

	}
	else
	{
		return oldresult[0];
	}
	return ( matResult );
}


//----------------------------------------------------------------------
/** Get the matrix to convert normals between the specified coordinate systems.
 */

CqMatrix	CqRenderer::matNSpaceToSpace( const char* strFrom, const char* strTo, const IqTransform* transShaderToWorld, const IqTransform* transObjectToWorld, TqFloat time )
{
	CqMatrix	matResult, matA, matB;

	TqUlong fhash, thash;

	// Get the hash keys for From,To spaces
	fhash = CqString::hash( strFrom );
	thash = CqString::hash( strTo );

	// Get the two component matrices.
	// First check for special cases.
	if ( fhash == ohash )
	{
		if (transObjectToWorld)
			matA = transObjectToWorld->matObjectToWorld(time);
	} else if ( fhash == shash )
	{
		if (transShaderToWorld)
			matA = transShaderToWorld->matObjectToWorld(time);
	} else if ( ( fhash == chash ) || ( fhash == cuhash ) )
	{
		if (m_pTransCamera)
			matA = m_pTransCamera->matObjectToWorld( time ).Inverse();
	} else
	{
		WhichMatToWorld ( matA, fhash );
	}

	if ( thash == ohash )
	{
		if (transObjectToWorld)
			matB = transObjectToWorld->matObjectToWorld(time).Inverse();
	} else if ( thash == shash )
	{
		if (transShaderToWorld)
			matB = transShaderToWorld->matObjectToWorld(time).Inverse();
	} else if ( ( thash == chash ) || ( thash == cuhash ) )
	{
		if (m_pTransCamera)
			matB = m_pTransCamera->matObjectToWorld( time );
	} else
	{
		WhichMatWorldTo ( matB, thash );
	}


	matResult = matB * matA;
	if (memcmp((void *) oldkey[1].pElements(), (void *) matResult.pElements(), sizeof(TqFloat) * 16) != 0)
	{
		oldkey[1] = matResult;
		matResult[ 3 ][ 0 ] = matResult[ 3 ][ 1 ] = matResult[ 3 ][ 2 ] = matResult[ 0 ][ 3 ] = matResult[ 1 ][ 3 ] = matResult[ 2 ][ 3 ] = 0.0;
		matResult[ 3 ][ 3 ] = 1.0;
		matResult = matResult.Inverse().Transpose();
		oldresult[1] = matResult;

	}
	else
	{
		return oldresult[1];
	}

	return ( matResult );
}


const	TqFloat*	CqRenderer::GetFloatOption( const char* strName, const char* strParam ) const
{
	return ( poptCurrent()->GetFloatOption( strName, strParam ) );
}

const	TqInt*	CqRenderer::GetIntegerOption( const char* strName, const char* strParam ) const
{
	return ( poptCurrent()->GetIntegerOption( strName, strParam ) );
}

const	CqString*	CqRenderer::GetStringOption( const char* strName, const char* strParam ) const
{
	return ( poptCurrent()->GetStringOption( strName, strParam ) );
}

const	CqVector3D*	CqRenderer::GetPointOption( const char* strName, const char* strParam ) const
{
	return ( poptCurrent()->GetPointOption( strName, strParam ) );
}

const	CqColor*	CqRenderer::GetColorOption( const char* strName, const char* strParam ) const
{
	return ( poptCurrent()->GetColorOption( strName, strParam ) );
}


TqFloat*	CqRenderer::GetFloatOptionWrite( const char* strName, const char* strParam )
{
	return ( poptWriteCurrent()->GetFloatOptionWrite( strName, strParam ) );
}

TqInt*	CqRenderer::GetIntegerOptionWrite( const char* strName, const char* strParam )
{
	return ( poptWriteCurrent()->GetIntegerOptionWrite( strName, strParam ) );
}

CqString*	CqRenderer::GetStringOptionWrite( const char* strName, const char* strParam )
{
	return ( poptWriteCurrent()->GetStringOptionWrite( strName, strParam ) );
}

CqVector3D*	CqRenderer::GetPointOptionWrite( const char* strName, const char* strParam )
{
	return ( poptWriteCurrent()->GetPointOptionWrite( strName, strParam ) );
}

CqColor*	CqRenderer::GetColorOptionWrite( const char* strName, const char* strParam )
{
	return ( poptWriteCurrent()->GetColorOptionWrite( strName, strParam ) );
}


//----------------------------------------------------------------------
/** Store the named coordinate system in the array of named coordinate systems, overwrite any existing
 * with the same name. Returns true if system already exists.
 */

bool	CqRenderer::SetCoordSystem( const char* strName, const CqMatrix& matToWorld )
{
	// Search for the same named system in the current list.
	TqUlong hash = CqString::hash( strName );
	for ( TqUint i = 0; i < m_aCoordSystems.size(); i++ )
	{
		if ( m_aCoordSystems[ i ].m_hash == hash )
		{
			m_aCoordSystems[ i ].m_matToWorld = matToWorld;
			m_aCoordSystems[ i ].m_matWorldTo = matToWorld.Inverse();
			return ( true );
		}
	}

	// If we got here, it didn't exists.
	m_aCoordSystems.push_back( SqCoordSys( strName, matToWorld, matToWorld.Inverse() ) );
	return ( false );
}


//----------------------------------------------------------------------
/** Find a parameter type declaration and return it.
 * \param strDecl Character pointer to the name of the declaration to find.
 */

SqParameterDeclaration CqRenderer::FindParameterDecl( const char* strDecl )
{
	CqInlineParse parser;
	std::string __strDecl( strDecl );
	parser.parse( __strDecl );

	if( parser.isInline() )
	{
		SqParameterDeclaration Decl;
		Decl.m_strName = parser.getIdentifier();
		Decl.m_Count = parser.getQuantity();
		Decl.m_Type = parser.getType();
		Decl.m_Class = parser.getClass();
		Decl.m_strSpace = "";

		// Get the creation function.
		switch ( Decl.m_Class )
		{
				case class_constant:
				{
					if ( Decl.m_Count > 1 )
						Decl.m_pCreate = gVariableCreateFuncsConstantArray[ Decl.m_Type ];
					else
						Decl.m_pCreate = gVariableCreateFuncsConstant[ Decl.m_Type ];
				}
				break;

				case class_uniform:
				{
					if ( Decl.m_Count > 1 )
						Decl.m_pCreate = gVariableCreateFuncsUniformArray[ Decl.m_Type ];
					else
						Decl.m_pCreate = gVariableCreateFuncsUniform[ Decl.m_Type ];
				}
				break;

				case class_varying:
				{
					if ( Decl.m_Count > 1 )
						Decl.m_pCreate = gVariableCreateFuncsVaryingArray[ Decl.m_Type ];
					else
						Decl.m_pCreate = gVariableCreateFuncsVarying[ Decl.m_Type ];
				}
				break;

				case class_vertex:
				{
					if ( Decl.m_Count > 1 )
						Decl.m_pCreate = gVariableCreateFuncsVertexArray[ Decl.m_Type ];
					else
						Decl.m_pCreate = gVariableCreateFuncsVertex[ Decl.m_Type ];
				}
				break;

				case class_facevarying:
				{
					if ( Decl.m_Count > 1 )
						Decl.m_pCreate = gVariableCreateFuncsFaceVaryingArray[ Decl.m_Type ];
					else
						Decl.m_pCreate = gVariableCreateFuncsFaceVarying[ Decl.m_Type ];
				}
				break;

				case class_facevertex:
				{
					if ( Decl.m_Count > 1 )
						Decl.m_pCreate = gVariableCreateFuncsFaceVertexArray[ Decl.m_Type ];
					else
						Decl.m_pCreate = gVariableCreateFuncsFaceVertex[ Decl.m_Type ];
				}
				break;

				default:
				{
					// left blank to avoid compiler warnings about unhandled types
					break;
				}
		}
		return ( Decl );
	}

	CqString strName = strDecl;
	// Search the local parameter declaration list.
	std::vector<SqParameterDeclaration>::iterator is;
	std::vector<SqParameterDeclaration>::iterator end = m_Symbols.end();
	TqUlong hash = CqString::hash( strDecl );
	for ( is = m_Symbols.begin(); is != end ; is++ )
	{
		if ( is->m_hash == 0)
		{
			is->m_hash = CqString::hash( is->m_strName.c_str() );
		}
		if ( hash == is->m_hash)
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
	SqParameterDeclaration Decl;
	try
	{
		Decl = FindParameterDecl( strDecl.c_str() );
	}
	catch( XqException e )
	{
		Aqsis::log() << error << e.strReason().c_str() << std::endl;
		return;
	}

	// Put new declaration at the top to make it take priority over previous
	m_Symbols.insert( m_Symbols.begin(), Decl );
}


//---------------------------------------------------------------------
/** Register a shader of the specified type with the specified name.
 */
#if 0
void CqRenderer::RegisterShader( const char* strName, EqShaderType type, IqShader* pShader )
{
	assert( pShader );
	m_Shaders.LinkLast( new CqShaderRegister( strName, type, pShader ) );
}
#endif


//---------------------------------------------------------------------
/** Find a shader of the specified type with the specified name.
 */
#if 0
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
#endif

//---------------------------------------------------------------------
/** Returns a pointer to the default surface.
 */
boost::shared_ptr<IqShader> CqRenderer::getDefaultSurfaceShader()
{
	// construct a key to index the default surface
	CqShaderKey key( "_def_", Type_Surface );

	// check for the shader in the existing map
	boost::shared_ptr<IqShader> pMapCheck =
	    CreateShader( "_def_", Type_Surface );
	if (pMapCheck)
	{
		// we must initialize the shader here.  non-default
		//  shaders are initialized in RiSurfaceV()
		pMapCheck->SetTransform( QGetRenderContext() ->ptransCurrent() );
		CqShaderVM* pCreated = static_cast<CqShaderVM*>( pMapCheck.get() );
		pCreated->PrepareDefArgs();

		return pMapCheck;
	}

	// insert the default surface template into the map
	boost::shared_ptr<IqShader> pRet( new CqShaderVM(this) );
	pRet->SetType( Type_Surface );
	CqShaderVM* pShader = static_cast<CqShaderVM*>( pRet.get() );
	pShader->SetstrName( "_def_" );
	pShader->DefaultSurface();
	pShader->SetTransform( ptransCurrent() );
	pShader->PrepareDefArgs();
	m_Shaders[key] = pRet;

	// return a clone of the default surface template
	boost::shared_ptr<IqShader> newShader(pRet->Clone());
        newShader->SetType ( Type_Surface );
	m_InstancedShaders.push_back(newShader);
	return (newShader);

}

//---------------------------------------------------------------------
/** Find a shader of the specified type with the specified name.
 * If not found, try to load one.
 */
boost::shared_ptr<IqShader> CqRenderer::CreateShader(
    const char* strName, EqShaderType type )
{
	// construct the key which is used to index the shader
	CqShaderKey key( strName, type );

	// first, look for the shader of the appropriate type and name in the
	//  map of shader "templates"
	if ( m_Shaders.find(key) != m_Shaders.end() )
	{
		// the shader template is present, so return its clone
		boost::shared_ptr<IqShader> newShader(m_Shaders[key]->Clone());
		newShader->SetType( type );
		m_InstancedShaders.push_back(newShader);
		return (newShader);
	}

	// we now create the shader...

	// search in the current directory first
	CqString strFilename( strName );
	strFilename += RI_SHADER_EXTENSION;
	CqRiFile SLXFile( strFilename.c_str(), "shader" );
	if ( SLXFile.IsValid() )
	{
		boost::shared_ptr<IqShader> pRet( new CqShaderVM(this) );

		CqShaderVM* pShader = static_cast<CqShaderVM*>( pRet.get() );
		const CqString* poptDSOPath = QGetRenderContext()->
		                              poptCurrent()->GetStringOption( "searchpath", "shader" );

		if(poptDSOPath)
		{
			Aqsis::log() << info << "DSO lib path set to \"" << poptDSOPath->c_str()
			<< "\"" << std::endl;

			pShader->SetDSOPath( poptDSOPath->c_str() );
		}

		CqString strRealName( SLXFile.strRealName() );
		Aqsis::log() << info << "Loading shader \"" << strName
		<< "\" from file \"" << strRealName.c_str()
		<< "\"" << std::endl;

		pShader->SetstrName( strName );
		pShader->LoadProgram( SLXFile );

		// add the shader to the map as a template and return its
		//  clone
		m_Shaders[key] = pRet;
		boost::shared_ptr<IqShader> newShader(pRet->Clone());
		newShader->SetType( type );
		m_InstancedShaders.push_back(newShader);
		return (newShader);
	}
	else
	{
		if (
		    (strcmp( strName, "null" )  != 0) &&
		    (strcmp( strName, "_def_" ) != 0)
		)
		{
			CqString strError;
			strError.Format( "Shader \"%s\" not found", strName ? strName : "" );
			Aqsis::log() << error << strError.c_str() << std::endl;
			const CqString* poptShaderPath = QGetRenderContext()->poptCurrent()->GetStringOption("searchpath", "shader");
			if(poptShaderPath != NULL)
				Aqsis::log() << info << "Shader searchpath is : " << poptShaderPath[0] << std::endl;
			else
				Aqsis::log() << info << "No shader searchpath specified" << std::endl;
		}
		if ( type == Type_Surface )
		{
			boost::shared_ptr<IqShader> pRet( new CqShaderVM(this) );

			pRet->SetType( type );
			CqShaderVM* pShader = static_cast<CqShaderVM*>(
			                          pRet.get() );
			pShader->SetstrName( "null" );
			pShader->DefaultSurface();

			// add the shader to the map and return its clone
			m_Shaders[key] = pRet;
			boost::shared_ptr<IqShader> newShader(pRet->Clone());
			newShader->SetType( type );
			m_InstancedShaders.push_back(newShader);
			return (newShader);
		}
		else
		{
			// the boost::shared_ptr analogue of return NULL:
			return boost::shared_ptr<IqShader>();
		}
	}

}

//----------------------------------------------------------------------
/** Add a new surface to the list of surfaces in the world.
 * \param pSurface A pointer to a CqSurface derived class, surface should at this point be in world space.
 */

void CqRenderer::StorePrimitive( const boost::shared_ptr<CqSurface>& pSurface )
{
	// If we are not in a mode that allows 'extra' passes, then fasttrack the primitive directly into the pipeline.
	const TqInt* pMultipass = GetIntegerOption("Render", "multipass");
	if(pMultipass && pMultipass[0])
		m_aWorld.push_back(pSurface);
	else
	{
		pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0 ),
						 QGetRenderContext() ->matNSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0 ),
						 QGetRenderContext() ->matVSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0 ) );
		pSurface->PrepareTrimCurve();
		PostSurface(pSurface);
	}
}

void CqRenderer::PostWorld()
{
	while(!m_aWorld.empty())
	{
		boost::shared_ptr<CqSurface> pSurface = m_aWorld.front();
		
		pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0 ),
						 QGetRenderContext() ->matNSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0 ),
						 QGetRenderContext() ->matVSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0 ) );
		pSurface->PrepareTrimCurve();
		PostSurface(pSurface);
		m_aWorld.pop_front();
	}
}
	

void CqRenderer::PostCloneOfWorld()
{
	std::deque<boost::shared_ptr<CqSurface> >::iterator i;
	for(i=m_aWorld.begin(); i!=m_aWorld.end(); i++)
	{
		boost::shared_ptr<CqSurface> pSurface((*i)->Clone());
		pSurface->Transform( QGetRenderContext() ->matSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0 ),
						 QGetRenderContext() ->matNSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0 ),
						 QGetRenderContext() ->matVSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0 ) );
		pSurface->PrepareTrimCurve();
		PostSurface(pSurface);
	}
}


void CqRenderer::PostSurface( const boost::shared_ptr<CqSurface>& pSurface )
{
	// Check the level of detail settings to see if this surface should be culled or not.
	const TqFloat* rangeAttr = pSurface->pAttributes()->GetFloatAttribute( "System", "LODRanges" );
	const TqFloat* boundAttr = pSurface->pAttributes()->GetFloatAttribute( "System", "LODBound" );

	CqBound bound(boundAttr);
	if(bound.Volume2() > 0)
	{
		bound.Transform( QGetRenderContext() ->matSpaceToSpace( "object", "raster", NULL, pSurface->pTransform().get(), QGetRenderContext()->Time() ) );

		TqFloat ruler = fabs( ( bound.vecMax().x() - bound.vecMin().x() ) * ( bound.vecMax().y() - bound.vecMin().y() ) );

		ruler *= QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "RelativeDetail" ) [ 0 ];

		CqString objname( "unnamed" );
		const CqString* pattrName = pSurface->pAttributes()->GetStringAttribute( "identifier", "name" );
		if ( pattrName != 0 )
			objname = pattrName[ 0 ];
		Aqsis::log() << info << "Object " << objname << " has an onscreen detail area of " << ruler << std::endl;

		TqFloat minImportance;
		if( rangeAttr[1] == rangeAttr[0] )
			minImportance = ruler < rangeAttr[1] ? 1.0f : 0.0f;
		else
			minImportance = CLAMP( ( rangeAttr[1] - ruler ) / ( rangeAttr[1] - rangeAttr[0] ), 0, 1 );

		TqFloat maxImportance;
		if ( rangeAttr[2] == rangeAttr[3] )
			maxImportance = ruler < rangeAttr[2] ? 1.0f : 0.0f;
		else
			maxImportance = CLAMP( ( rangeAttr[3] - ruler ) / ( rangeAttr[3] - rangeAttr[2] ), 0, 1 );

		if ( minImportance >= maxImportance )
			// Geomtry must be culled.
			return;

		Aqsis::log() << info << "LevelOfDetailBounds: " << minImportance << ", " << maxImportance << std::endl;
		pSurface->pAttributes()->GetFloatAttributeWrite( "System", "LevelOfDetailBounds" ) [ 0 ] = minImportance;
		pSurface->pAttributes()->GetFloatAttributeWrite( "System", "LevelOfDetailBounds" ) [ 1 ] = maxImportance;
	}

	pImage()->PostSurface(pSurface);
}


/** Prepare the shaders for rendering.
 */
void CqRenderer::PrepareShaders()
{
	std::vector< boost::shared_ptr<IqShader> >::iterator i;
	for(i = m_InstancedShaders.begin(); i!=m_InstancedShaders.end(); i++)
	{
		(*i)->PrepareShaderForUse();
	}
}


//---------------------------------------------------------------------
/** Find a shader of the specified type with the specified name.
 * If not found, try and load one.
 */
#if 0
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
			CqShaderVM * pShader = new CqShaderVM(this);
			const CqString *poptDSOPath = QGetRenderContext()->poptCurrent()->GetStringOption( "searchpath","shader" );
			pShader->SetDSOPath( poptDSOPath );

			CqString strRealName( SLXFile.strRealName() );
			Aqsis::log() << info << "Loading shader \"" << strName << "\" from file \"" << strRealName.c_str() << "\"" << std::endl;

			pShader->SetstrName( strName );
			pShader->LoadProgram( SLXFile );
			RegisterShader( strName, type, pShader );
			return ( pShader );
		}
		else
		{
			if ( strcmp( strName, "null" ) != 0 )
			{
				CqString strError;
				strError.Format( "Shader \"%s\" not found", strName ? strName : "" );
				Aqsis::log() << error << strError.c_str() << std::endl;
			}
			if( type == Type_Surface )
			{
				CqShaderVM * pShader = new CqShaderVM(this);
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
#endif



//---------------------------------------------------------------------
/** Add a new requested display driver to the list.
 */

void CqRenderer::AddDisplayRequest( const TqChar* name, const TqChar* type, const TqChar* mode, TqInt modeID, TqInt dataOffset, TqInt dataSize, std::map<std::string, void*>& mapOfArguments )
{
	m_pDDManager->AddDisplay( name, type, mode, modeID, dataOffset, dataSize, mapOfArguments );
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


bool	CqRenderer::GetBasisMatrix( CqMatrix& matBasis, const CqString& name )
{
	RtBasis basis;
	if ( BasisFromName( &basis, name.c_str() ) )
	{
		matBasis = basis;
		return ( true );
	}
	else
		return ( false );
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

	SqParameterDeclaration Decl;
	try
	{
		Decl = FindParameterDecl( name );
	}
	catch( XqException e )
	{
		Aqsis::log() << error << e.strReason().c_str() << std::endl;
		return(-1);
	}
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
				default:
				break;	// left blank to avoid compiler warnings about unhandled types
		}

		DataEntry.m_Offset = m_OutputDataOffset;
		DataEntry.m_NumSamples = NumSamples;
		DataEntry.m_Type = Decl.m_Type;
		m_OutputDataOffset += NumSamples;
		m_OutputDataTotalSize += NumSamples;

		// Add the new entry to the map, using the Decl name as the key.
		m_OutputDataEntries[Decl.m_strName] = DataEntry;

		return( DataEntry.m_Offset );
	}
	else
		Aqsis::log() << error << "Unrecognised AOV output variable \"" << name << "\"" << std::endl;

	return( -1 );
}

TqInt CqRenderer::OutputDataIndex( const char* name )
{
	SqParameterDeclaration Decl;
	try
	{
		Decl = FindParameterDecl( name );
	}
	catch( XqException e )
	{
		Aqsis::log() << error << e.strReason().c_str() << std::endl;
		return(-1);
	}
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
	SqParameterDeclaration Decl;
	try
	{
		Decl = FindParameterDecl( name );
	}
	catch( XqException e )
	{
		Aqsis::log() << error << e.strReason().c_str() << std::endl;
		return(-1);
	}
	if( Decl.m_Type != type_invalid )
	{
		std::map<std::string, SqOutputDataEntry>::iterator entry = m_OutputDataEntries.find( Decl.m_strName );
		if( entry != m_OutputDataEntries.end() )
			return( entry->second.m_NumSamples );
	}
	return( 0 );
}


TqInt CqRenderer::OutputDataType( const char* name )
{
	SqParameterDeclaration Decl;
	try
	{
		Decl = FindParameterDecl( name );
	}
	catch( XqException e )
	{
		Aqsis::log() << error << e.strReason().c_str() << std::endl;
		return(-1);
	}
	if( Decl.m_Type != type_invalid )
	{
		std::map<std::string, SqOutputDataEntry>::iterator entry = m_OutputDataEntries.find( Decl.m_strName );
		if( entry != m_OutputDataEntries.end() )
			return( entry->second.m_Type );
	}
	return( 0 );
}


CqObjectInstance* CqRenderer::OpenNewObjectInstance()
{
	assert( !m_bObjectOpen );
	m_bObjectOpen = true;
	CqObjectInstance* pNew = new CqObjectInstance;
	m_ObjectInstances.push_back(pNew);

	return( pNew );
}


void CqRenderer::InstantiateObject( CqObjectInstance* handle )
{
	// Ensure that the object passed in is valid.
	if( std::find(m_ObjectInstances.begin(), m_ObjectInstances.end(), handle ) != m_ObjectInstances.end() )
		handle->RecallInstance();
}


void TIFF_ErrorHandler(const char* mdl, const char* fmt, va_list va)
{
	char err_string[384];
	vsprintf( err_string, fmt, va );
	Aqsis::log() << error << err_string << " in file: \"" << mdl << "\"" << std::endl;
}

void TIFF_WarnHandler(const char* mdl, const char* fmt, va_list va)
{
	// Ignore warnings
}


void	CqRenderer::SetImage( CqImageBuffer* pImage )
{
	if(m_pImageBuffer != NULL)
		delete(m_pImageBuffer);
	m_pImageBuffer = pImage;
}

//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )

