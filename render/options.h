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
		\brief Declares the classes and structures for handling RenderMan options.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is options.h included already?
#ifndef OPTIONS_H_INCLUDED 
//{
#define OPTIONS_H_INCLUDED 1

#include	<vector>

#include	"ri.h"
#include	"matrix.h"
#include	"sstring.h"
#include	"refcount.h"
#include	"color.h"
#include	"exception.h"
#include	"parameters.h"
#include	"shadervm.h"


#define  _qShareName CORE
#include "share.h"

START_NAMESPACE( Aqsis )


class CqImagersource;

//----------------------------------------------------------------------
/** \class CqSystemOption
 * Renderman option/attribute class, has a name and a number of parameter name/value pairs.
 */

class CqSystemOption : public CqRefCount
{
	public:
		CqSystemOption( const char* strName ) : m_strName( strName )
		{}
		CqSystemOption( const CqSystemOption& From );
		~CqSystemOption()
		{
			for ( std::vector<CqParameter*>::iterator i = m_aParameters.begin(); i != m_aParameters.end(); i++ )
				delete( ( *i ) );
		}

		/** Get a refernece to the option name.
		 * \return A constant CqString reference.
		 */
		const	CqString&	strName() const
		{
			return ( m_strName );
		}

		/** Add a new name/value pair to this option/attribute.
		 * \param pParameter Pointer to a CqParameter containing the name/value pair.
		 */
		void	AddParameter( const CqParameter* pParameter )
		{
			// Look to see if one already exists
			for ( std::vector<CqParameter*>::iterator i = m_aParameters.begin(); i != m_aParameters.end(); i++ )
			{
				if ( ( *i ) ->strName().compare( pParameter->strName() ) == 0 )
				{
					delete( *i );
					( *i ) = const_cast<CqParameter*>( pParameter );
					return ;
				}
			}
			// If not append it.
			m_aParameters.push_back( const_cast<CqParameter*>( pParameter ) );
		}
		/** Get a read only pointer to a named parameter.
		 * \param strName Character pointer pointing to zero terminated parameter name.
		 * \return A pointer to a CqParameter or 0 if not found.
		 */
		const	CqParameter* pParameter( const char* strName ) const
		{
			for ( std::vector<CqParameter*>::const_iterator i = m_aParameters.begin(); i != m_aParameters.end(); i++ )
				if ( ( *i ) ->strName().compare( strName ) == 0 ) return ( *i );
			return ( 0 );
		}
		/** Get a pointer to a named parameter.
		 * \param strName Character pointer pointing to zero terminated parameter name.
		 * \return A pointer to a CqParameter or 0 if not found.
		 */
		CqParameter* pParameter( const char* strName )
		{
			for ( std::vector<CqParameter*>::iterator i = m_aParameters.begin(); i != m_aParameters.end(); i++ )
				if ( ( *i ) ->strName().compare( strName ) == 0 ) return ( *i );
			return ( 0 );
		}
	private:
		CqString	m_strName;			///< The name of this option/attribute.
		std::vector<CqParameter*>	m_aParameters;		///< A vector of name/value parameters.
}
;


//----------------------------------------------------------------------
/** \enum EqDisplayMode
 */
enum EqDisplayMode
{
    ModeNone = 0x0000, 	///< Invalid.
    ModeRGB = 0x0001, 		///< Red Green and Blue channels.
    ModeA = 0x0002, 		///< Alpha channel.
    ModeZ = 0x0004		///< Depth channel.
};

//----------------------------------------------------------------------
/** \class CqDisplay
 * Class to hold state information relating to the camera and frame output.
 */

