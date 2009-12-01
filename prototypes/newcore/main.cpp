#include "renderer.h"
#include "surfaces.h"

int main()
{
    Options opts;
    opts.maxSplits = 10;
    opts.xRes = 512;
    opts.yRes = 512;
    opts.gridSize = 16;
    opts.shadingRate = 1;
    opts.clipNear = 0;
    opts.clipFar = FLT_MAX;

    Renderer r(opts);

    r.add( boost::shared_ptr<Surface>(
           new Patch(opts, Vec3(-0.5,-0.5,5), Vec3(0.5,-0.5,1),
                           Vec3(-0.5,0.5,1), Vec3(0.5,0.5,5))) );

    r.render();

    return 0;
};
