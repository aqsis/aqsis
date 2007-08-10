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
#ifndef	___irenderer_Loaded___
#define	___irenderer_Loaded___

#include	"matrix.h"
//#include	"symbols.h"
#include 	"itransform.h"

START_NAMESPACE( Aqsis )

struct IqTextureMap;
class CqObjectInstance;

struct IqRenderer
{
	virtual	~IqRenderer()
	{}

	// Handle various coordinate system transformation requirements.

	virtual	CqMatrix	matSpaceToSpace	( const char* strFrom, const char* strTo, const IqTransform* transShaderToWorld, const IqTransform* transObjectToWorld, TqFloat time ) = 0;
	virtual	CqMatrix	matVSpaceToSpace	( const char* strFrom, const char* strTo, const IqTransform* transShaderToWorld, const IqTransform* transObjectToWorld, TqFloat time ) = 0;
	virtual	CqMatrix	matNSpaceToSpace	( const char* strFrom, const char* strTo, const IqTransform* transShaderToWorld, const IqTransform* transObjectToWorld, TqFloat time ) = 0;

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

	/*
	 *  Access to the texture map handling component
	 */

	/** Get a pointer to a loaded texturemap ready for processing.
	 */
	virtual	IqTextureMap* GetTextureMap( const CqString& strFileName ) = 0;
	virtual	IqTextureMap* GetEnvironmentMap( const CqString& strName ) = 0;
	virtual	IqTextureMap* GetShadowMap( const CqString& strName ) = 0;
	virtual	IqTextureMap* GetLatLongMap( const CqString& strName ) = 0;


	virtual	bool	GetBasisMatrix( CqMatrix& matBasis, const CqString& name ) = 0;

	virtual TqInt	RegisterOutputData( const char* name ) = 0;
	virtual TqInt	OutputDataIndex( const char* name ) = 0;
	virtual TqInt	OutputDataSamples( const char* name ) = 0;
	virtual TqInt	OutputDataType( const char* name ) = 0;

	virtual	void	SetCurrentFrame( TqInt FrameNo ) = 0;
	virtual	TqInt	CurrentFrame() const = 0;

	virtual	CqObjectInstance*	pCurrentObject() = 0;

	virtual	TqFloat	Time() const = 0;

	virtual	TqInt	bucketCount() = 0;

	virtual	bool	IsWorldBegin() const = 0;
};

IqRenderer* QGetRenderContextI();

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	//	___irenderer_Loaded___
