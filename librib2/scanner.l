/* 
 * THIS FILE HAS BEEN SUPERSCEDED BY SCANNER.LXX AMD SJPI:D
 * BE CONSIDERED DEPRECATED
 */



/* -------------- declaration section -------------- */

%{

#ifdef	WIN32
extern "C" int isatty(int);
#pragma warning(disable : 4786)
#pragma warning(disable : 4102)
#endif

#include <fstream>
#include <stack>
#include <string>
#include <vector>
#include <assert.h>
#include <stdio.h>

#include "libribtypes.h"
#ifdef	WIN32
#include "parser.hpp"
#else
#include "parser.h"
#endif
#include "bdec.h"

namespace librib
{

extern FILE* ParseInputFile;
extern CqRibBinaryDecoder *BinaryDecoder;
extern unsigned int ParseLineNumber;
extern std::string ParseStreamName;
extern std::string ArchivePath;

bool	fRequest;
bool	fParams;

}; // namespace librib

static std::stack<CqRibBinaryDecoder*> StreamStack;
static std::stack<YY_BUFFER_STATE> BufferStack;
static std::stack<std::string> StreamNameStack;
static std::stack<unsigned int> LineNumberStack;

static int scannerinput(char* Buffer, int MaxSize);
std::string FindArchive(const std::string strFilename);
#undef YY_INPUT
#define YY_INPUT(buffer, result, max_size) (result = scannerinput(buffer, max_size))

//#define YY_SKIP_YYWRAP
//static int yywrap();

%}

delim				[ \t]
whitespace		{delim}+
letter				[A-Za-z]
digit					[0-9]
float				[+\-]?{digit}*(\.{digit}*)?([eE][+\-]?{digit}+)?
integer			[+\-]?{digit}+
string				\"[^"\n]*\"
eol					\r\n|\r|\n
comment			#.*
array_start		\[
array_end		\]

%s params
%x incl

%option lex-compat

/* -------------- rules section -------------- */

%%

	if(fRequest)
	{
		BEGIN(INITIAL);
		fRequest=false;
	}

	if(fParams)
	{
		BEGIN(params);
		fParams=false;
	}

{comment}				{  }
{eol}							{ ParseLineNumber++; }
{whitespace}			{  }

<params>{integer}		{ yylval.itype = atoi(yytext); return INTEGER_TOKEN; }
<params>{float}			{ yylval.ftype = atof(yytext); return FLOAT_TOKEN; }
<params>{string}		{ std::string temp(yytext); yylval.stype = new char[temp.size()-1]; strcpy(yylval.stype, temp.substr(1, temp.size()-2).c_str()); return STRING_TOKEN; }
<INITIAL>{integer}		{ return INVALID_VALUE; }
<INITIAL>{float}		{ return INVALID_VALUE; }
<INITIAL>{string}		{ return INVALID_VALUE; }
<INITIAL>{array_start}	{ return INVALID_VALUE; }
<INITIAL>{array_end}	{ return INVALID_VALUE; }
							
