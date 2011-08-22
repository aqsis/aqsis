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

/// \file
/// \brief RIB writer implementation of Ri::Renderer
/// \author Chris Foster [chris42f (at) g mail (d0t) com]

#include <aqsis/riutil/ribwriter.h>
#include <aqsis/riutil/ricxx.h>
#include <aqsis/riutil/ricxx_filter.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#ifdef USE_GZIPPED_RIB
#   include <boost/iostreams/filtering_stream.hpp>
#   include <boost/iostreams/filter/gzip.hpp>
#endif
#include <boost/cstdint.hpp>

#include <aqsis/riutil/ribparser.h>
#include <aqsis/riutil/tokendictionary.h>
#include <aqsis/util/file.h>
#include <aqsis/util/logging.h>
#include <aqsis/util/smartptr.h>
#include "errorhandlerimpl.h"

namespace Aqsis {

namespace {

//------------------------------------------------------------------------------
/// A formatter object which turns a sequence of tokens into an ascii RIB
/// stream.
class AsciiFormatter
{
    private:
        std::ostream& m_out;
        int m_indent;
        int m_indentStep;
        char m_indentChar;
        std::string m_indentString;

    public:
        AsciiFormatter(std::ostream& out, const RibWriterOptions& opts)
            : m_out(out),
            m_indent(0),
            m_indentStep(opts.indentStep),
            m_indentChar(opts.indentChar),
            m_indentString()
        {
            // set the precision for floats.  You need at >= 9 decimal digits
            // to avoid loss of precision for 32 bit IEEE floats.
            m_out.precision(9);
        }

        void increaseIndent()
        {
            m_indent += m_indentStep;
            m_indentString.assign(m_indent, m_indentChar);
        }
        void decreaseIndent()
        {
            m_indent -= m_indentStep;
            if(m_indent < 0) // may happen if formatting unbalanced RIB fragments
                m_indent = 0;
            m_indentString.assign(m_indent, m_indentChar);
        }

        void whitespace()     { m_out << ' '; }

        void beginRequest(const char* name)
        {
            m_out << m_indentString;
            m_out << name;
        }
        void endRequest()
        {
            m_out << '\n';
        }

        void archiveRecord(RtConstToken type, const char* string)
        {
            if(!strcmp(type, "comment"))
                m_out << m_indentString << "#" << string << '\n';
            else if(!strcmp(type, "structure"))
                m_out << m_indentString << "##" << string << '\n';
            else if(!strcmp(type, "verbatim"))
                m_out << string;
        }

        void print(RtInt i)   { m_out << i; }
        void print(RtFloat f) { m_out << f; }

        void print(RtConstString s)
        {
            m_out << '\"';
            while(*s)
            {
                // Strings are more complicated than you might expect, since
                // we need to escape any non-printable characters.
                TqUint8 c = *s;
                switch(c)
                {
                    case '"':  m_out << "\\\""; break;
                    case '\n': m_out << "\\n";  break;
                    case '\r': m_out << "\\r";  break;
                    case '\t': m_out << "\\t";  break;
                    case '\b': m_out << "\\b";  break;
                    case '\f': m_out << "\\f";  break;
                    case '\\': m_out << "\\\\"; break;
                    default:
                        if(c >= 32 && c <= 176)
                            m_out.put(c);
                        else
                        {
                            // encode unprintable ASCII with octal escape seq
                            m_out << '\\';
                            m_out << std::oct << int(c) << std::dec;
                        }
                        break;
                }
                ++s;
            }
            m_out << '\"';
        }

        template<typename T>
        void print(const Ri::Array<T>& a)
        {
            m_out << '[';
            for(size_t i = 0; i < a.size(); ++i)
            {
                print(a[i]);
                if(i+1 != a.size())
                    whitespace();
            }
            m_out << ']';
        }

        // Print a float triple (color or point)
        void printTriple(const float* f)
        {
            m_out << f[0] << ' ' << f[1] << ' ' << f[2];
        }

        template<typename T>
        void printParam(const char* token, const Ri::Array<T>& a)
        {
            print(token);
            whitespace();
            print(a);
        }
};


/// Safe type punning through a union.
template<typename T1, typename T2>
T1 union_cast(T2 val)
{
    union {
        T1 v1;
        T2 v2;
    } converter;
    converter.v2 = val;
    return converter.v1;
}


//------------------------------------------------------------------------------
/// A formatter object which turns a sequence of tokens into a binary RIB
/// stream.
class BinaryFormatter
{
    private:
        std::ostream& m_out;
        typedef std::map<std::string, TqUint8> EncodedRequestMap;
        EncodedRequestMap m_encodedRequests;
        TqUint8 m_currRequestCode;

        // Unpack MSB into c[0], down to LSB into c[3]
        //
        // Order chosen so that m_out.write(c) writes the bytes from MSB to LSB
        static inline void unpack(TqUint32 i, char c[4])
        {
            // Unpack bytes with bitwise ops to avoid endianness issues.
            c[0] = i >> 24;
            c[1] = (i >> 16) & 0xFF;
            c[2] = (i >> 8) & 0xFF;
            c[3] = i & 0xFF;
        }

        // Get the highest byte index which is nonzero, where 3 is the MSB and
        // 0 is the LSB (note, opposite to the index of c).
        static inline int highestByteNonzero(const char c[4])
        {
            if(c[0]) return 3;
            if(c[1]) return 2;
            if(c[2]) return 1;
            return 0;
        }

        // Encode an unsigned int32, i to the stream
        //
        // baseCode is the binary RIB prefix code to use.
        void encodeInt32(TqUint32 i, TqUint8 baseCode)
        {
            char c[4];
            unpack(i, c);
            int high = highestByteNonzero(c);
            m_out.put(baseCode + high);
            m_out.write(c+3-high, high+1);
        }

    public:
        BinaryFormatter(std::ostream& out, const RibWriterOptions& opts)
            : m_out(out), m_encodedRequests(), m_currRequestCode(0) { }

        void increaseIndent() { }
        void decreaseIndent() { }
        void whitespace()     { }

        void beginRequest(const char* name)
        {
            TqUint8 code = 0;
            EncodedRequestMap::const_iterator i = m_encodedRequests.find(name);
            if(i == m_encodedRequests.end())
            {
                // Request not yet associated with a code; do so now.
                code = m_currRequestCode++;
                m_encodedRequests[name] = code;
                assert(m_currRequestCode != 0); // code shouldn't wrap around.
                m_out.put(0314);
                m_out.put(code);
                print(name);
            }
            else
                code = i->second;
            m_out.put(0246);
            m_out.put(code);
        }
        void endRequest() { }

        void archiveRecord(RtConstToken type, const char* string)
        {
            if(!strcmp(type, "comment"))
                m_out << "#" << string << '\n';
            else if(!strcmp(type, "structure"))
                m_out << "##" << string << '\n';
            else if(!strcmp(type, "verbatim"))
                m_out << string;
        }

        void print(RtInt i)
        {
            encodeInt32(i, 0200);
        }
        void print(RtFloat f)
        {
            m_out.put(0244);
            char c[4];
            unpack(union_cast<TqUint32>(f), c);
            m_out.write(c, 4);
        }
        void print(RtConstString s)
        {
            TqUint32 len = std::strlen(s);
            if(len < 16)
            {
                // special short string encoding
                m_out.put(0220 + len);
                m_out.write(s, len);
            }
            else
            {
                encodeInt32(len, 0240);
                m_out.write(s, len);
            }
        }

        template<typename T>
        void print(const Ri::Array<T>& a)
        {
            m_out.put('[');
            for(size_t i = 0; i < a.size(); ++i)
                print(a[i]);
            m_out.put(']');
        }

        void print(const Ri::FloatArray& a)
        {
            encodeInt32(a.size(), 0310);
            for(size_t i = 0; i < a.size(); ++i)
            {
                char c[4];
                unpack(union_cast<TqUint32>(a[i]), c);
                m_out.write(c, 4);
            }
        }

        void printTriple(const float* f)
        {
            print(f[0]); print(f[1]); print(f[2]);
        }

        template<typename T>
        void printParam(const char* token, const Ri::Array<T>& a)
        {
            print(token);
            print(a);
        }
};


//------------------------------------------------------------------------------
/// A bidirectional mapping between names and unique pointers
///
/// This class generates a sequence of unique pointers for a set of provided
/// names.  The names may be then mapped into the pointers, and the pointers
/// mapped into names via the find() function.
template<typename PtrT>
class NamePtrMapping
{
    private:
        typedef std::map<PtrT, std::string> MapTypePtoS;
        typedef std::map<std::string, PtrT> MapTypeStoP;
        MapTypePtoS m_ptrToString;
        MapTypeStoP m_stringToPtr;

