// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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

/** \file
		\brief Implements storage classes for passing data around the parser
		\author Tim Shead (tshead@k-3d.com)
*/

#ifndef LIBRIBTYPES_H
#define LIBRIBTYPES_H

#include "librib.h"
using namespace librib;

//#include <string>
#include <vector> 
//#include <cassert>

namespace librib
{

/// Defines standard Renderman Interface token-value types
typedef enum
{
    Type_Unknown = 0,

    Type_Float = 1,
    Type_Integer,
    Type_Point,
    Type_String,
    Type_Color,
    Type_Triple,
    Type_hPoint,
    Type_Normal,
    Type_Vector,
    Type_Void,
    Type_Matrix,
    Type_HexTuple,
    Type_Last,

    Type_Uniform = 0x8000,
    Type_Varying = 0x4000,
    Type_Vertex = 0x2000,

    Type_Drop = 0x0800,
    Type_Variable = 0x0400,
    Type_Param = 0x0200,
    Type_Array = 0x0100,

    Type_Mask = 0x00FF,
    Storage_Mask = 0xF000,
    Usage_Mask = 0x0F00,

    Storage_Shift = 12,
    Usage_Shift = 8,
} ParameterType;

/// Abstract interface for Renderman Interface arrays
class Array
{
public:
    virtual ~Array()
    {}

    virtual ParameterType Type() = 0;
    virtual RendermanInterface::RtInt Count() = 0;
    virtual RendermanInterface::RtPointer Values() = 0;
};

/// Encapsulates a Renderman Interface array of integers
class IntegerArray :
            public Array,
            public std::vector<RendermanInterface::RtInt>
{
public:
    IntegerArray()
    {}

    IntegerArray( const RendermanInterface::RtInt Value )
    {
        push_back( Value );
    }

    ParameterType Type()
    {
        return Type_Integer;
    }

    RendermanInterface::RtInt Count()
    {
        return size();
    }

    RendermanInterface::RtPointer Values()
    {
        return reinterpret_cast<RendermanInterface::RtPointer>( size() ? &front() : 0 );
    }
};

/// Encapsulates a Renderman Interface array of floats
class FloatArray :
            public Array,
            public std::vector<RendermanInterface::RtFloat>
{
public:
    FloatArray()
    {}

    FloatArray( const RendermanInterface::RtFloat Value )
    {
        push_back( Value );
    }

    FloatArray( IntegerArray* const Values )
    {
        for ( unsigned int i = 0; i < Values->size(); i++ )
            push_back( static_cast<RendermanInterface::RtFloat>( ( *Values ) [ i ] ) );
    }

    ParameterType Type()
    {
        return Type_Float;
    }

    RendermanInterface::RtInt Count()
    {
        return size();
    }

    RendermanInterface::RtPointer Values()
    {
        return reinterpret_cast<RendermanInterface::RtPointer>( size() ? &front() : 0 );
    }
};

/// Encapsulates a Renderman Interface array of strings
class StringArray :
            public Array,
            public std::vector<RendermanInterface::RtString>
{
public:
    StringArray()
    {}

    StringArray( const RendermanInterface::RtString Value )
    {
        push_back( Value );
    }

    ~StringArray()
    {
        for ( iterator string = begin(); string != end(); string++ )
            delete[] ( *string );
    }

    ParameterType Type()
    {
        return Type_String;
    }

    RendermanInterface::RtInt Count()
    {
        return size();
    }

    RendermanInterface::RtPointer Values()
    {
        return reinterpret_cast<RendermanInterface::RtPointer>( size() ? &front() : 0 );
    }
};

/// Encapsulates a Renderman Interface token-value pair
class TokenValuePair
{
public:
    TokenValuePair() :
            m_Token( 0 ),
            m_Array( 0 )
    {}

    TokenValuePair( RendermanInterface::RtToken Token, Array* const Values ) :
            m_Token( Token ),
            m_Array( Values )
    {}

    ~TokenValuePair()
    {
        delete[] m_Token;
        delete m_Array;
    }

    RendermanInterface::RtInt Count()
    {
        return m_Array->Count();
    }
    RendermanInterface::RtToken Token()
    {
        return m_Token;
    }
    RendermanInterface::RtPointer Values()
    {
        return m_Array->Values();
    }

private:
    RendermanInterface::RtToken m_Token;
    Array* const m_Array;
};

/// Encapsulates a set of Renderman Interface token-value pairs
class TokenValuePairs
{
public:
    TokenValuePairs()
    {}

    TokenValuePairs( TokenValuePair* const Pair )
    {
        AddPair( Pair );
    }

    ~TokenValuePairs()
    {
        for ( unsigned int i = 0; i < m_TokenValuePairs.size(); i++ )
            delete m_TokenValuePairs[ i ];
    }

    void AddPair( TokenValuePair* const Pair )
    {
        if ( 0 == Pair )
            return ;

        m_Counts.push_back( Pair->Count() );
        m_Tokens.push_back( Pair->Token() );
        m_Values.push_back( Pair->Values() );

        m_TokenValuePairs.push_back( Pair );
    }

    RendermanInterface::RtInt Count()
    {
        return m_Tokens.size();
    }
    RendermanInterface::RtInt* Counts()
    {
        return m_Counts.size() ? & m_Counts[ 0 ] : 0;
    }
    RendermanInterface::RtToken* Tokens()
    {
        return m_Tokens.size() ? &m_Tokens[ 0 ] : 0;
    }
    RendermanInterface::RtPointer* Values()
    {
        return m_Values.size() ? &m_Values[ 0 ] : 0;
    }

private:
    std::vector<RendermanInterface::RtInt> m_Counts;
    std::vector<RendermanInterface::RtToken> m_Tokens;
    std::vector<RendermanInterface::RtPointer> m_Values;

    std::vector<TokenValuePair*> m_TokenValuePairs;
};

}
; // namespace librib

#endif // LIBRIBTYPES_H
