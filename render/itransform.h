//------------------------------------------------------------------------------
/**
 *	@file	itransform.h
 *	@author	Authors name
 *	@brief	Brief description of the file contents
 *
 *	Last change by:		$Author: jpgrad $
 *	Last change date:	$Date: 2003/06/12 01:58:58 $
 */ 
//------------------------------------------------------------------------------
#ifndef	___itransform_Loaded___
#define	___itransform_Loaded___

#include	"aqsis.h"

START_NAMESPACE( Aqsis )

struct IqTransform
{
	virtual	~IqTransform()
	{}

	/** Get a writable copy of this, if the reference count is greater than 1
	 * create a new copy and retirn that.
	 */
	virtual void	SetCurrentTransform( TqFloat time, const CqMatrix& matTrans ) = 0;;
	virtual	void	ConcatCurrentTransform( TqFloat time, const CqMatrix& matTrans ) = 0;
	virtual	const CqMatrix&	matObjectToWorld( TqFloat time = 0.0f ) const = 0;
	virtual	TqFloat	Time( TqInt index ) const = 0;
	virtual	TqInt	cTimes() const = 0;

#ifndef _DEBUG
	virtual	void	Release() = 0;
	virtual	void	AddRef() = 0;
#else
	virtual void AddRef(const TqChar* file, TqInt line) = 0;
	virtual void Release(const TqChar* file, TqInt line) = 0;
#endif
};




END_NAMESPACE( Aqsis )


#endif	//	___itransform_Loaded___
