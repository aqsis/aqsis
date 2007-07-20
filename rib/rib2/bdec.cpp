// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
//
// -------------------------------------------------------------------------
//           The RenderMan (R) Interface Procedures and Protocol are:
//                     Copyright 1988, 1989, 2000, Pixar
//                           All rights reserved
// -------------------------------------------------------------------------

/** \file
    \brief Binary RIB decoding class and gzip file inflation.
    \author Lionel J. Lacour (intuition01@online.fr)
*/

#if _MSC_VER
#pragma warning(disable : 4786)
#endif

#include "logging.h"

#include <iostream>
#include <iomanip>

#include <stdio.h>
#include "bdec.h"

#ifdef AQSIS_SYSTEM_WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

using namespace librib;

static TqInt ignore_gz = -1;

/*--------------------------------------------*/
/**
 * Some macros
 **/
/**
   \def GET1
   Get one character from the input file.
   This char is stored in local variable b1.
*/
#define GET1 gc(b1)

/**
   \def GET2
   Get two characters from the input file.
   First one goes to local variable b1 and second char goes to b2.
*/
#define GET2 gc(b1); gc(b2)

/**
   \def GET3
   Get three characters from the input file.
   First char goes to local variable b1,
   second char is stored into local variable b2
   and third char goes to local variable b3.
*/
#define GET3 gc(b1); gc(b2); gc(b3)

/**
   \def GET4
   Get four characters from the input file.
   First char goes to local variable b1,
   second char goes to local variable b2,
   third char goes to local variable b3
   and the fourth goes to local variable b4.
*/
#define GET4 gc(b1); gc(b2); gc(b3); gc(b4)

/**
   \def GET8
   Get eight characters from the input file.
   Each char is stored in local variables b1, b2, b3, b4, b5, b6, b7 and b8.
   First taken goes to b1 and last taken goes to b8.
*/
#define GET8 gc(b1); gc(b2); gc(b3); gc(b4); gc(b5); gc(b6); gc(b7); gc(b8)

/*
 * Some support static functions 
 */
/*--------------------------------------------*/
/**
 * Convert a decimal number to ascii
 **/
static char *dtoa( TqInt i )
{
	static char decimal[ 100 ];
	sprintf( decimal, "%d", i );
	return decimal;
}
/*--------------------------------------------*/
/**
 * Convert a floating number to ascii
 **/
static char *ftoa( TqFloat f )
{
	static char floating[ 100 ];
	sprintf( floating, "%f", f );
	return floating;
}
/*--------------------------------------------*/
/**
 * Convert a double number to ascii
 **/
static char *lftoa( TqDouble d )
{
	static char floating[ 100 ];
	sprintf( floating, "%lf", d );
	return floating;
}

/*--------------------------------------------*/
/**
 * Initailise the buffers and structures used by zlib
 * Much of this is adapted from zlibs' gzio.c
 * Copyright (C) 1995-2002 Jean-loup Gailly.
 * 
 **/
static  TqChar gz_magic[2] = {(TqChar) 0x1f, (TqChar) 0x8b}; /* gzip magic header */
/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */

