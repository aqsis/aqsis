// -*- C++ -*-
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
    \brief Declare common data and functions for RIB parser.
    \author Tim Shead & Paul C. Gregory
*/

#ifdef	WIN32
#pragma warning(disable : 4786)
#endif


#include "librib.h"

#include <iostream>
#include <cassert>
#include <stdio.h>
#include "bdec.h"

extern int yyparse();
extern int yydebug;

namespace librib
{

extern void ParserDeclare( RendermanInterface& CallbackInterface, const std::string Name, const std::string Type );

FILE *ParseInputFile = stdin;
CqRibBinaryDecoder *BinaryDecoder = 0;
std::string ParseStreamName = "stdin";
RendermanInterface* ParseCallbackInterface = 0;
std::ostream* ParseErrorStream = &std::cerr;
unsigned int ParseLineNumber;
bool ParseSucceeded = true;

void StandardDeclarations( RendermanInterface& CallbackInterface )
{
	// Declare standard arguments
	ParserDeclare( CallbackInterface, "Ka", "uniform float" );
	ParserDeclare( CallbackInterface, "Kd", "uniform float" );
	ParserDeclare( CallbackInterface, "Ks", "uniform float" );
	ParserDeclare( CallbackInterface, "Kr", "uniform float" );
	ParserDeclare( CallbackInterface, "roughness", "uniform float" );
	ParserDeclare( CallbackInterface, "texturename", "uniform string" );
	ParserDeclare( CallbackInterface, "specularcolor", "uniform color" );
	ParserDeclare( CallbackInterface, "intensity", "uniform float" );
	ParserDeclare( CallbackInterface, "lightcolor", "uniform color" );
	ParserDeclare( CallbackInterface, "from", "uniform point" );
	ParserDeclare( CallbackInterface, "to", "uniform point" );
	ParserDeclare( CallbackInterface, "coneangle", "uniform float" );
	ParserDeclare( CallbackInterface, "conedeltaangle", "uniform float" );
	ParserDeclare( CallbackInterface, "beamdistribution", "uniform float" );
	ParserDeclare( CallbackInterface, "mindistance", "uniform float" );
	ParserDeclare( CallbackInterface, "maxdistance", "uniform float" );
	ParserDeclare( CallbackInterface, "distance", "uniform float" );
	ParserDeclare( CallbackInterface, "background", "uniform color" );
	ParserDeclare( CallbackInterface, "fov", "uniform float" );
	ParserDeclare( CallbackInterface, "P", "vertex point" );
	ParserDeclare( CallbackInterface, "Pz", "vertex point" );
	ParserDeclare( CallbackInterface, "Pw", "vertex hpoint" );
	ParserDeclare( CallbackInterface, "N", "varying normal" );
	ParserDeclare( CallbackInterface, "Np", "uniform normal" );
	ParserDeclare( CallbackInterface, "Cs", "varying color" );
	ParserDeclare( CallbackInterface, "Os", "varying color" );
	ParserDeclare( CallbackInterface, "s", "varying float" );
	ParserDeclare( CallbackInterface, "t", "varying float" );
	ParserDeclare( CallbackInterface, "st", "varying float" );
	ParserDeclare( CallbackInterface, "gridsize", "uniform integer" );
	ParserDeclare( CallbackInterface, "texturememory", "uniform integer" );
	ParserDeclare( CallbackInterface, "bucketsize", "uniform integer[2]" );
	ParserDeclare( CallbackInterface, "eyesplits", "uniform integer" );
	ParserDeclare( CallbackInterface, "shader", "uniform string" );
	ParserDeclare( CallbackInterface, "archive", "uniform string" );
	ParserDeclare( CallbackInterface, "texture", "uniform string" );
	ParserDeclare( CallbackInterface, "display", "uniform string" );
	ParserDeclare( CallbackInterface, "dsolibs", "uniform string" );
	ParserDeclare( CallbackInterface, "auto_shadows", "uniform string" );
	ParserDeclare( CallbackInterface, "endofframe", "uniform integer" );
	ParserDeclare( CallbackInterface, "verbose", "uniform integer" );
	ParserDeclare( CallbackInterface, "sphere", "uniform float" );
	ParserDeclare( CallbackInterface, "coordinatesystem", "uniform string" );
	ParserDeclare( CallbackInterface, "shadows", "uniform string" );
	ParserDeclare( CallbackInterface, "shadowmapsize", "uniform integer[2]" );
	ParserDeclare( CallbackInterface, "shadowangle", "uniform float" );
	ParserDeclare( CallbackInterface, "shadowmapname", "uniform string" );
	ParserDeclare( CallbackInterface, "shadow_shadingrate", "uniform float" );
	ParserDeclare( CallbackInterface, "name", "uniform string" );
	ParserDeclare( CallbackInterface, "shadinggroup", "uniform string" );
	ParserDeclare( CallbackInterface, "sense", "uniform string" );
	ParserDeclare( CallbackInterface, "compression", "uniform string" );
	ParserDeclare( CallbackInterface, "quality", "uniform integer" );
	ParserDeclare( CallbackInterface, "bias0", "uniform float" );
	ParserDeclare( CallbackInterface, "bias1", "uniform float" );
	ParserDeclare( CallbackInterface, "bias", "uniform float" );
	ParserDeclare( CallbackInterface, "jitter", "uniform integer" );
	ParserDeclare( CallbackInterface, "depthfilter", "uniform string" );
	ParserDeclare( CallbackInterface, "width", "varying float" );
	ParserDeclare( CallbackInterface, "constantwidth", "constant float" );
	ParserDeclare( CallbackInterface, "dsolibs", "uniform string" );
}


bool Parse( FILE *InputStream, const std::string StreamName, RendermanInterface& CallbackInterface, std::ostream& ErrorStream )
{
	ParseInputFile = InputStream;
	ParseStreamName = StreamName;
	ParseCallbackInterface = &CallbackInterface;
	ParseErrorStream = &ErrorStream;
	ParseLineNumber = 1;
	ParseSucceeded = true;

	BinaryDecoder = new CqRibBinaryDecoder( InputStream );
	// Reenable this to get parse tree output in debug version.
	//yydebug = 1;
	yyparse();
	delete BinaryDecoder;

	return ParseSucceeded;
}

void ResetParser()
{
	ParseInputFile = stdin;
	ParseStreamName = "stdin";
	ParseCallbackInterface = 0;
	ParseErrorStream = &std::cerr;
	ParseLineNumber = 1;
	ParseSucceeded = true;
}

}
; // namespace librib
