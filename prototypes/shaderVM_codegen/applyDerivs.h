#ifndef APPLYDERIVS_H_INCLUDED
#define APPLYDERIVS_H_INCLUDED

#include "binduniform.h"

template<typename SVarT, bool needsDiff>
struct ApplyDiff
{
	static DiffSVar<SVarT> apply(const SVarT& sVar)
	{
		return DiffSVar<SVarT>(sVar);
	}
};

template<typename SVarT>
struct ApplyDiff<SVarT, false>
{
	static const SVarT& apply(const SVarT& sVar)
	{
		return sVar;
	}
};

template<>
struct ApplyDiff<SVarUniform, true>
{
	static SVarUniform apply(const SVarUniform& sVar)
	{
		return SVarUniform(0);
	}
};

template<>
struct ApplyDiff<SVarUniform, false>
{
	static const SVarUniform& apply(const SVarUniform& sVar)
	{
		return sVar;
	}
};


// Apply difference operators to those arguments which need them.
template<typename FunctionalT, typename SVarT1, typename SVarResT>
void applyDiffs(const FunctionalT& fxn, const SVarT1& a, SVarResT& result)
{
	bindUniform(fxn,
			ApplyDiff<SVarT1, FunctionalT::arg1NeedsDiff>::apply(a),
			result);
}

template<typename FunctionalT, typename SVarT1, typename SVarT2, typename SVarResT>
void applyDiffs(const FunctionalT& fxn, const SVarT1& a, const SVarT2& b,
		SVarResT& result)
{
	bindUniform(fxn,
			ApplyDiff<SVarT1, FunctionalT::arg1NeedsDiff>::apply(a),
			ApplyDiff<SVarT2, FunctionalT::arg2NeedsDiff>::apply(b),
			result);
}

#endif // APPLYDERIVS_H_INCLUDED
