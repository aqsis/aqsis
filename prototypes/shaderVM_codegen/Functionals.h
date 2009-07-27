#ifndef FUNCTIONALS_H_INCLUDED
#define FUNCTIONALS_H_INCLUDED

#include <cmath>

#include "FunctionalBase.h"
#include "Binders.h"

// binary operator functionals
struct Add : public FxnBase<double>
{
	result_type operator()(param_type a, param_type b) const
	{
		return a+b;
	}
};

struct Mul : public FxnBase<double>
{
	result_type operator()(param_type a, param_type b) const
	{
		return a*b;
	}
};

struct Div : public FxnBase<double>
{
	result_type operator()(param_type a, param_type b) const
	{
		return a/b;
	}
};

// When the second argument of Div is bound, we can make it more efficient by
// precomputing the inverse and multiplying by that.  Therefore, we specialize
// the template Bind2nd<Div>.
template<>
class Bind2nd<Div> : public FxnBase<double>
{
	private:
		const Div& m_f;
		value_type m_invarg2;

	public:
		Bind2nd(const Div& f, param_type arg2)
			: m_f(f),
			m_invarg2(1/arg2)
		{ }
		result_type operator()(param_type val) const
		{
			return val * m_invarg2;
		}
};

//------------------------------------------------------------------------------
// unary operator functionals
struct Sin : public FxnBase<double>
{
	result_type operator()(param_type a) const
	{
		return std::sin(a);
	}
};

// A difference (discrete derivative) functional.  Note that this cannot
// actually compute the derivative inside the functional, since any functional
// in the current framework can act only *locally* on its argument vector.  That
// is, you can only work with a[i]; you may never compute a[i+1] - a[i]
// directly inside the functional.
//
// Instead, we make sure that Diff::arg1NeedsDiff is true so that the first
// argument will have a difference operator applied before being passed to Diff.
struct Diff : public FxnBase<double>
{
	static const bool arg1NeedsDiff = true;

	result_type operator()(param_type a) const
	{
		// The nonlocal diff operation will occur before the parameter a is
		// passed in, therefore we don't do anything here.
		return a;
	}
};

#endif // FUNCTIONALS_H_INCLUDED
