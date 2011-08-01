// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/// \file
/// \author Chris Foster

#ifndef AQSIS_ARRAYVIEW_H_INCLUDED
#define AQSIS_ARRAYVIEW_H_INCLUDED

#include <cassert>
#include <cstring> // for memcpy

#include <boost/type_traits/remove_cv.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>

namespace Aqsis {

//------------------------------------------------------------------------------
/// Utility metafunction to determine whether two types are the same after
/// stripping the const & volatile qualifiers
template<typename T1, typename T2>
struct is_same_nocv
    : boost::is_same<
        typename boost::remove_cv<T1>::type,
        typename boost::remove_cv<T2>::type
      >
{};


//------------------------------------------------------------------------------
/// View of an array as an array of different type.
///
/// This class should be used a base class in the Curiously Recurring Template
/// Pattern (CRTP) mixin class.  That is, to represent some particular array
/// view, a derived class should be created which inherits from ArrayView,
/// templated on the derived type, for example,
///
/// \code
///     class Vec3View : public ArrayView<Vec3View, float, V3f&>
///     {
///         //...
///     }
/// \endcode
///
/// Template parameters:
///
/// DerivedT is a class derived from ArrayView which should provide the
/// following methods.  (Note that these should be const or static.)
///
/// int elSize() const;
/// ElementRefT convertToElement(StorateType*) const;
///
/// StorageT is the type held in the underlying storage array.
///
/// ElementRefT is a reference to the type we're pretending the array is
/// made up of.  It is returned when indexing with operator[] or otherwise
/// getting at individual elements.
template<typename DerivedT, typename StorageT, typename ElementRefT>
class ArrayView
{
    private:
        StorageT* m_storage;  ///< ref to underlying storage
        int m_stride;            ///< stride between elements

    public:
        ArrayView() : m_storage(0), m_stride(0) {}

        ArrayView(StorageT* storage, int stride)
            : m_storage(storage), m_stride(stride) {}

        template<typename OtherDerived, typename OtherStorage, typename OtherRef>
        ArrayView(const ArrayView<OtherDerived, OtherStorage, OtherRef>& view,
                  typename boost::enable_if<
                      is_same_nocv<StorageT, OtherStorage>,
                      void*>::type = 0
                  )
            : m_storage(view.storage()), m_stride(view.stride()) {}

        /// Test whether the view points to valid data.
        /// For use in boolean contexts only.
        operator const void*() const { return m_storage; }

        /// Determine whether this view represents a single element
        int uniform() const { return m_stride == 0; }

        /// Get a pointer to the underlying storage
        StorageT* storage() const { return m_storage; }

        /// Get the stride between elements of underlying storage
        int stride() const { return m_stride; }

        /// Determine whether consecutive elements are consecutive in memory.
        /// The view is "dense" when there's no space between elements.
        bool isDense() const { return m_stride == derived().elSize(); }

        ElementRefT operator[](int i) const { return derived().convertToElement(m_storage + i*m_stride); }
        ElementRefT operator*() const { return derived().convertToElement(m_storage); }

        DerivedT& operator+=(int i) { m_storage += i*m_stride; return derived();}
        DerivedT& operator++() { m_storage += m_stride; return derived();}
        DerivedT operator+(int i) const { DerivedT d = derived(); d += i; return d; }

        /// Slice the view to select a regular subset of the values
        DerivedT slice(int strideMult) const
        {
            DerivedT d = derived();
            d.m_stride *= strideMult;
            return d;
        }

