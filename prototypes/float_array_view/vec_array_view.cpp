#include <vector>

/**
 * 3D "vector view" prototype
 *
 * This file tests out the idea of having multiple "views" onto an array of
 * floating point values.  Sometimes it might be an advantage to view an array
 * as just a big lump of float values; other times it might be useful to see it
 * as a collection of 3D vectors.
 *
 * With a view adaptor class, we can have the best of both worlds 
 *
 *   * When we want a float array, just use the basic data.
 *   * When we want an array of vectors, we construct a view onto that data
 *     which returns vector classes referencing the data when the view is
 *     indexed.
 */


/** A holder for vector data which is *owned* by the vector class.
 */
struct Data
{
	private:
		float m_v[3];
	public:
		Data()
		{
			m_v[0] = m_v[1] = m_v[2] = 0;
		}
		Data(float* v)
		{
			m_v[0] = v[0];
			m_v[1] = v[1];
			m_v[2] = v[2];
		}
		Data(float x, float y, float z)
		{
			m_v[0] = x;
			m_v[1] = y;
			m_v[2] = z;
		}

		float& operator[](int i)
		{
			return m_v[i];
		}
		float operator[](int i) const
		{
			return m_v[i];
		}
};

/** A holder for vector data *not* owned by the vector class.  This is simply a
 * reference to data held elsewhere.
 */
struct DataRef
{
	private:
		float* m_v;
	public:
		DataRef(float* v)
			: m_v(v)
		{ }
		float& operator[](int i)
		{
			return m_v[i];
		}
		float operator[](int i) const
		{
			return m_v[i];
		}
};

/** A vector class, with the storage scheme abstracted away into the template
 * parameter DataT.
 *
 * DataT is required to have an accessible member, v, which is a pointer to the
 * first element of the vector data (there's probably other ways to do this
 * too).
 */
template<typename DataT>
class BasicVec
{
	private:
		DataT m_data;
	public:
		/// Constructors
		BasicVec()
			: m_data()
		{ }
		/// Construct from a pointer to initial data.
		BasicVec(float* v)
			: m_data(v)
		{ }
		/// Construct from a set of numbers for the componenets.
		BasicVec(float x, float y, float z)
			: m_data(x,y,z)
		{ }

		//--------------------------------------------------
		/// component access
		float& x() { return m_data[0]; }
		float& y() { return m_data[1]; }
		float& z() { return m_data[2]; }
		float x() const { return m_data[0]; }
		float y() const { return m_data[1]; }
		float z() const { return m_data[2]; }

		//--------------------------------------------------
		/// Some selected arithmetic operators
		template<typename DataT2>
		BasicVec<Data> operator+(const BasicVec<DataT2>& rhs) const
		{
			return BasicVec<Data>(
				x() + rhs.x(),
				y() + rhs.y(),
				z() + rhs.z()
			);
		}
		// Cross product
		template<typename DataT2>
		BasicVec<Data> operator%(const BasicVec<DataT2>& rhs) const
		{
			return BasicVec<Data>(
				y()*rhs.z() - z()*rhs.y(),
				z()*rhs.y() - y()*rhs.z(),
				x()*rhs.y() - y()*rhs.z()
			);
		}

		//--------------------------------------------------
		/// Assignment operators
		template<typename DataT2>
		BasicVec<DataT>& operator=(const BasicVec<DataT2>& rhs)
		{
			x() = rhs.x();
			y() = rhs.y();
			z() = rhs.z();
			return *this;
		}
		template<typename DataT2>
		BasicVec<DataT>& operator+=(const BasicVec<DataT2>& rhs)
		{
			x() += rhs.x();
			y() += rhs.y();
			z() += rhs.z();
			return *this;
		}
};

// Convenient typdefs for 3D vectors - 
// 
// Vec is a vector type which owns the underlying data
typedef BasicVec<Data> Vec;
// RefVec is a vector type which references the underlying data from somewhere
// else.
typedef BasicVec<DataRef> RefVec;


//------------------------------------------------------------------------------
/** A "view" onto an underlying array of floats that makes it look like an
 * array of vectors.
 *
 * Note that we could include a different stride here rather than the hardcoded
 * value of 3.  This would easily allow us to have other data interspersed with
 * the vectors of interest.
 */
class VecArrayView
{
	private:
		float* m_data;
	public:
		VecArrayView(float* data)
			: m_data(data)
		{ }
		RefVec operator[](int i)
		{
			return RefVec(m_data + 3*i);
		}
};


//------------------------------------------------------------------------------
int main()
{
	// at numVects = 100 we should be entirely in cache.
	const int numVecs = 100;
	// Turn the repeats up to get better timing resolution on fast machines.
	const int numRepeats = 1000000;
	std::vector<float> floatArray(3*numVecs, 1);
	std::vector<float> floatArraySrc(3*numVecs, 1);
	std::vector<Vec> vecArray(numVecs, Vec(1,1,1));
	std::vector<Vec> vecArraySrc(numVecs, Vec(1,1,1));

	// Construct views onto the dest and source arrays of floats.
	VecArrayView refVecArray(&floatArray[0]);
	VecArrayView refVecArraySrc(&floatArraySrc[0]);

	Vec v(1,2,3);
	// performance test loops.
	for(int j = 0; j < numRepeats; ++j)
	{
		for(int i = 0; i < numVecs; ++i)
		{
			//--------------------------------------------------
			// Test of the composite operation
			//
			// out[i] += v + in[i] % v;
			//
			// where % is the cross product.

//			// Array of value-vectors
//			vecArray[i] += v + vecArraySrc[i] % v;        //##bench vecArray
//
//			// Array of reference vectors
//			refVecArray[i] += v + refVecArraySrc[i] % v;  //##bench refVecArray
//
//			// C-style
//			floatArray[3*i] += v.x() + floatArraySrc[3*i+1]*v.z() - floatArraySrc[3*i+2]*v.y();      //##bench CArray
//			floatArray[3*i+1] += v.y() + floatArraySrc[3*i+2]*v.x() - floatArraySrc[3*i]*v.z();      //##bench CArray
//			floatArray[3*i+2] += v.z() + floatArraySrc[3*i]*v.y() - floatArraySrc[3*i+1]*v.x();      //##bench CArray


#if 0
//			//--------------------------------------------------
//			// test of plain old assignment
//			// out[i] = v;
//			//
//			// Array of value-vectors
//			vecArray[i] = v;        //##bench vecArray
//
//			// Array of reference vectors
//			refVecArray[i] = v;     //##bench refVecArray
//
//			// C-style
//			floatArray[3*i] = v.x();    //##bench CArray
//			floatArray[3*i+1] = v.y();  //##bench CArray
//			floatArray[3*i+2] = v.z();  //##bench CArray
#endif

		}
	}
	return 0;
}

//##description vecArray Computation with array of vectors owning their underlying represnetation (like CqVector3D)
//##description refVecArray Computation with vectors which are just a view onto an underlying float array.
//##description CArray Reference computation using plain old C-arrays 

//##CXXFLAGS -O3 -Wall -DNDEBUG
