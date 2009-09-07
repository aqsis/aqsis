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
		\brief Declares the abstract interface for accessing options.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#ifndef IOPTIONS_H_INCLUDED
//{
#define IOPTIONS_H_INCLUDED

#include <aqsis/aqsis.h>

#include <boost/filesystem/path.hpp>

#include <aqsis/riutil/primvartype.h>
#include <aqsis/ri/ritypes.h>
#include <aqsis/math/vecfwd.h>

namespace Aqsis {

class CqImagersource;
class CqRegion;
class CqString;
struct IqShader;
struct IqChannelBuffer;

//----------------------------------------------------------------------
/** \enum EqDisplayMode
 */
enum EqDisplayMode
{
    DMode_None = 0x0000, ///< Invalid.
    DMode_RGB = 0x0001,  ///< Red Green and Blue channels.
    DMode_A = 0x0002,    ///< Alpha channel.
    DMode_Z = 0x0004     ///< Depth channel.
};


//----------------------------------------------------------------------
/** \enum EqProjection
 * Possible projection modes for the camera.
 */
enum EqProjection
{
	ProjectionNone,				///< Empty/NULL projection.
    ProjectionOrthographic,   		///< Orthographic projection.
    ProjectionPerspective		///< Perspective projection.
};

//----------------------------------------------------------------------
/** \enum EqCameraFlags
 */
enum EqCameraFlags
{
    CameraEmpty = 0x0000,   	///< Invalid.
    CameraScreenWindowSet = 0x0001,   		///< RiScreenWindow has been specified
    CameraFARSet = 0x0002,   		///< RiFrameAspectRatio has been specified
};

//----------------------------------------------------------------------
/** \class CqOptions
 * Storage for the graphics state options.
 */

struct IqOptions
{
	virtual ~IqOptions() {}

	virtual const	TqFloat*	GetFloatOption( const char* strName, const char* strParam ) const = 0;
	virtual const	TqInt*	GetIntegerOption( const char* strName, const char* strParam ) const = 0;
	virtual const	CqString* GetStringOption( const char* strName, const char* strParam ) const = 0;
	virtual const	CqVector3D*	GetPointOption( const char* strName, const char* strParam ) const = 0;
	virtual const	CqColor*	GetColorOption( const char* strName, const char* strParam ) const = 0;

	virtual TqFloat*	GetFloatOptionWrite( const char* strName, const char* strParam, TqInt arraySize = 1 ) = 0;
	virtual TqInt*	GetIntegerOptionWrite( const char* strName, const char* strParam, TqInt arraySize = 1 ) = 0;
	virtual CqString* GetStringOptionWrite( const char* strName, const char* strParam, TqInt arraySize = 1 ) = 0;
	virtual CqVector3D*	GetPointOptionWrite( const char* strName, const char* strParam, TqInt arraySize = 1 ) = 0;
	virtual CqColor*	GetColorOptionWrite( const char* strName, const char* strParam, TqInt arraySize = 1 ) = 0;

	virtual EqVariableType getParameterType(const char* strName, const char* strParam) const = 0;
	virtual EqVariableClass getParameterClass(const char* strName, const char* strParam) const = 0;
	virtual TqUint getParameterSize(const char* strName, const char* strParam) const = 0;
	virtual TqInt getParameterArraySize(const char* strName, const char* strParam) const = 0;
	
	virtual void	InitialiseCamera() = 0;
	/** Get a pointer to the pixel filter function.
	 */
	virtual RtFilterFunc funcFilter() const = 0;
	/** Set the pixel filter function to use.
	 * \param fValue A pointer to a function which follows the RtFilterFunc convention.
	 */
	virtual void	SetfuncFilter( const RtFilterFunc fValue ) = 0;
	virtual void SetpshadImager( const boost::shared_ptr<IqShader>& pshadImager ) = 0;
	virtual boost::shared_ptr<IqShader>	pshadImager() const = 0;

	virtual void	InitialiseColorImager( const CqRegion& DRegion, IqChannelBuffer* buffer ) = 0;
	virtual CqColor GetColorImager( TqFloat x, TqFloat y ) = 0;
	virtual CqColor GetOpacityImager( TqFloat x, TqFloat y ) = 0;
	virtual TqFloat GetAlphaImager( TqFloat x, TqFloat y ) = 0;

	virtual boost::filesystem::path findRiFile(const std::string& fileName,
			const char* riSearchPathName) const = 0;
	virtual boost::filesystem::path findRiFileNothrow(const std::string& fileName,
			const char* riSearchPathName) const = 0;
};


} // namespace Aqsis

//-----------------------------------------------------------------------
//}  // End of #ifdef IOPTIONS_H_INCLUDED
#endif
