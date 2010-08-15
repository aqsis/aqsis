// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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

/// \file
///
/// \brief Utilities to make working with the Ri types nicer
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///

#ifndef AQSIS_RICXXUTIL_H_INCLUDED
#define AQSIS_RICXXUTIL_H_INCLUDED

#include "ricxx.h"

#include <cassert>
#include <climits>
#include <string.h> // for strcmp

#include <aqsis/riutil/primvartoken.h>
#include <aqsis/riutil/interpclasscounts.h>

namespace Aqsis {

/// A class for convenient building of Ri::ParamList instances
///
/// Example:
/// \code
///     float dist = 0;
///     const char* name = "a string";
///     std::vector<int> frames(10);
///
///     // Construct the paramer list (this can also be done inline)
///     ParamListBuilder pList();
///     pList("float dist", &dist)
///          (Ri::TypeSpec(Ri::TypeSpec::String, "name", &name)
///          ("int frames", frames);
///
///     // Can assign to a ParamList, or pass to a function taking one such
///     Ri::ParamList realList = pList;
/// \endcode
///
class ParamListBuilder
{
    public:
        /// Add a single value, v to the parameter list
        template<typename T>
        ParamListBuilder& operator()(const char* token, T* v);
        /// Add a single value, v to the parameter list
        template<typename T>
        ParamListBuilder& operator()(const Ri::TypeSpec& spec,
                                     const char* name, T* v);

        /// Add an array, v to the parameter list
        template<typename T>
        ParamListBuilder& operator()(const char* token,
                                     const std::vector<T>& v);
        /// Add an array, v to the parameter list
        template<typename T>
        ParamListBuilder& operator()(const Ri::TypeSpec& spec,
                                     const char* name,
                                     const std::vector<T>& v);

        /// Implicity convert to Ri::ParamList
        operator Ri::ParamList();

