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
	TqInt proj = GetIntegerOption( "System", "Projection" ) [ 0 ];
	switch ( proj )
	{
			case	ProjectionOrthographic:
			{
				// Define a matrix to convert from right top left handed coordinate systems.
				CqMatrix Trl( 1, 1, -1 );

				TqFloat l = GetFloatOption( "System", "ScreenWindow" ) [ 0 ];
				TqFloat r = GetFloatOption( "System", "ScreenWindow" ) [ 1 ];
				TqFloat t = GetFloatOption( "System", "ScreenWindow" ) [ 2 ];
				TqFloat b = GetFloatOption( "System", "ScreenWindow" ) [ 3 ];
				TqFloat n = GetFloatOption( "System", "Clipping" ) [ 0 ];
				TqFloat f = GetFloatOption( "System", "Clipping" ) [ 1 ];

				matCameraToScreen.Identity();
				matCameraToScreen.SetfIdentity( TqFalse );
				matCameraToScreen.SetElement( 0, 0, 2.0f / ( r - l ) );
				matCameraToScreen.SetElement( 3, 0, -( r + l ) / ( r - l ) );
				matCameraToScreen.SetElement( 1, 1, 2.0f / ( t - b ) );
				matCameraToScreen.SetElement( 3, 1, -( t + b ) / ( t - b ) );
				matCameraToScreen.SetElement( 2, 2, -2.0f / ( f - n ) );
				matCameraToScreen.SetElement( 3, 2, -( f + n ) / ( f - n ) );
				matCameraToScreen.SetElement( 2, 3, 0 );
				matCameraToScreen.SetElement( 3, 3, 1 );

				// Combine with the right to left matrix.
				matCameraToScreen *= Trl;

				// Set up the screen to frame matrix
				TqFloat	FrameX = ( GetFloatOption( "System", "FrameAspectRatio" ) [ 0 ] >= 1.0 ) ? GetIntegerOption( "System", "Resolution" ) [ 0 ] :
				                 ( GetIntegerOption( "System", "Resolution" ) [ 1 ] * GetFloatOption( "System", "FrameAspectRatio" ) [ 0 ] ) / GetFloatOption( "System", "PixelAspectRatio" ) [ 0 ];
				TqFloat	FrameY = ( GetIntegerOption( "System", "FrameAspectRatio" ) [ 0 ] < 1.0 ) ? GetIntegerOption( "System", "Resolution" ) [ 1 ] :
				                 ( GetIntegerOption( "System", "Resolution" ) [ 0 ] * GetFloatOption( "System", "PixelAspectRatio" ) [ 0 ] ) / GetFloatOption( "System", "FrameAspectRatio" ) [ 0 ];
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

				// Setup the view frustum clipping volume
				// Left clipping plane
				CqPlane pl(1,0,0,fabs(l));
				QGetRenderContext()->clippingVolume().addPlane(pl);

				// Right clipping plane
				CqPlane pr(-1,0,0,fabs(r));
				QGetRenderContext()->clippingVolume().addPlane(pr);

				// Top clipping plane
				CqPlane pt(0,-1,0,fabs(t));
				QGetRenderContext()->clippingVolume().addPlane(pt);

				// Bottom clipping plane
				CqPlane pb(0,1,0,fabs(b));
				QGetRenderContext()->clippingVolume().addPlane(pb);

				// Near clipping plane
				CqPlane pn(0,0,1,fabs(n));
				QGetRenderContext()->clippingVolume().addPlane(pn);

				// Far clipping plane
				CqPlane pf(0,0,-1,fabs(f));
				QGetRenderContext()->clippingVolume().addPlane(pf);

				break;
			}

			case	ProjectionPerspective:
			{
				TqFloat fov = GetFloatOption( "System", "Clipping" ) [ 0 ] * ( tan( RAD( GetFloatOption( "System", "FOV" ) [ 0 ] / 2.0f ) ) );
				TqFloat l = GetFloatOption( "System", "ScreenWindow" ) [ 0 ] * fov;
				TqFloat r = GetFloatOption( "System", "ScreenWindow" ) [ 1 ] * fov;
				TqFloat t = GetFloatOption( "System", "ScreenWindow" ) [ 2 ] * fov;
				TqFloat b = GetFloatOption( "System", "ScreenWindow" ) [ 3 ] * fov;
				TqFloat n = GetFloatOption( "System", "Clipping" ) [ 0 ];
				TqFloat f = GetFloatOption( "System", "Clipping" ) [ 1 ];

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
				TqFloat	FrameX = ( GetFloatOption( "System", "FrameAspectRatio" ) [ 0 ] >= 1.0 ) ? GetIntegerOption( "System", "Resolution" ) [ 0 ] :
				                 ( GetIntegerOption( "System", "Resolution" ) [ 1 ] * GetFloatOption( "System", "FrameAspectRatio" ) [ 0 ] ) / GetFloatOption( "System", "PixelAspectRatio" ) [ 0 ];
				TqFloat	FrameY = ( GetFloatOption( "System", "FrameAspectRatio" ) [ 0 ] < 1.0 ) ? GetIntegerOption( "System", "Resolution" ) [ 1 ] :
				                 ( GetIntegerOption( "System", "Resolution" ) [ 0 ] * GetFloatOption( "System", "PixelAspectRatio" ) [ 0 ] ) / GetFloatOption( "System", "FrameAspectRatio" ) [ 0 ];

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

				// Setup the view frustum clipping volume
				// Left clipping plane
				CqPlane pl(matCameraToScreen[0][3] + matCameraToScreen[0][0],
				           matCameraToScreen[1][3] + matCameraToScreen[1][0],
				           matCameraToScreen[2][3] + matCameraToScreen[2][0],
				           matCameraToScreen[3][3] + matCameraToScreen[3][0]);
				QGetRenderContext()->clippingVolume().addPlane(pl);

				// Right clipping plane
				CqPlane pr(matCameraToScreen[0][3] - matCameraToScreen[0][0],
				           matCameraToScreen[1][3] - matCameraToScreen[1][0],
				           matCameraToScreen[2][3] - matCameraToScreen[2][0],
				           matCameraToScreen[3][3] - matCameraToScreen[3][0]);
				QGetRenderContext()->clippingVolume().addPlane(pr);

				// Top clipping plane
				CqPlane pt(matCameraToScreen[0][3] - matCameraToScreen[0][1],
				           matCameraToScreen[1][3] - matCameraToScreen[1][1],
				           matCameraToScreen[2][3] - matCameraToScreen[2][1],
				           matCameraToScreen[3][3] - matCameraToScreen[3][1]);
				QGetRenderContext()->clippingVolume().addPlane(pt);

				// Bottom clipping plane
				CqPlane pb(matCameraToScreen[0][3] + matCameraToScreen[0][1],
				           matCameraToScreen[1][3] + matCameraToScreen[1][1],
				           matCameraToScreen[2][3] + matCameraToScreen[2][1],
				           matCameraToScreen[3][3] + matCameraToScreen[3][1]);
				QGetRenderContext()->clippingVolume().addPlane(pb);

				// Near clipping plane
				CqPlane pn(matCameraToScreen[0][2],
				           matCameraToScreen[1][2],
				           matCameraToScreen[2][2],
				           matCameraToScreen[3][2]);
				QGetRenderContext()->clippingVolume().addPlane(pn);

				// Far clipping plane
				CqPlane pf(matCameraToScreen[0][3] - matCameraToScreen[0][2],
				           matCameraToScreen[1][3] - matCameraToScreen[1][2],
				           matCameraToScreen[2][3] - matCameraToScreen[2][2],
				           matCameraToScreen[3][3] - matCameraToScreen[3][2]);
				QGetRenderContext()->clippingVolume().addPlane(pf);

				break;
			}
	}
	CqMatrix matWorldToCamera( QGetRenderContext() ->matSpaceToSpace( "world", "camera", CqMatrix(), CqMatrix(), QGetRenderContext()->Time() ) );
	QGetRenderContext() ->SetmatScreen( matCameraToScreen * matWorldToCamera );
	QGetRenderContext() ->SetmatNDC( matScreenToNDC * matCameraToScreen * matWorldToCamera );
	QGetRenderContext() ->SetmatRaster( matNDCToRaster * matScreenToNDC * matCameraToScreen * matWorldToCamera );

	CqMatrix matWorldToScreen = matCameraToScreen * matWorldToCamera;

	CqMatrix dofm = QGetRenderContext() ->matVSpaceToSpace( "camera", "raster", CqMatrix(), CqMatrix(), QGetRenderContext()->Time() );

	CqVector3D	dofe( 1, 1, -1 );
	CqVector3D	dofc( 0, 0, -1 );

	dofe = dofm *  dofe;
	dofc = dofm *  dofc;

	QGetRenderContext() ->SetDepthOfFieldScale( fabs(dofe.x()-dofc.x()), fabs(dofe.y()-dofc.y()) );
}


