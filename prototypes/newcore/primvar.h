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

#ifndef AQSIS_PRIMVAR_H_INCLUDED
#define AQSIS_PRIMVAR_H_INCLUDED

#include <algorithm>
#include <stdexcept>
#include <vector>

#include <boost/scoped_array.hpp>

#include "arrayview.h"
#include "refcount.h"
#include "util.h"
#include "varspec.h"

namespace Aqsis {

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

    PrimvarSpec() : VarSpec(), iclass(Uniform) {}

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
                // Record a view of the data
                float* dest = m_storage.get() + offset;
                const int elSize = var->data.elSize();
                m_views[i] = FvecView(dest, elSize);
                // Copy the data for the i'th variable over
                const int size = var->storageSize(storCount);
                if(var->dataIndexBegin)
                {
                    const int* dataIndexEnd = var->dataIndexBegin +
                                              storCount.storage(var->iclass);
                    for(const int* idx = var->dataIndexBegin;
                        idx < dataIndexEnd; ++idx)
                    {
                        const float* src = var->data[*idx];
                        for(int k = 0; k < elSize; ++k)
                            dest[k] = src[k];
                        dest += elSize;
                    }
                }
                else
                    std::memcpy(dest, var->data.storage(), size*sizeof(float));
                offset += size;
            }
        }

    public:
        /// Get the set of contained variables
        const PrimvarSet& varSet() const { return m_vars; }

        /// Get storage amount used for each interpolation class
        const IclassStorage& iclassStorage() { return m_storCount; }

        /// Get allocated storage for the ith variable
        FvecView get(int i) { return m_views[i]; }
        ConstFvecView get(int i) const { return m_views[i]; }

        /// Get a view of the vertex position data
        DataView<V3f> P()
        {
            int Pidx = m_vars.find(StdIndices::P);
            assert(Pidx >= 0);
            return DataView<V3f>(m_views[Pidx]);
        }
        /// Get a const view of the vertex position data
        ConstDataView<V3f> P() const
        {
            int Pidx = m_vars.find(StdIndices::P);
            assert(Pidx >= 0);
            return ConstDataView<V3f>(m_views[Pidx]);
        }

        /// Transform primitive variables via the matrix trans.
        void transform(const M44f& trans)
        {
            M33f vecTrans = vectorTransform(trans);
            M33f norTrans = normalTransform(trans);
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
                            DataView<V3f> p = m_views[ivar];
                            assert(p.isDense());
                            for(int i = 0; i < nValues; ++i)
                                p[i] *= trans;
                        }
                        break;
                    case PrimvarSpec::Vector:
                        {
                            DataView<V3f> v = m_views[ivar];
                            assert(v.isDense());
                            for(int i = 0; i < nValues; ++i)
                                v[i] *= vecTrans;
                        }
                        break;
                    case PrimvarSpec::Normal:
                        {
                            DataView<V3f> n = m_views[ivar];
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
            ConstFvecView data; ///< data storage
            int dataLength;     ///< length of data storage (-1 == don't check)
            const int* dataIndexBegin;  /// Indices into data (null if indices are consecutive)
            PvarInitSpec(const PrimvarSpec& spec, ConstFvecView data,
                         int dataLength, const int* dataIndexBegin)
                : PrimvarSpec(spec),
                data(data),
                dataLength(dataLength),
                dataIndexBegin(dataIndexBegin)
            { }
        };
        std::vector<PvarInitSpec> m_vars;

    public:
        PrimvarStorageBuilder()
            : m_vars()
        { }

        /// Add a variable to be used during build()
        ///
        /// \param var - variable type and name
        /// \param data - flat array of values for var.  *Must* remain valid
        ///               until after build() has been called!
        /// \param dataLength - length of data array
        void add(const PrimvarSpec& var, const float* data, int dataLength)
        {
            int len = var.scalarSize();
            m_vars.push_back(PvarInitSpec(var, ConstFvecView(data, len),
                                          dataLength, 0));
        }

        /// Add a variable to be used during build()
        ///
        /// The i'th element of var is copied from the dest array by indexing
        /// dest as follows: dest[ dataIndexBegin[i] ]
        ///
        /// \param var - variable type and name
        /// \param data - array view of values for var.  *Must* remain valid
        ///               until after build() has been called!
        /// \param dataIndexBegin - array of indices into data
        void add(const PrimvarSpec& var, ConstFvecView data,
                 const int* dataIndexBegin)
        {
            m_vars.push_back(PvarInitSpec(var, data, -1, dataIndexBegin));
        }

        /// Build a PrimvarStorage object from add()'ed variables.
        ///
        /// After building, the builder object is reset to remove all added
        /// variables, ready for reuse if desired.  For allocation efficiency
        /// it's good to reuse the same builder object if you're going to be
        /// creating multiple PrimvarStorage objects at the same time.
        ///
        PrimvarStoragePtr build(const IclassStorage& storCount)
        {
            for(int i = 0, nvars = m_vars.size(); i < nvars; ++i)
            {
                int dataLength = m_vars[i].dataLength;
                if(dataLength >= 0)
                {
                    int desiredLength = m_vars[i].storageSize(storCount);
                    if(dataLength < desiredLength)
                    {
                        throw std::runtime_error("Not enough floats for "
                                                "primitive variable!");
                    }
                    else if(dataLength > desiredLength)
                        std::cerr << "Warning: excess floats in "
                                    "primitive variable array\n";
                }
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
            PrimvarStoragePtr storage(
                new PrimvarStorage(m_vars.begin(), m_vars.end(), storCount));
            m_vars.clear();
            return storage;
        }
};

//==============================================================================
// Implementation details

inline int PrimvarSpec::storageSize(const IclassStorage& storCount) const
{
    return storCount.storage(iclass)*scalarSize();
}


} // namespace Aqsis
#endif // AQSIS_PRIMVAR_H_INCLUDED
