#include <stdio.h>
#include "ri.h"

// Options strings
char *ri2rib_ascii = "Ascii";
char *ri2rib_binary = "Binary";
char *ri2rib_space = "Space";
char *ri2rib_tab = "Tab";
char *ri2rib_none = "None";
char *ri2rib_gzip = "Gzip";

RtFloat newFilter ( RtFloat a, RtFloat b, RtFloat c, RtFloat d )
{
	return 0.5;
}

void context_switching_test( void )
{
	RtContextHandle context[ 3 ];
	RtInt i;

	/* ======== ======== Context switching test ======== ======== */
	RiBegin( "RI2RIB_context_file1.rib" );
	context[ 0 ] = RiGetContext ();

	RiBegin( "RI2RIB_context_file2.rib" );
	context[ 1 ] = RiGetContext ();

	RiBegin( "RI2RIB_context_file3.rib" );
	context[ 2 ] = RiGetContext ();

	/* Write a sphere in 3 different files, but slightly translated in x */
	for ( i = 0; i < 3; i++ )
	{
		RiContext ( context[ i ] );
		RiTranslate ( i * 0.1, 0.0, 0.0 );
		RiSphere ( 10, -1.0, 1.0, 360.0, RI_NULL );
	}

	RiContext( context[ 0 ] );
	RiEnd();

	RiContext( context[ 1 ] );
	RiEnd();

	RiContext( context[ 2 ] );
	RiEnd();
}

void basic_test( void )
{
	RtColor blue = {0.0, 0.0, 1.0};
	RtMatrix matrix = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};
	RtPoint pts[ 4 ] = {{10, 20, 30}, { 7, 8, 9 }, {40, 50, 60}, {0.1, 0.1, 1}};
	RtFloat val[ 2 ] = {0.9999, 0.0001};

	RiBegin( "RI2RIB_basic_test.rib" );
	RiDeclare( "var", "float[2]" );
	RiDeclare( "matrix1", "matrix" );
	RiDeclare( "blue", "color" );

	RiSphere( 10, -1, 1, 360,
	          RI_P, ( RtPointer ) pts,
	          ( RtToken ) "var", ( RtPointer ) & val,
	          RI_NULL );

	/* Here is a test to see if libri2rib can discard not-declared tokens */
	RiSphere( 11, -1, 1, 360,
	          ( RtToken ) "blue", ( RtPointer ) & blue,
	          ( RtToken ) "high", ( RtPointer ) & val,
	          ( RtToken ) "matrix1", ( RtPointer ) & matrix,
	          RI_NULL );

	RiPixelFilter( RiSincFilter, 1, 1 );
	RiPixelFilter( newFilter, 1, 1 );
	RiPixelFilter( RiBoxFilter, 2, 2 );

	RiArchiveRecord( RI_VERBATIM, "\n" );
	RiArchiveRecord( RI_COMMENT, "RiArchiveRecord %s", "test" );
	RiArchiveRecord( RI_STRUCTURE, "Frames 10" );
	RiArchiveRecord( RI_VERBATIM, "  # # # # # \n" );
	RiEnd();
}

void indent_test( void )
{
	RtMatrix matrix = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};
	RtPoint pts[ 4 ] = {{10, 20, 30}, { 7, 8, 9 }, {40, 50, 60}, {0.1, 0.1, 1}};
	RtContextHandle context[ 3 ];
	RtInt i;

	RtInt eight = 8;
	RtInt one = 1;

	/* Let's get the 'Void' context handle */
	context[ 0 ] = RiGetContext();

	RiOption ( "RI2RIB_Indentation",
	           ( RtToken ) "Type", ( RtPointer ) & ri2rib_space,
	           ( RtToken ) "Size", ( RtPointer ) & eight, RI_NULL );
	RiBegin( "RI2RIB_indent_Space.rib" );
	context[ 1 ] = RiGetContext();

	RiContext( context[ 0 ] );
	RiOption ( "RI2RIB_Indentation",
	           ( RtToken ) "Type", ( RtPointer ) & ri2rib_tab,
	           ( RtToken ) "Size", ( RtPointer ) & one, RI_NULL );
	RiBegin( "RI2RIB_indent_Tab.rib" );
	context[ 2 ] = RiGetContext();

	for ( i = 1; i < 3; i++ )
	{
		RiContext( context[ i ] );

		RiFrameBegin( 1 );
		RiTransformBegin();
		RiConcatTransform( matrix );
		RiTransformEnd();

		RiWorldBegin();

		RiTransformBegin();
		RiConcatTransform( matrix );
		RiTransformBegin();
		RiConcatTransform( matrix );
		RiTransformBegin();
		RiConcatTransform( matrix );
		RiTransformEnd();
		RiTransformEnd();
		RiTransformEnd();

		RiObjectBegin();
		RiSphere( 10, -1, 1, 360, RI_P, ( RtPointer ) pts, RI_NULL );
		RiObjectEnd();

		RiWorldEnd();
		RiFrameEnd();
		RiEnd();
	}

	RiOption ( "RI2RIB_Indentation",
	           ( RtToken ) "Type", ( RtPointer ) & ri2rib_none, RI_NULL );
}

