#include <stdlib.h>
#include <string.h>

#include <aqsis/config.h>

#ifndef AQSIS_SYSTEM_WIN32
#include <unistd.h>
#else
#include <direct.h>
#endif

#ifdef AQSIS_SYSTEM_MACOSX
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif //!AQSIS_SYSTEM_MACOSX

#include "tiffio.h"

#include "sgigt.h"

#ifndef TRUE
#define	TRUE	1
#define	FALSE	0
#endif


static int ix = 0;
static TIFF* tif;
static char sgigt_newname[ 1024 ];
static char* stuff[] = {
                           "usage: aqiv [options] file.tif",
                           "where options are:",
                           " -r,g,b,a		display R,G,B,A pixel",
                           " -d dirnum	set initial directory (default is 0)",
                           " -e		enable display of TIFF error messages",
                           " -s  		stop decoding on first error (default is ignore errors)",
                           " -v		enable verbose mode",
                           " -w		enable display of TIFF warning messages",
                           NULL
                       };

GLubyte* g_Image = NULL;		/* displayable image */





/* From getopt() */
extern	char* optarg;
extern	int optind;
#ifdef AQSIS_SYSTEM_WIN32
extern int getopt( int nargc, const char **nargv, const char *ostr );
#endif

/* From iv.c */
extern int argc;
extern char **argv;
extern char iv_direct[ 1024 ];
extern short iv_which;                      /* flag R, G, B, or A display */
extern	uint32 g_ImageWidth, g_ImageHeight; /* window width & height */

char *filename = "iv";

extern void display();

static TIFFRGBAImage img;
extern TIFFErrorHandler oerror;
extern TIFFErrorHandler owarning;


static void usage();

static int verbose = 0;
static int stoponerr = 0;			/* stop on read error */

/*
 * switch the next file to load
 */
void sgigt_new_file( int idx )
{
	ix = idx;

	TIFFRGBAImageEnd( &img );
	if ( tif != NULL && argv[ ix ] != filename )
		TIFFClose( tif ), tif = NULL;
	if ( argv[ ix ] == NULL )
		return ;
#ifdef AQSIS_SYSTEM_WIN32
	if ( strstr( argv[ ix ], "\\" ) == NULL )
		sprintf( sgigt_newname, "%s\\%s", iv_direct, argv[ ix ] );
	else
		strcpy( sgigt_newname, argv[ ix ] );
#else
	if ( strstr( argv[ ix ], "/" ) == NULL )
		sprintf( sgigt_newname, "%s/%s", iv_direct, argv[ ix ] );
	else
		strcpy( sgigt_newname, argv[ ix ] );
#endif
	filename = sgigt_newname;

	if ( tif == NULL )
	{
		tif = TIFFOpen( filename, "r" );
		if ( tif == NULL )
		{
			TIFFError( filename, "Cannot open the file?" );
			return ;
		} else 
			TIFFPrintDirectory(tif, stdout, 0);
	}
	sgigt_new_page();

}

/*
 * switch to the next page with a tiff (mipmapped)
 */
void sgigt_new_page()
{
	uint32 w, h;
	TIFFRGBAImageEnd( &img );

	if ( !TIFFRGBAImageBegin( &img, tif, stoponerr, filename ) )
	{
		TIFFError( filename, filename );
		return ;
	}

	/*
	 * Setup the image g_Image as required.
	 */
	w = img.width;
	h = img.height;

	if ( g_Image != NULL )
		_TIFFfree( g_Image ), g_Image = NULL;
	g_Image = ( GLubyte* ) _TIFFmalloc( w * h * 4 );
	if ( g_Image == NULL )
	{
		g_ImageWidth = g_ImageHeight = 0;
		TIFFError( filename, "No space for g_Image buffer" );
		return ;
	}
	else
	{
		g_ImageWidth = w;
		g_ImageHeight = h;
		TIFFRGBAImageGet( &img, ( uint32 * ) g_Image, g_ImageWidth, g_ImageHeight );
	}


}


/*
 * read all the files until one could be open as valid TIF file
 */