version					{ return REQUEST_TOKEN_VERSION; }
Declare					{ return REQUEST_TOKEN_DECLARE; }
FrameBegin				{ return REQUEST_TOKEN_FRAMEBEGIN; }
FrameEnd				{ return REQUEST_TOKEN_FRAMEEND; }
WorldBegin				{ return REQUEST_TOKEN_WORLDBEGIN; }
WorldEnd				{ return REQUEST_TOKEN_WORLDEND; }
Format					{ return REQUEST_TOKEN_FORMAT; }
FrameAspectRatio		{ return REQUEST_TOKEN_FRAMEASPECTRATIO; }
ScreenWindow			{ return REQUEST_TOKEN_SCREENWINDOW; }
CropWindow				{ return REQUEST_TOKEN_CROPWINDOW; }
Projection				{ return REQUEST_TOKEN_PROJECTION; }
Clipping				{ return REQUEST_TOKEN_CLIPPING; }
DepthOfField			{ return REQUEST_TOKEN_DEPTHOFFIELD; }
Shutter					{ return REQUEST_TOKEN_SHUTTER; }
PixelVariance			{ return REQUEST_TOKEN_PIXELVARIANCE; }
PixelSamples			{ return REQUEST_TOKEN_PIXELSAMPLES; }
PixelFilter				{ return REQUEST_TOKEN_PIXELFILTER; }
Exposure				{ return REQUEST_TOKEN_EXPOSURE; }
Imager					{ return REQUEST_TOKEN_IMAGER; }
Quantize				{ return REQUEST_TOKEN_QUANTIZE; }
Display					{ return REQUEST_TOKEN_DISPLAY; }
GaussianFilter			{ return REQUEST_TOKEN_GAUSSIANFILTER; }
BoxFilter				{ return REQUEST_TOKEN_BOXFILTER; }
TriangleFilter			{ return REQUEST_TOKEN_TRIANGLEFILTER; }
CatmullRomFilter		{ return REQUEST_TOKEN_CATMULLROMFILTER; }
SincFilter				{ return REQUEST_TOKEN_SINCFILTER; }
DiskFilter				{ return REQUEST_TOKEN_DISKFILTER; }
BesselFilter			{ return REQUEST_TOKEN_BESSELFILTER; }
Hider					{ return REQUEST_TOKEN_HIDER; }
ColorSamples			{ return REQUEST_TOKEN_COLORSAMPLES; }
RelativeDetail			{ return REQUEST_TOKEN_RELATIVEDETAIL; }
Option					{ return REQUEST_TOKEN_OPTION; }
AttributeBegin			{ return REQUEST_TOKEN_ATTRIBUTEBEGIN; }
AttributeEnd			{ return REQUEST_TOKEN_ATTRIBUTEEND; }
Color					{ return REQUEST_TOKEN_COLOR; }
Opacity					{ return REQUEST_TOKEN_OPACITY; }
TextureCoordinates		{ return REQUEST_TOKEN_TEXTURECOORDINATES; }
LightSource				{ return REQUEST_TOKEN_LIGHTSOURCE; }
AreaLightSource			{ return REQUEST_TOKEN_AREALIGHTSOURCE; }
Illuminate				{ return REQUEST_TOKEN_ILLUMINATE; }
Surface					{ return REQUEST_TOKEN_SURFACE; }
Atmosphere				{ return REQUEST_TOKEN_ATMOSPHERE; }
Interior				{ return REQUEST_TOKEN_INTERIOR; }
Exterior				{ return REQUEST_TOKEN_EXTERIOR; }
ShadingRate				{ return REQUEST_TOKEN_SHADINGRATE; }
ShadingInterpolation	{ return REQUEST_TOKEN_SHADINGINTERPOLATION; }
Matte					{ return REQUEST_TOKEN_MATTE; }
Blobby      			{ return REQUEST_TOKEN_BLOBBY; }
Bound					{ return REQUEST_TOKEN_BOUND; }
Detail					{ return REQUEST_TOKEN_DETAIL; }
DetailRange				{ return REQUEST_TOKEN_DETAILRANGE; }
GeometricApproximation	{ return REQUEST_TOKEN_GEOMETRICAPPROXIMATION; }
Orientation				{ return REQUEST_TOKEN_ORIENTATION; }
ReverseOrientation		{ return REQUEST_TOKEN_REVERSEORIENTATION; }
Sides					{ return REQUEST_TOKEN_SIDES; }
Identity				{ return REQUEST_TOKEN_IDENTITY; }
Transform				{ return REQUEST_TOKEN_TRANSFORM; }
ConcatTransform			{ return REQUEST_TOKEN_CONCATTRANSFORM; }
Perspective				{ return REQUEST_TOKEN_PERSPECTIVE; }
Translate				{ return REQUEST_TOKEN_TRANSLATE; }
Rotate					{ return REQUEST_TOKEN_ROTATE; }
Scale					{ return REQUEST_TOKEN_SCALE; }
Skew					{ return REQUEST_TOKEN_SKEW; }
Deformation				{ return REQUEST_TOKEN_DEFORMATION; }
Displacement			{ return REQUEST_TOKEN_DISPLACEMENT; }
CoordinateSystem		{ return REQUEST_TOKEN_COORDINATESYSTEM; }
CoordSysTransform		{ return REQUEST_TOKEN_COORDSYSTRANSFORM; }
TransformPoints			{ return REQUEST_TOKEN_TRANSFORMPOINTS; }
TransformBegin			{ return REQUEST_TOKEN_TRANSFORMBEGIN; }
TransformEnd			{ return REQUEST_TOKEN_TRANSFORMEND; }
Attribute				{ return REQUEST_TOKEN_ATTRIBUTE; }
Polygon					{ return REQUEST_TOKEN_POLYGON; }
GeneralPolygon			{ return REQUEST_TOKEN_GENERALPOLYGON; }
Curves      			{ return REQUEST_TOKEN_CURVES; }
Points      			{ return REQUEST_TOKEN_POINTS; }
PointsPolygons			{ return REQUEST_TOKEN_POINTSPOLYGONS; }
PointsGeneralPolygons	{ return REQUEST_TOKEN_POINTSGENERALPOLYGONS; }
Basis					{ return REQUEST_TOKEN_BASIS; }
Patch					{ return REQUEST_TOKEN_PATCH; }
PatchMesh				{ return REQUEST_TOKEN_PATCHMESH; }
NuPatch					{ return REQUEST_TOKEN_NUPATCH; }
TrimCurve				{ return REQUEST_TOKEN_TRIMCURVE; }
Sphere					{ return REQUEST_TOKEN_SPHERE; }
Cone					{ return REQUEST_TOKEN_CONE; }
Cylinder				{ return REQUEST_TOKEN_CYLINDER; }
Hyperboloid				{ return REQUEST_TOKEN_HYPERBOLOID; }
Paraboloid				{ return REQUEST_TOKEN_PARABOLOID; }
Disk					{ return REQUEST_TOKEN_DISK; }
Torus					{ return REQUEST_TOKEN_TORUS; }
Procedural				{ return REQUEST_TOKEN_PROCEDURAL; }
Geometry				{ return REQUEST_TOKEN_GEOMETRY; }
SolidBegin				{ return REQUEST_TOKEN_SOLIDBEGIN; }
SolidEnd				{ return REQUEST_TOKEN_SOLIDEND; }
ObjectBegin				{ return REQUEST_TOKEN_OBJECTBEGIN; }
ObjectEnd				{ return REQUEST_TOKEN_OBJECTEND; }
ObjectInstance			{ return REQUEST_TOKEN_OBJECTINSTANCE; }
MotionBegin				{ return REQUEST_TOKEN_MOTIONBEGIN; }
MotionEnd				{ return REQUEST_TOKEN_MOTIONEND; }
MakeTexture				{ return REQUEST_TOKEN_MAKETEXTURE; }
MakeBump				{ return REQUEST_TOKEN_MAKEBUMP; }
MakeLatLongEnvironment	{ return REQUEST_TOKEN_MAKELATLONGENVIRONMENT; }
MakeCubeFaceEnvironment { return REQUEST_TOKEN_MAKECUBEFACEENVIRONMENT; }
MakeShadow				{ return REQUEST_TOKEN_MAKESHADOW; }
ErrorHandler			{ return REQUEST_TOKEN_ERRORHANDLER; }
SubdivisionMesh			{ return REQUEST_TOKEN_SUBDIVISIONMESH; }
ReadArchive				{ BEGIN(incl); }
{letter}({letter}|{digit})*	{ return UNKNOWN_TOKEN; }
.						{ return(yytext[0]); }

