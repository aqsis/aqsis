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
 *  \brief This dictionary can handle inline declaration as described in RiSpec V3.2
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#ifdef DEBUG
#include <iostream.h>
#include <iomanip.h>
#endif
#include "dictionary.h"
#include "inlineparse.h"
#include "error.h"

using namespace libri2rib;

#ifdef DEBUG
using std::cout;
using std::endl;

void SqTokenEntry::printClassType ()
{
	cout << setw( 9 );
	switch ( tclass )
	{
			case CONSTANT: cout << "CONSTANT"; break;
			case UNIFORM: cout << "UNIFORM"; break;
			case VARYING: cout << "VARYING"; break;
			case VERTEX: cout << "VERTEX";
	}
	cout << setw( 8 );
	switch ( ttype )
	{
			case HPOINT: cout << "HPOINT"; break;
			case MATRIX: cout << "MATRIX"; break;
			case NORMAL: cout << "NORMAL"; break;
			case VECTOR: cout << "VECTOR"; break;
			case FLOAT: cout << "FLOAT"; break;
			case INTEGER: cout << "INTEGER"; break;
			case STRING: cout << "STRING"; break;
			case POINT: cout << "POINT"; break;
			case COLOR: cout << "COLOR";
	}
}
#endif



CqDictionary::CqDictionary()
{
	//=== Standard Geometric Primitive Variables ===
	addToken ( RI_P, VERTEX, POINT );
	addToken ( RI_PZ, VERTEX, FLOAT );
	addToken ( RI_PW, VERTEX, HPOINT );
	addToken ( RI_N, VARYING, NORMAL );
	addToken ( RI_NP, UNIFORM, NORMAL );
	addToken ( RI_CS, VARYING, COLOR );
	addToken ( RI_OS, VARYING, COLOR );
	addToken ( RI_S, VARYING, FLOAT );
	addToken ( RI_T, VARYING, FLOAT );
	addToken ( RI_ST, VARYING, FLOAT, 2 );

	//=== Standard Light Source Shader Parameters ===
	addToken ( RI_INTENSITY, CONSTANT, FLOAT );
	addToken ( RI_LIGHTCOLOR, CONSTANT, COLOR );
	addToken ( RI_FROM, CONSTANT, POINT );
	addToken ( RI_TO, CONSTANT, POINT );
	addToken ( RI_CONEANGLE, CONSTANT, FLOAT );
	addToken ( RI_CONEDELTAANGLE, CONSTANT, FLOAT );
	addToken ( RI_BEAMDISTRIBUTION, CONSTANT, FLOAT );

	//=== Standard Surface Shader Parameters ===
	addToken ( RI_KA, CONSTANT, FLOAT );
	addToken ( RI_KD, CONSTANT, FLOAT );
	addToken ( RI_KS, CONSTANT, FLOAT );
	addToken ( RI_KR, CONSTANT, FLOAT );
	addToken ( RI_ROUGHNESS, CONSTANT, FLOAT );
	addToken ( RI_SPECULARCOLOR, CONSTANT, COLOR );
	addToken ( RI_TEXTURENAME, CONSTANT, STRING );

	//=== Standard Volume Shader Parameters ===
	addToken ( RI_MINDISTANCE, CONSTANT, FLOAT );
	addToken ( RI_MAXDISTANCE, CONSTANT, FLOAT );
	addToken ( RI_BACKGROUND, CONSTANT, COLOR );
	addToken ( RI_DISTANCE, CONSTANT, FLOAT );

	//=== Standard Displacement Shader Parameter ===
	addToken ( RI_AMPLITUDE, CONSTANT, FLOAT );

	addToken ( RI_FOV, CONSTANT, FLOAT );
	//addToken (RI_ORIGIN, CONSTANT, INTEGER, 2);
	addToken ( RI_WIDTH, VARYING, FLOAT );
	addToken ( RI_CONSTANTWIDTH, CONSTANT, FLOAT );
}


