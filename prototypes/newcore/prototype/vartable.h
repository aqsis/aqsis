#ifndef VARSPEC_H_INCLUDED
#define VARSPEC_H_INCLUDED

#include <vector>
#include <iostream>

#include <boost/unordered_map.hpp>

#include "ustring.h"
//#include "vset.h"

struct VarSpec
{
    /// Variable type.
    enum Type
    {
        Float,
        Point,
        Hpoint,
        Vector,
        Normal,
        Color,
        Matrix,
        String
    };

    Type     type;       ///< Variable type
    int      arraySize;  ///< Array size
    ustring  name;       ///< Variable name

    VarSpec() : type(Float), arraySize(1), name() { }

    VarSpec(Type type, int arraySize, ustring name)
        : type(type), arraySize(arraySize), name(name) { }

    /// Get number of scalar values required for the given type
    static int sizeForType(Type type)
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
            case String: return 1;
        }
    }

    /// Get number of scalar values required for the variable
    ///
    /// Each element of the variable requires a number of scalar values to
    /// represent it; these are floats, except in the case of strings.
    int scalarSize() const
    {
        return sizeForType(type)*arraySize;
    }

    friend bool operator<(const VarSpec& l, const VarSpec& r)
    {
        return l.name < r.name
            && l.type < r.type
            && l.arraySize < r.arraySize;
    }

    friend bool operator==(const VarSpec& l, const VarSpec& r)
    {
        return l.name == r.name
            && l.type == r.type
            && l.arraySize == r.arraySize;
    }
};

/// Hash function for use with boost::hash
std::size_t hash_value(const VarSpec& var)
{
    std::size_t h = 0;
    boost::hash_combine(h, var.type);
    boost::hash_combine(h, var.arraySize);
    boost::hash_combine(h, var.name.hash());
    return h;
}

std::ostream& operator<<(std::ostream& out, const VarSpec& var)
{
    switch(var.type)
    {
        case VarSpec::Float:  out << "float";  break;
        case VarSpec::Point:  out << "point";  break;
        case VarSpec::Hpoint: out << "hpoint"; break;
        case VarSpec::Vector: out << "vector"; break;
        case VarSpec::Normal: out << "normal"; break;
        case VarSpec::Color:  out << "color";  break;
        case VarSpec::Matrix: out << "matrix"; break;
        case VarSpec::String: out << "string"; break;
    }
    if(var.arraySize > 1)
        out << "[" << var.arraySize << "]";
    out << ' ' << var.name;
    return out;
}

//typedef VSet<VarSpec> VarSet;


class VarId
{
    private:
        int m_id;
        friend class VarTable;

    public:
        VarId(int id = -1) : m_id(id) {}

        /// Test whether the id is a valid one.  ( eg, if(id) { /*...*/ } )
        operator const void*() const
        { return m_id >= 0 ? reinterpret_cast<const void*>(this) : 0; }

        friend bool operator<(VarId l, VarId r) { return l.m_id < r.m_id; }
        friend bool operator==(VarId l, VarId r) { return l.m_id == r.m_id; }
};


class VarTable
{
    private:
        typedef boost::unordered_map<VarSpec, VarId> VarToIdMap;
        VarToIdMap m_varToId;
        std::vector<VarSpec> m_idToVar;
    public:
        VarTable(size_t reserveSize = 256)
            : m_varToId(reserveSize),
            m_idToVar()
        {
            m_idToVar.reserve(reserveSize);
        }

        /// Look up a variable id & insert it if not already present.
        VarId getId(const VarSpec& var)
        {
            VarId& id = m_varToId[var];
            if(!id)
            {
                // If not valid, the id was default initialized by the map;
                // need to initailize it properly here & store the associated
                // var id.
                id.m_id = m_idToVar.size();
                m_idToVar.push_back(var);
            }
            return id;
        }

        /// Look up a variable id.  If not present, return an invalid id.
        VarId getIdNoInsert(const VarSpec& var) const
        {
            VarToIdMap::const_iterator i = m_varToId.find(var);
            if(i == m_varToId.end())
                return VarId();
            return i->second;
        }

        /// Find the var spec corresponding to a variable id
        const VarSpec& getVar(const VarId& id) const
        {
            assert(id);
            return m_idToVar[id.m_id];
        }
};


#endif // VARSPEC_H_INCLUDED