<incl>[ \t]*		{ }	
<incl>{string}	{
							std::string filepath(yytext);
							filepath = filepath.substr(1, filepath.size() - 2);

							filepath = FindArchive( filepath );
							
							CqRibBinaryDecoder* newstream = new CqRibBinaryDecoder(filepath);
							if(newstream->fail())
								{
									std::cerr << "librib > scanner > Unable to open archive file " << filepath << std::endl;
									delete newstream;
								}
							else
								{
									StreamStack.push(newstream);
									StreamNameStack.push(ParseStreamName);
									ParseStreamName = filepath;
									LineNumberStack.push(ParseLineNumber);
									ParseLineNumber = 1;
									BufferStack.push(YY_CURRENT_BUFFER);
									yy_switch_to_buffer(yy_create_buffer(0, YY_BUF_SIZE));
								}

							BEGIN(INITIAL);
						}
						
<<EOF>>			{
							if(BufferStack.empty())
								{
									yyterminate();
									return EOF_TOKEN;
								}
							else
								{
									assert(!StreamStack.empty());
									delete StreamStack.top();
									StreamStack.pop();
									ParseStreamName = StreamNameStack.top();
									StreamNameStack.pop();
									ParseLineNumber = LineNumberStack.top();
									LineNumberStack.pop();
									yy_delete_buffer(YY_CURRENT_BUFFER);
									yy_switch_to_buffer(BufferStack.top());
									BufferStack.pop();
								}
						}


