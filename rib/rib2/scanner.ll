/* -------------- declaration section -------------- */

%{

#ifdef	WIN32
extern "C" int isatty(int);
#pragma warning(disable : 4786)
#pragma warning(disable : 4102)
#endif

#include "aqsis.h"

#include <fstream>
#include <stack>
#include <string>
#include <vector>
#include <assert.h>
#include <stdio.h>

#include "libribtypes.h"
#include "parser.hpp"

#include "bdec.h"

namespace librib
{

extern FILE* ParseInputFile;
extern CqRibBinaryDecoder *BinaryDecoder;
extern unsigned int ParseLineNumber;
extern std::string ParseStreamName;
extern RtArchiveCallback pArchiveCallback;

bool fRequest;
bool fParams;
bool fSkipFrame;

}; // namespace librib

static std::stack<CqRibBinaryDecoder*> StreamStack;
static std::stack<YY_BUFFER_STATE> BufferStack;
static std::stack<std::string> StreamNameStack;
static std::stack<unsigned int> LineNumberStack;

#define YY_DECL int yylex( YYSTYPE *lvalp )

static int scannerinput(char* Buffer, int MaxSize);
#undef YY_INPUT
#define YY_INPUT(buffer, result, max_size) (result = scannerinput(buffer, max_size))

//#define YY_SKIP_YYWRAP
//static int yywrap();

%}

delim				[ \t\\]
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
%x incl skip_frame

/* %option lex-compat */
%option noreject

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

	if(fSkipFrame)
	{
		BEGIN(skip_frame);
		fSkipFrame=false;
	}

{comment}			{ 
						lvalp->stype = new char[strlen(yytext)+1]; strcpy(lvalp->stype, yytext); return COMMENT; 
					}
<*>{eol}			{ ParseLineNumber++; }
<*>{whitespace}		{  }

<skip_frame>FrameEnd	{BEGIN(INITIAL);}
<skip_frame>.

<params>{integer}		{ lvalp->itype = atoi(yytext); return INTEGER_TOKEN; }
<params>{float}			{ lvalp->ftype = atof(yytext); return FLOAT_TOKEN; }
<params>{string}		{ std::string temp(yytext); lvalp->stype = new char[temp.size()-1]; strcpy(lvalp->stype, temp.substr(1, temp.size()-2).c_str()); return STRING_TOKEN; }
<INITIAL>{integer}		{ return INVALID_VALUE; }
<INITIAL>{float}		{ return INVALID_VALUE; }
<INITIAL>{string}		{ return INVALID_VALUE; }
<INITIAL>{array_start}	{ return INVALID_VALUE; }
<INITIAL>{array_end}	{ return INVALID_VALUE; }

