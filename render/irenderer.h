//------------------------------------------------------------------------------
/**
 *	@file	irenderer.h
 *	@author	Authors name
 *	@brief	Declare the common interface structure for a Renderer core class.
 *
 *	Last change by:		$Author: pgregory $
 *	Last change date:	$Date: 2002/05/01 22:11:34 $
 */
//------------------------------------------------------------------------------
#ifndef	___irenderer_Loaded___
#define	___irenderer_Loaded___

#include	"matrix.h"

START_NAMESPACE( Aqsis )

struct IqRenderer
{
		virtual	~IqRenderer()	{}

		// Handle various coordinate system transformation requirements.
		virtual	CqMatrix	matSpaceToSpace	( const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld = CqMatrix(), const CqMatrix& matObjectToWorld = CqMatrix() ) = 0;
		virtual	CqMatrix	matVSpaceToSpace	( const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld = CqMatrix(), const CqMatrix& matObjectToWorld = CqMatrix() ) = 0;
		virtual	CqMatrix	matNSpaceToSpace	( const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld = CqMatrix(), const CqMatrix& matObjectToWorld = CqMatrix() ) = 0;

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
		 * \param msg A SqMessage structure to print.
		 */
		virtual	void	PrintString( const char* str ) = 0;
};

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	//	___irenderer_Loaded___
