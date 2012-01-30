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
		\brief Declares the base CqRenderer class which is the central core of the rendering main loop.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is renderer.h included already?
#ifndef RENDERER_H_INCLUDED
//{
#define RENDERER_H_INCLUDED 1

#include	<vector>
#include	<iostream>
#include	<time.h>

#include	<aqsis/aqsis.h>

#include	<aqsis/ri/ri.h>
#include	"graphicsstate.h"
#include	"stats.h"
#include	<aqsis/math/vector2d.h>

#include	"shaders.h"
#include	<aqsis/riutil/tokendictionary.h>
#include	"iddmanager.h"
#include	<aqsis/core/irenderer.h>
#include	"iraytrace.h"
#include	"iraytrace.h"
#include	<aqsis/tex/filtering/itexturecache.h>
#include	"lights.h"

#include	"clippingvolume.h"

namespace Aqsis {

class CqImageBuffer;
class CqModeBlock;

struct SqCoordSys
{
	SqCoordSys( const char* strName, const CqMatrix& matToWorld, const CqMatrix& matWorldTo ) :
			m_matWorldTo( matWorldTo ),
			m_matToWorld( matToWorld ),
			m_strName( strName )
	{
		m_hash = CqString::hash((char *) strName);
	}
	SqCoordSys()
	{}

	CqMatrix	m_matWorldTo;
	CqMatrix	m_matToWorld;
	CqString	m_strName;
	TqUlong         m_hash;
};

enum EqCoordSystems
{
    CoordSystem_Camera = 0,
    CoordSystem_Current,
    CoordSystem_World,
    CoordSystem_Screen,
    CoordSystem_NDC,
    CoordSystem_Raster,

    CoordSystem_Last,
};

enum EqRenderMode
{
    RenderMode_Image = 0,

    RenderMode_Shadows,
    RenderMode_Reflection,
};


//----------------------------------------------------------------------
/** \class  CqRenderer
 * The main renderer control class.  Contains all information relating to
 * the current image being rendered.  There is only ever one of these,
 * statically defined in the CPP file for this class, and globally available.
 *
 * \todo <b>Code Review</b> The renderer class is large and does many disparate
 * tasks.  It should be broken up into a bunch of helper classes which perform
 * the details of each task.  These helper classes can be members of CqRenderer.
 */

class CqRenderer;
AQSIS_CORE_SHARE extern CqRenderer* pCurrRenderer;

class CqRenderer : public IqRenderer
{
	public:
		CqRenderer();
		virtual	~CqRenderer();

		virtual	boost::shared_ptr<CqModeBlock>	BeginMainModeBlock();
		virtual	boost::shared_ptr<CqModeBlock>	BeginFrameModeBlock();
		virtual	boost::shared_ptr<CqModeBlock>	BeginWorldModeBlock();
		virtual	boost::shared_ptr<CqModeBlock>	BeginAttributeModeBlock();
		virtual	boost::shared_ptr<CqModeBlock>	BeginTransformModeBlock();
		virtual	boost::shared_ptr<CqModeBlock>	BeginSolidModeBlock( CqString& type );
		virtual	boost::shared_ptr<CqModeBlock>	BeginObjectModeBlock();
		virtual	boost::shared_ptr<CqModeBlock>	BeginMotionModeBlock( TqInt N, TqFloat times[] );
		virtual	boost::shared_ptr<CqModeBlock>	BeginResourceModeBlock();

		virtual	void	EndMainModeBlock();
		virtual	void	EndFrameModeBlock();
		virtual	void	EndWorldModeBlock();
		virtual	void	EndAttributeModeBlock();
		virtual	void	EndTransformModeBlock();
		virtual	void	EndSolidModeBlock();
		virtual	void	EndObjectModeBlock();
		virtual	void	EndMotionModeBlock();
		virtual	void	EndResourceModeBlock();

		virtual	const IqOptionsPtr	poptCurrent() const;
		virtual	IqOptionsPtr	poptWriteCurrent();
		virtual IqOptionsPtr	pushOptions();
		virtual IqOptionsPtr	popOptions();
		virtual	CqAttributesPtr	pattrCurrent() const;
		virtual	CqAttributesPtr	pattrWriteCurrent() const;
		virtual	CqTransformPtr	ptransCurrent() const;
		void	ptransSetTime( const CqMatrix& matTrans );
		void	ptransSetCurrentTime( const CqMatrix& matTrans );
		void	ptransConcatCurrentTime( const CqMatrix& matTrans );