class CqDisplay
{
	public:
		CqDisplay() : m_PixelXSamples( 2 ),
				m_PixelYSamples( 2 ),
				m_funcFilter( RiGaussianFilter ),
				m_fFilterXWidth( 2.0 ),
				m_fFilterYWidth( 2.0 ),
				m_fExposureGain( 1.0 ),
				m_fExposureGamma( 1.0 ),
				m_strImager( "null" ),
				m_pshadImager( NULL),
				m_iColorQuantizeOne( 255 ),
				m_iColorQuantizeMin( 0 ),
				m_iColorQuantizeMax( 255 ),
				m_fColorQuantizeDitherAmplitude( 0.5 ),
				m_iDepthQuantizeOne( 0 ),
				m_iDepthQuantizeMin( 0 ),
				m_iDepthQuantizeMax( 255 ),
				m_fDepthQuantizeDitherAmplitude( 0 ),
				m_strDisplayType( "file" ),
				m_strDisplayName( "aqsis.tif" ),
				m_iDisplayMode( ModeRGB )
		{}

		virtual	~CqDisplay()
		{ 
			DeleteImager();
		}

		/** Get the pixel variance as a float.
		 */
		TqFloat	fPixelVariance() const
		{
			return ( m_fPixelVariance );
		}
		/** Set the pixel variance.
		 * \param fValue the new value for the pixel variance option.
		 */
		void	SetfPixelVariance( const TqFloat fValue )
		{
			m_fPixelVariance = fValue;
		}
		/** Get the number of pixel samples in x.
		 */
		TqInt	PixelXSamples() const
		{
			return ( m_PixelXSamples );
		}
		/** Set the number of pixel samples in x.
		 * \param fValue the new pixel sample count in x.
		 */
		void	SetPixelXSamples( const TqInt Value )
		{
			m_PixelXSamples = Value;
		}
		/** Get the number of pixel samples in y.
		 */
		TqInt	PixelYSamples() const
		{
			return ( m_PixelYSamples );
		}
		/** Set the number of pixel samples in y.
		 * \param fValue the new pixel sample count in y.
		 */
		void	SetPixelYSamples( const TqInt Value )
		{
			m_PixelYSamples = Value;
		}
		/** Get a pointer to the pixel filter function.
		 */
		RtFilterFunc funcFilter() const
		{
			return ( m_funcFilter );
		}
		/** Set the pixel filter function to use.
		 * \param fValue A pointer to a function which follows the RtFilterFunc convention.
		 */
		void	SetfuncFilter( const RtFilterFunc fValue )
		{
			m_funcFilter = fValue;
		}
		/** Get the pixel filter x width.
		 */
		TqFloat	fFilterXWidth() const
		{
			return ( m_fFilterXWidth );
		}
		/** Set the pixel filter x width.
		 * \param fValue Float pixel filter width.
		 */
		void	SetfFilterXWidth( const TqFloat fValue )
		{
			m_fFilterXWidth = fValue;
		}
		/** Get the pixel filter y width.
		 */
		TqFloat	fFilterYWidth() const
		{
			return ( m_fFilterYWidth );
		}
		/** Set the pixel filter y width.
		 * \param fValue Float pixel filter width.
		 */
		void	SetfFilterYWidth( const TqFloat fValue )
		{
			m_fFilterYWidth = fValue;
		}
		/** Get the exposure gain value.
		 */
		TqFloat	fExposureGain() const
		{
			return ( m_fExposureGain );
		}
		/** Set the exposure gain value.
		 * \param fValue Float exposure gain.
		 */
		void	SetfExposureGain( const TqFloat fValue )
		{
			m_fExposureGain = fValue;
		}
		/** Get the exposure gamma value.
		 */
		TqFloat	fExposureGamma() const
		{
			return ( m_fExposureGamma );
		}
		/** Set the exposure gamma value.
		 * \param fValue Float exposure gamma.
		 */
		void	SetfExposureGamma( const TqFloat fValue )
		{
			m_fExposureGamma = fValue;
		}
		/** Get the name of the Imager shader.
		 */
		const CqString&	strImager() const
		{
			return ( m_strImager );
		}
		/** Get a pointer to the imager shader.
		 */
		const CqImagersource*	pshadImager() const
		{
			return ( m_pshadImager );
		}
		/** Set the name of the Imager shader.
		 * \param strValue Character pointer to name of Imager shader.
		 */
		void	LoadImager( const char* strValue );
		void DeleteImager();
		void	SetstrImager( const char* strValue )
		{
			m_strImager = strValue;
			LoadImager( strValue );
		}
		void SetValueImager( char *token, char *value );