//---------------------------------------------------------------------
/** Default constructor.
 */



CqOptions::CqOptions() :
		m_funcFilter( RiGaussianFilter ),
		m_pshadImager( NULL )
{
	InitialiseDefaultOptions();
}

//---------------------------------------------------------------------
/** Copy constructor.
 */

CqOptions::CqOptions( const CqOptions& From ) :
		m_funcFilter( RiGaussianFilter ),
		m_pshadImager( NULL )
{
	*this = From;
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqOptions::~CqOptions()
{
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqOptions& CqOptions::operator=( const CqOptions& From )
{
	m_funcFilter = From.m_funcFilter;
	m_pshadImager = From.m_pshadImager;

	//DeleteImager();

	// Copy the system options.
	m_aOptions.resize( From.m_aOptions.size() );
	TqInt i = From.m_aOptions.size();
	while ( i-- > 0 )
	{
		m_aOptions[ i ] = From.m_aOptions[ i ];
	}

	return ( *this );
}



#define	ADD_SYSTEM_PARAM(name, type, sltype, id, def) \
	CqParameterTypedUniform<type,id,sltype>* p##name = new CqParameterTypedUniform<type,id,sltype>(#name); \
	p##name->pValue()[0] = ( def ); \
	pdefopts->AddParameter(p##name);

#define	ADD_SYSTEM_PARAM2(name, type, sltype, id, def0, def1) \
	CqParameterTypedUniformArray<type,id,sltype>* p##name = new CqParameterTypedUniformArray<type,id,sltype>(#name,2); \
	p##name->pValue()[0] = ( def0 ); \
	p##name->pValue()[1] = ( def1 ); \
	pdefopts->AddParameter(p##name);

#define	ADD_SYSTEM_PARAM3(name, type, sltype, id, def0, def1, def2) \
	CqParameterTypedUniformArray<type,id,sltype>* p##name = new CqParameterTypedUniformArray<type,id,sltype>(#name,3); \
	p##name->pValue()[0] = ( def0 ); \
	p##name->pValue()[1] = ( def1 ); \
	p##name->pValue()[2] = ( def2 ); \
	pdefopts->AddParameter(p##name);

#define	ADD_SYSTEM_PARAM4(name, type, sltype, id, def0, def1, def2, def3) \
	CqParameterTypedUniformArray<type,id,sltype>* p##name = new CqParameterTypedUniformArray<type,id,sltype>(#name,4); \
	p##name->pValue()[0] = ( def0 ); \
	p##name->pValue()[1] = ( def1 ); \
	p##name->pValue()[2] = ( def2 ); \
	p##name->pValue()[3] = ( def3 ); \
	pdefopts->AddParameter(p##name);


void CqOptions::InitialiseDefaultOptions()
{
	boost::shared_ptr<CqNamedParameterList> pdefopts( new CqNamedParameterList( "System" ) );

	ADD_SYSTEM_PARAM( PixelVariance, TqFloat, TqFloat, type_float, 1.0f );
	ADD_SYSTEM_PARAM2( PixelSamples, TqInt, TqFloat, type_integer, 2, 2 );
	ADD_SYSTEM_PARAM2( FilterWidth, TqFloat, TqFloat, type_float, 2.0f, 2.0f );
	ADD_SYSTEM_PARAM2( Exposure, TqFloat, TqFloat, type_float, 1.0f, 1.0f );
	ADD_SYSTEM_PARAM( Imager, CqString, CqString, type_string, "null" );
	ADD_SYSTEM_PARAM( DisplayType, CqString, CqString, type_string, "file" );
	ADD_SYSTEM_PARAM( DisplayName, CqString, CqString, type_string, "aqsis.tif" );
	ADD_SYSTEM_PARAM( DisplayMode, TqInt, TqFloat, type_integer, ModeRGB );

	ADD_SYSTEM_PARAM( Hider, CqString, CqString, type_string, "hidden" );
	ADD_SYSTEM_PARAM( ColorSamples, TqInt, TqFloat, type_integer, 3 );
	ADD_SYSTEM_PARAM( RelativeDetail, TqFloat, TqFloat, type_float, 1.0f );

	ADD_SYSTEM_PARAM2( Resolution, TqInt, TqFloat, type_integer, 640, 480 );
	ADD_SYSTEM_PARAM( PixelAspectRatio, TqFloat, TqFloat, type_float, 1.0f );
	ADD_SYSTEM_PARAM4( CropWindow, TqFloat, TqFloat, type_float, 0.0f, 1.0f, 0.0f, 1.0f );
	ADD_SYSTEM_PARAM( FrameAspectRatio, TqFloat, TqFloat, type_float, 4.0f / 3.0f );
	ADD_SYSTEM_PARAM4( ScreenWindow, TqFloat, TqFloat, type_float, -( 4.0f / 3.0f ), ( 4.0f / 3.0f ), 1.0f, -1.0f );
	ADD_SYSTEM_PARAM( Projection, TqInt, TqFloat, type_integer, ProjectionOrthographic );
	ADD_SYSTEM_PARAM2( Clipping, TqFloat, TqFloat, type_float, FLT_EPSILON, FLT_MAX );
	ADD_SYSTEM_PARAM3( DepthOfField, TqFloat, TqFloat, type_float, FLT_MAX, FLT_MAX, FLT_MAX );
	ADD_SYSTEM_PARAM2( Shutter, TqFloat, TqFloat, type_float, 0.0f, 1.0f );
	ADD_SYSTEM_PARAM( FOV, TqFloat, TqFloat, type_float, 90.0f );
	ADD_SYSTEM_PARAM( SqrtGridSize, TqFloat, TqFloat, type_float, 16.0f );					// Grid size square root.

	AddOption( pdefopts );

	pdefopts = boost::shared_ptr<CqNamedParameterList>( new CqNamedParameterList( "Quantize" ) );

	ADD_SYSTEM_PARAM4( Color, TqFloat, TqFloat, type_float, 255.0f, 0.0f, 255.0f, 0.5f );
	ADD_SYSTEM_PARAM4( Depth, TqFloat, TqFloat, type_float,   0.0f, 0.0f,   0.0f, 0.0f );

	AddOption( pdefopts );
}


//---------------------------------------------------------------------
/** Get a system option parameter, takes name and parameter name.
 * \param strName The name of the option.
 * \param strParam The name of the paramter on the option.
 * \return CqParameter pointer or 0 if not found.
 */

const CqParameter* CqOptions::pParameter( const char* strName, const char* strParam ) const
{
	const CqNamedParameterList* pList = pOption( strName ).get();
	if ( pList )
	{
		const CqParameter * pParam;
		if ( ( pParam = pList->pParameter( strParam ) ) != 0 )
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
	CqNamedParameterList* pList = pOptionWrite( strName ).get();
	if ( pList )
	{
		CqParameter * pParam;
		if ( ( pParam = pList->pParameter( strParam ) ) != 0 )
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

TqFloat* CqOptions::GetFloatOptionWrite( const char* strName, const char* strParam, TqInt arraySize )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 )
		return ( static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam ) ->pValue() );
	else
	{
		// As we are getting a writeable copy, we should create if it doesn't exist.
		CqNamedParameterList* pList = pOptionWrite( strName ).get();
		CqParameterTypedUniform<TqFloat, type_float, TqFloat>* pOpt = new CqParameterTypedUniform<TqFloat, type_float, TqFloat>(strParam, arraySize);
		pList->AddParameter(pOpt);
		return ( pOpt->pValue() );
	}
}


//---------------------------------------------------------------------
/** Get an integer system option parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return Integer pointer 0 if not found.
 */

TqInt* CqOptions::GetIntegerOptionWrite( const char* strName, const char* strParam, TqInt arraySize )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 )
		return ( static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam ) ->pValue() );
	else
	{
		// As we are getting a writeable copy, we should create if it doesn't exist.
		CqNamedParameterList* pList = pOptionWrite( strName ).get();
		CqParameterTypedUniform<TqInt, type_integer, TqInt>* pOpt = new CqParameterTypedUniform<TqInt, type_integer, TqInt>(strParam, arraySize);
		pList->AddParameter(pOpt);
		return ( pOpt->pValue() );
	}
}


//---------------------------------------------------------------------
/** Get a string system option parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqString pointer 0 if not found.
 */

CqString* CqOptions::GetStringOptionWrite( const char* strName, const char* strParam, TqInt arraySize )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 )
		return ( static_cast<CqParameterTyped<CqString, CqString>*>( pParam ) ->pValue() );
	else
	{
		// As we are getting a writeable copy, we should create if it doesn't exist.
		CqNamedParameterList* pList = pOptionWrite( strName ).get();
		CqParameterTypedUniform<CqString, type_string, CqString>* pOpt = new CqParameterTypedUniform<CqString, type_string, CqString>(strParam, arraySize);
		pList->AddParameter(pOpt);
		return ( pOpt->pValue() );
	}
}


//---------------------------------------------------------------------
/** Get a point system option parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqVector3D pointer 0 if not found.
 */

CqVector3D* CqOptions::GetPointOptionWrite( const char* strName, const char* strParam, TqInt arraySize )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 )
		return ( static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam ) ->pValue() );
	else
	{
		// As we are getting a writeable copy, we should create if it doesn't exist.
		CqNamedParameterList* pList = pOptionWrite( strName ).get();
		CqParameterTypedUniform<CqVector3D, type_point, CqVector3D>* pOpt = new CqParameterTypedUniform<CqVector3D, type_point, CqVector3D>(strParam, arraySize);
		pList->AddParameter(pOpt);
		return ( pOpt->pValue() );
	}
}


