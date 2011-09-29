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

/// \file Utilities for in-memory caching of Ri::Renderer calls.
///
/// \author Chris Foster

#ifndef AQSIS_API_CACHE_H_INCLUDED
#define AQSIS_API_CACHE_H_INCLUDED

#include <aqsis/riutil/ricxx.h>

#include <cstring>
#include <boost/ptr_container/ptr_vector.hpp>

#include "multistringbuffer.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/// A single cached RI call.
///
/// For every RI call, there's a derived class of this which caches the
/// arguments, and is able to call the appropriate interface function again at
/// a later time.  These live in the RiCache namespace; for example,
/// RiCache::Sphere.
class CachedRequest
{
    public:
        /// Re-call the interface function on the given context.
        virtual void reCall(Ri::Renderer& context) const = 0;

        virtual ~CachedRequest() {}

    protected:
        // These types make the codegen a little more convenient.
        typedef Ri::IntArray IntArray;
        typedef Ri::FloatArray FloatArray;
        typedef Ri::StringArray StringArray;
        typedef Ri::TokenArray TokenArray;
};


//------------------------------------------------------------------------------
/// A stream of cached RI calls held in memory.
///
/// The stream can be replayed into the provided context, streaming the
/// commands from memory.  This should be a bit faster than saving out as RIB,
/// at the cost of some memory.
class CachedRiStream
{
    private:
        boost::ptr_vector<CachedRequest> m_requests;
        std::string m_name;

    public:
        CachedRiStream(RtConstToken name)
            : m_name(name)
        { }
        void push_back(CachedRequest* req)
        {
            m_requests.push_back(req);
        }
        void replay(Ri::Renderer& context) const
        {
            for(int i = 0, iend = m_requests.size(); i < iend; ++i)
                m_requests[i].reCall(context);
        }
        const std::string& name() const { return m_name; }
};



//==============================================================================
// Implementation details follow.

//------------------------------------------------------------------------------
/// Namespace for implementations of CachedRequest
///
/// Also holds helpers for caching interface types.
namespace RiCache {

// Classes defining cache space for RI arguments.
//
// There's a class here for each argument type which is not a value-type.  (That
// includes all arrays and strings, as well as tuples like colors.)

class CachedString
{
    private:
        std::string m_str;
    public:
        CachedString(RtConstString str) : m_str(str) {}

        operator RtConstString() const { return m_str.c_str(); }
};

template<typename T>
class CachedArray
{
    private:
        std::vector<T> m_vec;
    public:
        CachedArray(const Ri::Array<T>& a) : m_vec(a.begin(), a.end()) {}

        operator Ri::Array<T>() const
        {
            if(m_vec.empty())
                return Ri::Array<T>();
            return Ri::Array<T>(&m_vec[0], m_vec.size());
        }
};

typedef CachedArray<RtInt> CachedIntArray;
typedef CachedArray<RtFloat> CachedFloatArray;

class CachedStringArray
{
    private:
        MultiStringBuffer m_buf;
    public:
        CachedStringArray(const Ri::StringArray& a)
        {
            for(size_t i = 0; i < a.size(); ++i)
                m_buf.push_back(a[i]);
        }
        operator Ri::StringArray() const
        {
            const std::vector<const char*>& strings = m_buf.toCstringVec();
            if(strings.empty())
                return Ri::StringArray();
            return Ri::StringArray(&strings[0], strings.size());
        }
};

template<int N>
class CachedFloatTuple
{
    private:
        typedef RtFloat FloatTuple[N];
        typedef const RtFloat ConstFloatTuple[N];
        FloatTuple m_data;
    public:
        CachedFloatTuple(ConstFloatTuple p)
        {
            for(int i = 0; i < N; ++i)
                m_data[i] = p[i];
        }

        operator ConstFloatTuple&() const { return m_data; }
};

class CachedMatrix
{
    private:
        RtFloat m_data[4][4];
    public:
        CachedMatrix(RtConstMatrix m)
        {
            for(int j = 0; j < 4; ++j)
            for(int i = 0; i < 4; ++i)
                m_data[j][i] = m[j][i];
        }

        operator RtConstMatrix&() const { return m_data; }
};

// Cache for parameter lists.
class CachedParamList
{
    private:
        boost::scoped_array<RtInt> m_ints;
        boost::scoped_array<RtFloat> m_floats;
        boost::scoped_array<RtPointer> m_pointers;
        boost::scoped_array<char> m_chars;
        boost::scoped_array<RtConstString> m_strings;
        std::vector<Ri::Param> m_pList;