		/** Get the one value for the color quantisation.
		 */
		TqInt	iColorQuantizeOne() const
		{
			return ( m_iColorQuantizeOne );
		}
		/** Set the color quantisation one value.
		 * \param fValue Float one value.
		 */
		void	SetiColorQuantizeOne( const TqInt iValue )
		{
			m_iColorQuantizeOne = iValue;
		}
		/** Get the min value for the color quantisation.
		 */
		TqInt	iColorQuantizeMin() const
		{
			return ( m_iColorQuantizeMin );
		}
		/** Set the color quantisation min value.
		 * \param fValue Float min value.
		 */
		void	SetiColorQuantizeMin( const TqInt iValue )
		{
			m_iColorQuantizeMin = iValue;
		}
		/** Get the max value for the color quantisation.
		 */
		TqInt	iColorQuantizeMax() const
		{
			return ( m_iColorQuantizeMax );
		}
		/** Set the color quantisation max value.
		 * \param fValue Float max value.
		 */
		void	SetiColorQuantizeMax( const TqInt iValue )
		{
			m_iColorQuantizeMax = iValue;
		}
		/** Get the dither amplitude for the color quantisation.
		 */
		TqFloat	fColorQuantizeDitherAmplitude() const
		{
			return ( m_fColorQuantizeDitherAmplitude );
		}
		/** Set the color quantisation dither amplitude.
		 * \param fValue Float dither amplitude.
		 */
		void	SetfColorQuantizeDitherAmplitude( const TqFloat fValue )
		{
			m_fColorQuantizeDitherAmplitude = fValue;
		}
		/** Get the one value for the depth quantisation.
		 */
		TqInt	iDepthQuantizeOne() const
		{
			return ( m_iDepthQuantizeOne );
		}
		/** Set the depth quantisation one value.
		 * \param fValue Float one value.
		 */
		void	SetiDepthQuantizeOne( const TqInt iValue )
		{
			m_iDepthQuantizeOne = iValue;
		}
		/** Get the min value for the depth quantisation.
		 */
		TqInt	iDepthQuantizeMin() const
		{
			return ( m_iDepthQuantizeMin );
		}
		/** Set the depth quantisation min value.
		 * \param fValue Float min value.
		 */
		void	SetiDepthQuantizeMin( const TqInt iValue )
		{
			m_iDepthQuantizeMin = iValue;
		}
		/** Get the max value for the depth quantisation.
		 */
		TqInt	iDepthQuantizeMax() const
		{
			return ( m_iDepthQuantizeMax );
		}
		/** Set the depth quantisation max value.
		 * \param fValue Float max value.
		 */
		void	SetiDepthQuantizeMax( const TqInt iValue )
		{
			m_iDepthQuantizeMax = iValue;
		}
		/** Get the dither amplitude for the depth quantisation.
		 */
		TqFloat	fDepthQuantizeDitherAmplitude() const
		{
			return ( m_fDepthQuantizeDitherAmplitude );
		}
		/** Set the depth quantisation dither amplitude.
		 * \param fValue Float dither amplitude.
		 */
		void	SetfDepthQuantizeDitherAmplitude( const TqFloat fValue )
		{
			m_fDepthQuantizeDitherAmplitude = fValue;
		}
		/** Get the name of the display type.
		 */
		const CqString&	strDisplayType() const
		{
			return ( m_strDisplayType );
		}
		/** Set the name of the display type to use.
		 * \param strValue Character pointer to display type name.
		 */
		void	SetstrDisplayType( const char* strValue )
		{
			m_strDisplayType = strValue;
		}
		/** Get the name to give to the display driver.
		 */
		const CqString&	strDisplayName() const
		{
			return ( m_strDisplayName );
		}
		/** Set the name of the display.
		 * \param strValue Character pointer to display name.
		 */
		void	SetstrDisplayName( const char* strValue )
		{
			m_strDisplayName = strValue;
		}
		/** Get the display mode, from EqDisplayMode.
		 */
		TqInt	iDisplayMode() const
		{
			return ( m_iDisplayMode );
		}
		/** Set the display mode.
		 * \param iValue Integer display mode from EqDisplayMode.
		 */
		void	SetiDisplayMode( const TqInt iValue )
		{
			m_iDisplayMode = iValue;
		}
		void InitialiseColorImager( TqInt bx, TqInt by,
		                            TqFloat x, TqFloat y,
		                            CqColor *color, CqColor *opacity,
		                            TqFloat *depth, TqFloat *coverage );
		CqColor GetColorImager( TqFloat x, TqFloat y );
		CqColor GetOpacityImager( TqFloat x, TqFloat y );
		TqFloat GetAlphaImager( TqFloat x, TqFloat y );