//---------------------------------------------------------------------
/** Get a color system option parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqColor pointer 0 if not found.
 */

CqColor* CqOptions::GetColorOptionWrite( const char* strName, const char* strParam, TqInt arraySize )
{
	CqParameter * pParam = pParameterWrite( strName, strParam );
	if ( pParam != 0 )
		return ( static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam ) ->pValue() );
	else
	{
		// As we are getting a writeable copy, we should create if it doesn't exist.
		CqNamedParameterList* pList = pOptionWrite( strName ).get();
		CqParameterTypedUniform<CqColor, type_color, CqColor>* pOpt = new CqParameterTypedUniform<CqColor, type_color, CqColor>(strParam, arraySize);
		pList->AddParameter(pOpt);
		return ( pOpt->pValue() );
	}
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
		return ( static_cast<const CqParameterTyped<TqFloat, TqFloat>*>( pParam ) ->pValue() );
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
		return ( static_cast<const CqParameterTyped<TqInt, TqFloat>*>( pParam ) ->pValue() );
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
		return ( static_cast<const CqParameterTyped<CqString, CqString>*>( pParam ) ->pValue() );
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
		return ( static_cast<const CqParameterTyped<CqVector3D, CqVector3D>*>( pParam ) ->pValue() );
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
		return ( static_cast<const CqParameterTyped<CqColor, CqColor>*>( pParam ) ->pValue() );
	else
		return ( 0 );
}

