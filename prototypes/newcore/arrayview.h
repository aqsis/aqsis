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
        DataView(float* storage, int stride = sizeof(T)/sizeof(float))
            : m_storage(storage),
            m_stride(stride)
        {}

        /// Test whether the view points to valid data.
        /// For use in boolean contexts only.
        operator const void*() const { return m_storage; }

        /// Indexing operators.
        ///
        /// The casting here is undefined behaviour, but provided T is a plain
        /// aggregate of floats, it's hard to imagine how this could be
        /// undefined behaviour in a sane implementation.  Perhaps we could
        /// have some troubles with strict aliasing in rather unusual
        /// circumstances.
        T& operator[](int i) { return *((T*)(m_storage + m_stride*i)); }
        const T& operator[](int i) const { return *((const T*)(m_storage + m_stride*i)); }

        DataView& operator+=(int i) { m_storage += i*m_stride; return *this; }
};


template<typename T>
class ConstDataView
{
    private:
        const float* m_storage;
        int m_stride;
    public:
        ConstDataView(const float* storage, int stride = sizeof(T)/sizeof(float))
            : m_storage(storage),
            m_stride(stride)
        {}

        ConstDataView(const DataView<T>& view)
            : m_storage(view.m_storage),
            m_stride(view.m_stride)
        {}

        /// Test whether the view points to valid data.
        /// For use in boolean contexts only.
        operator const void*() const { return m_storage; }

        const T& operator[](int i) const { return *((const T*)(m_storage + m_stride*i)); }

        ConstDataView& operator+=(int i) { m_storage += i*m_stride; return *this; }
};



/// Strided view of a float array as an array of short float vectors
class FvecView
{
    private:
        float* m_storage;
        int m_stride;
        int m_elSize;

    public:
        FvecView() : m_storage(0), m_stride(0), m_elSize(0) {}

        FvecView(float* storage, int stride, int elSize)
            : m_storage(storage),
            m_stride(stride),
            m_elSize(elSize)
        {}

        /// Test whether the view points to valid data.
        /// For use in boolean contexts only.
        operator const void*() const { return m_storage; }

        int uniform() const { return m_stride == 0; }

        /// Element access
        float* operator[](int i) { return m_storage + i*m_stride; }
        const float* operator[](int i) const { return m_storage + i*m_stride; }

        float* storage() { return m_storage; }
        const float* storage() const { return m_storage; }
        int stride() const { return m_stride; }
        int elSize() const { return m_elSize; }

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

        ConstFvecView(const float* storage, int stride, int elSize)
            : m_storage(storage),
            m_stride(stride),
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

        ConstFvecView& operator+=(int i) { m_storage += i*m_stride; return *this;}
        ConstFvecView& operator++() { m_storage += m_stride; return *this;}
};

#endif // ARRAYVIEW_H_INCLUDED
