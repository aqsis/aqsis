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
		\brief Implements the classes and structures for handling RenderMan options.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<math.h>

#include	"aqsis.h"
#include	"options.h"
#include	"renderer.h"
#include	"imagebuffer.h"
#include	"imagers.h"
#include	"ri.h"

START_NAMESPACE( Aqsis )


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqSystemOption::CqSystemOption( const CqSystemOption& From ) :
		m_strName( From.m_strName )
{
	TqInt i = From.m_aParameters.size();
	while ( i-- > 0 )
	{
		m_aParameters.push_back( From.m_aParameters[ i ] ->Clone() );
	}
}



//---------------------------------------------------------------------
/** Initialise the matrices for this camera according to the status of the camera attributes.
 */

void CqOptions::InitialiseCamera()
{
	// Setup the Normalising and projection matrices according to the projection
	// type
	CqMatrix	matCameraToScreen;
	CqMatrix	matScreenToCamera;
	CqMatrix	matScreenToNDC;
	CqMatrix	matNDCToRaster;
	TqInt proj = GetIntegerOption("System", "CameraProjection")[0];
	switch ( proj )
	{
			case	ProjectionOrthographic:
			{
				// Define a matrix to convert from right top left handed coordinate systems.
				CqMatrix Trl( 1, 1, -1 );

				TqFloat l = GetFloatOption("System", "ScreenWindowLeft")[0];
				TqFloat r = GetFloatOption("System", "ScreenWindowRight")[0];
				TqFloat t = GetFloatOption("System", "ScreenWindowTop")[0];
				TqFloat b = GetFloatOption("System", "ScreenWindowBottom")[0];
				TqFloat n = GetFloatOption("System", "ClippingPlaneNear")[0];
				TqFloat f = GetFloatOption("System", "ClippingPlaneFar")[0];

				matCameraToScreen.Identity();
				matCameraToScreen.SetfIdentity( TqFalse );
				matCameraToScreen.SetElement( 0, 0, 2.0f / ( r - l ) );
				matCameraToScreen.SetElement( 3, 0, ( r + l ) / ( r - l ) );
				matCameraToScreen.SetElement( 1, 1, 2.0f / ( t - b ) );
				matCameraToScreen.SetElement( 3, 1, ( t + b ) / ( t - b ) );
				matCameraToScreen.SetElement( 2, 2, -2.0f / ( f - n ) );
				matCameraToScreen.SetElement( 3, 2, ( f + n ) / ( f - n ) );
				matCameraToScreen.SetElement( 2, 3, 0 );
				matCameraToScreen.SetElement( 3, 3, 1 );

				// Combine with the right to left matrix.
				matCameraToScreen *= Trl;

				// Set up the screen to frame matrix
				TqFloat	FrameX = ( GetFloatOption("System", "FrameAspectRatio")[0] >= 1.0 ) ? GetIntegerOption("System", "XResolution")[0] :
				                 ( GetIntegerOption("System", "XResolution")[0] * GetFloatOption("System", "FrameAspectRatio")[0] ) / GetFloatOption("System", "PixelAspectRatio")[0];
				TqFloat	FrameY = ( GetIntegerOption("System", "FrameAspectRatio")[0] < 1.0 ) ? GetIntegerOption("System", "YResolution")[0] :
				                 ( GetIntegerOption("System", "XResolution")[0] * GetFloatOption("System", "PixelAspectRatio")[0] ) / GetFloatOption("System", "FrameAspectRatio")[0];
				matScreenToNDC.Identity();
				matNDCToRaster.Identity();
				// Translate from -1,-1-->1,1 to 0,0-->2,2
				CqMatrix	T;
				T.Translate( 1, 1, 0 );
				// Scale by 0.5 (0,0 --> 1,1) NDC
				CqMatrix	S( 0.5, 0.5, 0 );
				CqMatrix	S2( FrameX, FrameY, 0 );
				// Invert y to fit top down format
				CqMatrix	S3( 1, -1, 1 );
				matScreenToNDC = S * T * S3; // S*T*S2
				matNDCToRaster = S2;

				break;
			}

			case	ProjectionPerspective:
			{
				TqFloat fov = GetFloatOption("System", "ClippingPlaneNear")[0] * ( tan( RAD( GetFloatOption("System", "FOV")[0] / 2.0f ) ) );
				TqFloat l = GetFloatOption("System", "ScreenWindowLeft")[0] * fov;
				TqFloat r = GetFloatOption("System", "ScreenWindowRight")[0] * fov;
				TqFloat t = GetFloatOption("System", "ScreenWindowTop")[0] * fov;
				TqFloat b = GetFloatOption("System", "ScreenWindowBottom")[0] * fov;
				TqFloat n = GetFloatOption("System", "ClippingPlaneNear")[0];
				TqFloat f = GetFloatOption("System", "ClippingPlaneFar")[0];

				matCameraToScreen.Identity();
				matCameraToScreen.SetfIdentity( TqFalse );
				matCameraToScreen.SetElement( 0, 0, ( 2.0f * n ) / ( r - l ) );
				matCameraToScreen.SetElement( 2, 0, ( r + l ) / ( r - l ) );
				matCameraToScreen.SetElement( 1, 1, ( 2.0f * n ) / ( t - b ) );
				matCameraToScreen.SetElement( 2, 1, ( t + b ) / ( t - b ) );
				TqFloat a = f / ( f - n );
				//			matCameraToScreen.SetElement(2,2,-((f+n)/(f-n)));
				matCameraToScreen.SetElement( 2, 2, a );
				//			matCameraToScreen.SetElement(3,2,-((2.0f*f*n)/(f-n)));
				matCameraToScreen.SetElement( 3, 2, -n * a );
				matCameraToScreen.SetElement( 2, 3, 1 );
				matCameraToScreen.SetElement( 3, 3, 0 );

				// Set up the screen to frame matrix
				TqFloat	FrameX = ( GetFloatOption("System", "FrameAspectRatio")[0] >= 1.0 ) ? GetIntegerOption("System", "XResolution")[0] :
				                 ( GetIntegerOption("System", "YResolution")[0] * GetFloatOption("System", "FrameAspectRatio")[0] ) / GetFloatOption("System", "PixelAspectRatio")[0];
				TqFloat	FrameY = ( GetFloatOption("System", "FrameAspectRatio")[0] < 1.0 ) ? GetIntegerOption("System", "YResolution")[0] :
				                 ( GetIntegerOption("System", "XResolution")[0] * GetFloatOption("System", "PixelAspectRatio")[0] ) / GetFloatOption("System", "FrameAspectRatio")[0];

				matScreenToNDC.Identity();
				matNDCToRaster.Identity();
				// Translate from -1,-1-->1,1 to 0,0-->2,2
				CqMatrix	T;
				T.Translate( 1.0f, 1.0f, 0.0f );
				// Scale by 0.5 (0,0 --> 1,1) NDC
				CqMatrix	S( 0.5f, 0.5f, 1.0f );
				CqMatrix	S2( FrameX, FrameY, 1.0f );
				// Invert y to fit top down format
				CqMatrix	S3( 1.0f, -1.0f, 1.0f );
				matScreenToNDC = S * T * S3; // S*T*S2
				matNDCToRaster = S2;

				break;
			}
	}
	CqMatrix matWorldToCamera( QGetRenderContext() ->matWorldToCamera() );
	QGetRenderContext() ->SetmatScreen( matCameraToScreen * matWorldToCamera );
	QGetRenderContext() ->SetmatNDC( matScreenToNDC * matCameraToScreen * matWorldToCamera );
	QGetRenderContext() ->SetmatRaster( matNDCToRaster * matScreenToNDC * matCameraToScreen * matWorldToCamera );

	CqMatrix matWorldToScreen = matCameraToScreen * matWorldToCamera;

	CqVector3D	vecf( 0, 0, 7 );
	CqVector3D	vecn( 0, 0, -2 );

	vecf = vecf * matWorldToScreen;
	vecn = vecn * matWorldToScreen;

	// Set some additional information about the clip range.
	m_fClippingRange = GetFloatOption("System", "ClippingPlaneFar")[0] - GetFloatOption("System", "ClippingPlaneNear")[0];
}