%%

/* -------------- code section -------------- */

int yywrap()
{
	return 1;
}

static int scannerinput(CqRibBinaryDecoder* Stream, char* Buffer, int MaxSize)
{
	// Sanity checks ...
	assert(Stream);
	assert(Buffer);
	assert(MaxSize);

	int count = 0;
	if(!Stream->eof())
		count=Stream->read(Buffer,MaxSize);

 	return count;
}

static int scannerinput(char* Buffer, int MaxSize)
{
	if(StreamStack.empty())
		return scannerinput(BinaryDecoder, Buffer, MaxSize);

	return scannerinput(StreamStack.top(), Buffer, MaxSize);
}



std::string FindArchive(const std::string strFilename)
{
	std::ifstream* pFStream = new std::ifstream( strFilename.c_str(), std::ios_base::binary|std::ios_base::in );
	if ( !pFStream->is_open() )
	{
		// If a searchpath option name was specified, use it.
		if ( strcmp( ArchivePath.c_str(), "" ) != 0 )
		{
			// if not found there, search in the specified option searchpath.
			std::string SearchPath( ArchivePath );
			// Search each specified path in the search path (separated by ':' or ';')
			unsigned int start = 0;
			while ( 1 )
			{
				// Find the next search path in the spec.
				unsigned int len = SearchPath.find_first_of( ";:", start ) - start;
				// Check if it is realy meant as a drive spec.
				if ( len == 1 && isalpha( SearchPath[ start ] ) )
					len += strcspn( &SearchPath[ start + 2 ], ";:" ) + 1;
				std::string strPath = SearchPath.substr( start, len );
				if ( strPath == "" ) break;

				// See if the shader can be found in this directory
				std::string strAlternativeFilename = strPath;
				// Check the path is correctly terminated
				if ( strAlternativeFilename[ strAlternativeFilename.size() - 1 ] != '/' &&
				        strAlternativeFilename[ strAlternativeFilename.size() - 1 ] != '\\' )
				strAlternativeFilename += "/";
				strAlternativeFilename += strFilename;

				// Clear the previous error first.
				pFStream->clear();
				pFStream->open( strAlternativeFilename.c_str(), std::ios::in );
				if ( pFStream->is_open() )
				{
					pFStream->close();
					return( strAlternativeFilename );
				}
				
				if ( len < strlen( &SearchPath[ start ] ) )
					start += len + 1;
				else
					break;
			}
		}
	}
	pFStream->close();

	return( strFilename );
}


