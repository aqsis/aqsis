#include <stdio.h>
#include "ri.h"

RtFloat newFilter ( RtFloat a, RtFloat b, RtFloat c, RtFloat d )
{
	return 0.5;
}

int main()
{
	RtColor blue = {0.0, 0.0, 1.0};
	RtMatrix matrix = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};
	RtPoint pts[ 4 ] = {{10, 20, 30}, { 7, 8, 9 }, {40, 50, 60}, {0.1, 0.1, 1}};
	RtFloat val[ 2 ] = {0.9999, 0.0001};

	RtContextHandle context[ 3 ];
	RtInt i;


	/* ======== ======== Context switching test ======== ======== */
	RiBegin( "context1.rib" );
	context[ 0 ] = RiGetContext ();

	RiBegin( "context2.rib" );
	context[ 1 ] = RiGetContext ();

	RiBegin( "context3.rib" );
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


	RiBegin( "test.rib" );
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
	          ( RtToken ) "hello", ( RtPointer ) & val,
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

	return 0;
}