void colorsamples_test()
{
	RtFloat mat1[] = { 1, 2, 3 };
	RtFloat mat3[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
	RtFloat mat4[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

	RtFloat color1[] = { .1, .2, .3, .4 };
	RtFloat color3[] = { .1, .2, .3, .4, .5, .6, .7, .8, .9, .10, .11, .12 };
	RtFloat color4[] = { .1, .2, .3, .4, .5, .6, .7, .8, .9, .10, .11, .12, .13, .14, .15, .16 };

	RiBegin( "RI2RIB_ColorSamples.rib" );

	RiArchiveRecord( RI_VERBATIM, "\n" );
	RiArchiveRecord( RI_COMMENT, "  1 color sample" );
	RiColorSamples( 1, mat1, mat1 );
	RiWorldBegin();
	RiSphere( 10, -1, 1, 360, RI_CS, ( RtPointer ) color1, RI_NULL );
	RiWorldEnd();

	RiArchiveRecord( RI_VERBATIM, "\n" );
	RiArchiveRecord( RI_COMMENT, "  3 color samples" );
	RiColorSamples( 3, mat3, mat3 );
	RiWorldBegin();
	RiSphere( 10, -1, 1, 360, RI_CS, ( RtPointer ) color3, RI_NULL );
	RiWorldEnd();

	RiArchiveRecord( RI_VERBATIM, "\n" );
	RiArchiveRecord( RI_COMMENT, "  4 color samples" );
	RiColorSamples( 4, mat4, mat4 );
	RiWorldBegin();
	RiSphere( 10, -1, 1, 360, RI_CS, ( RtPointer ) color4, RI_NULL );
	RiWorldEnd();

	RiEnd();
}

void torus_test()
{
	RtInt one = 1;
	RtFloat flt1 = 36.0;
	RtFloat flt2 = 150000;
	RtFloat flt3 = 100000;

	RtMatrix mtrx1 = {{1.0, 0.0, 0.0, 0},
	                  {0.0, 0.435544, -0.900167, 0},
	                  {0.0, 0.900167, 0.435544, 0},
	                  {0.955721, 9.27594, -440.36, 1}};

	RtMatrix mtrx2 = {{1.0, 0.0, 0.0, 0},
	                  {0.0, 1.0, 0.0, 0},
	                  {0.0, 0.0, 1.0, 0},
	                  {204.187, -437.336, 53.0255, 1}};

	RtMatrix mtrx3 = {{1.0, 0.0, 0.0, 0},
	                  {0.0, 1.0, 0.0, 0},
	                  {0.0, 0.0, 1.0, 0},
	                  { -320.159, -429.109, -87.5622, 1}};

	RtMatrix mtrx4 = {{1.0, 0.0, 0.0, 0},
	                  {0.0, 1.0, 0.0, 0},
	                  {0.0, 0.0, 1.0, 0},
	                  {121.927, 4.77862, 516.121, 1}};

	RtMatrix mtrx5 = {{1.0, 0.0, 0.0, 0},
	                  {0.0, 1.0, 0.0, 0},
	                  {0.0, 0.0, 1.0, 0},
	                  {0.0, 0.0, 0.0, 1}};

	char *path = "../shaders:&";
	char *deflate = "deflate";
	char *off = "off";

	RtColor clr1 = {0.823529, 0.4313726, 0.933333};
	RtColor clr2 = {0.5, 0.5, 0.5};


	RtContextHandle context[ 5 ];
	RtInt i;

	context[ 0 ] = RiGetContext();
	RiBegin ( "RI2RIB_torus.rib" );
	context[ 1 ] = RiGetContext();

	RiContext( context[ 0 ] );
	RiOption ( "RI2RIB_Output",
	           ( RtToken ) "Compression", ( RtPointer ) & ri2rib_gzip, RI_NULL );
	RiBegin( "RI2RIB_torus.rib.gz" );
	context[ 2 ] = RiGetContext();

	RiContext( context[ 0 ] );
	RiOption ( "RI2RIB_Output",
	           ( RtToken ) "Compression", ( RtPointer ) & ri2rib_none, RI_NULL );
	RiOption ( "RI2RIB_Output",
	           ( RtToken ) "Type", ( RtPointer ) & ri2rib_binary, RI_NULL );
	RiBegin ( "RI2RIB_torus_bin.rib" );
	context[ 3 ] = RiGetContext();

	RiContext( context[ 0 ] );
	RiOption ( "RI2RIB_Output",
	           ( RtToken ) "Compression", ( RtPointer ) & ri2rib_gzip, RI_NULL );
	RiBegin( "RI2RIB_torus_bin.rib.gz" );
	context[ 4 ] = RiGetContext();

	for ( i = 1; i < 5; i++ )
	{
		RiContext( context[ i ] );
		RiOrientation ( "lh" );
		RiFormat ( 320, 240, 1 );
		RiPixelSamples ( 2, 2 );
		RiShadingRate ( 1.0 );
		RiQuantize ( "rgba", 255, 0, 255, 0.5 );
		RiOption ( "statistics", ( RtToken ) "endofframe", ( RtPointer ) & one, RI_NULL );
		RiOption ( "searchpath", ( RtToken ) "shader", ( RtPointer ) & path, RI_NULL );

		RiFrameBegin ( 0 );
		RiDisplay ( "torus.tif", "file", "rgba", ( RtToken ) "compression", ( RtPointer ) & deflate, RI_NULL );
		RiDisplay ( "+torus.tif", "framebuffer", "rgba", RI_NULL );
		RiProjection ( "perspective", ( RtToken ) "fov", ( RtPointer ) & flt1, RI_NULL );
		RiRotate ( 180, 0, 1, 0 );
		RiConcatTransform ( mtrx1 );

		RiWorldBegin();

		RiSurface ( "matte", RI_NULL );

		RiTransformBegin();
		RiConcatTransform ( mtrx2 );
		RiAttribute ( "light", ( RtToken ) "shadows", ( RtPointer ) & off, RI_NULL );
		RiLightSource ( "pointlight", ( RtToken ) "intensity", ( RtPointer ) & flt2, RI_NULL );
		RiTransformEnd();

		RiTransformBegin();
		RiConcatTransform ( mtrx3 );
		RiAttribute ( "light", ( RtToken ) "shadows", ( RtPointer ) & off, RI_NULL );
		RiLightSource ( "pointlight", ( RtToken ) "intensity", ( RtPointer ) & flt3, RI_NULL );
		RiTransformEnd();

		RiTransformBegin();
		RiConcatTransform( mtrx4 );
		RiAttribute ( "light", ( RtToken ) "shadows", ( RtPointer ) & off, RI_NULL );
		RiLightSource ( "pointlight", ( RtToken ) "intensity", ( RtPointer ) & flt3, RI_NULL );
		RiTransformEnd();

		RiAttributeBegin();
		RiColor ( clr1 );
		RiOpacity ( clr2 );
		RiConcatTransform ( mtrx5 );
		RiTorus ( 85.0, 15.0, 0, 360, 360, RI_NULL );
		RiAttributeEnd();

		RiWorldEnd();
		RiFrameEnd();
		RiEnd();
	}

	RiOption ( "RI2RIB_Output",
	           ( RtToken ) "Type", ( RtPointer ) & ri2rib_ascii, RI_NULL );
	RiOption ( "RI2RIB_Output",
	           ( RtToken ) "Compression", ( RtPointer ) & ri2rib_none, RI_NULL );
}

int main()
{
	printf( "Context switching test\n" );
	context_switching_test();
	printf( "\nBasic test\n" );
	basic_test();
	printf( "\nIndent test\n" );
	indent_test();
	printf( "\nColorSamples test\n" );
	colorsamples_test();
	printf( "\nBinary output test\n" );
	torus_test();
	return 0;
}

