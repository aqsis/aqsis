//------------------------------------------------------------------------------
/**
 *	@file	iattributes.h
 *	@author	Paul Gregory
 *	@brief	Declare the interface class for common attributes access.
 *
 *	Last change by:		$Author: pgregory $
 *	Last change date:	$Date: 2002/05/03 16:18:15 $
 */
//------------------------------------------------------------------------------


#ifndef	___iattributes_Loaded___
#define	___iattributes_Loaded___

#include	"aqsis.h"

#include	"sstring.h"
#include	"vector3d.h"
#include	"matrix.h"
#include	"color.h"

START_NAMESPACE( Aqsis )

struct IqAttributes
{
	virtual ~IqAttributes()	{}

	/** Get a named float attribute as read only
	 */
	virtual	const	TqFloat*	GetFloatAttribute( const char* strName, const char* strParam ) const = 0;
	/** Get a named integer attribute as read only
	 */
	virtual	const	TqInt*	GetIntegerAttribute( const char* strName, const char* strParam ) const = 0;
	/** Get a named string attribute as read only
	 */
	virtual	const	CqString* GetStringAttribute( const char* strName, const char* strParam ) const = 0;
	/** Get a named point attribute as read only
	 */
	virtual	const	CqVector3D*	GetPointAttribute( const char* strName, const char* strParam ) const = 0;
	/** Get a named color attribute as read only
	 */
	virtual	const	CqColor*	GetColorAttribute( const char* strName, const char* strParam ) const = 0;
	/** Get a named matrix attribute as read only
	 */
	virtual	const	CqMatrix*	GetMatrixAttribute( const char* strName, const char* strParam ) const = 0;

	/** Get a named float attribute as writable
	 */
	virtual	TqFloat*	GetFloatAttributeWrite( const char* strName, const char* strParam ) = 0;
	/** Get a named integer attribute as writable
	 */
	virtual	TqInt*	GetIntegerAttributeWrite( const char* strName, const char* strParam ) = 0;
	/** Get a named string attribute as writable
	 */
	virtual	CqString* GetStringAttributeWrite( const char* strName, const char* strParam ) = 0;
	/** Get a named point attribute as writable
	 */
	virtual	CqVector3D*	GetPointAttributeWrite( const char* strName, const char* strParam ) = 0;
	/** Get a named color attribute as writable
	 */
	virtual	CqColor*	GetColorAttributeWrite( const char* strName, const char* strParam ) = 0;
	/** Get a named matrix attribute as writable
	 */
	virtual	CqMatrix*	GetMatrixAttributeWrite( const char* strName, const char* strParam ) = 0;
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	//	___iattributes_Loaded___
