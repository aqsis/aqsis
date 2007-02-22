/* -------------- declaration section -------------- */
%expect 47

%{

#ifdef	WIN32
#pragma warning(disable : 4786)
#include <cstdio>
#include <memory>
#include <malloc.h>
namespace std
{ using ::size_t; 
  using ::malloc;
  using ::free;
}
#endif

#include "librib.h"
using namespace librib;

#include "libribtypes.h"
#include "logging.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <cassert>

#ifdef	_DEBUG
#define	YYDEBUG	1
#endif

namespace librib
{

void ParserDeclare(librib::RendermanInterface* CallbackInterface, const std::string Name, const std::string Type);
extern std::string ParseStreamName;
extern RendermanInterface* ParseCallbackInterface;
extern std::ostream* ParseErrorStream;
extern unsigned int ParseLineNumber;
extern bool ParseSucceeded;
extern bool fRequest;
extern bool fParams;
extern bool fSkipFrame;
extern RtArchiveCallback pArchiveCallback;

bool fRecovering;

}; // namespace librib


%}

%pure_parser

%union
{
	int itype;
	float ftype;
	char* stype;
	char ctype;

	IntegerArray* aitype;
	FloatArray* aftype;
	StringArray* astype;
	TokenValuePair* tvtype;
	TokenValuePairs* atvtype;
}


%{

extern int yylex( YYSTYPE* );

static void yyerror(const std::string Message);

static void ExpectRequest();
static void ExpectParams();
static void SkipFrame();
static bool IsFrameSkipped(TqUint);
static void DiscardStringValue(char* const Value);
static void DiscardTokenValuePairs(TokenValuePairs* const Value);
static void DiscardArrayValue(Array* const Value);

static void DeclareLightHandle(const RendermanInterface::RtInt ID, const RendermanInterface::RtLightHandle Handle);
static RendermanInterface::RtLightHandle LookupLightHandle(const RendermanInterface::RtInt ID);

static void DeclareLightHandleString(const RendermanInterface::RtString ID, const RendermanInterface::RtLightHandle Handle);
static RendermanInterface::RtLightHandle LookupLightHandleString(const RendermanInterface::RtString ID);

static void DeclareObjectHandle(const RendermanInterface::RtInt ID, const RendermanInterface::RtObjectHandle Handle);
static RendermanInterface::RtObjectHandle LookupObjectHandle(const RendermanInterface::RtInt ID);

static void DeclareObjectHandleString(const RendermanInterface::RtString ID, const RendermanInterface::RtObjectHandle Handle);
static RendermanInterface::RtObjectHandle LookupObjectHandleString(const RendermanInterface::RtString ID);

static ParameterType DeclareParameterType(const std::string& Name, const std::string& Type);
static ParameterType LookupParameterType(const std::string& Name);

static RendermanInterface::RtInt ColorSamples = 3;

%}

%token <ctype> EOL_TOKEN EOF_TOKEN CHAR_TOKEN UNKNOWN_TOKEN INVALID_VALUE
%token <ftype> FLOAT_TOKEN
%token <itype> INTEGER_TOKEN
%token <stype> STRING_TOKEN
%token <stype> COMMENT
%type <itype> integer
%type <ftype> float
%type <stype> string
%type <aftype> scalar_array scalar_array_or_empty
%type <aftype> floats float_array
%type <aitype> integers integer_array integer_array_or_empty
%type <astype> strings string_array string_array_or_empty 
%type <tvtype> tvpair
%type <atvtype> tvpairs
%type <atvtype> opttvpairs

%token	<ctype>	REQUEST_TOKEN_AREALIGHTSOURCE
%token	<ctype>	REQUEST_TOKEN_ATMOSPHERE
%token	<ctype>	REQUEST_TOKEN_ATTRIBUTE
%token	<ctype>	REQUEST_TOKEN_ATTRIBUTEBEGIN
%token	<ctype>	REQUEST_TOKEN_ATTRIBUTEEND
%token	<ctype>	REQUEST_TOKEN_BASIS
%token	<ctype>	REQUEST_TOKEN_BESSELFILTER
%token	<ctype>	REQUEST_TOKEN_BLOBBY
%token	<ctype>	REQUEST_TOKEN_BOUND
%token	<ctype>	REQUEST_TOKEN_BOXFILTER
%token	<ctype>	REQUEST_TOKEN_CATMULLROMFILTER
%token	<ctype>	REQUEST_TOKEN_CLIPPING
%token	<ctype>	REQUEST_TOKEN_COLOR
%token	<ctype>	REQUEST_TOKEN_COLORSAMPLES
%token	<ctype>	REQUEST_TOKEN_CONCATTRANSFORM
%token	<ctype>	REQUEST_TOKEN_CONE
%token	<ctype>	REQUEST_TOKEN_COORDINATESYSTEM
%token	<ctype>	REQUEST_TOKEN_COORDSYSTRANSFORM
%token	<ctype>	REQUEST_TOKEN_CROPWINDOW
%token	<ctype>	REQUEST_TOKEN_CURVES
%token	<ctype>	REQUEST_TOKEN_CYLINDER
%token	<ctype>	REQUEST_TOKEN_DECLARE
%token	<ctype>	REQUEST_TOKEN_DEFORMATION
%token	<ctype>	REQUEST_TOKEN_DEPTHOFFIELD
%token	<ctype>	REQUEST_TOKEN_DETAIL
%token	<ctype>	REQUEST_TOKEN_DETAILRANGE
%token	<ctype>	REQUEST_TOKEN_DISK
%token	<ctype>	REQUEST_TOKEN_DISKFILTER
%token	<ctype>	REQUEST_TOKEN_DISPLACEMENT
%token	<ctype>	REQUEST_TOKEN_DISPLAY
%token	<ctype>	REQUEST_TOKEN_ELSE
%token	<ctype>	REQUEST_TOKEN_ELSEIF
%token	<ctype>	REQUEST_TOKEN_ERRORABORT
%token	<ctype>	REQUEST_TOKEN_ERRORHANDLER
%token	<ctype>	REQUEST_TOKEN_ERRORIGNORE
%token	<ctype>	REQUEST_TOKEN_ERRORPRINT
%token	<ctype>	REQUEST_TOKEN_EXPOSURE
%token	<ctype>	REQUEST_TOKEN_EXTERIOR
%token	<ctype>	REQUEST_TOKEN_FORMAT
%token	<ctype>	REQUEST_TOKEN_FRAMEASPECTRATIO
%token	<ctype>	REQUEST_TOKEN_FRAMEBEGIN
%token	<ctype>	REQUEST_TOKEN_FRAMEEND
%token	<ctype>	REQUEST_TOKEN_GAUSSIANFILTER
%token	<ctype>	REQUEST_TOKEN_GENERALPOLYGON
%token	<ctype>	REQUEST_TOKEN_GEOMETRICAPPROXIMATION
%token	<ctype>	REQUEST_TOKEN_GEOMETRY
%token	<ctype>	REQUEST_TOKEN_HIDER
%token	<ctype>	REQUEST_TOKEN_HYPERBOLOID
%token	<ctype>	REQUEST_TOKEN_IDENTITY
%token	<ctype>	REQUEST_TOKEN_IFBEGIN
%token	<ctype>	REQUEST_TOKEN_IFEND
%token	<ctype>	REQUEST_TOKEN_ILLUMINATE
%token	<ctype>	REQUEST_TOKEN_IMAGER
%token	<ctype>	REQUEST_TOKEN_INTERIOR
%token	<ctype>	REQUEST_TOKEN_LIGHTSOURCE
%token	<ctype>	REQUEST_TOKEN_MAKEOCCLUSION
%token	<ctype>	REQUEST_TOKEN_MAKEBUMP
%token	<ctype>	REQUEST_TOKEN_MAKECUBEFACEENVIRONMENT
%token	<ctype>	REQUEST_TOKEN_MAKELATLONGENVIRONMENT
%token	<ctype>	REQUEST_TOKEN_MAKESHADOW
%token	<ctype>	REQUEST_TOKEN_MAKETEXTURE
%token	<ctype>	REQUEST_TOKEN_MATTE
%token	<ctype>	REQUEST_TOKEN_MITCHELLFILTER
%token	<ctype>	REQUEST_TOKEN_MOTIONBEGIN
%token	<ctype>	REQUEST_TOKEN_MOTIONEND
%token	<ctype>	REQUEST_TOKEN_NUPATCH
%token	<ctype>	REQUEST_TOKEN_OBJECTBEGIN
%token	<ctype>	REQUEST_TOKEN_OBJECTEND
%token	<ctype>	REQUEST_TOKEN_OBJECTINSTANCE
%token	<ctype>	REQUEST_TOKEN_OPACITY
%token	<ctype>	REQUEST_TOKEN_OPTION
%token	<ctype>	REQUEST_TOKEN_ORIENTATION
%token	<ctype>	REQUEST_TOKEN_PARABOLOID
%token	<ctype>	REQUEST_TOKEN_PATCH
%token	<ctype>	REQUEST_TOKEN_PATCHMESH
%token	<ctype>	REQUEST_TOKEN_PERSPECTIVE
%token	<ctype>	REQUEST_TOKEN_PIXELFILTER
%token	<ctype>	REQUEST_TOKEN_PIXELSAMPLES
%token	<ctype>	REQUEST_TOKEN_PIXELVARIANCE
%token	<ctype>	REQUEST_TOKEN_POINTS
%token	<ctype>	REQUEST_TOKEN_POINTSGENERALPOLYGONS
%token	<ctype>	REQUEST_TOKEN_POINTSPOLYGONS
%token	<ctype>	REQUEST_TOKEN_POLYGON
%token	<ctype>	REQUEST_TOKEN_PROCEDURAL
%token	<ctype>	REQUEST_TOKEN_PROJECTION
%token  <ctype> REQUEST_TOKEN_RESOURCE
%token  <ctype> REQUEST_TOKEN_RESOURCEBEGIN
%token  <ctype> REQUEST_TOKEN_RESOURCEEND
%token	<ctype>	REQUEST_TOKEN_QUANTIZE
%token	<ctype>	REQUEST_TOKEN_READARCHIVE
%token	<ctype>	REQUEST_TOKEN_RELATIVEDETAIL
%token	<ctype>	REQUEST_TOKEN_REVERSEORIENTATION
%token	<ctype>	REQUEST_TOKEN_ROTATE
%token	<ctype>	REQUEST_TOKEN_SCALE
%token	<ctype>	REQUEST_TOKEN_SCREENWINDOW
%token	<ctype>	REQUEST_TOKEN_SHADINGINTERPOLATION
%token	<ctype>	REQUEST_TOKEN_SHADINGRATE
%token	<ctype>	REQUEST_TOKEN_SHUTTER
%token	<ctype>	REQUEST_TOKEN_SIDES
%token	<ctype>	REQUEST_TOKEN_SINCFILTER
%token	<ctype>	REQUEST_TOKEN_SKEW
%token	<ctype>	REQUEST_TOKEN_SOLIDBEGIN
%token	<ctype>	REQUEST_TOKEN_SOLIDEND
%token	<ctype>	REQUEST_TOKEN_SPHERE
%token	<ctype>	REQUEST_TOKEN_SUBDIVISIONMESH
%token	<ctype>	REQUEST_TOKEN_SURFACE
%token	<ctype>	REQUEST_TOKEN_TEXTURECOORDINATES
%token	<ctype>	REQUEST_TOKEN_TORUS
%token	<ctype>	REQUEST_TOKEN_TRANSFORM
%token	<ctype>	REQUEST_TOKEN_TRANSFORMBEGIN
%token	<ctype>	REQUEST_TOKEN_TRANSFORMEND
%token	<ctype>	REQUEST_TOKEN_TRANSFORMPOINTS
%token	<ctype>	REQUEST_TOKEN_TRANSLATE
%token	<ctype>	REQUEST_TOKEN_TRIANGLEFILTER
%token	<ctype>	REQUEST_TOKEN_TRIMCURVE
%token	<ctype>	REQUEST_TOKEN_VERSION
%token	<ctype>	REQUEST_TOKEN_WORLDBEGIN
%token	<ctype>	REQUEST_TOKEN_WORLDEND
%token	<ctype> REQUEST_TOKEN_SHADERLAYER
%token	<ctype> REQUEST_TOKEN_CONNECTSHADERLAYERS


