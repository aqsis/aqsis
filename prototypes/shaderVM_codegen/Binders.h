#ifndef BINDERS_H_INCLUDED
#define BINDERS_H_INCLUDED

#include "FunctionalBase.h"

// Custom binder classes.  In principle we could use boost::bind or
// std::bind1st etc, but I prefer to have all the control here.  In particular,
// we need to be able to easily specialize these binders for some functionals
// to be bound (eg, there is a good optimization when binding the 2nd argument
// of a divison).

// Bind all arguments of a functional.  This functional just immediately
// computes the result of such an operation and caches it.
template<typename FunctionalT>
class BindAll : public FxnBase<double>
{
	private:
		const value_type m_result;

	public:
		BindAll(const FunctionalT& f, param_type arg1)
			: m_result(f(arg1))
		{ }
		BindAll(const FunctionalT& f, param_type arg1, param_type arg2)
			: m_result(f(arg1, arg2))
		{ }
		result_type operator()() const
		{
			return m_result;
		}
};

// Bind the first argument of a functional.  Caches the argument by *value*
template<typename FunctionalT>
class Bind1st : public FxnBase<double>
{
	private:
		const FunctionalT& m_f;
		const value_type m_arg2;

	public:
		static const bool arg1NeedsDiff = FunctionalT::arg2NeedsDiff;

		Bind1st(const FunctionalT& f, param_type arg2)
			: m_f(f),
			m_arg2(arg2)
		{ }
		result_type operator()(param_type val) const
		{
			return m_f(val, m_arg2);
		}
};

// Bind the second argument of a functional.  Caches the argument by *value*
template<typename FunctionalT>
class Bind2nd : public FxnBase<double>
{
	private:
		const FunctionalT& m_f;
		const value_type m_arg2;

	public:
		static const bool arg1NeedsDiff = FunctionalT::arg1NeedsDiff;

		Bind2nd(const FunctionalT& f, param_type arg2)
			: m_f(f),
			m_arg2(arg2)
		{ }
		result_type operator()(param_type val) const
		{
			return m_f(val, m_arg2);
		}
};

#endif // BINDERS_H_INCLUDED
