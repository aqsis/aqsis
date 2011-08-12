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


/** \file
		\brief Implements the base CqRenderer class which is the central core of the rendering main loop.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<aqsis/aqsis.h>

#include	<cstring> // for memcmp, strcmp
#include	<time.h>
#include	<boost/bind.hpp>
#include	<boost/filesystem/fstream.hpp>

#include	"imagebuffer.h"
#include	"lights.h"
#include	"renderer.h"
#include	"shaders.h"
#include	"nurbs.h"
#include	"points.h"
#include	"lath.h"
#include	"transform.h"
#include	"texturemap_old.h"
#include	<aqsis/shadervm/ishader.h>
#include	"tiffio.h"


namespace Aqsis {

extern IqDDManager* CreateDisplayDriverManager();
extern IqRaytrace* CreateRaytracer();

//static CqShaderRegister * pOShaderRegister = NULL;

CqRenderer* pCurrRenderer = 0;

// Forward declaration
//-------------------------------- Tiff error handlers
void TIFF_ErrorHandler(const char*, const char*, va_list);
void TIFF_WarnHandler(const char*, const char*, va_list);


static const TqUlong ohash = CqString::hash( "object" ); //< == "object"
static const TqUlong shash = CqString::hash( "shader" ); //< == "shader"
static const TqUlong chash = CqString::hash( "camera" ); //< == "camera"
static const TqUlong cuhash = CqString::hash( "current" ); //< == "current"

static CqMatrix oldkey[2];  //< to eliminate Inverse(), Transpose() matrix ops.
static CqMatrix oldresult[2];

//---------------------------------------------------------------------
/** Default constructor for the main renderer class. Initialises current state.
 */

CqRenderer::CqRenderer()
	: m_pconCurrent(),
	m_Stats(),
	m_pAttrDefault(new CqAttributes()),
	m_poptDefault(new CqOptions()),
	m_pTransDefault(new CqTransform()),
	m_pImageBuffer(new CqImageBuffer()),
	m_pDDManager(CreateDisplayDriverManager()),
	m_Mode(RenderMode_Image),
	m_Shaders(),
	m_InstancedShaders(),
	m_lights(),
	m_textureCache(),
	m_fSaveGPrims(false),
	m_pTransCamera(new CqTransform()),
	m_pTransDefObj(new CqTransform()),
	m_fWorldBegin(false),
	m_tokenDict(),
	m_DofMultiplier(0),
	m_OneOverFocalDistance(FLT_MAX),
	m_UsingDepthOfField(false),       // DoF for pinhole lens
	m_DepthOfFieldScale(),
	m_OutputDataEntries(),
	m_OutputDataOffset(9),		// Cs, Os, z, coverage, a
	m_OutputDataTotalSize(9),	// Cs, Os, z, coverage, a
	m_FrameNo(0),
	m_pErrorHandler(&RiErrorPrint),
	m_pProgressHandler(0),
	m_pRaytracer(CreateRaytracer()),
	m_clippingVolume(),
	m_aWorld(),
	m_cropWindowXMin(0),
	m_cropWindowXMax(0),
	m_cropWindowYMin(0),
	m_cropWindowYMax(0),
	m_aCoordSystems(CoordSystem_Last)
{
	m_pDDManager->Initialise();

	m_pRaytracer->Initialise();

	m_textureCache = IqTextureCache::create(
			boost::bind(&CqRenderer::textureSearchPath, this));

	// Initialise the array of coordinate systems.
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
	shutdownShaderVM();

	// Close down the Display device manager.
	m_pDDManager->Shutdown();
	delete m_pDDManager;

	delete m_pRaytracer;

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

CqAttributesPtr CqRenderer::pattrCurrent() const
{
	if ( m_pconCurrent )
		return ( m_pconCurrent->pattrCurrent() );
	else
		return ( m_pAttrDefault );
}


//----------------------------------------------------------------------
/** Return a writable pointer to the current attributes.
 */

CqAttributesPtr CqRenderer::pattrWriteCurrent() const
{
	if ( m_pconCurrent )
		return ( m_pconCurrent->pattrWriteCurrent() );
	else
		return ( m_pAttrDefault );
}


//----------------------------------------------------------------------
/** Return a pointer to the current transform.
 */

CqTransformPtr CqRenderer::ptransCurrent() const
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
	assert(m_pconCurrent);

	CqTransformPtr newTrans( new CqTransform( m_pconCurrent->ptransCurrent(), Time(), matTrans, CqTransform::Set() ) );
	m_pconCurrent->ptransSetCurrent( newTrans );
}

