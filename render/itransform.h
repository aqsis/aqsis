//------------------------------------------------------------------------------
/**
 *	@file	itransform.h
 *	@author	Authors name
 *	@brief	Brief description of the file contents
 *
 *	Last change by:		$Author: pgregory $
 *	Last change date:	$Date: 2004/03/23 22:26:55 $
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
    virtual	const CqMatrix&	matObjectToWorld( TqFloat time ) const = 0;
    virtual	TqFloat	Time( TqInt index ) const = 0;
    virtual	TqInt	cTimes() const = 0;
	/** Set the handedness of the current transform 
	 */
	virtual	TqBool GetHandedness(TqFloat time) const=0;
	/** Flip the handedness of the current coordinate system.
	 */
	virtual void FlipHandedness(TqFloat time)=0;

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