//---------------------------------------------------------------------
/** Default constructor.
 */

#define	ADD_SYSTEM_PARAM(name, type, id, def) \
	CqParameterTypedUniform<type,id>* p##name = new CqParameterTypedUniform<type,id>(#name); \
	p##name->pValue()[0] = ( def ); \
	pdefopts->AddParameter(p##name);


CqOptions::CqOptions() :
	m_pErrorHandler( &RiErrorPrint ),
	m_pProgressHandler( NULL ),
	m_pPreRenderFunction( NULL ),
	m_funcFilter( RiGaussianFilter ),
	m_pshadImager( NULL)
{
	InitialiseDefaultOptions();
}

//---------------------------------------------------------------------
/** Copy constructor.
 */

CqOptions::CqOptions( const CqOptions& From )
{
	*this = From;
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqOptions::~CqOptions()
{
	// Unreference the system options.
	TqInt i = m_aOptions.size();
	while ( i-- > 0 )
	{
		m_aOptions[ i ] ->Release();
		m_aOptions[ i ] = 0;
	}

	DeleteImager();
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqOptions& CqOptions::operator=( const CqOptions& From )
{
	m_pErrorHandler = From.m_pErrorHandler;
	m_pProgressHandler = From.m_pProgressHandler;
	m_pPreRenderFunction = From.m_pPreRenderFunction;

	// Copy the system options.
	m_aOptions.resize( From.m_aOptions.size() );
	TqInt i = From.m_aOptions.size();
	while ( i-- > 0 )
	{
		m_aOptions[ i ] = From.m_aOptions[ i ];
		m_aOptions[ i ] ->AddRef();
	}

	return ( *this );
}


void CqOptions::InitialiseDefaultOptions()
{
	CqSystemOption*  pdefopts = new CqSystemOption("System");

	ADD_SYSTEM_PARAM(PixelVariance, TqFloat, type_float, 1.0f );
	ADD_SYSTEM_PARAM(PixelSamplesX, TqInt, type_integer, 2 );
	ADD_SYSTEM_PARAM(PixelSamplesY, TqInt, type_integer, 2 );
	ADD_SYSTEM_PARAM(FilterWidthX, TqFloat, type_float, 2.0f );
	ADD_SYSTEM_PARAM(FilterWidthY, TqFloat, type_float, 2.0f );
	ADD_SYSTEM_PARAM(ExposureGain, TqFloat, type_float, 1.0f );
	ADD_SYSTEM_PARAM(ExposureGamma, TqFloat, type_float, 1.0f );
	ADD_SYSTEM_PARAM(ColorQuantizeOne, TqInt, type_integer, 255 );
	ADD_SYSTEM_PARAM(ColorQuantizeMin, TqInt, type_integer, 0 );
	ADD_SYSTEM_PARAM(ColorQuantizeMax, TqInt, type_integer, 2255 );
	ADD_SYSTEM_PARAM(ColorQuantizeDitherAmplitude, TqFloat, type_float, 0.5f );
	ADD_SYSTEM_PARAM(DepthQuantizeOne, TqInt, type_integer, 0 );
	ADD_SYSTEM_PARAM(DepthQuantizeMin, TqInt, type_integer, 0 );
	ADD_SYSTEM_PARAM(DepthQuantizeMax, TqInt, type_integer, 255 );
	ADD_SYSTEM_PARAM(DepthQuantizeDitherAmplitude, TqFloat, type_float, 0.0f );
	ADD_SYSTEM_PARAM(DisplayType, CqString, type_string, "file" );
	ADD_SYSTEM_PARAM(DisplayName, CqString, type_string, "aqsis.tif" );
	ADD_SYSTEM_PARAM(DisplayMode, TqInt, type_integer, ModeRGB );

	ADD_SYSTEM_PARAM(Hider,CqString, type_string, "hidden" );
	ADD_SYSTEM_PARAM(ColorSamples, TqInt, type_integer, 3 );
	ADD_SYSTEM_PARAM(RelativeDetail, TqFloat, type_float, 1.0f );

	ADD_SYSTEM_PARAM(XResolution, TqInt, type_integer, 640 );
	ADD_SYSTEM_PARAM(YResolution, TqInt, type_integer, 480 );
	ADD_SYSTEM_PARAM(PixelAspectRatio, TqFloat, type_float, 1.0f );
	ADD_SYSTEM_PARAM(CropWindowMinX, TqFloat, type_float, 0.0f );
	ADD_SYSTEM_PARAM(CropWindowMinY, TqFloat, type_float, 0.0f );
	ADD_SYSTEM_PARAM(CropWindowMaxX, TqFloat, type_float, 1.0f );
	ADD_SYSTEM_PARAM(CropWindowMaxY, TqFloat, type_float, 1.0f );
	ADD_SYSTEM_PARAM(FrameAspectRatio, TqFloat, type_float, 4.0f / 3.0f );
	ADD_SYSTEM_PARAM(ScreenWindowLeft, TqFloat, type_float, -( 4.0f / 3.0f ) );
	ADD_SYSTEM_PARAM(ScreenWindowRight, TqFloat, type_float, 4.0f / 3.0f );
	ADD_SYSTEM_PARAM(ScreenWindowTop, TqFloat, type_float, 1.0f );
	ADD_SYSTEM_PARAM(ScreenWindowBottom, TqFloat, type_float, -1.0f );
	ADD_SYSTEM_PARAM(CameraProjection, TqInt, type_integer, ProjectionOrthographic );
	ADD_SYSTEM_PARAM(ClippingPlaneNear, TqFloat, type_float, FLT_EPSILON );
	ADD_SYSTEM_PARAM(ClippingPlaneFar, TqFloat, type_float, FLT_MAX );
	ADD_SYSTEM_PARAM(FStop, TqFloat, type_float, , FLT_MAX );
	ADD_SYSTEM_PARAM(FocalLength, TqFloat, type_float, , FLT_MAX );
	ADD_SYSTEM_PARAM(FocalDistance, TqFloat, type_float, FLT_MAX );
	ADD_SYSTEM_PARAM(ShutterOpen, TqFloat, type_float, 0.0f );
	ADD_SYSTEM_PARAM(ShutterClose, TqFloat, type_float, 1.0f );
	ADD_SYSTEM_PARAM(FOV, TqFloat, type_float, 90.0f );

	AddOption(pdefopts);
}


//---------------------------------------------------------------------
/** Get a system option parameter, takes name and parameter name.
 * \param strName The name of the option.
 * \param strParam The name of the paramter on the option.
 * \return CqParameter pointer or 0 if not found.
 */

const CqParameter* CqOptions::pParameter( const char* strName, const char* strParam ) const
{
	const CqSystemOption * pOpt;
	if ( ( pOpt = pOption( strName ) ) != 0 )
	{
		const CqParameter * pParam;
		if ( ( pParam = pOpt->pParameter( strParam ) ) != 0 )
			return ( pParam );
	}
	return ( 0 );
}


//---------------------------------------------------------------------
/** Get a system option parameter, takes name and parameter name.
 * \param strName The name of the option.
 * \param strParam The name of the paramter on the option.
 * \return CqParameter pointer or 0 if not found.
 */

CqParameter* CqOptions::pParameterWrite( const char* strName, const char* strParam )
{
	CqSystemOption * pOpt;
	if ( ( pOpt = pOptionWrite( strName ) ) != 0 )
	{
		CqParameter * pParam;
		if ( ( pParam = pOpt->pParameter( strParam ) ) != 0 )
			return ( pParam );
	}
	return ( 0 );
}


//---------------------------------------------------------------------
/** Get a float system option parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return Float pointer 0 if not found.
 */

TqFloat* CqOptions::GetFloatOptionWrite( const char* strName, const char* strParam )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 )
		return ( static_cast<CqParameterTyped<TqFloat>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get an integer system option parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return Integer pointer 0 if not found.
 */

TqInt* CqOptions::GetIntegerOptionWrite( const char* strName, const char* strParam )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 )
		return ( static_cast<CqParameterTyped<TqInt>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a string system option parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqString pointer 0 if not found.
 */

CqString* CqOptions::GetStringOptionWrite( const char* strName, const char* strParam )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 )
		return ( static_cast<CqParameterTyped<CqString>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a point system option parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqVector3D pointer 0 if not found.
 */

CqVector3D* CqOptions::GetPointOptionWrite( const char* strName, const char* strParam )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 )
		return ( static_cast<CqParameterTyped<CqVector3D>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a color system option parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqColor pointer 0 if not found.
 */

CqColor* CqOptions::GetColorOptionWrite( const char* strName, const char* strParam )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 )
		return ( static_cast<CqParameterTyped<CqColor>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a float system option parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return Float pointer 0 if not found.
 */

const TqFloat* CqOptions::GetFloatOption( const char* strName, const char* strParam ) const
{
	const CqParameter * pParam = pParameter( strName, strParam );
	if ( pParam != 0 )
		return ( static_cast<const CqParameterTyped<TqFloat>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get an integer system option parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return Integer pointer 0 if not found.
 */

const TqInt* CqOptions::GetIntegerOption( const char* strName, const char* strParam ) const
{
	const CqParameter * pParam = pParameter( strName, strParam );
	if ( pParam != 0 )
		return ( static_cast<const CqParameterTyped<TqInt>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a string system option parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqString pointer 0 if not found.
 */

const CqString* CqOptions::GetStringOption( const char* strName, const char* strParam ) const
{
	const CqParameter * pParam = pParameter( strName, strParam );
	if ( pParam != 0 )
		return ( static_cast<const CqParameterTyped<CqString>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a point system option parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqVector3D pointer 0 if not found.
 */

const CqVector3D* CqOptions::GetPointOption( const char* strName, const char* strParam ) const
{
	const CqParameter * pParam = pParameter( strName, strParam );
	if ( pParam != 0 )
		return ( static_cast<const CqParameterTyped<CqVector3D>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Get a color system option parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return Color pointer 0 if not found.
 */

const CqColor* CqOptions::GetColorOption( const char* strName, const char* strParam ) const
{
	const CqParameter * pParam = pParameter( strName, strParam );
	if ( pParam != 0 )
		return ( static_cast<const CqParameterTyped<CqColor>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}


//---------------------------------------------------------------------
/** Force the imager shader to be executed
 * \param gx, gy the size of the bucket grid
 * \param x, y its origin 
 */
void CqOptions::InitialiseColorImager(TqInt gx, TqInt gy, 
									  TqFloat x, TqFloat y,
									  CqColor *color, CqColor *opacity,
									  TqFloat *depth, TqFloat *coverage)
{
	// Each time with finished up a bucket; we will execute the imager shader
	// on the gridsize about the same size as the bucket
	if (m_pshadImager != NULL) 
	{
		m_pshadImager->Initialise(gx, gy, x, y, color, opacity, depth, coverage);
	}
}

//---------------------------------------------------------------------
/** Get a color from the imager shader.
 * \param x The X in raster coordinate system.
 * \param y The Y in raster coordiante system.
 * \return Color  Black if not found.
 * Right now it is returning the current background colour if found
 */
CqColor CqOptions::GetColorImager(TqFloat x, TqFloat y)
{
	CqColor result(0,0,0);

	if (m_pshadImager != NULL)
	{
		// get the color from the current imager than
		result = m_pshadImager->Color(x,y);	
	}

	return result;
}

//---------------------------------------------------------------------
/** Get a color from the imager shader.
 * \param x The X in raster coordinate system.
 * \param y The Y in raster coordiante system.
 * \return Color  Black if not found.
 * Right now it is returning the current background colour if found
 */
TqFloat CqOptions::GetAlphaImager(TqFloat x, TqFloat y)
{
	TqFloat result = 1.0;

	if (m_pshadImager != NULL)
	{
		// get the color from the current imager than
		result = m_pshadImager->Alpha(x,y);	
	}

	return result;
}


//---------------------------------------------------------------------
/** Get an opacity from the imager shader.
 * \param x The X in raster coordinate system.
 * \param y The Y in raster coordiante system.
 * \return Color  White right now
 * Right now it is returning the current background colour if found
 */

CqColor CqOptions::GetOpacityImager(TqFloat x, TqFloat y)
{
	CqColor result = gColWhite;

	if (m_pshadImager != NULL)
	{
		// get the opacity from the current imager than
		result = m_pshadImager->Opacity(x,y);	
	}


	return result;
}

//---------------------------------------------------------------------
/** Load an Imager shader find it
 * 
 * \param strName the name of the shader like background.sl, 
 * Right now it is doing nothing.
 */
void CqOptions::LoadImager(const CqString& strName)
{
	DeleteImager();
	TqFloat dummy = 0.0;
	
	IqShader* pShader=static_cast<IqShader*>(QGetRenderContext()->CreateShader(strName.c_str(), Type_Imager));

	if(pShader==0)	return;

	m_pshadImager=new CqImagersource(pShader,RI_TRUE);
	m_pshadImager->pShader()->PrepareDefArgs();
	
}

void   CqOptions::DeleteImager()
{

	if (m_pshadImager != NULL)
	{
		delete m_pshadImager; 
		m_pshadImager = NULL;
	}
}
void CqOptions::SetValueImager(char *token, char *value)
{
	if (m_pshadImager != NULL) {
		
		m_pshadImager->pShader()->SetValue(token,value);
	}

}



//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )
