#include <vector>
#include <stdexcept>

#include <boost/shared_ptr.hpp>

#include "util.h"
#include "fixedstrings.h"

struct StorageCount;

struct VarSpec
{
    enum Iclass
    {
        Constant,
        Uniform,
        Varying,
        Vertex,
        FaceVarying,
        FaceVertex
    };

    enum Type
    {
        Float,
        Point,
        Hpoint,
        Vector,
        Normal,
        Color,
        Matrix
    };

    static int scalarSize(Type type)
    {
        switch(type)
        {
            case Float:  return 1;
            case Point:  return 3;
            case Hpoint: return 4;
            case Vector: return 3;
            case Normal: return 3;
            case Color:  return 3;
            case Matrix: return 16;
        }
    }

    // Description of the variable
    Iclass   iclass;
    Type     type;
    int      count;
    ustring  name;

    VarSpec(Iclass iclass, Type type, int count, ustring name)
        : iclass(iclass), type(type), count(count), name(name) {}

    int scalarSize() const
    {
        return scalarSize(type)*count;
    }

    int storageSize(const StorageCount& storCount) const;
};


struct StorageCount
{
    int uniform;
    int varying;
    int vertex;
    int facevarying;
    int facevertex;

    StorageCount(int uniform, int varying, int vertex, int facevarying, int facevertex)
        : uniform(uniform),
        varying(varying),
        vertex(vertex),
        facevarying(facevarying),
        facevertex(facevertex)
    { }

    int operator[](VarSpec::Iclass iclass) const
    {
        switch(iclass)
        {
            case VarSpec::Constant:    return 1;
            case VarSpec::Uniform:     return uniform;
            case VarSpec::Varying:     return varying;
            case VarSpec::Vertex:      return vertex;
            case VarSpec::FaceVarying: return facevarying;
            case VarSpec::FaceVertex:  return facevertex;
        }
    }
};


int VarSpec::storageSize(const StorageCount& storCount) const
{
    return storCount[iclass]*scalarSize();
}


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
};


class VarList
{
    private:
        std::vector<VarSpec> m_varSpecs;
        int m_P_idx;

    public:
        VarList()
            : m_varSpecs(),
            m_P_idx(-1)
        { }

        /// Add a variable, and return the associated variable offset
        int add(const VarSpec& var)
        {
            int index = m_varSpecs.size();
            if(var.name == Str::P)
            {
                if(var.type != VarSpec::Point || var.iclass != VarSpec::Vertex || var.count != 1)
                    throw std::runtime_error("Wrong type for variable \"P\"");
                m_P_idx = index;
            }
            m_varSpecs.push_back(var);
            return index;
        }

        int size() const { return m_varSpecs.size(); }

        const VarSpec& operator[](int i) const
        {
            assert(i >= 0 && i < m_varSpecs.size());
            return m_varSpecs[i];
        }

        int P() const { return m_P_idx; }
};


class PrimvarStorage
{
    private:
        StorageCount m_storCount;
        std::vector<float> m_storage;
        std::vector<int> m_offsets;
        boost::shared_ptr<VarList> m_vars;

    public:
        PrimvarStorage(const StorageCount& storCount)
            : m_storCount(storCount),
            m_storage(),
            m_offsets(),
            m_vars(new VarList())
        { }

        PrimvarStorage(const StorageCount& storCount,
                    const boost::shared_ptr<VarList>& vars)
            : m_storCount(storCount),
            m_storage(),
            m_offsets(),
            m_vars(vars)
        {
            // Allocate space for the variables
            int nvars = m_vars->size();
            m_offsets.resize(nvars, 0);
            int storageTot = 0;
            for(int i = 0; i < nvars; ++i)
                storageTot += (*m_vars)[i].storageSize(m_storCount);
            m_storage.resize(storageTot, 0);
        }

        int add(const VarSpec& var, float* data, int srcLength)
        {
            int index = m_vars->add(var);
            int length = var.storageSize(m_storCount);
            if(srcLength != length)
                throw std::runtime_error("Wrong number of floats for primitive variable!");
            m_offsets.push_back(m_storage.size());
            m_storage.insert(m_storage.end(), data, data+length);
            return index;
        }

        float* get(int idx)
        {
            assert(idx >= 0 && idx < m_offsets.size());
            return &m_storage[m_offsets[idx]];
        }

        // Get a view of the vertex position data
        DataView<Vec3> P()
        {
            assert(m_vars->P() >= 0);
            return DataView<Vec3>(&m_storage[m_offsets[m_vars->P()]]);
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
PrimvarList* createPrimvarList(const StorageCount& storageSize, int count,
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