// If the token already exists, addToken() return the corresponding id
TqTokenId CqDictionary::addToken ( std::string n, EqTokenClass tc, EqTokenType tt, TqUint qnt, TqBool inln )
{
	std::vector<SqTokenEntry>::iterator first = te.begin();
	TqTokenId i;

	for ( i = 1;first != te.end();first++, i++ )
	{
		if ( ( n == first->name ) &&
		        ( tc == first->tclass ) &&
		        ( tt == first->ttype ) &&
		        ( qnt == first->quantity ) )
			return i;
	}

	te.push_back( SqTokenEntry ( n, tc, tt, qnt, inln ) );
	return i;
}

// If the token given as input is in fact an inline definition,
// getTokenId() will add it to the dictionary.
TqTokenId CqDictionary::getTokenId ( std::string n )
{
	CqInlineParse ip;
	TqTokenId i, j = 0;

	ip.parse( n );
	if ( ip.isInline() == TqTrue )
	{
		j = addToken ( ip.getIdentifier(), ip.getClass(), ip.getType(), ip.getQuantity(), TqTrue );
	}
	else
	{
		std::vector<SqTokenEntry>::iterator first = te.begin();

		for ( i = 1;first != te.end();first++, i++ )
		{
			if ( ( n == first->name ) && ( first->in_line == TqFalse ) )
				j = i;
		}
		if ( j == 0 )
		{
			std::string st( "Token not declared: " );
			st += n;
			throw CqError( RIE_ILLSTATE, RIE_ERROR, st, TqFalse );
		}
	}
	return j;
}


TqUint CqDictionary::allocSize ( TqTokenId id, TqUint vertex, TqUint varying, TqUint uniform )
{
	TqUint size;
	std::vector<SqTokenEntry>::iterator first = te.begin();
	first += id - 1;
	size = getTypeSize( first->ttype );
	switch ( first->tclass )
	{
			case VERTEX: size *= vertex;
			break;
			case VARYING: size *= varying;
			break;
			case UNIFORM: size *= uniform;
			break;
			case CONSTANT:
			break;
	}
	size *= ( first->quantity );
	return size;
}

TqUint CqDictionary::getTypeSize ( EqTokenType t )
{
	switch ( t )
	{
			case FLOAT: return 1;
			case POINT: return 3;
			case VECTOR: return 3;
			case NORMAL: return 3;
			case COLOR: return 3;
			case STRING: return 1;
			case MATRIX: return 16;
			case HPOINT: return 4;
			case INTEGER: return 1;
			default :
			throw CqError( RIE_BUG, RIE_SEVERE,
			               "CqDictionary::getTypeSize(TokenType) --> Unknown token type", TqFalse );
	}
}

void CqDictionary::isValid ( TqTokenId id )
{
	if ( id > te.size() || id == 0 )
		throw CqError( RIE_BUG, RIE_SEVERE,
		               "CqDictionary::isValid(TokenId) --> Bad ID", TqFalse );
}

EqTokenType CqDictionary::getType ( TqTokenId id )
{
	isValid ( id );
	std::vector<SqTokenEntry>::iterator first = te.begin();
	return ( ( first + id - 1 ) ->ttype );
}

TqUint CqDictionary::getQuantity ( TqTokenId id )
{
	isValid ( id );
	std::vector<SqTokenEntry>::iterator first = te.begin();
	return ( ( first + id - 1 ) ->quantity );
}

#ifdef DEBUG
void CqDictionary::stats ()
{
	std::vector<SqTokenEntry>::iterator first = te.begin();
	TqTokenId i;

	cout << endl;
	cout << "Dictionary   Number of entries: " << te.size() << endl;
	cout << "------------------------------------------------------" << endl;
	cout << "NAME                  CLASS    TYPE   [SIZE] IS_INLINE" << endl;
	cout << "------------------------------------------------------" << endl;

	for ( i = 1;first != te.end();first++, i++ )
	{
		cout << std::setw( 20 ) << std::setfill ( ' ' );
		cout << std::setiosflags( ios::left ) << ( first->name ).c_str() << "  ";
		first->printClassType ();
		cout << "[" << getQuantity( i ) << "]  ";
		if ( ( first->in_line ) == TqTrue )
		{
			cout << " inline";
		}
		cout << endl;
	}
	cout << "------------------------------------------------------" << endl;
	cout << endl;
}
#endif