    public:
        template<int n>
        NamePtrMapping(const char* (&names)[n])
        {
            // Generate a set of unique, nonzero pointers.
            for(int i = 0; i < n; ++i)
            {
                PtrT p = reinterpret_cast<PtrT>(i+1);
                m_ptrToString[p] = names[i];
                m_stringToPtr[names[i]] = p;
            }
        }

        void insert(const char* name, PtrT p)
        {
            m_ptrToString[p] = name;
            m_stringToPtr[name] = p;
        }

        const char* find(PtrT p) const
        {
            typename MapTypePtoS::const_iterator i = m_ptrToString.find(p);
            if(i == m_ptrToString.end())
                AQSIS_THROW_XQERROR(XqValidation, EqE_BadHandle, "could not find handle");
            return i->second.c_str();
        }

        PtrT find(const char* name) const
        {
            typename MapTypeStoP::const_iterator i = m_stringToPtr.find(name);
            if(i == m_stringToPtr.end())
                AQSIS_THROW_XQERROR(XqValidation, EqE_BadToken, "could not find name");
            return i->second;
        }
};


const char* filterFuncNames[8] = {
    "box", "gaussian", "triangle", "mitchell", "catmull-rom",
    "sinc", "bessel", "disk",
};
const char* errorFuncNames[] = { "ignore", "print", "abort" };
const char* subdivFuncNames[] = {
    "DelayedReadArchive", "RunProgram", "DynamicLoad"
};

RtBasis g_bezierBasis =     {{ -1.0f,  3.0f, -3.0f,  1.0f},
                             {  3.0f, -6.0f,  3.0f,  0.0f},
                             { -3.0f,  3.0f,  0.0f,  0.0f},
                             {  1.0f,  0.0f,  0.0f,  0.0f}};
RtBasis g_bSplineBasis =    {{ -1.0f/6.0f,  0.5f,      -0.5f,      1.0f/6.0f},
                             {  0.5f,       -1.0f,      0.5f,      0.0f},
                             { -0.5f,        0.0f,      0.5f,      0.0f},
                             {  1.0f/6.0f,   2.0f/3.0f, 1.0f/6.0f, 0.0f}};
RtBasis g_catmullRomBasis = {{ -0.5f,  1.5f, -1.5f,  0.5f},
                             {  1.0f, -2.5f,  2.0f, -0.5f},
                             { -0.5f,  0.0f,  0.5f,  0.0f},
                             {  0.0f,  1.0f,  0.0f,  0.0f}};
RtBasis g_hermiteBasis =    {{  2.0f,  1.0f, -2.0f,  1.0f},
                             { -3.0f, -2.0f,  3.0f, -1.0f},
                             {  0.0f,  1.0f,  0.0f,  0.0f},
                             {  1.0f,  0.0f,  0.0f,  0.0f}};
RtBasis g_powerBasis =      {{  1.0f,  0.0f,  0.0f,  0.0f},
                             {  0.0f,  1.0f,  0.0f,  0.0f},
                             {  0.0f,  0.0f,  1.0f,  0.0f},
                             {  0.0f,  0.0f,  0.0f,  1.0f}};

// Compare two bases for equality.
bool basisEqual(RtConstBasis b1, RtConstBasis b2)
{
    return std::equal(&**b1, &**b1 + 16, &**b2);
}

const char* basisName(RtConstBasis b)
{
    // Quickly check whether addresses of the first elements are equal
    if(&**g_bezierBasis     == &**b) return "bezier";
    if(&**g_bSplineBasis    == &**b) return "b-spline";
    if(&**g_catmullRomBasis == &**b) return "catmull-rom";
    if(&**g_hermiteBasis    == &**b) return "hermite";
    if(&**g_powerBasis      == &**b) return "power";
    // if not, check whether basis *values* are equal
    if(basisEqual(g_bezierBasis, b))     return "bezier";
    if(basisEqual(g_bSplineBasis, b))    return "b-spline";
    if(basisEqual(g_catmullRomBasis, b)) return "catmull-rom";
    if(basisEqual(g_hermiteBasis, b))    return "hermite";
    if(basisEqual(g_powerBasis, b))      return "power";
    // otherwise, dunno. It's nonstandard.
    return 0;
}

} // anon. namespace

//------------------------------------------------------------------------------
class RibWriterServicesImpl : public RibWriterServices
{
    private:
        /// Contained RIB writer interface
        boost::shared_ptr<Ri::Renderer> m_writer;
        /// Dictionary containing Declare()'d tokens
        TokenDict m_tokenDict;
        /// Mappings between standard function/basis names and pointers
        NamePtrMapping<RtFilterFunc> m_filterFuncMap;
        NamePtrMapping<RtErrorFunc> m_errorFuncMap;
        NamePtrMapping<RtProcSubdivFunc> m_subdivFuncMap;
        /// Parser for ReadArchive.  May be NULL (created on demand).
        boost::shared_ptr<RibParser> m_parser;
        /// Chain of filters
        std::vector<boost::shared_ptr<Ri::Renderer> > m_filterChain;
        /// Error handler.
        AqsisLogErrorHandler m_errorHandler;

    public:
        RibWriterServicesImpl()
            : m_writer(),
            m_filterFuncMap(filterFuncNames),
            m_errorFuncMap(errorFuncNames),
            m_subdivFuncMap(subdivFuncNames),
            m_parser(),
            m_errorHandler()
        { }

        //--------------------------------------------------
        // utilities for RibWriter

        // Set the contained RIB writer.  Only needed because
        // RibWriterServicesImpl must be constructed first since RibWriter
        // needs a ref to it.
        void setWriter(const boost::shared_ptr<Ri::Renderer>& writer)
            { m_writer = writer; }

        void declare(RtConstString name, RtConstString declaration)
            { m_tokenDict.declare(name, declaration); }
        const char* getFilterFuncName(RtFilterFunc f)
            { return m_filterFuncMap.find(f); }
        const char* getErrorFuncName(RtErrorFunc f)
            { return m_errorFuncMap.find(f); }
        const char* getSubdivFuncName(RtProcSubdivFunc f)
            { return m_subdivFuncMap.find(f); }

        virtual void registerFilterFunc(RtConstString name, RtFilterFunc func)
            { m_filterFuncMap.insert(name, func); }
        virtual void registerProcSubdivFunc(RtConstString name,
                                            RtProcSubdivFunc func)
            { m_subdivFuncMap.insert(name, func); }
        virtual void registerErrorFunc(RtConstString name, RtErrorFunc func)
            { m_errorFuncMap.insert(name, func); }

        //--------------------------------------------------
        // methods from RendererServices.
        virtual ErrorHandler& errorHandler()
        {
            return m_errorHandler;
        }
        virtual RtFilterFunc getFilterFunc(RtConstToken name) const
        {
            return m_filterFuncMap.find(name);
        }
        virtual RtConstBasis* getBasis(RtConstToken name) const
        {
            if     (!strcmp(name, "bezier"))      return &g_bezierBasis;
            else if(!strcmp(name, "b-spline"))    return &g_bSplineBasis;
            else if(!strcmp(name, "catmull-rom")) return &g_catmullRomBasis;
            else if(!strcmp(name, "hermite"))     return &g_hermiteBasis;
            else if(!strcmp(name, "power"))       return &g_powerBasis;
            else
            {
                AQSIS_THROW_XQERROR(XqValidation, EqE_BadToken,
                    "unknown basis \"" << name << "\"");
                return 0;
            }
        }
        virtual RtErrorFunc getErrorFunc(RtConstToken name) const
        {
            return m_errorFuncMap.find(name);
        }
        virtual RtProcSubdivFunc getProcSubdivFunc(RtConstToken name) const
        {
            return m_subdivFuncMap.find(name);
        }

        virtual Ri::TypeSpec getDeclaration(RtConstToken token,
                                        const char** nameBegin = 0,
                                        const char** nameEnd = 0) const
        {
            return m_tokenDict.lookup(token, nameBegin, nameEnd);
        }

        virtual Ri::Renderer& firstFilter()
        {
            if(!m_filterChain.empty())
                return *m_filterChain.back();
            return *m_writer;
        }

        virtual void addFilter(const char* name,
                               const Ri::ParamList& filterParams)
        {
            boost::shared_ptr<Ri::Filter> filter(
                    createFilter(name, filterParams));
            if(filter)
            {
                filter->setNextFilter(firstFilter());
                filter->setRendererServices(*this);
                m_filterChain.push_back(filter);
            }
            else
            {
                AQSIS_THROW_XQERROR(XqValidation, EqE_BadToken,
                        "filter \"" << name << "\" not found");
            }
        }