    public:
        CachedParamList(const Ri::ParamList& pList)
        {
            if(pList.size() == 0)
                return;
            int intCount = 0;
            int floatCount = 0;
            int ptrCount = 0;
            int charCount = 0;
            int stringCount = 0;
            // First scan the list, figure out how much storage we need
            for(size_t i = 0; i < pList.size(); ++i)
            {
                charCount += std::strlen(pList[i].name()) + 1;
                switch(pList[i].spec().storageType())
                {
                    case Ri::TypeSpec::Integer:
                        intCount += pList[i].size();
                        break;
                    case Ri::TypeSpec::Float:
                        floatCount += pList[i].size();
                        break;
                    case Ri::TypeSpec::Pointer:
                        ptrCount += pList[i].size();
                        break;
                    case Ri::TypeSpec::String:
                        {
                            Ri::StringArray strings = pList[i].stringData();
                            for(size_t j = 0; j < strings.size(); ++j)
                                charCount += std::strlen(strings[j]) + 1;
                            stringCount += strings.size();
                        }
                        break;
                    default:
                        assert(0 && "unknown type");
                }
            }
            // allocate storage
            if(intCount)    m_ints.reset(new RtInt[intCount]);
            if(floatCount)  m_floats.reset(new RtFloat[floatCount]);
            if(ptrCount)  m_pointers.reset(new RtPointer[ptrCount]);
            if(stringCount) m_strings.reset(new RtConstString[stringCount]);
            if(charCount)   m_chars.reset(new char[charCount]);
            // Finally, copy over the data
            intCount = 0;
            floatCount = 0;
            ptrCount = 0;
            charCount = 0;
            stringCount = 0;
            m_pList.reserve(pList.size());
            for(size_t i = 0; i < pList.size(); ++i)
            {
                // Copy parameter name
                const char* nameIn = pList[i].name();
                int nameLen = std::strlen(nameIn) + 1;
                char* paramName = m_chars.get() + charCount;
                std::memcpy(paramName, nameIn, nameLen);
                charCount += nameLen;
                size_t size = pList[i].size();
                // Copy parameter data
                void* data = 0;
                switch(pList[i].spec().storageType())
                {
                    case Ri::TypeSpec::Integer:
                        data = m_ints.get() + intCount;
                        std::memcpy(data, pList[i].data(), size*sizeof(RtInt));
                        intCount += size;
                        break;
                    case Ri::TypeSpec::Float:
                        data = m_floats.get() + floatCount;
                        std::memcpy(data, pList[i].data(), size*sizeof(RtFloat));
                        floatCount += size;
                        break;
                    case Ri::TypeSpec::Pointer:
                        data = m_pointers.get() + ptrCount;
                        std::memcpy(data, pList[i].data(), size*sizeof(RtPointer));
                        ptrCount += pList[i].size();
                        break;
                    case Ri::TypeSpec::String:
                        {
                            Ri::StringArray strings = pList[i].stringData();
                            data = m_strings.get() + stringCount;
                            for(size_t j = 0; j < strings.size(); ++j)
                            {
                                int len = std::strlen(strings[j]) + 1;
                                std::memcpy(m_chars.get() + charCount, strings[j], len);
                                m_strings[stringCount] = m_chars.get() + charCount;
                                stringCount += 1;
                                charCount += len;
                            }
                        }
                        break;
                    default:
                        assert(0 && "unknown type");
                }
                // Store parameter
                m_pList.push_back(Ri::Param(pList[i].spec(), paramName, data, size));
            }
        }

        operator Ri::ParamList() const
        {
            if(m_pList.empty())
                return Ri::ParamList();
            return Ri::ParamList(&m_pList[0], m_pList.size());
        }
};


/*
--------------------------------------------------------------------------------
Code generator for all CachedRequest implementations.
--------------------------------------------------------------------------------

[[[cog
from codegenutils import *
from Cheetah.Template import Template
riXml = parseXml(riXmlPath);

cacheTemplate = \
'''
class ${procName} : public CachedRequest
{
    private:
#for $type,$name in $memberData
        $type m_$name;
#end for
    public:
        ${procName}($formals)
#for ($i, ($type,$name)) in enumerate($memberData)
            ${':' if $i == 0 else ','} m_${name}($name)
#end for
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.${procName}($callArgs);
        }
};'''

customImplementations = set(['Procedural'])

memberTypeMap = {
    'RtString': 'CachedString',
    'RtToken': 'CachedString',
    'RtIntArray': 'CachedIntArray',
    'RtFloatArray': 'CachedFloatArray',
    'RtStringArray': 'CachedStringArray',
    'RtTokenArray': 'CachedStringArray',
    'RtPoint': 'CachedFloatTuple<3>',
    'RtColor': 'CachedFloatTuple<3>',
    'RtBound': 'CachedFloatTuple<6>',
    'RtBasis': 'CachedMatrix',
    'RtMatrix': 'CachedMatrix',
}
def getMemberType(arg):
    type = arg.findtext('Type')
    return memberTypeMap.get(type, type)

for proc in riXml.findall('Procedures/Procedure'):
    if proc.findall('Rib'):
        procName = proc.findtext('Name')
        if procName in customImplementations:
            continue
        argsXml = [a for a in proc.findall('Arguments/Argument')
                   if not a.findall('RibValue')]
        formals = [formalArg(a) for a in argsXml]
        memberData = [(getMemberType(a), a.findtext('Name')) for a in argsXml]
        hasPList = proc.findall('Arguments/ParamList')
        if hasPList:
            memberData += [('CachedParamList', 'pList')]
            formals += ['const Ri::ParamList& pList']
        formals = ', '.join(formals)
        callArgs = ', '.join(['m_%s' % name for type,name in memberData])
        cog.outl(str(Template(cacheTemplate, searchList=locals())))

]]]*/

class Declare : public CachedRequest
{
    private:
        CachedString m_name;
        CachedString m_declaration;
    public:
        Declare(RtConstString name, RtConstString declaration)
            : m_name(name)
            , m_declaration(declaration)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Declare(m_name, m_declaration);
        }
};

class FrameBegin : public CachedRequest
{
    private:
        RtInt m_number;
    public:
        FrameBegin(RtInt number)
            : m_number(number)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.FrameBegin(m_number);
        }
};

class FrameEnd : public CachedRequest
{
    private:
    public:
        FrameEnd()
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.FrameEnd();
        }
};

class WorldBegin : public CachedRequest
{
    private:
    public:
        WorldBegin()
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.WorldBegin();
        }
};

class WorldEnd : public CachedRequest
{
    private:
    public:
        WorldEnd()
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.WorldEnd();
        }
};

class IfBegin : public CachedRequest
{
    private:
        CachedString m_condition;
    public:
        IfBegin(RtConstString condition)
            : m_condition(condition)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.IfBegin(m_condition);
        }
};

class ElseIf : public CachedRequest
{
    private:
        CachedString m_condition;
    public:
        ElseIf(RtConstString condition)
            : m_condition(condition)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ElseIf(m_condition);
        }
};

class Else : public CachedRequest
{
    private:
    public:
        Else()
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Else();
        }
};

class IfEnd : public CachedRequest
{
    private:
    public:
        IfEnd()
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.IfEnd();
        }
};

class Format : public CachedRequest
{
    private:
        RtInt m_xresolution;
        RtInt m_yresolution;
        RtFloat m_pixelaspectratio;
    public:
        Format(RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio)
            : m_xresolution(xresolution)
            , m_yresolution(yresolution)
            , m_pixelaspectratio(pixelaspectratio)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Format(m_xresolution, m_yresolution, m_pixelaspectratio);
        }
};