%type	<ctype>	arealightsource
%type	<ctype>	atmosphere
%type	<ctype>	attribute
%type	<ctype>	attributebegin
%type	<ctype>	attributeend
%type	<ctype>	basis
%type	<ctype>	blobby
%type	<ctype>	bound
%type	<ctype>	clipping
%type	<ctype>	color
%type	<ctype>	colorsamples
%type	<ctype>	concattransform
%type	<ctype>	cone
%type	<ctype>	coordinatesystem
%type	<ctype>	coordsystransform
%type	<ctype>	cropwindow
%type	<ctype>	curves
%type	<ctype>	cylinder
%type	<ctype>	declare
%type	<ctype>	deformation
%type	<ctype>	depthoffield
%type	<ctype>	detail
%type	<ctype>	detailrange
%type	<ctype>	disk
%type	<ctype>	displacement
%type	<ctype>	display
%type	<ctype>	else
%type	<ctype>	elseif
%type	<ctype>	errorabort
%type	<ctype>	errorhandler
%type	<ctype>	errorignore
%type	<ctype>	errorprint
%type	<ctype>	exposure
%type	<ctype>	exterior
%type	<ctype>	format
%type	<ctype>	frameaspectratio
%type	<ctype>	framebegin
%type	<ctype>	frameend
%type	<ctype>	generalpolygon
%type	<ctype>	geometricapproximation
%type	<ctype>	geometry
%type	<ctype>	hider
%type	<ctype>	hyperboloid
%type	<ctype>	identity
%type	<ctype>	ifbegin
%type	<ctype>	ifend
%type	<ctype>	illuminate
%type	<ctype>	imager
%type	<ctype>	interior
%type	<ctype>	lightsource
%type	<ctype>	makeocclusion
%type	<ctype>	makebump
%type	<ctype>	makecubefaceenvironment
%type	<ctype>	makelatlongenvironment
%type	<ctype>	makeshadow
%type	<ctype>	maketexture
%type	<ctype>	matte
%type	<ctype>	motionbegin
%type	<ctype>	motionend
%type	<ctype>	nupatch
%type	<ctype>	objectbegin
%type	<ctype>	objectend
%type	<ctype>	objectinstance
%type	<ctype>	opacity
%type	<ctype>	option
%type	<ctype>	orientation
%type	<ctype>	procedural
%type	<ctype>	paraboloid
%type	<ctype>	patch
%type	<ctype>	patchmesh
%type	<ctype>	perspective
%type	<ctype>	pixelfilter
%type	<ctype>	pixelsamples
%type	<ctype>	pixelvariance
%type	<ctype>	points
%type	<ctype>	pointsgeneralpolygons
%type	<ctype>	pointspolygons
%type	<ctype>	polygon
%type	<ctype>	projection
%type	<ctype>	quantize
%type	<ctype>	readarchive
%type	<ctype>	relativedetail
%type	<ctype>	reverseorientation
%type	<ctype>	rotate
%type	<ctype> resource
%type	<ctype>	resourcebegin
%type	<ctype>	resourceend
%type	<ctype>	scale
%type	<ctype>	screenwindow
%type	<ctype>	shadinginterpolation
%type	<ctype>	shadingrate
%type	<ctype>	shutter
%type	<ctype>	sides
%type	<ctype>	skew
%type	<ctype>	solidbegin
%type	<ctype>	solidend
%type	<ctype>	sphere
%type	<ctype>	subdivisionmesh
%type	<ctype>	surface
%type	<ctype>	texturecoordinates
%type	<ctype>	torus
%type	<ctype>	transform
%type	<ctype>	transformbegin
%type	<ctype>	transformend
%type	<ctype>	transformpoints
%type	<ctype>	translate
%type	<ctype>	trimcurve
%type	<ctype>	version
%type	<ctype>	worldbegin
%type	<ctype>	worldend
%type	<ctype> shaderlayer
%type	<ctype> connectshaderlayers

/* -------------- rules section -------------- */
%%
file : requests
			{ YYABORT; }
	;

requests : request
	|	requests request
	;

request : complete_request
			{ 
				ExpectRequest(); 
				fRecovering = false;
			}
	|	COMMENT
			{
				// This is used to mark the end of a RunProgram chunk
				// TODO: limit this to within a RunProgram only
				if (strchr(yyvsp[0].stype, 0377))
				{
					DiscardStringValue($1);
					ExpectRequest(); 
					fRecovering = false;
					YYACCEPT;	
				};

				if( pArchiveCallback != NULL )
				{
					// RISpec 3.2 isn't very clear, not sure if stripping the '#'s is right or not
					if( $1[0] == '#' && $1[1] == '#' ) // Structure comment
					{
						pArchiveCallback("structure", "%s", &( $1[2]) ); 
					} else {  // User Data Record comment
						pArchiveCallback("comment", "%s", &( $1[1]) ); 
					};
				};
				DiscardStringValue($1);

				ExpectRequest(); 
				fRecovering = false;
			}
	;

