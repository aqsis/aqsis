// -*- C++ -*-
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

/** \file
    \brief Declare common data and functions for RIB parser.
    \author Tim Shead & Paul C. Gregory
*/

#ifdef	WIN32
#pragma warning(disable : 4786)
#endif


#include "logging.h"
#include "librib.h"
#include "libribtypes.h"
#include "parserstate.h"

#include <iostream>
#include <cassert>
#include <stdio.h>
#include "bdec.h"

//
// from scanner.lxx and parser.yxx
struct yy_buffer_state;
extern struct yy_buffer_state* current_flex_buffer( void ) ;
extern struct yy_buffer_state* yy_create_buffer( FILE*, int ) ;
extern void yy_switch_to_buffer( struct yy_buffer_state* ) ;
extern void yy_delete_buffer( struct yy_buffer_state* ) ;
extern int yyparse();
extern int yydebug;

namespace librib
{

extern void ParserDeclare( RendermanInterface* CallbackInterface, const std::string Name, const std::string Type );
extern void	ClearDeclarations();

FILE *ParseInputFile = stdin;
CqRibBinaryDecoder *BinaryDecoder = 0;
std::string ParseStreamName = "stdin";
RendermanInterface* ParseCallbackInterface = 0;
std::ostream* ParseErrorStream = &Aqsis::log();
unsigned int ParseLineNumber;
bool ParseSucceeded = true;

// ReadArchive callback function
RtArchiveCallback pArchiveCallback;

// From the parser itself
extern bool fRecovering;

// From inside the scanner
extern bool fRequest;
extern bool fParams;

extern "C" const char* StandardParameters[][2] =
	    {
	        {"Ka", "uniform float"
	        },
	        {"Kd", "uniform float" },
	        {"Ks", "uniform float" },
	        {"Kr", "uniform float" },
	        {"roughness", "uniform float" },
	        {"texturename", "uniform string" },
	        {"specularcolor", "uniform color" },
	        {"intensity", "uniform float" },
	        {"lightcolor", "uniform color" },
	        {"from", "uniform point" },
	        {"to", "uniform point" },
	        {"coneangle", "uniform float" },
	        {"conedeltaangle", "uniform float" },
	        {"beamdistribution", "uniform float" },
	        {"mindistance", "uniform float" },
	        {"maxdistance", "uniform float" },
	        {"distance", "uniform float" },
	        {"background", "uniform color" },
	        {"fov", "uniform float" },
	        {"P", "vertex point" },
	        {"Pz", "vertex point" },
	        {"Pw", "vertex hpoint" },
	        {"N", "varying normal" },
	        {"Ng", "varying normal" },
	        {"Np", "uniform normal" },
	        {"Cs", "varying color" },
	        {"Os", "varying color" },
	        {"s", "varying float" },
	        {"t", "varying float" },
	        {"st", "varying float[2]" },
	        {"gridsize", "uniform integer" },
	        {"texturememory", "uniform integer" },
	        {"bucketsize", "uniform integer[2]" },
	        {"eyesplits", "uniform integer" },
	        {"shader", "uniform string" },
	        {"archive", "uniform string" },
	        {"texture", "uniform string" },
	        {"display", "uniform string" },
	        {"plugin", "uniform string" },
	        {"auto_shadows", "uniform string" },
	        {"endofframe", "uniform integer" },
	        {"offset", "uniform float"},
	        {"sphere", "uniform float" },
	        {"coordinatesystem", "uniform string" },
	        {"shadows", "uniform string" },
	        {"shadowmapsize", "uniform integer[2]" },
	        {"shadowangle", "uniform float" },
	        {"shadowmapname", "uniform string" },
	        {"shadow_shadingrate", "uniform float" },
	        {"name", "uniform string" },
	        {"shadinggroup", "uniform string" },
	        {"sense", "uniform string" },
	        {"compression", "uniform string" },
	        {"quality", "uniform integer" },
	        {"bias0", "uniform float" },
	        {"bias1", "uniform float" },
	        {"bias", "uniform float" },
	        {"jitter", "uniform integer" },
	        {"depthfilter", "uniform string" },
	        {"width", "varying float" },
	        {"constantwidth", "constant float" },
	        {"binary", "uniform integer" },
	        {"procedural", "uniform string" },
	        {"quantize", "uniform float[4]" },
	        {"dither", "uniform float" },
	        {"interpolateboundary", "uniform integer" },
	        {"zthreshold", "uniform color" },
		{"enabled", "uniform integer" },
		{"echoapi", "uniform integer" },
	        { NULL , NULL }
	    };

void StandardDeclarations( RendermanInterface* CallbackInterface )
{
	// Declare standard arguments
	unsigned int i = 0;
	while( StandardParameters[ i ][0] != NULL )
	{
		std::string name( StandardParameters[i][0] );
		std::string type( StandardParameters[i][1] );
		ParserDeclare( CallbackInterface, name, type );
		i++;
	};
}


void CleanupDeclarations( RendermanInterface& CallbackInterface )
{
	ClearDeclarations();
}


/// Retrieve the current state of the parser internal variables
CqRIBParserState GetParserState()
{
	CqRIBParserState state;

	state.m_pParseInputFile = ParseInputFile;
	state.m_ParseStreamName = ParseStreamName;
	state.m_pBinaryDecoder = BinaryDecoder;
	state.m_pParseErrorStream = ParseErrorStream;
	state.m_pArchiveCallback = pArchiveCallback;

	state.m_pParseCallbackInterface = ParseCallbackInterface;
	state.m_ParseLineNumber = ParseLineNumber;
	state.m_ParseSucceeded = ParseSucceeded;

	state.m_fRecovering = fRecovering;
	state.m_fRequest = fRequest;
	state.m_fParams = fParams;

	state.m_pYY_STATE = ( void* ) current_flex_buffer();

	return state;
};

/// Retrieve the current state of the parser internal variables
void SetParserState( CqRIBParserState& state )
{
	ParseInputFile = state.m_pParseInputFile;
	ParseStreamName = state.m_ParseStreamName;
	BinaryDecoder = state.m_pBinaryDecoder;
	ParseErrorStream = state.m_pParseErrorStream;
	pArchiveCallback = state.m_pArchiveCallback;

	ParseCallbackInterface = state.m_pParseCallbackInterface;
	ParseLineNumber = state.m_ParseLineNumber;
	ParseSucceeded = state.m_ParseSucceeded;

	fRecovering = state.m_fRecovering;
	fRequest = state.m_fRequest;
	fParams = state.m_fParams;

	yy_switch_to_buffer( ( struct yy_buffer_state* ) state.m_pYY_STATE ) ;
};

bool Parse( FILE *InputStream, const std::string StreamName, RendermanInterface& CallbackInterface, std::ostream& ErrorStream, RtArchiveCallback callback )
{
	ParseInputFile = InputStream;
	ParseStreamName = StreamName;
	ParseCallbackInterface = &CallbackInterface;
	ParseErrorStream = &ErrorStream;
	ParseLineNumber = 1;
	ParseSucceeded = true;
	fRequest = false;
	fRecovering = false;
	fParams = false;
	pArchiveCallback = callback;

	BinaryDecoder = new CqRibBinaryDecoder( InputStream );
	// 16384 is knicked from flex, in reality since we read via BinaryDecoder, this is not used
	struct yy_buffer_state *yybuffer = ::yy_create_buffer( InputStream, 16384 );
	::yy_switch_to_buffer( yybuffer );

	// Reenable this to get parse tree output in debug version.
	//yydebug = 1;
	yyparse();

	::yy_delete_buffer( yybuffer );
	delete BinaryDecoder;

	return ParseSucceeded;
}

bool ParseOpenStream( CqRibBinaryDecoder *decoder, const std::string StreamName, RendermanInterface& CallbackInterface, std::ostream& ErrorStream, RtArchiveCallback callback)
{
	BinaryDecoder = decoder;
	ParseStreamName = StreamName;
	ParseCallbackInterface = &CallbackInterface;
	ParseErrorStream = &ErrorStream;
	ParseLineNumber = 1;
	ParseSucceeded = true;
	fRequest = false;
	fRecovering = false;
	fParams = false;
	pArchiveCallback = callback;

	// 1 is used to attempt to force interactive scanning
	struct yy_buffer_state *yybuffer = ::yy_create_buffer( stdin, 1);
	::yy_switch_to_buffer(yybuffer);

	// Reenable this to get parse tree output in debug version.
	// yydebug = 1;
	yyparse();

	::yy_delete_buffer(yybuffer);

	return ParseSucceeded;
}

void ResetParser()
{
	ParseInputFile = stdin;
	ParseStreamName = "stdin";
	ParseCallbackInterface = 0;
	ParseErrorStream = &Aqsis::log();
	ParseLineNumber = 1;
	ParseSucceeded = true;
}

}
; // namespace librib