class FrameAspectRatio : public CachedRequest
{
    private:
        RtFloat m_frameratio;
    public:
        FrameAspectRatio(RtFloat frameratio)
            : m_frameratio(frameratio)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.FrameAspectRatio(m_frameratio);
        }
};

class ScreenWindow : public CachedRequest
{
    private:
        RtFloat m_left;
        RtFloat m_right;
        RtFloat m_bottom;
        RtFloat m_top;
    public:
        ScreenWindow(RtFloat left, RtFloat right, RtFloat bottom, RtFloat top)
            : m_left(left)
            , m_right(right)
            , m_bottom(bottom)
            , m_top(top)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ScreenWindow(m_left, m_right, m_bottom, m_top);
        }
};

class CropWindow : public CachedRequest
{
    private:
        RtFloat m_xmin;
        RtFloat m_xmax;
        RtFloat m_ymin;
        RtFloat m_ymax;
    public:
        CropWindow(RtFloat xmin, RtFloat xmax, RtFloat ymin, RtFloat ymax)
            : m_xmin(xmin)
            , m_xmax(xmax)
            , m_ymin(ymin)
            , m_ymax(ymax)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.CropWindow(m_xmin, m_xmax, m_ymin, m_ymax);
        }
};

class Projection : public CachedRequest
{
    private:
        CachedString m_name;
        CachedParamList m_pList;
    public:
        Projection(RtConstToken name, const Ri::ParamList& pList)
            : m_name(name)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Projection(m_name, m_pList);
        }
};

class Clipping : public CachedRequest
{
    private:
        RtFloat m_cnear;
        RtFloat m_cfar;
    public:
        Clipping(RtFloat cnear, RtFloat cfar)
            : m_cnear(cnear)
            , m_cfar(cfar)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Clipping(m_cnear, m_cfar);
        }
};

class ClippingPlane : public CachedRequest
{
    private:
        RtFloat m_x;
        RtFloat m_y;
        RtFloat m_z;
        RtFloat m_nx;
        RtFloat m_ny;
        RtFloat m_nz;
    public:
        ClippingPlane(RtFloat x, RtFloat y, RtFloat z, RtFloat nx, RtFloat ny, RtFloat nz)
            : m_x(x)
            , m_y(y)
            , m_z(z)
            , m_nx(nx)
            , m_ny(ny)
            , m_nz(nz)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ClippingPlane(m_x, m_y, m_z, m_nx, m_ny, m_nz);
        }
};

class DepthOfField : public CachedRequest
{
    private:
        RtFloat m_fstop;
        RtFloat m_focallength;
        RtFloat m_focaldistance;
    public:
        DepthOfField(RtFloat fstop, RtFloat focallength, RtFloat focaldistance)
            : m_fstop(fstop)
            , m_focallength(focallength)
            , m_focaldistance(focaldistance)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.DepthOfField(m_fstop, m_focallength, m_focaldistance);
        }
};

class Shutter : public CachedRequest
{
    private:
        RtFloat m_opentime;
        RtFloat m_closetime;
    public:
        Shutter(RtFloat opentime, RtFloat closetime)
            : m_opentime(opentime)
            , m_closetime(closetime)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Shutter(m_opentime, m_closetime);
        }
};

class PixelVariance : public CachedRequest
{
    private:
        RtFloat m_variance;
    public:
        PixelVariance(RtFloat variance)
            : m_variance(variance)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.PixelVariance(m_variance);
        }
};

class PixelSamples : public CachedRequest
{
    private:
        RtFloat m_xsamples;
        RtFloat m_ysamples;
    public:
        PixelSamples(RtFloat xsamples, RtFloat ysamples)
            : m_xsamples(xsamples)
            , m_ysamples(ysamples)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.PixelSamples(m_xsamples, m_ysamples);
        }
};

class PixelFilter : public CachedRequest
{
    private:
        RtFilterFunc m_function;
        RtFloat m_xwidth;
        RtFloat m_ywidth;
    public:
        PixelFilter(RtFilterFunc function, RtFloat xwidth, RtFloat ywidth)
            : m_function(function)
            , m_xwidth(xwidth)
            , m_ywidth(ywidth)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.PixelFilter(m_function, m_xwidth, m_ywidth);
        }
};

class Exposure : public CachedRequest
{
    private:
        RtFloat m_gain;
        RtFloat m_gamma;
    public:
        Exposure(RtFloat gain, RtFloat gamma)
            : m_gain(gain)
            , m_gamma(gamma)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Exposure(m_gain, m_gamma);
        }
};

class Imager : public CachedRequest
{
    private:
        CachedString m_name;
        CachedParamList m_pList;
    public:
        Imager(RtConstToken name, const Ri::ParamList& pList)
            : m_name(name)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Imager(m_name, m_pList);
        }
};

class Quantize : public CachedRequest
{
    private:
        CachedString m_type;
        RtInt m_one;
        RtInt m_min;
        RtInt m_max;
        RtFloat m_ditheramplitude;
    public:
        Quantize(RtConstToken type, RtInt one, RtInt min, RtInt max, RtFloat ditheramplitude)
            : m_type(type)
            , m_one(one)
            , m_min(min)
            , m_max(max)
            , m_ditheramplitude(ditheramplitude)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Quantize(m_type, m_one, m_min, m_max, m_ditheramplitude);
        }
};

class Display : public CachedRequest
{
    private:
        CachedString m_name;
        CachedString m_type;
        CachedString m_mode;
        CachedParamList m_pList;
    public:
        Display(RtConstToken name, RtConstToken type, RtConstToken mode, const Ri::ParamList& pList)
            : m_name(name)
            , m_type(type)
            , m_mode(mode)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Display(m_name, m_type, m_mode, m_pList);
        }
};

class Hider : public CachedRequest
{
    private:
        CachedString m_name;
        CachedParamList m_pList;
    public:
        Hider(RtConstToken name, const Ri::ParamList& pList)
            : m_name(name)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Hider(m_name, m_pList);
        }
};

