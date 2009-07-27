#ifndef ARRAY_H_INCLUDED
#define ARRAY_H_INCLUDED

#include <boost/utility.hpp>

#include <vector>

struct SVarBase
{
	virtual bool isUniform() const = 0;
	virtual ~SVarBase() {}
};

// A trivial dynamic array "shader variable" class, designed to roughly model a
// shader data vector of type double.
class SVar : public SVarBase, boost::noncopyable
{
	private:
		double* m_a;
		int m_length;
	public:
		SVar(int length)
			: m_a(new double[length]),
			m_length(length)
		{ }
		~SVar()
		{
			delete[] m_a;
		}

		double& operator[](int i)
		{
			return m_a[i];
		}
		double operator[](int i) const
		{
			return m_a[i];
		}

		int length() const
		{
			return m_length;
		}
		virtual bool isUniform() const
		{
			return false;
		}
};

// cast to varying.
const SVar& varying(const SVarBase& sVar)
{
	return static_cast<const SVar&>(sVar);
}
SVar& varying(SVarBase& sVar)
{
	return static_cast<SVar&>(sVar);
}

class SVarUniform : public SVarBase
{
	private:
		double m_a;
	public:
		SVarUniform(double a = 0)
			: m_a(a)
		{ }

		double& value()
		{
			return m_a;
		}
		double value() const
		{
			return m_a;
		}

		virtual bool isUniform() const
		{
			return true;
		}
};

// cast to uniform.
const SVarUniform& uniform(const SVarBase& sVar)
{
	return static_cast<const SVarUniform&>(sVar);
}
SVarUniform& uniform(SVarBase& sVar)
{
	return static_cast<SVarUniform&>(sVar);
}

// Class to hold indexing arrays for optimizing differentiation.
class DiffIndex
{
	private:
		std::vector<int> m_idx1;
		std::vector<int> m_idx2;

		void initIndexVectors(int length)
		{
			m_idx1.resize(length);
			m_idx2.resize(length);
			m_idx1[0] = 0;
			m_idx2[0] = 1;
			for(int i = 1; i < length; ++i)
			{
				m_idx1[i] = i-1;
				m_idx2[i] = i;
			}
		}
	public:
		DiffIndex(int length)
			: m_idx1(length),
			m_idx2(length)
		{
			initIndexVectors(length);
		}

		double diff(const SVar& a, int i)
		{
			return a[m_idx2[i]] - a[m_idx1[i]];
		}

		int length() const
		{
			return m_idx1.size();
		}
		void setLength(int newLen)
		{
			if(length() != newLen)
			{
				initIndexVectors(newLen);
			}
		}
};

// An adaptor for shader vars which applies a difference operator to the
// underlying array elements.
template<typename SVarT>
class DiffSVar : public SVarBase
{
	private:
		// There are various possibilities for calculating difference operators
		// on a grid:
		//
		// One possibility is to compute them at construction-time, and store
		// the results in a seperate buffer. However, this is grossly
		// inefficient for cases when the shader running state has most
		// elements turned off, and probaboy would cause excessive dynamic
		// allocation / deallocation in any case.
		//
		// Another possibility is to store a reference to the underlying data,
		// and compute the derivative on-demand, directly from the 1D grid
		// index.  In 1D this isn't too bad, but in 2D it involves the very
		// expensive integer div or mod to go from the 1D grid index to the 2D
		// grid coordinates.
		//
		// It may be better for the shader execution environment to provide a
		// special "difference object", which has appropriate indexing arrays
		// describing the difference kernel at each SIMD index i.  That is,
		//
		// diff(a, i) = a[highIndex[i]] - a[lowIndex[i]];
		//
		// A prototype implementation of the idea is given in the DiffIndex
		// class, and seems to be comparably efficient to the direct
		// calculation on a 1D grid.  Throwing a mod or integer divison
		// operation in there to represent 2D considerations shows that it's
		// *much* more efficient in that case.
		const SVarT& m_a;
		static DiffIndex sm_diffIdx;
	public:
		DiffSVar(const SVarT& a)
			: m_a(a)
		{
			sm_diffIdx.setLength(m_a.length());
		}

		double operator[](int i) const
		{
			// Currently using a first-order backward difference here for
			// simplicity, but a second-order centred difference would probably
			// be better.
//			if(i > 0)
//				return m_a[i] - m_a[i-1];
//			else
//				return m_a[1] - m_a[0];
			// The alternative is to use indexing arrays.  This will be very
			// important for efficiency when it comes to two dimensional grids
			// and higher order differencing schemes.
			return sm_diffIdx.diff(m_a, i);
		}

		int length() const
		{
			return m_a.length();
		}
		virtual bool isUniform() const
		{
			return m_a.isUniform();
		}
};

template<typename SVarT>
DiffIndex DiffSVar<SVarT>::sm_diffIdx(0);

#endif // ARRAY_H_INCLUDED
