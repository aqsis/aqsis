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
/** \enum EqProjection
 * Possible projection modes for the camera.
 */
enum EqProjection
{
    ProjectionOrthographic, 		///< Orthographic projection.
    ProjectionPerspective		///< Perspective projection.
};


#define	DEFINE_SYSTEM_OPTION(index, type, id, def)	\
	type Get_##index() const \
	{ \
		static type __##index = ( def ); \
		if( NULL == m_pParameters[ option_##index ] )	return( __##index ); \
		return(*static_cast<CqParameterTyped<type>*>(m_pParameters[ option_##index ])->pValue()); \
	} \
	void Set_##index(const type##& value) \
	{ \
		if( NULL == m_pParameters[ option_##index ] ) \
			m_pParameters[ option_##index ] = new CqParameterTypedUniform<type, id>(#index); \
		*static_cast<CqParameterTyped<type>*>(m_pParameters[ option_##index ])->pValue() = value; \
	}


//----------------------------------------------------------------------
/** \class CqOptions
 * Storage for the graphics state options.
 */

class CqOptions 
{
	public:
		CqOptions() :
			m_pErrorHandler( &RiErrorPrint ),
			m_pProgressHandler( NULL ),
			m_pPreRenderFunction( NULL ),
			m_funcFilter( RiGaussianFilter ),
			m_pshadImager( NULL)
		{
			TqInt i;
			for( i = 0; i < option_last; i++)
				m_pParameters[i] = NULL;
		}
		CqOptions( const CqOptions& From );
		~CqOptions();

		CqOptions&	operator=( const CqOptions& From );

		enum EqDisplayParameters
		{
			option_pixelvariance,
			option_pixelsamplesx,
			option_pixelsamplesy,
			option_filterwidthx,
			option_filterwidthy,
			option_exposuregain,
			option_exposuregamma,
			option_imager,
			option_colorquantizeone,
			option_colorquantizemin,
			option_colorquantizemax,
			option_colorquantizeditheramplitude,
			option_depthquantizeone,
			option_depthquantizemin,
			option_depthquantizemax,
			option_depthquantizeditheramplitude,
			option_displaytype,
			option_displayname,
			option_displaymode,

			option_hider,
			option_colorsamples,
			option_relativedetail,

			option_resolutionx,
			option_resolutiony,
			option_pixelaspectratio,
			option_cropwindowminx,
			option_cropwindowminy,
			option_cropwindowmaxx,
			option_cropwindowmaxy,
			option_frameaspectratio,
			option_screenwindowleft,
			option_screenwindowright,
			option_screenwindowtop,
			option_screenwindowbottom,
			option_cameraprojection,
			option_clippingplanenear,
			option_clippingplanefar,
			option_fstop,
			option_focallength,
			option_focaldistance,
			option_shutteropen,
			option_shutterclose,
			option_fov,

			option_last
		};


		DEFINE_SYSTEM_OPTION(pixelvariance, TqFloat, type_float, 1.0f );
		DEFINE_SYSTEM_OPTION(pixelsamplesx, TqInt, type_integer, 2 );
		DEFINE_SYSTEM_OPTION(pixelsamplesy, TqInt, type_integer, 2 );
		DEFINE_SYSTEM_OPTION(filterwidthx, TqFloat, type_float, 2.0f );
		DEFINE_SYSTEM_OPTION(filterwidthy, TqFloat, type_float, 2.0f );
		DEFINE_SYSTEM_OPTION(exposuregain, TqFloat, type_float, 1.0f );
		DEFINE_SYSTEM_OPTION(exposuregamma, TqFloat, type_float, 1.0f );
//		DEFINE_SYSTEM_OPTION(imager, CqString, type_string, "null" );
		DEFINE_SYSTEM_OPTION(colorquantizeone, TqInt, type_integer, 255 );
		DEFINE_SYSTEM_OPTION(colorquantizemin, TqInt, type_integer, 0 );
		DEFINE_SYSTEM_OPTION(colorquantizemax, TqInt, type_integer, 2255 );
		DEFINE_SYSTEM_OPTION(colorquantizeditheramplitude, TqFloat, type_float, 0.5f );
		DEFINE_SYSTEM_OPTION(depthquantizeone, TqInt, type_integer, 0 );
		DEFINE_SYSTEM_OPTION(depthquantizemin, TqInt, type_integer, 0 );
		DEFINE_SYSTEM_OPTION(depthquantizemax, TqInt, type_integer, 255 );
		DEFINE_SYSTEM_OPTION(depthquantizeditheramplitude, TqFloat, type_float, 0.0f );
		DEFINE_SYSTEM_OPTION(displaytype, CqString, type_string, "file" );
		DEFINE_SYSTEM_OPTION(displayname, CqString, type_string, "aqsis.tif" );
		DEFINE_SYSTEM_OPTION(displaymode, TqInt, type_integer, ModeRGB );

		DEFINE_SYSTEM_OPTION(hider,CqString, type_string, "hidden" );
		DEFINE_SYSTEM_OPTION(colorsamples, TqInt, type_integer, 3 );
		DEFINE_SYSTEM_OPTION(relativedetail, TqFloat, type_float, 1.0f );

		DEFINE_SYSTEM_OPTION(resolutionx, TqInt, type_integer, 640 );
		DEFINE_SYSTEM_OPTION(resolutiony, TqInt, type_integer, 480 );
		DEFINE_SYSTEM_OPTION(pixelaspectratio, TqFloat, type_float, 1.0f );
		DEFINE_SYSTEM_OPTION(cropwindowminx, TqFloat, type_float, 0.0f );
		DEFINE_SYSTEM_OPTION(cropwindowminy, TqFloat, type_float, 0.0f );
		DEFINE_SYSTEM_OPTION(cropwindowmaxx, TqFloat, type_float, 1.0f );
		DEFINE_SYSTEM_OPTION(cropwindowmaxy, TqFloat, type_float, 1.0f );
		DEFINE_SYSTEM_OPTION(frameaspectratio, TqFloat, type_float, 4.0f / 3.0f );
		DEFINE_SYSTEM_OPTION(screenwindowleft, TqFloat, type_float, -( 4.0f / 3.0f ) );
		DEFINE_SYSTEM_OPTION(screenwindowright, TqFloat, type_float, 4.0f / 3.0f );
		DEFINE_SYSTEM_OPTION(screenwindowtop, TqFloat, type_float, 1.0f );
		DEFINE_SYSTEM_OPTION(screenwindowbottom, TqFloat, type_float, -1.0f );
		DEFINE_SYSTEM_OPTION(cameraprojection, TqInt, type_integer, ProjectionOrthographic );
		DEFINE_SYSTEM_OPTION(clippingplanenear, TqFloat, type_float, FLT_EPSILON );
		DEFINE_SYSTEM_OPTION(clippingplanefar, TqFloat, type_float, FLT_MAX );
		DEFINE_SYSTEM_OPTION(fstop, TqFloat, type_float, , FLT_MAX );
		DEFINE_SYSTEM_OPTION(focallength, TqFloat, type_float, , FLT_MAX );
		DEFINE_SYSTEM_OPTION(focaldistance, TqFloat, type_float, FLT_MAX );
		DEFINE_SYSTEM_OPTION(shutteropen, TqFloat, type_float, 0.0f );
		DEFINE_SYSTEM_OPTION(shutterclose, TqFloat, type_float, 1.0f );
		DEFINE_SYSTEM_OPTION(fov, TqFloat, type_float, 90.0f );


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
		/** Get the name of the Imager shader.
		 */
		CqString	Get_imager() const
		{
			if( NULL == m_pParameters[ option_imager ] )	return( "null" );
			
			return(*static_cast<CqParameterTyped<CqString>*>(m_pParameters[option_imager])->pValue());
		}
		/** Set the name of the Imager shader.
		 * \param strValue Character pointer to name of Imager shader.
		 */
		void	Set_imager( const CqString& value )
		{
			if( NULL == m_pParameters[ option_imager ] ) 
				m_pParameters[ option_imager ] = new CqParameterTypedUniform<CqString, type_string>("imager"); 
			*static_cast<CqParameterTyped<CqString>*>(m_pParameters[ option_imager ])->pValue() = value; 
			
			LoadImager( value );
		}
		void	LoadImager( const CqString& strValue );
		void	DeleteImager();
		void	SetValueImager( char *token, char *value );
		/** Get a pointer to the imager shader.
		 */
		const CqImagersource*	pshadImager() const
		{
			return ( m_pshadImager );
		}

		void	InitialiseColorImager( TqInt bx, TqInt by,
		                            TqFloat x, TqFloat y,
		                            CqColor *color, CqColor *opacity,
		                            TqFloat *depth, TqFloat *coverage );
		CqColor GetColorImager( TqFloat x, TqFloat y );
		CqColor GetOpacityImager( TqFloat x, TqFloat y );
		TqFloat GetAlphaImager( TqFloat x, TqFloat y );

	private:
		RtErrorFunc	m_pErrorHandler;		///< A pointer to the error hadling function.
		RtProgressFunc	m_pProgressHandler;		///< A pointer to the progress hadling function.
		RtFunc	m_pPreRenderFunction;	///< A pointer to the function called just prior to rendering.

		std::vector<CqSystemOption*>	m_aOptions;				///< Vector of user specified options.
		TqFloat	m_fClippingRange;

		CqParameter*	m_pParameters[option_last];
		RtFilterFunc m_funcFilter;						///< Pointer to the pixel filter function.
		CqImagersource* m_pshadImager;		///< Pointer to the imager shader.

		TqBool	m_bFrameAspectRatioCalled, 		///< Indicate RiFrameAspectRatio has been called. Calculation of the screen geometry is reliant on which of these have been called.
		m_bScreenWindowCalled, 			///< Indicate RiScreenWindow has been called. Calculation of the screen geometry is reliant on which of these have been called.
		m_bFormatCalled;				///< Indicate RiFormat has been called. Calculation of the screen geometry is reliant on which of these have been called.
}
;


END_NAMESPACE( Aqsis )

//-----------------------------------------------------------------------
//}  // End of #ifdef OPTIONS_H_INCLUDED
#endif
