//------------------------------------------------------------------------------
/**
 *	@file	isurface.h
 *	@author	Authors name
 *	@brief	Brief description of the file contents
 *
 *	Last change by:		$Author: pgregory $
 *	Last change date:	$Date: 2002/08/19 15:37:47 $
 */
//------------------------------------------------------------------------------
#ifndef	___isurface_Loaded___
#define	___isurface_Loaded___

#include	"aqsis.h"

START_NAMESPACE( Aqsis )


struct IqAttributes;
struct IqTransform;


//----------------------------------------------------------------------
/** \struct IqSurface
 * Abstract base surface class, which provides interfaces to geometry.  
 */

struct IqSurface
{
	virtual	~IqSurface() {}
	/** Transform this GPrim using the specified matrices.
	 * \param matTx Reference to the transformation matrix.
	 * \param matITTx Reference to the inverse transpose of the transformation matrix, used to transform normals.
	 * \param matRTx Reference to the rotation only transformation matrix, used to transform vectors.
	 */
	virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx ) = 0;
	/** Get the number of uniform parameters required for this GPrim.
	 */
	virtual	TqUint	cUniform() const = 0;
	/** Get the number of varying parameters required for this GPrim.
	 */
	virtual	TqUint	cVarying() const = 0;
	/** Get the number of vertex parameters required for this GPrim.
	 */
	virtual	TqUint	cVertex() const = 0;
	/** Get the number of facearying parameters required for this GPrim.
	 */
	virtual	TqUint	cFaceVarying() const = 0;

	virtual CqString	strName() const = 0;
	virtual	TqInt	Uses() const = 0;

	/** Get a pointer to the attributes state associated with this GPrim.
	 * \return A pointer to a CqAttributes class.
	 */
	virtual const	IqAttributes* pAttributes() const = 0;
	/** Get a pointer to the transformation state associated with this GPrim.
	 * \return A pointer to a CqTransform class.
	 */
	virtual const	IqTransform* pTransform() const = 0;
	/** Interpolate the specified value using the natural interpolation method for the surface.
	 *  Fills in the given shader data with the resulting data.
	 */
	virtual void	NaturalInterpolate(CqParameter* pParameter, TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pData) = 0;
};


END_NAMESPACE( Aqsis )


#endif	//	___isurface_Loaded___
