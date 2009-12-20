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

        const T& operator[](int i) const { return *((const T*)(m_storage + m_stride*i)); }

        ConstDataView& operator+=(int i) { m_storage += i*m_stride; return *this; }
};



/// Strided view of a float array as an array of shorter float vectors
class FvecView
{
    private:
        float* m_storage;
        int m_stride;
        friend class ConstFvecView;
    public:
        FvecView(float* storage, int stride)
            : m_storage(storage),
            m_stride(stride)
        {}

        float* operator[](int i) { return m_storage + i*m_stride; }
        const float* operator[](int i) const { return m_storage + i*m_stride; }
        float* operator*() { return m_storage; }
        const float* operator*() const { return m_storage; }

        FvecView& operator+=(int i) { m_storage += i*m_stride; return *this;}
        FvecView& operator++() { m_storage += m_stride; return *this;}

        /// Return the length of the contained float vectors
        int size() const { return m_stride; }
};


class ConstFvecView
{
    private:
        const float* m_storage;
        int m_stride;
    public:
        ConstFvecView(const float* storage, int stride)
            : m_storage(storage),
            m_stride(stride)
        {}

        ConstFvecView(const FvecView& view)
            : m_storage(view.m_storage),
            m_stride(view.m_stride)
        {}

        const float* operator[](int i) const { return m_storage + i*m_stride; }
        const float* operator*() const { return m_storage; }

        ConstFvecView& operator+=(int i) { m_storage += i*m_stride; return *this;}
        ConstFvecView& operator++() { m_storage += m_stride; return *this;}

        int size() const { return m_stride; }
};

#endif // ARRAYVIEW_H_INCLUDED