class ColorSamples : public CachedRequest
{
    private:
        CachedFloatArray m_nRGB;
        CachedFloatArray m_RGBn;
    public:
        ColorSamples(const FloatArray& nRGB, const FloatArray& RGBn)
            : m_nRGB(nRGB)
            , m_RGBn(RGBn)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ColorSamples(m_nRGB, m_RGBn);
        }
};

class RelativeDetail : public CachedRequest
{
    private:
        RtFloat m_relativedetail;
    public:
        RelativeDetail(RtFloat relativedetail)
            : m_relativedetail(relativedetail)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.RelativeDetail(m_relativedetail);
        }
};

class Option : public CachedRequest
{
    private:
        CachedString m_name;
        CachedParamList m_pList;
    public:
        Option(RtConstToken name, const Ri::ParamList& pList)
            : m_name(name)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Option(m_name, m_pList);
        }
};

class AttributeBegin : public CachedRequest
{
    private:
    public:
        AttributeBegin()
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.AttributeBegin();
        }
};

class AttributeEnd : public CachedRequest
{
    private:
    public:
        AttributeEnd()
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.AttributeEnd();
        }
};

class Color : public CachedRequest
{
    private:
        CachedFloatTuple<3> m_Cq;
    public:
        Color(RtConstColor Cq)
            : m_Cq(Cq)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Color(m_Cq);
        }
};

class Opacity : public CachedRequest
{
    private:
        CachedFloatTuple<3> m_Os;
    public:
        Opacity(RtConstColor Os)
            : m_Os(Os)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Opacity(m_Os);
        }
};

class TextureCoordinates : public CachedRequest
{
    private:
        RtFloat m_s1;
        RtFloat m_t1;
        RtFloat m_s2;
        RtFloat m_t2;
        RtFloat m_s3;
        RtFloat m_t3;
        RtFloat m_s4;
        RtFloat m_t4;
    public:
        TextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4)
            : m_s1(s1)
            , m_t1(t1)
            , m_s2(s2)
            , m_t2(t2)
            , m_s3(s3)
            , m_t3(t3)
            , m_s4(s4)
            , m_t4(t4)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.TextureCoordinates(m_s1, m_t1, m_s2, m_t2, m_s3, m_t3, m_s4, m_t4);
        }
};

class LightSource : public CachedRequest
{
    private:
        CachedString m_shadername;
        CachedString m_name;
        CachedParamList m_pList;
    public:
        LightSource(RtConstToken shadername, RtConstToken name, const Ri::ParamList& pList)
            : m_shadername(shadername)
            , m_name(name)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.LightSource(m_shadername, m_name, m_pList);
        }
};

class AreaLightSource : public CachedRequest
{
    private:
        CachedString m_shadername;
        CachedString m_name;
        CachedParamList m_pList;
    public:
        AreaLightSource(RtConstToken shadername, RtConstToken name, const Ri::ParamList& pList)
            : m_shadername(shadername)
            , m_name(name)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.AreaLightSource(m_shadername, m_name, m_pList);
        }
};

class Illuminate : public CachedRequest
{
    private:
        CachedString m_name;
        RtBoolean m_onoff;
    public:
        Illuminate(RtConstToken name, RtBoolean onoff)
            : m_name(name)
            , m_onoff(onoff)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Illuminate(m_name, m_onoff);
        }
};

class Surface : public CachedRequest
{
    private:
        CachedString m_name;
        CachedParamList m_pList;
    public:
        Surface(RtConstToken name, const Ri::ParamList& pList)
            : m_name(name)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Surface(m_name, m_pList);
        }
};

class Displacement : public CachedRequest
{
    private:
        CachedString m_name;
        CachedParamList m_pList;
    public:
        Displacement(RtConstToken name, const Ri::ParamList& pList)
            : m_name(name)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Displacement(m_name, m_pList);
        }
};

class Atmosphere : public CachedRequest
{
    private:
        CachedString m_name;
        CachedParamList m_pList;
    public:
        Atmosphere(RtConstToken name, const Ri::ParamList& pList)
            : m_name(name)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Atmosphere(m_name, m_pList);
        }
};

class Interior : public CachedRequest
{
    private:
        CachedString m_name;
        CachedParamList m_pList;
    public:
        Interior(RtConstToken name, const Ri::ParamList& pList)
            : m_name(name)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Interior(m_name, m_pList);
        }
};

class Exterior : public CachedRequest
{
    private:
        CachedString m_name;
        CachedParamList m_pList;
    public:
        Exterior(RtConstToken name, const Ri::ParamList& pList)
            : m_name(name)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Exterior(m_name, m_pList);
        }
};

class ShaderLayer : public CachedRequest
{
    private:
        CachedString m_type;
        CachedString m_name;
        CachedString m_layername;
        CachedParamList m_pList;
    public:
        ShaderLayer(RtConstToken type, RtConstToken name, RtConstToken layername, const Ri::ParamList& pList)
            : m_type(type)
            , m_name(name)
            , m_layername(layername)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ShaderLayer(m_type, m_name, m_layername, m_pList);
        }
};

class ConnectShaderLayers : public CachedRequest
{
    private:
        CachedString m_type;
        CachedString m_layer1;
        CachedString m_variable1;
        CachedString m_layer2;
        CachedString m_variable2;
    public:
        ConnectShaderLayers(RtConstToken type, RtConstToken layer1, RtConstToken variable1, RtConstToken layer2, RtConstToken variable2)
            : m_type(type)
            , m_layer1(layer1)
            , m_variable1(variable1)
            , m_layer2(layer2)
            , m_variable2(variable2)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ConnectShaderLayers(m_type, m_layer1, m_variable1, m_layer2, m_variable2);
        }
};

class ShadingRate : public CachedRequest
{
    private:
        RtFloat m_size;
    public:
        ShadingRate(RtFloat size)
            : m_size(size)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ShadingRate(m_size);
        }
};

class ShadingInterpolation : public CachedRequest
{
    private:
        CachedString m_type;
    public:
        ShadingInterpolation(RtConstToken type)
            : m_type(type)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ShadingInterpolation(m_type);
        }
};

