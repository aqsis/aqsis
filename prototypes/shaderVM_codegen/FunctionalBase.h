#ifndef FUNCTIONALBASE_H_INCLUDED
#define FUNCTIONALBASE_H_INCLUDED

#include <boost/call_traits.hpp>

// base struct to provide functionals with appropriate typedefs.
template<typename T>
struct FxnBase
{
	// The following are typedefs to determine how the worker functionals will
	// hold, pass back, and accept parameters of type T.
	typedef typename boost::call_traits<T>::value_type result_type;
	typedef typename boost::call_traits<T>::value_type value_type;
	typedef typename boost::call_traits<T>::param_type param_type;

	// "Override" the following by hiding them in a child class to indicate
	// which arguments of a functional need to have a difference operator
	// applied to them.  The default is that none need difference operators.
	static const bool arg1NeedsDiff = false;
	static const bool arg2NeedsDiff = false;
};

#endif // FUNCTIONALBASE_H_INCLUDED
