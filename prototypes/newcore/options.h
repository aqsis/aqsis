#ifndef OPTIONS_H_INCLUDED
#define OPTIONS_H_INCLUDED

#include <cfloat>

struct Options
{
    int maxSplits;   ///< maximum number of splits before discarding a surface
    int xRes;        ///< image x-resolution
    int yRes;        ///< image y-resolution
    //Imath::V2i nsumSamples; ///< number of subsamples
    int gridSize;    ///< Desired grid side length.
    float shadingRate; ///< Desired micropoly area
    float clipNear;  ///< Near clipping plane (cam coords)
    float clipFar;   ///< Far clipping plane (cam coords)
    bool smoothShading; ///< Type of shading interpolation

    Options()
        : maxSplits(20),
        xRes(640),
        yRes(480),
        gridSize(16),
        shadingRate(1),
        clipNear(FLT_EPSILON),
        clipFar(FLT_MAX),
        smoothShading(true)
    { }
};

#endif // OPTIONS_H_INCLUDED
