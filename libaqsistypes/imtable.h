//------------------------------------------------------------------------------
/**
 *	@file	imtable.h
 *	@author	Matthäus G. Chajdas
 *	@brief	Declare the interface structure for the message table class.
 *
 *	Last change by:		$Author: mw_matti $
 *	Last change date:	$Date: 2003/03/22 11:00:55 $
 */ 
//------------------------------------------------------------------------------
#ifndef	___imtable_Loaded___
#define	___imtable_Loaded___

#include "aqsis.h"


START_NAMESPACE( Aqsis )


struct IqMessageTable
{
	public:
		virtual ~IqMessageTable()
		{}

		/** Get an error from a message table
			@param	table		The message table
			@param	error_id	The error ID
		 */
		virtual const char* getError( const int table, const int error_id ) = 0;
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	//	___imtable_Loaded___