int
sgigt_read_images( int nargc, char *nargv[] )
{
	uint32 w, h;
	int c;
	int dirnum = -1;

	uint32 diroff = 0;


	argc = nargc;
	argv = nargv;

	oerror = TIFFSetErrorHandler( NULL );
	owarning = TIFFSetWarningHandler( NULL );

	while ( ( c = getopt( nargc, nargv, "d:o:p:ergbawvh" ) ) != -1 )
	{
		switch ( c )
		{
				case 'd':
				dirnum = atoi( optarg );
				break;
				case 'e':
				oerror = TIFFSetErrorHandler( oerror );
				break;
				case 'o':
				diroff = strtoul( optarg, NULL, 0 );
				break;
				case 'r':
				iv_which = RED;
				break;
				case 'g':
				iv_which = GREEN;
				break;
				case 'b':
				iv_which = BLUE;
				break;
				case 'a':
				iv_which = ALPHA;
				break;
				case 'w':
				owarning = TIFFSetWarningHandler( owarning );
				break;
				case 'v':
				verbose = 1;
				break;
				case 'h':
				usage();
				break;
				/*NOTREACHED*/
		}
	}
	if ( argc - optind < 1 )
		usage();

	ix = optind;
	filename = argv[ ix ];

	strcpy( sgigt_newname, filename );

#ifdef AQSIS_SYSTEM_WIN32
	if ( strstr( argv[ ix ], "\\" ) == NULL )
	{
		sprintf( sgigt_newname, "%s\\%s", iv_direct, filename );
	}
#else
	if ( strstr( argv[ ix ], "/" ) == NULL )
	{
		sprintf( sgigt_newname, "%s/%s", iv_direct, filename );
	}
#endif

	filename = sgigt_newname;

	do
	{
		tif = TIFFOpen( filename, "r" );
	}
	while ( tif == NULL && ( ix = sgigt_next_image( argv, ix, optind, argc, FALSE ) ) );
	if ( tif == NULL )
	{
		TIFFError( filename, filename );
		exit( 0 );
	}
	if ( ix == optind )
	{
		/*
		 * Set initial directory if user-specified
		 * file was opened successfully.
		 */
		if ( dirnum != -1 && !TIFFSetDirectory( tif, dirnum ) )
			TIFFError( argv[ ix ], "Error, seeking to directory %d", dirnum );
		if ( diroff != 0 && !TIFFSetSubDirectory( tif, diroff ) )
			TIFFError( argv[ ix ], "Error, setting subdirectory at %#x", diroff );
	}
	sgigt_new_file( ix );


	if ( !TIFFRGBAImageBegin( &img, tif, stoponerr, filename ) )
	{
		TIFFError( filename, filename );
		goto bad2;
	}

	/*
	 * Setup the image g_Image as required.
	 */
	w = img.width;
	h = img.height;
	if ( w != g_ImageWidth || h != g_ImageHeight )
	{
		if ( g_Image != NULL )
			_TIFFfree( g_Image ), g_Image = NULL;
		g_Image = ( GLubyte* ) _TIFFmalloc( w * h * 4 );
		if ( g_Image == NULL )
		{
			g_ImageWidth = g_ImageHeight = 0;
			TIFFError( filename, "No space for g_Image buffer" );
			goto bad3;
		}
		g_ImageWidth = w;
		g_ImageHeight = h;
	}


	( void ) TIFFRGBAImageGet( &img, ( uint32 * ) g_Image, g_ImageWidth, g_ImageHeight );

	sgigt_new_file( ix );
	return 0;
bad3:
	TIFFRGBAImageEnd( &img );
bad2:
	TIFFClose( tif ), tif = NULL;
	exit( 1 );
}


/*
 * switch to the previous filename
 */
int sgigt_prev_image( char* argv[], int ix, int b, int e, int wrap )
{
	int i;

	for ( i = ix - 1; i >= b && argv[ i ] == NULL; i-- )
		;
	if ( i < b )
	{
		if ( wrap )
		{
			for ( i = e - 1; i > ix && argv[ i ] == NULL; i-- )
				;
		}
		else
			i = 0;
	}
	return ( i );
}

/*
 * switch the next filename
 */
int sgigt_next_image( char* argv[], int ix, int b, int e, int wrap )
{
	int i;

	for ( i = ix + 1; i < e && argv[ i ] == NULL; i++ )
		;
	if ( i >= e )
	{
		if ( wrap )
		{
			for ( i = b; i < ix && argv[ i ] == NULL; i++ )
				;
		}
		else
			i = 0;
	}
	return ( i );
}

/*
 * return the current fullpathname. 
 */
char* sgigt_get_filename()
{
	return filename;
}

/*
 * return the current index (with argc)
 */
int sgigt_get_ix()
{
	return ix;
}

/*
 * return the static variable TIFF for iv.c
 */
TIFF *sgigt_get_tif()
{
	return tif;
}


/*
 * Print a simple usage with some printf
 */
static void usage()
{
	char buf[ BUFSIZ ];
	int i;

	setbuf( stderr, buf );
	for ( i = 0; stuff[ i ] != NULL; i++ )
		fprintf( stderr, "%s\n", stuff[ i ] );
	exit( -1 );
}