		virtual	TqFloat	Time() const;
		virtual	void	AdvanceTime();

		/** Set a pointer to the current context.
		 * Primarily for Procedural objects
		 * \return Pointer to a previous CqModeBlock.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	pconCurrent(const boost::shared_ptr<CqModeBlock>& pcon )
		{
			boost::shared_ptr<CqModeBlock> prev = m_pconCurrent;
			m_pconCurrent = pcon;
			return ( prev );
		}
		/** Get a pointer to the current context.
		 * \return Pointer to a CqModeBlock derived class.
		 */
		virtual	boost::shared_ptr<CqModeBlock>	pconCurrent()
		{
			return ( m_pconCurrent );
		}
		/** Get a erad only pointer to the current context.
		 * \return Pointer to a CqModeBlock derived class.
		 */
		virtual const	boost::shared_ptr<CqModeBlock>	pconCurrent() const
		{
			return ( m_pconCurrent );
		}
		/** Get a pointer to the current image buffer.
		 * \return A CqImageBuffer pointer.
		 */
		virtual CqImageBuffer* pImage()
		{
			return ( m_pImageBuffer );
		}

		// Handle various coordinate system transformation requirements.
		virtual	bool	matSpaceToSpace	( const char* strFrom, const char* strTo, const IqTransform* transShaderToWorld, const IqTransform* transObjectToWorld, TqFloat time, CqMatrix& result );
		virtual	bool	matVSpaceToSpace	( const char* strFrom, const char* strTo, const IqTransform* transShaderToWorld, const IqTransform* transObjectToWorld, TqFloat time, CqMatrix& result );
		virtual	bool	matNSpaceToSpace	( const char* strFrom, const char* strTo, const IqTransform* transShaderToWorld, const IqTransform* transObjectToWorld, TqFloat time, CqMatrix& result );

		virtual	const	TqFloat*	GetFloatOption( const char* strName, const char* strParam ) const;
		virtual	const	TqInt*	GetIntegerOption( const char* strName, const char* strParam ) const;
		virtual	const	CqString* GetStringOption( const char* strName, const char* strParam ) const;
		virtual	const	CqVector3D*	GetPointOption( const char* strName, const char* strParam ) const;
		virtual	const	CqColor*	GetColorOption( const char* strName, const char* strParam ) const;

		virtual	TqFloat*	GetFloatOptionWrite( const char* strName, const char* strParam );
		virtual	TqInt*	GetIntegerOptionWrite( const char* strName, const char* strParam );
		virtual	CqString* GetStringOptionWrite( const char* strName, const char* strParam );
		virtual	CqVector3D*	GetPointOptionWrite( const char* strName, const char* strParam );
		virtual	CqColor*	GetColorOptionWrite( const char* strName, const char* strParam );

		virtual	void	PrintString( const char* str )
		{
			std::cout << str;
		}

		virtual	IqTextureCache& textureCache();
		virtual	IqTextureMapOld* GetEnvironmentMap( const CqString& strFileName );
		virtual	IqTextureMapOld* GetOcclusionMap(const CqString& fileName);
		virtual	IqTextureMapOld* GetLatLongMap( const CqString& strFileName );


		/** \brief Return the current texture search path.
		 *
		 * This is used as a callback function by the texture library to obtain
		 * the texture search path when necessary.
		 */
		const char* textureSearchPath();

		virtual	bool	GetBasisMatrix( CqMatrix& matBasis, const CqString& name );


		/** Get a read only reference to the current transformation matrix.
		 * \return A constant reference to a CqMatrix.
		 */
		virtual const	CqMatrix&	matCurrent( TqFloat time ) const
		{
			return ( pconCurrent() ->matCurrent( time ) );
		}

		virtual	bool	SetCoordSystem( const char* strName, const CqMatrix& matToWorld );

		// Function which can be overridden by the derived class.
		virtual	void	Initialise();
		virtual	void	RenderWorld(bool clone = false);
		virtual void	RenderAutoShadows();

		virtual	void	AddDisplayRequest( const TqChar* name, const TqChar* type, const TqChar* mode, TqInt modeID, TqInt dataOffset, TqInt dataSize, std::map<std::string, void*>& mapOfArguments );
		virtual	void	ClearDisplayRequests();
		virtual	IqDDManager*	pDDmanager()
		{
			return ( m_pDDManager );
		}

