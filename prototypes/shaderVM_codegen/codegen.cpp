#include <cassert>

#include "Array.h"
#include "Binders.h"
#include "Functionals.h"
#include "uvselect.h"
#include "applyvarying.h"

// Template-based code generator prototype for the aqsis shader VM
//
// The code generator is designed to be as simple as possible, while hopefully
// having sufficient generality to allow any of the shadeops in the real
// shaderVM to be constructed with it.  There is one obvious caveat to this:
// the prototype only deals with SVars (ie, shader data) of a single type.
//
// Two things will need to be changed to fix this problem:
// 1) Parametrization of a lot of the basic templates based on argument & return types
// 2) Introduction of a superclass for all shader data arrays, and casting to
//    the appropriate subclass array type for particular shadeops.  This can be
//    done via a kind of specialized substitue for dynamic_cast, or simply
//    using a blatent static_cast, depending on the level of safety we want.


//------------------------------------------------------------------------------
// Some hand-coded shadeop functions to benchmark our template-based code
// generator against.

void SO_add_byhand(const SVarBase& a, const SVarBase& b, SVarBase& result)
{
	int uniformVaryingIndicator = (a.isUniform() << 2) + (b.isUniform() << 1) + result.isUniform();
	switch(uniformVaryingIndicator)
	{
		case VarInd_uuu:
			uniform(result).value() = uniform(a).value() + uniform(b).value();
			break;
		case VarInd_uuv:
			{
				double r0 = uniform(a).value() + uniform(b).value();
				SVar& resV = varying(result);
				for(int i = 0, end = resV.length(); i < end; ++i)
					resV[i] = r0;
			}
			break;
		case VarInd_uvv:
			{
				double a0 = uniform(a).value();
				const SVar& bV = varying(b);
				SVar& resV = varying(result);
				for(int i = 0, end = resV.length(); i < end; ++i)
					resV[i] = a0 + bV[i];
			}
			break;
		case VarInd_vuv:
			{
				const SVar& aV = varying(a);
				double b0 = uniform(b).value();
				SVar& resV = varying(result);
				for(int i = 0, end = resV.length(); i < end; ++i)
					resV[i] = aV[i] + b0;
			}
			break;
		case VarInd_vvv:
			{
				const SVar& aV = varying(a);
				const SVar& bV = varying(b);
				SVar& resV = varying(result);
				for(int i = 0, end = resV.length(); i < end; ++i)
					resV[i] = aV[i] + bV[i];
			}
			break;
		default:
			// Assiging varying to uniform doesn't make sense & should be a
			// compiler error.
			assert(0);
			break;
	}
}

void SO_div_byhand(const SVarBase& a, const SVarBase& b, SVarBase& result)
{
	int uniformVaryingIndicator = (a.isUniform() << 2) + (b.isUniform() << 1) + result.isUniform();
	switch(uniformVaryingIndicator)
	{
		case VarInd_uuu:
			uniform(result).value() = uniform(a).value() + uniform(b).value();
			break;
		case VarInd_uuv:
			{
				double r0 = uniform(a).value() / uniform(b).value();
				SVar& resV = varying(result);
				for(int i = 0, end = resV.length(); i < end; ++i)
					resV[i] = r0;
			}
			break;
		case VarInd_uvv:
			{
				double a0 = uniform(a).value();
				const SVar& bV = varying(b);
				SVar& resV = varying(result);
				for(int i = 0, end = resV.length(); i < end; ++i)
					resV[i] = a0 / bV[i];
			}
			break;
		case VarInd_vuv:
			{
				const SVar& aV = varying(a);
				double invb0 = uniform(b).value();
				SVar& resV = varying(result);
				for(int i = 0, end = resV.length(); i < end; ++i)
					resV[i] = aV[i] * invb0;
			}
			break;
		case VarInd_vvv:
			{
				const SVar& aV = varying(a);
				const SVar& bV = varying(b);
				SVar& resV = varying(result);
				for(int i = 0, end = resV.length(); i < end; ++i)
					resV[i] = aV[i] / bV[i];
			}
			break;
		default:
			// Assiging varying to uniform doesn't make sense & should be a
			// compiler error.
			assert(0);
			break;
	}
}