	private:
		TqFloat	m_fPixelVariance;					///< Pixel variance (not used).
		TqInt	m_PixelXSamples, 					///< Pixel samples in x.
		m_PixelYSamples;					///< Pixel samples in y.
		RtFilterFunc m_funcFilter;						///< Pointer to the pixel filter function.
		TqFloat	m_fFilterXWidth, 					///< Pixel filter width in x.
		m_fFilterYWidth;					///< Pixel filter width in y.
		TqFloat	m_fExposureGain, 					///< Exposure gain value.
		m_fExposureGamma;					///< Exposure gamma value.
		CqString	m_strImager;						///< Name of the Imager shader.
		CqImagersource* m_pshadImager;		///< Pointer to the imager shader.
		TqInt	m_iColorQuantizeOne, 				///< Color quantisation one value.
		m_iColorQuantizeMin, 				///< Color quantisation min value.
		m_iColorQuantizeMax;				///< Color quantisation max value.
		TqFloat	m_fColorQuantizeDitherAmplitude;	///< Color quantisation dither amplitude.
		TqInt	m_iDepthQuantizeOne, 				///< Depth quantisation one value.
		m_iDepthQuantizeMin, 				///< Depth quantisation min value.
		m_iDepthQuantizeMax;				///< Depth quantisation max value.
		TqFloat	m_fDepthQuantizeDitherAmplitude;	///< Depth quantisation dither amplitude.
		CqString	m_strDisplayType;					///< Name of the display type to use.
		CqString	m_strDisplayName;					///< Name of the display.
		TqInt	m_iDisplayMode;						///< Display mode, from EqDisplayMode.
}
;


//----------------------------------------------------------------------
/** \enum EqProjection
 * Possible projection modes for the camera.
 */
enum EqProjection
{
    ProjectionOrthographic, 		///< Orthographic projection.
    ProjectionPerspective		///< Perspective projection.
};


//----------------------------------------------------------------------
/** \class CqCamera
 * Describe the camera and all of its required information.
 */

class CqCamera
{
	public:
		CqCamera();
		virtual	~CqCamera()
		{}