void
CqRibBinaryDecoder::initZlib( int buffersize )
{

	zavailable = 0;
	zstrm.zalloc = (alloc_func)0;
	zstrm.zfree = (free_func)0;
	zstrm.opaque = (voidpf)0;

	zstrm.next_in  = zin = new Bytef[ buffersize < 2 ? 2 : buffersize ];
	zcurrent = zin;
	zstrm.avail_in = 0;
	zstrm.next_out = zout = new Bytef[ buffersize < 2 ? 2 : buffersize ];
	zstrm.avail_out = zbuffersize;
	zerr = inflateInit2( &zstrm, -MAX_WBITS);
	is_gzip = 0;
	TqChar c;
	int len;

	/* Check the gzip magic number */
	// Start by only buffering 2 bytes
	zbuffersize = 2;
	for (len = 0; len < 2; len++)
	{
		try
		{
			gc( c );
		}
		catch(std::string)
		{
			return;
		}
		if (c != gz_magic[len])
		{
			if (len != 0)
				zavailable++, zcurrent--;
			if (c != EOF)
				zavailable++, zcurrent--;
			is_gzip = 0;
			zbuffersize = buffersize;

			return;
		}
	}

	// It's now safe to use the requested buffering
	zbuffersize = buffersize;
	/* Read the gzip header */
	zerr = Z_OK;
	gc( c );
	int method = (int) c;
	gc( c );
	int flags = (int) c;

	if (method != Z_DEFLATED || (flags & RESERVED) != 0)
	{
		zerr = Z_DATA_ERROR;
		return;
	}

	/* Discard time, xflags and OS code: */
	for (len = 0; len < 6; len++)
		gc(c);

	if ((flags & EXTRA_FIELD) != 0)
	{ 	/* skip the extra field */
		gc( c );
		len  =  (uInt) c;
		gc( c );
		len += ((uInt) c )<<8;
		/* len is garbage if EOF but the loop below will quit anyway */
		gc( c );
		while( len-- != 0 && c != EOF )
			gc( c );
	}
	if ((flags & ORIG_NAME) != 0)
	{ /* skip the original file name */
		gc( c );
		while( c != 0 && c != EOF )
			gc( c );
	}
	if ((flags & COMMENT) != 0)
	{   /* skip the .gz file comment */
		gc( c );
		while( c != 0 && c != EOF )
			gc( c );
	}
	if ((flags & HEAD_CRC) != 0)
	{  /* skip the header crc */
		for (len = 0; len < 2; len++)
			gc( c );
	}

	// Slightly confusing, but this transitions from in/out via
	//zin in plain text to in -> inflate -> out.
	zcurrent = zout;
	zavailable = 0;
	is_gzip = 1;
}

/*--------------------------------------------*/
/**
 * Open a rib file using ZLIB 
 **/

CqRibBinaryDecoder::CqRibBinaryDecoder( std::string filename, int buffersize )
{
	file = fopen(filename.c_str(), "rb");

	if ( file == NULL )
	{
		fail_flag = true;
		eof_flag = true;
	}
	else
	{
		fail_flag = false;
		eof_flag = false;
		initZlib( buffersize );

	}

}

/*--------------------------------------------*/
/**
 * ReOpen using ZLIB a stream (point to RIB file)
 * buffersize defaults to the same as gzdopen from zlib
 **/
CqRibBinaryDecoder::CqRibBinaryDecoder( FILE *filehandle, int buffersize )
{
	//We dup here because we don't want to close the original
	//filehandle.
	file = fdopen( dup( fileno( filehandle ) ), "rb");


	if ( file == NULL )
	{
		fail_flag = true;
		eof_flag = true;
	}
	else
	{
		fail_flag = false;
		eof_flag = false;
		initZlib( buffersize );
	}
}

/*--------------------------------------------*/
/**
 * Destructor; close the stream
 **/
CqRibBinaryDecoder::~CqRibBinaryDecoder()
{
	if (zin)
		delete[] zin;
	zin = 0;
	if (zout)
		delete[] zout;
	zout = 0;
	if ( file )
		fclose( file );

	inflateEnd( &zstrm );
}


/* Convert To Signed Integer */
TqInt CqRibBinaryDecoder::ctsi( TqChar a )
{
	return a;
}
TqInt CqRibBinaryDecoder::ctsi( TqChar a, TqChar b )
{
	TqInt i, j;
	i = a;
	i <<= 8;
	j = b;
	j &= 0xff;
	i |= j;
	return i;
}
TqInt CqRibBinaryDecoder::ctsi( TqChar a, TqChar b, TqChar c )
{
	TqInt i, j;
	i = a;
	i <<= 8;
	j = b;
	j &= 0xff;
	i |= j;
	i <<= 8;
	j = c;
	j &= 0xff;
	i |= j;
	return i;
}
TqInt CqRibBinaryDecoder::ctsi( TqChar a, TqChar b, TqChar c, TqChar d )
{
	TqInt i, j;
	i = a;
	i <<= 8;
	j = b;
	j &= 0xff;
	i |= j;
	i <<= 8;
	j = c;
	j &= 0xff;
	i |= j;
	i <<= 8;
	j = d;
	j &= 0xff;
	i |= j;
	return i;
}

