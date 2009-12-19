#ifndef GRIDVAR_H_INCLUDED
#define GRIDVAR_H_INCLUDED

#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

#include "arrayview.h"
#include "util.h"

struct GridvarSpec
{
    enum Type
    {
        Float,
        Point,
        Vector,
        Normal,
        Color,
        Matrix,
        String
    };

    bool uniform; ///< True if the variable is constant over the grid
    Type type;    ///< Variable type
    int arraySize; ///< Array size
    ustring name; ///< Variable name

    GridvarSpec(bool uniform, Type type, int arraySize, ustring name)
        : uniform(uniform), type(type), arraySize(arraySize), name(name) {}

    /// Get number of scalar values required for the given type
    static int scalarSize(Type type)
    {
        switch(type)
        {
            case Float:  return 1;
            case Point:  return 3;
            case Vector: return 3;
            case Normal: return 3;
            case Color:  return 3;
            case Matrix: return 16;
            case String: return 1;
        }
        assert(0); return 0;
    }

    int scalarSize() const
    {
        return scalarSize(type)*arraySize;
    }

    /// Get number of scalars for the variable on a grid with nVerts vertices.
    int storageSize(int nVerts)
    {
        return (uniform ? 1 : nVerts) * scalarSize(type);
    }
};


GridvarSpec::Type pvarToGvarType(PrimvarSpec::Type type)
{
    switch(type)
    {
        // Following are the same.
        case PrimvarSpec::Float:  return GridvarSpec::Float;
        case PrimvarSpec::Point:  return GridvarSpec::Point;
        case PrimvarSpec::Vector: return GridvarSpec::Vector;
        case PrimvarSpec::Normal: return GridvarSpec::Normal;
        case PrimvarSpec::Color:  return GridvarSpec::Color;
        case PrimvarSpec::Matrix: return GridvarSpec::Matrix;
        case PrimvarSpec::String: return GridvarSpec::String;
        // Following are different
        case PrimvarSpec::Hpoint: return GridvarSpec::Point;
    }
}


class GridvarList
{
    private:
        std::vector<GridvarSpec> m_vars;
    public:
        GridvarList(const PrimvarList& primVars)
        {
            int nvars = primVars.size();
            m_vars.reserve(nvars);
            for(int i = 0; i < nvars; ++i)
            {
                const PrimvarSpec& var = primVars[i];
                m_vars.push_back(
                    GridvarSpec(var.iclass == PrimvarSpec::Constant ||
                                var.iclass == PrimvarSpec::Uniform,
                                pvarToGvarType(var.type), var.arraySize,
                                var.name) );
            }
        }

        int add(const GridvarSpec& var)
        {
            int index = m_vars.size();
            m_vars.push_back(var);
            return index;
        }

        int size() const { return m_vars.size(); }

        int maxAggregateSize() const
        {
            int maxSize = 0;
            for(int i = 0, nvars = m_vars.size(); i < nvars; ++i)
            {
                int currSize = m_vars[i].scalarSize();
                if(maxSize < currSize)
                    maxSize = currSize;
            }
            return maxSize;
        }

        const GridvarSpec& operator[](int i) const
        {
            assert(i >= 0 && i < (int)m_vars.size());
            return m_vars[i];
        }
};


class GridvarStorage
{
    private:
        boost::scoped_array<float> m_storage;
        struct VarInfo
        {
            int offset;
            int stride;
        };
        boost::scoped_array<VarInfo> m_varInfo;
        boost::shared_ptr<GridvarList> m_vars;

    public:
        GridvarStorage(boost::shared_ptr<GridvarList> vars, int nverts)
            : m_storage(),
            m_varInfo(),
            m_vars(vars)
        {
            // Compute offsets for each variable into the shared storage, and
            // compute the total size of the storage array.
            const int nvars = m_vars->size();
            m_varInfo.reset(new VarInfo[nvars]);
            int totStorage = 0;
            for(int i = 0; i < nvars; ++i)
            {
                m_varInfo[i].offset = totStorage;
                const int nfloats = (*m_vars)[i].scalarSize();
                m_varInfo[i].stride = nfloats;
                totStorage += nverts * nfloats;
            }
            m_storage.reset(new float[totStorage]);
        }

        const GridvarList& varList() { return *m_vars; }

        /// Get allocated storage for the ith variable
        FvecView get(int i)
        {
            return FvecView(&m_storage[m_varInfo[i].offset],
                            m_varInfo[i].stride);
        }

        DataView<Vec3> P()
        {
            // Search for the P variable.
            const int nvars = m_vars->size();
            for(int i = 0; i < nvars; ++i)
            {
                if((*m_vars)[i].name == "P")
                    return DataView<Vec3>(&m_storage[m_varInfo[i].offset]);
            }
            assert(0); return DataView<Vec3>(0);
        }
};

#endif // GRIDVAR_H_INCLUDED
