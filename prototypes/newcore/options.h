#ifndef OPTIONS_H_INCLUDED
#define OPTIONS_H_INCLUDED

struct Options
{
	int maxSplits;   ///< maximum number of splits before discarding a surface
	int xRes;        ///< image x-resolution
	int yRes;        ///< image y-resolution
	//Imath::V2i nsumSamples; ///< number of subsamples
    int gridSize;    ///< Desired grid side length.
    int shadingRate; ///< Desired micropoly area
    float clipNear;  ///< Near clipping plane (cam coords)
    float clipFar;   ///< Far clipping plane (cam coords)
};

#endif // OPTIONS_H_INCLUDED