/* Convert To Unsigned Integer */
TqUint CqRibBinaryDecoder::ctui( TqChar a )
{
	TqUint i;
	i = a;
	i &= 0xff;
	return i;
}
TqUint CqRibBinaryDecoder::ctui( TqChar a, TqChar b )
{
	TqUint i, j;
	i = a;
	i &= 0xff;
	i <<= 8;
	j = b;
	j &= 0xff;
	i |= j;
	return i;
}
TqUint CqRibBinaryDecoder::ctui( TqChar a, TqChar b, TqChar c )
{
	TqUint i, j;
	i = a;
	i &= 0xff;
	i <<= 8;
	j = b;
	j &= 0xff;
	i |= j;
	i <<= 8;
	j = c;
	j &= 0xff;
	i |= j;
	return i;
}
TqUint CqRibBinaryDecoder::ctui( TqChar a, TqChar b, TqChar c, TqChar d )
{
	TqUint i, j;
	i = a;
	i &= 0xff;
	i <<= 8;
	j = b;
	j &= 0xff;
	i |= j;
	i <<= 8;
	j = c;
	j &= 0xff;
	i |= j;
	i <<= 8;
	j = d;
	j &= 0xff;
	i |= j;
	return i;
}


/*--------------------------------------------------------------------*/
/* Get 1 Char
 */

void CqRibBinaryDecoder::gc( TqChar &c )
{

	if (ignore_gz == -1)
		ignore_gz = (getenv("IGNOREGZ")!= NULL);

	if( !zavailable )
	{
		if( is_gzip )
		{

			zstrm.next_out = zout;
			zstrm.avail_out = zbuffersize;
			while( (int) zstrm.avail_out == zbuffersize )
			{
				if( !zstrm.avail_in )
				{
					int count = fread( zin, 1, zbuffersize, file );
					zstrm.avail_in = count;
					zstrm.next_in = zin;
				};

				zerr = inflate(&(zstrm), Z_SYNC_FLUSH);

				if( zerr == Z_STREAM_END && (int) zstrm.avail_out == zbuffersize)
				{
					eof_flag = true;
					throw std::string( "" );
				};

				if( zerr != Z_OK && zerr != Z_STREAM_END )
				{
					fail_flag = eof_flag = true;
					throw std::string( "" );
				};
			};
			zcurrent = zout;
			zavailable = zbuffersize - zstrm.avail_out;
		}
		else
		{
			// Read into zin, zout would be clearer but
			//this makes the uncompressed->compress transition
			//easier.
			zavailable = zstrm.avail_in = fread( zin, 1, zbuffersize, file );
			if( zavailable == 0 )
			{
				eof_flag = true;
				throw std::string( "" );
				return;
			};
			zcurrent = zstrm.next_in = zin;
			if (ignore_gz)
				fseek(file,0,SEEK_SET);
		};
	};

	c = (char) *zcurrent ;
	zavailable--;
	if( !is_gzip )
		zstrm.avail_in--;
	zcurrent++;
	if( !is_gzip )
		zstrm.next_in++;
}

/*--------------------------------------------------------------------*/
/* Get N Char
 * call gc() in order to fill str with n characters.
 */
void CqRibBinaryDecoder::snc( TqUint n, std::string &str )
{
	TqChar a;
	for ( TqUint i = 0; i < n; i++ )
	{
		gc( a );
		str += a;
	}
}

/*--------------------------------------------------------------------*/
/* Get 1 float
 * call gc() in order to fill str with n characters.
 */