class Matte : public CachedRequest
{
    private:
        RtBoolean m_onoff;
    public:
        Matte(RtBoolean onoff)
            : m_onoff(onoff)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Matte(m_onoff);
        }
};

class Bound : public CachedRequest
{
    private:
        CachedFloatTuple<6> m_bound;
    public:
        Bound(RtConstBound bound)
            : m_bound(bound)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Bound(m_bound);
        }
};

class Detail : public CachedRequest
{
    private:
        CachedFloatTuple<6> m_bound;
    public:
        Detail(RtConstBound bound)
            : m_bound(bound)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Detail(m_bound);
        }
};

class DetailRange : public CachedRequest
{
    private:
        RtFloat m_offlow;
        RtFloat m_onlow;
        RtFloat m_onhigh;
        RtFloat m_offhigh;
    public:
        DetailRange(RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh)
            : m_offlow(offlow)
            , m_onlow(onlow)
            , m_onhigh(onhigh)
            , m_offhigh(offhigh)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.DetailRange(m_offlow, m_onlow, m_onhigh, m_offhigh);
        }
};

class GeometricApproximation : public CachedRequest
{
    private:
        CachedString m_type;
        RtFloat m_value;
    public:
        GeometricApproximation(RtConstToken type, RtFloat value)
            : m_type(type)
            , m_value(value)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.GeometricApproximation(m_type, m_value);
        }
};

class Orientation : public CachedRequest
{
    private:
        CachedString m_orientation;
    public:
        Orientation(RtConstToken orientation)
            : m_orientation(orientation)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Orientation(m_orientation);
        }
};

class ReverseOrientation : public CachedRequest
{
    private:
    public:
        ReverseOrientation()
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ReverseOrientation();
        }
};

class Sides : public CachedRequest
{
    private:
        RtInt m_nsides;
    public:
        Sides(RtInt nsides)
            : m_nsides(nsides)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Sides(m_nsides);
        }
};

class Identity : public CachedRequest
{
    private:
    public:
        Identity()
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Identity();
        }
};

class Transform : public CachedRequest
{
    private:
        CachedMatrix m_transform;
    public:
        Transform(RtConstMatrix transform)
            : m_transform(transform)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Transform(m_transform);
        }
};

class ConcatTransform : public CachedRequest
{
    private:
        CachedMatrix m_transform;
    public:
        ConcatTransform(RtConstMatrix transform)
            : m_transform(transform)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ConcatTransform(m_transform);
        }
};

class Perspective : public CachedRequest
{
    private:
        RtFloat m_fov;
    public:
        Perspective(RtFloat fov)
            : m_fov(fov)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Perspective(m_fov);
        }
};

class Translate : public CachedRequest
{
    private:
        RtFloat m_dx;
        RtFloat m_dy;
        RtFloat m_dz;
    public:
        Translate(RtFloat dx, RtFloat dy, RtFloat dz)
            : m_dx(dx)
            , m_dy(dy)
            , m_dz(dz)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Translate(m_dx, m_dy, m_dz);
        }
};

class Rotate : public CachedRequest
{
    private:
        RtFloat m_angle;
        RtFloat m_dx;
        RtFloat m_dy;
        RtFloat m_dz;
    public:
        Rotate(RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz)
            : m_angle(angle)
            , m_dx(dx)
            , m_dy(dy)
            , m_dz(dz)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Rotate(m_angle, m_dx, m_dy, m_dz);
        }
};

class Scale : public CachedRequest
{
    private:
        RtFloat m_sx;
        RtFloat m_sy;
        RtFloat m_sz;
    public:
        Scale(RtFloat sx, RtFloat sy, RtFloat sz)
            : m_sx(sx)
            , m_sy(sy)
            , m_sz(sz)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Scale(m_sx, m_sy, m_sz);
        }
};

class Skew : public CachedRequest
{
    private:
        RtFloat m_angle;
        RtFloat m_dx1;
        RtFloat m_dy1;
        RtFloat m_dz1;
        RtFloat m_dx2;
        RtFloat m_dy2;
        RtFloat m_dz2;
    public:
        Skew(RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1, RtFloat dx2, RtFloat dy2, RtFloat dz2)
            : m_angle(angle)
            , m_dx1(dx1)
            , m_dy1(dy1)
            , m_dz1(dz1)
            , m_dx2(dx2)
            , m_dy2(dy2)
            , m_dz2(dz2)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Skew(m_angle, m_dx1, m_dy1, m_dz1, m_dx2, m_dy2, m_dz2);
        }
};

class CoordinateSystem : public CachedRequest
{
    private:
        CachedString m_space;
    public:
        CoordinateSystem(RtConstToken space)
            : m_space(space)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.CoordinateSystem(m_space);
        }
};

class CoordSysTransform : public CachedRequest
{
    private:
        CachedString m_space;
    public:
        CoordSysTransform(RtConstToken space)
            : m_space(space)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.CoordSysTransform(m_space);
        }
};

class TransformBegin : public CachedRequest
{
    private:
    public:
        TransformBegin()
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.TransformBegin();
        }
};

class TransformEnd : public CachedRequest
{
    private:
    public:
        TransformEnd()
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.TransformEnd();
        }
};

class Resource : public CachedRequest
{
    private:
        CachedString m_handle;
        CachedString m_type;
        CachedParamList m_pList;
    public:
        Resource(RtConstToken handle, RtConstToken type, const Ri::ParamList& pList)
            : m_handle(handle)
            , m_type(type)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Resource(m_handle, m_type, m_pList);
        }
};

class ResourceBegin : public CachedRequest
{
    private:
    public:
        ResourceBegin()
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ResourceBegin();
        }
};

class ResourceEnd : public CachedRequest
{
    private:
    public:
        ResourceEnd()
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ResourceEnd();
        }
};

class Attribute : public CachedRequest
{
    private:
        CachedString m_name;
        CachedParamList m_pList;
    public:
        Attribute(RtConstToken name, const Ri::ParamList& pList)
            : m_name(name)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Attribute(m_name, m_pList);
        }
};