void	CqRenderer::ptransSetCurrentTime( const CqMatrix& matTrans )
{
	assert(m_pconCurrent);

	CqTransformPtr newTrans( new CqTransform( m_pconCurrent->ptransCurrent(), Time(), matTrans, CqTransform::SetCurrent() ) );
	m_pconCurrent->ptransSetCurrent( newTrans );
}

void	CqRenderer::ptransConcatCurrentTime( const CqMatrix& matTrans )
{
	assert(m_pconCurrent);

	CqTransformPtr newTrans( new CqTransform( m_pconCurrent->ptransCurrent(), Time(), matTrans, CqTransform::ConcatCurrent() ) );
	m_pconCurrent->ptransSetCurrent( newTrans );
}


//----------------------------------------------------------------------
/** Render all surface in the current list to the image buffer.
 */

void CqRenderer::RenderWorld(bool clone)
{
	// While rendering, all primitives should fasttrack straight into the pipeline, 
	// and shaders should automatically initialise, the easiest way to ensure this
	// is to switch into 'non' multipass mode.
	TqInt multiPass = 0;
	TqInt* pMultipass = GetIntegerOptionWrite("Render", "multipass");
	if(pMultipass)
	{
		multiPass = pMultipass[0];
		pMultipass[0] = 0;
	}

	initialiseCropWindow();

	// Ensure that the camera and projection matrices are initialised.
	poptCurrent()->InitialiseCamera();
	pImage()->SetImage();

	PrepareShaders();

	if(clone)
		PostCloneOfWorld();
	else
		PostWorld();

	m_pDDManager->OpenDisplays(m_cropWindowXMax - m_cropWindowXMin, m_cropWindowYMax - m_cropWindowYMin);
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
		for(TqLightMap::iterator ilight = m_lights.begin(),
			lend = m_lights.end(); ilight != lend; ++ilight)
		{
			CqLightsourcePtr light = ilight->second;
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
				opts->GetIntegerOptionWrite( "System", "DisplayMode" ) [ 0 ] = DMode_Z;

				// Set the pixel samples to 1,1 for shadow rendering.
				opts->GetIntegerOptionWrite( "System", "PixelSamples" ) [ 0 ] = 1;
				opts->GetIntegerOptionWrite( "System", "PixelSamples" ) [ 1 ] = 1;

				// Set the pixel filter to box, 1,1 for shadow rendering.
				opts->SetfuncFilter( RiBoxFilter );
				opts->GetFloatOptionWrite( "System", "FilterWidth" ) [ 0 ] = 1;
				opts->GetFloatOptionWrite( "System", "FilterWidth" ) [ 1 ] = 1;

				// Turn off jitter for shadow rendering.
				opts->GetIntegerOptionWrite("Hider", "jitter")[0] = 0;

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
				AddDisplayRequest(pMapName[0].c_str(), "shadow", "z", DMode_Z, 0, 1, args);

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

				CqTextureMapOld::FlushCache();
				m_textureCache->flush();
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


bool	CqRenderer::matSpaceToSpace( const char* strFrom, const char* strTo, const IqTransform* transShaderToWorld, const IqTransform* transObjectToWorld, TqFloat time, CqMatrix& result )
{
	CqMatrix	matA, matB;

	// Get the hash keys for From,To spaces
	const TqUlong fhash = CqString::hash( strFrom );
	const TqUlong thash = CqString::hash( strTo );

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
		if(!WhichMatToWorld( matA, fhash ))
			return(false);
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
		if(!WhichMatWorldTo( matB, thash ))
			return(false);
	}

	result = matB * matA;

	return ( true );
}



//----------------------------------------------------------------------
/** Get the matrix to convert vectors between the specified coordinate systems.
 */

bool	CqRenderer::matVSpaceToSpace( const char* strFrom, const char* strTo, const IqTransform* transShaderToWorld, const IqTransform* transObjectToWorld, TqFloat time, CqMatrix& result )
{
	CqMatrix	matA, matB;

	// Get the hash keys for From,To spaces
	const TqUlong fhash = CqString::hash( strFrom );
	const TqUlong thash = CqString::hash( strTo );

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
		if(!WhichMatToWorld ( matA, fhash ))
			return(false);
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
		if(!WhichMatWorldTo ( matB, thash ))
			return(false);
	}

	result = matB * matA;



	if (memcmp((void *) oldkey[0].pElements(), (void *) result.pElements(), sizeof(TqFloat) * 16) != 0)
	{
		oldkey[0] = result;
		result[ 3 ][ 0 ] = result[ 3 ][ 1 ] = result[ 3 ][ 2 ] = result[ 0 ][ 3 ] = result[ 1 ][ 3 ] = result[ 2 ][ 3 ] = 0.0;
		result[ 3 ][ 3 ] = 1.0;
		oldresult[0] = result;

	}
	else
	{
		result = oldresult[0];
	}
	return ( true );
}