complete_request
	:	version float
			{  }
	|	declare string string
			{
				ParserDeclare(ParseCallbackInterface, $2, $3);
				DiscardStringValue($2);
				DiscardStringValue($3);
			}
	|	framebegin integer
			{ 
				/* Check if the frame should be skipped */
				if(IsFrameSkipped($2))
					SkipFrame();
				else
					ParseCallbackInterface->RiFrameBegin($2);
			}
	|	frameend
			{ ParseCallbackInterface->RiFrameEnd(); }
	|	worldbegin
			{ ParseCallbackInterface->RiWorldBegin(); }
	|	worldend
			{ ParseCallbackInterface->RiWorldEnd(); /* YYACCEPT; */ }
	|	format integer integer float
			{ ParseCallbackInterface->RiFormat($2, $3, $4); }
	|	frameaspectratio float
			{ ParseCallbackInterface->RiFrameAspectRatio($2); }
	|	screenwindow float float float float
			{ ParseCallbackInterface->RiScreenWindow($2, $3, $4, $5); }
	|	cropwindow float float float float
			{ ParseCallbackInterface->RiCropWindow($2, $3, $4, $5); }
	|	projection string opttvpairs
			{
				ParseCallbackInterface->RiProjectionV($2, $3->Count(), $3->Tokens(), $3->Values());
				DiscardStringValue($2);
				DiscardTokenValuePairs($3);
			}
	|	clipping float float
			{ ParseCallbackInterface->RiClipping($2, $3); }
	|	depthoffield float float float
			{ ParseCallbackInterface->RiDepthOfField($2, $3, $4); }
	|	shutter float float
			{ ParseCallbackInterface->RiShutter($2, $3); }
	|	pixelvariance float
			{ ParseCallbackInterface->RiPixelVariance($2); }
	|	pixelsamples float float
			{ ParseCallbackInterface->RiPixelSamples($2, $3); }
	|	pixelfilter string float float
			{
				RendermanInterface::RtFilterFunc pFilter=ParseCallbackInterface->GetFilterFunction($2);
				ParseCallbackInterface->RiPixelFilter(pFilter, $3, $4);

				DiscardStringValue($2);
			}
	|	else 
			{
				ParseCallbackInterface->RiElse();
			}
	|	elseif string
			{
				ParseCallbackInterface->RiElseIf($2);
				DiscardStringValue($2);
			}
	|	exposure float float
			{ ParseCallbackInterface->RiExposure($2, $3); }
	|	imager string opttvpairs
			{
				ParseCallbackInterface->RiImagerV($2, $3->Count(), $3->Tokens(), $3->Values());
				DiscardStringValue($2);
				DiscardTokenValuePairs($3);
			}
	|	quantize string integer integer integer float
			{
				ParseCallbackInterface->RiQuantize((char*)($2), $3, $4, $5, $6);
				DiscardStringValue($2);
			}
	|	display string string string opttvpairs
			{
				ParseCallbackInterface->RiDisplayV($2, const_cast<char*>($3), const_cast<char*>($4), $5->Count(), $5->Tokens(), $5->Values());
				DiscardStringValue($2);
				DiscardStringValue($3);
				DiscardStringValue($4);
				DiscardTokenValuePairs($5);
			}
	|	hider string opttvpairs
			{
				ParseCallbackInterface->RiHiderV(const_cast<char*>($2), $3->Count(), $3->Tokens(), $3->Values());
				DiscardStringValue($2);
				DiscardTokenValuePairs($3);
			}
	|	colorsamples scalar_array scalar_array
			{
				ColorSamples = $2->Count();
				ParseCallbackInterface->RiColorSamples($2->Count(), &(*$2)[0], &(*$3)[0]);
				DiscardArrayValue($2);
				DiscardArrayValue($3);
			}
	|	relativedetail float
			{ ParseCallbackInterface->RiRelativeDetail($2); }
	|	option string opttvpairs
			{
				ParseCallbackInterface->RiOptionV($2, $3->Count(), $3->Tokens(), $3->Values());
				DiscardStringValue($2);
				DiscardTokenValuePairs($3);
			}
	|	attributebegin
			{ ParseCallbackInterface->RiAttributeBegin(); }
	|	attributeend
			{ ParseCallbackInterface->RiAttributeEnd(); }
	|	color scalar_array
			{
				if($2->Count() == ColorSamples)
					ParseCallbackInterface->RiColor(&(*$2)[0]);
				else
					yyerror("Incorrect Number of Color Samples");
					
				DiscardArrayValue($2);
			}
	|	color float float float
			{
				if(3 == ColorSamples)
					{
						RendermanInterface::RtColor Cq;
						Cq[0] = $2;
						Cq[1] = $3;
						Cq[2] = $4;
						ParseCallbackInterface->RiColor(Cq);
					}
				else
					{
						yyerror("Incorrect Number of Color Samples");
					}
			}
	|	opacity scalar_array
			{
				if($2->Count() == ColorSamples)
					ParseCallbackInterface->RiOpacity(&(*$2)[0]);
				else
					yyerror("Incorrect Number of Color Samples");
					
				DiscardArrayValue($2);
			}
	|	opacity float float float
			{
				if(3 == ColorSamples)
					{
						RendermanInterface::RtColor Cq;
						Cq[0]=$2;
						Cq[1]=$3;
						Cq[2]=$4;
						ParseCallbackInterface->RiOpacity(Cq);
					}
				else
					{
						yyerror("Incorrect Number of Color Samples");
					}
			}
	|	texturecoordinates float float float float float float float float
			{ ParseCallbackInterface->RiTextureCoordinates($2, $3, $4, $5, $6, $7, $8, $9); }
	|	texturecoordinates scalar_array
			{
				if($2->Count() == 8)
					ParseCallbackInterface->RiTextureCoordinates((*$2)[0], (*$2)[1], (*$2)[2], (*$2)[3], (*$2)[4], (*$2)[5], (*$2)[6], (*$2)[7]);
				else
					yyerror("Expecting 8 Values");
					
				DiscardArrayValue($2);
			}
	|	lightsource string integer opttvpairs
			{
				DeclareLightHandle($3, ParseCallbackInterface->RiLightSourceV($2, $4->Count(), $4->Tokens(), $4->Values()));
				DiscardStringValue($2);
				DiscardTokenValuePairs($4);
			}
	|	lightsource string string opttvpairs
			{
				DeclareLightHandleString($3, ParseCallbackInterface->RiLightSourceV($2, $4->Count(), $4->Tokens(), $4->Values()));
				DiscardStringValue($2);
				DiscardStringValue($3);
				DiscardTokenValuePairs($4);
			}
	|	arealightsource string integer opttvpairs
			{
				DeclareLightHandle($3, ParseCallbackInterface->RiAreaLightSourceV($2, $4->Count(), $4->Tokens(), $4->Values()));
				DiscardStringValue($2);
				DiscardTokenValuePairs($4);
			}
	|	arealightsource string string opttvpairs
			{
				DeclareLightHandleString($3, ParseCallbackInterface->RiAreaLightSourceV($2, $4->Count(), $4->Tokens(), $4->Values()));
				DiscardStringValue($2);
				DiscardStringValue($3);
				DiscardTokenValuePairs($4);
			}
	|	ifbegin string
			{
				ParseCallbackInterface->RiIfBegin($2);
				DiscardStringValue($2);
			}
	|	ifend 
			{
				ParseCallbackInterface->RiIfEnd();
			}
	|	illuminate integer integer
			{
				RendermanInterface::RtLightHandle handle = LookupLightHandle($2);
				if(handle)
					ParseCallbackInterface->RiIlluminate(handle, $3 != 0);
				else
					yyerror("Undeclared Light");
			}
	|	illuminate string integer
			{
				RendermanInterface::RtLightHandle handle = LookupLightHandleString($2);
				if(handle)
					ParseCallbackInterface->RiIlluminate(handle, $3 != 0);
				else
					yyerror("Undeclared Light");
				DiscardStringValue($2);
			}
	|	surface string opttvpairs
			{
				ParseCallbackInterface->RiSurfaceV($2, $3->Count(), $3->Tokens(), $3->Values());
				DiscardStringValue($2);
				DiscardTokenValuePairs($3);
			}
	|	atmosphere string opttvpairs
			{
				ParseCallbackInterface->RiAtmosphereV($2, $3->Count(), $3->Tokens(), $3->Values());
				DiscardStringValue($2);
				DiscardTokenValuePairs($3);
			}
	|	interior string opttvpairs
			{
				ParseCallbackInterface->RiInteriorV($2, $3->Count(), $3->Tokens(), $3->Values());
				DiscardStringValue($2);
				DiscardTokenValuePairs($3);
			}
	|	exterior string opttvpairs
			{
				ParseCallbackInterface->RiExteriorV($2, $3->Count(), $3->Tokens(), $3->Values());
				DiscardStringValue($2);
				DiscardTokenValuePairs($3);
			}
	|	shadingrate float
			{ ParseCallbackInterface->RiShadingRate($2); }
	|	shadinginterpolation string
			{
				ParseCallbackInterface->RiShadingInterpolation((char*)($2));
				DiscardStringValue($2);
			}
	|	matte integer
			{ ParseCallbackInterface->RiMatte($2!=0); }
	|	bound	float float float float float float
			{
				RendermanInterface::RtBound b;
				b[0]=$2; b[1]=$3; b[2]=$4; b[3]=$5; b[4]=$6; b[5]=$7;
				ParseCallbackInterface->RiBound(b);
			}
	|	bound	scalar_array
			{
				if($2->Count() == 6)
					ParseCallbackInterface->RiBound(&(*$2)[0]);
				else
					yyerror("Expecting 6 Values");
					
				DiscardArrayValue($2);
			}
	|	detail float float float float float float
			{
				RendermanInterface::RtBound b;
				b[0]=$2; b[1]=$3; b[2]=$4; b[3]=$5; b[4]=$6; b[5]=$7;
				ParseCallbackInterface->RiDetail(b);
			}
	|	detail scalar_array
			{
				if($2->Count() == 6)
					ParseCallbackInterface->RiDetail(&(*$2)[0]);
				else
					yyerror("Expecting 6 Values");
				DiscardArrayValue($2);
			}
	|	detailrange float float float float
			{ ParseCallbackInterface->RiDetailRange($2, $3, $4, $5); }
	|	detailrange scalar_array
			{
				if($2->Count() == 4)
					ParseCallbackInterface->RiDetailRange((*$2)[0], (*$2)[1], (*$2)[2], (*$2)[3]);
				else
					yyerror("Expecting 4 Values");
				DiscardArrayValue($2);
			}
	|	geometricapproximation string float
			{
				ParseCallbackInterface->RiGeometricApproximation((char*)($2), $3);
				DiscardStringValue($2);
			}
	|	geometricapproximation string scalar_array
			{
				// Note: This version is only included to allow invalid RIB files to work.
				ParseCallbackInterface->RiGeometricApproximation((char*)($2), (*$3)[0]);
				DiscardStringValue($2);
				DiscardArrayValue($3);
			}
	|	orientation string
			{
				ParseCallbackInterface->RiOrientation((char*)($2));
				DiscardStringValue($2);
			}
	|	reverseorientation
			{ ParseCallbackInterface->RiReverseOrientation(); }
	|	sides integer
			{ ParseCallbackInterface->RiSides($2); }
	|	identity
			{ ParseCallbackInterface->RiIdentity(); }
	|	transform scalar_array
			{
				RendermanInterface::RtMatrix Trans;
				if($2->Count() == 16)
					{
						Trans[0][0]=(*$2)[0];
						Trans[0][1]=(*$2)[1];
						Trans[0][2]=(*$2)[2];
						Trans[0][3]=(*$2)[3];
						Trans[1][0]=(*$2)[4];
						Trans[1][1]=(*$2)[5];
						Trans[1][2]=(*$2)[6];
						Trans[1][3]=(*$2)[7];
						Trans[2][0]=(*$2)[8];
						Trans[2][1]=(*$2)[9];
						Trans[2][2]=(*$2)[10];
						Trans[2][3]=(*$2)[11];
						Trans[3][0]=(*$2)[12];
						Trans[3][1]=(*$2)[13];
						Trans[3][2]=(*$2)[14];
						Trans[3][3]=(*$2)[15];
						ParseCallbackInterface->RiTransform(Trans);
					}
				else
					{
						yyerror("Expecting 16 Values");
					}
				DiscardArrayValue($2);
			}
	|	concattransform scalar_array
			{
				RendermanInterface::RtMatrix Trans;
				if($2->Count() == 16)
					{
						Trans[0][0]=(*$2)[0];
						Trans[0][1]=(*$2)[1];
						Trans[0][2]=(*$2)[2];
						Trans[0][3]=(*$2)[3];
						Trans[1][0]=(*$2)[4];
						Trans[1][1]=(*$2)[5];
						Trans[1][2]=(*$2)[6];
						Trans[1][3]=(*$2)[7];
						Trans[2][0]=(*$2)[8];
						Trans[2][1]=(*$2)[9];
						Trans[2][2]=(*$2)[10];
						Trans[2][3]=(*$2)[11];
						Trans[3][0]=(*$2)[12];
						Trans[3][1]=(*$2)[13];
						Trans[3][2]=(*$2)[14];
						Trans[3][3]=(*$2)[15];
						ParseCallbackInterface->RiConcatTransform(Trans);
					}
				else
					{
						yyerror("Expecting 16 Values");
					}
				DiscardArrayValue($2);
			}
	|	perspective float
			{ ParseCallbackInterface->RiPerspective($2); }
	|	translate	float float float
			{ ParseCallbackInterface->RiTranslate($2, $3, $4); }
	|	rotate float float float float
			{ ParseCallbackInterface->RiRotate($2, $3, $4, $5); }
	|	scale float float float
			{ ParseCallbackInterface->RiScale($2, $3, $4); }
	|	skew float float float float float float float
			{ ParseCallbackInterface->RiSkew($2, $3, $4, $5, $6, $7, $8); }
	|	skew scalar_array
			{
				if($2->Count( ) == 7)
					ParseCallbackInterface->RiSkew((*$2)[0], (*$2)[1], (*$2)[2], (*$2)[3], (*$2)[4], (*$2)[5], (*$2)[6]);
				else
					yyerror("Expecting 7 Values");
				DiscardArrayValue($2);
			}
	|	deformation string opttvpairs
			{
				ParseCallbackInterface->RiDeformationV($2, $3->Count(), $3->Tokens(), $3->Values());
				DiscardStringValue($2);
				DiscardTokenValuePairs($3);
			}
	|	displacement string opttvpairs
			{
				ParseCallbackInterface->RiDisplacementV($2, $3->Count(), $3->Tokens(), $3->Values());
				DiscardStringValue($2);
				DiscardTokenValuePairs($3);
			}
	|	coordinatesystem string
			{
				ParseCallbackInterface->RiCoordinateSystem($2);
				DiscardStringValue($2);
			}
	|	coordsystransform string
			{
				ParseCallbackInterface->RiCoordSysTransform($2);
				DiscardStringValue($2);
			}
	|	transformpoints
			{ printf("TRANSFORMPOINTS\n"); }
	|	transformbegin
			{ ParseCallbackInterface->RiTransformBegin(); }
	|	transformend
			{ ParseCallbackInterface->RiTransformEnd(); }
	|	attribute string opttvpairs
			{
				ParseCallbackInterface->RiAttributeV($2, $3->Count(), $3->Tokens(), $3->Values());
				DiscardStringValue($2);
				DiscardTokenValuePairs($3);
			}
	|	polygon tvpairs
			{
				int vertices = 0;
				for(int i = 0; i < $2->Count(); i++)
					{
						if($2->Tokens()[i] == std::string("P"))
							vertices = $2->Counts()[i] / 3;
					}
				ParseCallbackInterface->RiPolygonV(vertices, $2->Count(), $2->Tokens(), $2->Values());
				DiscardTokenValuePairs($2);
			}
	|	generalpolygon integer_array tvpairs
			{
				ParseCallbackInterface->RiGeneralPolygonV($2->Count(), &(*$2)[0], $3->Count(), $3->Tokens(), $3->Values());
				DiscardArrayValue($2);
				DiscardTokenValuePairs($3);
			}
    |   curves string integer_array string opttvpairs
            {
                ParseCallbackInterface->RiCurvesV($2, $3->Count(), &(*$3)[0], $4, $5->Count(), $5->Tokens(), $5->Values());
                DiscardStringValue($2);
                DiscardArrayValue($3); 
                DiscardStringValue($4);  
		DiscardTokenValuePairs($5);
            }
    |   blobby integer integer_array float_array string_array opttvpairs
            {
                ParseCallbackInterface->RiBlobbyV($2, $3->Count(), &(*$3)[0], 
                        $4->Count(), &(*$4)[0],
                        $5->Count(), &(*$5)[0],
                        $6->Count(), $6->Tokens(), $6->Values());
                DiscardArrayValue($3);
                DiscardArrayValue($4);
                DiscardArrayValue($5); 
		        DiscardTokenValuePairs($6);
            }
    |   readarchive string 
	    {
                ParseCallbackInterface->RiReadArchive( $2, NULL );
		DiscardStringValue( $2 );
	    }
    |   procedural string string_array scalar_array opttvpairs
            {
                if (!(
                     (strcmp($2, "RunProgram") == 0) ||
                     (strcmp($2, "DelayedReadArchive") == 0) ||
                     (strcmp($2, "DynamicLoad") == 0) ) )
                     yyerror("Expect RunProgram, DelayedReadArchive or DynamicLoad"); 
		    
		        if($4->Count() != 6)
		           yyerror("Expecting 6 Values");

		// We jump through a few hoops here to meet the spec
		// The arguments should be free()'able, so we must allocate them in one
		// big lump
		size_t size = 0;
		int i;
		for(i=0; i<$3->Count(); i++)
		{
			size += sizeof( char* ); //one pointer for this entry
			size += strlen((*$3)[i]) + 1; //and space for the string
		}

		void* pdata = (void *) malloc( size );
		char* stringstart = (char*) ((char**) pdata + $3->Count() ) ;
		for(i=0; i<$3->Count(); i++)
		{
			((char**) pdata)[i] = stringstart;
			strcpy(stringstart, (*$3)[i]);
			stringstart += strlen( stringstart ) + 1;
		}

                RendermanInterface::RtFunc pFunc = ParseCallbackInterface->GetProceduralFunction( $2 );
                RendermanInterface::RtFunc pFreeFunc = ParseCallbackInterface->GetProceduralFunction( "RiProcFree" );
		ParseCallbackInterface->RiProcedural(pdata, &(*$4)[0], (void (*)(void ) ) pFunc, pFreeFunc);
                DiscardStringValue($2);
                DiscardArrayValue($3);
                DiscardArrayValue($4); 
	        DiscardTokenValuePairs($5);
            }
	|   points opttvpairs
            {
                int vertices = 0;
                for(int i = 0; i < $2->Count(); i++) 
                {
		    /// \todo: this will not work if "P" is specified as "vertex point P", need to investigate.
                    if($2->Tokens()[i] == std::string("P"))
                    vertices = $2->Counts()[i] / 3;
                }
                if (vertices > 0)
                    ParseCallbackInterface->RiPointsV(vertices, $2->Count(), $2->Tokens(), $2->Values());
                else 
                    yyerror("Expecting \"P\"");
                    
				DiscardTokenValuePairs($2);
            }
	|	pointspolygons integer_array integer_array tvpairs
			{
				ParseCallbackInterface->RiPointsPolygonsV($2->Count(), &(*$2)[0], &(*$3)[0], $4->Count(), $4->Tokens(), $4->Values());
				DiscardArrayValue($2);
				DiscardArrayValue($3);
				DiscardTokenValuePairs($4);
			}
	|	pointsgeneralpolygons	integer_array integer_array integer_array tvpairs
			{
				ParseCallbackInterface->RiPointsGeneralPolygonsV($2->Count(), &(*$2)[0], &(*$3)[0], &(*$4)[0], $5->Count(), $5->Tokens(), $5->Values());
				DiscardArrayValue($2);
				DiscardArrayValue($3);
				DiscardArrayValue($4);
				DiscardTokenValuePairs($5);
			}
	|	basis string integer string integer
			{

				RendermanInterface::RtBasis *u=ParseCallbackInterface->GetBasisMatrix($2);
				RendermanInterface::RtBasis *v=ParseCallbackInterface->GetBasisMatrix($4);
				if(u == 0)
					yyerror("Invalid uBasis \"" + std::string($2) + "\"");
				if(v == 0)
					yyerror("Invalid vBasis \"" + std::string($4) + "\"");
				if(u != 0 && v != 0)
					ParseCallbackInterface->RiBasis(*u, $3, *v, $5);
				DiscardStringValue($2);
				DiscardStringValue($4);

			}
	|	basis string integer scalar_array integer
			{

				if($4->Count() == 16)
					{
						RtBasis v;
						int i,j;
						for(i=0; i<4; i++)
							for(j=0; j<4; j++)
								v[i][j]=(*$4)[(i*4)+j];

						RendermanInterface::RtBasis *u=ParseCallbackInterface->GetBasisMatrix($2);
						ParseCallbackInterface->RiBasis(*u, $3,v, $5);
					}
				else
					{
						yyerror("Expecting 16 floats");
					}
				DiscardStringValue($2);
				DiscardArrayValue($4);

									}
	|	basis scalar_array integer string integer
			{

				if($2->Count() == 16)
					{

						RtBasis u;
						int i,j;
						for(i=0; i<4; i++)
							for(j=0; j<4; j++)
								u[i][j]=(*$2)[(i*4)+j];

						RendermanInterface::RtBasis *v=ParseCallbackInterface->GetBasisMatrix($4);
						ParseCallbackInterface->RiBasis(u, $3,*v, $5);
					}
				else
					{
						yyerror("Expecting 16 floats");
					}
				DiscardArrayValue($2);
				DiscardStringValue($4);

									}
	|	basis scalar_array integer scalar_array integer
			{
				if($2->Count( ) == 16 && $4->Count() == 16)
					{
						// Build a matrix for the u and v directions
						RendermanInterface::RtBasis u, v;
						int i,j;
						for(i=0; i<4; i++)
							{
								for(j=0; j<4; j++)
									{
										u[i][j]=(*$2)[(i*4)+j];
										v[i][j]=(*$4)[(i*4)+j];
									}
							}
						ParseCallbackInterface->RiBasis(u, $3,v, $5);
					}
				else
					{
						yyerror("Expecting 16 Values");
					}
				DiscardArrayValue($2);
				DiscardArrayValue($4);
			}
	|	patch string tvpairs
			{
				ParseCallbackInterface->RiPatchV(const_cast<char*>($2), $3->Count(), $3->Tokens(), $3->Values());
				DiscardStringValue($2);
				DiscardTokenValuePairs($3);
			}
	|	patchmesh	string integer string integer string tvpairs
			{
				ParseCallbackInterface->RiPatchMeshV(const_cast<char*>($2), $3, const_cast<char*>($4), $5, const_cast<char*>($6), $7->Count(), $7->Tokens(), $7->Values());
				DiscardStringValue($2);
				DiscardStringValue($4);
				DiscardStringValue($6);
				DiscardTokenValuePairs($7);
			}
	|	nupatch integer integer scalar_array float float integer integer scalar_array float float tvpairs
			{
				ParseCallbackInterface->RiNuPatchV($2, $3, &(*$4)[0], $5, $6, $7, $8, &(*$9)[0], $10, $11, $12->Count(), $12->Tokens(), $12->Values());
				DiscardArrayValue($4);
				DiscardArrayValue($9);
				DiscardTokenValuePairs($12);
			}
	|	trimcurve integer_array integer_array scalar_array scalar_array scalar_array integer_array scalar_array scalar_array scalar_array
			{
				ParseCallbackInterface->RiTrimCurve($2->Count(), &(*$2)[0], &(*$3)[0], &(*$4)[0], &(*$5)[0], &(*$6)[0], &(*$7)[0], &(*$8)[0], &(*$9)[0], &(*$10)[0]);
				DiscardArrayValue($2);
				DiscardArrayValue($3);
				DiscardArrayValue($4);
				DiscardArrayValue($5);
				DiscardArrayValue($6);
				DiscardArrayValue($7);
				DiscardArrayValue($8);
				DiscardArrayValue($9);
				DiscardArrayValue($10);
			}
	|	sphere float float float float opttvpairs
			{
				ParseCallbackInterface->RiSphereV($2, $3, $4, $5, $6->Count(), $6->Tokens(), $6->Values());
				DiscardTokenValuePairs($6);
			}
	|	cone float float float opttvpairs
			{
				ParseCallbackInterface->RiConeV($2, $3, $4, $5->Count(), $5->Tokens(), $5->Values());
				DiscardTokenValuePairs($5);
			}
	|	cylinder float float float float opttvpairs
			{
				ParseCallbackInterface->RiCylinderV($2, $3, $4, $5, $6->Count(), $6->Tokens(), $6->Values());
				DiscardTokenValuePairs($6);
			}
	|	hyperboloid scalar_array opttvpairs
			{
				ParseCallbackInterface->RiHyperboloidV(&(*$2)[0], &(*$2)[3], (*$2)[6], $3->Count(), $3->Tokens(), $3->Values());
				DiscardArrayValue($2);
				DiscardTokenValuePairs($3);
			}
	|	hyperboloid float float float float float float float opttvpairs
			{
				RendermanInterface::RtPoint p1, p2;
				p1[0]=$2;
				p1[1]=$3;
				p1[2]=$4;
				p2[0]=$5;
				p2[1]=$6;
				p2[2]=$7;
				ParseCallbackInterface->RiHyperboloidV(p1, p2, $8, $9->Count(), $9->Tokens(), $9->Values());
				DiscardTokenValuePairs($9);
			}
	|	paraboloid float float float float opttvpairs
			{
				ParseCallbackInterface->RiParaboloidV($2, $3, $4, $5, $6->Count(), $6->Tokens(), $6->Values());
				DiscardTokenValuePairs($6);
			}
	|	disk float float float opttvpairs
			{
				ParseCallbackInterface->RiDiskV($2, $3, $4, $5->Count(), $5->Tokens(), $5->Values());
				DiscardTokenValuePairs($5);
			}
	|	torus float float float float float opttvpairs
			{
				ParseCallbackInterface->RiTorusV($2, $3, $4, $5, $6, $7->Count(), $7->Tokens(), $7->Values());
				DiscardTokenValuePairs($7);
			}
	|	geometry string opttvpairs
			{
				ParseCallbackInterface->RiGeometryV($2, $3->Count(), $3->Tokens(), $3->Values());
				DiscardStringValue($2);
				DiscardTokenValuePairs($3);
			}
	|	solidbegin string
			{
				ParseCallbackInterface->RiSolidBegin(const_cast<char*>($2));
				DiscardStringValue($2);
			}
	|	solidend
			{ ParseCallbackInterface->RiSolidEnd(); }
	|	objectbegin integer
			{
				DeclareObjectHandle($2, ParseCallbackInterface->RiObjectBegin());
			}
	|	objectbegin string
			{
				DeclareObjectHandleString($2, ParseCallbackInterface->RiObjectBegin());
				DiscardStringValue($2);
			}
	|	objectend
			{ ParseCallbackInterface->RiObjectEnd(); }
	|	objectinstance integer
			{
				RendermanInterface::RtObjectHandle handle = LookupObjectHandle($2);
				if(handle)
					ParseCallbackInterface->RiObjectInstance(handle);
				else
					yyerror("Undeclared Object");
			}
	|	objectinstance string
			{
				RendermanInterface::RtObjectHandle handle = LookupObjectHandleString($2);
				if(handle)
					ParseCallbackInterface->RiObjectInstance(handle);
				else
					yyerror("Undeclared Object");
				DiscardStringValue($2);
			}
	|	motionbegin scalar_array
			{
				ParseCallbackInterface->RiMotionBeginV($2->Count(), &(*$2)[0]);
				DiscardArrayValue($2);
			}
	|	motionend
			{ ParseCallbackInterface->RiMotionEnd(); }
	|	maketexture	string string string string string float float opttvpairs
			{
				RendermanInterface::RtFilterFunc pFilter=ParseCallbackInterface->GetFilterFunction($6);
				ParseCallbackInterface->RiMakeTextureV((char*)($2), (char*)($3), (char*)($4), (char*)($5), pFilter, $7, $8,
														$9->Count(), $9->Tokens(), $9->Values());
			}
	|	makebump string string string string string float float opttvpairs
			{
				RendermanInterface::RtFilterFunc pFilter=ParseCallbackInterface->GetFilterFunction($6);
				ParseCallbackInterface->RiMakeBumpV((char*)($2), (char*)($3),(char*)($4), (char*)($5),pFilter,$7, $8,
													$9->Count(), $9->Tokens(), $9->Values());
			}
	|	makelatlongenvironment string string string float float opttvpairs
			{
				RendermanInterface::RtFilterFunc pFilter=ParseCallbackInterface->GetFilterFunction($4);
				ParseCallbackInterface->RiMakeLatLongEnvironmentV((char*)($2), (char*)($3),pFilter,$5, $6,
																$7->Count(), $7->Tokens(), $7->Values());
			}
	|	makecubefaceenvironment	string string string string string string string float string float float opttvpairs
			{
				RendermanInterface::RtFilterFunc pFilter=ParseCallbackInterface->GetFilterFunction($10);
				ParseCallbackInterface->RiMakeCubeFaceEnvironmentV(
							 (char*)($2), (char*)($3),
							 (char*)($4), (char*)($5),
							 (char*)($6), (char*)($7),
							 (char*)($8),
							 $9,
							 pFilter,
							 $11, $12,
							 $13->Count(), $13->Tokens(), $13->Values());
			}
	|	makeshadow string string opttvpairs
			{
				ParseCallbackInterface->RiMakeShadowV($2, $3, $4->Count(), $4->Tokens(), $4->Values());
			}
	|	makeocclusion string_array string opttvpairs
			{
				ParseCallbackInterface->RiMakeOcclusionV($2->Count(), (RtString*)&(*$2)[0], $3, $4->Count(), $4->Tokens(), $4->Values());
			}
	|	errorhandler string
			{
/*
				// TODO: names should be stored elsewhere.
				if(strcmp($2,"ignore")==0)
					ParseCallbackInterface->RiErrorHandler(&ParseCallbackInterface->RiErrorIgnore);
				else if(strcmp($2,"print")==0)
					ParseCallbackInterface->RiErrorHandler(&ParseCallbackInterface->RiErrorPrint);
				else if(strcmp($2,"abort")==0)
					ParseCallbackInterface->RiErrorHandler(&ParseCallbackInterface->RiErrorAbort);
*/
									}
	|	errorignore
			{ printf("ERRORIGNORE\n"); }
	|	errorprint
			{ printf("ERRORPRINT\n"); }
	|	errorabort
			{ printf("ERRORABORT\n"); }
	|	subdivisionmesh string integer_array integer_array string_array_or_empty integer_array_or_empty integer_array_or_empty scalar_array_or_empty tvpairs
			{
				char** ptags=new char*[$5->Count()];
				int i;
				for(i=0; i<$5->Count(); i++)
				{
					char* ptag=new char[strlen((*$5)[i]) + 1];
					strcpy(ptag, (*$5)[i]);
					ptags[i]=ptag;
				}
				// Do some sanity checking on the tags data
				if($6->Count() != $5->Count() * 2)
				{
					std::stringstream error;
					error << "SubdivisionMesh: Invalid tags list, nargs length (" << $6->Count() << ") expected " << $5->Count()*2 << " (ntags (" << $5->Count() << ") times 2, see spec.)" << std::ends;
					yyerror(error.str());
				}

				ParseCallbackInterface->RiSubdivisionMeshV(const_cast<char*>($2),
								$3->Count(),&(*$3)[0], &(*$4)[0],
								$5->Count(), ptags,
								&(*$6)[0],
								&(*$7)[0],
								&(*$8)[0],
								$9->Count(), $9->Tokens(), $9->Values());

				for(i=0; i<$5->Count(); i++)
					delete[](ptags[i]);
				delete[](ptags);

				DiscardStringValue($2);
				DiscardArrayValue($3);
				DiscardArrayValue($4);
				DiscardArrayValue($5);
				DiscardArrayValue($6);
				DiscardArrayValue($7);
				DiscardArrayValue($8);
				DiscardTokenValuePairs($9);
			}
	|	subdivisionmesh string integer_array integer_array tvpairs
			{
				ParseCallbackInterface->RiSubdivisionMeshV(const_cast<char*>($2), $3->Count(), &(*$3)[0], &(*$4)[0], 0, NULL, NULL, NULL, NULL, $5->Count(), $5->Tokens(), $5->Values());
				DiscardStringValue($2);
				DiscardArrayValue($3);
				DiscardArrayValue($4);
				DiscardTokenValuePairs($5);
			}
	|	shaderlayer string string string opttvpairs
			{
				ParseCallbackInterface->RiShaderLayerV(const_cast<char*>($2), const_cast<char*>($3), const_cast<char*>($4), $5->Count(), $5->Tokens(), $5->Values());
				DiscardStringValue($2);
				DiscardStringValue($3);
				DiscardStringValue($4);
				DiscardTokenValuePairs($5);
			}
	|	connectshaderlayers string string string string string
			{
				ParseCallbackInterface->RiConnectShaderLayers(const_cast<char*>($2), const_cast<char*>($3), const_cast<char*>($4), const_cast<char*>($5), const_cast<char*>($6));
				DiscardStringValue($2);
				DiscardStringValue($3);
				DiscardStringValue($4);
				DiscardStringValue($5);
				DiscardStringValue($6);
			}
	|	resource string string opttvpairs
			{
				ParseCallbackInterface->RiResourceV(const_cast<char*>($2), const_cast<char*>($3), $4->Count(), $4->Tokens(), $4->Values());
				DiscardStringValue($2);
				DiscardStringValue($3);
				DiscardTokenValuePairs($4);
			}
	|	resourcebegin
			{
				ParseCallbackInterface->RiResourceBegin();
			}
	|	resourceend
			{
				ParseCallbackInterface->RiResourceEnd();
			}
	|	UNKNOWN_TOKEN
			{
				// Print the error, then force the scanner into 'request'
				// mode which will make it ignore all parameters until the
				// next request token.
				std::stringstream error;
				error << "Unrecognised RIB request : " << $1 << std::ends;
				yyerror(error.str().c_str());
				fRecovering = true;
				ExpectRequest();
			}
	|	error
			{
				// Print the error, then force the scanner into 'request'
				// mode which will make it ignore all parameters until the
				// next request token.
				if( !fRecovering )
				{
					yyerror("Unrecognised RIB request");
					fRecovering = true;
					ExpectRequest();
				}
			}
	|	INVALID_VALUE
			{
				// A value has been found when a request is expected, quietly eat it...
			}
	;

arealightsource : REQUEST_TOKEN_AREALIGHTSOURCE	{ ExpectParams(); };
atmosphere : REQUEST_TOKEN_ATMOSPHERE	{ ExpectParams(); };
attribute : REQUEST_TOKEN_ATTRIBUTE	{ ExpectParams(); };
attributebegin : REQUEST_TOKEN_ATTRIBUTEBEGIN	{ ExpectParams(); };
attributeend : REQUEST_TOKEN_ATTRIBUTEEND	{ ExpectParams(); };
basis : REQUEST_TOKEN_BASIS	{ ExpectParams(); };
blobby : REQUEST_TOKEN_BLOBBY	{ ExpectParams(); };
bound : REQUEST_TOKEN_BOUND	{ ExpectParams(); };
clipping : REQUEST_TOKEN_CLIPPING	{ ExpectParams(); };
color : REQUEST_TOKEN_COLOR	{ ExpectParams(); };
colorsamples : REQUEST_TOKEN_COLORSAMPLES	{ ExpectParams(); };
concattransform : REQUEST_TOKEN_CONCATTRANSFORM	{ ExpectParams(); };
cone : REQUEST_TOKEN_CONE	{ ExpectParams(); };
coordinatesystem : REQUEST_TOKEN_COORDINATESYSTEM	{ ExpectParams(); };
coordsystransform : REQUEST_TOKEN_COORDSYSTRANSFORM	{ ExpectParams(); };
cropwindow : REQUEST_TOKEN_CROPWINDOW	{ ExpectParams(); };
cylinder : REQUEST_TOKEN_CYLINDER	{ ExpectParams(); };
declare : REQUEST_TOKEN_DECLARE	{ ExpectParams(); };
deformation : REQUEST_TOKEN_DEFORMATION	{ ExpectParams(); };
depthoffield : REQUEST_TOKEN_DEPTHOFFIELD	{ ExpectParams(); };
detail : REQUEST_TOKEN_DETAIL	{ ExpectParams(); };
detailrange : REQUEST_TOKEN_DETAILRANGE	{ ExpectParams(); };
disk : REQUEST_TOKEN_DISK	{ ExpectParams(); };
displacement : REQUEST_TOKEN_DISPLACEMENT	{ ExpectParams(); };
display : REQUEST_TOKEN_DISPLAY	{ ExpectParams(); };
else   : REQUEST_TOKEN_ELSE	{  ExpectParams();};
elseif : REQUEST_TOKEN_ELSEIF	{  ExpectParams();};
errorabort : REQUEST_TOKEN_ERRORABORT	{ ExpectParams(); };
errorhandler : REQUEST_TOKEN_ERRORHANDLER	{ ExpectParams(); };
errorignore : REQUEST_TOKEN_ERRORIGNORE	{ ExpectParams(); };
errorprint : REQUEST_TOKEN_ERRORPRINT	{ ExpectParams(); };
exposure : REQUEST_TOKEN_EXPOSURE	{ ExpectParams(); };
exterior : REQUEST_TOKEN_EXTERIOR	{ ExpectParams(); };
format : REQUEST_TOKEN_FORMAT	{ ExpectParams(); };
frameaspectratio : REQUEST_TOKEN_FRAMEASPECTRATIO	{ ExpectParams(); };
framebegin : REQUEST_TOKEN_FRAMEBEGIN	{ ExpectParams(); };
frameend : REQUEST_TOKEN_FRAMEEND	{ ExpectParams(); };
generalpolygon : REQUEST_TOKEN_GENERALPOLYGON	{ ExpectParams(); };
geometricapproximation : REQUEST_TOKEN_GEOMETRICAPPROXIMATION	{ ExpectParams(); };
geometry : REQUEST_TOKEN_GEOMETRY	{ ExpectParams(); };
hider : REQUEST_TOKEN_HIDER	{ ExpectParams(); };
hyperboloid : REQUEST_TOKEN_HYPERBOLOID	{ ExpectParams(); };
identity : REQUEST_TOKEN_IDENTITY	{ ExpectParams(); };
ifbegin : REQUEST_TOKEN_IFBEGIN	{ ExpectParams(); };
ifend : REQUEST_TOKEN_IFEND	{ ExpectParams(); };
illuminate : REQUEST_TOKEN_ILLUMINATE	{ ExpectParams(); };
imager : REQUEST_TOKEN_IMAGER	{ ExpectParams(); };
interior : REQUEST_TOKEN_INTERIOR	{ ExpectParams(); };
lightsource : REQUEST_TOKEN_LIGHTSOURCE	{ ExpectParams(); };
makeocclusion : REQUEST_TOKEN_MAKEOCCLUSION	{ ExpectParams(); };
makebump : REQUEST_TOKEN_MAKEBUMP	{ ExpectParams(); };
makecubefaceenvironment : REQUEST_TOKEN_MAKECUBEFACEENVIRONMENT	{ ExpectParams(); };
makelatlongenvironment : REQUEST_TOKEN_MAKELATLONGENVIRONMENT	{ ExpectParams(); };
makeshadow : REQUEST_TOKEN_MAKESHADOW	{ ExpectParams(); };
maketexture : REQUEST_TOKEN_MAKETEXTURE	{ ExpectParams(); };
matte : REQUEST_TOKEN_MATTE	{ ExpectParams(); };
motionbegin : REQUEST_TOKEN_MOTIONBEGIN	{ ExpectParams(); };
motionend : REQUEST_TOKEN_MOTIONEND	{ ExpectParams(); };
nupatch : REQUEST_TOKEN_NUPATCH	{ ExpectParams(); };
objectbegin : REQUEST_TOKEN_OBJECTBEGIN	{ ExpectParams(); };
objectend : REQUEST_TOKEN_OBJECTEND	{ ExpectParams(); };
objectinstance : REQUEST_TOKEN_OBJECTINSTANCE	{ ExpectParams(); };
opacity : REQUEST_TOKEN_OPACITY	{ ExpectParams(); };
option : REQUEST_TOKEN_OPTION	{ ExpectParams(); };
orientation : REQUEST_TOKEN_ORIENTATION	{ ExpectParams(); };
paraboloid : REQUEST_TOKEN_PARABOLOID	{ ExpectParams(); };
patch : REQUEST_TOKEN_PATCH	{ ExpectParams(); };
patchmesh : REQUEST_TOKEN_PATCHMESH	{ ExpectParams(); };
perspective : REQUEST_TOKEN_PERSPECTIVE	{ ExpectParams(); };
pixelfilter : REQUEST_TOKEN_PIXELFILTER	{ ExpectParams(); };
pixelsamples : REQUEST_TOKEN_PIXELSAMPLES	{ ExpectParams(); };
pixelvariance : REQUEST_TOKEN_PIXELVARIANCE	{ ExpectParams(); };
curves: REQUEST_TOKEN_CURVES	{ ExpectParams(); };
procedural: REQUEST_TOKEN_PROCEDURAL	{ ExpectParams(); };
points: REQUEST_TOKEN_POINTS	{ ExpectParams(); };
pointsgeneralpolygons : REQUEST_TOKEN_POINTSGENERALPOLYGONS	{ ExpectParams(); };
pointspolygons : REQUEST_TOKEN_POINTSPOLYGONS	{ ExpectParams(); };
polygon : REQUEST_TOKEN_POLYGON	{ ExpectParams(); };
projection : REQUEST_TOKEN_PROJECTION	{ ExpectParams(); };
quantize : REQUEST_TOKEN_QUANTIZE	{ ExpectParams(); };
readarchive: REQUEST_TOKEN_READARCHIVE	{ ExpectParams(); };
relativedetail : REQUEST_TOKEN_RELATIVEDETAIL	{ ExpectParams(); };
reverseorientation : REQUEST_TOKEN_REVERSEORIENTATION	{ ExpectParams(); };
rotate : REQUEST_TOKEN_ROTATE	{ ExpectParams(); };
resource : REQUEST_TOKEN_RESOURCE	{ ExpectParams(); };
resourcebegin : REQUEST_TOKEN_RESOURCEBEGIN	{ ExpectParams(); };
resourceend : REQUEST_TOKEN_RESOURCEEND	{ ExpectParams(); };
scale : REQUEST_TOKEN_SCALE	{ ExpectParams(); };
screenwindow : REQUEST_TOKEN_SCREENWINDOW	{ ExpectParams(); };
shadinginterpolation : REQUEST_TOKEN_SHADINGINTERPOLATION	{ ExpectParams(); };
shadingrate : REQUEST_TOKEN_SHADINGRATE	{ ExpectParams(); };
shutter : REQUEST_TOKEN_SHUTTER	{ ExpectParams(); };
sides : REQUEST_TOKEN_SIDES	{ ExpectParams(); };
skew : REQUEST_TOKEN_SKEW	{ ExpectParams(); };
solidbegin : REQUEST_TOKEN_SOLIDBEGIN	{ ExpectParams(); };
solidend : REQUEST_TOKEN_SOLIDEND	{ ExpectParams(); };
sphere : REQUEST_TOKEN_SPHERE	{ ExpectParams(); };
subdivisionmesh : REQUEST_TOKEN_SUBDIVISIONMESH	{ ExpectParams(); };
surface : REQUEST_TOKEN_SURFACE	{ ExpectParams(); };
texturecoordinates : REQUEST_TOKEN_TEXTURECOORDINATES	{ ExpectParams(); };
torus : REQUEST_TOKEN_TORUS	{ ExpectParams(); };
transform : REQUEST_TOKEN_TRANSFORM	{ ExpectParams(); };
transformbegin : REQUEST_TOKEN_TRANSFORMBEGIN	{ ExpectParams(); };
transformend : REQUEST_TOKEN_TRANSFORMEND	{ ExpectParams(); };
transformpoints : REQUEST_TOKEN_TRANSFORMPOINTS	{ ExpectParams(); };
translate : REQUEST_TOKEN_TRANSLATE	{ ExpectParams(); };
trimcurve : REQUEST_TOKEN_TRIMCURVE	{ ExpectParams(); };
version : REQUEST_TOKEN_VERSION	{ ExpectParams(); };
worldbegin : REQUEST_TOKEN_WORLDBEGIN	{ ExpectParams(); };
worldend : REQUEST_TOKEN_WORLDEND	{ ExpectParams(); };
shaderlayer : REQUEST_TOKEN_SHADERLAYER	{ ExpectParams(); };
connectshaderlayers : REQUEST_TOKEN_CONNECTSHADERLAYERS	{ ExpectParams(); };


float : FLOAT_TOKEN
	|	integer
			{ $$ = static_cast<float>($1); }
	;

floats :	FLOAT_TOKEN
			{
				$$ = new FloatArray($1);
			}
	|	floats FLOAT_TOKEN
			{
				$$ = $1;
				$$->push_back($2);
			}
	|	floats INTEGER_TOKEN
			{
				$$ = $1;
				$$->push_back($2);
			}
	|	integers FLOAT_TOKEN
			{
				$$ = new FloatArray($1);
				$$->push_back($2);
				DiscardArrayValue($1);
			}
	;


float_array : '[' floats ']'
			{
				$$ = $2;
			}
	;

integer : INTEGER_TOKEN
			{
				$$ = $1;
			}
	;

integers : INTEGER_TOKEN
			{
				$$ = new IntegerArray($1);
			}
	|	integers integer
			{
				$$ = $1;
				$$->push_back($2);
			}
	;

integer_array : '[' integers ']'
			{
				$$ = $2;
			}
	;

integer_array_or_empty : integer_array
	|	'[' ']'
			{
				$$ = new IntegerArray;
			}
	;

string : STRING_TOKEN
			{
				$$ = $1;
			}
	;

strings : STRING_TOKEN
			{
				$$ = new StringArray($1);
			}
	|	strings STRING_TOKEN
			{
				$$ = $1;
				$$->push_back($2);
			}
	;

string_array : '[' strings ']'
			{
				$$ = $2;
			}
	;

string_array_or_empty : string_array
	|	'[' ']'
			{
				$$ = new StringArray;
			}
	;

scalar_array : float_array
	|	integer_array
			{
				$$ = new FloatArray($1);
				DiscardArrayValue($1);
			}
	;


scalar_array_or_empty : scalar_array
	|	'[' ']'
			{
				$$ = new FloatArray;
			}
	;

tvpair	: string INTEGER_TOKEN
			{
				// Set default value ...
				$$ = 0;

				switch(LookupParameterType($1))
					{
						case Type_Unknown:
							yyerror("Undeclared Integer Parameter [" + std::string($1) + "]");
							DiscardStringValue($1);
							break;

						case Type_Integer:
							// We don't have to discard $1 because the TokenValuePair assumes ownership of the string ...
							$$ = new TokenValuePair($1, new IntegerArray($2));
							break;

						// This is where we perform type conversion from integer to float, based on the declared parameter type ...
						case Type_Float:
						case Type_Point:
						case Type_Color:
						case Type_hPoint:
						case Type_Normal:
						case Type_Vector:
						case Type_Matrix:
							// We don't have to discard $1 because the TokenValuePair assumes ownership of the string ...
							$$ = new TokenValuePair($1, new FloatArray(static_cast<RendermanInterface::RtFloat>($2)));
							break;

						default:
							yyerror("Integer Parameter Type Mismatch [" + std::string($1) + "]");
							DiscardStringValue($1);
							break;
					}
			}
	|	string integer_array
			{
				// Set default value ...
				$$ = 0;

				switch(LookupParameterType($1))
					{
						case Type_Unknown:
							yyerror("Undeclared Integer Array Parameter [" + std::string($1) + "]");
							DiscardStringValue($1);
							DiscardArrayValue($2);
							break;

						case Type_Integer:
							// We don't have to discard $1 because the TokenValuePair assumes ownership of the string ...
							$$ = new TokenValuePair($1, $2);
							break;

						// This is where we perform type conversion from integer to float, based on the declared parameter type ...
						case Type_Float:
						case Type_Point:
						case Type_Color:
						case Type_hPoint:
						case Type_Normal:
						case Type_Vector:
						case Type_Matrix:
							// We don't have to discard $1 because the TokenValuePair assumes ownership of the string ...
							$$ = new TokenValuePair($1, new FloatArray($2));
							DiscardArrayValue($2);
							break;

						default:
							yyerror("Integer Array Parameter Type Mismatch [" + std::string($1) + "]");
							DiscardStringValue($1);
							DiscardArrayValue($2);
							break;
					}
			}
	|	string FLOAT_TOKEN
			{
				// Set default value ...
				$$ = 0;

				switch(LookupParameterType($1))
					{
						case Type_Unknown:
							yyerror("Undeclared Float Parameter: [" + std::string($1) + "]");
							DiscardStringValue($1);
							break;

						case Type_Float:
							// We don't have to discard $1 because the TokenValuePair assumes ownership of the string ...
							$$ = new TokenValuePair($1, new FloatArray($2));
							break;

						default:
							yyerror("Float Parameter Type Mismatch [" + std::string($1) + "]");
							DiscardStringValue($1);
							break;
					}
			}
	|	string float_array
			{
				// Set default value ...
				$$ = 0;

				switch(LookupParameterType($1))
					{
						case Type_Unknown:
							yyerror("Undeclared Float Array Parameter: [" + std::string($1) + "]");
							DiscardStringValue($1);
							DiscardArrayValue($2);
							break;

						case Type_Float:
						case Type_Point:
						case Type_Color:
						case Type_hPoint:
						case Type_Normal:
						case Type_Vector:
						case Type_Matrix:
							// We don't have to discard $1 because the TokenValuePair assumes ownership of the string ...
							$$ = new TokenValuePair($1, $2);
							break;

						default:
							yyerror("Float Array Parameter Type Mismatch [" + std::string($1) + "]");
							DiscardStringValue($1);
							DiscardArrayValue($2);
							break;
					}
			}
	|	string STRING_TOKEN
			{
				// Set default value ...
				$$ = 0;

				switch(LookupParameterType($1))
					{
						case Type_Unknown:
							yyerror("Undeclared String Parameter [" + std::string($1) + "]");
							DiscardStringValue($1);
							DiscardStringValue($2);
							break;

						case Type_String:
							// We don't have to discard $1 because the TokenValuePair assumes ownership of the string ...
							$$ = new TokenValuePair($1, new StringArray($2));
							break;

						default:
							yyerror("String Parameter Type Mismatch [" + std::string($1) + "]");
							DiscardStringValue($1);
							DiscardStringValue($2);
							break;
					}
			}
	|	string string_array
			{
				// Set default value ...
				$$ = 0;

				switch(LookupParameterType($1))
					{
						case Type_Unknown:
							yyerror("Undeclared String Array Parameter [" + std::string($1) + "]");
							DiscardStringValue($1);
							DiscardArrayValue($2);
							break;

						case Type_String:
							// We don't have to discard $1 because the TokenValuePair assumes ownership of the string ...
							$$ = new TokenValuePair($1, $2);
							break;

						default:
							yyerror("String Array Parameter Type Mismatch [" + std::string($1) + "]");
							DiscardStringValue($1);
							DiscardArrayValue($2);
							break;
					}
			}
	;

opttvpairs : /* Empty */
			{
				$$ = new TokenValuePairs;
			}
	|	error
			{
				$$ = new TokenValuePairs;
			}
	|	tvpairs
	;

tvpairs
	:	tvpair
			{
				// We don't need to delete $1 here, because the new TokenValuePairs object owns it, now ...
				$$ = new TokenValuePairs($1);
			}
	|	error tvpair
			{
				// We don't need to delete $2 here, because the new TokenValuePairs object owns it, now ...
				$$ = new TokenValuePairs($2);
			}
	|	tvpairs tvpair
			{
				// We don't need to delete $1 here, because we transfer its ownership to $$ ...
				// We don't need to delete $2 here, because the TokenValuePairs object in $$ owns it, now ...
				$$ = $1;
				$$->AddPair($2);
			}
	|	tvpairs error
	;

%%

static void yyerror(const std::string Message)
{
	ParseSucceeded = false;
	(*ParseErrorStream) << Aqsis::error << Message << " at " << ParseStreamName << " line " << ParseLineNumber << std::endl;
}

static void ExpectRequest()
{
	fRequest=true;
}

static void ExpectParams()
{
	fParams=true;
}

static void SkipFrame()
{
	fSkipFrame=true;
}

static void DiscardStringValue(char* const Value)
{
	delete[] Value;
}

static void DiscardTokenValuePairs(TokenValuePairs* const Value)
{
	delete Value;
}

static void DiscardArrayValue(Array* const Value)
{
	delete Value;
}

static bool notspace(char C)
{
bool retval = true;

	if ((C == 0x20) || 
		(( C >= 0x09 ) && (C <= 0x0D)))
		retval = false;
		
	return retval;
}
static bool sspace(char C)
{
	bool retval = false;

	if ((C == 0x20) || 
		(( C >= 0x09 ) && (C <= 0x0D)))
		retval = true;
		
	return retval;
}




typedef std::vector<std::string> Strings;
typedef Strings::iterator StringIterator;
static Strings Words(const std::string& Source)
{
	Strings results;

	std::string::const_iterator i = Source.begin();
	while(i != Source.end())
		{
			i = std::find_if(i, Source.end(), notspace);
			if(i == Source.end())
				break;
			
            std::string::const_iterator j = std::find_if(i, Source.end(), sspace);

			
			results.push_back(std::string(i, j));
			
			i = j;
		}
	
	return results;
}

static std::string CleanParameterType(const std::string& Source)
{
	return Source.substr(0, Source.find("["));
}

static ParameterType ParseParameterType(const std::string& Source)
{
	if(0 == Source.size())
		{
			yyerror("Cannot Parse Empty Parameter Type");
			return Type_Unknown;
		}

	// Break the source string into whitespace delimited "words" ...
	Strings words(Words(Source));

	// Figure out the parameter type ...
	for(StringIterator word = words.begin(); word != words.end(); word++)
		{
			std::string cleanword = CleanParameterType(*word);
		
			if(cleanword == "integer")
				return Type_Integer;

			if(cleanword == "int")
				return Type_Integer;

			if(cleanword == "float")
				return Type_Float;
				
			if(cleanword == "hpoint")
				return Type_hPoint;
			
			if(cleanword == "point")
				return Type_Point;
				
			if(cleanword == "color")
				return Type_Color;
				
			if(cleanword == "vector")
				return Type_Vector;
				
			if(cleanword == "normal")
				return Type_Normal;
				
			if(cleanword == "matrix")
				return Type_Matrix;
				
			if(cleanword == "string")
				return Type_String;
		}

	return Type_Unknown;
}

typedef std::map<RendermanInterface::RtInt, RendermanInterface::RtLightHandle> LightHandleMap;
typedef LightHandleMap::iterator LightHandleIterator;
static LightHandleMap LightMap;

typedef std::map<std::string, RendermanInterface::RtLightHandle> LightStringHandleMap;
typedef LightStringHandleMap::iterator LightStringHandleIterator;
static LightStringHandleMap LightMapString;

typedef std::map<RendermanInterface::RtInt, RendermanInterface::RtObjectHandle> ObjectHandleMap;
typedef ObjectHandleMap::iterator ObjectHandleIterator;
static ObjectHandleMap ObjectMap;

typedef std::map<std::string, RendermanInterface::RtObjectHandle> ObjectStringHandleMap;
typedef ObjectStringHandleMap::iterator ObjectStringHandleIterator;
static ObjectStringHandleMap ObjectMapString;

static void DeclareLightHandle(const RendermanInterface::RtInt ID, const RendermanInterface::RtLightHandle Handle)
{
	LightMap[ID] = Handle;
}

static RendermanInterface::RtLightHandle LookupLightHandle(const RendermanInterface::RtInt ID)
{
	if(LightMap.find(ID) == LightMap.end())
		{
			yyerror("Undeclared Light ID");
			return 0;
		}
		
	return LightMap[ID];
}


static void DeclareLightHandleString(const RendermanInterface::RtString ID, const RendermanInterface::RtLightHandle Handle)
{
	if(0 == Handle)
		{
			yyerror("NULL Light Handle");
			return;
		}

	LightMapString[ID] = Handle;
}

static RendermanInterface::RtLightHandle LookupLightHandleString(const RendermanInterface::RtString ID)
{
	if(LightMapString.find(ID) == LightMapString.end())
		{
			yyerror("Undeclared Light name");
			return 0;
		}
		
	return LightMapString[ID];
}


static void DeclareObjectHandle(const RendermanInterface::RtInt ID, const RendermanInterface::RtObjectHandle Handle)
{
	if(0 == Handle)
		{
			yyerror("NULL Object Handle");
			return;
		}

	ObjectMap[ID] = Handle;
}

static RendermanInterface::RtObjectHandle LookupObjectHandle(const RendermanInterface::RtInt ID)
{
	if(ObjectMap.find(ID) == ObjectMap.end())
		{
			yyerror("Undeclared Object ID");
			return 0;
		}
		
	return ObjectMap[ID];
}


static void DeclareObjectHandleString(const RendermanInterface::RtString ID, const RendermanInterface::RtObjectHandle Handle)
{
	if(0 == Handle)
		{
			yyerror("NULL Object Handle");
			return;
		}

	ObjectMapString[ID] = Handle;
}

static RendermanInterface::RtObjectHandle LookupObjectHandleString(const RendermanInterface::RtString ID)
{
	if(ObjectMapString.find(ID) == ObjectMapString.end())
		{
			yyerror("Undeclared Object name");
			return 0;
		}
		
	return ObjectMapString[ID];
}

typedef std::map<std::string, ParameterType> ParameterTypeMap;
typedef ParameterTypeMap::iterator ParameterTypeIterator;
static ParameterTypeMap TypeMap;

static ParameterType DeclareParameterType(const std::string& Name, const std::string& Type)
{
	if(0 == Name.size())
		{
			yyerror("Cannot Declare Unnamed Type");
			return Type_Unknown;
		}

	const ParameterType type = ParseParameterType(Type);

	if(type != Type_Unknown)
		TypeMap[Name] = type;

	return type;
}

static ParameterType LookupParameterType(const std::string& Name)
{
	if(0 == Name.size())
		{
			yyerror("Cannot Lookup Unnamed Type");
			return Type_Unknown;
		}

	// See if this parameter has already been declared ...
	ParameterTypeIterator parameter = TypeMap.find(Name);
	if(parameter != TypeMap.end())
		return static_cast<ParameterType>(parameter->second & Type_Mask);

	// Check to see if the type declaration is part of the parameter name ...
	return ParseParameterType(Name);
}


std::vector<int>	FrameList;
bool IsFrameSkipped(TqUint number)
{
	// Check the frame list to see if the requested frame is included.
	if(FrameList.size() == 0)
		return(false);
	else if(FrameList.size() <= number)
		return(true);
	else if(FrameList[number])
		return(false);
	else
		return(true);
}


namespace librib
{

void ParserDeclare(librib::RendermanInterface* CallbackInterface, const std::string Name, const std::string Type)
{
	if(Type_Unknown == DeclareParameterType(Name, Type))
		yyerror("RiDeclare: Unknown type [" + Name + ", " + Type + "]");
		
	if(CallbackInterface != NULL)
	{
		char *_name = new char[Name.length()+1];
		strcpy(_name, Name.c_str());
		char *_type = new char[Type.length()+1];
		strcpy(_type, Type.c_str());
		CallbackInterface->RiDeclare(_name, _type);
		delete[](_name);
		delete[](_type);
	}
}

void ClearDeclarations()
{
	TypeMap.clear();
}

void ClearFrames()
{
	FrameList.clear();
}

int AppendFrames(const char* frames)
{
	TqUint n=0;
	TqUint length = strlen(frames);
	char *endptr;
	const char* nptr = frames;
	while(n < length)
	{
		TqUint f1, f2;
		f1 = strtol(nptr, &endptr, 10);
		if(endptr != nptr)
		{
			n += endptr - nptr;
			// Check for a range.
			if(endptr[0] == '-')
			{
				nptr = endptr+1;
				n+=1;
				f2 = strtol(nptr, &endptr, 10);
				if(endptr != nptr)
				{
					n += endptr - nptr;
					// Store the range between f1 and f2;
					if(FrameList.size() <= MAX(f1, f2))
						FrameList.resize(MAX(f1, f2)+1, 0);
					TqUint start = MIN(f1, f2);
					TqUint end = MAX(f1, f2);
					TqUint i;
					for(i = start; i <= end; i++)
						FrameList[i] = 1;
					nptr = endptr;
				}
			}
			else
			{
				// store the single frame value
				if(FrameList.size() <= f1)
					FrameList.resize(f1+1, 0);
				FrameList[f1] = 1;
				nptr = endptr;
			}
		}
		else if(endptr[0] == ',')
		{
			n += 1;
			nptr += 1;
		}
		else
		{
			return(-1);
		}
	}
	return(0);
}

}; // namespace librib