void CqRibBinaryDecoder::sendFloat( std::string &str )
{
	TqChar b1, b2, b3, b4;
	TqFloat f;
	TqPchar g;
	GET4;
	g = reinterpret_cast<TqPchar>( &f );
#ifdef WORDS_BIGENDIAN

	g[ 0 ] = b1;
	g[ 1 ] = b2;
	g[ 2 ] = b3;
	g[ 3 ] = b4;
#else

	g[ 0 ] = b4;
	g[ 1 ] = b3;
	g[ 2 ] = b2;
	g[ 3 ] = b1;
#endif

	str += " ";
	str += ftoa( f );
	str += " ";

}

/*--------------------------------------------------------------------*/
/* Get 1 double
 * call gc() in order to fill str with n characters.
 */
/* Send N Characters */

void CqRibBinaryDecoder::sendDouble( std::string &str )
{
	TqChar b1, b2, b3, b4, b5, b6, b7, b8;
	TqDouble d;
	TqPchar g;
	GET8;
	g = reinterpret_cast<TqPchar>( &d );
#ifdef WORDS_BIGENDIAN

	g[ 0 ] = b1;
	g[ 1 ] = b2;
	g[ 2 ] = b3;
	g[ 3 ] = b4;
	g[ 4 ] = b5;
	g[ 5 ] = b6;
	g[ 6 ] = b7;
	g[ 7 ] = b8;
#else

	g[ 0 ] = b8;
	g[ 1 ] = b7;
	g[ 2 ] = b6;
	g[ 3 ] = b5;
	g[ 4 ] = b4;
	g[ 5 ] = b3;
	g[ 6 ] = b2;
	g[ 7 ] = b1;
#endif

	str += " ";
	str += lftoa( d );
	str += " ";
}

/*--------------------------------------------------------------------*/
/* Get 1 string
 * call gc() in order to fill str with n characters.
 */

void CqRibBinaryDecoder::readString ( TqChar c, std::string &str )
{
	TqChar b1, b2, b3, b4;
	TqUint ui;

	switch ( c )
	{
			case '\220':
			break;
			case '\221':
			snc( 1, str );
			break;
			case '\222':
			snc( 2, str );
			break;
			case '\223':
			snc( 3, str );
			break;
			case '\224':
			snc( 4, str );
			break;
			case '\225':
			snc( 5, str );
			break;
			case '\226':
			snc( 6, str );
			break;
			case '\227':
			snc( 7, str );
			break;
			case '\230':
			snc( 8, str );
			break;
			case '\231':
			snc( 9, str );
			break;
			case '\232':
			snc( 10, str );
			break;
			case '\233':
			snc( 11, str );
			break;
			case '\234':
			snc( 12, str );
			break;
			case '\235':
			snc( 13, str );
			break;
			case '\236':
			snc( 14, str );
			break;
			case '\237':
			snc( 15, str );
			break;
			case '\240':
			GET1;
			ui = ctui( b1 );
			snc( ui, str );
			break;
			case '\241':
			GET2;
			ui = ctui( b1, b2 );
			snc( ui, str );
			break;
			case '\242':
			GET3;
			ui = ctui( b1, b2, b3 );
			snc( ui, str );
			break;
			case '\243':
			GET4;
			ui = ctui( b1, b2, b3, b4 );
			snc( ui, str );
			break;
			case '"':
			for ( gc( c );c != '"';gc( c ) )
			{
				str += c;
			}
			break;
			default:
			throw std::string( "CqRibBinaryDecoder::readString (TqChar, string &) --> invalid char" );
	}
}


/*--------------------------------------------------------------------*/
/* Get 1 token
 * call gc() in order to fill str with n characters.
 */