        virtual void addFilter(Ri::Filter& filter)
        {
            filter.setNextFilter(firstFilter());
            filter.setRendererServices(*this);
            m_filterChain.push_back(boost::shared_ptr<Ri::Renderer>(&filter,
                                                                    nullDeleter));
        }

        virtual void parseRib(std::istream& ribStream, const char* name,
                              Ri::Renderer& context)
        {
            if(!m_parser)
                m_parser.reset(RibParser::create(*this));
            m_parser->parseStream(ribStream, name, context);
        }
};


//------------------------------------------------------------------------------
/// Class which serializes Ri::Renderer calls to a RIB stream.
///
/// The Formatter template argument specifies a formatting class which will do
/// the job of actually formatting the RIB tokens into output characters.
template<typename Formatter>
class RibWriter : public Ri::Renderer
{
    private:
        /// gzip compressor for compressed output
        boost::shared_ptr<std::ostream> m_gzipStream;
        /// Formatting object
        Formatter m_formatter;
        /// Flag determining whether to read and insert RIB archives or not
        bool m_interpolateArchives;
        /// If true, print procedurals and free procedural data after use
        bool m_handleProcedurals;
        /// Path in which to search for archive files
        std::string m_archiveSearchPath;
        /// Renderer services for RIB writer.
        RibWriterServicesImpl& m_services;

        /// Print a parameter list to the formatter
        void printParamList(const Ri::ParamList& pList)
        {
            for(size_t i = 0; i < pList.size(); ++i)
            {
                m_formatter.whitespace();
                const Ri::Param& param = pList[i];
                // TODO try/catch for flow control is ugly - need to refactor
                // token dict to avoid this...
                bool useInlineDecl = true;
                try
                {
                    Ri::TypeSpec definedSpec =
                        m_services.getDeclaration(param.name());
                    if(definedSpec == param.spec())
                        useInlineDecl = false;
                }
                catch(XqValidation&)
                { }
                const char* token = param.name();
                std::ostringstream fmt;
                if(useInlineDecl)
                {
                    // Format inline declarations.
                    fmt << CqPrimvarToken(param.spec(), param.name());
                    token = fmt.str().c_str();
                }
                switch(param.spec().storageType())
                {
                    case Ri::TypeSpec::Float:
                        m_formatter.printParam(token, param.floatData());
                        break;
                    case Ri::TypeSpec::Integer:
                        m_formatter.printParam(token, param.intData());
                        break;
                    case Ri::TypeSpec::String:
                        m_formatter.printParam(token, param.stringData());
                        break;
                    case Ri::TypeSpec::Pointer:
                        m_services.errorHandler().warning(EqE_BadToken,
                            "Ignoring parameter \"%s\" because pointer "
                            "parameters cannot be serialized to RIB.", token);
                        break;
                    default:
                        assert(0);
                }
            }
        }

        void printBasis(RtConstBasis basis)
        {
            const char* name = basisName(basis);
            if(name)
                m_formatter.print(name);
            else
                m_formatter.print(
                    FloatArray(static_cast<const float*>(basis[0]),16)
                );
        }

        /// Allocate a gzip stream filter, if desired.
        static boost::shared_ptr<std::ostream> setupGzipStream(std::ostream& out, bool useGzip)
        {
            if(!useGzip)
                return boost::shared_ptr<std::ostream>();
#           ifdef USE_GZIPPED_RIB
            namespace io = boost::iostreams;
            boost::shared_ptr<io::filtering_stream<io::output> > gzipStream(
                new io::filtering_stream<io::output>());
            gzipStream->push(io::gzip_compressor());
            gzipStream->push(out);
            return gzipStream;
#           else
            Aqsis::log() << Aqsis::error
                << "Aqsis compiled without gzip support." << std::endl;
            return boost::shared_ptr<std::ostream>();
#           endif
        }

    public:
        RibWriter(RibWriterServicesImpl& services, std::ostream& out,
                  const RibWriterOptions& opts)
            : m_gzipStream(setupGzipStream(out, opts.useGzip)),
            m_formatter(opts.useGzip ? *m_gzipStream : out, opts),
            m_interpolateArchives(opts.interpolateArchives),
            m_handleProcedurals(opts.handleProcedurals),
            m_archiveSearchPath(opts.archivePath),
            m_services(services)
        { }