//----------------------------------------------------------------------
/** Get the matrix to convert normals between the specified coordinate systems.
 */

bool	CqRenderer::matNSpaceToSpace( const char* strFrom, const char* strTo, const IqTransform* transShaderToWorld, const IqTransform* transObjectToWorld, TqFloat time, CqMatrix& result )
{
	CqMatrix	matA, matB;

	// Get the hash keys for From,To spaces
	const TqUlong fhash = CqString::hash( strFrom );
	const TqUlong thash = CqString::hash( strTo );

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
		if(!WhichMatToWorld ( matA, fhash ))
			return(false);
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
		if(!WhichMatWorldTo ( matB, thash ))
			return(false);
	}


	result = matB * matA;
	if (memcmp((void *) oldkey[1].pElements(), (void *) result.pElements(), sizeof(TqFloat) * 16) != 0)
	{
		oldkey[1] = result;
		result[ 3 ][ 0 ] = result[ 3 ][ 1 ] = result[ 3 ][ 2 ] = result[ 0 ][ 3 ] = result[ 1 ][ 3 ] = result[ 2 ][ 3 ] = 0.0;
		result[ 3 ][ 3 ] = 1.0;
		result = result.Inverse().Transpose();
		oldresult[1] = result;

	}
	else
	{
		result = oldresult[1];
	}

	return ( true );
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
	const TqUlong hash = CqString::hash( strName );
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
		pMapCheck->PrepareDefArgs();
		return pMapCheck;
	}

	// insert the default surface template into the map
	boost::shared_ptr<IqShader> pShader = createShaderVM(this);
	pShader->SetType( Type_Surface );
	pShader->SetstrName( "_def_" );
	pShader->DefaultSurface();
	pShader->SetTransform( ptransCurrent() );
	pShader->PrepareDefArgs();
	m_Shaders[key] = pShader;

	// return a clone of the default surface template
	boost::shared_ptr<IqShader> newShader(pShader->Clone());
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
	TqShaderMap::const_iterator shadLocation = m_Shaders.find(key);
	if(shadLocation != m_Shaders.end())
	{
		if(!shadLocation->second)
			return boost::shared_ptr<IqShader>();
		// The shader template is present, so return its clone
		boost::shared_ptr<IqShader> newShader(shadLocation->second->Clone());
		newShader->SetType( type );
		m_InstancedShaders.push_back(newShader);
		return (newShader);
	}

	// we now create the shader...

	// search in the current directory first
	std::string fileName(strName);
	fileName += RI_SHADER_EXTENSION;
	boost::filesystem::path shaderPath
		= poptCurrent()->findRiFileNothrow(fileName, "shader");
	boost::filesystem::ifstream shaderFile(shaderPath);
	if(shaderFile)
	{
		Aqsis::log() << info << "Loading shader \"" << strName
			<< "\" from file \"" << native(shaderPath)
			<< "\"" << std::endl;

		std::string dsoPath;
		const CqString* poptDSOPath = QGetRenderContext()->poptCurrent()
			->GetStringOption( "searchpath", "shader" );
		if(poptDSOPath)
		{
			dsoPath = poptDSOPath->c_str();
			Aqsis::log() << info << "DSO lib path set to \"" << dsoPath
				<< "\"" << std::endl;
		}

		boost::shared_ptr<IqShader> pShader;
		try
		{
			pShader = createShaderVM(this, shaderFile, dsoPath);
		}
		catch(XqBadShader& e)
		{
			Aqsis::log() << error << "could not load shader \"" << strName << "\": "
				<< e.what() << "\n";
			// couldn't load the shader; put a null pointer in the map so we
			// don't try again, and return null.
			m_Shaders[key] = boost::shared_ptr<IqShader>();
			return boost::shared_ptr<IqShader>();
		}

		pShader->SetstrName( strName );
		// add the shader to the map as a template and return its clone
		m_Shaders[key] = pShader;
		boost::shared_ptr<IqShader> newShader(pShader->Clone());
		newShader->SetType( type );
		m_InstancedShaders.push_back(newShader);
		return (newShader);
	}
	else
	{
		if( strcmp(strName, "null") != 0  &&  strcmp(strName, "_def_") != 0 )
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
			boost::shared_ptr<IqShader> pShader = createShaderVM(this);

			pShader->SetType( type );
			pShader->SetstrName( "null" );
			pShader->DefaultSurface();

			// add the shader to the map and return its clone
			m_Shaders[key] = pShader;
			boost::shared_ptr<IqShader> newShader(pShader->Clone());
			newShader->SetType( type );
			m_InstancedShaders.push_back(newShader);
			return (newShader);
		}
		else
		{
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
		CqMatrix matWtoC, matNWtoC, matVWtoC;
		QGetRenderContext() ->matSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0, matWtoC );
		QGetRenderContext() ->matNSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0, matNWtoC );
		QGetRenderContext() ->matVSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0, matVWtoC );
		pSurface->Transform( matWtoC, matNWtoC, matVWtoC);
		pSurface->PrepareTrimCurve();
		PostSurface(pSurface);
	}
}