class Polygon : public CachedRequest
{
    private:
        CachedParamList m_pList;
    public:
        Polygon(const Ri::ParamList& pList)
            : m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Polygon(m_pList);
        }
};

class GeneralPolygon : public CachedRequest
{
    private:
        CachedIntArray m_nverts;
        CachedParamList m_pList;
    public:
        GeneralPolygon(const IntArray& nverts, const Ri::ParamList& pList)
            : m_nverts(nverts)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.GeneralPolygon(m_nverts, m_pList);
        }
};

class PointsPolygons : public CachedRequest
{
    private:
        CachedIntArray m_nverts;
        CachedIntArray m_verts;
        CachedParamList m_pList;
    public:
        PointsPolygons(const IntArray& nverts, const IntArray& verts, const Ri::ParamList& pList)
            : m_nverts(nverts)
            , m_verts(verts)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.PointsPolygons(m_nverts, m_verts, m_pList);
        }
};

class PointsGeneralPolygons : public CachedRequest
{
    private:
        CachedIntArray m_nloops;
        CachedIntArray m_nverts;
        CachedIntArray m_verts;
        CachedParamList m_pList;
    public:
        PointsGeneralPolygons(const IntArray& nloops, const IntArray& nverts, const IntArray& verts, const Ri::ParamList& pList)
            : m_nloops(nloops)
            , m_nverts(nverts)
            , m_verts(verts)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.PointsGeneralPolygons(m_nloops, m_nverts, m_verts, m_pList);
        }
};

class Basis : public CachedRequest
{
    private:
        CachedMatrix m_ubasis;
        RtInt m_ustep;
        CachedMatrix m_vbasis;
        RtInt m_vstep;
    public:
        Basis(RtConstBasis ubasis, RtInt ustep, RtConstBasis vbasis, RtInt vstep)
            : m_ubasis(ubasis)
            , m_ustep(ustep)
            , m_vbasis(vbasis)
            , m_vstep(vstep)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Basis(m_ubasis, m_ustep, m_vbasis, m_vstep);
        }
};

class Patch : public CachedRequest
{
    private:
        CachedString m_type;
        CachedParamList m_pList;
    public:
        Patch(RtConstToken type, const Ri::ParamList& pList)
            : m_type(type)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Patch(m_type, m_pList);
        }
};

class PatchMesh : public CachedRequest
{
    private:
        CachedString m_type;
        RtInt m_nu;
        CachedString m_uwrap;
        RtInt m_nv;
        CachedString m_vwrap;
        CachedParamList m_pList;
    public:
        PatchMesh(RtConstToken type, RtInt nu, RtConstToken uwrap, RtInt nv, RtConstToken vwrap, const Ri::ParamList& pList)
            : m_type(type)
            , m_nu(nu)
            , m_uwrap(uwrap)
            , m_nv(nv)
            , m_vwrap(vwrap)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.PatchMesh(m_type, m_nu, m_uwrap, m_nv, m_vwrap, m_pList);
        }
};

class NuPatch : public CachedRequest
{
    private:
        RtInt m_nu;
        RtInt m_uorder;
        CachedFloatArray m_uknot;
        RtFloat m_umin;
        RtFloat m_umax;
        RtInt m_nv;
        RtInt m_vorder;
        CachedFloatArray m_vknot;
        RtFloat m_vmin;
        RtFloat m_vmax;
        CachedParamList m_pList;
    public:
        NuPatch(RtInt nu, RtInt uorder, const FloatArray& uknot, RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder, const FloatArray& vknot, RtFloat vmin, RtFloat vmax, const Ri::ParamList& pList)
            : m_nu(nu)
            , m_uorder(uorder)
            , m_uknot(uknot)
            , m_umin(umin)
            , m_umax(umax)
            , m_nv(nv)
            , m_vorder(vorder)
            , m_vknot(vknot)
            , m_vmin(vmin)
            , m_vmax(vmax)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.NuPatch(m_nu, m_uorder, m_uknot, m_umin, m_umax, m_nv, m_vorder, m_vknot, m_vmin, m_vmax, m_pList);
        }
};

class TrimCurve : public CachedRequest
{
    private:
        CachedIntArray m_ncurves;
        CachedIntArray m_order;
        CachedFloatArray m_knot;
        CachedFloatArray m_min;
        CachedFloatArray m_max;
        CachedIntArray m_n;
        CachedFloatArray m_u;
        CachedFloatArray m_v;
        CachedFloatArray m_w;
    public:
        TrimCurve(const IntArray& ncurves, const IntArray& order, const FloatArray& knot, const FloatArray& min, const FloatArray& max, const IntArray& n, const FloatArray& u, const FloatArray& v, const FloatArray& w)
            : m_ncurves(ncurves)
            , m_order(order)
            , m_knot(knot)
            , m_min(min)
            , m_max(max)
            , m_n(n)
            , m_u(u)
            , m_v(v)
            , m_w(w)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.TrimCurve(m_ncurves, m_order, m_knot, m_min, m_max, m_n, m_u, m_v, m_w);
        }
};

class SubdivisionMesh : public CachedRequest
{
    private:
        CachedString m_scheme;
        CachedIntArray m_nvertices;
        CachedIntArray m_vertices;
        CachedStringArray m_tags;
        CachedIntArray m_nargs;
        CachedIntArray m_intargs;
        CachedFloatArray m_floatargs;
        CachedParamList m_pList;
    public:
        SubdivisionMesh(RtConstToken scheme, const IntArray& nvertices, const IntArray& vertices, const TokenArray& tags, const IntArray& nargs, const IntArray& intargs, const FloatArray& floatargs, const Ri::ParamList& pList)
            : m_scheme(scheme)
            , m_nvertices(nvertices)
            , m_vertices(vertices)
            , m_tags(tags)
            , m_nargs(nargs)
            , m_intargs(intargs)
            , m_floatargs(floatargs)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.SubdivisionMesh(m_scheme, m_nvertices, m_vertices, m_tags, m_nargs, m_intargs, m_floatargs, m_pList);
        }
};