        /// Get the derived view type.
        ///
        /// End users probably shouldn't really need to use this.
        inline DerivedT& derived()
        {
            return static_cast<DerivedT&>(*this);
        }
        inline const DerivedT& derived() const
        {
            return static_cast<const DerivedT&>(*this);
        }
};


/// Create a sliced strided view of an existing view.
template<typename DerivedViewT, typename StorageT, typename ElementRefT>
inline DerivedViewT slice(
        const ArrayView<DerivedViewT, StorageT, ElementRefT>& view,
        int strideMult)
{
    return view.slice(strideMult);
}


/// Perform a deep copy of numElements from the source to dest view
template<typename DestViewT, typename DestStorageT, typename DestRefT,
         typename SrcViewT, typename SrcStorageT, typename SrcRefT>
inline
typename boost::enable_if< is_same_nocv<DestStorageT, SrcStorageT> >::type
copy(const ArrayView<DestViewT, DestStorageT, DestRefT>& dest,
     const ArrayView<SrcViewT, SrcStorageT, SrcRefT>& src,
     int numElements)
{
    int elSize = dest.derived().elSize();
    assert(elSize == src.derived().elSize());
    int elBytes = sizeof(DestStorageT)*elSize;
    if(dest.isDense() && src.isDense())
    {
        // If the data is densely packed, can just memcpy it.
        std::memcpy(dest.storage(), src.storage(), elBytes*numElements);
    }
    else
    {
        DestStorageT* d = dest.storage();
        int destStride = dest.stride();
        SrcStorageT* s = src.storage();
        int srcStride = src.stride();
        // Otherwise, we need to use a loop.
        for(int i = 0; i < numElements; ++i, d += destStride, s += srcStride)
            std::memcpy(d, s, elBytes);
    }
}


//------------------------------------------------------------------------------
/// View of an array as a set of shorter fixed length arrays of the same type.
template<typename T>
class BasicVecView : public ArrayView<BasicVecView<T>, T, T*>
{
    private:
        int m_elSize;

        typedef ArrayView<BasicVecView<T>, T, T*> BaseView;
        friend class ArrayView<BasicVecView<T>, T, T*>;

        static T* convertToElement(T* p) { return p; }

    public:
        BasicVecView() : BaseView(), m_elSize(0) {}

        BasicVecView(T* storage, int elSize, int stride)
            : BaseView(storage, stride),
            m_elSize(elSize)
        {}
        BasicVecView(T* storage, int elSize)
            : BaseView(storage, elSize),
            m_elSize(elSize)
        {}

        int elSize() const { return m_elSize; }
};


/// Const version of BasicVecView
template<typename T>
class ConstBasicVecView : public ArrayView<ConstBasicVecView<T>, const T, const T*>
{
    private:
        int m_elSize;

        typedef ArrayView<ConstBasicVecView<T>, const T, const T*> BaseView;
        friend class ArrayView<ConstBasicVecView<T>, const T, const T*>;

        static const T* convertToElement(const T* p) { return p; }

    public:
        ConstBasicVecView() : BaseView(), m_elSize(0) {}

        ConstBasicVecView(const T* storage, int elSize, int stride)
            : BaseView(storage, stride),
            m_elSize(elSize)
        {}
        ConstBasicVecView(const T* storage, int elSize)
            : BaseView(storage, elSize),
            m_elSize(elSize)
        {}
        ConstBasicVecView(const BasicVecView<T>& view)
            : BaseView(view),
            m_elSize(view.elSize())
        {}

        int elSize() const { return m_elSize; }
};

typedef BasicVecView<float> FvecView;
typedef ConstBasicVecView<float> ConstFvecView;


//------------------------------------------------------------------------------
/// View of a float array as an array of a different type.
///
/// Rather dirty implementation.  The well-defined way to do this would be to
/// implement reference versions of our classes T.
template<typename T>
class DataView : public ArrayView<DataView<T>, float, T&>
{
    private:
        typedef ArrayView<DataView<T>, float, T&> BaseView;
        friend class ArrayView<DataView<T>, float, T&>;

        // Number of floats used for each element
        enum { elementSize = sizeof(T)/sizeof(float) };

        // The casting here is undefined behaviour, but provided T is a plain
        // aggregate of floats, it's hard to imagine how this could lead to
        // incorrect results in a sane implementation.  Perhaps we could have
        // some troubles with strict aliasing in rather unusual circumstances.
        static T& convertToElement(float* p) { return *reinterpret_cast<T*>(p); }

    public:
        DataView(float* storage, int stride = elementSize)
            : BaseView(storage, stride) {}

        DataView(const FvecView& view)
            : BaseView(view)
        {
            assert(view.elSize() == elementSize || !this->storage());
        }

        static int elSize() { return elementSize; }
};


/// Const version of DataView
template<typename T>
class ConstDataView : public ArrayView<ConstDataView<T>, const float, const T&>
{
    private:
        typedef ArrayView<ConstDataView<T>, const float, const T&> BaseView;
        friend class ArrayView<ConstDataView<T>, const float, const T&>;

        // Number of floats used for each element
        enum { elementSize = sizeof(T)/sizeof(float) };

        // Ugh, casting here is dodgy; see comments in DataView
        static const T& convertToElement(const float* p) {
            return *reinterpret_cast<const T*>(p);
        }

    public:
        ConstDataView(const float* storage, int stride = elementSize)
            : BaseView(storage, stride) {}

        ConstDataView(const DataView<T>& view)
            : BaseView(view.storage(), view.stride()) {}

        ConstDataView(const FvecView& view)
            : BaseView(view)
        {
            assert(view.elSize() == elementSize || !this->storage());
        }

        ConstDataView(const ConstFvecView& view)
            : BaseView(view)
        {
            assert(view.elSize() == elementSize || !this->storage());
        }

        static int elSize() { return elementSize; }
};


//------------------------------------------------------------------------------
// Utility functions for dealing with data views.

/// Compute the first difference on a grid; general strided version
///
/// This function computes the first difference (the "difference between
/// adjacent grid points") using a symmetrical centred difference scheme if
/// possible and useCentred is true.  It falls back to 2nd order accurate one
/// sided differences at the grid edges.  If all else fails (or useCentred is
/// false) it uses one-sided 1st order accurate differences.
///
/// \param data - View of values from which to compute the difference.  It is
///               assumed that data[0] is the correct grid point at which to
///               compute the differences.
/// \param n - Index for one-sided checks.  One-sided differences will be used
///            if n == 0 or n == length-1.
/// \param length - number of data points along the dimension
/// \param useCentred - if true, use a centred difference scheme,
///                     if false use one-sided.
///
/// For computing derivatives on a 2D grid, the data should simply be
template<typename T>
inline T diff(const ConstDataView<T>& data, int n, int length,
              bool useCentred = true)
{
    if(useCentred && length > 2)
    {
        // 2nd order difference scheme, appropriate for use with smooth
        // shading interpolation.  A symmetric centred difference is
        // used where possible, with second order 3-point stencils at
        // the edges of the grids.
        //
        // Using a second order scheme like this is very important to
        // avoid artifacts when neighbouring grids have u and v
        // increasing in different directions, which this is
        // unavoidable for some surface types like SDS.
        if(n == 0)
            return -1.5f*data[0] + 2.0f*data[1] - 0.5f*data[2];
        else if(n == length-1)
            return 1.5f*data[0] - 2.0f*data[-1] + 0.5f*data[-2];
        else
            return 0.5f*(data[1] - data[-1]);
    }
    else
    {
        // Use 1st order one-sided difference scheme.  This is
        // appropriate for use with constant shading interpolation:
        // The one-sided difference may be thought of as a centred
        // differece *between* grid points, which corresponds to
        // micropolygons centres.
        if(n == length-1)
            return data[0] - data[-1];
        else
            return data[1] - data[0];
    }
}
template<typename T>
inline T diff(const DataView<T>& data, int n, int length, bool useCentred = true)
{
    return diff(ConstDataView<T>(data), n, length, useCentred);
}


} // namespace Aqsis

#endif // AQSIS_ARRAYVIEW_H_INCLUDED
