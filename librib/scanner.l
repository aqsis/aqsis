/* -------------- declaration section -------------- */
%name RIBScanner

%define	IOSTREAM
%define LEX_PARAM YY_RIBParser_STYPE *val,YY_RIBParser_LTYPE *loc
%define MEMBERS public: int m_Line; long m_Len;\
						TqBool m_fRequest, m_fParams; \
						std::stack<YY_BUFFER_STATE> m_BufferStack; \
						std::stack<std::istream>	m_FileStack; \
						std::stack<std::string>		m_FileNameStack; \
						void	SetFile(std::istream& file, const char* name)	\
								{ \
									yy_initialize(); \
									m_FileStack.push(file); \
									m_FileNameStack.push(std::string(name)); \
									m_Len=-1; \
									yyin=&file; \
									m_Line=1; \
									m_fRequest=TqTrue; \
								} \
						void	ExpectRequest()				{m_fRequest=TqTrue;} \
						void	ExpectParams()				{m_fParams=TqTrue;} 
%define CONSTRUCTOR_INIT : m_Line(1),m_Len(-1)
%define	CONSTRUCTOR_PARAM	
%define	CONSTRUCTOR_CODE	
%define INPUT_CODE	/*m_Len=m_FileStack.top().Length();*/ \
					/*float pos=(100.0f/m_Len)*m_FileStack.top().Position(); */\
					/*QGetRenderContext()->Stats().SetComplete(pos); */\
					\
					if(YY_RIBScanner_IN->eof())  \
						result=0; \
					else \
					{ \
						YY_RIBScanner_IN->read(buffer,max_size); \
						result=YY_RIBScanner_IN->gcount(); \
						YY_RIBScanner_IN->clear(YY_RIBScanner_IN->rdstate()&(~std::ios::failbit)); \
						if(YY_RIBScanner_IN->bad()) \
							result= -1; \
					} \
					return result;
%define DEBUG 0
%header{ 
#include	<stack>

using namespace Aqsis;
%}

%s params
%x incl

/* -------------- rules section -------------- */

delim		[ \t]
whitespace	{delim}+
letter		[A-Za-z]
digit		[0-9]
float		[+\-]?{digit}*(\.{digit}+)?([eE][+\-]?{digit}+)?
integer		[+\-]?{digit}+
string		\"[^"\n]*\"
LF			(\r\n)|(\r)|(\n)|(\x0c)
comment		#.*{LF}

%%
	if(m_fRequest)
	{
		BEGIN(INITIAL);
		m_fRequest=TqFalse;
	}

	if(m_fParams)
	{
		BEGIN(params);
		m_fParams=TqFalse;
	}

{comment}				{
							// Ignore comments
							m_Line++;
						}
{LF}					{
							m_Line++;
						}

{whitespace}			{/* no action and no return */}
<params>{integer}		{val->itype=atoi((char*)yytext); return(RIBParser::INTEGER_TOKEN); }
<params>{float}			{val->ftype=(TqFloat)atof((char*)yytext); return(RIBParser::FLOAT_TOKEN); }
<params>{string}		{
							TqInt l=strcspn((char*)yytext+1, "\"");
							CqString strText((char*)yytext);
							val->stype=new CqString(strText.substr(1,l));
							return(RIBParser::STRING_TOKEN);
						}
