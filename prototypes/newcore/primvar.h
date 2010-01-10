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

#include <vector>
#include <stdexcept>

#include <boost/shared_ptr.hpp>

#include "util.h"
#include "fixedstrings.h"
#include "arrayview.h"
#include "varspec.h"

struct IclassStorage;

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
///   - Uniform: constant over geometry-defined elements of the primitive
///   - Varying: linearly interpolated over elements
///   - Vertex: interpolated using the same rules as the position primvar
///   - FaceVarying: linearly interpolated over elements (faces), but is
///                  specified per-face so may be discontinuous at face
///                  boundaries
///   - FaceVertex: interpolated the same as Vertex when continuous across
///                 faces.  When discontinuous use some geometry-specific rule
///                 to behave more like FaceVarying.
///
struct PrimvarSpec : public VarSpec
{
    /// Variable interpolation class.
    ///
    /// This determines how the variable is interpolated across the surface of
    /// the geometry.
    enum Iclass
    {
        Constant,
        Uniform,
        Varying,
        Vertex,
        FaceVarying,
        FaceVertex
    };

    Iclass   iclass;     ///< Interpolation class

    PrimvarSpec(Iclass iclass, Type type, int arraySize, ustring name)
        : VarSpec(type, arraySize, name), iclass(iclass) {}

    /// Return the number of scalar values required for the variable
    int storageSize(const IclassStorage& storCount) const;
};


/// Standard primitive variable names
namespace Primvar
{
    extern const PrimvarSpec P;
    extern const PrimvarSpec Cs;
    extern const PrimvarSpec st;
}


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

inline int PrimvarSpec::storageSize(const IclassStorage& storCount) const
{
    return storCount.storage(iclass)*scalarSize();
}

struct StdVarIndices
{
    int P;
    int Cs;
    int Ci;
    int Os;
    int Oi;
    int st;
    int I;
    int N;

    StdVarIndices()
        : P(-1), Cs(-1), Ci(-1), Os(-1), Oi(-1), st(-1), I(-1), N(-1) {}

    void add(int index, const VarSpec& var)
    {
        if(var == Stdvar::P)
            P = index;
        else if(var == Stdvar::Cs)
            Cs = index;
        else if(var == Stdvar::Ci)
            Ci = index;
        else if(var == Stdvar::Os)
            Os = index;
        else if(var == Stdvar::Oi)
            Oi = index;
        else if(var == Stdvar::st)
            st = index;
        else if(var == Stdvar::I)
            I = index;
        else if(var == Stdvar::N)
            N = index;
    }
};


/// A list of primitive variables
class PrimvarList
{
    private:
        std::vector<PrimvarSpec> m_varSpecs;
        StdVarIndices m_stdIndices;

    public:
        typedef std::vector<PrimvarSpec>::const_iterator const_iterator;

        PrimvarList()
            : m_varSpecs(),
            m_stdIndices()
        { }

        /// Add a variable, and return the associated variable offset
        int add(const PrimvarSpec& var)
        {
            int index = m_varSpecs.size();
            m_stdIndices.add(index, var);
            m_varSpecs.push_back(var);
            return index;
        }

        int size() const { return m_varSpecs.size(); }

        const_iterator begin() const { return m_varSpecs.begin(); }
        const_iterator end() const { return m_varSpecs.end(); }

        const PrimvarSpec& operator[](int i) const
        {
            assert(i >= 0 && i < (int)m_varSpecs.size());
            return m_varSpecs[i];
        }

        const StdVarIndices& stdIndices() const { return m_stdIndices; }
};


class PrimvarStorage
{
    private:
        IclassStorage m_storCount;
        std::vector<float> m_storage;
        struct VarInfo
        {
            int offset;
            int stride;
            int elSize;
            VarInfo(int offset, int stride, int elSize)
                : offset(offset), stride(stride), elSize(elSize) {}
        };
        std::vector<VarInfo> m_varInfo;
        boost::shared_ptr<PrimvarList> m_vars;

    public:
        PrimvarStorage(const IclassStorage& storCount)
            : m_storCount(storCount),
            m_storage(),
            m_varInfo(),
            m_vars(new PrimvarList())
        { }

        int add(const PrimvarSpec& var, float* data, int srcLength)
        {
            int index = m_vars->add(var);
            int length = var.storageSize(m_storCount);
            if(srcLength < length)
                throw std::runtime_error("Not enough floats for "
                                         "primitive variable!");
            else if(srcLength > length)
                std::cerr << "Warning: excess floats in "
                             "primitive variable array\n";
            int elSize = var.scalarSize();
            m_varInfo.push_back(VarInfo(m_storage.size(), elSize, elSize));
            m_storage.insert(m_storage.end(), data, data+length);
            return index;
        }