void CqRibBinaryDecoder::getNext ()
{
	TqChar c;
	TqChar b1, b2, b3, b4;
	TqUint ui;
	TqFloat f;
	/// \todo <b>Code Review</b>: use a stream for cv directly rather than using the temporary string ostr.
	std::string ostr;
	std::string str;
	std::string tmpstr;

	gc( c );

	if ( (TqUint) c < 127 )
	{
		ostr += c;
	}
	else
	{
		switch ( c )
		{
				/* Decode integers */
				case '\200':     //0x80 B
				GET1;
				ostr = " ";
				ostr += dtoa( ctsi( b1 ) );
				ostr += " ";
				break;

				case '\201':     //0x81 BB
				GET2;
				ostr = " ";
				ostr += dtoa( ctsi( b1, b2 ) );
				ostr += " ";
				break;

				case '\202':     //0x82 BBB
				GET3;
				ostr = " ";
				ostr += ctsi( b1, b2, b3 );
				ostr += " ";
				break;

				case '\203':     //0x83 BBBB
				GET4;
				ostr = " ";
				ostr += dtoa( ctsi( b1, b2, b3, b4 ) );
				ostr += " ";
				break;

				/* Decode fixed point numbers */
				case '\204':     //0x84 .B
				GET1;
				f = ctui( b1 );
				f /= 256;
				ostr = " ";
				ostr += ftoa( f );
				ostr += " ";
				break;

				case '\205':     //0x85 B.B
				GET2;
				f = ctui( b2 );
				f /= 256;
				f += ctsi( b1 );
				ostr = " ";
				ostr += ftoa( f );
				ostr += " ";
				break;

				case '\206':     //0x86 BB.B
				GET3;
				f = ctui( b3 );
				f /= 256;
				f += ctsi( b1, b2 );
				ostr = " ";
				ostr += ftoa( f );
				ostr += " ";
				break;

				case '\207':     //0x87 BBB.B
				GET4;
				f = b4;
				f /= 256;
				f += ctsi( b1, b2, b3 );
				ostr = " ";
				ostr += ftoa( f );
				ostr += " ";
				break;

				case '\210':
				break; // -

				case '\211':     //0x89 .BB
				GET2;
				f = ctui( b1, b2 );
				f /= 65536;
				ostr = " ";
				ostr += ftoa( f );
				ostr += " ";
				break;

				case '\212':     //0x8A B.BB
				GET3;
				f = ctui( b2, b3 );
				f /= 65536;
				f += ctsi( b1 );
				ostr = " ";
				ostr += ftoa( f );
				ostr += " ";
				break;

				case '\213':     //0x8B BB.BB
				GET4;
				f = ctui( b3, b4 );
				f /= 65536;
				f += ctsi( b1, b2 );
				ostr = " ";
				ostr += ftoa( f );
				ostr += " ";
				break;

				case '\214':
				break; // -
				case '\215':
				break; // -

				case '\216':     // 0x8E .BBB
				GET3;
				f = ctui( b1, b2, b3 );
				f /= 16777216;
				ostr = " ";
				ostr += ftoa( f );
				ostr += " ";
				break;

				case '\217':     // 0x8F B.BBB
				GET4;
				f = ctui( b2, b3, b4 );
				f /= 16777216;
				f += ctsi( b1 );
				ostr = " ";
				ostr += ftoa( f );
				ostr += " ";
				break;

				/* Decode small strings */
				case '\220':
				break;  // 0x90
				case '\221':
				ostr = " \"";
				snc( 1, ostr );
				ostr += "\" ";
				break;
				case '\222':
				ostr = " \"";
				snc( 2, ostr );
				ostr += "\" ";
				break;
				case '\223':
				ostr = " \"";
				snc( 3, ostr );
				ostr += "\" ";
				break;
				case '\224':
				ostr = " \"";
				snc( 4, ostr );
				ostr += "\" ";
				break;
				case '\225':
				ostr = " \"";
				snc( 5, ostr );
				ostr += "\" ";
				break;
				case '\226':
				ostr = " \"";
				snc( 6, ostr );
				ostr += "\" ";
				break;
				case '\227':
				ostr = " \"";
				snc( 7, ostr );
				ostr += "\" ";
				break;
				case '\230':
				ostr = " \"";
				snc( 8, ostr );
				ostr += "\" ";
				break;
				case '\231':
				ostr = " \"";
				snc( 9, ostr );
				ostr += "\" ";
				break;
				case '\232':
				ostr = " \"";
				snc( 10, ostr );
				ostr += "\" ";
				break;
				case '\233':
				ostr = " \"";
				snc( 11, ostr );
				ostr += "\" ";
				break;
				case '\234':
				ostr = " \"";
				snc( 12, ostr );
				ostr += "\" ";
				break;
				case '\235':
				ostr = " \"";
				snc( 13, ostr );
				ostr += "\" ";
				break;
				case '\236':
				ostr = " \"";
				snc( 14, ostr );
				ostr += "\" ";
				break;
				case '\237':
				ostr = " \"";
				snc( 15, ostr );
				ostr += "\" ";
				break;

				/* Decode strings of any size */
				case '\240':
				GET1;
				ui = ctui( b1 );
				ostr = " \"";
				snc( ui, ostr );
				ostr += "\" ";
				break;  // 0xA0
				case '\241':
				GET2;
				ui = ctui( b1, b2 );
				ostr = " \"";
				snc( ui, ostr );
				ostr += "\" ";
				break;
				case '\242':
				GET3;
				ui = ctui( b1, b2, b3 );
				ostr = " \"";
				snc( ui, ostr );
				ostr += "\" ";
				break;
				case '\243':
				GET4;
				ui = ctui( b1, b2, b3, b4 );
				ostr = " \"";
				snc( ui, ostr );
				ostr += "\" ";
				break;

				/* Decode a single precision floating point value */
				case '\244':
				sendFloat( ostr );
				break;  // 0xA4

				/* Decode a double precision floating point value */
				case '\245':
				sendDouble( ostr );
				break;  // 0xA5

				/* Decode a RI request previously declared */
				case '\246':     // 0xA6
				GET1;
				ui = ctui( b1 );
				ostr = "\n";
				ostr += ritab[ ui ];
				ostr += " ";
				break;


				case '\247':
				case '\250':
				case '\251':
				case '\252':
				case '\253':
				case '\254':
				case '\255':
				case '\256':
				case '\257':
				case '\260':
				case '\261':
				case '\262':
				case '\263':
				case '\264':
				case '\265':
				case '\266':
				case '\267':
				case '\270':
				case '\271':
				case '\272':
				case '\273':
				case '\274':
				case '\275':
				case '\276':
				case '\277':
				case '\300':
				case '\301':
				case '\302':
				case '\303':
				case '\304':
				case '\305':
				case '\306':
				case '\307':
				throw std::string( " BINARY_DECODER_WARNING: reserved byte encountered" );
				break;

				/* Decode arrays of single precision floating point values */
				case '\310':     // 0xC8 L
				GET1;
				ui = ctui( b1 );
				ostr += "[";
				for ( ;ui > 0; ui-- )
					sendFloat( ostr );
				ostr += "]";
				break;

				case '\311':     // 0xC9 LL
				GET2;
				ui = ctui( b1, b2 );
				ostr += "[";
				for ( ;ui > 0; ui-- )
					sendFloat( ostr );
				ostr += "]";
				break;

				case '\312':     // 0xCA LLL
				GET3;
				ui = ctui( b1, b2, b3 );
				ostr += "[";
				for ( ;ui > 0; ui-- )
					sendFloat( ostr );
				ostr += "]";
				break;

				case '\313':     // 0xCB LLLL
				GET4;
				ui = ctui( b1, b2, b3, b4 );
				ostr = "[";
				for ( ;ui > 0; ui-- )
					sendFloat( ostr );
				ostr += "]";
				break;

				/* Declare a RI request */
				case '\314':     // 0xCC B <string>
				GET1;
				ui = ctui( b1 );
				gc( c );
				readString( c, str ); //str << std::ends;
				tmpstr = str.c_str();
				ritab[ ui ] = tmpstr;
				break;

				/* Declare strings */
				case '\315':     // 0xCD L
				GET1;
				ui = ctui( b1 );
				gc( c );
				str += "\"";
				readString( c, str );
				str += "\"" ; //<< std::ends;
				if ( stringtab.size() <= ui )
					stringtab.resize( ui + 1 );
				tmpstr = str.c_str();
				stringtab[ ui ] = tmpstr;
				break;

				case '\316':     // 0xCE LL
				GET2;
				ui = ctui( b1, b2 );
				gc( c );
				str += "\"";
				readString( c, str );
				str += "\"" ; //std::ends;
				if ( stringtab.size() <= ui )
					stringtab.resize( ui + 1 );
				tmpstr = str.c_str();
				stringtab[ ui ] = tmpstr;
				break;

				/* Decode previously declared strings */
				case '\317':     // 0xCF L
				GET1;
				ui = ctui( b1 );
				ostr += " ";
				ostr += stringtab[ ui ];
				ostr += " ";
				break;

				case '\320':    // 0xD0 LL
				GET2;
				ui = ctui( b1, b2 );
				ostr += " ";
				ostr += stringtab[ ui ];
				ostr += " ";
				break;


				case '\321':
				case '\322':
				case '\323':
				case '\324':
				case '\325':
				case '\326':
				case '\327':
				case '\330':
				case '\331':
				case '\332':
				case '\333':
				case '\334':
				case '\335':
				case '\336':
				case '\337':
				case '\340':
				case '\341':
				case '\342':
				case '\343':
				case '\344':
				case '\345':
				case '\346':
				case '\347':
				case '\350':
				case '\351':
				case '\352':
				case '\353':
				case '\354':
				case '\355':
				case '\356':
				case '\357':
				case '\360':
				case '\361':
				case '\362':
				case '\363':
				case '\364':
				case '\365':
				case '\366':
				case '\367':
				case '\370':
				case '\371':
				case '\372':
				case '\373':
				case '\374':
				case '\375':
				case '\376':
				//case '\377': Signals the end of a RunProgram block
				throw std::string( " BINARY_DECODER_WARNING: reserved byte encountered" );
				break;

				default:
				ostr += c;
		}
	}

	for ( ui = 0;ui < ostr.size();ui++ )
		cv.push_back( ostr[ ui ] );

}

/*--------------------------------------------------------------------*/
/* Write N Chars
 * 
 */


TqInt CqRibBinaryDecoder::writeToBuffer( TqPchar buffer, TqUint size )
{
	TqUint count = cv.size();

	if ( count < size )
	{
		for ( TqUint i = 0; i < count; i++ )
			buffer[ i ] = cv[ i ];
		cv.clear();
		return count;
	}
	else
	{
		for ( TqUint i = 0; i < size; i++ )
			buffer[ i ] = cv[ i ];

		cv.erase( cv.begin(), cv.begin() + size );
		return size;
	}
}

/*--------------------------------------------------------------------*/
/* Get N Char
 * call gc() in order to fill str with n characters.
 */


TqInt CqRibBinaryDecoder::read( TqPchar buffer, TqUint size )
{
	TqInt n;

	if (!ignore_gz)
	{
		try
		{
			while ( cv.size() < size )
			{
				getNext();

				if( !cv.empty() )
				{
					if( cv.back() == '\n' )
					{
						size = cv.size() + 1;
						break;
					}
				}
			}
		}
		catch ( std::string & s )
		{
			if ( s != "" )
				Aqsis::log() << Aqsis::error << s << std::endl;
		}

		n = writeToBuffer( buffer, size );

	}
	else
	{
		TqUint t = fread(buffer,1, size,file);
		n  = t;
	}
	return n;
}


/*--------------------------------------------------------------------*/
void CqRibBinaryDecoder::dumpToStream(std::ostream& out)
{
	const TqInt bufSize = 1024;
	TqPchar buf = new TqChar[bufSize];
	while(!eof())
	{
		TqInt readSize = read(buf, bufSize-1);
		buf[readSize] = 0; // add null termination char.
		out << buf;
	}
}
