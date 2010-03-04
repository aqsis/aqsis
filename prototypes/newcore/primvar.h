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

#ifndef PRIMVAR_H_INCLUDED
#define PRIMVAR_H_INCLUDED

#include <algorithm>
#include <stdexcept>
#include <vector>

#include <boost/scoped_array.hpp>

#include "util.h"
#include "arrayview.h"
#include "varspec.h"

struct IclassStorage;

//------------------------------------------------------------------------------
/// Full type specification for a primitive variable
///
/// In general, a primitive variable (primvar) is a named, spatially dependent
/// field attached to the surface of a geometric primitive.  That is, it's a
/// variable which has a value dependent on position on the surface.  Typically
/// a primvar is attached to the vertices of a primitive and interpolated to
/// arbitrary positions on the surface using one of several interpolation
/// rules, as specified by the variable's interpolation class or "iclass":
///
///   - Constant: constant over the entire primitive
///   - Uniform: piecewise constant on geometric elements (eg, constant on
///              faces)
///   - Varying: linearly interpolated over elements
///   - Vertex: interpolated using the same rules as the vertex position
///             primvar
///   - FaceVarying: linearly interpolated over elements (faces), but is
///                  specified per-face so may be discontinuous at face
///                  boundaries
///   - FaceVertex: interpolated the same as Vertex when continuous at face
///                 boundaries.  When discontinuous, use some geometry-specific
///                 rule to behave something like FaceVarying.
///
struct PrimvarSpec : public VarSpec
{
    /// Interpolation class.
    enum Iclass
    {
        Constant,
        Uniform,
        Varying,
        Vertex,
        FaceVarying,
        FaceVertex
    };

    Iclass iclass;

    PrimvarSpec(Iclass iclass, Type type, int arraySize, ustring name)
        : VarSpec(type, arraySize, name), iclass(iclass) {}

    /// Return the number of scalar values required for the variable
    int storageSize(const IclassStorage& storCount) const;
};


/// Some standard primitive variable names for convenience.
namespace Primvar
{
    extern const PrimvarSpec P;
    extern const PrimvarSpec N;
    extern const PrimvarSpec Cs;
    extern const PrimvarSpec st;
}


typedef BasicVarSet<PrimvarSpec, StdIndices> PrimvarSet;


//------------------------------------------------------------------------------
/// Storage requirements for various interpolation classes
///
/// The number of elements of a Primvar required for each interpolation class
/// depends on the type of geometry.  This struct holds such numbers in a
/// geometry-neutral format.
struct IclassStorage
{
    int uniform;
    int varying;
    int vertex;
    int facevarying;
    int facevertex;

    IclassStorage(int uniform, int varying, int vertex, int facevarying, int facevertex)
        : uniform(uniform),
        varying(varying),
        vertex(vertex),
        facevarying(facevarying),
        facevertex(facevertex)
    { }

    int storage(PrimvarSpec::Iclass iclass) const
    {
        switch(iclass)
        {
            case PrimvarSpec::Constant:    return 1;
            case PrimvarSpec::Uniform:     return uniform;
            case PrimvarSpec::Varying:     return varying;
            case PrimvarSpec::Vertex:      return vertex;
            case PrimvarSpec::FaceVarying: return facevarying;
            case PrimvarSpec::FaceVertex:  return facevertex;
        }
        assert(0); return -1; // Kill bogus compiler warning.
    }
};


//------------------------------------------------------------------------------
class PrimvarStorage : public RefCounted
{
    private:
        boost::scoped_array<float> m_storage;
        boost::scoped_array<FvecView> m_views;
        IclassStorage m_storCount;
        PrimvarSet m_vars;

        friend class PrimvarStorageBuilder;

        template<typename VarIterT>
        PrimvarStorage(VarIterT varBegin, VarIterT varEnd,
                       const IclassStorage& storCount)
            : m_storage(),
            m_views(),
            m_storCount(storCount),
            m_vars(varBegin, varEnd)
        {
            const int nvars = m_vars.size();
            // Compute the total size of the storage array & allocate
            int totStorage = 0;
            for(VarIterT i = varBegin; i != varEnd; ++i)
                totStorage += i->storageSize(storCount);
            m_storage.reset(new float[totStorage]);
            // Cache data views for each variable.
            m_views.reset(new FvecView[nvars]);
            int offset = 0;
            VarIterT var = varBegin;
            for(int i = 0; i < nvars; ++i, ++var)
            {
                // Copy the data over
                const int size = var->storageSize(storCount);
                float* data = &m_storage[0] + offset;
                std::memcpy(data, var->data, size*sizeof(float));
                // Record a view of the data
                const int elSize = var->scalarSize();
                m_views[i] = FvecView(data, elSize);
                offset += size;
            }
        }