EqVariableType CqOptions::getParameterType(const char* strName, const char* strParam) const
{
	const CqParameter* pParam = pParameter(strName, strParam);
	if(pParam == NULL)
		return type_invalid;
	else
		return(pParam->Type());
}

EqVariableClass CqOptions::getParameterClass(const char* strName, const char* strParam) const
{
	const CqParameter* pParam = pParameter(strName, strParam);
	if(pParam == NULL)
		return class_invalid;
	else
		return(pParam->Class());
}

TqUint CqOptions::getParameterSize(const char* strName, const char* strParam) const
{
	const CqParameter* pParam = pParameter(strName, strParam);
	if(pParam == NULL)
		return 0;
	else
		return(pParam->Size());
}

TqInt CqOptions::getParameterArraySize(const char* strName, const char* strParam) const
{
	const CqParameter* pParam = pParameter(strName, strParam);
	if(pParam == NULL)
		return 0;
	else
		return(pParam->Count());
}

void CqOptions::SetpshadImager( const boost::shared_ptr<IqShader>& pshadImager )
{
	delete m_pshadImager;

	m_pshadImager = new CqImagersource(pshadImager, TqTrue);
        m_pshadImager->pShader()->PrepareDefArgs();
}


boost::shared_ptr<IqShader>	CqOptions::pshadImager() const
{
	if(m_pshadImager)
		return ( m_pshadImager->pShader() );
	else
		return boost::shared_ptr<IqShader>();
}

