#ifndef BINDUNIFORM_H_INCLUDED
#define BINDUNIFORM_H_INCLUDED

#include "Array.h"
#include "Binders.h"
#include "applyvarying.h"

// Overloaded functions which bind any uniform variable into the slots of the
// accompanying functional.

// single-argument functional.
template<typename FunctionalT, typename SVarT1, typename SVarResT>
void bindUniform(const FunctionalT& fxn, const SVarT1& a, SVarResT& result)
{
	// vv
	applyVarying(fxn, a, result);
}
template<typename FunctionalT, typename SVarResT>
void bindUniform(const FunctionalT& fxn, const SVarUniform& a, SVarResT& result)
{
	// uv
	applyVarying(BindAll<FunctionalT>(fxn, a.value()), result);
}
template<typename FunctionalT>
void bindUniform(const FunctionalT& fxn, const SVarUniform& a, SVarUniform& result)
{
	// uu
	result.value() = fxn(a.value());
}

// two-argument functional.
template<typename FunctionalT, typename SVarT1, typename SVarT2, typename SVarResT>
void bindUniform(const FunctionalT& fxn, const SVarT1& a, const SVarT2& b,
		SVarResT& result)
{
	// vvv
	applyVarying(fxn, a, b, result);
}
template<typename FunctionalT, typename SVarT1, typename SVarResT>
void bindUniform(const FunctionalT& fxn, const SVarT1& a, const SVarUniform& b,
		SVarResT& result)
{
	// vuv
	applyVarying(Bind2nd<FunctionalT>(fxn, b.value()), a, result);
}
template<typename FunctionalT, typename SVarT2, typename SVarResT>
void bindUniform(const FunctionalT& fxn, const SVarUniform& a, const SVarT2& b,
		SVarResT& result)
{
	// uvv
	applyVarying(Bind1st<FunctionalT>(fxn, a.value()), b, result);
}
template<typename FunctionalT, typename SVarResT>
void bindUniform(const FunctionalT& fxn, const SVarUniform& a, const SVarUniform& b,
		SVarResT& result)
{
	// uuv
	applyVarying(BindAll<FunctionalT>(fxn, a.value(), b.value()), result);
}
template<typename FunctionalT>
void bindUniform(const FunctionalT& fxn, const SVarUniform& a, const SVarUniform& b,
		SVarUniform& result)
{
	// uuu
	result.value() = fxn(a.value(), b.value());
}

#endif // BINDUNIFORM_H_INCLUDED