		virtual	void	Quit();
		virtual	void	UpdateStatus()
		{}
		/** Get the global statistics class.
		 * \return A reference to the CqStats class on this renderer.
		 */
		virtual	CqStats&	Stats()
		{
			return ( m_Stats );
		}

		/// Return the current dictionary of declared tokens.
		TokenDict& tokenDict();
		/// Return the current dictionary of declared tokens.
		const TokenDict& tokenDict() const;

		/** Get the list of currently registered shaders.
		 * \return A reference to a list of CqShaderRegister classes.
		 */
		virtual boost::shared_ptr<IqShader> CreateShader( const char* strName, EqShaderType type );

		/** Flush any registered shaders.
		 */
		virtual	void	FlushShaders()
		{
			m_Shaders.clear();
			m_InstancedShaders.clear();
		}

		/** Prepare the shaders for rendering.
		 */
		virtual void	PrepareShaders();

		/// Register a light source with the given name
		void registerLight(const char* name, CqLightsourcePtr light);
		/// Find the light associated with the given name
		CqLightsourcePtr findLight(const char* name);

		void	PostSurface( const boost::shared_ptr<CqSurface>& pSurface );
		void	StorePrimitive( const boost::shared_ptr<CqSurface>& pSurface );
		void	PostWorld();
		void	PostCloneOfWorld();

		/** Set the world to screen matrix.
		 * \param mat The new matrix to use as the world to screen transformation.
		 */
		virtual	void	SetmatScreen( const CqMatrix& mat )
		{
			m_aCoordSystems[ CoordSystem_Screen ].m_matWorldTo = mat;
			if(mat.Determinant() != 0)
				m_aCoordSystems[ CoordSystem_Screen ].m_matToWorld = mat.Inverse();
			else
				m_aCoordSystems[ CoordSystem_Screen ].m_matToWorld = CqMatrix();
		}
		/** Set the world to NDC matrix.
		 * \param mat The new matrix to use as the world to NDC transformation.
		 */
		virtual	void	SetmatNDC( const CqMatrix& mat )
		{
			m_aCoordSystems[ CoordSystem_NDC ].m_matWorldTo = mat;
			if(mat.Determinant() != 0)
				m_aCoordSystems[ CoordSystem_NDC ].m_matToWorld = mat.Inverse();
			else
				m_aCoordSystems[ CoordSystem_NDC ].m_matToWorld = CqMatrix();
		}
		/** Set the world to raster matrix.
		 * \param mat The new matrix to use as the world to raster transformation.
		 */
		virtual	void	SetmatRaster( const CqMatrix& mat )
		{
			m_aCoordSystems[ CoordSystem_Raster ].m_matWorldTo = mat;
			if(mat.Determinant() != 0)
				m_aCoordSystems[ CoordSystem_Raster ].m_matToWorld = mat.Inverse();
			else
				m_aCoordSystems[ CoordSystem_Raster ].m_matToWorld = CqMatrix();
		}
		/** Set the world to camera transform.
		 * \param ptrans A pointer to the transformation object which represents the world to camera transform.
		 */
		virtual	void	SetCameraTransform( const CqTransformPtr& ptrans )
		{
			m_pTransCamera = ptrans;
		}
		/** Get the world to camera tramsform.
		 * \return A pointer to the transformation object which represents the world to camera transform.
		 */
		virtual	CqTransformPtr	GetCameraTransform( )
		{
			return( m_pTransCamera );
		}
		/** Set the initial object transformation, this transform takes into account motion imparted by the camera.
		*  This is used when resetting the object transform in prepareation for a RiTransform.
		 * \param ptrans A pointer to the transformation object which represents the initial object space transform.
		 */
		virtual	void	SetDefObjTransform( const CqTransformPtr& ptrans )
		{
			m_pTransDefObj = ptrans;
		}
		/** Get initial object transformation.
		 * \return A pointer to the transformation object which represents the initial object space transform.
		 */
		virtual	CqTransformPtr	GetDefObjTransform( )
		{
			return( m_pTransDefObj );
		}
		/** Set the pre projection transform.
		 * \param ptrans A pointer to the transformation object which represents the transform prior to projection.
		 */
		virtual	void	SetpreProjectionTransform( const CqTransformPtr& ptrans )
		{
			m_preProjectionTransform = ptrans;
		}
		/** Get the pre projection tramsform.
		 * \return A pointer to the transformation object which represents the the transform prior to projection.
		 */
		virtual	CqTransformPtr	GetpreProjectionTransform( )
		{
			return( m_preProjectionTransform );
		}