<INITIAL>{integer}		{}
<INITIAL>{float}		{}
<INITIAL>{string}		{}
version					{return(RIBParser::REQUEST_TOKEN_VERSION);}
Declare					{return(RIBParser::REQUEST_TOKEN_DECLARE);}
FrameBegin				{return(RIBParser::REQUEST_TOKEN_FRAMEBEGIN);}
FrameEnd				{return(RIBParser::REQUEST_TOKEN_FRAMEEND);}
WorldBegin				{return(RIBParser::REQUEST_TOKEN_WORLDBEGIN);}
WorldEnd				{return(RIBParser::REQUEST_TOKEN_WORLDEND);}
Format					{return(RIBParser::REQUEST_TOKEN_FORMAT);}
FrameAspectRatio		{return(RIBParser::REQUEST_TOKEN_FRAMEASPECTRATIO);}
ScreenWindow			{return(RIBParser::REQUEST_TOKEN_SCREENWINDOW);}
CropWindow				{return(RIBParser::REQUEST_TOKEN_CROPWINDOW);}
Projection				{return(RIBParser::REQUEST_TOKEN_PROJECTION);}
Clipping				{return(RIBParser::REQUEST_TOKEN_CLIPPING);}
DepthOfField			{return(RIBParser::REQUEST_TOKEN_DEPTHOFFIELD);}
Shutter					{return(RIBParser::REQUEST_TOKEN_SHUTTER);}
PixelVariance			{return(RIBParser::REQUEST_TOKEN_PIXELVARIANCE);}
PixelSamples			{return(RIBParser::REQUEST_TOKEN_PIXELSAMPLES);}
PixelFilter				{return(RIBParser::REQUEST_TOKEN_PIXELFILTER);}
Exposure				{return(RIBParser::REQUEST_TOKEN_EXPOSURE);}
Imager					{return(RIBParser::REQUEST_TOKEN_IMAGER);}
Quantize				{return(RIBParser::REQUEST_TOKEN_QUANTIZE);}
Display					{return(RIBParser::REQUEST_TOKEN_DISPLAY);}
GaussianFilter			{return(RIBParser::REQUEST_TOKEN_GAUSSIANFILTER);}
BoxFilter				{return(RIBParser::REQUEST_TOKEN_BOXFILTER);}
TriangleFilter			{return(RIBParser::REQUEST_TOKEN_TRIANGLEFILTER);}
CatmullRomFilter		{return(RIBParser::REQUEST_TOKEN_CATMULLROMFILTER);}
SincFilter				{return(RIBParser::REQUEST_TOKEN_SINCFILTER);}
Hider					{return(RIBParser::REQUEST_TOKEN_HIDER);}
ColorSamples			{return(RIBParser::REQUEST_TOKEN_COLORSAMPLES);}
RelativeDetail			{return(RIBParser::REQUEST_TOKEN_RELATIVEDETAIL);}
Option					{return(RIBParser::REQUEST_TOKEN_OPTION);}
AttributeBegin			{return(RIBParser::REQUEST_TOKEN_ATTRIBUTEBEGIN);}
AttributeEnd			{return(RIBParser::REQUEST_TOKEN_ATTRIBUTEEND);}
Color					{return(RIBParser::REQUEST_TOKEN_COLOR);}
Opacity					{return(RIBParser::REQUEST_TOKEN_OPACITY);}
TextureCoordinates		{return(RIBParser::REQUEST_TOKEN_TEXTURECOORDINATES);}
LightSource				{return(RIBParser::REQUEST_TOKEN_LIGHTSOURCE);}
AreaLightSource			{return(RIBParser::REQUEST_TOKEN_AREALIGHTSOURCE);}
Illuminate				{return(RIBParser::REQUEST_TOKEN_ILLUMINATE);}
Surface					{return(RIBParser::REQUEST_TOKEN_SURFACE);}
Atmosphere				{return(RIBParser::REQUEST_TOKEN_ATMOSPHERE);}
Interior				{return(RIBParser::REQUEST_TOKEN_INTERIOR);}
Exterior				{return(RIBParser::REQUEST_TOKEN_EXTERIOR);}
ShadingRate				{return(RIBParser::REQUEST_TOKEN_SHADINGRATE);}
ShadingInterpolation	{return(RIBParser::REQUEST_TOKEN_SHADINGINTERPOLATION);}
Matte					{return(RIBParser::REQUEST_TOKEN_MATTE);}
Bound					{return(RIBParser::REQUEST_TOKEN_BOUND);}
Detail					{return(RIBParser::REQUEST_TOKEN_DETAIL);}
DetailRange				{return(RIBParser::REQUEST_TOKEN_DETAILRANGE);}
GeometricApproximation	{return(RIBParser::REQUEST_TOKEN_GEOMETRICAPPROXIMATION);}
Orientation				{return(RIBParser::REQUEST_TOKEN_ORIENTATION);}
ReverseOrientation		{return(RIBParser::REQUEST_TOKEN_REVERSEORIENTATION);}
Sides					{return(RIBParser::REQUEST_TOKEN_SIDES);}
Identity				{return(RIBParser::REQUEST_TOKEN_IDENTITY);}
Transform				{return(RIBParser::REQUEST_TOKEN_TRANSFORM);}
ConcatTransform			{return(RIBParser::REQUEST_TOKEN_CONCATTRANSFORM);}
Perspective				{return(RIBParser::REQUEST_TOKEN_PERSPECTIVE);}
Translate				{return(RIBParser::REQUEST_TOKEN_TRANSLATE);}
Rotate					{return(RIBParser::REQUEST_TOKEN_ROTATE);}
Scale					{return(RIBParser::REQUEST_TOKEN_SCALE);}
Skew					{return(RIBParser::REQUEST_TOKEN_SKEW);}
Deformation				{return(RIBParser::REQUEST_TOKEN_DEFORMATION);}
Displacement			{return(RIBParser::REQUEST_TOKEN_DISPLACEMENT);}
CoordinateSystem		{return(RIBParser::REQUEST_TOKEN_COORDINATESYSTEM);}
CoordSysTransform		{return(RIBParser::REQUEST_TOKEN_COORDSYSTRANSFORM);}
TransformPoints			{return(RIBParser::REQUEST_TOKEN_TRANSFORMPOINTS);}
TransformBegin			{return(RIBParser::REQUEST_TOKEN_TRANSFORMBEGIN);}
TransformEnd			{return(RIBParser::REQUEST_TOKEN_TRANSFORMEND);}
Attribute				{return(RIBParser::REQUEST_TOKEN_ATTRIBUTE);}
Polygon					{return(RIBParser::REQUEST_TOKEN_POLYGON);}
GeneralPolygon			{return(RIBParser::REQUEST_TOKEN_GENERALPOLYGON);}
PointsPolygons			{return(RIBParser::REQUEST_TOKEN_POINTSPOLYGONS);}
PointsGeneralPolygons	{return(RIBParser::REQUEST_TOKEN_POINTSGENERALPOLYGONS);}
Basis					{return(RIBParser::REQUEST_TOKEN_BASIS);}
Patch					{return(RIBParser::REQUEST_TOKEN_PATCH);}
PatchMesh				{return(RIBParser::REQUEST_TOKEN_PATCHMESH);}
NuPatch					{return(RIBParser::REQUEST_TOKEN_NUPATCH);}
TrimCurve				{return(RIBParser::REQUEST_TOKEN_TRIMCURVE);}
Sphere					{return(RIBParser::REQUEST_TOKEN_SPHERE);}
Cone					{return(RIBParser::REQUEST_TOKEN_CONE);}
Cylinder				{return(RIBParser::REQUEST_TOKEN_CYLINDER);}
Hyperboloid				{return(RIBParser::REQUEST_TOKEN_HYPERBOLOID);}
Paraboloid				{return(RIBParser::REQUEST_TOKEN_PARABOLOID);}
Disk					{return(RIBParser::REQUEST_TOKEN_DISK);}
Torus					{return(RIBParser::REQUEST_TOKEN_TORUS);}
Procedural				{return(RIBParser::REQUEST_TOKEN_PROCEDURAL);}
Geometry				{return(RIBParser::REQUEST_TOKEN_GEOMETRY);}
SolidBegin				{return(RIBParser::REQUEST_TOKEN_SOLIDBEGIN);}
SolidEnd				{return(RIBParser::REQUEST_TOKEN_SOLIDEND);}
ObjectBegin				{return(RIBParser::REQUEST_TOKEN_OBJECTBEGIN);}
ObjectEnd				{return(RIBParser::REQUEST_TOKEN_OBJECTEND);}
ObjectInstance			{return(RIBParser::REQUEST_TOKEN_OBJECTINSTANCE);}
MotionBegin				{return(RIBParser::REQUEST_TOKEN_MOTIONBEGIN);}
MotionEnd				{return(RIBParser::REQUEST_TOKEN_MOTIONEND);}
MakeTexture				{return(RIBParser::REQUEST_TOKEN_MAKETEXTURE);}
MakeBump				{return(RIBParser::REQUEST_TOKEN_MAKEBUMP);}
MakeLatLongEnvironment	{return(RIBParser::REQUEST_TOKEN_MAKELATLONGENVIRONMENT);}
MakeCubeFaceEnvironment {return(RIBParser::REQUEST_TOKEN_MAKECUBEFACEENVIRONMENT);}
MakeShadow				{return(RIBParser::REQUEST_TOKEN_MAKESHADOW);}
ErrorHandler			{return(RIBParser::REQUEST_TOKEN_ERRORHANDLER);}
SubdivisionMesh			{return(RIBParser::REQUEST_TOKEN_SUBDIVISIONMESH);}

ReadArchive				{BEGIN(incl);}
{letter}({letter}|{digit})*	{return(RIBParser::UNKNOWN_TOKEN);}
.						{return(yytext[0]);}
<incl>[ \t]*			
<incl>{string}			{
							TqInt l=strcspn((char*)yytext+1, "\"");
							CqString strFilename((char*)yytext);
							CqFile* pnewFile=new CqFile(strFilename.substr(1,l).c_str(),"archive");
							if(pnewFile->IsValid())
							{
								m_BufferStack.push(YY_CURRENT_BUFFER);
								m_FileStack.push(*pnewFile);			
								m_FileNameStack.push(pnewFile->strRealName());
								yyin=*pnewFile;
								yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
							}
							else
							{
								CqString strErr("Cannot find archive file ");
								strErr+=strFilename;
								CqBasicError(0,Severity_Normal,strErr.c_str());
							}

							BEGIN(INITIAL);
						}
<<EOF>>					{
							if(m_BufferStack.empty())
							{
								yyterminate();
								return(RIBParser::EOF_TOKEN);
							}
							else
							{
								m_FileStack.pop();
								m_FileNameStack.pop();
								yy_switch_to_buffer(m_BufferStack.top());
								m_BufferStack.pop();
							}
						}
%%