//---------------------------------------------------------------------
/** Force the imager shader to be executed
 * \param gx The width of the bucket in pixels.
 * \param gy the height of the bucket in pixels.
 * \param x The origin of the bucket within the overall image.
 * \param y The origin of the bucket within the overall image.
 * \param color Initial value Ci.
 * \param opacity Initial value Oi.
 * \param depth Initial value depth (not required).
 * \param coverage Initial value "alpha"
 */
void CqOptions::InitialiseColorImager( IqBucket* pBucket )
{
	// Each time with finished up a bucket; we will execute the imager shader
	// on the gridsize about the same size as the bucket
	if ( m_pshadImager != NULL )
	{
		m_pshadImager->Initialise( pBucket );
	}
}

//---------------------------------------------------------------------
/** Get a color from the imager shader.
 * \param x The X in raster coordinate system.
 * \param y The Y in raster coordiante system.
 * \return Color  Black if not found.
 * Right now it is returning the current background colour if found
 */
CqColor CqOptions::GetColorImager( TqFloat x, TqFloat y )
{
	CqColor result( 0, 0, 0 );

	if ( m_pshadImager != NULL )
	{
		// get the color from the current imager than
		result = m_pshadImager->Color( x, y );
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
TqFloat CqOptions::GetAlphaImager( TqFloat x, TqFloat y )
{
	TqFloat result = 1.0;

	if ( m_pshadImager != NULL )
	{
		// get the color from the current imager than
		result = m_pshadImager->Alpha( x, y );
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

CqColor CqOptions::GetOpacityImager( TqFloat x, TqFloat y )
{
	CqColor result = gColWhite;

	if ( m_pshadImager != NULL )
	{
		// get the opacity from the current imager than
		result = m_pshadImager->Opacity( x, y );
	}


	return result;
}


//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )
