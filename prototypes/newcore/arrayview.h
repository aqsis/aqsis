// Aqsis
// Copyright (C) 1997 - 2010, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef ARRAYVIEW_H_INCLUDED
#define ARRAYVIEW_H_INCLUDED

#include <cassert>
#include <cstring> // for memcpy

// TODO: Figure out how to implement FvecView & DataView together in the same
// class

/// Strided view of a float array as an array of short float vectors
class FvecView
{
    private:
        float* m_storage;
        int m_stride;
        int m_elSize;

    public:
        FvecView() : m_storage(0), m_stride(0), m_elSize(0) {}

        FvecView(float* storage, int elSize, int stride)
            : m_storage(storage),
            m_stride(stride),
            m_elSize(elSize)
        {}
        FvecView(float* storage, int elSize)
            : m_storage(storage),
            m_stride(elSize),
            m_elSize(elSize)
        {}

        /// Test whether the view points to valid data.
        /// For use in boolean contexts only.
        operator const void*() const { return m_storage; }

        int uniform() const { return m_stride == 0; }

        /// Element access
        float* operator[](int i) const { return m_storage + i*m_stride; }

        float* storage() const { return m_storage; }
        int stride() const { return m_stride; }
        int elSize() const { return m_elSize; }
        /// Determine whether consecutive elements are consecutive in memory.
        /// The view is "dense" when there's no space between elements.
        bool isDense() const { return m_stride == m_elSize; }

        FvecView& operator+=(int i) { m_storage += i*m_stride; return *this;}
        FvecView& operator++() { m_storage += m_stride; return *this;}
};


/// Const version of FvecView
class ConstFvecView
{
    private:
        const float* m_storage;
        int m_stride;
        int m_elSize;

    public:
        ConstFvecView() : m_storage(0), m_stride(0), m_elSize(0) {}

        ConstFvecView(const float* storage, int elSize, int stride)
            : m_storage(storage),
            m_stride(stride),
            m_elSize(elSize)
        {}
        ConstFvecView(const float* storage, int elSize)
            : m_storage(storage),
            m_stride(elSize),
            m_elSize(elSize)
        {}

        ConstFvecView(const FvecView& view)
            : m_storage(view.storage()),
            m_stride(view.stride()),
            m_elSize(view.elSize())
        {}

        /// Test whether the view points to valid data.
        /// For use in boolean contexts only.
        operator const void*() const { return m_storage; }

        int uniform() const { return m_stride == 0; }

        /// Element access
        const float* operator[](int i) const { return m_storage + i*m_stride; }

        const float* storage() const { return m_storage; }
        int stride() const { return m_stride; }
        int elSize() const { return m_elSize; }
        /// Determine whether consecutive elements are consecutive in memory.
        /// The view is "dense" when there's no space between elements.
        bool isDense() const { return m_stride == m_elSize; }

        ConstFvecView& operator+=(int i) { m_storage += i*m_stride; return *this;}
        ConstFvecView& operator++() { m_storage += m_stride; return *this;}
};


//------------------------------------------------------------------------------
/// View of a float array as an array of a different type.
///
/// Very quick & dirty implementation.  The well-defined way to do this is to
/// implement reference versions of our classes T.
template<typename T>
class DataView
{
    private:
        float* m_storage;
        int m_stride;
        template<typename> friend class ConstDataView;
    public:
        /// Number of floats used for each element
        enum { elementSize = sizeof(T)/sizeof(float) };

        DataView(float* storage, int stride = elementSize)
            : m_storage(storage),
            m_stride(stride)
        {}

        DataView(FvecView view)
            : m_storage(view.storage()),
            m_stride(view.stride())
        {
            assert(view.elSize() == elementSize);
        }

        /// Test whether the view points to valid data.
        /// For use in boolean contexts only.
        operator const void*() const { return m_storage; }

        /// Indexing operator.
        ///
        /// The casting here is undefined behaviour, but provided T is a plain
        /// aggregate of floats, it's hard to imagine how this could be
        /// undefined behaviour in a sane implementation.  Perhaps we could
        /// have some troubles with strict aliasing in rather unusual
        /// circumstances.
        T& operator[](int i) const { return *((T*)(m_storage + m_stride*i)); }

        float* storage() const { return m_storage; }
        int stride() const { return m_stride; }
        /// Determine whether consecutive elements are consecutive in memory.
        /// The view is "dense" when there's no space between elements.
        bool isDense() const { return m_stride == elementSize; }

        DataView& operator+=(int i) { m_storage += i*m_stride; return *this; }
};


template<typename T>
class ConstDataView
{
    private:
        const float* m_storage;
        int m_stride;

    public:
        /// Number of floats used for each element
        enum { elementSize = sizeof(T)/sizeof(float) };

        ConstDataView(const float* storage, int stride = elementSize)
            : m_storage(storage),
            m_stride(stride)
        {}

        ConstDataView(const DataView<T>& view)
            : m_storage(view.m_storage),
            m_stride(view.m_stride)
        {}

        ConstDataView(const FvecView& view)
            : m_storage(view.storage()),
            m_stride(view.stride())
        {
            assert(view.elSize() == elementSize);
        }
        ConstDataView(const ConstFvecView& view)
            : m_storage(view.storage()),
            m_stride(view.stride())
        {
            assert(view.elSize() == elementSize);
        }

        /// Test whether the view points to valid data.
        /// For use in boolean contexts only.
        operator const void*() const { return m_storage; }

        const T& operator[](int i) const { return *((const T*)(m_storage + m_stride*i)); }

        /// Get the base storage for this view
        const float* storage() const { return m_storage; }
        /// Get the stride of the base storage between elements
        int stride() const { return m_stride; }
        /// Determine whether consecutive elements are consecutive in memory.
        /// The view is "dense" when there's no space between elements.
        bool isDense() const { return m_stride == elementSize; }

        ConstDataView& operator+=(int i) { m_storage += i*m_stride; return *this; }
};


//------------------------------------------------------------------------------
// Utility functions for dealing with data views.

/// Copy one float vec view into another.  Arguments order is like memcpy.
inline void copy(FvecView dest, ConstFvecView src, int nelems)
{
    assert(dest.elSize() == src.elSize());
    if(dest.isDense() && src.isDense())
    {
        // If the data is densely packed, can just memcpy it.
        std::memcpy(dest.storage(), src.storage(),
                    sizeof(float)*dest.elSize()*nelems);
    }
    else
    {
        // Otherwise, we need to use a loop.
        for(int i = 0; i < nelems; ++i)
            std::memcpy(dest[i], src[i], sizeof(float)*dest.elSize());
    }
}


/// Copy one data view into another.  Arguments order is like memcpy.
template<typename T>
inline void copy(DataView<T> dest, ConstDataView<T> src, int nelems)
{
    if(dest.isDense() && src.isDense())
    {
        // If the data is densely packed, can just memcpy it.
        std::memcpy(dest.storage(), src.storage(),
                    sizeof(float)*dest.elementSize*nelems);
    }
    else
    {
        // Otherwise, we need to use a loop.
        for(int i = 0; i < nelems; ++i)
            dest[i] = src[i];
    }
}

template<typename T>
inline void copy(DataView<T> dest, DataView<T> src, int nelems)
{
    copy(dest, ConstDataView<T>(src), nelems);
}

#endif // ARRAYVIEW_H_INCLUDED