    private:
        std::vector<Ri::Param> m_paramStorage;
};


//--------------------------------------------------
/// Get the array size from counts corresponding to iclass.
int iclassCount(const SqInterpClassCounts& counts,
                       Ri::TypeSpec::IClass iclass);

/// Add elements of a
int sum(const Ri::IntArray& a);

/// Add elements of a, with given starting index and stride
int sum(const Ri::IntArray& a, int start, int step);

/// Get the maximum element in array a
int max(const Ri::IntArray& a);

/// Get the length of the array a
template<typename T>
int size(const Ri::Array<T>& a);


/// Get the interpolation class counts for RiPatchMesh.
SqInterpClassCounts patchMeshIClassCounts(const char* type, int nu,
                                          const char* uwrap,
                                          int nv, const char* vwrap,
                                          int basisUstep, int basisVstep);

/// Get the interpolation class counts for RiCurves.
SqInterpClassCounts curvesIClassCounts(const char* type,
                                       const Ri::IntArray& nvertices,
                                       const char* wrap, int basisVstep);

//==============================================================================
// implementation details

template<typename T>
inline ParamListBuilder& ParamListBuilder::operator()(const char* token, T* v)
{
    const char* nameBegin = 0;
    const char* nameEnd = 0;
    Ri::TypeSpec spec = parseDeclaration(token, &nameBegin, &nameEnd);
    assert(*nameEnd == '\0');
    m_paramStorage.push_back(
        Ri::Param(spec, nameBegin, Ri::Array<T>(v, 1)));
    return *this;
}

template<typename T>
inline ParamListBuilder& ParamListBuilder::operator()(const char* token,
                                               const std::vector<T>& v)
{
    const char* nameBegin = 0;
    const char* nameEnd = 0;
    Ri::TypeSpec spec = parseDeclaration(token, &nameBegin, &nameEnd);
    assert(*nameEnd == '\0');
    m_paramStorage.push_back(
        Ri::Param(spec, nameBegin,
                    Ri::Array<T>(v.empty() ? 0 : &v[0], v.size())));
    return *this;
}

template<typename T>
inline ParamListBuilder& ParamListBuilder::operator()(const Ri::TypeSpec& spec,
                                               const char* name, T* v)
{
    m_paramStorage.push_back(
        Ri::Param(spec, name, Ri::Array<T>(v, 1)));
    return *this;
}

template<typename T>
inline ParamListBuilder& ParamListBuilder::operator()(const Ri::TypeSpec& spec,
                                               const char* name,
                                               const std::vector<T>& v)
{
    m_paramStorage.push_back(
        Ri::Param(spec, name,
                    Ri::Array<T>(v.empty() ? 0 : &v[0], v.size())));
    return *this;
}

inline ParamListBuilder::operator Ri::ParamList()
{
    if(m_paramStorage.empty())
        return Ri::ParamList();
    return Ri::ParamList(&m_paramStorage[0], m_paramStorage.size());
}

//--------------------------------------------------
inline int iclassCount(const SqInterpClassCounts& counts,
                       Ri::TypeSpec::IClass iclass)
{
    switch(iclass)
    {
        case Ri::TypeSpec::Constant:    return 1;
        case Ri::TypeSpec::Uniform:     return counts.uniform;
        case Ri::TypeSpec::Varying:     return counts.varying;
        case Ri::TypeSpec::Vertex:      return counts.vertex;
        case Ri::TypeSpec::FaceVarying: return counts.facevarying;
        case Ri::TypeSpec::FaceVertex:  return counts.facevertex;
        default:
            assert(0 && "Unknown interpolation class"); return 0;
    }
}

inline int sum(const Ri::IntArray& a)
{
    int s = 0;
    for(size_t i = 0; i < a.size(); ++i)
        s += a[i];
    return s;
}

inline int sum(const Ri::IntArray& a, int start, int step)
{
    int s = 0;
    for(size_t i = start; i < a.size(); i+=step)
        s += a[i];
    return s;
}

inline int max(const Ri::IntArray& a)
{
    int m = INT_MIN;
    for(size_t i = 0; i < a.size(); ++i)
        if(m < a[i])
            m = a[i];
    return m;
}

template<typename T>
inline int size(const Ri::Array<T>& a)
{
    return a.size();
}


//--------------------------------------------------
inline SqInterpClassCounts patchMeshIClassCounts(const char* type, int nu, const char* uwrap,
                                   int nv, const char* vwrap, int basisUstep, int basisVstep)
{
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    bool uperiodic = strcmp(uwrap, "periodic") == 0;
    bool vperiodic = strcmp(vwrap, "periodic") == 0;
    if(strcmp(type, "bilinear")==0)
    {
        iclassCounts.uniform = (uperiodic ? nu : nu-1) *
                               (vperiodic ? nv : nv-1);
        iclassCounts.varying = nu*nv;
    }
    else
    {
        int nupatches = uperiodic ? nu/basisUstep : (nu-4)/basisUstep + 1;
        int nvpatches = vperiodic ? nv/basisVstep : (nv-4)/basisVstep + 1;
        iclassCounts.uniform = nupatches * nvpatches;
        iclassCounts.varying = ((uperiodic ? 0 : 1) + nupatches) *
                               ((vperiodic ? 0 : 1) + nvpatches);
    }
    iclassCounts.vertex = nu*nv;
    // TODO: are facevertex/facevarying valid for a patch mesh?
    iclassCounts.facevarying = 1; //iclassCounts.uniform*4; //??
    iclassCounts.facevertex = 1;
    return iclassCounts;
}

inline SqInterpClassCounts curvesIClassCounts(const char* type,
                                       const Ri::IntArray& nvertices,
                                       const char* wrap, int basisVstep)
{
    SqInterpClassCounts iclassCounts(1,1,1,1,1);
    bool periodic = strcmp(wrap, "periodic") == 0;
    int basisStep = basisVstep;
    iclassCounts.uniform = size(nvertices);
    iclassCounts.vertex = sum(nvertices);
    if(strcmp(type, "cubic") == 0)
    {
        if(periodic)
        {
            int segmentCount = 0;
            for(size_t i = 0; i < nvertices.size(); ++i)
                segmentCount += nvertices[i]/basisStep;
            iclassCounts.varying = segmentCount;
        }
        else
        {
            int segmentCount = 0;
            for(size_t i = 0; i < nvertices.size(); ++i)
                segmentCount += (nvertices[i]-4)/basisStep + 1;
            iclassCounts.varying = segmentCount + size(nvertices);
        }
    }
    else
    {
        // linear curves
        iclassCounts.varying = iclassCounts.vertex;
    }
    // TODO: are facevertex/facevarying valid for curves?
    iclassCounts.facevarying = 1;
    iclassCounts.facevertex = 1;
    return iclassCounts;
}



}

#endif // AQSIS_RICXXUTIL_H_INCLUDED
// vi: set et:
