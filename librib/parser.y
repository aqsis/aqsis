/* -------------- declaration section -------------- */

%name RIBParser
%define LSP_NEEDED
%define ERROR_BODY =0
%define LEX_BODY =0
%define	MEMBERS		virtual void ExpectRequest()	{} \
					virtual void ExpectParams()		{} \
							void DiscardValue(void* p)	{delete(p);} \
							void DiscardStringValue(void* p); \
							void DiscardArrayValue(struct yy_RIBParser_stype::array_union& array); \
							void DiscardTokenValuePair(void* p); \
							void DiscardTokenValuePairs(void* p); \
							char* CheckParameterArrayType(const char* strname, struct yy_RIBParser_stype::array_union& array, int& count, int& type); \
							bool CheckArrayType(EqVariableType type, struct yy_RIBParser_stype::array_union& array); \
					public: \
					void Initialise() \
					{ \
						m_almpCurrent.clear(); \
					} \
					struct SqLightsourceMap \
					{ \
						SqLightsourceMap()	: m_RIBSeqNum(-1), m_Handle(0) {} \
						SqLightsourceMap(TqInt sn, RtLightHandle h)	: m_RIBSeqNum(sn), m_Handle(h) {} \
						TqInt			m_RIBSeqNum; \
						RtLightHandle	m_Handle; \
					}; \
					std::vector<SqLightsourceMap>	m_almpCurrent;
%define CONSTRUCTOR_CODE { \
						m_almpCurrent.clear(); \
					}
%define DEBUG 1
%header{
#include	"sstring.h"

using namespace Aqsis;

#include	"shadervariable.h"
%}


%union {
	TqInt itype;
	TqFloat ftype;
	CqString* stype;
	char ctype;
	struct array_union
	{
		std::vector<TqInt>* aitype;
		std::vector<TqFloat>* aftype;
		std::vector<CqString*>* astype;
		EqVariableType	type;
	}atype;
	struct SqTV
	{
		char* token;
		char* value;
		int	  count;
		int   type;
	}tvtype;
	struct SqListTV
	{
		std::vector<char*>* atokens;
		std::vector<char*>* avalues;
		std::vector<int>*	  acounts;
		std::vector<int>*	  atypes;
	}atvtype;
}
%token <ctype>	EOL_TOKEN EOF_TOKEN CHAR_TOKEN UNKNOWN_TOKEN
%token <ftype>	FLOAT_TOKEN
%token <itype>	INTEGER_TOKEN
%token <stype>	STRING_TOKEN
%type  <itype>	integer
%type  <ftype>	float
%type  <stype>	string
%type  <atype>	array
%type  <atype> floats float_array
%type  <atype> integers integer_array
%type  <atype>	strings string_array
%type  <tvtype> tvpair
%type  <atvtype> tvpairs
%type  <atvtype> opttvpairs

%token <ctype>	REQUEST_TOKEN_VERSION
%token <ctype>	REQUEST_TOKEN_DECLARE
%token <ctype>	REQUEST_TOKEN_FRAMEBEGIN
%token <ctype>	REQUEST_TOKEN_FRAMEEND
%token <ctype>	REQUEST_TOKEN_WORLDBEGIN
%token <ctype>	REQUEST_TOKEN_WORLDEND
%token <ctype>	REQUEST_TOKEN_FORMAT
%token <ctype>	REQUEST_TOKEN_FRAMEASPECTRATIO
%token <ctype>	REQUEST_TOKEN_SCREENWINDOW
%token <ctype>	REQUEST_TOKEN_CROPWINDOW
%token <ctype>	REQUEST_TOKEN_PROJECTION
%token <ctype>	REQUEST_TOKEN_CLIPPING
%token <ctype>	REQUEST_TOKEN_DEPTHOFFIELD
%token <ctype>	REQUEST_TOKEN_SHUTTER
%token <ctype>	REQUEST_TOKEN_PIXELVARIANCE
%token <ctype>	REQUEST_TOKEN_PIXELSAMPLES
%token <ctype>	REQUEST_TOKEN_PIXELFILTER
%token <ctype>	REQUEST_TOKEN_EXPOSURE
%token <ctype>	REQUEST_TOKEN_IMAGER
%token <ctype>	REQUEST_TOKEN_QUANTIZE
%token <ctype>	REQUEST_TOKEN_DISPLAY
%token <ctype>	REQUEST_TOKEN_GAUSSIANFILTER
%token <ctype>	REQUEST_TOKEN_BOXFILTER
%token <ctype>	REQUEST_TOKEN_TRIANGLEFILTER
%token <ctype>	REQUEST_TOKEN_CATMULLROMFILTER
%token <ctype>	REQUEST_TOKEN_SINCFILTER
%token <ctype>	REQUEST_TOKEN_HIDER
%token <ctype>	REQUEST_TOKEN_COLORSAMPLES
%token <ctype>	REQUEST_TOKEN_RELATIVEDETAIL
%token <ctype>	REQUEST_TOKEN_OPTION
%token <ctype>	REQUEST_TOKEN_ATTRIBUTEBEGIN
%token <ctype>	REQUEST_TOKEN_ATTRIBUTEEND
%token <ctype>	REQUEST_TOKEN_COLOR
%token <ctype>	REQUEST_TOKEN_OPACITY
%token <ctype>	REQUEST_TOKEN_TEXTURECOORDINATES
%token <ctype>	REQUEST_TOKEN_LIGHTSOURCE
%token <ctype>	REQUEST_TOKEN_AREALIGHTSOURCE
%token <ctype>	REQUEST_TOKEN_ILLUMINATE
%token <ctype>	REQUEST_TOKEN_SURFACE
%token <ctype>	REQUEST_TOKEN_ATMOSPHERE
%token <ctype>	REQUEST_TOKEN_INTERIOR
%token <ctype>	REQUEST_TOKEN_EXTERIOR
%token <ctype>	REQUEST_TOKEN_SHADINGRATE
%token <ctype>	REQUEST_TOKEN_SHADINGINTERPOLATION
%token <ctype>	REQUEST_TOKEN_MATTE
%token <ctype>	REQUEST_TOKEN_BOUND
%token <ctype>	REQUEST_TOKEN_DETAIL
%token <ctype>	REQUEST_TOKEN_DETAILRANGE
%token <ctype>	REQUEST_TOKEN_GEOMETRICAPPROXIMATION
%token <ctype>	REQUEST_TOKEN_ORIENTATION
%token <ctype>	REQUEST_TOKEN_REVERSEORIENTATION
%token <ctype>	REQUEST_TOKEN_SIDES
%token <ctype>	REQUEST_TOKEN_IDENTITY
%token <ctype>	REQUEST_TOKEN_TRANSFORM
%token <ctype>	REQUEST_TOKEN_CONCATTRANSFORM
%token <ctype>	REQUEST_TOKEN_PERSPECTIVE
%token <ctype>	REQUEST_TOKEN_TRANSLATE
%token <ctype>	REQUEST_TOKEN_ROTATE
%token <ctype>	REQUEST_TOKEN_SCALE
%token <ctype>	REQUEST_TOKEN_SKEW
%token <ctype>	REQUEST_TOKEN_DEFORMATION
%token <ctype>	REQUEST_TOKEN_DISPLACEMENT
%token <ctype>	REQUEST_TOKEN_COORDINATESYSTEM
%token <ctype>	REQUEST_TOKEN_COORDSYSTRANSFORM
%token <ctype>	REQUEST_TOKEN_TRANSFORMPOINTS
%token <ctype>	REQUEST_TOKEN_TRANSFORMBEGIN
%token <ctype>	REQUEST_TOKEN_TRANSFORMEND
%token <ctype>	REQUEST_TOKEN_ATTRIBUTE
%token <ctype>	REQUEST_TOKEN_POLYGON
%token <ctype>	REQUEST_TOKEN_GENERALPOLYGON
%token <ctype>	REQUEST_TOKEN_POINTSPOLYGONS
%token <ctype>	REQUEST_TOKEN_POINTSGENERALPOLYGONS
%token <ctype>	REQUEST_TOKEN_BASIS
%token <ctype>	REQUEST_TOKEN_PATCH
%token <ctype>	REQUEST_TOKEN_PATCHMESH
%token <ctype>	REQUEST_TOKEN_NUPATCH
%token <ctype>	REQUEST_TOKEN_TRIMCURVE
%token <ctype>	REQUEST_TOKEN_SPHERE
%token <ctype>	REQUEST_TOKEN_CONE
%token <ctype>	REQUEST_TOKEN_CYLINDER
%token <ctype>	REQUEST_TOKEN_HYPERBOLOID
%token <ctype>	REQUEST_TOKEN_PARABOLOID
%token <ctype>	REQUEST_TOKEN_DISK
%token <ctype>	REQUEST_TOKEN_TORUS
%token <ctype>	REQUEST_TOKEN_PROCEDURAL
%token <ctype>	REQUEST_TOKEN_GEOMETRY
%token <ctype>	REQUEST_TOKEN_SOLIDBEGIN
%token <ctype>	REQUEST_TOKEN_SOLIDEND
%token <ctype>	REQUEST_TOKEN_OBJECTBEGIN
%token <ctype>	REQUEST_TOKEN_OBJECTEND
%token <ctype>	REQUEST_TOKEN_OBJECTINSTANCE
%token <ctype>	REQUEST_TOKEN_MOTIONBEGIN
%token <ctype>	REQUEST_TOKEN_MOTIONEND
%token <ctype>	REQUEST_TOKEN_MAKETEXTURE
%token <ctype>	REQUEST_TOKEN_MAKEBUMP
%token <ctype>	REQUEST_TOKEN_MAKELATLONGENVIRONMENT
%token <ctype>	REQUEST_TOKEN_MAKECUBEFACEENVIRONMENT
%token <ctype>	REQUEST_TOKEN_MAKESHADOW
%token <ctype>	REQUEST_TOKEN_ERRORHANDLER
%token <ctype>	REQUEST_TOKEN_ERRORIGNORE
%token <ctype>	REQUEST_TOKEN_ERRORPRINT
%token <ctype>	REQUEST_TOKEN_ERRORABORT
%token <ctype>	REQUEST_TOKEN_SUBDIVISIONMESH

%type <ctype>	version
%type <ctype>	declare
%type <ctype>	framebegin
%type <ctype>	frameend
%type <ctype>	worldbegin
%type <ctype>	worldend
%type <ctype>	format
%type <ctype>	frameaspectratio
%type <ctype>	screenwindow
%type <ctype>	cropwindow
%type <ctype>	projection
%type <ctype>	clipping
%type <ctype>	depthoffield
%type <ctype>	shutter
%type <ctype>	pixelvariance
%type <ctype>	pixelsamples
%type <ctype>	pixelfilter
%type <ctype>	exposure
%type <ctype>	imager
%type <ctype>	quantize
%type <ctype>	display
%type <ctype>	hider
%type <ctype>	colorsamples
%type <ctype>	relativedetail
%type <ctype>	option
%type <ctype>	attributebegin
%type <ctype>	attributeend
%type <ctype>	color
%type <ctype>	opacity
%type <ctype>	texturecoordinates
%type <ctype>	lightsource
%type <ctype>	arealightsource
%type <ctype>	illuminate
%type <ctype>	surface
%type <ctype>	atmosphere
%type <ctype>	interior
%type <ctype>	exterior
%type <ctype>	shadingrate
%type <ctype>	shadinginterpolation
%type <ctype>	matte
%type <ctype>	bound
%type <ctype>	detail
%type <ctype>	detailrange
%type <ctype>	geometricapproximation
%type <ctype>	orientation
%type <ctype>	reverseorientation
%type <ctype>	sides
%type <ctype>	identity
%type <ctype>	transform
%type <ctype>	concattransform
%type <ctype>	perspective
%type <ctype>	translate
%type <ctype>	rotate
%type <ctype>	scale
%type <ctype>	skew
%type <ctype>	deformation
%type <ctype>	displacement
%type <ctype>	coordinatesystem
%type <ctype>	coordsystransform
%type <ctype>	transformpoints
%type <ctype>	transformbegin
%type <ctype>	transformend
%type <ctype>	attribute
%type <ctype>	polygon
%type <ctype>	generalpolygon
%type <ctype>	pointspolygons
%type <ctype>	pointsgeneralpolygons
%type <ctype>	basis
%type <ctype>	patch
%type <ctype>	patchmesh
%type <ctype>	nupatch
%type <ctype>	trimcurve
%type <ctype>	sphere
%type <ctype>	cone
%type <ctype>	cylinder
%type <ctype>	hyperboloid
%type <ctype>	paraboloid
%type <ctype>	disk
%type <ctype>	torus
%type <ctype>	geometry
%type <ctype>	solidbegin
%type <ctype>	solidend
%type <ctype>	objectbegin
%type <ctype>	objectend
%type <ctype>	objectinstance
%type <ctype>	motionbegin
%type <ctype>	motionend
%type <ctype>	maketexture
%type <ctype>	makebump
%type <ctype>	makelatlongenvironment
%type <ctype>	makecubefaceenvironment
%type <ctype>	makeshadow
%type <ctype>	errorhandler
%type <ctype>	errorignore
%type <ctype>	errorprint
%type <ctype>	errorabort
%type <ctype>	subdivisionmesh

/* -------------- rules section -------------- */
%%
file : requests						{ YYABORT; }
	;

requests : request			
	|	requests request	
	;

request : complete_request			{ExpectRequest();}

complete_request	
	:	version float				{}
	|		declare string string 	{
										RiDeclare((char*)($2->c_str()), (char*)($3->c_str()));
										DiscardStringValue($2);
										DiscardStringValue($3);
									}
	|		framebegin integer		{ RiFrameBegin($2); }
	|		frameend				{ RiFrameEnd(); }
	|		worldbegin				{ RiWorldBegin(); }
	|		worldend				{ RiWorldEnd();	/* YYACCEPT; */ }
	|		format integer integer float
									{ RiFormat($2,$3,$4); }
	|		frameaspectratio float	{ RiFrameAspectRatio($2); }
	|		screenwindow	float float float float 
									{ RiScreenWindow($2,$3,$4,$5); }
	|		cropwindow float float float float
									{ RiCropWindow($2,$3,$4,$5); }
	|		projection string opttvpairs
									{
										RiProjectionV((char*)($2->c_str()),
														$3.atokens->size(), 
														&(*$3.atokens)[0],
														&(*$3.avalues)[0]);
										DiscardStringValue($2);
										DiscardTokenValuePairs(&($3));
									}
	|		clipping float float	{ RiClipping($2,$3); }
	|		depthoffield float float float
									{ RiDepthOfField($2,$3,$4); }
	|		shutter float float		{ RiShutter($2,$3); }
	|		pixelvariance float		{ RiPixelVariance($2); }
	|		pixelsamples float float
									{ RiPixelSamples($2,$3); }
	|		pixelfilter string float float
									{
										RtFilterFunc pFilter=RiGaussianFilter;
										// TODO: names shoudl be stored elsewhere.
										if(strcmp($2->c_str(),"box")==0)		pFilter=RiBoxFilter;
										if(strcmp($2->c_str(),"triangle")==0)	pFilter=RiTriangleFilter;
										if(strcmp($2->c_str(),"sinc")==0)		pFilter=RiSincFilter;
										if(strcmp($2->c_str(),"catmull-rom")==0) pFilter=RiCatmullRomFilter;
										RiPixelFilter(pFilter,$3,$4);
										DiscardStringValue($2);
									}
	|		exposure float float	{ RiExposure($2,$3); }
	|		imager string opttvpairs
									{
										RiImagerV((char*)($2->c_str()),
														$3.atokens->size(), 
														&(*$3.atokens)[0],
														&(*$3.avalues)[0]);
										DiscardStringValue($2);
										DiscardTokenValuePairs(&($3));
									}
	|		quantize string integer integer integer float					
									{
										RiQuantize((char*)($2->c_str()),$3,$4,$5,$6);
										DiscardStringValue($2);
									}
	|		display string string string opttvpairs
									{
										RiDisplay($2->c_str(), (char*)$3->c_str(), (char*)$4->c_str(),
														$5.atokens->size(), 
														&(*$5.atokens)[0],
														&(*$5.avalues)[0]);
										
										DiscardStringValue($2);
										DiscardStringValue($3);
										DiscardStringValue($4);
										DiscardTokenValuePairs(&($5));
									}
	|		hider string opttvpairs	{
										RiHiderV((char*)($2->c_str()),
														$3.atokens->size(), 
														&(*$3.atokens)[0],
														&(*$3.avalues)[0]);
										DiscardStringValue($2);
										DiscardTokenValuePairs(&($3));
									}
	|		colorsamples array array				
									{
										if(CheckArrayType(Type_Float,$2) &&
										   CheckArrayType(Type_Float,$3))
										{
											RiColorSamples($2.aftype->size(), &(*$2.aftype)[0], &(*$3.aftype)[0]); 
										}
										else
											yyerror("Expecting float array");
										DiscardArrayValue($2);
										DiscardArrayValue($3);
									}
	|		relativedetail float	{ RiRelativeDetail($2); }
	|		option string opttvpairs{
										RiOptionV((char*)($2->c_str()),
														$3.atokens->size(), 
														&(*$3.atokens)[0],
														&(*$3.avalues)[0]);
										DiscardStringValue($2);
										DiscardTokenValuePairs(&($3));
									}
	|		attributebegin			{ RiAttributeBegin(); }
	|		attributeend			{ RiAttributeEnd(); }
	|		color	array		{
										if(CheckArrayType(Type_Float,$2))
											RiColor(&(*$2.aftype)[0]);
										else
											yyerror("Expecting float array");
										DiscardArrayValue($2);
									}
	|		color	float float float
									{
										RtColor Cq;
										Cq[0]=$2;
										Cq[1]=$3;
										Cq[2]=$4;
										RiColor(Cq);
									}
	|		opacity array		{
										if(CheckArrayType(Type_Float,$2))
											RiOpacity(&(*$2.aftype)[0]);
										else
											yyerror("Expecting float array");
										DiscardArrayValue($2);
									}
	|		opacity	float float float
									{
										RtColor Cq;
										Cq[0]=$2;
										Cq[1]=$3;
										Cq[2]=$4;
										RiOpacity(Cq);
									}
	|		texturecoordinates float float float float float float float float
									{ RiTextureCoordinates($2,$3,$4,$5,$6,$7,$8,$9); }
	|		texturecoordinates array
									{
										if(CheckArrayType(Type_Float,$2) &&
										   $2.aftype->size()>=8)
											RiTextureCoordinates((*$2.aftype)[0],(*$2.aftype)[1],(*$2.aftype)[2],(*$2.aftype)[3],(*$2.aftype)[4],(*$2.aftype)[5],(*$2.aftype)[6],(*$2.aftype)[7]);
										else
											yyerror("Expecting 8 floats");
										DiscardArrayValue($2);
									}
	|		lightsource string integer opttvpairs				
									{
										RtLightHandle hlNew=RiLightSourceV((char*)($2->c_str()),
														$4.atokens->size(), 
														&(*$4.atokens)[0],
														&(*$4.avalues)[0]);
										// Store a new lightsource map entry for the specified light
										SqLightsourceMap lmpNew($3,hlNew);
										m_almpCurrent.push_back(lmpNew);
										DiscardStringValue($2);
										DiscardTokenValuePairs(&($4));
									}
	|		arealightsource string integer opttvpairs				
									{
										RtLightHandle hlNew=RiAreaLightSourceV((char*)($2->c_str()),
														$4.atokens->size(), 
														&(*$4.atokens)[0],
														&(*$4.avalues)[0]);
										// Store a new lightsource map entry for the specified light
										SqLightsourceMap lmpNew($3,hlNew);
										m_almpCurrent.push_back(lmpNew);
										DiscardStringValue($2);
										DiscardTokenValuePairs(&($4));
									}
	|		illuminate integer integer
									{ 
										// Search for the relevant entry in the ligthsource map table
										TqInt i;
										for(i=0; i<m_almpCurrent.size(); i++)
										{
											if(m_almpCurrent[i].m_RIBSeqNum==$2)
												RiIlluminate(m_almpCurrent[i].m_Handle,$3!=0);
										}
									}
	|		surface string opttvpairs
									{
										RiSurfaceV((char*)($2->c_str()),
														$3.atokens->size(), 
														&(*$3.atokens)[0],
														&(*$3.avalues)[0]);
										DiscardStringValue($2);
										DiscardTokenValuePairs(&($3));
									}
	|		atmosphere string opttvpairs
									{
										RiAtmosphereV((char*)($2->c_str()),
														$3.atokens->size(), 
														&(*$3.atokens)[0],
														&(*$3.avalues)[0]);
										DiscardStringValue($2);
										DiscardTokenValuePairs(&($3));
									}
	|		interior string opttvpairs
									{
										RiInteriorV((char*)($2->c_str()),
														$3.atokens->size(), 
														&(*$3.atokens)[0],
														&(*$3.avalues)[0]);
										DiscardStringValue($2);
										DiscardTokenValuePairs(&($3));
									}
	|		exterior string opttvpairs
									{
										RiExteriorV((char*)($2->c_str()),
														$3.atokens->size(), 
														&(*$3.atokens)[0],
														&(*$3.avalues)[0]);
										DiscardStringValue($2);
										DiscardTokenValuePairs(&($3));
									}
	|		shadingrate float		{ RiShadingRate($2); }
	|		shadinginterpolation string
									{
										RiShadingInterpolation((char*)($2->c_str()));
										DiscardStringValue($2);
									}
	|		matte integer			{ RiMatte($2!=0); }
	|		bound	float float float float float float
									{ 	
										RtBound b;
										b[0]=$2; b[1]=$3; b[2]=$4; b[3]=$5; b[4]=$6; b[5]=$7;
										RiBound(b); 
									}
	|		bound	array		{ 
										if(CheckArrayType(Type_Float,$2) &&
										   $2.aftype->size()>=6)
											RiBound(&(*$2.aftype)[0]);
										else
											yyerror("Expecting 6 floats");
										DiscardArrayValue($2);
									}
	|		detail float float float float float float
									{ 
										RtBound b;
										b[0]=$2; b[1]=$3; b[2]=$4; b[3]=$5; b[4]=$6; b[5]=$7;
										RiDetail(b); 
									}
	|		detail array		{ 
										if(CheckArrayType(Type_Float,$2) &&
										   $2.aftype->size()>=6)							
											RiDetail(&(*$2.aftype)[0]);
										else
											yyerror("Expecting 6 floats");
										DiscardArrayValue($2);
									}
	|		detailrange float float float float
									{ RiDetailRange($2,$3,$4,$5); }
	|		detailrange array	{ 
										if(CheckArrayType(Type_Float,$2) &&
										   $2.aftype->size()>=4)							
											RiDetailRange((*$2.aftype)[0],(*$2.aftype)[1],(*$2.aftype)[2],(*$2.aftype)[3]);
										else
											yyerror("Expecting 4 floats");
										DiscardArrayValue($2);
									}
	|		geometricapproximation string float
									{
										RiGeometricApproximation((char*)($2->c_str()),$3);
										DiscardStringValue($2);
									}
	|		orientation string		{
										RiOrientation((char*)($2->c_str()));
										DiscardStringValue($2);
									}
	|		reverseorientation		{ RiReverseOrientation(); }
	|		sides integer			{ RiSides($2); }
	|		identity				{ RiIdentity(); }
	|		transform array	{
										RtMatrix Trans;
										if(CheckArrayType(Type_Float,$2) &&
										   $2.aftype->size()>=16)							
										{
											Trans[0][0]=(*$2.aftype)[0];
											Trans[0][1]=(*$2.aftype)[1];
											Trans[0][2]=(*$2.aftype)[2];
											Trans[0][3]=(*$2.aftype)[3];
											Trans[1][0]=(*$2.aftype)[4];
											Trans[1][1]=(*$2.aftype)[5];
											Trans[1][2]=(*$2.aftype)[6];
											Trans[1][3]=(*$2.aftype)[7];
											Trans[2][0]=(*$2.aftype)[8];
											Trans[2][1]=(*$2.aftype)[9];
											Trans[2][2]=(*$2.aftype)[10];
											Trans[2][3]=(*$2.aftype)[11];
											Trans[3][0]=(*$2.aftype)[12];
											Trans[3][1]=(*$2.aftype)[13];
											Trans[3][2]=(*$2.aftype)[14];
											Trans[3][3]=(*$2.aftype)[15];
											RiTransform(Trans);
										}
										else
											yyerror("Expecting 16 floats");
										DiscardArrayValue($2);
									}
	|		concattransform array
									{
										RtMatrix Trans;
										if(CheckArrayType(Type_Float,$2) &&
										   $2.aftype->size()>=16)							
										{
											Trans[0][0]=(*$2.aftype)[0];
											Trans[0][1]=(*$2.aftype)[1];
											Trans[0][2]=(*$2.aftype)[2];
											Trans[0][3]=(*$2.aftype)[3];
											Trans[1][0]=(*$2.aftype)[4];
											Trans[1][1]=(*$2.aftype)[5];
											Trans[1][2]=(*$2.aftype)[6];
											Trans[1][3]=(*$2.aftype)[7];
											Trans[2][0]=(*$2.aftype)[8];
											Trans[2][1]=(*$2.aftype)[9];
											Trans[2][2]=(*$2.aftype)[10];
											Trans[2][3]=(*$2.aftype)[11];
											Trans[3][0]=(*$2.aftype)[12];
											Trans[3][1]=(*$2.aftype)[13];
											Trans[3][2]=(*$2.aftype)[14];
											Trans[3][3]=(*$2.aftype)[15];
											RiConcatTransform(Trans);
										}
										else
											yyerror("Expecting 16 floats");
										DiscardArrayValue($2);
									}
	|		perspective float		{ RiPerspective($2); }
	|		translate	float float float				
									{ RiTranslate($2,$3,$4); }
	|		rotate float float float float 
									{ RiRotate($2,$3,$4,$5); }
	|		scale float float float	{ RiScale($2,$3,$4); }
	|		skew float float float float float float float
									{ RiSkew($2,$3,$4,$5,$6,$7,$8); }
	|		skew array		{
										if(CheckArrayType(Type_Float,$2) &&
										   $2.aftype->size()>=7)							
											RiSkew((*$2.aftype)[0],(*$2.aftype)[1],(*$2.aftype)[2],(*$2.aftype)[3],(*$2.aftype)[4],(*$2.aftype)[5],(*$2.aftype)[6]);
										else
											yyerror("Expecting 7 floats");
										DiscardArrayValue($2);
									}
	|		deformation string opttvpairs
									{
										RiDeformationV((char*)($2->c_str()),
														$3.atokens->size(), 
														&(*$3.atokens)[0],
														&(*$3.avalues)[0]);
										DiscardStringValue($2);
										DiscardTokenValuePairs(&($3));
									}
	|		displacement string opttvpairs
									{
										RiDisplacementV((char*)($2->c_str()),
														$3.atokens->size(), 
														&(*$3.atokens)[0],
														&(*$3.avalues)[0]);
										DiscardStringValue($2);
										DiscardTokenValuePairs(&($3));
									}
	|		coordinatesystem string	{
										RiCoordinateSystem((char*)($2->c_str()));
										DiscardStringValue($2);
									}
	|		coordsystransform string	{
										RiCoordSysTransform((char*)($2->c_str()));
										DiscardStringValue($2);
									}
	|		transformpoints			{printf("TRANSFORMPOINTS\n");}
	|		transformbegin			{ RiTransformBegin(); }
	|		transformend			{ RiTransformEnd(); }
	|		attribute string opttvpairs
									{
										RiAttributeV((char*)($2->c_str()),
														$3.atokens->size(), 
														&(*$3.atokens)[0],
														&(*$3.avalues)[0]);
										DiscardStringValue($2);
										DiscardTokenValuePairs(&($3));
									}
	|		polygon tvpairs			{
										// TODO: Find size of polygon from "P" parameter.
										TqInt cVerts;
										TqInt i;
										for(i=0; i<$2.atokens->size(); i++)
										{
											if(strcmp((*$2.atokens)[i],"P")==0)
												cVerts=(*$2.acounts)[i]/3;
										}
										RiPolygonV(cVerts,$2.atokens->size()
													,&(*$2.atokens)[0]
													,&(*$2.avalues)[0]);
										
										DiscardTokenValuePairs(&($2));
									}
	|		generalpolygon array tvpairs
									{
										if(CheckArrayType(Type_Float,$2))
										{
											RiGeneralPolygonV($2.aitype->size(),&(*$2.aitype)[0],
															$3.atokens->size(), 
															&(*$3.atokens)[0],
															&(*$3.avalues)[0]);
										}
										else
											yyerror("Expecting float array");
										DiscardArrayValue($2);
										DiscardTokenValuePairs(&($3));
									}
	|		pointspolygons array array tvpairs
									{
										if(CheckArrayType(Type_Integer,$2) &&
										   CheckArrayType(Type_Integer,$3))
										{
											RiPointsPolygonsV(
														 $2.aitype->size(),&(*$2.aitype)[0], &(*$3.aitype)[0],
															$4.atokens->size(), 
															&(*$4.atokens)[0],
															&(*$4.avalues)[0]);
										}
										else
											yyerror("Expecting two integer arrays");
										
										DiscardArrayValue($2);
										DiscardArrayValue($3);
										DiscardTokenValuePairs(&($4));
									}
	|		pointsgeneralpolygons	array array array tvpairs	
									{
										if(CheckArrayType(Type_Integer,$2) &&
										   CheckArrayType(Type_Integer,$3) &&
										   CheckArrayType(Type_Integer,$4))
										{
											RiPointsGeneralPolygonsV($2.aitype->size(), &(*$2.aitype)[0], &(*$3.aitype)[0], &(*$4.aitype)[0],
															$5.atokens->size(), 
															&(*$5.atokens)[0],
															&(*$5.avalues)[0]);
										}
										else
											yyerror("Expecting 3 integer arrays");
										DiscardArrayValue($2);
										DiscardArrayValue($3);
										DiscardArrayValue($4);
										DiscardTokenValuePairs(&($5));
									}
	|		basis string integer string integer
									{										
										RtBasis u,v;
										BasisFromName(u,$2->c_str());
										BasisFromName(v,$4->c_str());
										RiBasis(u,$3,v,$5);
										DiscardStringValue($2);
										DiscardStringValue($4);
									}
	|		basis string integer array integer
									{										
										if(CheckArrayType(Type_Float,$4) &&
										   $4.aftype->size()>=16)
										{
											RtBasis u,v;
											TqInt i,j;
											for(i=0; i<4; i++)
												for(j=0; j<4; j++)
													v[i][j]=(*$4.aftype)[(i*4)+j];

											BasisFromName(u,$2->c_str());
											RiBasis(u,$3,v,$5);						
										}
										else
											yyerror("Expecting 16 floats");
										DiscardStringValue($2);
										DiscardArrayValue($4);
									}
	|		basis array integer string integer
									{										
										if(CheckArrayType(Type_Float,$2) &&
										   $2.aftype->size()>=16)
										{
											RtBasis u,v;
											TqInt i,j;
											for(i=0; i<4; i++)
												for(j=0; j<4; j++)
													u[i][j]=(*$2.aftype)[(i*4)+j];

											BasisFromName(v,$4->c_str());
											RiBasis(u,$3,v,$5);						
										}
										else
											yyerror("Expecting 16 floats");	
										DiscardArrayValue($2);
										DiscardStringValue($4);
									}
	|		basis array integer array integer
									{		
										if(CheckArrayType(Type_Float,$2) &&
										   CheckArrayType(Type_Float,$4) &&
										   $2.aftype->size()>=16 &&
										   $4.aftype->size()>=16)
										{
											// Build a matrix for the u and v directions
											RtBasis u,v;
											TqInt i,j;
											for(i=0; i<4; i++)
											{
												for(j=0; j<4; j++)
												{
													u[i][j]=(*$2.aftype)[(i*4)+j];
													v[i][j]=(*$4.aftype)[(i*4)+j];
												}
											}		
											RiBasis(u,$3,v,$5);						
										}
										else
											yyerror("Expecting 16 floats");
										DiscardArrayValue($2);
										DiscardArrayValue($4);
									}
	|		patch	string tvpairs	{
										RiPatchV((char*)($2->c_str()),
														$3.atokens->size(), 
														&(*$3.atokens)[0],
														&(*$3.avalues)[0]);
										DiscardStringValue($2);
										DiscardTokenValuePairs(&($3));
									}
	|		patchmesh	string integer string integer string tvpairs
									{
										RiPatchMeshV((char*)($2->c_str()),
												 $3, (char*)($4->c_str()),
												 $5, (char*)($6->c_str()),
														$7.atokens->size(), 
														&(*$7.atokens)[0],
														&(*$7.avalues)[0]);
										DiscardStringValue($2);
										DiscardStringValue($4);
										DiscardStringValue($6);
										DiscardTokenValuePairs(&($7));
									}
	|		nupatch integer integer array float float integer integer array float float tvpairs					
									{
										if(CheckArrayType(Type_Float,$4) &&
										   CheckArrayType(Type_Float,$9))
											RiNuPatchV($2,$3,&(*$4.aftype)[0],$5,$6,
													   $7,$8,&(*$9.aftype)[0],$10,$11,
															$12.atokens->size(), 
															&(*$12.atokens)[0],
															&(*$12.avalues)[0]);
										else
											yyerror("Expecting float array");
										DiscardArrayValue($4);
										DiscardArrayValue($9);
										DiscardTokenValuePairs(&($12));
									}
	|		trimcurve	array array array array array array array array array
									{
										if(CheckArrayType(Type_Integer,$2) &&
										   CheckArrayType(Type_Integer,$3) &&
										   CheckArrayType(Type_Float,$4) &&
										   CheckArrayType(Type_Float,$5) &&
										   CheckArrayType(Type_Float,$6) &&
										   CheckArrayType(Type_Integer,$7) &&
										   CheckArrayType(Type_Float,$8) &&
										   CheckArrayType(Type_Float,$9) &&
										   CheckArrayType(Type_Float,$10))
											RiTrimCurve($2.aitype->size(),
														&(*$2.aitype)[0],
														&(*$3.aitype)[0],
														&(*$4.aftype)[0],
														&(*$5.aftype)[0],
														&(*$6.aftype)[0],
														&(*$7.aitype)[0],
														&(*$8.aftype)[0],
														&(*$9.aftype)[0],
														&(*$10.aftype)[0]);
										else
											// TODO: more informative error
											yyerror("Invalid array arguments");
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
	|		sphere float float float float opttvpairs
									{
										RiSphereV($2,$3,$4,$5,
														$6.atokens->size(), 
														&(*$6.atokens)[0],
														&(*$6.avalues)[0]);
										DiscardTokenValuePairs(&($6));
									}
	|		cone float float float opttvpairs
									{
										RiConeV($2,$3,$4,
														$5.atokens->size(), 
														&(*$5.atokens)[0],
														&(*$5.avalues)[0]);
										DiscardTokenValuePairs(&($5));
									}
	|		cylinder float float float float opttvpairs
									{
										RiCylinderV($2,$3,$4,$5,
														$6.atokens->size(), 
														&(*$6.atokens)[0],
														&(*$6.avalues)[0]);
										DiscardTokenValuePairs(&($6));
									}
	|		hyperboloid array opttvpairs
									{
										if(CheckArrayType(Type_Float,$2))
											RiHyperboloidV(&(*$2.aftype)[0],&(*$2.aftype)[3],(*$2.aftype)[6],
															$3.atokens->size(), 
															&(*$3.atokens)[0],
															&(*$3.avalues)[0]);
										else
											yyerror("Expecting float array");
										DiscardArrayValue($2);
										DiscardTokenValuePairs(&($3));
									}
	|		hyperboloid float float float float float float float opttvpairs
									{
										RtPoint p1, p2;
										p1[0]=$2;
										p1[1]=$3;
										p1[2]=$4;
										p2[0]=$5;
										p2[1]=$6;
										p2[2]=$7;
										RiHyperboloidV(p1,p2,$8,
														$9.atokens->size(), 
														&(*$9.atokens)[0],
														&(*$9.avalues)[0]);
										DiscardTokenValuePairs(&($9));
									}
	|		paraboloid float float float float opttvpairs
									{
										RiParaboloidV($2,$3,$4,$5,
														$6.atokens->size(), 
														&(*$6.atokens)[0],
														&(*$6.avalues)[0]);
										DiscardTokenValuePairs(&($6));
									}
	|		disk float float float opttvpairs
									{
										RiDiskV($2,$3,$4,
														$5.atokens->size(), 
														&(*$5.atokens)[0],
														&(*$5.avalues)[0]);
										DiscardTokenValuePairs(&($5));
									}
	|		torus float float float float float opttvpairs
									{
										RiTorusV($2,$3,$4,$5,$6,
														$7.atokens->size(), 
														&(*$7.atokens)[0],
														&(*$7.avalues)[0]);
										DiscardTokenValuePairs(&($7));
									}
	|		geometry string opttvpairs
									{
										RiGeometryV((char*)($2->c_str()),
														$3.atokens->size(), 
														&(*$3.atokens)[0],
														&(*$3.avalues)[0]);
										DiscardStringValue($2);
										DiscardTokenValuePairs(&($3));
									}
	|		solidbegin string		{
										RiSolidBegin((char*)($2->c_str()));
										DiscardStringValue($2);
									}
	|		solidend				{ RiSolidEnd(); }
	|		objectbegin integer		{
										// TODO: Record information about handle.
										RiObjectBegin();
									}
	|		objectend				{ RiObjectEnd(); }
	|		objectinstance integer	{
										// TODO: Find object pointer from handle.
										RiObjectInstance(0);
									}
	|		motionbegin array	{
										if(CheckArrayType(Type_Float,$2))										
											RiMotionBeginV($2.aftype->size(), &(*$2.aftype)[0]);
										else
											yyerror("Expecting float array");
										DiscardArrayValue($2);
									}
	|		motionend				{ RiMotionEnd(); }
	|		maketexture	string string string string string float float opttvpairs
									{
										RtFilterFunc pFilter=RiGaussianFilter;
										// TODO: names shoudl be stored elsewhere.
										if(strcmp($6->c_str(),"box")==0)		pFilter=RiBoxFilter;
										if(strcmp($6->c_str(),"triangle")==0)	pFilter=RiTriangleFilter;
										if(strcmp($6->c_str(),"sinc")==0)		pFilter=RiSincFilter;
										if(strcmp($6->c_str(),"catmull-rom")==0) pFilter=RiCatmullRomFilter;

										RiMakeTexture((char*)($2->c_str()), (char*)($3->c_str()), /*in, out*/
													  (char*)($4->c_str()), (char*)($5->c_str()), /*swrap, twrap*/
													  pFilter, /*filterfunc*/
													  $7, $8 /*swidth, twidth*/);
									}
	|		makebump string string string string string float float opttvpairs
									{
										RtFilterFunc pFilter=RiGaussianFilter;
										// TODO: names shoudl be stored elsewhere.
										if(strcmp($6->c_str(),"box")==0)		pFilter=RiBoxFilter;
										if(strcmp($6->c_str(),"triangle")==0)	pFilter=RiTriangleFilter;
										if(strcmp($6->c_str(),"sinc")==0)		pFilter=RiSincFilter;
										if(strcmp($6->c_str(),"catmull-rom")==0) pFilter=RiCatmullRomFilter;

										RiMakeBump((char*)($2->c_str()), (char*)($3->c_str()), /*in, out*/
												   (char*)($4->c_str()), (char*)($5->c_str()), /*swrap, twrap*/
												   pFilter, /*filterfunc*/
												   $7, $8 /*swidth, twidth*/);
									}
	|		makelatlongenvironment string string string float float opttvpairs
									{
										RtFilterFunc pFilter=RiGaussianFilter;
										// TODO: names shoudl be stored elsewhere.
										if(strcmp($4->c_str(),"box")==0)		pFilter=RiBoxFilter;
										if(strcmp($4->c_str(),"triangle")==0)	pFilter=RiTriangleFilter;
										if(strcmp($4->c_str(),"sinc")==0)		pFilter=RiSincFilter;
										if(strcmp($4->c_str(),"catmull-rom")==0) pFilter=RiCatmullRomFilter;

										RiMakeLatLongEnvironment(
												   (char*)($2->c_str()), (char*)($3->c_str()), /*in, out*/
												   pFilter, /*filterfunc*/
												   $5, $6 /*swidth, twidth*/);
									}
	|		makecubefaceenvironment	string string string string string string string float string float float opttvpairs
									{
										RtFilterFunc pFilter=RiGaussianFilter;
										// TODO: names should be stored elsewhere.
										if(strcmp($10->c_str(),"box")==0)		pFilter=RiBoxFilter;
										if(strcmp($10->c_str(),"triangle")==0)	pFilter=RiTriangleFilter;
										if(strcmp($10->c_str(),"sinc")==0)		pFilter=RiSincFilter;
										if(strcmp($10->c_str(),"catmull-rom")==0) pFilter=RiCatmullRomFilter;

										RiMakeCubeFaceEnvironment(
												   (char*)($2->c_str()), (char*)($3->c_str()), /*px, nx*/
												   (char*)($4->c_str()), (char*)($5->c_str()), /*py, ny*/
												   (char*)($6->c_str()), (char*)($7->c_str()), /*pz, nz*/
												   (char*)($8->c_str()), /*texturename*/
												   $9, /*fov*/
												   pFilter, /*filterfunc*/
												   $11, $12 /*swidth, twidth*/);
									}
	|		makeshadow string string opttvpairs
									{
										RiMakeShadow((char*)($2->c_str()), (char*)($3->c_str()));
									}
	|		errorhandler string		{
										// TODO: names should be stored elsewhere.
										if(strcmp($2->c_str(),"ignore")==0)		RiErrorHandler(&RiErrorIgnore);
										else if(strcmp($2->c_str(),"print")==0)	RiErrorHandler(&RiErrorPrint);
										else if(strcmp($2->c_str(),"abort")==0)	RiErrorHandler(&RiErrorAbort);
									}
	|		errorignore				{printf("ERRORIGNORE\n");}
	|		errorprint				{printf("ERRORPRINT\n");}
	|		errorabort				{printf("ERRORABORT\n");}
	|		subdivisionmesh string array array array array array array tvpairs
									{
										if(CheckArrayType(Type_Integer,$3) &&
										   CheckArrayType(Type_Integer,$4) &&
										   CheckArrayType(Type_String,$5) &&
										   CheckArrayType(Type_Integer,$6) &&
										   CheckArrayType(Type_Integer,$7) &&
										   CheckArrayType(Type_Float,$8))
										{
											char** ptags=new char*[$5.astype->size()];
											int i;
											for(i=0; i<$5.astype->size(); i++)
											{
												char* ptag=new char[(*$5.astype)[i]->size()];
												strncpy(ptag,(*$5.astype)[i]->c_str(),(*$5.astype)[i]->size());
												ptags[i]=ptag;
											}
											
											RiSubdivisionMeshV((char*)($2->c_str()),
															$3.aitype->size(),&(*$3.aitype)[0], &(*$4.aitype)[0],
															$5.astype->size(), ptags, 
															&(*$6.aitype)[0],
															&(*$7.aitype)[0],
															&(*$8.aftype)[0],
															$9.atokens->size(), 
															&(*$9.atokens)[0],
															&(*$9.avalues)[0]);
											
											for(i=0; i<$5.astype->size(); i++)
												delete[](ptags[i]);
											delete[](ptags);
										}
										else
											// TODO: More meaningful error
											yyerror("Invalid array arguments");

										DiscardStringValue($2);
										DiscardArrayValue($3);
										DiscardArrayValue($4);
										DiscardArrayValue($5);
										DiscardArrayValue($6);
										DiscardArrayValue($7);
										DiscardArrayValue($8);
										DiscardTokenValuePairs(&($9));
									}
	|		subdivisionmesh string array array tvpairs
									{
										if(CheckArrayType(Type_Integer,$3) &&
										   CheckArrayType(Type_Integer,$3))
										{
											RiSubdivisionMeshV((char*)($2->c_str()),
															$3.aitype->size(),&(*$3.aitype)[0], &(*$4.aitype)[0],
															0, NULL, 
															NULL,
															NULL,
															NULL,
															$5.atokens->size(), 
															&(*$5.atokens)[0],
															&(*$5.avalues)[0]);
										}
										else
											yyerror("Invalid array arguments");
										
										DiscardStringValue($2);
										DiscardArrayValue($3);
										DiscardArrayValue($4);
										DiscardTokenValuePairs(&($5));
									}
	|		UNKNOWN_TOKEN			{
										// Print the error, then force the scanner into 'request'
										// mode which will make it ignore all parameters until the
										// next request token.
										yyerror("Unrecognised RIB request");
										ExpectRequest();
									}
	;

version : REQUEST_TOKEN_VERSION						{ExpectParams();}
declare : REQUEST_TOKEN_DECLARE						{ExpectParams();}
framebegin : REQUEST_TOKEN_FRAMEBEGIN				{ExpectParams();}
frameend : REQUEST_TOKEN_FRAMEEND					{ExpectParams();}
worldbegin : REQUEST_TOKEN_WORLDBEGIN				{ExpectParams();}
worldend : REQUEST_TOKEN_WORLDEND					{ExpectParams();}
format : REQUEST_TOKEN_FORMAT						{ExpectParams();}
frameaspectratio : REQUEST_TOKEN_FRAMEASPECTRATIO	{ExpectParams();}
screenwindow : REQUEST_TOKEN_SCREENWINDOW			{ExpectParams();}
cropwindow : REQUEST_TOKEN_CROPWINDOW				{ExpectParams();}
projection : REQUEST_TOKEN_PROJECTION				{ExpectParams();}
clipping : REQUEST_TOKEN_CLIPPING					{ExpectParams();}
depthoffield : REQUEST_TOKEN_DEPTHOFFIELD			{ExpectParams();}
shutter : REQUEST_TOKEN_SHUTTER						{ExpectParams();}
pixelvariance : REQUEST_TOKEN_PIXELVARIANCE			{ExpectParams();}
pixelsamples : REQUEST_TOKEN_PIXELSAMPLES			{ExpectParams();}
pixelfilter : REQUEST_TOKEN_PIXELFILTER				{ExpectParams();}
exposure : REQUEST_TOKEN_EXPOSURE					{ExpectParams();}
imager : REQUEST_TOKEN_IMAGER						{ExpectParams();}
quantize : REQUEST_TOKEN_QUANTIZE					{ExpectParams();}
display : REQUEST_TOKEN_DISPLAY						{ExpectParams();}
hider : REQUEST_TOKEN_HIDER							{ExpectParams();}
colorsamples : REQUEST_TOKEN_COLORSAMPLES			{ExpectParams();}
relativedetail : REQUEST_TOKEN_RELATIVEDETAIL		{ExpectParams();}
option : REQUEST_TOKEN_OPTION						{ExpectParams();}
attributebegin : REQUEST_TOKEN_ATTRIBUTEBEGIN		{ExpectParams();}
attributeend : REQUEST_TOKEN_ATTRIBUTEEND			{ExpectParams();}
color : REQUEST_TOKEN_COLOR							{ExpectParams();}
opacity : REQUEST_TOKEN_OPACITY						{ExpectParams();}
texturecoordinates : REQUEST_TOKEN_TEXTURECOORDINATES {ExpectParams();}
lightsource : REQUEST_TOKEN_LIGHTSOURCE				{ExpectParams();}
arealightsource : REQUEST_TOKEN_AREALIGHTSOURCE		{ExpectParams();}
illuminate : REQUEST_TOKEN_ILLUMINATE				{ExpectParams();}
surface : REQUEST_TOKEN_SURFACE						{ExpectParams();}
atmosphere : REQUEST_TOKEN_ATMOSPHERE				{ExpectParams();}
interior : REQUEST_TOKEN_INTERIOR					{ExpectParams();}
exterior : REQUEST_TOKEN_EXTERIOR					{ExpectParams();}
shadingrate : REQUEST_TOKEN_SHADINGRATE				{ExpectParams();}
shadinginterpolation : REQUEST_TOKEN_SHADINGINTERPOLATION {ExpectParams();}
matte : REQUEST_TOKEN_MATTE							{ExpectParams();}
bound : REQUEST_TOKEN_BOUND							{ExpectParams();}
detail : REQUEST_TOKEN_DETAIL						{ExpectParams();}
detailrange : REQUEST_TOKEN_DETAILRANGE				{ExpectParams();}
geometricapproximation : REQUEST_TOKEN_GEOMETRICAPPROXIMATION {ExpectParams();}
orientation : REQUEST_TOKEN_ORIENTATION				{ExpectParams();}
reverseorientation : REQUEST_TOKEN_REVERSEORIENTATION {ExpectParams();}
sides : REQUEST_TOKEN_SIDES							{ExpectParams();}
identity : REQUEST_TOKEN_IDENTITY					{ExpectParams();}
transform : REQUEST_TOKEN_TRANSFORM					{ExpectParams();}
concattransform : REQUEST_TOKEN_CONCATTRANSFORM		{ExpectParams();}
perspective : REQUEST_TOKEN_PERSPECTIVE				{ExpectParams();}
translate : REQUEST_TOKEN_TRANSLATE					{ExpectParams();}
rotate : REQUEST_TOKEN_ROTATE						{ExpectParams();}
scale : REQUEST_TOKEN_SCALE							{ExpectParams();}
skew : REQUEST_TOKEN_SKEW							{ExpectParams();}
deformation : REQUEST_TOKEN_DEFORMATION				{ExpectParams();}
displacement : REQUEST_TOKEN_DISPLACEMENT			{ExpectParams();}
coordinatesystem : REQUEST_TOKEN_COORDINATESYSTEM	{ExpectParams();}
coordsystransform : REQUEST_TOKEN_COORDSYSTRANSFORM	{ExpectParams();}
transformpoints : REQUEST_TOKEN_TRANSFORMPOINTS		{ExpectParams();}
transformbegin : REQUEST_TOKEN_TRANSFORMBEGIN		{ExpectParams();}
transformend : REQUEST_TOKEN_TRANSFORMEND			{ExpectParams();}
attribute : REQUEST_TOKEN_ATTRIBUTE					{ExpectParams();}
polygon : REQUEST_TOKEN_POLYGON						{ExpectParams();}
generalpolygon : REQUEST_TOKEN_GENERALPOLYGON		{ExpectParams();}
pointspolygons : REQUEST_TOKEN_POINTSPOLYGONS		{ExpectParams();}
pointsgeneralpolygons : REQUEST_TOKEN_POINTSGENERALPOLYGONS {ExpectParams();}
basis : REQUEST_TOKEN_BASIS							{ExpectParams();}
patch : REQUEST_TOKEN_PATCH							{ExpectParams();}
patchmesh : REQUEST_TOKEN_PATCHMESH					{ExpectParams();}
nupatch : REQUEST_TOKEN_NUPATCH						{ExpectParams();}
trimcurve : REQUEST_TOKEN_TRIMCURVE					{ExpectParams();}
sphere : REQUEST_TOKEN_SPHERE						{ExpectParams();}
cone : REQUEST_TOKEN_CONE							{ExpectParams();}
cylinder : REQUEST_TOKEN_CYLINDER					{ExpectParams();}
hyperboloid : REQUEST_TOKEN_HYPERBOLOID				{ExpectParams();}
paraboloid : REQUEST_TOKEN_PARABOLOID				{ExpectParams();}
disk : REQUEST_TOKEN_DISK							{ExpectParams();}
torus : REQUEST_TOKEN_TORUS							{ExpectParams();}
geometry : REQUEST_TOKEN_GEOMETRY					{ExpectParams();}
solidbegin : REQUEST_TOKEN_SOLIDBEGIN				{ExpectParams();}
solidend : REQUEST_TOKEN_SOLIDEND					{ExpectParams();}
objectbegin : REQUEST_TOKEN_OBJECTBEGIN				{ExpectParams();}	
objectend : REQUEST_TOKEN_OBJECTEND					{ExpectParams();}
objectinstance : REQUEST_TOKEN_OBJECTINSTANCE		{ExpectParams();}
motionbegin : REQUEST_TOKEN_MOTIONBEGIN				{ExpectParams();}
motionend : REQUEST_TOKEN_MOTIONEND					{ExpectParams();}	
maketexture : REQUEST_TOKEN_MAKETEXTURE				{ExpectParams();}
makebump : REQUEST_TOKEN_MAKEBUMP					{ExpectParams();}
makelatlongenvironment : REQUEST_TOKEN_MAKELATLONGENVIRONMENT {ExpectParams();}
makecubefaceenvironment : REQUEST_TOKEN_MAKECUBEFACEENVIRONMENT {ExpectParams();}
makeshadow : REQUEST_TOKEN_MAKESHADOW				{ExpectParams();}
errorhandler : REQUEST_TOKEN_ERRORHANDLER			{ExpectParams();}
errorignore : REQUEST_TOKEN_ERRORIGNORE				{ExpectParams();}
errorprint : REQUEST_TOKEN_ERRORPRINT				{ExpectParams();}
errorabort : REQUEST_TOKEN_ERRORABORT				{ExpectParams();}
subdivisionmesh : REQUEST_TOKEN_SUBDIVISIONMESH		{ExpectParams();}

float :		FLOAT_TOKEN
	|		integer				{$$=(TqFloat)$1;}
	;

floats :	FLOAT_TOKEN			{
									$$.aftype=new std::vector<TqFloat>(); 
									$$.type=Type_Float;
									$$.aftype->push_back($1);
								}
	|		floats FLOAT_TOKEN	{
									$1.aftype->push_back($2);
									$$=$1;
								}
	|		floats INTEGER_TOKEN{
									$1.aftype->push_back($2);
									$$=$1;
								}
	|		integers FLOAT_TOKEN{
									std::vector<TqFloat>* pnew=new std::vector<TqFloat>();
									pnew->resize($1.aitype->size());
									TqInt i;
									for(i=0; i<$1.aitype->size(); i++)
										(*pnew)[i]=(*$1.aitype)[i];
									DiscardArrayValue($1);
									$$.aftype=pnew;
									$$.aftype->push_back($2);
									$$.type=Type_Float;
								}
	;

array :		float_array	
	|		integer_array
	|		string_array
	|		'[' ']'				{$$.aftype=new std::vector<TqFloat>();}
	;

float_array : '[' floats ']'	{$$=$2;}
	;

integer :	INTEGER_TOKEN		{$$=$1;}
	;

integers :	INTEGER_TOKEN		{
									$$.aitype=new std::vector<TqInt>(); 
									$$.type=Type_Integer;
									$$.aitype->push_back($1);
								}
	|		integers integer	{
									$1.aitype->push_back($2);
									$$=$1;
								}

integer_array : '[' integers ']' {$$=$2;}
	;

string :	STRING_TOKEN		{$$=$1;}
	;

strings :	STRING_TOKEN		{
									$$.astype=new std::vector<CqString*>(); 
									$$.type=Type_String;
									$$.astype->push_back($1);
								}
	|		strings STRING_TOKEN
								{
									$1.astype->push_back($2);
									$$.astype=$1.astype;
								}

string_array : '[' strings ']' {$$=$2;}
	;

tvpair	:	string INTEGER_TOKEN {
									// Set default value.
									$$.value=0;

									// Find token in the list of declared tokens
									SqParameterDeclaration Decl=QGetRenderContext()->FindParameterDecl($1->c_str());
									if(Decl.m_strName.compare(""))
									{
										if((Decl.m_Type&Type_Mask)==Type_Float)
										{
											TqFloat* pv=new TqFloat[1];
											pv[0]=($2);
											$$.value=reinterpret_cast<char*>(pv);
										}
										else
										{
											TqInt* pv=new TqInt[1];
											pv[0]=($2);
											$$.value=reinterpret_cast<char*>(pv);
										}
										$$.count=1;
										$$.type=0;
										$$.token=new char[$1->size()+1];
										strcpy($$.token,$1->c_str());
									}
									else
									{
										CqString strErr("Unrecognised parameter : ");
										strErr+=*$1;
										DiscardStringValue($1);
										yyerror(strErr.c_str());
										YYERROR;
									}

									DiscardStringValue($1);
								}
	|		string FLOAT_TOKEN	{
									// Find token in the list of declared tokens
									SqParameterDeclaration Decl=QGetRenderContext()->FindParameterDecl($1->c_str());
									if(Decl.m_strName.compare(""))
									{
										if((Decl.m_Type&Type_Mask)==Type_Float)
										{
											TqFloat* pv=new TqFloat[1];
											pv[0]=($2);
											$$.value=reinterpret_cast<char*>(pv);
										}
										else
										{
											CqString strErr("Invalid type for parameter : ");
											strErr+=*$1;
											DiscardStringValue($1);
											yyerror(strErr.c_str());
											YYERROR;
										}
										$$.count=1;
										$$.type=0;
										$$.token=new char[$1->size()+1];
										strcpy($$.token,$1->c_str());
									}
									else
									{
										CqString strErr("Unrecognised parameter : ");
										strErr+=*$1;
										DiscardStringValue($1);
										yyerror(strErr.c_str());
										YYERROR;
									}
									DiscardStringValue($1);
								}
	|		string STRING_TOKEN	{
									// Find token in the list of declared tokens
									SqParameterDeclaration Decl=QGetRenderContext()->FindParameterDecl($1->c_str());
									if(Decl.m_strName.compare(""))
									{
										if((Decl.m_Type&Type_Mask)==Type_String)
										{
											char** ps=new char*[1];
											$$.value=(char*)ps;
											char* pVal=0;
											if((*$2).c_str()!=0)
											{
												pVal=new char[(*$2).size()+1];
												strcpy(pVal,(*$2).c_str());
											}
											ps[0]=pVal;
										}
										else
										{
											CqString strErr("Invalid type for parameter : ");
											strErr+=*$1;
											DiscardStringValue($1);
											yyerror(strErr.c_str());
											YYERROR;
										}
										$$.type=1;
										$$.count=1;
										$$.token=new char[$1->size()+1];
										strcpy($$.token,$1->c_str());
									}
									else
									{
										CqString strErr("Unrecognised parameter : ");
										strErr+=*$1;
										DiscardStringValue($1);
										DiscardStringValue($2);
										yyerror(strErr.c_str());
										YYERROR;
									}
									DiscardStringValue($1);
									DiscardStringValue($2);
								}
	|		string array		{
									$$.value=0;
									if(($$.value=CheckParameterArrayType($1->c_str(),$2,$$.count,$$.type))!=0)
									{
										$$.token=new char[$1->size()+1];
										strcpy($$.token,$1->c_str());
									}
									else
									{
										CqString strErr("Unrecognised parameter : ");
										strErr+=*$1;
										DiscardStringValue($1);
										DiscardArrayValue($2);
										yyerror(strErr.c_str());
										YYERROR;
									}

									DiscardStringValue($1);
									DiscardArrayValue($2);
								}
	;

opttvpairs : 						/* Empty */
								{
									$$.atokens=new std::vector<char*>;
									$$.avalues=new std::vector<char*>;
	 								$$.acounts=new std::vector<int>;
	 								$$.atypes=new std::vector<int>;
								}
	|	error					{
									$$.atokens=new std::vector<char*>;
									$$.avalues=new std::vector<char*>;
									$$.acounts=new std::vector<int>;
	 								$$.atypes=new std::vector<int>;
								}
	|	tvpairs
	;

tvpairs 
	:	tvpair					{
									$$.atokens=new std::vector<char*>;
									$$.avalues=new std::vector<char*>;
									$$.acounts=new std::vector<int>;
	 								$$.atypes=new std::vector<int>;
									$$.atokens->push_back($1.token);
									$$.avalues->push_back($1.value);
									$$.acounts->push_back($1.count);
									$$.atypes->push_back($1.type);
								}
	|	error tvpair			{
									$$.atokens=new std::vector<char*>;
									$$.avalues=new std::vector<char*>;
									$$.acounts=new std::vector<int>;
	 								$$.atypes=new std::vector<int>;
									$$.atokens->push_back($2.token);
									$$.avalues->push_back($2.value);
									$$.acounts->push_back($2.count);
									$$.atypes->push_back($2.type);
								}
	|	tvpairs tvpair			{
									$$=$1;
									$$.atokens->push_back($2.token);
									$$.avalues->push_back($2.value);
									$$.acounts->push_back($2.count);
									$$.atypes->push_back($2.type);
								}
	|	tvpairs error
	;
%%
/* -------------- body section -------------- */

void RIBParser::DiscardStringValue(void* p)
{
	CqString* ps=static_cast<CqString*>(p);
	delete(ps);
}

void RIBParser::DiscardArrayValue(struct yy_RIBParser_stype::array_union& array)
{
	switch(array.type)
	{
		case Type_Integer:
			delete(array.aitype);
			break;

		case Type_Float:
			delete(array.aftype);
			break;

		case Type_String:
			delete(array.astype);
			break;
	}
}


void RIBParser::DiscardTokenValuePair(void* p)
{
	YY_RIBParser_STYPE::SqTV* pp=static_cast<YY_RIBParser_STYPE::SqTV*>(p);
	// If the type is string, then we must treat it as a char** and delete the string as well
	if(pp->type)
	{
		char** ps=(char**)pp->value;
		delete(*ps);
	}
	delete(pp->token);
	delete(pp->value);
}


void RIBParser::DiscardTokenValuePairs(void* p)
{
	YY_RIBParser_STYPE::SqListTV* pp=static_cast<YY_RIBParser_STYPE::SqListTV*>(p);
	TqInt i;
	for(i=0; i<pp->atokens->size(); i++)	delete[]((*pp->atokens)[i]);
	for(i=0; i<pp->avalues->size(); i++)
	{
		// If the type is string, then we must treat it as a char** and delete the strings as well
		if((*pp->atypes)[i])
		{
			char** ps=(char**)(*pp->avalues)[i];
			TqInt j;
			for(j=0; j<(*pp->acounts)[i]; j++)
				delete[](ps[j]);
		}	
		delete[]((*pp->avalues)[i]);
	}
	delete(pp->atokens);
	delete(pp->avalues);
	delete(pp->acounts);
	delete(pp->atypes);
}


char* RIBParser::CheckParameterArrayType(const char* strname, struct yy_RIBParser_stype::array_union& array, int& count, int& type)
{
	char* ret=0;
	
	// Find token in the list of declared tokens
	SqParameterDeclaration Decl=QGetRenderContext()->FindParameterDecl(strname);
	if(Decl.m_strName.compare(""))
	{
		// First check if the types match, and build the array as appropriate
		int dType=Decl.m_Type&Type_Mask;
		if(dType==Type_Integer && array.type==Type_Integer)
		{
			// TODO: Would really like to reuse the array here, instead of reallocating.
			TqInt* pi=new TqInt[array.aitype->size()];
			TqInt i;
			for(i=0; i<array.aitype->size(); i++)
				pi[i]=(*array.aitype)[i];
			count=array.aitype->size();
			type=0;
			ret=reinterpret_cast<char*>(pi);
		}
		else if((dType==Type_Float || 
				 dType==Type_Point || 
				 dType==Type_hPoint || 
				 dType==Type_Vector || 
				 dType==Type_Normal ||
				 dType==Type_Color ||
				 dType==Type_Matrix) && array.type==Type_Float)
		{
			// TODO: Would really like to reuse the array here, instead of reallocating.
			TqFloat* pf=new TqFloat[array.aftype->size()];
			TqInt i;
			for(i=0; i<array.aftype->size(); i++)
				pf[i]=(*array.aftype)[i];
			count=array.aftype->size();
			type=0;
			ret=reinterpret_cast<char*>(pf);
		}
		else if(dType==Type_String && array.type==Type_String)
		{
			char** ps=new char*[array.astype->size()];
			TqInt i;
			for(i=0; i<array.astype->size(); i++)
			{
				char* pVal=0;
				// Check for passing a null string ("") to a string parameter.
				if((*array.astype)[i]->c_str()!=0)
				{
					pVal=new char[(*array.astype)[i]->size()+1];
					strcpy(pVal,(*array.astype)[i]->c_str());
				}
				ps[i]=pVal;
			}
			type=1;
			count=array.astype->size();
			ret=reinterpret_cast<char*>(ps);
		}
		// The only auto cast we handle is integer to float.
		else if((dType==Type_Float || 
				 dType==Type_Point || 
				 dType==Type_hPoint || 
				 dType==Type_Vector || 
				 dType==Type_Normal ||
				 dType==Type_Color ||
				 dType==Type_Matrix) && array.type==Type_Integer)
		{
			// TODO: Would really like to reuse the array here, instead of reallocating.
			TqFloat* pf=new TqFloat[array.aitype->size()];
			TqInt i;
			for(i=0; i<array.aitype->size(); i++)
				pf[i]=(*array.aitype)[i];
			count=array.aitype->size();
			type=0;
			ret=reinterpret_cast<char*>(pf);
		}
	}
	return(ret);
}


bool RIBParser::CheckArrayType(EqVariableType type, struct yy_RIBParser_stype::array_union& array)
{
	bool ret=TqFalse;
	
	// First check if the types match, and build the array as appropriate
	if(type==Type_Integer && array.type==Type_Integer)
		ret=TqTrue;
	else if(type==Type_Float && array.type==Type_Float)
		ret=TqTrue;
	else if(type==Type_String && array.type==Type_String)
		ret=TqTrue;
	// The only auto cast we handle is integer to float.
	else if(type==Type_Float && array.type==Type_Integer)
	{
		std::vector<TqFloat>* pf=new std::vector<TqFloat>();
		pf->resize(array.aitype->size());
		TqInt i;
		for(i=0; i<array.aitype->size(); i++)
			(*pf)[i]=(*array.aitype)[i];
		delete(array.aitype);
		array.aitype=0;
		array.aftype=pf;
		array.type=Type_Float;
		ret=TqTrue;
	}
	return(ret);
}