class Sphere : public CachedRequest
{
    private:
        RtFloat m_radius;
        RtFloat m_zmin;
        RtFloat m_zmax;
        RtFloat m_thetamax;
        CachedParamList m_pList;
    public:
        Sphere(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, const Ri::ParamList& pList)
            : m_radius(radius)
            , m_zmin(zmin)
            , m_zmax(zmax)
            , m_thetamax(thetamax)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Sphere(m_radius, m_zmin, m_zmax, m_thetamax, m_pList);
        }
};

class Cone : public CachedRequest
{
    private:
        RtFloat m_height;
        RtFloat m_radius;
        RtFloat m_thetamax;
        CachedParamList m_pList;
    public:
        Cone(RtFloat height, RtFloat radius, RtFloat thetamax, const Ri::ParamList& pList)
            : m_height(height)
            , m_radius(radius)
            , m_thetamax(thetamax)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Cone(m_height, m_radius, m_thetamax, m_pList);
        }
};

class Cylinder : public CachedRequest
{
    private:
        RtFloat m_radius;
        RtFloat m_zmin;
        RtFloat m_zmax;
        RtFloat m_thetamax;
        CachedParamList m_pList;
    public:
        Cylinder(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, const Ri::ParamList& pList)
            : m_radius(radius)
            , m_zmin(zmin)
            , m_zmax(zmax)
            , m_thetamax(thetamax)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Cylinder(m_radius, m_zmin, m_zmax, m_thetamax, m_pList);
        }
};

class Hyperboloid : public CachedRequest
{
    private:
        CachedFloatTuple<3> m_point1;
        CachedFloatTuple<3> m_point2;
        RtFloat m_thetamax;
        CachedParamList m_pList;
    public:
        Hyperboloid(RtConstPoint point1, RtConstPoint point2, RtFloat thetamax, const Ri::ParamList& pList)
            : m_point1(point1)
            , m_point2(point2)
            , m_thetamax(thetamax)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Hyperboloid(m_point1, m_point2, m_thetamax, m_pList);
        }
};

class Paraboloid : public CachedRequest
{
    private:
        RtFloat m_rmax;
        RtFloat m_zmin;
        RtFloat m_zmax;
        RtFloat m_thetamax;
        CachedParamList m_pList;
    public:
        Paraboloid(RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, const Ri::ParamList& pList)
            : m_rmax(rmax)
            , m_zmin(zmin)
            , m_zmax(zmax)
            , m_thetamax(thetamax)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Paraboloid(m_rmax, m_zmin, m_zmax, m_thetamax, m_pList);
        }
};

class Disk : public CachedRequest
{
    private:
        RtFloat m_height;
        RtFloat m_radius;
        RtFloat m_thetamax;
        CachedParamList m_pList;
    public:
        Disk(RtFloat height, RtFloat radius, RtFloat thetamax, const Ri::ParamList& pList)
            : m_height(height)
            , m_radius(radius)
            , m_thetamax(thetamax)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Disk(m_height, m_radius, m_thetamax, m_pList);
        }
};

class Torus : public CachedRequest
{
    private:
        RtFloat m_majorrad;
        RtFloat m_minorrad;
        RtFloat m_phimin;
        RtFloat m_phimax;
        RtFloat m_thetamax;
        CachedParamList m_pList;
    public:
        Torus(RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, const Ri::ParamList& pList)
            : m_majorrad(majorrad)
            , m_minorrad(minorrad)
            , m_phimin(phimin)
            , m_phimax(phimax)
            , m_thetamax(thetamax)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Torus(m_majorrad, m_minorrad, m_phimin, m_phimax, m_thetamax, m_pList);
        }
};

class Points : public CachedRequest
{
    private:
        CachedParamList m_pList;
    public:
        Points(const Ri::ParamList& pList)
            : m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Points(m_pList);
        }
};

class Curves : public CachedRequest
{
    private:
        CachedString m_type;
        CachedIntArray m_nvertices;
        CachedString m_wrap;
        CachedParamList m_pList;
    public:
        Curves(RtConstToken type, const IntArray& nvertices, RtConstToken wrap, const Ri::ParamList& pList)
            : m_type(type)
            , m_nvertices(nvertices)
            , m_wrap(wrap)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Curves(m_type, m_nvertices, m_wrap, m_pList);
        }
};

class Blobby : public CachedRequest
{
    private:
        RtInt m_nleaf;
        CachedIntArray m_code;
        CachedFloatArray m_floats;
        CachedStringArray m_strings;
        CachedParamList m_pList;
    public:
        Blobby(RtInt nleaf, const IntArray& code, const FloatArray& floats, const TokenArray& strings, const Ri::ParamList& pList)
            : m_nleaf(nleaf)
            , m_code(code)
            , m_floats(floats)
            , m_strings(strings)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Blobby(m_nleaf, m_code, m_floats, m_strings, m_pList);
        }
};

class Geometry : public CachedRequest
{
    private:
        CachedString m_type;
        CachedParamList m_pList;
    public:
        Geometry(RtConstToken type, const Ri::ParamList& pList)
            : m_type(type)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Geometry(m_type, m_pList);
        }
};

class SolidBegin : public CachedRequest
{
    private:
        CachedString m_type;
    public:
        SolidBegin(RtConstToken type)
            : m_type(type)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.SolidBegin(m_type);
        }
};

class SolidEnd : public CachedRequest
{
    private:
    public:
        SolidEnd()
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.SolidEnd();
        }
};

class ObjectBegin : public CachedRequest
{
    private:
        CachedString m_name;
    public:
        ObjectBegin(RtConstToken name)
            : m_name(name)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ObjectBegin(m_name);
        }
};

class ObjectEnd : public CachedRequest
{
    private:
    public:
        ObjectEnd()
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ObjectEnd();
        }
};

class ObjectInstance : public CachedRequest
{
    private:
        CachedString m_name;
    public:
        ObjectInstance(RtConstToken name)
            : m_name(name)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ObjectInstance(m_name);
        }
};

class MotionBegin : public CachedRequest
{
    private:
        CachedFloatArray m_times;
    public:
        MotionBegin(const FloatArray& times)
            : m_times(times)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.MotionBegin(m_times);
        }
};

