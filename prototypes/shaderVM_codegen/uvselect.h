#ifndef UVSELECT_H_INCLUDED
#define UVSELECT_H_INCLUDED

#include <cassert>

#include "applyDerivs.h"

// Indicator constants for which arguments are varying and which are uniform.
//
// The result type is on the right.  For example, "vuv" means a function taking
// a varying and uniform argument, returning a varying value:
//
//   varying fxn(varying, uniform)
enum VarInd
{
	// unary function indicators
	VarInd_uu = (1 << 1) + 1,
	VarInd_uv = (1 << 1) + 0,
	VarInd_vv = (0 << 1) + 0,

	// binary function indicators
	VarInd_uuu = (1 << 2) + (1 << 1) + 1,
	VarInd_uuv = (1 << 2) + (1 << 1) + 0,
	VarInd_uvv = (1 << 2) + (0 << 1) + 0,
	VarInd_vuv = (0 << 2) + (1 << 1) + 0,
	VarInd_vvv = (0 << 2) + (0 << 1) + 0
};


// The following functions apply the given functional type to the arguments,
// carefully selecting the version which is used, based on whether the
// arguments and result are uniform or varying.
//
// In principle, these can be methods of the shader execution environment, and
// therefore have access to the shader context.  This context should be passed
// to the functional type on initialization, and the constructor for the
// functional can then can do any necessary initialization of variables outside
// the shader SIMD loop.

template<typename UnaryFxnlT>
void SO_ApplyFxnl(const SVarBase& a, SVarBase& result)
{
	int uniformVaryingIndicator = (a.isUniform() << 1) + result.isUniform();
	UnaryFxnlT op; // should initialize with the context here.
	switch(uniformVaryingIndicator)
	{
		case VarInd_uu:
			applyDiffs(op, uniform(a), uniform(result));
			break;
		case VarInd_uv:
			applyDiffs(op, uniform(a), varying(result));
			break;
		case VarInd_vv:
			applyDiffs(op, varying(a), varying(result));
			break;
		default:
			// Assiging varying to uniform doesn't make sense & should be a
			// compiler error, so we should never get here.
			assert(0);
			break;
	}
}

template<typename BinFxnlT>
void SO_ApplyFxnl(const SVarBase& a, const SVarBase& b, SVarBase& result)
{
	int uniformVaryingIndicator = (a.isUniform() << 2) + (b.isUniform() << 1) + result.isUniform();
	BinFxnlT op; // should initialize with the context here.
	switch(uniformVaryingIndicator)
	{
		case VarInd_uuu:
			applyDiffs(op, uniform(a), uniform(b), uniform(result));
			break;
		case VarInd_uuv:
			applyDiffs(op, uniform(a), uniform(b), varying(result));
			break;
		case VarInd_uvv:
			applyDiffs(op, uniform(a), varying(b), varying(result));
			break;
		case VarInd_vuv:
			applyDiffs(op, varying(a), uniform(b), varying(result));
			break;
		case VarInd_vvv:
			applyDiffs(op, varying(a), varying(b), varying(result));
			break;
		default:
			// Assiging varying to uniform doesn't make sense & should be a
			// compiler error, so we should never get here.
			assert(0);
			break;
	}
}

#endif // UVSELECT_H_INCLUDED