        FvecView get(int i)
        {
            assert(i >= 0 && i < (int)m_varInfo.size());
            return FvecView(&m_storage[m_varInfo[i].offset],
                              m_varInfo[i].stride, m_varInfo[i].elSize);
        }

        const PrimvarList& varList() const { return *m_vars; }

        // Get a view of the vertex position data
        DataView<Vec3> P()
        {
            int Pidx = m_vars->stdIndices().P;
            assert(Pidx >= 0);
            return DataView<Vec3>(&m_storage[m_varInfo[Pidx].offset]);
        }

        // Get a view of the vertex position data
        ConstDataView<Vec3> P() const
        {
            int Pidx = m_vars->stdIndices().P;
            assert(Pidx >= 0);
            return ConstDataView<Vec3>(&m_storage[m_varInfo[Pidx].offset]);
        }

        void transform(const Mat4& m)
        {
            // Iterate over all primvars & transform as appropriate.
            for(int ivar = 0, nvars = m_vars->size(); ivar < nvars; ++ivar)
            {
                const PrimvarSpec& spec = (*m_vars)[ivar];
                switch(spec.type)
                {
                    case PrimvarSpec::Float:
                    case PrimvarSpec::Color:
                    case PrimvarSpec::String:
                        // No need to transform these.
                        break;
                    case PrimvarSpec::Point:
                        {
                            int aSize = spec.arraySize;
                            int nElems = m_storCount.storage(spec.iclass);
                            FvecView v = get(ivar);
                            for(int j = 0; j < nElems; ++j)
                            {
                                DataView<Vec3> p(v[j]);
                                for(int i = 0; i < aSize; ++i)
                                    p[i] *= m;
                            }
                        }
                        break;
                    case PrimvarSpec::Vector:
                    case PrimvarSpec::Normal:
                    case PrimvarSpec::Hpoint:
                    case PrimvarSpec::Matrix:
                        assert(0 && "Transform not yet implemented!");
                        break;
                }
            }
        }
};



// shader required primvar list
// AOV required primvar list

// For the purposes of shaders and AOVs, a "primvar" needs to be a combination
// of name, type and array length.  (interpolation class is irrelevant)
//
// At start of world, compute required AOV primvars:
//   RAOV = AOV required list
//
// For each shader:
//   RShader = shader requested primvar list
//
// For each primitive:
//   R = requested primvars = RAOV union RShader
//   RP = required primitive primvars = R intersect (avaliable primvars)
//   copy elements of RP into a PrimvarList container
//
// When dicing:
//   dice all elements of RP
//
// When shading:
//   Use RP -> shader mapping to init shader arguments
//
// On shader exit:
//   Use shader -> AOV


#if 0
PrimvarList* createPrimvarList(const IclassStorage& storageSize, int count,
                               RtToken* tokens, RtPointer* values)
{
    // An array to hold the tokens identifying the primvars
    std::vector<PrimvarToken> parsedTokens;
    parsedTokens.reserve(count);
    std::vector<int> validTokens;
    validTokens.reserve(count);
    int numFloatTokens = 0;
    int numStringTokens = 0;
    int floatSize = 0;
    int stringSize = 0;
    for(int i = 0; i < count; ++i)
    {
        PrimvarToken tok(tokens[i]);
        if(/* primvar is used in a shader or an AOV and primvar is not an int or bool
              and any std primvar is as expected (Eg, "P" has type "vertex point[1]")
            */)
        {
            parsedTokens.push_back(tok);
            validTokens.push_back(i);
            int size = tok.storageCount() * storageSize[tok.Class()];
            if(tok.type == type_string)
                stringSize += size;
            else
                floatSize += size;
        }
    }
    // Allocate primvar storage.
    std::vector<float> floatData(floatSize);
    std::vector<std::string> stringData(stringSize);
    // Copy the values into the storage space.
    int floatOffset = 0;
    int stringOffset = 0;
    for(int i = 0; i < parsedTokens.size(); ++i)
    {
        const PrimvarToken& tok = parsedTokens[i];
        int size = tok.storageCount() * storageSize[tok.Class()];
        switch(tok.type)
        {
            case type_string:
                // Copy strings
                for(int j = 0; j < size; ++j)
                    stringData[stringOffset + j] = reinterpret_cast<const char*>(values[validTokens[i]])[j];
                stringOffset += size;
                break;
            default:
                // Possible token types here use float storage.
                memcpy(&floatData[floatOffset], values[validTokens[i]], size*sizeof(float));
                floatOffset += size;
                break;
        }
    }
    return new PrimvarList(parsedTokens, floatData, stringData);
}
#endif

#endif // PRIMVAR_H_INCLUDED