    public:
        /// Get the set of contained variables
        const PrimvarSet& varSet() const { return m_vars; }

        /// Get allocated storage for the ith variable
        FvecView get(int i) { return m_views[i]; }
        ConstFvecView get(int i) const { return m_views[i]; }

        /// Get a view of the vertex position data
        DataView<Vec3> P()
        {
            int Pidx = m_vars.find(StdIndices::P);
            assert(Pidx >= 0);
            return DataView<Vec3>(m_views[Pidx]);
        }
        /// Get a const view of the vertex position data
        ConstDataView<Vec3> P() const
        {
            int Pidx = m_vars.find(StdIndices::P);
            assert(Pidx >= 0);
            return ConstDataView<Vec3>(m_views[Pidx]);
        }

        /// Transform primitive variables via the matrix trans.
        void transform(const Mat4& trans)
        {
            Mat3 vecTrans = vectorTransform(trans);
            Mat3 norTrans = normalTransform(trans);
            // Iterate over all primvars & transform as appropriate.
            for(int ivar = 0, nvars = m_vars.size(); ivar < nvars; ++ivar)
            {
                const PrimvarSpec& spec = m_vars[ivar];
                int nValues = m_storCount.storage(spec.iclass)
                              * spec.arraySize;
                switch(spec.type)
                {
                    case PrimvarSpec::Float:
                    case PrimvarSpec::Color:
                    case PrimvarSpec::String:
                        // No need to transform these.
                        break;
                    case PrimvarSpec::Point:
                        {
                            DataView<Vec3> p = m_views[ivar];
                            assert(p.isDense());
                            for(int i = 0; i < nValues; ++i)
                                p[i] *= trans;
                        }
                        break;
                    case PrimvarSpec::Vector:
                        {
                            DataView<Vec3> v = m_views[ivar];
                            assert(v.isDense());
                            for(int i = 0; i < nValues; ++i)
                                v[i] *= vecTrans;
                        }
                        break;
                    case PrimvarSpec::Normal:
                        {
                            DataView<Vec3> n = m_views[ivar];
                            assert(n.isDense());
                            for(int i = 0; i < nValues; ++i)
                                n[i] *= norTrans;
                        }
                        break;
                    case PrimvarSpec::Hpoint:
                    case PrimvarSpec::Matrix:
                        assert(0 && "Transform not yet implemented!");
                        break;
                }
            }
        }
};

typedef boost::intrusive_ptr<PrimvarStorage> PrimvarStoragePtr;


//------------------------------------------------------------------------------
class PrimvarStorageBuilder
{
    private:
        struct PvarInitSpec : public PrimvarSpec
        {
            const float* data; ///< data storage
            int srcLength;     ///< length of data storage
            PvarInitSpec(const PrimvarSpec& spec, const float* data,
                         int srcLength)
                : PrimvarSpec(spec),
                data(data),
                srcLength(srcLength)
            { }
        };
        typedef std::vector<PvarInitSpec> InitSpecVec;
        InitSpecVec m_vars;

    public:
        PrimvarStorageBuilder()
            : m_vars()
        { }

        void add(const PrimvarSpec& var, const float* data, int srcLength)
        {
            m_vars.push_back(PvarInitSpec(var, data, srcLength));
        }

        PrimvarStoragePtr build(const IclassStorage& storCount)
        {
            for(int i = 0, nvars = m_vars.size(); i < nvars; ++i)
            {
                int length = m_vars[i].storageSize(storCount);
                int srcLength = m_vars[i].srcLength;
                if(srcLength < length)
                {
                    throw std::runtime_error("Not enough floats for "
                                            "primitive variable!");
                }
                else if(srcLength > length)
                    std::cerr << "Warning: excess floats in "
                                "primitive variable array\n";
            }
            // TODO: Have some way to flag primvars that won't be used for
            // deletion.
            //
            // TODO: Figure out a way to add required stdvars if they're not
            // present.  Primvars to be added by the renderer:
            //
            //   Cs, Os,
            //   u, v,
            //   s, t
            //
            std::sort(m_vars.begin(), m_vars.end());
            return PrimvarStoragePtr(
                new PrimvarStorage(m_vars.begin(), m_vars.end(), storCount));
        }
};

//==============================================================================
// Implementation details

inline int PrimvarSpec::storageSize(const IclassStorage& storCount) const
{
    return storCount.storage(iclass)*scalarSize();
}

#endif // PRIMVAR_H_INCLUDED