class MotionEnd : public CachedRequest
{
    private:
    public:
        MotionEnd()
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.MotionEnd();
        }
};

class MakeTexture : public CachedRequest
{
    private:
        CachedString m_imagefile;
        CachedString m_texturefile;
        CachedString m_swrap;
        CachedString m_twrap;
        RtFilterFunc m_filterfunc;
        RtFloat m_swidth;
        RtFloat m_twidth;
        CachedParamList m_pList;
    public:
        MakeTexture(RtConstString imagefile, RtConstString texturefile, RtConstToken swrap, RtConstToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, const Ri::ParamList& pList)
            : m_imagefile(imagefile)
            , m_texturefile(texturefile)
            , m_swrap(swrap)
            , m_twrap(twrap)
            , m_filterfunc(filterfunc)
            , m_swidth(swidth)
            , m_twidth(twidth)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.MakeTexture(m_imagefile, m_texturefile, m_swrap, m_twrap, m_filterfunc, m_swidth, m_twidth, m_pList);
        }
};

class MakeLatLongEnvironment : public CachedRequest
{
    private:
        CachedString m_imagefile;
        CachedString m_reflfile;
        RtFilterFunc m_filterfunc;
        RtFloat m_swidth;
        RtFloat m_twidth;
        CachedParamList m_pList;
    public:
        MakeLatLongEnvironment(RtConstString imagefile, RtConstString reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, const Ri::ParamList& pList)
            : m_imagefile(imagefile)
            , m_reflfile(reflfile)
            , m_filterfunc(filterfunc)
            , m_swidth(swidth)
            , m_twidth(twidth)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.MakeLatLongEnvironment(m_imagefile, m_reflfile, m_filterfunc, m_swidth, m_twidth, m_pList);
        }
};

class MakeCubeFaceEnvironment : public CachedRequest
{
    private:
        CachedString m_px;
        CachedString m_nx;
        CachedString m_py;
        CachedString m_ny;
        CachedString m_pz;
        CachedString m_nz;
        CachedString m_reflfile;
        RtFloat m_fov;
        RtFilterFunc m_filterfunc;
        RtFloat m_swidth;
        RtFloat m_twidth;
        CachedParamList m_pList;
    public:
        MakeCubeFaceEnvironment(RtConstString px, RtConstString nx, RtConstString py, RtConstString ny, RtConstString pz, RtConstString nz, RtConstString reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, const Ri::ParamList& pList)
            : m_px(px)
            , m_nx(nx)
            , m_py(py)
            , m_ny(ny)
            , m_pz(pz)
            , m_nz(nz)
            , m_reflfile(reflfile)
            , m_fov(fov)
            , m_filterfunc(filterfunc)
            , m_swidth(swidth)
            , m_twidth(twidth)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.MakeCubeFaceEnvironment(m_px, m_nx, m_py, m_ny, m_pz, m_nz, m_reflfile, m_fov, m_filterfunc, m_swidth, m_twidth, m_pList);
        }
};

class MakeShadow : public CachedRequest
{
    private:
        CachedString m_picfile;
        CachedString m_shadowfile;
        CachedParamList m_pList;
    public:
        MakeShadow(RtConstString picfile, RtConstString shadowfile, const Ri::ParamList& pList)
            : m_picfile(picfile)
            , m_shadowfile(shadowfile)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.MakeShadow(m_picfile, m_shadowfile, m_pList);
        }
};

class MakeOcclusion : public CachedRequest
{
    private:
        CachedStringArray m_picfiles;
        CachedString m_shadowfile;
        CachedParamList m_pList;
    public:
        MakeOcclusion(const StringArray& picfiles, RtConstString shadowfile, const Ri::ParamList& pList)
            : m_picfiles(picfiles)
            , m_shadowfile(shadowfile)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.MakeOcclusion(m_picfiles, m_shadowfile, m_pList);
        }
};

class ErrorHandler : public CachedRequest
{
    private:
        RtErrorFunc m_handler;
    public:
        ErrorHandler(RtErrorFunc handler)
            : m_handler(handler)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ErrorHandler(m_handler);
        }
};

class ReadArchive : public CachedRequest
{
    private:
        CachedString m_name;
        RtArchiveCallback m_callback;
        CachedParamList m_pList;
    public:
        ReadArchive(RtConstToken name, RtArchiveCallback callback, const Ri::ParamList& pList)
            : m_name(name)
            , m_callback(callback)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ReadArchive(m_name, m_callback, m_pList);
        }
};

class ArchiveBegin : public CachedRequest
{
    private:
        CachedString m_name;
        CachedParamList m_pList;
    public:
        ArchiveBegin(RtConstToken name, const Ri::ParamList& pList)
            : m_name(name)
            , m_pList(pList)
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ArchiveBegin(m_name, m_pList);
        }
};

class ArchiveEnd : public CachedRequest
{
    private:
    public:
        ArchiveEnd()
        { }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.ArchiveEnd();
        }
};
//[[[end]]]


// Special case implementation of cached Procedurals.
//
// The reason this is special is the need to carefully manage the lifetime of
// the procedural data.  In particular, we need to avoid deleting the data
// until the destructor is called so that reCall() will work multiple times.
class Procedural : public CachedRequest
{
    private:
        RtPointer m_data;
        CachedFloatTuple<6> m_bound;
        RtProcSubdivFunc m_refineproc;
        RtProcFreeFunc m_freeproc;

        static void doNothingFreeProc(void* ptr) {}

    public:
        Procedural(RtPointer data, RtConstBound bound, RtProcSubdivFunc refineproc, RtProcFreeFunc freeproc)
            : m_data(data)
            , m_bound(bound)
            , m_refineproc(refineproc)
            , m_freeproc(freeproc)
        { }

        ~Procedural()
        {
            m_freeproc(m_data);
        }

        virtual void reCall(Ri::Renderer& context) const
        {
            context.Procedural(m_data, m_bound, m_refineproc, &doNothingFreeProc);
        }
};


} // namespace RiCache

} // namespace Aqsis

#endif // AQSIS_API_CACHE_H_INCLUDED

// vi: set et:
