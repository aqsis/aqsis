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

#ifdef	WIN32
#pragma warning(disable : 4786)
#endif

#include <iostream>

#include <stdio.h>
#include "bdec.h"

using namespace librib;


CqRibBinaryDecoder::CqRibBinaryDecoder(std::string filename)
{
    gzf=gzopen(filename.c_str(),"rb");
    if (gzf==NULL) { fail_flag=TqTrue; eof_flag=TqTrue; }
    else { fail_flag=TqFalse; eof_flag=TqFalse; }
}

CqRibBinaryDecoder::CqRibBinaryDecoder(FILE *filename)
{
    gzf=gzdopen(fileno(filename),"rb");
    if (gzf==NULL) { fail_flag=TqTrue; eof_flag=TqTrue; }
    else { fail_flag=TqFalse; eof_flag=TqFalse; }
}

CqRibBinaryDecoder::~CqRibBinaryDecoder()
{
    gzclose(gzf);
}

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



/* Convert To Signed Integer */
TqInt CqRibBinaryDecoder::ctsi(TqChar a)
{
    return a;
}
TqInt CqRibBinaryDecoder::ctsi(TqChar a, TqChar b)
{
    TqInt i,j;
    i=a; i<<=8;
    j=b; j&=0xff; i|=j;
    return i;
}
TqInt CqRibBinaryDecoder::ctsi(TqChar a, TqChar b, TqChar c)
{
    TqInt i,j;
    i=a; i<<=8;
    j=b; j&=0xff; i|=j; i<<=8;
    j=c; j&=0xff; i|=j;
    return i;
}
TqInt CqRibBinaryDecoder::ctsi(TqChar a, TqChar b, TqChar c, TqChar d)
{
    TqInt i,j;
    i=a; i<<=8;
    j=b; j&=0xff; i|=j; i<<=8;
    j=c; j&=0xff; i|=j; i<<=8;
    j=d; j&=0xff; i|=j;
    return i;
}



/* Convert To Unsigned Integer */
TqUint CqRibBinaryDecoder::ctui(TqChar a)
{
    TqUint i;
    i=a; i&=0xff;
    return i;
}
TqUint CqRibBinaryDecoder::ctui(TqChar a, TqChar b)
{
    TqUint i,j;
    i=a; i&=0xff; i<<=8;
    j=b; j&=0xff; i|=j;
    return i;
}
TqUint CqRibBinaryDecoder::ctui(TqChar a, TqChar b, TqChar c)
{
    TqUint i,j;
    i=a; i&=0xff; i<<=8;
    j=b; j&=0xff; i|=j; i<<=8;
    j=c; j&=0xff; i|=j;
    return i;
}
TqUint CqRibBinaryDecoder::ctui(TqChar a, TqChar b, TqChar c, TqChar d)
{
    TqUint i,j;
    i=a; i&=0xff; i<<=8;
    j=b; j&=0xff; i|=j; i<<=8;
    j=c; j&=0xff; i|=j; i<<=8;
    j=d; j&=0xff; i|=j;
    return i;
}



/* Get Char */
void CqRibBinaryDecoder::gc(TqChar &c)
{
    TqInt i;
    i=gzgetc(gzf);
    if (i==-1) {
	if (gzeof(gzf)==1) eof_flag=TqTrue;
	throw std::string("");
    }
    c=i;
}


/* Send N Characters */
void CqRibBinaryDecoder::snc(TqUint n, std::strstream &str)
{
    TqChar a;
    for (TqUint i=0; i<n; i++) {
	gc(a);
	str << a;
    }
}


void CqRibBinaryDecoder::sendFloat(std::strstream &str)
{
    TqChar b1,b2,b3,b4;
    TqFloat f;
    TqPchar g;
    GET4;
    g=reinterpret_cast<TqPchar>(&f);
#ifdef WORDS_BIGENDIAN
    g[0]=b1; g[1]=b2; g[2]=b3; g[3]=b4;
#else
    g[0]=b4; g[1]=b3; g[2]=b2; g[3]=b1;
#endif
    str << " " << f << " ";
}

void CqRibBinaryDecoder::sendDouble(std::strstream &str)
{
    TqChar b1,b2,b3,b4,b5,b6,b7,b8;
    TqDouble d;
    TqPchar g;
    GET8;
    g=reinterpret_cast<TqPchar>(&d);
#ifdef WORDS_BIGENDIAN
    g[0]=b1; g[1]=b2; g[2]=b3; g[3]=b4; g[4]=b5; g[5]=b6; g[6]=b7; g[7]=b8;
#else
    g[0]=b8; g[1]=b7; g[2]=b6; g[3]=b5; g[4]=b4; g[5]=b3; g[6]=b2; g[7]=b1;
#endif
    str << " " << d << " ";
}

void CqRibBinaryDecoder::readString (TqChar c, std::strstream &str)
{
    TqChar b1,b2,b3,b4;
    TqUint ui;

    switch (c) {
    case '\220': break;
    case '\221': snc(1,str); break;
    case '\222': snc(2,str); break;
    case '\223': snc(3,str); break;
    case '\224': snc(4,str); break;
    case '\225': snc(5,str); break;
    case '\226': snc(6,str); break;
    case '\227': snc(7,str); break;
    case '\230': snc(8,str); break;
    case '\231': snc(9,str); break;
    case '\232': snc(10,str); break;
    case '\233': snc(11,str); break;
    case '\234': snc(12,str); break;
    case '\235': snc(13,str); break;
    case '\236': snc(14,str); break;
    case '\237': snc(15,str); break;
    case '\240': GET1; ui=ctui(b1); snc(ui,str); break;
    case '\241': GET2; ui=ctui(b1,b2); snc(ui,str); break;
    case '\242': GET3; ui=ctui(b1,b2,b3); snc(ui,str); break;
    case '\243': GET4; ui=ctui(b1,b2,b3,b4); snc(ui,str); break;
    case '"':
	for (gc(c);c!='"';gc(c)) {
	    str << c;
	}
	break;
    default:
	throw std::string("CqRibBinaryDecoder::readString (TqChar, strstream &) --> invalid char");
    }
}


void CqRibBinaryDecoder::getNext ()
{
    TqChar c, b1, b2, b3, b4;
    TqUint ui; 
    TqFloat f;
    TqPchar cp;
    std::strstream ostr;
    std::strstream str;
    std::string tmpstr;

    gc(c);

    switch (c) {
	/* Encoded Integers and fixed point numbers */
    case '\200':  //0x80 B	
	GET1;
	ostr << " " << ctsi(b1) << " ";
	break;

    case '\201':  //0x81 BB
	GET2;
	ostr << " " << ctsi(b1,b2) << " ";
	break;

    case '\202':  //0x82 BBB
	GET3;
	ostr << " " << ctsi(b1,b2,b3) << " ";
	break;

    case '\203':  //0x83 BBBB
	GET4;
	ostr << " " << ctsi(b1,b2,b3,b4) << " ";
	break;

    case '\204':  //0x84 .B
	GET1;
	f=ctui(b1); f/=256;
	ostr << " " << f << " ";
	break;

    case '\205':  //0x85 B.B
	GET2;
	f=ctui(b2); f/=256; f+=ctsi(b1);
	ostr << " " << f << " ";
	break;

    case '\206':  //0x86 BB.B
	GET3;
	f=ctui(b3); f/=256; f+=ctsi(b1,b2);
	ostr << " " << f << " ";
	break;

    case '\207':  //0x87 BBB.B
	GET4;
	f=b4; f/=256; f+=ctsi(b1,b2,b3);
	ostr << " " << f << " ";
	break;

    case '\210': break; // -

    case '\211':  //0x89 .BB
	GET2;
	f=ctui(b1,b2); f/=65536;
	ostr << " " << f << " ";
	break;

    case '\212':  //0x8A B.BB
	GET3;
	f=ctui(b2,b3); f/=65536; f+=ctsi(b1);
	ostr << " " << f << " ";
	break;

    case '\213':  //0x8B BB.BB
	GET4;
	f=ctui(b3,b4); f/=65536; f+=ctsi(b1,b2);
	ostr << " " << f << " ";
	break;

    case '\214': break; // -
    case '\215': break; // -

    case '\216':  // 0x8E .BBB
	GET3;
	f=ctui(b1,b2,b3); f/=16777216;
	ostr << " " << f << " ";
	break;

    case '\217':  // 0x8F B.BBB
	GET4;
	f=ctui(b2,b3,b4); f/=16777216; f+=ctsi(b1);
	ostr << " " << f << " ";
	break;

	/* Encoded strings of no more than 15 characters */
    case '\220': break;  // 0x90
    case '\221': ostr << " \""; snc(1, ostr); ostr << "\" "; break;
    case '\222': ostr << " \""; snc(2, ostr);  ostr << "\" "; break;
    case '\223': ostr << " \""; snc(3, ostr);  ostr << "\" "; break;
    case '\224': ostr << " \""; snc(4, ostr);  ostr << "\" "; break;
    case '\225': ostr << " \""; snc(5, ostr);  ostr << "\" "; break;
    case '\226': ostr << " \""; snc(6, ostr);  ostr << "\" "; break;
    case '\227': ostr << " \""; snc(7, ostr);  ostr << "\" "; break;
    case '\230': ostr << " \""; snc(8, ostr);  ostr << "\" "; break;
    case '\231': ostr << " \""; snc(9, ostr);  ostr << "\" "; break;
    case '\232': ostr << " \""; snc(10, ostr);  ostr << "\" "; break;
    case '\233': ostr << " \""; snc(11, ostr);  ostr << "\" "; break;
    case '\234': ostr << " \""; snc(12, ostr);  ostr << "\" "; break;
    case '\235': ostr << " \""; snc(13, ostr);  ostr << "\" "; break;
    case '\236': ostr << " \""; snc(14, ostr);  ostr << "\" "; break;
    case '\237': ostr << " \""; snc(15, ostr);  ostr << "\" "; break;

	/* Encoded strings longer than 15 characters */
    case '\240': GET1; ui=ctui(b1); ostr << " \""; snc(ui, ostr); ostr << "\" "; break;  // 0xA0
    case '\241': GET2; ui=ctui(b1,b2); ostr << " \""; snc(ui, ostr); ostr << "\" "; break;
    case '\242': GET3; ui=ctui(b1,b2,b3); ostr << " \""; snc(ui, ostr); ostr << "\" "; break;
    case '\243': GET4; ui=ctui(b1,b2,b3,b4); ostr << " \""; snc(ui, ostr); ostr << "\" "; break;

	/* Encoded single precision IEEE floating point value */
    case '\244': sendFloat(ostr); break;  // 0xA4

	/* Encoded double precision IEEE floating point value */
    case '\245': sendDouble(ostr); break;  // 0xA5

	/* Encoded Ri request */
    case '\246':  // 0xA6
	GET1;
	ui=ctui(b1);
	ostr << "\n" << ritab[ui] << " ";
	break;

	/* RESERVED */
    case '\247': case '\250': case '\251': case '\252': case '\253': case '\254': case '\255': case '\256':
    case '\257': case '\260': case '\261': case '\262': case '\263': case '\264': case '\265': case '\266':
    case '\267': case '\270': case '\271': case '\272': case '\273': case '\274': case '\275': case '\276':
    case '\277': case '\300': case '\301': case '\302': case '\303': case '\304': case '\305': case '\306':
    case '\307':
	throw std::string(" BINARY_DECODER_WARNING: reserved byte encountered");
	break;

	/* Encoded single precision array */
    case '\310':  // 0xC8 L
	GET1;
	ui=ctui(b1);
	ostr << "[";
	for (;ui>0; ui--)
	    sendFloat(ostr);
	ostr << "]";
	break;

    case '\311':  // 0xC9 LL
	GET2;
	ui=ctui(b1,b2);
	ostr << "[";
	for (;ui>0; ui--)
	    sendFloat(ostr);
	ostr << "]";
	break;

    case '\312':  // 0xCA LLL
	GET3;
	ui=ctui(b1,b2,b3);
	ostr << "[";
	for (;ui>0; ui--)
	    sendFloat(ostr);
	ostr << "]";
	break;

    case '\313':  // 0xCB LLLL
	GET4;
	ui=ctui(b1,b2,b3,b4);
	ostr << "[";
	for (;ui>0; ui--)
	    sendFloat(ostr);
	ostr << "]";
	break;

	/* Define encoded request */
    case '\314':  // 0xCC B <string>
	GET1;
	ui=ctui(b1);
	gc(c);
	readString(c,str); str << std::ends;
	tmpstr=str.str();
	ritab[ui]=tmpstr;
	str.freeze(false);
	break;

	/* Define encoded string token */
    case '\315':  // 0xCD L
	GET1;
	ui=ctui(b1);
	gc(c);
	str << "\""; readString(c,str); str << "\"" << std::ends;
	if (stringtab.size() <= ui) stringtab.resize(ui+1);
	tmpstr=str.str();
	stringtab[ui]=tmpstr;
	str.freeze(false);
	break;

    case '\316':  // 0xCE LL
	GET2;
	ui=ctui(b1,b2);
	gc(c);
	str << "\""; readString(c,str); str << "\"" << std::ends;
	if (stringtab.size() <= ui) stringtab.resize(ui+1);
	tmpstr=str.str();
	stringtab[ui]=tmpstr;
	str.freeze(false);
	break;

	/* Interpolate defined string */
    case '\317':  // 0xCF L
	GET1;
	ui=ctui(b1);
	ostr << " " << stringtab[ui] << " ";
	break;

    case '\320': // 0xD0
	GET2;
	ui=ctui(b1,b2);
	ostr << " " << stringtab[ui] << " ";
	break;

	    /* RESERVED */
    case '\321': case '\322': case '\323': case '\324': case '\325': case '\326': case '\327': case '\330':
    case '\331': case '\332': case '\333': case '\334': case '\335': case '\336': case '\337': case '\340':
    case '\341': case '\342': case '\343': case '\344': case '\345': case '\346': case '\347': case '\350':
    case '\351': case '\352': case '\353': case '\354': case '\355': case '\356': case '\357': case '\360':
    case '\361': case '\362': case '\363': case '\364': case '\365': case '\366': case '\367': case '\370':
    case '\371': case '\372': case '\373': case '\374': case '\375': case '\376': case '\377':
	throw std::string(" BINARY_DECODER_WARNING: reserved byte encountered");
	break;

    default:
	ostr << c;
    }

    TqUint size=ostr.pcount();
    cp=ostr.str();
    for(ui=0;ui<size;ui++)
	cv.push_back(cp[ui]);
}


TqInt CqRibBinaryDecoder::writeToBuffer(TqPchar buffer, TqUint size)
{
    TqUint count=cv.size();

    if (count<size) {
	for (TqUint i=0; i<count; i++)
	    buffer[i]=cv[i];
	cv.clear();
	return count;
    } else {
	for (TqUint i=0; i<size; i++)
	    buffer[i]=cv[i];

	cv.erase(cv.begin(),cv.begin()+size);
	return size;
    }
}


TqInt CqRibBinaryDecoder::read(TqPchar buffer, TqUint size)
{
    if (gzf==NULL) return -1;

    try {
	while (cv.size() < size) {
	    getNext();
	}
    } catch (std::string &s) {
	if (s!="") std::cerr << s << std::endl;
    }

    return writeToBuffer(buffer,size);
}
