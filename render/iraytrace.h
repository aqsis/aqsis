//------------------------------------------------------------------------------
/**
 *	@file	iraytrace.h
 *	@author	Paul Gregory
 *	@brief	Declare the interface class for common raytracer access.
 *
 *	Last change by:		$Author: pseudonym $
 *	Last change date:	$Date: 2004/03/19 06:11:16 $
 */ 
//------------------------------------------------------------------------------


#ifndef	___iraytrace_Loaded___
#define	___iraytrace_Loaded___

#include	"aqsis.h"
#include	<boost/shared_ptr.hpp>

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
	virtual	void	AddPrimitive(const boost::shared_ptr<IqSurface>& pSurface)=0;

	/** Prepare the structure for raytrace queries.
	 */
	virtual void	Finalise()=0;
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	//	___iraytrace_Loaded___
