#ifndef APPLYVARYING_H_INCLUDED
#define APPLYVARYING_H_INCLUDED

#include "Array.h"

// Implementation of varying loops, allowing different array types (arrays
// might be wrapped in a difference operator)
template<typename FunctionalT, typename SVarResT>
void applyVarying(const FunctionalT& fxn, SVarResT& result)
{
	for(int i = 0, end = result.length(); i < end; ++i)
		result[i] = fxn();
}

template<typename FunctionalT, typename SVarT1, typename SVarResT>
void applyVarying(const FunctionalT& fxn, const SVarT1& a, SVarResT& result)
{
	for(int i = 0, end = result.length(); i < end; ++i)
		result[i] = fxn(a[i]);
}

template<typename FunctionalT, typename SVarT1, typename SVarT2, typename SVarResT>
void applyVarying(const FunctionalT& fxn, const SVarT1& a, const SVarT2& b, SVarResT& result)
{
	for(int i = 0, end = result.length(); i < end; ++i)
		result[i] = fxn(a[i], b[i]);
}

#endif // APPLYVARYING_H_INCLUDED
