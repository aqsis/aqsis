//------------------------------------------------------------------------------
/**
 *	@file	refcount.h
 *	@author	Paul Gregory
 *	@brief	Declare a reference counting class.
 *
 *	Last change by:		$Author$
 *	Last change date:	$Date$
 */
//------------------------------------------------------------------------------

#ifndef	___refcount_Loaded___
#define	___refcount_Loaded___

#include	<aqsis/aqsis.h>
#include	<aqsis/util/sstring.h>
#include	<vector>
#include	<list>

/**
 * These are debug and non-debug versions of the macros ADDREF and RELEASEREF.
 *
 * ADDREF increments the reference count of a pointer to a CqRefCount-derived 
 * class, while RELEASEREF decrements the reference count, causing the object 
 * to be freed when the reference count is less than or equal to zero.
 * 
 * The non-debug versions simply call AddRef() and Release() methods on the
 * object (which is assumed to be a pointer to a CqRefCount-derived class).  
 * The debug versions include the file name and line where the macro was 
 * called, to enable this information to be tracked for debugging purposes.
 */
#ifdef _DEBUG
#define ADDREF(x) (x)->AddRef(__FILE__, __LINE__)
#define RELEASEREF(x) (x)->Release(__FILE__, __LINE__)
#else
#define ADDREF(x) (x)->AddRef()
#define RELEASEREF(x) (x)->Release()
#endif

#ifdef _DEBUG
// report_refcounts() should be called at the end of rendering, at such a
// time when all reference-counted objects should have been released.
#ifdef	AQSIS_SYSTEM_WIN32
extern "C" __declspec(dllexport) void report_refcounts();
#else
extern "C" void report_refcounts();
#endif	// AQSIS_SYSTEM_WIN32

class RefCountRecord;
typedef std::vector<RefCountRecord*> RecordVector;
typedef std::vector<RefCountRecord*>::iterator RecordIterator;
typedef std::vector<RefCountRecord*>::const_iterator ConstRecordIterator;
#endif

/**
 * \class CqRefCount
 *
 * This class provides a reference counting form of memory management for
 * Aqsis.
 *
 * This class is implemented in two different ways.  One way is a debug
 * version, which keeps track of AddRef() and Release() calls so that bugs
 * can be tracked easily.  The second version is a much faster, minimal
 * implementation that is used for non-debug builds.
 * 
 * The basic idea with this class is that derived classes will have the
 * AddRef() function called whenever a new object needs access to the
 * derived class.  Calling AddRef() effectively marks the derived class as
 * "in use", and prevents it from being de-allocated.  When the object no 
 * longer needs the derived class, it will call Release().  Finally, when 
 * the last class has called Release(), the reference counter realises
 * that nobody needs it anymore and automatically deletes itself.
 */
class CqRefCount
{

#ifdef _DEBUG

	public:
		/// These methods are the debug versions.
		CqRefCount();
		CqRefCount( const CqRefCount& From );
		virtual ~CqRefCount();
		TqInt RefCount() const;
		void AddRef(const TqChar* file, TqInt line);
		void Release(const TqChar* file, TqInt line);
		void dump() const;
		/**
		 * Returns the name of the class.  This function can be
		 * overridden in sub-classes to provide better debugging
		 * information.  Remember to only include it if _DEBUG
		 * is defined.
		 */
		virtual Aqsis::CqString className() const
		{
			return Aqsis::CqString("unknown");
		}

	private:
		/// Record of reference counting events for this
		///  particular instance of CqRefCount.
		RecordVector m_records;
		/// Flag that, when true, indicates that the instance had
		///  ADDREF called at least once.
		bool m_addRefCalled;

#else ///< #ifdef _DEBUG

		/// The methods below are inlined for use in a non-debug
		///  compile.
	public:
		/// Constructor
		CqRefCount() : m_cReferences( 0 )
		{}

		/// Copy Constructor, does not copy reference count.
		CqRefCount( const CqRefCount& From )
		{}
		virtual ~CqRefCount()
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
			m_cReferences--;
			if ( m_cReferences <= 0 )
				delete( this );
		}

#endif ///< #ifdef _DEBUG

	private:
		TqInt	m_cReferences;		///< Count of references to this object.
};


#endif	//	___refcount_Loaded___