AreaLightSource			{ return REQUEST_TOKEN_AREALIGHTSOURCE; }
Atmosphere			{ return REQUEST_TOKEN_ATMOSPHERE; }
Attribute			{ return REQUEST_TOKEN_ATTRIBUTE; }
AttributeBegin			{ return REQUEST_TOKEN_ATTRIBUTEBEGIN; }
AttributeEnd			{ return REQUEST_TOKEN_ATTRIBUTEEND; }
Basis				{ return REQUEST_TOKEN_BASIS; }
BesselFilter			{ return REQUEST_TOKEN_BESSELFILTER; }
Blobby      			{ return REQUEST_TOKEN_BLOBBY; }
Bound				{ return REQUEST_TOKEN_BOUND; }
BoxFilter			{ return REQUEST_TOKEN_BOXFILTER; }
CatmullRomFilter		{ return REQUEST_TOKEN_CATMULLROMFILTER; }
Clipping			{ return REQUEST_TOKEN_CLIPPING; }
Color				{ return REQUEST_TOKEN_COLOR; }
ColorSamples			{ return REQUEST_TOKEN_COLORSAMPLES; }
ConcatTransform			{ return REQUEST_TOKEN_CONCATTRANSFORM; }
Cone				{ return REQUEST_TOKEN_CONE; }
CoordSysTransform		{ return REQUEST_TOKEN_COORDSYSTRANSFORM; }
CoordinateSystem		{ return REQUEST_TOKEN_COORDINATESYSTEM; }
CropWindow			{ return REQUEST_TOKEN_CROPWINDOW; }
Curves      			{ return REQUEST_TOKEN_CURVES; }
Cylinder			{ return REQUEST_TOKEN_CYLINDER; }
Declare				{ return REQUEST_TOKEN_DECLARE; }
Deformation			{ return REQUEST_TOKEN_DEFORMATION; }
DepthOfField			{ return REQUEST_TOKEN_DEPTHOFFIELD; }
Detail				{ return REQUEST_TOKEN_DETAIL; }
DetailRange			{ return REQUEST_TOKEN_DETAILRANGE; }
Disk				{ return REQUEST_TOKEN_DISK; }
DiskFilter			{ return REQUEST_TOKEN_DISKFILTER; }
Displacement			{ return REQUEST_TOKEN_DISPLACEMENT; }
Display				{ return REQUEST_TOKEN_DISPLAY; }
Else				{ return REQUEST_TOKEN_ELSE; }
ElseIf				{ return REQUEST_TOKEN_ELSEIF; }
ErrorHandler			{ return REQUEST_TOKEN_ERRORHANDLER; }
Exposure			{ return REQUEST_TOKEN_EXPOSURE; }
Exterior			{ return REQUEST_TOKEN_EXTERIOR; }
Format				{ return REQUEST_TOKEN_FORMAT; }
FrameAspectRatio		{ return REQUEST_TOKEN_FRAMEASPECTRATIO; }
FrameBegin			{ return REQUEST_TOKEN_FRAMEBEGIN; }
FrameEnd			{ return REQUEST_TOKEN_FRAMEEND; }
GaussianFilter			{ return REQUEST_TOKEN_GAUSSIANFILTER; }
GeneralPolygon			{ return REQUEST_TOKEN_GENERALPOLYGON; }
GeometricApproximation		{ return REQUEST_TOKEN_GEOMETRICAPPROXIMATION; }
Geometry			{ return REQUEST_TOKEN_GEOMETRY; }
Hider				{ return REQUEST_TOKEN_HIDER; }
Hyperboloid			{ return REQUEST_TOKEN_HYPERBOLOID; }
Identity			{ return REQUEST_TOKEN_IDENTITY; }
Illuminate			{ return REQUEST_TOKEN_ILLUMINATE; }
Imager				{ return REQUEST_TOKEN_IMAGER; }
Interior			{ return REQUEST_TOKEN_INTERIOR; }
IfBegin				{ return REQUEST_TOKEN_IFBEGIN; }
IfEnd				{ return REQUEST_TOKEN_IFEND; }
LightSource			{ return REQUEST_TOKEN_LIGHTSOURCE; }
MakeBump			{ return REQUEST_TOKEN_MAKEBUMP; }
MakeCubeFaceEnvironment 	{ return REQUEST_TOKEN_MAKECUBEFACEENVIRONMENT; }
MakeLatLongEnvironment		{ return REQUEST_TOKEN_MAKELATLONGENVIRONMENT; }
MakeOcclusion			{ return REQUEST_TOKEN_MAKEOCCLUSION; }
MakeShadow			{ return REQUEST_TOKEN_MAKESHADOW; }
MakeTexture			{ return REQUEST_TOKEN_MAKETEXTURE; }
Matte				{ return REQUEST_TOKEN_MATTE; }
MotionBegin			{ return REQUEST_TOKEN_MOTIONBEGIN; }
MotionEnd			{ return REQUEST_TOKEN_MOTIONEND; }
NuPatch				{ return REQUEST_TOKEN_NUPATCH; }
ObjectBegin			{ return REQUEST_TOKEN_OBJECTBEGIN; }
ObjectEnd			{ return REQUEST_TOKEN_OBJECTEND; }
ObjectInstance			{ return REQUEST_TOKEN_OBJECTINSTANCE; }
Opacity				{ return REQUEST_TOKEN_OPACITY; }
Option				{ return REQUEST_TOKEN_OPTION; }
Orientation			{ return REQUEST_TOKEN_ORIENTATION; }
Paraboloid			{ return REQUEST_TOKEN_PARABOLOID; }
Patch				{ return REQUEST_TOKEN_PATCH; }
PatchMesh			{ return REQUEST_TOKEN_PATCHMESH; }
Perspective			{ return REQUEST_TOKEN_PERSPECTIVE; }
PixelFilter			{ return REQUEST_TOKEN_PIXELFILTER; }
PixelSamples			{ return REQUEST_TOKEN_PIXELSAMPLES; }
PixelVariance			{ return REQUEST_TOKEN_PIXELVARIANCE; }
Points      			{ return REQUEST_TOKEN_POINTS; }
PointsGeneralPolygons		{ return REQUEST_TOKEN_POINTSGENERALPOLYGONS; }
PointsPolygons			{ return REQUEST_TOKEN_POINTSPOLYGONS; }
Polygon				{ return REQUEST_TOKEN_POLYGON; }
Procedural			{ return REQUEST_TOKEN_PROCEDURAL; }
Projection			{ return REQUEST_TOKEN_PROJECTION; }
Quantize			{ return REQUEST_TOKEN_QUANTIZE; }
ReadArchive			{ return REQUEST_TOKEN_READARCHIVE; }
RelativeDetail			{ return REQUEST_TOKEN_RELATIVEDETAIL; }
ReverseOrientation		{ return REQUEST_TOKEN_REVERSEORIENTATION; }
Rotate				{ return REQUEST_TOKEN_ROTATE; }
Scale				{ return REQUEST_TOKEN_SCALE; }
ScreenWindow			{ return REQUEST_TOKEN_SCREENWINDOW; }
ShadingInterpolation		{ return REQUEST_TOKEN_SHADINGINTERPOLATION; }
ShadingRate			{ return REQUEST_TOKEN_SHADINGRATE; }
Shutter				{ return REQUEST_TOKEN_SHUTTER; }
Sides				{ return REQUEST_TOKEN_SIDES; }
SincFilter			{ return REQUEST_TOKEN_SINCFILTER; }
Skew				{ return REQUEST_TOKEN_SKEW; }
SolidBegin			{ return REQUEST_TOKEN_SOLIDBEGIN; }
SolidEnd			{ return REQUEST_TOKEN_SOLIDEND; }
Sphere				{ return REQUEST_TOKEN_SPHERE; }
SubdivisionMesh			{ return REQUEST_TOKEN_SUBDIVISIONMESH; }
Surface				{ return REQUEST_TOKEN_SURFACE; }
TextureCoordinates		{ return REQUEST_TOKEN_TEXTURECOORDINATES; }
Torus				{ return REQUEST_TOKEN_TORUS; }
Transform			{ return REQUEST_TOKEN_TRANSFORM; }
TransformBegin			{ return REQUEST_TOKEN_TRANSFORMBEGIN; }
TransformEnd			{ return REQUEST_TOKEN_TRANSFORMEND; }
TransformPoints			{ return REQUEST_TOKEN_TRANSFORMPOINTS; }
Translate			{ return REQUEST_TOKEN_TRANSLATE; }
TriangleFilter			{ return REQUEST_TOKEN_TRIANGLEFILTER; }
TrimCurve			{ return REQUEST_TOKEN_TRIMCURVE; }
WorldBegin			{ return REQUEST_TOKEN_WORLDBEGIN; }
WorldEnd			{ return REQUEST_TOKEN_WORLDEND; }
ShaderLayer			{ return REQUEST_TOKEN_SHADERLAYER; }
ConnectShaderLayers { return REQUEST_TOKEN_CONNECTSHADERLAYERS; }
version				{ return REQUEST_TOKEN_VERSION; }

{letter}({letter}|{digit})*	{ return UNKNOWN_TOKEN; }
.				{ return(yytext[0]); }

<<EOF>>			{
				yyterminate();
				return EOF_TOKEN;
			}


%%

/* -------------- code section -------------- */

int yywrap()
{
	return 1;
}

static int scannerinput(char* Buffer, int MaxSize)
{
	// Sanity checks ...
	assert(BinaryDecoder);
	assert(Buffer);
	assert(MaxSize);

	int count = 0;
	if(!BinaryDecoder->eof())
		count=BinaryDecoder->read(Buffer,MaxSize);

 	return count;
}

struct yy_buffer_state* current_flex_buffer(void)
{
        return YY_CURRENT_BUFFER;
}
