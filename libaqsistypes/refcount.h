//------------------------------------------------------------------------------
/**
 *	@file	refcount.h
 *	@author	Paul Gregory
 *	@brief	Declare a reference counting class.
 *
 *	Last change by:		$Author: pgregory $
 *	Last change date:	$Date: 2002/10/31 11:51:14 $
 */ 
//------------------------------------------------------------------------------

#ifndef	___refcount_Loaded___
#define	___refcount_Loaded___

#include	"aqsis.h"


class CqRefCount
{
	public:
		/// Constructor
		CqRefCount() : m_cReferences( 0 )
		{}
		/// Copy Constructor, does not copy reference count.



		CqRefCount( const CqRefCount& From )
		{}
		virtual	~CqRefCount()
		{}


		TqInt	RefCount() const
		{
			return ( m_cReferences );
		}
		void	AddRef()
		{
			m_cReferences++;
		}
		void	Release()
		{
			m_cReferences--; if ( m_cReferences <= 0 ) delete( this );
		}


	private:
		TqInt	m_cReferences;		///< Count of references to this object.
}
;


#endif	//	___refcount_Loaded___