		/** Set the lens data associated with depth of field effects.
		    * \param fstop The f-stop of the lens.
		    * \param focalLength The size of the lens.
		    * \param focalDistance The distance at which everything is in focus.
		    */
		void	SetDepthOfFieldData( TqFloat fstop, TqFloat focalLength, TqFloat focalDistance )
		{
			m_UsingDepthOfField = (fstop < FLT_MAX);
			if (fstop < FLT_MAX)
			{
				TqFloat lensDiameter = focalLength / fstop;
				m_DofMultiplier = 0.5 * lensDiameter * focalDistance / (focalDistance + lensDiameter);
				m_OneOverFocalDistance = 1.0 / focalDistance;
			}
		}

		/** Find out if we are using depth of field or not.
		 * \return true if depth of field is turned on.
		 */
		bool	UsingDepthOfField( ) const
		{
			return m_UsingDepthOfField;
		}

		/** Set the scale for dof to transform the coc from camera to raster space
		 * \param x the scale in x
		 * \param y the scale in y
		 */
		void SetDepthOfFieldScale( TqFloat x, TqFloat y )
		{
			m_DepthOfFieldScale.x( x );
			m_DepthOfFieldScale.y( y );
		}

		/** Get the circle of confusion at the specified depth
		 * \param depth The depth in camera space
		 * \return A 2d vector with the radius of the coc in raster space along x and y.
		 */
		const CqVector2D GetCircleOfConfusion( TqFloat depth ) const
		{
			assert(m_UsingDepthOfField);
			TqFloat c = m_DofMultiplier * fabs(1.0f / depth - m_OneOverFocalDistance);
			return CqVector2D(m_DepthOfFieldScale * c);
		}

		/** \brief Return minimum circle of confusion radius for objects inside a bound
		 *
		 * If the bound spans the focal plane 0 is returned.  In other cases
		 * the minimum circle of confusion is nonzero and lies on the bound.
		 */
		const TqFloat MinCoCForBound(const CqBound& bound) const;

		boost::shared_ptr<IqShader> getDefaultSurfaceShader();

		struct SqOutputDataEntry
		{
			SqOutputDataEntry() : m_Offset(0), m_NumSamples(0) {}
			SqOutputDataEntry(TqInt off, TqInt num, TqInt type) : m_Offset(off), m_NumSamples(num) {}
			TqInt	m_Offset;
			TqInt	m_NumSamples;
		};
		TqInt	RegisterOutputData( const char* name );
		TqInt	OutputDataIndex( const char* name );
		TqInt	OutputDataSamples( const char* name );
		TqInt	OutputDataType(const char* name );
		std::map<std::string, SqOutputDataEntry>& GetMapOfOutputDataEntries()
		{
			return( m_OutputDataEntries );
		}


		/** Get the number if samples needed to fulfil the display requests.
		 */
		TqInt	GetOutputDataTotalSize() const
		{
			return( m_OutputDataTotalSize );
		}

		virtual	void	SetCurrentFrame( TqInt FrameNo )
		{
			m_FrameNo = FrameNo;
		}
		virtual	TqInt	CurrentFrame() const
		{
			return( m_FrameNo );
		}

		/** Get a pointer to the error handler function.
		 */
		RtErrorFunc	pErrorHandler()
		{
			return ( m_pErrorHandler );
		}
		/** Set the error handler function to use.
		 * \param perrorhandler A pointer to a function which conforms to RtErrorFunc.
		 */
		void	SetpErrorHandler( RtErrorFunc perrorhandler )
		{
			m_pErrorHandler = perrorhandler;
		}

		/** Get a pointer to the progress handler function.
		 */
		RtProgressFunc	pProgressHandler()
		{
			return ( m_pProgressHandler );
		}
		/** Set the progress handler function to use.
		 * \param pprogresshandler A pointer to a function which conforms to RtProgressFunc.
		 */
		void	SetpProgressHandler( RtProgressFunc pprogresshandler )
		{
			m_pProgressHandler = pprogresshandler;
		}

		/** Get a pointer to the raytracing subsystem
		 */
		IqRaytrace*	pRaytracer() const
		{
			return( m_pRaytracer );
		}

