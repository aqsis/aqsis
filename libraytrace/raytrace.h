//------------------------------------------------------------------------------
/**
 *	@file	raytrace.h
 *	@author	Paul Gregory
 *	@brief	Implement the basic raytracer subsystem interface implementation.
 *
 *	Last change by:		$Author: pgregory $
 *	Last change date:	$Date: 2004/02/09 20:04:49 $
 */ 
//------------------------------------------------------------------------------


#ifndef	___raytrace_Loaded___
#define	___raytrace_Loaded___

#include	"aqsis.h"
#include	"iraytrace.h"

START_NAMESPACE( Aqsis )


struct CqRaytrace : public IqRaytrace
{
			 CqRaytrace()
	{}
    virtual ~CqRaytrace()
    {}


	// Interface functions overridden from IqRaytrace
	virtual	void	Initialise();
	virtual	void	AddPrimitive(IqSurface* pSurface);
	virtual void	Finalise();
};




//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	//	___raytrace_Loaded___