		/** Get the x resolution of the image.
		 */
		TqInt	iXResolution() const
		{
			return ( m_iXResolution );
		}
		/** Set the x resolution of the image.
		 * \param iValue The new x resolution.
		 */
		void	SetiXResolution( const TqInt iValue )
		{
			m_iXResolution = iValue;
		}
		/** Get the y resolution of the image.
		 */
		TqInt	iYResolution() const
		{
			return ( m_iYResolution );
		}
		/** Set the y resolution of the image.
		 * \param iValue The new y resolution.
		 */
		void	SetiYResolution( const TqInt iValue )
		{
			m_iYResolution = iValue;
		}
		/** Get the pixel aspect ratio of the image.
		 */
		TqFloat	fPixelAspectRatio() const
		{
			return ( m_fPixelAspectRatio );
		}
		/** Set the pixel aspect ratio.
		 * \param fValue Float aspect ratio.
		 */
		void	SetfPixelAspectRatio( const TqFloat fValue )
		{
			m_fPixelAspectRatio = fValue;
		}
		/** Get the crop window x minimum as a fraction of the x resolution.
		 */
		TqFloat	fCropWindowXMin() const
		{
			return ( m_fCropWindowXMin );
		}
		/** Set the crop window x minimum as a fraction of the x resolution.
		 * \param fValue Float value.
		 */
		void	SetfCropWindowXMin( const TqFloat fValue )
		{
			m_fCropWindowXMin = fValue;
		}
		/** Get the crop window x maximum as a fraction of the x resolution.
		 */
		TqFloat	fCropWindowXMax() const
		{
			return ( m_fCropWindowXMax );
		}
		/** Set the crop window x maximum as a fraction of the x resolution.
		 * \param fValue Float value.
		 */
		void	SetfCropWindowXMax( const TqFloat fValue )
		{
			m_fCropWindowXMax = fValue;
		}
		/** Get the crop window y minimum as a fraction of the y resolution.
		 */
		TqFloat	fCropWindowYMin() const
		{
			return ( m_fCropWindowYMin );
		}
		/** Set the crop window y minimum as a fraction of the y resolution.
		 * \param fValue Float value.
		 */
		void	SetfCropWindowYMin( const TqFloat fValue )
		{
			m_fCropWindowYMin = fValue;
		}
		/** Get the crop window y maximum as a fraction of the y resolution.
		 */
		TqFloat	fCropWindowYMax() const
		{
			return ( m_fCropWindowYMax );
		}
		/** Set the crop window y maximum as a fraction of the y resolution.
		 * \param fValue Float value.
		 */
		void	SetfCropWindowYMax( const TqFloat fValue )
		{
			m_fCropWindowYMax = fValue;
		}
		/** Get the frame aspect ratio.
		 */
		TqFloat	fFrameAspectRatio() const
		{
			return ( m_fFrameAspectRatio );
		}
		/** Set the frame aspect ratio.
		 * \param fValue Float value.
		 */
		void	SetfFrameAspectRatio( const TqFloat fValue )
		{
			m_fFrameAspectRatio = fValue;
		}
		/** Get screen window left value.
		 */
		TqFloat	fScreenWindowLeft() const
		{
			return ( m_fScreenWindowLeft );
		}
		/** Set the screen window left extreme.
		 * \param fValue Float value.
		 */
		void	SetfScreenWindowLeft( const TqFloat fValue )
		{
			m_fScreenWindowLeft = fValue;
		}
		/** Get screen window right value.
		 */
		TqFloat	fScreenWindowRight() const
		{
			return ( m_fScreenWindowRight );
		}
		/** Set the screen window right extreme.
		 * \param fValue Float value.
		 */
		void	SetfScreenWindowRight( const TqFloat fValue )
		{
			m_fScreenWindowRight = fValue;
		}
		/** Get screen window top value.
		 */
		TqFloat	fScreenWindowTop() const
		{
			return ( m_fScreenWindowTop );
		}
		/** Set the screen window top extreme.
		 * \param fValue Float value.
		 */
		void	SetfScreenWindowTop( const TqFloat fValue )
		{
			m_fScreenWindowTop = fValue;
		}
		/** Get screen window bottom value.
		 */
		TqFloat	fScreenWindowBottom() const
		{
			return ( m_fScreenWindowBottom );
		}
		/** Set the screen window bottom extreme.
		 * \param fValue Float value.
		 */
		void	SetfScreenWindowBottom( const TqFloat fValue )
		{
			m_fScreenWindowBottom = fValue;
		}
		/** Get the camera projection mode, from EqProjection.
		 */
		EqProjection eCameraProjection() const
		{
			return ( m_eCameraProjection );
		}
		/** Set the camera projection mode.
		 * \param fValue Integer value from EqProjection.
		 */
		void	SeteCameraProjection( const EqProjection eValue )
		{
			m_eCameraProjection = eValue;
		}
		/** Get the near clipping plane distance.
		 */
		TqFloat	fClippingPlaneNear() const
		{
			return ( m_fClippingPlaneNear );
		}
		/** Set the near clipping distance from the camera.
		 * \param fValue Float value.
		 */
		void	SetfClippingPlaneNear( const TqFloat fValue )
		{
			m_fClippingPlaneNear = fValue;
		}
		/** Get the far clipping plane distance.
		 */
		TqFloat	fClippingPlaneFar() const
		{
			return ( m_fClippingPlaneFar );
		}
		/** Set the far clipping distance from the camera.
		 * \param fValue Float value.
		 */
		void	SetfClippingPlaneFar( const TqFloat fValue )
		{
			m_fClippingPlaneFar = fValue;
		}
		/** Get the calculated clipping range.
		 */
		TqFloat	fClippingRange() const
		{
			return ( m_fClippingRange );
		}
		/** Set the clipping range.
		 * \param fValue Float value.
		 */
		void	SetfClippingRange( const TqFloat fValue )
		{
			m_fClippingRange = fValue;
		}
		/** Get the camera lens fstop value.
		 */
		TqFloat	ffStop() const
		{
			return ( m_ffStop );
		}
		/** Set the camera fStop value.
		 * \param fValue Float value.
		 */
		void	SetffStop( const TqFloat fValue )
		{
			m_ffStop = fValue;
		}
		/** Get the camera lens focal length.
		 */
		TqFloat	fFocalLength() const
		{
			return ( m_fFocalLength );
		}
		/** Set the camera focal length.
		 * \param fValue Float value.
		 */
		void	SetfFocalLength( const TqFloat fValue )
		{
			m_fFocalLength = fValue;
		}
		/** Get the camera lens focal distance.
		 */
		TqFloat	fFocalDistance() const
		{
			return ( m_fFocalDistance );
		}
		/** Set the camera focal distance.
		 * \param fValue Float value.
		 */
		void	SetfFocalDistance( const TqFloat fValue )
		{
			m_fFocalDistance = fValue;
		}
		/** Get the camera shutter open time.
		 */
		TqFloat	fShutterOpen() const
		{
			return ( m_fShutterOpen );
		}
		/** Set the camera shutter open time.
		 * \param fValue Float value.
		 */
		void	SetfShutterOpen( const TqFloat fValue )
		{
			m_fShutterOpen = fValue;
		}
		/** Get the camera shutter close time.
		 */
		TqFloat	fShutterClose() const
		{
			return ( m_fShutterClose );
		}
		/** Set the camera shutter close time.
		 * \param fValue Float value.
		 */
		void	SetfShutterClose( const TqFloat fValue )
		{
			m_fShutterClose = fValue;
		}
		/** Get the camera field of view.
		 */
		TqFloat	fFOV() const
		{
			return ( m_fFOV );
		}
		/** Set the camera field of view.
		 * \param fValue Float value.
		 */
		void	SetfFOV( const TqFloat fFOV )
		{
			m_fFOV = fFOV;
		}