        // Code generator for autogenerated method declarations
        /*[[[cog
        from codegenutils import *

        riXml = parseXml(riXmlPath)

        for p in riXml.findall('Procedures/Procedure'):
            if p.findall('Rib'):
                decl = 'virtual %s;' % (riCxxMethodDecl(p),)
                cog.outl(wrapDecl(decl, 72, wrapIndent=20))
        ]]]*/
        virtual RtVoid Declare(RtConstString name, RtConstString declaration);
        virtual RtVoid FrameBegin(RtInt number);
        virtual RtVoid FrameEnd();
        virtual RtVoid WorldBegin();
        virtual RtVoid WorldEnd();
        virtual RtVoid IfBegin(RtConstString condition);
        virtual RtVoid ElseIf(RtConstString condition);
        virtual RtVoid Else();
        virtual RtVoid IfEnd();
        virtual RtVoid Format(RtInt xresolution, RtInt yresolution,
                            RtFloat pixelaspectratio);
        virtual RtVoid FrameAspectRatio(RtFloat frameratio);
        virtual RtVoid ScreenWindow(RtFloat left, RtFloat right, RtFloat bottom,
                            RtFloat top);
        virtual RtVoid CropWindow(RtFloat xmin, RtFloat xmax, RtFloat ymin,
                            RtFloat ymax);
        virtual RtVoid Projection(RtConstToken name, const ParamList& pList);
        virtual RtVoid Clipping(RtFloat cnear, RtFloat cfar);
        virtual RtVoid ClippingPlane(RtFloat x, RtFloat y, RtFloat z,
                            RtFloat nx, RtFloat ny, RtFloat nz);
        virtual RtVoid DepthOfField(RtFloat fstop, RtFloat focallength,
                            RtFloat focaldistance);
        virtual RtVoid Shutter(RtFloat opentime, RtFloat closetime);
        virtual RtVoid PixelVariance(RtFloat variance);
        virtual RtVoid PixelSamples(RtFloat xsamples, RtFloat ysamples);
        virtual RtVoid PixelFilter(RtFilterFunc function, RtFloat xwidth,
                            RtFloat ywidth);
        virtual RtVoid Exposure(RtFloat gain, RtFloat gamma);
        virtual RtVoid Imager(RtConstToken name, const ParamList& pList);
        virtual RtVoid Quantize(RtConstToken type, RtInt one, RtInt min,
                            RtInt max, RtFloat ditheramplitude);
        virtual RtVoid Display(RtConstToken name, RtConstToken type,
                            RtConstToken mode, const ParamList& pList);
        virtual RtVoid Hider(RtConstToken name, const ParamList& pList);
        virtual RtVoid ColorSamples(const FloatArray& nRGB,
                            const FloatArray& RGBn);
        virtual RtVoid RelativeDetail(RtFloat relativedetail);
        virtual RtVoid Option(RtConstToken name, const ParamList& pList);
        virtual RtVoid AttributeBegin();
        virtual RtVoid AttributeEnd();
        virtual RtVoid Color(RtConstColor Cq);
        virtual RtVoid Opacity(RtConstColor Os);
        virtual RtVoid TextureCoordinates(RtFloat s1, RtFloat t1, RtFloat s2,
                            RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4,
                            RtFloat t4);
        virtual RtVoid LightSource(RtConstToken shadername, RtConstToken name,
                            const ParamList& pList);
        virtual RtVoid AreaLightSource(RtConstToken shadername,
                            RtConstToken name, const ParamList& pList);
        virtual RtVoid Illuminate(RtConstToken name, RtBoolean onoff);
        virtual RtVoid Surface(RtConstToken name, const ParamList& pList);
        virtual RtVoid Displacement(RtConstToken name, const ParamList& pList);
        virtual RtVoid Atmosphere(RtConstToken name, const ParamList& pList);
        virtual RtVoid Interior(RtConstToken name, const ParamList& pList);
        virtual RtVoid Exterior(RtConstToken name, const ParamList& pList);
        virtual RtVoid ShaderLayer(RtConstToken type, RtConstToken name,
                            RtConstToken layername, const ParamList& pList);
        virtual RtVoid ConnectShaderLayers(RtConstToken type,
                            RtConstToken layer1, RtConstToken variable1,
                            RtConstToken layer2, RtConstToken variable2);
        virtual RtVoid ShadingRate(RtFloat size);
        virtual RtVoid ShadingInterpolation(RtConstToken type);
        virtual RtVoid Matte(RtBoolean onoff);
        virtual RtVoid Bound(RtConstBound bound);
        virtual RtVoid Detail(RtConstBound bound);
        virtual RtVoid DetailRange(RtFloat offlow, RtFloat onlow,
                            RtFloat onhigh, RtFloat offhigh);
        virtual RtVoid GeometricApproximation(RtConstToken type,
                            RtFloat value);
        virtual RtVoid Orientation(RtConstToken orientation);
        virtual RtVoid ReverseOrientation();
        virtual RtVoid Sides(RtInt nsides);
        virtual RtVoid Identity();
        virtual RtVoid Transform(RtConstMatrix transform);
        virtual RtVoid ConcatTransform(RtConstMatrix transform);
        virtual RtVoid Perspective(RtFloat fov);
        virtual RtVoid Translate(RtFloat dx, RtFloat dy, RtFloat dz);
        virtual RtVoid Rotate(RtFloat angle, RtFloat dx, RtFloat dy,
                            RtFloat dz);
        virtual RtVoid Scale(RtFloat sx, RtFloat sy, RtFloat sz);
        virtual RtVoid Skew(RtFloat angle, RtFloat dx1, RtFloat dy1,
                            RtFloat dz1, RtFloat dx2, RtFloat dy2,
                            RtFloat dz2);
        virtual RtVoid CoordinateSystem(RtConstToken space);
        virtual RtVoid CoordSysTransform(RtConstToken space);
        virtual RtVoid TransformBegin();
        virtual RtVoid TransformEnd();
        virtual RtVoid Resource(RtConstToken handle, RtConstToken type,
                            const ParamList& pList);
        virtual RtVoid ResourceBegin();
        virtual RtVoid ResourceEnd();
        virtual RtVoid Attribute(RtConstToken name, const ParamList& pList);
        virtual RtVoid Polygon(const ParamList& pList);
        virtual RtVoid GeneralPolygon(const IntArray& nverts,
                            const ParamList& pList);
        virtual RtVoid PointsPolygons(const IntArray& nverts,
                            const IntArray& verts, const ParamList& pList);
        virtual RtVoid PointsGeneralPolygons(const IntArray& nloops,
                            const IntArray& nverts, const IntArray& verts,
                            const ParamList& pList);
        virtual RtVoid Basis(RtConstBasis ubasis, RtInt ustep,
                            RtConstBasis vbasis, RtInt vstep);
        virtual RtVoid Patch(RtConstToken type, const ParamList& pList);
        virtual RtVoid PatchMesh(RtConstToken type, RtInt nu,
                            RtConstToken uwrap, RtInt nv, RtConstToken vwrap,
                            const ParamList& pList);
        virtual RtVoid NuPatch(RtInt nu, RtInt uorder, const FloatArray& uknot,
                            RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder,
                            const FloatArray& vknot, RtFloat vmin, RtFloat vmax,
                            const ParamList& pList);
        virtual RtVoid TrimCurve(const IntArray& ncurves, const IntArray& order,
                            const FloatArray& knot, const FloatArray& min,
                            const FloatArray& max, const IntArray& n,
                            const FloatArray& u, const FloatArray& v,
                            const FloatArray& w);
        virtual RtVoid SubdivisionMesh(RtConstToken scheme,
                            const IntArray& nvertices, const IntArray& vertices,
                            const TokenArray& tags, const IntArray& nargs,
                            const IntArray& intargs,
                            const FloatArray& floatargs,
                            const ParamList& pList);
        virtual RtVoid Sphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList);
        virtual RtVoid Cone(RtFloat height, RtFloat radius, RtFloat thetamax,
                            const ParamList& pList);
        virtual RtVoid Cylinder(RtFloat radius, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList);
        virtual RtVoid Hyperboloid(RtConstPoint point1, RtConstPoint point2,
                            RtFloat thetamax, const ParamList& pList);
        virtual RtVoid Paraboloid(RtFloat rmax, RtFloat zmin, RtFloat zmax,
                            RtFloat thetamax, const ParamList& pList);
        virtual RtVoid Disk(RtFloat height, RtFloat radius, RtFloat thetamax,
                            const ParamList& pList);
        virtual RtVoid Torus(RtFloat majorrad, RtFloat minorrad, RtFloat phimin,
                            RtFloat phimax, RtFloat thetamax,
                            const ParamList& pList);
        virtual RtVoid Points(const ParamList& pList);
        virtual RtVoid Curves(RtConstToken type, const IntArray& nvertices,
                            RtConstToken wrap, const ParamList& pList);
        virtual RtVoid Blobby(RtInt nleaf, const IntArray& code,
                            const FloatArray& floats, const TokenArray& strings,
                            const ParamList& pList);
        virtual RtVoid Procedural(RtPointer data, RtConstBound bound,
                            RtProcSubdivFunc refineproc,
                            RtProcFreeFunc freeproc);
        virtual RtVoid Geometry(RtConstToken type, const ParamList& pList);
        virtual RtVoid SolidBegin(RtConstToken type);
        virtual RtVoid SolidEnd();
        virtual RtVoid ObjectBegin(RtConstToken name);
        virtual RtVoid ObjectEnd();
        virtual RtVoid ObjectInstance(RtConstToken name);
        virtual RtVoid MotionBegin(const FloatArray& times);
        virtual RtVoid MotionEnd();
        virtual RtVoid MakeTexture(RtConstString imagefile,
                            RtConstString texturefile, RtConstToken swrap,
                            RtConstToken twrap, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList);
        virtual RtVoid MakeLatLongEnvironment(RtConstString imagefile,
                            RtConstString reflfile, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList);
        virtual RtVoid MakeCubeFaceEnvironment(RtConstString px,
                            RtConstString nx, RtConstString py,
                            RtConstString ny, RtConstString pz,
                            RtConstString nz, RtConstString reflfile,
                            RtFloat fov, RtFilterFunc filterfunc,
                            RtFloat swidth, RtFloat twidth,
                            const ParamList& pList);
        virtual RtVoid MakeShadow(RtConstString picfile,
                            RtConstString shadowfile, const ParamList& pList);
        virtual RtVoid MakeOcclusion(const StringArray& picfiles,
                            RtConstString shadowfile, const ParamList& pList);
        virtual RtVoid ErrorHandler(RtErrorFunc handler);
        virtual RtVoid ReadArchive(RtConstToken name,
                            RtArchiveCallback callback,
                            const ParamList& pList);
        virtual RtVoid ArchiveBegin(RtConstToken name, const ParamList& pList);
        virtual RtVoid ArchiveEnd();
        //[[[end]]]

        virtual RtVoid ArchiveRecord(RtConstToken type, const char* string)
        {
            m_formatter.archiveRecord(type, string);
        }
};

//--------------------------------------------------
// Custom method implementations for a few oddball requests, or requests which
// effect the state of the output object.