void SO_sin_byhand(const SVarBase& a, SVarBase& result)
{
	int uniformVaryingIndicator = (a.isUniform() << 1) + result.isUniform();
	switch(uniformVaryingIndicator)
	{
		case VarInd_uu:
			uniform(result).value() = std::sin(uniform(a).value());
			break;
		case VarInd_uv:
			{
				double r0 = std::sin(uniform(a).value());
				SVar& resV = varying(result);
				for(int i = 0, end = resV.length(); i < end; ++i)
					resV[i] = r0;
			}
			break;
		case VarInd_vv:
			{
				const SVar& aV = varying(a);
				SVar& resV = varying(result);
				for(int i = 0, end = resV.length(); i < end; ++i)
					resV[i] = std::sin(aV[i]);
			}
			break;
		default:
			// Assiging varying to uniform doesn't make sense & should be a
			// compiler error.
			assert(0);
			break;
	}
}

void SO_diff_byhand(const SVarBase& a, SVarBase& result)
{
	int uniformVaryingIndicator = (a.isUniform() << 1) + result.isUniform();
	switch(uniformVaryingIndicator)
	{
		case VarInd_uu:
			uniform(result).value() = 0;
			break;
		case VarInd_uv:
			{
				SVar& resV = varying(result);
				for(int i = 0, end = resV.length(); i < end; ++i)
					resV[i] = 0;
			}
			break;
		case VarInd_vv:
			{
				const SVar& aV = varying(a);
				SVar& resV = varying(result);
				resV[0] = aV[1] - aV[0];
				for(int i = 1, end = resV.length(); i < end; ++i)
					resV[i] = aV[i] - aV[i-1];
			}
			break;
		default:
			// Assiging varying to uniform doesn't make sense & should be a
			// compiler error.
			assert(0);
			break;
	}
}


//------------------------------------------------------------------------------
// The follwoing are shadeops generated using the templated code generator
// instead of written out explicitly.
void SO_add_template(const SVarBase& a, const SVarBase& b, SVarBase& result)
{
	SO_ApplyFxnl<Add>(a, b, result);
}

void SO_div_template(const SVarBase& a, const SVarBase& b, SVarBase& result)
{
	SO_ApplyFxnl<Div>(a, b, result);
}

void SO_sin_template(const SVarBase& a, SVarBase& result)
{
	SO_ApplyFxnl<Sin>(a, result);
}

void SO_diff_template(const SVarBase& a, SVarBase& result)
{
	SO_ApplyFxnl<Diff>(a, result);
}


//------------------------------------------------------------------------------
// Test main
int main()
{
	const int arrayLength = 200;
	// a, b, r are varying.
	SVar a(arrayLength);
	SVar b(arrayLength);
	SVar r(arrayLength);

	// aU, bU, rU are uniform
	SVarUniform aU;
	SVarUniform bU;
	SVarUniform rU;

	const int numIters = 10000000;
	for(int j = 0; j < numIters; ++j)
	{
		// Benchmarks for addition.
		// SO_add_byhand(a,b,r);      //##bench SO_add_byhand
		// SO_add_template(a,b,r);    //##bench SO_add_template

		// Benchmarks for division, showing optimization.
		// SO_div_byhand(a,bU,r);      //##bench SO_div_byhand
		// SO_div_template(a,bU,r);    //##bench SO_div_template

		// Benchmarks for sin()
		// SO_sin_byhand(a, r);         //##bench SO_sin_byhand
		// SO_sin_template(a, r);       //##bench SO_sin_template

		// Benchmarks for a non-local operation (derivative)
		// SO_diff_byhand(aU, rU);         //##bench SO_diff_byhand
		// SO_diff_template(aU, rU);       //##bench SO_diff_template
	}

	return 0;
}

//------------------------------------------------------------------------------
// Benchmark descriptions:

//##description SO_add_byhand hand-optimized version of SO_add
//##description SO_add_template SO_add version using templated code generation.

//##description SO_div_byhand hand-optimized version of SO_div
//##description SO_div_template SO_div version using templated code generation.

//##description SO_sin_byhand hand-optimized version of SO_sin
//##description SO_sin_template version of SO_sin using templated code generation.

//##description SO_diff_byhand hand optimized version of SO_diff.  This has an unfair advantage, since this version of the code won't work for shader running states.
//##description SO_diff_template version of SO_diff using templated code generation.

// Compiler options
//##CXXFLAGS -DNDEBUG -O3
//##//CXX g++-4.1.2


//------------------------------------------------------------------------------
// Running the benchmarks above on a athlon64 with g++-3.4.6, shows that the
// template code generator is working exactly as we'd like it to.  That is, all
// the versions using templates are just as fast - ok, not quite, but very
// close to - the hand-coded versions at the top of this file.