		void	InitialiseCamera();

		/** Indicate that the RiFormat function has been called.
		 * This and other similar are used to overcome the problem with multiple ways to specify frame dimensions.
		 */
		void	CallFormat()
		{
			m_bFormatCalled = TqTrue;
		}
		/** Indicate that the RiScreenWindow function has been called.
		 * This and other similar are used to overcome the problem with multiple ways to specify frame dimensions.
		 */
		void	CallScreenWindow()
		{
			m_bScreenWindowCalled = TqTrue;
		}
		/** Indicate that the RiFrameAspectRatio function has been called.
		 * This and other similar are used to overcome the problem with multiple ways to specify frame dimensions.
		 */
		void	CallFrameAspectRatio()
		{
			m_bFrameAspectRatioCalled = TqTrue;
		}
		/** Determine if the RiFormat function has been called.
		 * This and other similar are used to overcome the problem with multiple ways to specify frame dimensions.
		 */
		TqBool	FormatCalled() const
		{
			return ( m_bFormatCalled );
		}
		/** Determine if the RiScreenWindow function has been called.
		 * This and other similar are used to overcome the problem with multiple ways to specify frame dimensions.
		 */
		TqBool	ScreenWindowCalled() const
		{
			return ( m_bScreenWindowCalled );
		}
		/** Determine if the RiFrameAspectRatio function has been called.
		 * This and other similar are used to overcome the problem with multiple ways to specify frame dimensions.
		 */
		TqBool	FrameAspectRatioCalled() const
		{
			return ( m_bFrameAspectRatioCalled );
		}

