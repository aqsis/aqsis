#ifndef PIXELSAVE_H
#define PIXELSAVE_H


/* Save to a tiff file the content of RGB (or RGBA) pixels values
 * the name used will be filename
 * the pixels values     raster
 * its width              width
 * its height            length
 * number of component    1, 3 or 4
 * conversion   something like "jpg2tif", "gif2tif" , "pcx2tif"
 *
 * Since this function is used now in jpg2tif, gif2tif, pcx2tif  plugins
 */
extern
#ifndef WIN32
inline
#endif
void save_tiff( char *filename, unsigned char *raster,
	                       int width, int length, int samples, char *conversion );

#endif
