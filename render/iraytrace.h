//------------------------------------------------------------------------------
/**
 *	@file	iraytrace.h
 *	@author	Paul Gregory
 *	@brief	Declare the interface class for common raytracer access.
 *
 *	Last change by:		$Author: pgregory $
 *	Last change date:	$Date: 2004/02/09 20:04:49 $
 */ 
//------------------------------------------------------------------------------


#ifndef	___iraytrace_Loaded___
#define	___iraytrace_Loaded___

#include	"aqsis.h"

START_NAMESPACE( Aqsis )

struct IqSurface;

struct IqRaytrace
{
    virtual ~IqRaytrace()
    {}


	/** Initialise the raytracing subsystem.
	 */
	virtual	void	Initialise()=0;

	/** Add a primitive to the raytracing space subdivision structure.
	 */
	virtual	void	AddPrimitive(IqSurface* pSurface)=0;

	/** Prepare the structure for raytrace queries.
	 */
	virtual void	Finalise()=0;
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	//	___iraytrace_Loaded___