	private:
		TqInt	m_iXResolution, 					///< X resolution of the raster plane.
		m_iYResolution;					///< Y resolution of the raster plane.
		TqFloat	m_fPixelAspectRatio;			///< Pixel aspect ratio of the raster plane.
		TqFloat	m_fCropWindowXMin, 				///< Crop window min x value as a fraction of the total x resolution.
		m_fCropWindowXMax, 				///< Crop window max x value as a fraction of the total x resolution.
		m_fCropWindowYMin, 				///< Crop window min y value as a fraction of the total y resolution.
		m_fCropWindowYMax;				///< Crop window max y value as a fraction of the total y resolution.
		TqFloat	m_fFrameAspectRatio;			///< Frame aspect ratio.
		TqFloat	m_fScreenWindowLeft, 			///< Screen window on the projection plane to be rendered.
		m_fScreenWindowRight, 			///< Screen window on the projection plane to be rendered.
		m_fScreenWindowTop, 				///< Screen window on the projection plane to be rendered.
		m_fScreenWindowBottom;			///< Screen window on the projection plane to be rendered.
		EqProjection m_eCameraProjection;			///< Projection mode, from EqProjection.
		TqFloat	m_fClippingPlaneNear, 			///< Distance of the near clipping plane from the camera.
		m_fClippingPlaneFar, 			///< Distance of the far clipping plane from the camera.
		m_fClippingRange;				///< Calculated clipping range for quantisation.
		TqFloat	m_ffStop, 						///< Camera lens fStop value.
		m_fFocalLength, 					///< Camera lens focal length.
		m_fFocalDistance;				///< Camera lens focal distance.
		TqFloat	m_fShutterOpen, 					///< Camera shutter open time.
		m_fShutterClose;				///< Camera shutter close time.
		TqFloat	m_fFOV;							///< Camera field of view.

		TqBool	m_bFrameAspectRatioCalled, 		///< Indicate RiFrameAspectRatio has been called. Calculation of the screen geometry is reliant on which of these have been called.
		m_bScreenWindowCalled, 			///< Indicate RiScreenWindow has been called. Calculation of the screen geometry is reliant on which of these have been called.
		m_bFormatCalled;				///< Indicate RiFormat has been called. Calculation of the screen geometry is reliant on which of these have been called.
}
;


//----------------------------------------------------------------------
/** \class CqOptions
 * Storage for the graphics state options.
 */

class CqOptions : public CqDisplay, public CqCamera
{
	public:
		CqOptions();
		CqOptions( const CqOptions& From );
		~CqOptions();

		CqOptions&	operator=( const CqOptions& From );

