//------------------------------------------------------------------------------
/**
 *	@file	itransform.h
 *	@author	Authors name
 *	@brief	Brief description of the file contents
 *
 *	Last change by:		$Author$
 *	Last change date:	$Date$
 */
//------------------------------------------------------------------------------
#ifndef	___itransform_Loaded___
#define	___itransform_Loaded___

#include	<aqsis/aqsis.h>
#include	<boost/shared_ptr.hpp>

namespace Aqsis {

struct IqTransform
{
	virtual	~IqTransform()
	{}

	/** Get a writable copy of this, if the reference count is greater than 1
	 * create a new copy and retirn that.
	 */
	// virtual void	SetCurrentTransform( TqFloat time, const CqMatrix& matTrans ) = 0;;
	// virtual	void	ConcatCurrentTransform( TqFloat time, const CqMatrix& matTrans ) = 0;
	virtual	const CqMatrix&	matObjectToWorld( TqFloat time ) const = 0;
	virtual	TqFloat	Time( TqInt index ) const = 0;
	virtual	TqInt	cTimes() const = 0;
	/** Set the handedness of the current transform
	 */
	virtual	bool GetHandedness(TqFloat time) const=0;
	/** Flip the handedness of the current coordinate system.
	 */
	// virtual void FlipHandedness(TqFloat time)=0;
};


} // namespace Aqsis


#endif	//	___itransform_Loaded___