void CqRenderer::PostWorld()
{
	while(!m_aWorld.empty())
	{
		boost::shared_ptr<CqSurface> pSurface = m_aWorld.front();
		
		CqMatrix matWtoC, matNWtoC, matVWtoC;
		QGetRenderContext() ->matSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0, matWtoC );
		QGetRenderContext() ->matNSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0, matNWtoC );
		QGetRenderContext() ->matVSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0, matVWtoC );
		pSurface->Transform( matWtoC, matNWtoC, matVWtoC);
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
		CqMatrix matWtoC, matNWtoC, matVWtoC;
		QGetRenderContext() ->matSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0, matWtoC );
		QGetRenderContext() ->matNSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0, matNWtoC );
		QGetRenderContext() ->matVSpaceToSpace( "world", "camera", NULL, pSurface->pTransform().get(), 0, matVWtoC );
		pSurface->Transform( matWtoC, matNWtoC, matVWtoC);
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
	if(bound.vecCross().Magnitude2() > 0)
	{
		CqMatrix mat;
		QGetRenderContext() ->matSpaceToSpace( "object", "raster", NULL, pSurface->pTransform().get(), QGetRenderContext()->Time(), mat );
		bound.Transform( mat );

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
			minImportance = clamp(( rangeAttr[1] - ruler ) / ( rangeAttr[1] - rangeAttr[0] ), 0.0f, 1.0f);

		TqFloat maxImportance;
		if ( rangeAttr[2] == rangeAttr[3] )
			maxImportance = ruler < rangeAttr[2] ? 1.0f : 0.0f;
		else
			maxImportance = clamp((rangeAttr[3] - ruler)/(rangeAttr[3] - rangeAttr[2]), 0.0f, 1.0f);

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

void CqRenderer::registerLight(const char* name, CqLightsourcePtr light)
{
	m_lights[name] = light;
}

CqLightsourcePtr CqRenderer::findLight(const char* name)
{
	TqLightMap::iterator i = m_lights.find(name);
	if(i == m_lights.end())
		AQSIS_THROW_XQERROR(XqValidation, EqE_BadHandle,
				"unknown light \"" << name << "\" encountered");
	return i->second;
}

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


IqTextureCache& CqRenderer::textureCache()
{
	return *m_textureCache;
}

IqTextureMapOld* CqRenderer::GetEnvironmentMap( const CqString& strFileName )
{
	return ( CqTextureMapOld::GetEnvironmentMap( strFileName ) );
}

IqTextureMapOld* CqRenderer::GetOcclusionMap( const CqString& strFileName )
{
	return ( CqTextureMapOld::GetShadowMap( strFileName ) );
}

IqTextureMapOld* CqRenderer::GetLatLongMap( const CqString& strFileName )
{
	return ( CqTextureMapOld::GetLatLongMap( strFileName ) );
}

const char* CqRenderer::textureSearchPath()
{
	const CqString* pathPtr = poptCurrent()->GetStringOption("searchpath", "texture");
	if(pathPtr)
		return pathPtr->c_str();
	else
		return "";
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
bool CqRenderer::WhichMatToWorld( CqMatrix &matA, TqUlong thash )
{
	static TqInt awhich = 0;
	TqInt tmp = awhich;


	for ( ; awhich >= 0; awhich-- )
	{
		if ( m_aCoordSystems[ awhich ].m_hash == thash )
		{
			matA = m_aCoordSystems[ awhich ].m_matToWorld;
			return(true);
		}
	}

	TqInt size = m_aCoordSystems.size() - 1;
	for ( awhich = size; awhich > tmp; awhich-- )
	{
		if ( m_aCoordSystems[ awhich ].m_hash == thash )
		{
			matA = m_aCoordSystems[ awhich ].m_matToWorld;
			return(true);
		}
	}
	return(false);
}

//---------------------------------------------------------------------
/** Which matrix will be used in WorldTo
 */

bool CqRenderer::WhichMatWorldTo( CqMatrix &matB, TqUlong thash )
{
	static TqInt bwhich = 0;
	TqInt tmp = bwhich;


	for ( ; bwhich >= 0; bwhich-- )
	{
		if ( m_aCoordSystems[ bwhich ].m_hash == thash )
		{
			matB = m_aCoordSystems[ bwhich ].m_matWorldTo;
			return(true);
		}
	}

	TqInt size = m_aCoordSystems.size() - 1;
	for ( bwhich = size; bwhich > tmp; bwhich-- )
	{
		if ( m_aCoordSystems[ bwhich ].m_hash == thash )
		{
			matB = m_aCoordSystems[ bwhich ].m_matWorldTo;
			return(true);
		}
	}
	return(false);
}


TqInt CqRenderer::RegisterOutputData( const char* name )
{
	TqInt offset;
	if( ( offset = OutputDataIndex( name ) ) != -1 )
		return(offset);

	std::string baseName;
	Ri::TypeSpec spec = m_tokenDict.lookup(name, &baseName);
	if(spec.type == Ri::TypeSpec::Unknown || spec.type == Ri::TypeSpec::String)
		AQSIS_THROW_XQERROR(XqValidation, EqE_BadToken,
			"Cannot use \"" << name << "\" as an AOV");
	if(spec.arraySize != 1)
		AQSIS_THROW_XQERROR(XqValidation, EqE_BadToken,
			"Cannot use array \"" << name << "\" as an AOV");

	TqInt NumSamples = spec.storageCount();
	SqOutputDataEntry DataEntry;

	DataEntry.m_Offset = m_OutputDataOffset;
	DataEntry.m_NumSamples = NumSamples;
	m_OutputDataOffset += NumSamples;
	m_OutputDataTotalSize += NumSamples;

	// Add the new entry to the map, using the token name as the key.
	m_OutputDataEntries[baseName] = DataEntry;

	return DataEntry.m_Offset;
}

TqInt CqRenderer::OutputDataIndex( const char* name )
{
	const SqOutputDataEntry* entry = FindOutputDataEntry(name);
	if(!entry)
		return -1;
	return( entry->m_Offset );
}

TqInt CqRenderer::OutputDataSamples( const char* name )
{
	const SqOutputDataEntry* entry = FindOutputDataEntry(name);
	if(!entry)
		return 0;
	return entry->m_NumSamples;
}

/** \brief Found the AOV data entry corresponding to the given name
 *
 * \param name - name of the AOV data
 * \return the data entry or 0 if not found
 */
const CqRenderer::SqOutputDataEntry* CqRenderer::FindOutputDataEntry(const char* name)
{
	std::string baseName;
	Ri::TypeSpec spec;
	try
	{
		spec = m_tokenDict.lookup(name, &baseName);
	}
	catch(XqValidation& e)
	{
		Aqsis::log() << error << e.what() << std::endl;
		return 0;
	}
	std::map<std::string, SqOutputDataEntry>::iterator entry
		= m_OutputDataEntries.find(baseName);
	if( entry != m_OutputDataEntries.end() )
		return &entry->second;
	return 0;
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

const TqFloat CqRenderer::MinCoCForBound(const CqBound& bound) const
{
	assert(m_UsingDepthOfField);
	TqFloat z1 = bound.vecMin().z();
	TqFloat z2 = bound.vecMax().z();
	// First check whether the bound spans the focal plane; if so, return 0.
	TqFloat focalDist = 1/m_OneOverFocalDistance;
	if((z1 - focalDist)*(z2 - focalDist) < 0)
		return 0;
	// Otherwise, the minimum focal blur is achieved at one of the
	// z-extents of the bound.
	TqFloat minBlur = min(std::fabs(1/z1 - m_OneOverFocalDistance),
				std::fabs(1/z2 - m_OneOverFocalDistance));
	return m_DofMultiplier * min(m_DepthOfFieldScale.x(),
			m_DepthOfFieldScale.y()) * minBlur;
}

void CqRenderer::initialiseCropWindow()
{
	TqInt iXRes = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "Resolution" ) [ 0 ];
	TqInt iYRes = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "Resolution" ) [ 1 ];
	m_cropWindowXMin = clamp<TqInt>(lceil( iXRes * QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "CropWindow" ) [ 0 ] ), 0, iXRes);
	m_cropWindowXMax = clamp<TqInt>(lceil( iXRes * QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "CropWindow" ) [ 1 ] ), 0, iXRes);
	m_cropWindowYMin = clamp<TqInt>(lceil( iYRes * QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "CropWindow" ) [ 2 ] ), 0, iYRes);
	m_cropWindowYMax = clamp<TqInt>(lceil( iYRes * QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "CropWindow" ) [ 3 ] ), 0, iYRes);
}

//---------------------------------------------------------------------

} // namespace Aqsis