		/** Add a named user option to the current state.
		 * \param pOption A pointer to a new CqSystemOption class containing the initialised option.
		 */
		void	AddOption( CqSystemOption* pOption )
		{
			m_aOptions.push_back( pOption );
		}
		/** Clear all user options from the state.
		 */
		void	ClearOptions()
		{
			m_aOptions.clear();
		}
		/** Get a read only pointer to a named user option.
		 * \param strName Character pointer to the requested options name.
		 * \return A pointer to the option, or 0 if not found. 
		 */
		const	CqSystemOption* pOption( const char* strName ) const
		{
			for ( std::vector<CqSystemOption*>::const_iterator i = m_aOptions.begin(); i != m_aOptions.end(); i++ )
				if ( ( *i ) ->strName().compare( strName ) == 0 ) return ( *i );
			return ( 0 );
		}
		/** Get a pointer to a named user option.
		 * \param strName Character pointer to the requested options name.
		 * \return A pointer to the option, or 0 if not found. 
		 */
		CqSystemOption* pOptionWrite( const char* strName )
		{
			for ( std::vector<CqSystemOption*>::iterator i = m_aOptions.begin(); i != m_aOptions.end(); i++ )
			{
				if ( ( *i ) ->strName().compare( strName ) == 0 )
				{
					if ( ( *i ) ->RefCount() == 1 )
						return ( *i );
					else
					{
						( *i ) ->Release();
						( *i ) = new CqSystemOption( *( *i ) );
						( *i ) ->AddRef();
						return ( *i );
					}
				}
			}
			m_aOptions.push_back( new CqSystemOption( strName ) );
			m_aOptions.back() ->AddRef();
			return ( m_aOptions.back() );
		}
		const	CqParameter* pParameter( const char* strName, const char* strParam ) const;
		CqParameter* pParameterWrite( const char* strName, const char* strParam );
		/** Get the name of the hidden surface mode.
		 */
		const CqString&	strHider()
		{
			return ( m_strHider );
		}
		/** Set the name of the hidden surface mode.
		 */
		void	SetstrHider( const char* hider )
		{
			m_strHider = hider;
		}
		/** Get the number of samples in a color value.
		 */
		TqInt	iColorSamples()
		{
			return ( m_iColorSamples );
		}
		/** Set the number of samples per color value.
		 * \param samples Integer color samples.
		 */
		void	SetiColorSamples( TqInt samples )
		{
			m_iColorSamples = samples;
		}
		/** Get the Relative detail value.
		 */
		TqFloat	fRelativeDetail()
		{
			return ( m_fRelativeDetail );
		}
		/** Set the relative detail to use when rendering.
		 * \param detail The new relative detail setting.
		 */
		void	SetfRelativeDetail( TqFloat detail )
		{
			m_fRelativeDetail = detail;
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

		RtFunc	pPreRenderFunction()
		{
			return ( m_pPreRenderFunction );
		}
		/** Set the progress handler function to use.
		 * \param pprogresshandler A pointer to a function which conforms to RtProgressFunc.
		 */
		void	SetpPreRenderFunction( RtFunc pfunction )
		{
			m_pPreRenderFunction = pfunction;
		}

		const	TqFloat*	GetFloatOption( const char* strName, const char* strParam ) const;
		const	TqInt*	GetIntegerOption( const char* strName, const char* strParam ) const;
		const	CqString* GetStringOption( const char* strName, const char* strParam ) const;
		const	CqVector3D*	GetPointOption( const char* strName, const char* strParam ) const;
		const	CqColor*	GetColorOption( const char* strName, const char* strParam ) const;

		TqFloat*	GetFloatOptionWrite( const char* strName, const char* strParam );
		TqInt*	GetIntegerOptionWrite( const char* strName, const char* strParam );
		CqString* GetStringOptionWrite( const char* strName, const char* strParam );
		CqVector3D*	GetPointOptionWrite( const char* strName, const char* strParam );
		CqColor*	GetColorOptionWrite( const char* strName, const char* strParam );

	private:
		CqString	m_strHider;				///< The name of the hidden surface mode.
		TqInt	m_iColorSamples;		///< The number of samples per color value.
		TqFloat	m_fRelativeDetail;		///< The relative detail setting.
		RtErrorFunc	m_pErrorHandler;		///< A pointer to the error hadling function.
		RtProgressFunc	m_pProgressHandler;		///< A pointer to the progress hadling function.
		RtFunc	m_pPreRenderFunction;	///< A pointer to the function called just prior to rendering.
		std::vector<CqSystemOption*>	m_aOptions;				///< Vector of user specified options.
}
;


END_NAMESPACE( Aqsis )

//-----------------------------------------------------------------------
//}  // End of #ifdef OPTIONS_H_INCLUDED
#endif
