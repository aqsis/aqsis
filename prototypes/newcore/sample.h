#ifndef SAMPLE_H_INCLUDED
#define SAMPLE_H_INCLUDED

#include <cfloat>

#include "util.h"

struct Sample
{
    Vec2 p;    //< Position of sample in image plane
    float z; //< Things behind this depth are occluded

    Sample(const Vec2& p) : p(p), z(FLT_MAX) {}
};

#endif // SAMPLE_H_INCLUDED