template<typename Formatter>
RtVoid RibWriter<Formatter>::Declare(RtConstString name, RtConstString declaration)
{
    m_services.declare(name, declaration);
    m_formatter.beginRequest("Declare");
    m_formatter.whitespace();
    m_formatter.print(name);
    m_formatter.whitespace();
    m_formatter.print(declaration);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Procedural(RtPointer data, RtConstBound bound,
                                     RtProcSubdivFunc refineproc,
                                     RtProcFreeFunc freeproc)
{
    if(!m_handleProcedurals)
        return;
    const char* name = m_services.getSubdivFuncName(refineproc);
    m_formatter.beginRequest("Procedural");
    m_formatter.whitespace();
    m_formatter.print(name);
    m_formatter.whitespace();
    if(!strcmp(name, "DelayedReadArchive"))
        m_formatter.print(StringArray(static_cast<RtToken*>(data), 1));
    else if(!strcmp(name, "RunProgram"))
        m_formatter.print(StringArray(static_cast<RtToken*>(data), 2));
    else if(!strcmp(name, "DynamicLoad"))
        m_formatter.print(StringArray(static_cast<RtToken*>(data), 2));
    m_formatter.whitespace();
    m_formatter.print(FloatArray(bound,6));
    m_formatter.endRequest();
    freeproc(data);
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Option(RtConstToken name, const ParamList& pList)
{
    m_formatter.beginRequest("Option");
    m_formatter.whitespace();
    m_formatter.print(name);
    printParamList(pList);
    m_formatter.endRequest();
    if(!strcmp(name, "searchpath"))
    {
        for(size_t i = 0; i < pList.size(); ++i)
        {
            const Ri::Param& param = pList[i];
            if(!strcmp(param.name(), "archive") &&
               param.spec() == TypeSpec::String)
            {
                // Save "archive" searchpaths so we can find archive files.
                // TODO: "resource" paths? "defaultarchive" path?
                m_archiveSearchPath = expandSearchPath(param.stringData()[0],
                                                       m_archiveSearchPath);
            }
        }
    }
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ReadArchive(RtConstToken name,
                                      RtArchiveCallback callback,
                                      const ParamList& pList)
{
    bool didRead = false;
    if(m_interpolateArchives)
    {
        // Try to find, open, and parse the archive file.  On error, print a
        // message and fall back to emitting the ReadArchive call.
        boostfs::path path = findFileNothrow(name, m_archiveSearchPath);
        if(!path.empty())
        {
            std::ifstream inputFile(native(path).c_str());
            if(inputFile)
            {
                m_services.parseRib(inputFile, name, m_services.firstFilter());
                didRead = true;
            }
        }
        if(!didRead)
        {
            Aqsis::log() << Aqsis::error << "could not ReadArchive file \""
                         << name << "\"" << std::endl;
        }
    }
    if(!didRead)
    {
        m_formatter.beginRequest("ReadArchive");
        m_formatter.whitespace();
        m_formatter.print(name);
        printParamList(pList);
        m_formatter.endRequest();
    }
}

//------------------------------------------------------------------------------
// Autogenerated method implementations

/*[[[cog
from codegenutils import *
from Cheetah.Template import Template

customImpl = set((
    'Declare',
    'Procedural',
    'ReadArchive',
    'Option',
))

formatterStatements = {
    'RtColor':       'm_formatter.printTriple(%s);',
    'RtPoint':       'm_formatter.printTriple(%s);',
    'RtMatrix':      'm_formatter.print(FloatArray(static_cast<const float*>(%s[0]),16));',
    'RtBound':       'm_formatter.print(FloatArray(%s,6));',
    'RtBasis':       'printBasis(%s);',
    'RtFilterFunc':  'm_formatter.print(m_services.getFilterFuncName(%s));',
    'RtErrorFunc':   'm_formatter.print(m_services.getErrorFuncName(%s));',
}

def formatterStatement(argXml):
    formatter = formatterStatements.get(argXml.findtext('Type'),
                                        'm_formatter.print(%s);')
    return formatter % (argXml.findtext('Name'),)

methodTemplate = '''
template<typename Formatter>
$wrapDecl($riCxxMethodDecl($proc, className='RibWriter<Formatter>'), 80)
{
#if $doDedent
    m_formatter.decreaseIndent();
#end if
    m_formatter.beginRequest("$procName");
#for $i, $arg in enumerate($args)
    m_formatter.whitespace();
    $formatterStatement($arg)
#end for
#if $proc.findall('Arguments/ParamList')
    printParamList(pList);
#end if
    m_formatter.endRequest();
#if $doIndent
    m_formatter.increaseIndent();
#end if
}
'''

for proc in riXml.findall('Procedures/Procedure'):
    if proc.findall('Rib') and proc.findtext('Name') not in customImpl:
        args = [x for x in proc.findall('Arguments/Argument')
                if not x.findall('RibValue')]
        procName = proc.findtext('Name')
        doIndent = procName.endswith('Begin')
        doDedent = procName.endswith('End')
        cog.out(str(Template(methodTemplate, searchList=locals())));

]]]*/

template<typename Formatter>
RtVoid RibWriter<Formatter>::FrameBegin(RtInt number)
{
    m_formatter.beginRequest("FrameBegin");
    m_formatter.whitespace();
    m_formatter.print(number);
    m_formatter.endRequest();
    m_formatter.increaseIndent();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::FrameEnd()
{
    m_formatter.decreaseIndent();
    m_formatter.beginRequest("FrameEnd");
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::WorldBegin()
{
    m_formatter.beginRequest("WorldBegin");
    m_formatter.endRequest();
    m_formatter.increaseIndent();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::WorldEnd()
{
    m_formatter.decreaseIndent();
    m_formatter.beginRequest("WorldEnd");
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::IfBegin(RtConstString condition)
{
    m_formatter.beginRequest("IfBegin");
    m_formatter.whitespace();
    m_formatter.print(condition);
    m_formatter.endRequest();
    m_formatter.increaseIndent();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ElseIf(RtConstString condition)
{
    m_formatter.beginRequest("ElseIf");
    m_formatter.whitespace();
    m_formatter.print(condition);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Else()
{
    m_formatter.beginRequest("Else");
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::IfEnd()
{
    m_formatter.decreaseIndent();
    m_formatter.beginRequest("IfEnd");
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Format(RtInt xresolution, RtInt yresolution,
                                    RtFloat pixelaspectratio)
{
    m_formatter.beginRequest("Format");
    m_formatter.whitespace();
    m_formatter.print(xresolution);
    m_formatter.whitespace();
    m_formatter.print(yresolution);
    m_formatter.whitespace();
    m_formatter.print(pixelaspectratio);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::FrameAspectRatio(RtFloat frameratio)
{
    m_formatter.beginRequest("FrameAspectRatio");
    m_formatter.whitespace();
    m_formatter.print(frameratio);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ScreenWindow(RtFloat left, RtFloat right,
                                          RtFloat bottom, RtFloat top)
{
    m_formatter.beginRequest("ScreenWindow");
    m_formatter.whitespace();
    m_formatter.print(left);
    m_formatter.whitespace();
    m_formatter.print(right);
    m_formatter.whitespace();
    m_formatter.print(bottom);
    m_formatter.whitespace();
    m_formatter.print(top);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::CropWindow(RtFloat xmin, RtFloat xmax,
                                        RtFloat ymin, RtFloat ymax)
{
    m_formatter.beginRequest("CropWindow");
    m_formatter.whitespace();
    m_formatter.print(xmin);
    m_formatter.whitespace();
    m_formatter.print(xmax);
    m_formatter.whitespace();
    m_formatter.print(ymin);
    m_formatter.whitespace();
    m_formatter.print(ymax);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Projection(RtConstToken name,
                                        const ParamList& pList)
{
    m_formatter.beginRequest("Projection");
    m_formatter.whitespace();
    m_formatter.print(name);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Clipping(RtFloat cnear, RtFloat cfar)
{
    m_formatter.beginRequest("Clipping");
    m_formatter.whitespace();
    m_formatter.print(cnear);
    m_formatter.whitespace();
    m_formatter.print(cfar);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ClippingPlane(RtFloat x, RtFloat y, RtFloat z,
                                           RtFloat nx, RtFloat ny, RtFloat nz)
{
    m_formatter.beginRequest("ClippingPlane");
    m_formatter.whitespace();
    m_formatter.print(x);
    m_formatter.whitespace();
    m_formatter.print(y);
    m_formatter.whitespace();
    m_formatter.print(z);
    m_formatter.whitespace();
    m_formatter.print(nx);
    m_formatter.whitespace();
    m_formatter.print(ny);
    m_formatter.whitespace();
    m_formatter.print(nz);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::DepthOfField(RtFloat fstop, RtFloat focallength,
                                          RtFloat focaldistance)
{
    m_formatter.beginRequest("DepthOfField");
    m_formatter.whitespace();
    m_formatter.print(fstop);
    m_formatter.whitespace();
    m_formatter.print(focallength);
    m_formatter.whitespace();
    m_formatter.print(focaldistance);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Shutter(RtFloat opentime, RtFloat closetime)
{
    m_formatter.beginRequest("Shutter");
    m_formatter.whitespace();
    m_formatter.print(opentime);
    m_formatter.whitespace();
    m_formatter.print(closetime);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::PixelVariance(RtFloat variance)
{
    m_formatter.beginRequest("PixelVariance");
    m_formatter.whitespace();
    m_formatter.print(variance);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::PixelSamples(RtFloat xsamples, RtFloat ysamples)
{
    m_formatter.beginRequest("PixelSamples");
    m_formatter.whitespace();
    m_formatter.print(xsamples);
    m_formatter.whitespace();
    m_formatter.print(ysamples);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::PixelFilter(RtFilterFunc function, RtFloat xwidth,
                                         RtFloat ywidth)
{
    m_formatter.beginRequest("PixelFilter");
    m_formatter.whitespace();
    m_formatter.print(m_services.getFilterFuncName(function));
    m_formatter.whitespace();
    m_formatter.print(xwidth);
    m_formatter.whitespace();
    m_formatter.print(ywidth);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Exposure(RtFloat gain, RtFloat gamma)
{
    m_formatter.beginRequest("Exposure");
    m_formatter.whitespace();
    m_formatter.print(gain);
    m_formatter.whitespace();
    m_formatter.print(gamma);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Imager(RtConstToken name, const ParamList& pList)
{
    m_formatter.beginRequest("Imager");
    m_formatter.whitespace();
    m_formatter.print(name);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Quantize(RtConstToken type, RtInt one, RtInt min,
                                      RtInt max, RtFloat ditheramplitude)
{
    m_formatter.beginRequest("Quantize");
    m_formatter.whitespace();
    m_formatter.print(type);
    m_formatter.whitespace();
    m_formatter.print(one);
    m_formatter.whitespace();
    m_formatter.print(min);
    m_formatter.whitespace();
    m_formatter.print(max);
    m_formatter.whitespace();
    m_formatter.print(ditheramplitude);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Display(RtConstToken name, RtConstToken type,
                                     RtConstToken mode, const ParamList& pList)
{
    m_formatter.beginRequest("Display");
    m_formatter.whitespace();
    m_formatter.print(name);
    m_formatter.whitespace();
    m_formatter.print(type);
    m_formatter.whitespace();
    m_formatter.print(mode);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Hider(RtConstToken name, const ParamList& pList)
{
    m_formatter.beginRequest("Hider");
    m_formatter.whitespace();
    m_formatter.print(name);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ColorSamples(const FloatArray& nRGB,
                                          const FloatArray& RGBn)
{
    m_formatter.beginRequest("ColorSamples");
    m_formatter.whitespace();
    m_formatter.print(nRGB);
    m_formatter.whitespace();
    m_formatter.print(RGBn);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::RelativeDetail(RtFloat relativedetail)
{
    m_formatter.beginRequest("RelativeDetail");
    m_formatter.whitespace();
    m_formatter.print(relativedetail);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::AttributeBegin()
{
    m_formatter.beginRequest("AttributeBegin");
    m_formatter.endRequest();
    m_formatter.increaseIndent();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::AttributeEnd()
{
    m_formatter.decreaseIndent();
    m_formatter.beginRequest("AttributeEnd");
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Color(RtConstColor Cq)
{
    m_formatter.beginRequest("Color");
    m_formatter.whitespace();
    m_formatter.printTriple(Cq);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Opacity(RtConstColor Os)
{
    m_formatter.beginRequest("Opacity");
    m_formatter.whitespace();
    m_formatter.printTriple(Os);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::TextureCoordinates(RtFloat s1, RtFloat t1,
                                                RtFloat s2, RtFloat t2,
                                                RtFloat s3, RtFloat t3,
                                                RtFloat s4, RtFloat t4)
{
    m_formatter.beginRequest("TextureCoordinates");
    m_formatter.whitespace();
    m_formatter.print(s1);
    m_formatter.whitespace();
    m_formatter.print(t1);
    m_formatter.whitespace();
    m_formatter.print(s2);
    m_formatter.whitespace();
    m_formatter.print(t2);
    m_formatter.whitespace();
    m_formatter.print(s3);
    m_formatter.whitespace();
    m_formatter.print(t3);
    m_formatter.whitespace();
    m_formatter.print(s4);
    m_formatter.whitespace();
    m_formatter.print(t4);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::LightSource(RtConstToken shadername,
                                         RtConstToken name,
                                         const ParamList& pList)
{
    m_formatter.beginRequest("LightSource");
    m_formatter.whitespace();
    m_formatter.print(shadername);
    m_formatter.whitespace();
    m_formatter.print(name);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::AreaLightSource(RtConstToken shadername,
                                             RtConstToken name,
                                             const ParamList& pList)
{
    m_formatter.beginRequest("AreaLightSource");
    m_formatter.whitespace();
    m_formatter.print(shadername);
    m_formatter.whitespace();
    m_formatter.print(name);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Illuminate(RtConstToken name, RtBoolean onoff)
{
    m_formatter.beginRequest("Illuminate");
    m_formatter.whitespace();
    m_formatter.print(name);
    m_formatter.whitespace();
    m_formatter.print(onoff);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Surface(RtConstToken name, const ParamList& pList)
{
    m_formatter.beginRequest("Surface");
    m_formatter.whitespace();
    m_formatter.print(name);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Displacement(RtConstToken name,
                                          const ParamList& pList)
{
    m_formatter.beginRequest("Displacement");
    m_formatter.whitespace();
    m_formatter.print(name);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Atmosphere(RtConstToken name,
                                        const ParamList& pList)
{
    m_formatter.beginRequest("Atmosphere");
    m_formatter.whitespace();
    m_formatter.print(name);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Interior(RtConstToken name,
                                      const ParamList& pList)
{
    m_formatter.beginRequest("Interior");
    m_formatter.whitespace();
    m_formatter.print(name);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Exterior(RtConstToken name,
                                      const ParamList& pList)
{
    m_formatter.beginRequest("Exterior");
    m_formatter.whitespace();
    m_formatter.print(name);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ShaderLayer(RtConstToken type, RtConstToken name,
                                         RtConstToken layername,
                                         const ParamList& pList)
{
    m_formatter.beginRequest("ShaderLayer");
    m_formatter.whitespace();
    m_formatter.print(type);
    m_formatter.whitespace();
    m_formatter.print(name);
    m_formatter.whitespace();
    m_formatter.print(layername);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ConnectShaderLayers(RtConstToken type,
                                                 RtConstToken layer1,
                                                 RtConstToken variable1,
                                                 RtConstToken layer2,
                                                 RtConstToken variable2)
{
    m_formatter.beginRequest("ConnectShaderLayers");
    m_formatter.whitespace();
    m_formatter.print(type);
    m_formatter.whitespace();
    m_formatter.print(layer1);
    m_formatter.whitespace();
    m_formatter.print(variable1);
    m_formatter.whitespace();
    m_formatter.print(layer2);
    m_formatter.whitespace();
    m_formatter.print(variable2);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ShadingRate(RtFloat size)
{
    m_formatter.beginRequest("ShadingRate");
    m_formatter.whitespace();
    m_formatter.print(size);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ShadingInterpolation(RtConstToken type)
{
    m_formatter.beginRequest("ShadingInterpolation");
    m_formatter.whitespace();
    m_formatter.print(type);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Matte(RtBoolean onoff)
{
    m_formatter.beginRequest("Matte");
    m_formatter.whitespace();
    m_formatter.print(onoff);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Bound(RtConstBound bound)
{
    m_formatter.beginRequest("Bound");
    m_formatter.whitespace();
    m_formatter.print(FloatArray(bound,6));
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Detail(RtConstBound bound)
{
    m_formatter.beginRequest("Detail");
    m_formatter.whitespace();
    m_formatter.print(FloatArray(bound,6));
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::DetailRange(RtFloat offlow, RtFloat onlow,
                                         RtFloat onhigh, RtFloat offhigh)
{
    m_formatter.beginRequest("DetailRange");
    m_formatter.whitespace();
    m_formatter.print(offlow);
    m_formatter.whitespace();
    m_formatter.print(onlow);
    m_formatter.whitespace();
    m_formatter.print(onhigh);
    m_formatter.whitespace();
    m_formatter.print(offhigh);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::GeometricApproximation(RtConstToken type,
                                                    RtFloat value)
{
    m_formatter.beginRequest("GeometricApproximation");
    m_formatter.whitespace();
    m_formatter.print(type);
    m_formatter.whitespace();
    m_formatter.print(value);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Orientation(RtConstToken orientation)
{
    m_formatter.beginRequest("Orientation");
    m_formatter.whitespace();
    m_formatter.print(orientation);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ReverseOrientation()
{
    m_formatter.beginRequest("ReverseOrientation");
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Sides(RtInt nsides)
{
    m_formatter.beginRequest("Sides");
    m_formatter.whitespace();
    m_formatter.print(nsides);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Identity()
{
    m_formatter.beginRequest("Identity");
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Transform(RtConstMatrix transform)
{
    m_formatter.beginRequest("Transform");
    m_formatter.whitespace();
    m_formatter.print(FloatArray(static_cast<const float*>(transform[0]),16));
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ConcatTransform(RtConstMatrix transform)
{
    m_formatter.beginRequest("ConcatTransform");
    m_formatter.whitespace();
    m_formatter.print(FloatArray(static_cast<const float*>(transform[0]),16));
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Perspective(RtFloat fov)
{
    m_formatter.beginRequest("Perspective");
    m_formatter.whitespace();
    m_formatter.print(fov);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Translate(RtFloat dx, RtFloat dy, RtFloat dz)
{
    m_formatter.beginRequest("Translate");
    m_formatter.whitespace();
    m_formatter.print(dx);
    m_formatter.whitespace();
    m_formatter.print(dy);
    m_formatter.whitespace();
    m_formatter.print(dz);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Rotate(RtFloat angle, RtFloat dx, RtFloat dy,
                                    RtFloat dz)
{
    m_formatter.beginRequest("Rotate");
    m_formatter.whitespace();
    m_formatter.print(angle);
    m_formatter.whitespace();
    m_formatter.print(dx);
    m_formatter.whitespace();
    m_formatter.print(dy);
    m_formatter.whitespace();
    m_formatter.print(dz);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Scale(RtFloat sx, RtFloat sy, RtFloat sz)
{
    m_formatter.beginRequest("Scale");
    m_formatter.whitespace();
    m_formatter.print(sx);
    m_formatter.whitespace();
    m_formatter.print(sy);
    m_formatter.whitespace();
    m_formatter.print(sz);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Skew(RtFloat angle, RtFloat dx1, RtFloat dy1,
                                  RtFloat dz1, RtFloat dx2, RtFloat dy2,
                                  RtFloat dz2)
{
    m_formatter.beginRequest("Skew");
    m_formatter.whitespace();
    m_formatter.print(angle);
    m_formatter.whitespace();
    m_formatter.print(dx1);
    m_formatter.whitespace();
    m_formatter.print(dy1);
    m_formatter.whitespace();
    m_formatter.print(dz1);
    m_formatter.whitespace();
    m_formatter.print(dx2);
    m_formatter.whitespace();
    m_formatter.print(dy2);
    m_formatter.whitespace();
    m_formatter.print(dz2);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::CoordinateSystem(RtConstToken space)
{
    m_formatter.beginRequest("CoordinateSystem");
    m_formatter.whitespace();
    m_formatter.print(space);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::CoordSysTransform(RtConstToken space)
{
    m_formatter.beginRequest("CoordSysTransform");
    m_formatter.whitespace();
    m_formatter.print(space);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::TransformBegin()
{
    m_formatter.beginRequest("TransformBegin");
    m_formatter.endRequest();
    m_formatter.increaseIndent();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::TransformEnd()
{
    m_formatter.decreaseIndent();
    m_formatter.beginRequest("TransformEnd");
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Resource(RtConstToken handle, RtConstToken type,
                                      const ParamList& pList)
{
    m_formatter.beginRequest("Resource");
    m_formatter.whitespace();
    m_formatter.print(handle);
    m_formatter.whitespace();
    m_formatter.print(type);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ResourceBegin()
{
    m_formatter.beginRequest("ResourceBegin");
    m_formatter.endRequest();
    m_formatter.increaseIndent();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ResourceEnd()
{
    m_formatter.decreaseIndent();
    m_formatter.beginRequest("ResourceEnd");
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Attribute(RtConstToken name,
                                       const ParamList& pList)
{
    m_formatter.beginRequest("Attribute");
    m_formatter.whitespace();
    m_formatter.print(name);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Polygon(const ParamList& pList)
{
    m_formatter.beginRequest("Polygon");
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::GeneralPolygon(const IntArray& nverts,
                                            const ParamList& pList)
{
    m_formatter.beginRequest("GeneralPolygon");
    m_formatter.whitespace();
    m_formatter.print(nverts);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::PointsPolygons(const IntArray& nverts,
                                            const IntArray& verts,
                                            const ParamList& pList)
{
    m_formatter.beginRequest("PointsPolygons");
    m_formatter.whitespace();
    m_formatter.print(nverts);
    m_formatter.whitespace();
    m_formatter.print(verts);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::PointsGeneralPolygons(const IntArray& nloops,
                                                   const IntArray& nverts,
                                                   const IntArray& verts,
                                                   const ParamList& pList)
{
    m_formatter.beginRequest("PointsGeneralPolygons");
    m_formatter.whitespace();
    m_formatter.print(nloops);
    m_formatter.whitespace();
    m_formatter.print(nverts);
    m_formatter.whitespace();
    m_formatter.print(verts);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Basis(RtConstBasis ubasis, RtInt ustep,
                                   RtConstBasis vbasis, RtInt vstep)
{
    m_formatter.beginRequest("Basis");
    m_formatter.whitespace();
    printBasis(ubasis);
    m_formatter.whitespace();
    m_formatter.print(ustep);
    m_formatter.whitespace();
    printBasis(vbasis);
    m_formatter.whitespace();
    m_formatter.print(vstep);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Patch(RtConstToken type, const ParamList& pList)
{
    m_formatter.beginRequest("Patch");
    m_formatter.whitespace();
    m_formatter.print(type);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::PatchMesh(RtConstToken type, RtInt nu,
                                       RtConstToken uwrap, RtInt nv,
                                       RtConstToken vwrap,
                                       const ParamList& pList)
{
    m_formatter.beginRequest("PatchMesh");
    m_formatter.whitespace();
    m_formatter.print(type);
    m_formatter.whitespace();
    m_formatter.print(nu);
    m_formatter.whitespace();
    m_formatter.print(uwrap);
    m_formatter.whitespace();
    m_formatter.print(nv);
    m_formatter.whitespace();
    m_formatter.print(vwrap);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::NuPatch(RtInt nu, RtInt uorder,
                                     const FloatArray& uknot, RtFloat umin,
                                     RtFloat umax, RtInt nv, RtInt vorder,
                                     const FloatArray& vknot, RtFloat vmin,
                                     RtFloat vmax, const ParamList& pList)
{
    m_formatter.beginRequest("NuPatch");
    m_formatter.whitespace();
    m_formatter.print(nu);
    m_formatter.whitespace();
    m_formatter.print(uorder);
    m_formatter.whitespace();
    m_formatter.print(uknot);
    m_formatter.whitespace();
    m_formatter.print(umin);
    m_formatter.whitespace();
    m_formatter.print(umax);
    m_formatter.whitespace();
    m_formatter.print(nv);
    m_formatter.whitespace();
    m_formatter.print(vorder);
    m_formatter.whitespace();
    m_formatter.print(vknot);
    m_formatter.whitespace();
    m_formatter.print(vmin);
    m_formatter.whitespace();
    m_formatter.print(vmax);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::TrimCurve(const IntArray& ncurves,
                                       const IntArray& order,
                                       const FloatArray& knot,
                                       const FloatArray& min,
                                       const FloatArray& max, const IntArray& n,
                                       const FloatArray& u, const FloatArray& v,
                                       const FloatArray& w)
{
    m_formatter.beginRequest("TrimCurve");
    m_formatter.whitespace();
    m_formatter.print(ncurves);
    m_formatter.whitespace();
    m_formatter.print(order);
    m_formatter.whitespace();
    m_formatter.print(knot);
    m_formatter.whitespace();
    m_formatter.print(min);
    m_formatter.whitespace();
    m_formatter.print(max);
    m_formatter.whitespace();
    m_formatter.print(n);
    m_formatter.whitespace();
    m_formatter.print(u);
    m_formatter.whitespace();
    m_formatter.print(v);
    m_formatter.whitespace();
    m_formatter.print(w);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::SubdivisionMesh(RtConstToken scheme,
                                             const IntArray& nvertices,
                                             const IntArray& vertices,
                                             const TokenArray& tags,
                                             const IntArray& nargs,
                                             const IntArray& intargs,
                                             const FloatArray& floatargs,
                                             const ParamList& pList)
{
    m_formatter.beginRequest("SubdivisionMesh");
    m_formatter.whitespace();
    m_formatter.print(scheme);
    m_formatter.whitespace();
    m_formatter.print(nvertices);
    m_formatter.whitespace();
    m_formatter.print(vertices);
    m_formatter.whitespace();
    m_formatter.print(tags);
    m_formatter.whitespace();
    m_formatter.print(nargs);
    m_formatter.whitespace();
    m_formatter.print(intargs);
    m_formatter.whitespace();
    m_formatter.print(floatargs);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Sphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
                                    RtFloat thetamax, const ParamList& pList)
{
    m_formatter.beginRequest("Sphere");
    m_formatter.whitespace();
    m_formatter.print(radius);
    m_formatter.whitespace();
    m_formatter.print(zmin);
    m_formatter.whitespace();
    m_formatter.print(zmax);
    m_formatter.whitespace();
    m_formatter.print(thetamax);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Cone(RtFloat height, RtFloat radius,
                                  RtFloat thetamax, const ParamList& pList)
{
    m_formatter.beginRequest("Cone");
    m_formatter.whitespace();
    m_formatter.print(height);
    m_formatter.whitespace();
    m_formatter.print(radius);
    m_formatter.whitespace();
    m_formatter.print(thetamax);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Cylinder(RtFloat radius, RtFloat zmin,
                                      RtFloat zmax, RtFloat thetamax,
                                      const ParamList& pList)
{
    m_formatter.beginRequest("Cylinder");
    m_formatter.whitespace();
    m_formatter.print(radius);
    m_formatter.whitespace();
    m_formatter.print(zmin);
    m_formatter.whitespace();
    m_formatter.print(zmax);
    m_formatter.whitespace();
    m_formatter.print(thetamax);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Hyperboloid(RtConstPoint point1,
                                         RtConstPoint point2, RtFloat thetamax,
                                         const ParamList& pList)
{
    m_formatter.beginRequest("Hyperboloid");
    m_formatter.whitespace();
    m_formatter.printTriple(point1);
    m_formatter.whitespace();
    m_formatter.printTriple(point2);
    m_formatter.whitespace();
    m_formatter.print(thetamax);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Paraboloid(RtFloat rmax, RtFloat zmin,
                                        RtFloat zmax, RtFloat thetamax,
                                        const ParamList& pList)
{
    m_formatter.beginRequest("Paraboloid");
    m_formatter.whitespace();
    m_formatter.print(rmax);
    m_formatter.whitespace();
    m_formatter.print(zmin);
    m_formatter.whitespace();
    m_formatter.print(zmax);
    m_formatter.whitespace();
    m_formatter.print(thetamax);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Disk(RtFloat height, RtFloat radius,
                                  RtFloat thetamax, const ParamList& pList)
{
    m_formatter.beginRequest("Disk");
    m_formatter.whitespace();
    m_formatter.print(height);
    m_formatter.whitespace();
    m_formatter.print(radius);
    m_formatter.whitespace();
    m_formatter.print(thetamax);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Torus(RtFloat majorrad, RtFloat minorrad,
                                   RtFloat phimin, RtFloat phimax,
                                   RtFloat thetamax, const ParamList& pList)
{
    m_formatter.beginRequest("Torus");
    m_formatter.whitespace();
    m_formatter.print(majorrad);
    m_formatter.whitespace();
    m_formatter.print(minorrad);
    m_formatter.whitespace();
    m_formatter.print(phimin);
    m_formatter.whitespace();
    m_formatter.print(phimax);
    m_formatter.whitespace();
    m_formatter.print(thetamax);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Points(const ParamList& pList)
{
    m_formatter.beginRequest("Points");
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Curves(RtConstToken type,
                                    const IntArray& nvertices,
                                    RtConstToken wrap, const ParamList& pList)
{
    m_formatter.beginRequest("Curves");
    m_formatter.whitespace();
    m_formatter.print(type);
    m_formatter.whitespace();
    m_formatter.print(nvertices);
    m_formatter.whitespace();
    m_formatter.print(wrap);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Blobby(RtInt nleaf, const IntArray& code,
                                    const FloatArray& floats,
                                    const TokenArray& strings,
                                    const ParamList& pList)
{
    m_formatter.beginRequest("Blobby");
    m_formatter.whitespace();
    m_formatter.print(nleaf);
    m_formatter.whitespace();
    m_formatter.print(code);
    m_formatter.whitespace();
    m_formatter.print(floats);
    m_formatter.whitespace();
    m_formatter.print(strings);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::Geometry(RtConstToken type,
                                      const ParamList& pList)
{
    m_formatter.beginRequest("Geometry");
    m_formatter.whitespace();
    m_formatter.print(type);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::SolidBegin(RtConstToken type)
{
    m_formatter.beginRequest("SolidBegin");
    m_formatter.whitespace();
    m_formatter.print(type);
    m_formatter.endRequest();
    m_formatter.increaseIndent();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::SolidEnd()
{
    m_formatter.decreaseIndent();
    m_formatter.beginRequest("SolidEnd");
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ObjectBegin(RtConstToken name)
{
    m_formatter.beginRequest("ObjectBegin");
    m_formatter.whitespace();
    m_formatter.print(name);
    m_formatter.endRequest();
    m_formatter.increaseIndent();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ObjectEnd()
{
    m_formatter.decreaseIndent();
    m_formatter.beginRequest("ObjectEnd");
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ObjectInstance(RtConstToken name)
{
    m_formatter.beginRequest("ObjectInstance");
    m_formatter.whitespace();
    m_formatter.print(name);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::MotionBegin(const FloatArray& times)
{
    m_formatter.beginRequest("MotionBegin");
    m_formatter.whitespace();
    m_formatter.print(times);
    m_formatter.endRequest();
    m_formatter.increaseIndent();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::MotionEnd()
{
    m_formatter.decreaseIndent();
    m_formatter.beginRequest("MotionEnd");
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::MakeTexture(RtConstString imagefile,
                                         RtConstString texturefile,
                                         RtConstToken swrap, RtConstToken twrap,
                                         RtFilterFunc filterfunc,
                                         RtFloat swidth, RtFloat twidth,
                                         const ParamList& pList)
{
    m_formatter.beginRequest("MakeTexture");
    m_formatter.whitespace();
    m_formatter.print(imagefile);
    m_formatter.whitespace();
    m_formatter.print(texturefile);
    m_formatter.whitespace();
    m_formatter.print(swrap);
    m_formatter.whitespace();
    m_formatter.print(twrap);
    m_formatter.whitespace();
    m_formatter.print(m_services.getFilterFuncName(filterfunc));
    m_formatter.whitespace();
    m_formatter.print(swidth);
    m_formatter.whitespace();
    m_formatter.print(twidth);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::MakeLatLongEnvironment(RtConstString imagefile,
                                                    RtConstString reflfile,
                                                    RtFilterFunc filterfunc,
                                                    RtFloat swidth,
                                                    RtFloat twidth,
                                                    const ParamList& pList)
{
    m_formatter.beginRequest("MakeLatLongEnvironment");
    m_formatter.whitespace();
    m_formatter.print(imagefile);
    m_formatter.whitespace();
    m_formatter.print(reflfile);
    m_formatter.whitespace();
    m_formatter.print(m_services.getFilterFuncName(filterfunc));
    m_formatter.whitespace();
    m_formatter.print(swidth);
    m_formatter.whitespace();
    m_formatter.print(twidth);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::MakeCubeFaceEnvironment(RtConstString px,
                                                     RtConstString nx,
                                                     RtConstString py,
                                                     RtConstString ny,
                                                     RtConstString pz,
                                                     RtConstString nz,
                                                     RtConstString reflfile,
                                                     RtFloat fov,
                                                     RtFilterFunc filterfunc,
                                                     RtFloat swidth,
                                                     RtFloat twidth,
                                                     const ParamList& pList)
{
    m_formatter.beginRequest("MakeCubeFaceEnvironment");
    m_formatter.whitespace();
    m_formatter.print(px);
    m_formatter.whitespace();
    m_formatter.print(nx);
    m_formatter.whitespace();
    m_formatter.print(py);
    m_formatter.whitespace();
    m_formatter.print(ny);
    m_formatter.whitespace();
    m_formatter.print(pz);
    m_formatter.whitespace();
    m_formatter.print(nz);
    m_formatter.whitespace();
    m_formatter.print(reflfile);
    m_formatter.whitespace();
    m_formatter.print(fov);
    m_formatter.whitespace();
    m_formatter.print(m_services.getFilterFuncName(filterfunc));
    m_formatter.whitespace();
    m_formatter.print(swidth);
    m_formatter.whitespace();
    m_formatter.print(twidth);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::MakeShadow(RtConstString picfile,
                                        RtConstString shadowfile,
                                        const ParamList& pList)
{
    m_formatter.beginRequest("MakeShadow");
    m_formatter.whitespace();
    m_formatter.print(picfile);
    m_formatter.whitespace();
    m_formatter.print(shadowfile);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::MakeOcclusion(const StringArray& picfiles,
                                           RtConstString shadowfile,
                                           const ParamList& pList)
{
    m_formatter.beginRequest("MakeOcclusion");
    m_formatter.whitespace();
    m_formatter.print(picfiles);
    m_formatter.whitespace();
    m_formatter.print(shadowfile);
    printParamList(pList);
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ErrorHandler(RtErrorFunc handler)
{
    m_formatter.beginRequest("ErrorHandler");
    m_formatter.whitespace();
    m_formatter.print(m_services.getErrorFuncName(handler));
    m_formatter.endRequest();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ArchiveBegin(RtConstToken name,
                                          const ParamList& pList)
{
    m_formatter.beginRequest("ArchiveBegin");
    m_formatter.whitespace();
    m_formatter.print(name);
    printParamList(pList);
    m_formatter.endRequest();
    m_formatter.increaseIndent();
}

template<typename Formatter>
RtVoid RibWriter<Formatter>::ArchiveEnd()
{
    m_formatter.decreaseIndent();
    m_formatter.beginRequest("ArchiveEnd");
    m_formatter.endRequest();
}
//[[[end]]]


//------------------------------------------------------------------------------
/// Create an object which serializes Ri::Renderer calls into a RIB stream.
RibWriterServices* createRibWriter(std::ostream& out,
                                   const RibWriterOptions& opts)
{
    RibWriterServicesImpl* services = new RibWriterServicesImpl();
    boost::shared_ptr<Ri::Renderer> writer;
    if(opts.useBinary)
        writer.reset(new RibWriter<BinaryFormatter>(*services, out, opts));
    else
        writer.reset(new RibWriter<AsciiFormatter>(*services, out, opts));
    services->setWriter(writer);
    return services;
}


} // namespace Aqsis
// vi: set et:
