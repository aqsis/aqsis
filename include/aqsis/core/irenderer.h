//------------------------------------------------------------------------------
/**
 *	@file	irenderer.h
 *	@author	Authors name
 *	@brief	Declare the common interface structure for a Renderer core class.
 *
 *	Last change by:		$Author$
 *	Last change date:	$Date$
 */
//------------------------------------------------------------------------------
#ifndef	IRENDERER_H_INCLUDED
#define	IRENDERER_H_INCLUDED

#include <iosfwd>
#include <string>

#include <boost/function.hpp>

#include <aqsis/math/matrix.h>
#include <aqsis/core/itransform.h>
#include <aqsis/util/sstring.h>
#include <aqsis/math/vecfwd.h>

namespace Aqsis {

struct IqTextureMapOld;
struct IqTextureCache;

struct IqRenderer
{
	virtual	~IqRenderer()
	{}

	// Handle various coordinate system transformation requirements.

	virtual	bool	matSpaceToSpace	( const char* strFrom, const char* strTo, const IqTransform* transShaderToWorld, const IqTransform* transObjectToWorld, TqFloat time, CqMatrix& result ) = 0;
	virtual	bool	matVSpaceToSpace	( const char* strFrom, const char* strTo, const IqTransform* transShaderToWorld, const IqTransform* transObjectToWorld, TqFloat time, CqMatrix& result) = 0;
	virtual	bool	matNSpaceToSpace	( const char* strFrom, const char* strTo, const IqTransform* transShaderToWorld, const IqTransform* transObjectToWorld, TqFloat time, CqMatrix& result ) = 0;

	virtual	const	TqFloat*	GetFloatOption( const char* strName, const char* strParam ) const = 0;
	virtual	const	TqInt*	GetIntegerOption( const char* strName, const char* strParam ) const = 0;
	virtual	const	CqString* GetStringOption( const char* strName, const char* strParam ) const = 0;
	virtual	const	CqVector3D*	GetPointOption( const char* strName, const char* strParam ) const = 0;
	virtual	const	CqColor*	GetColorOption( const char* strName, const char* strParam ) const = 0;

	virtual	TqFloat*	GetFloatOptionWrite( const char* strName, const char* strParam ) = 0;
	virtual	TqInt*	GetIntegerOptionWrite( const char* strName, const char* strParam ) = 0;
	virtual	CqString* GetStringOptionWrite( const char* strName, const char* strParam ) = 0;
	virtual	CqVector3D*	GetPointOptionWrite( const char* strName, const char* strParam ) = 0;
	virtual	CqColor*	GetColorOptionWrite( const char* strName, const char* strParam ) = 0;


	/** Get the global statistics class.
	 * \return A reference to the CqStats class on this renderer.
	 */
	//		virtual	CqStats&	Stats() = 0;
	/** Print a message to stdout, along with any relevant message codes.
	 * \param str A SqMessage structure to print.
	 */
	virtual	void	PrintString( const char* str ) = 0;

	//--------------------------------------------------
	/// \name Texture map access
	//@{
	/** \brief Get a reference to the texture cache.
	 *
	 * \param fileName - File name to search for in the texture resource path.
	 * \return the texture sampler (always valid).
	 */
	virtual	IqTextureCache& textureCache() = 0;

	/// \deprecated
	virtual	IqTextureMapOld* GetEnvironmentMap( const CqString& fileName ) = 0;
	/// \deprecated
	virtual	IqTextureMapOld* GetOcclusionMap( const CqString& fileName ) = 0;
	/// \deprecated
	virtual	IqTextureMapOld* GetLatLongMap( const CqString& fileName ) = 0;
	//@}

	virtual	bool	GetBasisMatrix( CqMatrix& matBasis, const CqString& name ) = 0;

	virtual TqInt	RegisterOutputData( const char* name ) = 0;
	virtual TqInt	OutputDataIndex( const char* name ) = 0;
	virtual TqInt	OutputDataSamples( const char* name ) = 0;

	virtual	void	SetCurrentFrame( TqInt FrameNo ) = 0;
	virtual	TqInt	CurrentFrame() const = 0;

	virtual	TqFloat	Time() const = 0;

	virtual	bool	IsWorldBegin() const = 0;
};

AQSIS_CORE_SHARE IqRenderer* QGetRenderContextI();

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif //IRENDERER_H_INCLUDED
