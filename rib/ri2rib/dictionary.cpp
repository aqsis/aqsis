// Aqsis
// Copyright  1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
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

USING_NAMESPACE( libri2rib );

#ifdef DEBUG
using std::cout;
using std::endl;

void SqTokenEntry::printClassType ()
{
	cout << setw( 9 );
	switch ( tclass )
	{
			case CONSTANT:
			cout << "CONSTANT";
			break;
			case UNIFORM:
			cout << "UNIFORM";
			break;
			case VARYING:
			cout << "VARYING";
			break;
			case VERTEX:
			cout << "VERTEX";
			break;
			case FACEVARYING:
			cout <<"FACEVARYING";
	}
	cout << setw( 8 );
	switch ( ttype )
	{
			case HPOINT:
			cout << "HPOINT";
			break;
			case MATRIX:
			cout << "MATRIX";
			break;
			case NORMAL:
			cout << "NORMAL";
			break;
			case VECTOR:
			cout << "VECTOR";
			break;
			case FLOAT:
			cout << "FLOAT";
			break;
			case INTEGER:
			cout << "INTEGER";
			break;
			case STRING:
			cout << "STRING";
			break;
			case POINT:
			cout << "POINT";
			break;
			case COLOR:
			cout << "COLOR";
	}
}
#endif



CqDictionary::CqDictionary()
{
	addToken( RI_KA, UNIFORM, FLOAT );
	addToken( RI_KD, UNIFORM, FLOAT );
	addToken( RI_KS, UNIFORM, FLOAT );
	addToken( RI_KR, UNIFORM, FLOAT );
	addToken( RI_ROUGHNESS, UNIFORM, FLOAT );
	addToken( RI_TEXTURENAME, UNIFORM, STRING );
	addToken( RI_SPECULARCOLOR, UNIFORM, COLOR );
	addToken( RI_INTENSITY, UNIFORM, FLOAT );
	addToken( RI_LIGHTCOLOR, UNIFORM, COLOR );
	addToken( RI_FROM, UNIFORM, POINT );
	addToken( RI_TO, UNIFORM, POINT );
	addToken( RI_CONEANGLE, UNIFORM, FLOAT );
	addToken( RI_CONEDELTAANGLE, UNIFORM, FLOAT );
	addToken( RI_BEAMDISTRIBUTION, UNIFORM, FLOAT );
	addToken( RI_MINDISTANCE, UNIFORM, FLOAT );
	addToken( RI_MAXDISTANCE, UNIFORM, FLOAT );
	addToken( RI_DISTANCE, UNIFORM, FLOAT );
	addToken( RI_BACKGROUND, UNIFORM, COLOR );
	addToken( RI_FOV, UNIFORM, FLOAT );
	addToken( RI_P, VERTEX, POINT );
	addToken( RI_PZ, VERTEX, POINT );
	addToken( RI_PW, VERTEX, HPOINT );
	addToken( RI_N, VARYING, NORMAL );
	addToken( RI_NP, UNIFORM, NORMAL );
	addToken( RI_CS, VARYING, COLOR );
	addToken( RI_OS, VARYING, COLOR );
	addToken( RI_S, VARYING, FLOAT );
	addToken( RI_T, VARYING, FLOAT );
	addToken( RI_ST, VARYING, FLOAT, 2 );

	addToken ( RI_AMPLITUDE, UNIFORM, FLOAT );
	addToken ( RI_WIDTH, VARYING, FLOAT );
	addToken ( RI_CONSTANTWIDTH, CONSTANT, FLOAT );

	addToken( "gridsize", UNIFORM, INTEGER );
	addToken( "texturememory", UNIFORM, INTEGER );
	addToken( "bucketsize", UNIFORM, INTEGER, 2 );
	addToken( "eyesplits", UNIFORM, INTEGER );
	addToken( RI_SHADER, UNIFORM, STRING );
	addToken( "archive", UNIFORM, STRING );
	addToken( "texture", UNIFORM, STRING );
	addToken( "display", UNIFORM, STRING );
	addToken( "auto_shadows", UNIFORM, STRING );
	addToken( "endofframe", UNIFORM, INTEGER );
	addToken( "sphere", UNIFORM, FLOAT );
	addToken( "coordinatesystem", UNIFORM, STRING );
	addToken( "shadows", UNIFORM, STRING );
	addToken( "shadowmapsize", UNIFORM, INTEGER, 2 );
	addToken( "shadowangle", UNIFORM, FLOAT );
	addToken( "shadowmapname", UNIFORM, STRING );
	addToken( "shadow_shadingrate", UNIFORM, FLOAT );
	addToken( RI_NAME, UNIFORM, STRING );
	addToken( "shadinggroup", UNIFORM, STRING );
	addToken( "sense", UNIFORM, STRING );
	addToken( "compression", UNIFORM, STRING );
	addToken( "quality", UNIFORM, INTEGER );
	addToken( "bias0", UNIFORM, FLOAT );
	addToken( "bias1", UNIFORM, FLOAT );
	addToken( "jitter", UNIFORM, INTEGER );
	addToken( "depthfilter", UNIFORM, STRING );
}


// If the token already exists, addToken() return the corresponding id
TqTokenId CqDictionary::addToken ( std::string n, EqTokenClass tc, EqTokenType tt, TqUint qnt, bool inln )
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
	if ( ip.isInline() == true )
	{
		j = addToken ( ip.getIdentifier(), ip.getClass(), ip.getType(), ip.getQuantity(), true );
	}
	else
	{
		std::vector<SqTokenEntry>::iterator first = te.begin();

		for ( i = 1;first != te.end();first++, i++ )
		{
			if ( ( n == first->name ) && ( first->in_line == false ) )
				j = i;
		}
		if ( j == 0 )
		{
			std::string st( "Token not declared: " );
			st += n;
			throw CqError( RIE_ILLSTATE, RIE_ERROR, st, false );
		}
	}
	return j;
}


TqUint CqDictionary::allocSize ( TqTokenId id, TqUint vertex, TqUint varying, TqUint uniform, TqUint facevarying )
{
	TqUint size;
	std::vector<SqTokenEntry>::iterator first = te.begin();
	first += id - 1;
	size = getTypeSize( first->ttype );
	switch ( first->tclass )
	{
			case VERTEX:
			size *= vertex;
			break;
			case VARYING:
			size *= varying;
			break;
			case UNIFORM:
			size *= uniform;
			break;
			case CONSTANT:
			break;
			case FACEVARYING:
			size *= facevarying;
			break;
	}
	size *= ( first->quantity );
	return size;
}

TqUint CqDictionary::getTypeSize ( EqTokenType t )
{
	switch ( t )
	{
			case FLOAT:
			return 1;
			case POINT:
			return 3;
			case VECTOR:
			return 3;
			case NORMAL:
			return 3;
			case COLOR:
			return 1;
			case STRING:
			return 1;
			case MATRIX:
			return 16;
			case HPOINT:
			return 4;
			case INTEGER:
			return 1;
			default :
			throw CqError( RIE_BUG, RIE_SEVERE,
			               "CqDictionary::getTypeSize(TokenType) --> Unknown token type", false );
	}
}

void CqDictionary::isValid ( TqTokenId id )
{
	if ( id > te.size() || id == 0 )
		throw CqError( RIE_BUG, RIE_SEVERE,
		               "CqDictionary::isValid(TokenId) --> Bad ID", false );
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
		if ( ( first->in_line ) == true )
		{
			cout << " inline";
		}
		cout << endl;
	}
	cout << "------------------------------------------------------" << endl;
	cout << endl;
}
#endif