		bool	IsWorldBegin() const
		{
			return(m_fWorldBegin);
		}

		void SetWorldBegin(bool begin = true)
		{
			m_fWorldBegin = begin;
		}

		CqClippingVolume& clippingVolume()
		{
			return(m_clippingVolume);
		}
		const CqClippingVolume& clippingVolume() const
		{
			return(m_clippingVolume);
		}

		void initialiseCropWindow();

		const TqInt cropWindowXMin() const
		{
			return m_cropWindowXMin;
		}
		const TqInt cropWindowXMax() const
		{
			return m_cropWindowXMax;
		}
		const TqInt cropWindowYMin() const
		{
			return m_cropWindowYMin;
		}
		const TqInt cropWindowYMax() const
		{
			return m_cropWindowYMax;
		}

	private:
		const SqOutputDataEntry* FindOutputDataEntry(const char* name);

		/// Map type to hold loaded reference shaders.
		typedef std::map< CqShaderKey, boost::shared_ptr<IqShader> > TqShaderMap;

		boost::shared_ptr<CqModeBlock>	m_pconCurrent;					///< Pointer to the current context.
		CqStats	m_Stats;						///< Global statistics.
		CqAttributesPtr	m_pAttrDefault;					///< Default attributes.
		CqOptionsPtr m_poptDefault;  					///< Pointer to default options.
		CqTransformPtr	m_pTransDefault;				///< Default transformation.
		CqImageBuffer*	m_pImageBuffer;					///< Pointer to the current image buffer.

		IqDDManager*	m_pDDManager;

		EqRenderMode	m_Mode;
		TqShaderMap m_Shaders;
		std::vector< boost::shared_ptr<IqShader> >  m_InstancedShaders;

		typedef std::map<std::string, CqLightsourcePtr> TqLightMap;
		TqLightMap m_lights;

		boost::shared_ptr<IqTextureCache> m_textureCache; ///< Cache for aqsistex texture access.
		 

		bool	m_fSaveGPrims;
		CqTransformPtr	m_pTransCamera;					///< The camera transform.
		CqTransformPtr	m_pTransDefObj;				///< The initial transformation for objects.
		CqTransformPtr	m_preProjectionTransform;	///< The transformation that was applied prior to projection.
		bool			m_fWorldBegin;
		/// Renderman symbol table
		TokenDict m_tokenDict;

		/// Variables for depth of field.  \todo Move these to a DoF calculator object.
		TqFloat			m_DofMultiplier;
		TqFloat			m_OneOverFocalDistance;
		bool			m_UsingDepthOfField;
		CqVector2D		m_DepthOfFieldScale;

		bool WhichMatWorldTo(CqMatrix &a, TqUlong thash);
		bool WhichMatToWorld(CqMatrix &b, TqUlong thash);

		std::map<std::string, SqOutputDataEntry>	m_OutputDataEntries;
		TqInt	m_OutputDataOffset;
		TqInt	m_OutputDataTotalSize;


		TqInt	m_FrameNo;

		// Error Handling
		RtErrorFunc m_pErrorHandler; ///< pointer to the error handling function
		bool        m_abortRender;   ///< bool to signal the renderer to cleanup and exit

		RtProgressFunc	m_pProgressHandler;		///< A pointer to the progress hadling function.

		IqRaytrace*	m_pRaytracer;		///< Pointer to the raytracing subsystem interface.

		CqClippingVolume	m_clippingVolume;

		std::deque<boost::shared_ptr<CqSurface> >	m_aWorld;

		// Cached calculated cropwindow coordinates in raster space.
		TqInt				m_cropWindowXMin;
		TqInt				m_cropWindowXMax;
		TqInt				m_cropWindowYMin;
		TqInt				m_cropWindowYMax;

		std::vector<SqCoordSys>	m_aCoordSystems; ///< List of registered coordinate systems.
}
;


inline CqRenderer* QGetRenderContext()
{
	return ( pCurrRenderer );
}


void	QSetRenderContext( CqRenderer* pRenderer );


//==============================================================================
// Implementation details
//==============================================================================

inline TokenDict& CqRenderer::tokenDict()
{
	return m_tokenDict;
}

inline const TokenDict& CqRenderer::tokenDict() const
{
	return m_tokenDict;
}

} // namespace Aqsis

//}  // End of #ifdef RENDERER_H_INCLUDED
#endif
