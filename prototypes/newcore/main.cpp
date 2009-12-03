#include "renderer.h"
#include "surfaces.h"

int main()
{
    Options opts;
    opts.xRes = 1024;
    opts.yRes = 1024;
    opts.gridSize = 8;
    opts.shadingRate = 1000;
    opts.smoothShading = false;

    Renderer r(opts);

    boost::shared_ptr<Geometry> patch(
           new Patch(opts, Vec3(0.2,0.2,5), Vec3(0.5,-0.5,1),
                           Vec3(-0.5,0.5,1), Vec3(0.5,0.5,5)) );
//    patch->transform(Mat4().setTranslation(Vec3(-0.3,-0.3,0))
//            * Mat4().setScale(Vec3(4, 4, 1)));

    r.add(patch);
    r.render();

    return 0;
};
