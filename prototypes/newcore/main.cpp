#include "renderer.h"
#include "surfaces.h"
#include "simple.h"

#define ARRLEN(ar) sizeof(ar)/sizeof(ar[0])

boost::shared_ptr<Geometry> createPatch(const Options& opts,
                                       const Vec3& a, const Vec3& b,
                                       const Vec3& c, const Vec3& d,
                                       const Mat4& trans = Mat4())
{
    IclassStorage storReq(1,4,4,4,4);
    boost::shared_ptr<PrimvarStorage> vars(new PrimvarStorage(storReq));
    float P[] = {
        a.x, a.y, a.z,  b.x, b.y, b.z,
        c.x, c.y, c.z,  d.x, d.y, d.z,
    };
    vars->add(Primvar::P, P, ARRLEN(P));
    float Cs[] = {
        1, 0, 0,  0, 1, 0,  0, 0, 1,  1, 1, 1
    };
    vars->add(Primvar::Cs, Cs, ARRLEN(Cs));
    boost::shared_ptr<Geometry> patch(new Patch(opts, vars));
//    boost::shared_ptr<Geometry> patch(new PatchSimple(opts, a,b,c,d));
    patch->transform(trans);
    return patch;
}

void addCube(const Options& opts, Renderer& r, const Mat4& otow)
{
    r.add(createPatch(opts, Vec3(-1,-1,-1), Vec3(-1,-1,1),
                            Vec3(-1,1,-1), Vec3(-1,1,1), otow) );
    r.add(createPatch(opts, Vec3(1,-1,-1), Vec3(1,-1,1),
                            Vec3(1,1,-1), Vec3(1,1,1), otow) );

    r.add(createPatch(opts, Vec3(-1,-1,-1), Vec3(-1,-1,1),
                            Vec3(1,-1,-1), Vec3(1,-1,1), otow) );
    r.add(createPatch(opts, Vec3(-1,1,-1), Vec3(-1,1,1),
                            Vec3(1,1,-1), Vec3(1,1,1), otow) );

    r.add(createPatch(opts, Vec3(-1,-1,-1), Vec3(-1,1,-1),
                            Vec3(1,-1,-1), Vec3(1,1,-1), otow) );
    r.add(createPatch(opts, Vec3(-1,-1,1), Vec3(-1,1,1),
                            Vec3(1,-1,1), Vec3(1,1,1), otow) );
}

int main(int argc, char* argv[])
{
    Options opts;
    opts.xRes = 1024;
    opts.yRes = 1024;
    opts.gridSize = 8;
    opts.shadingRate = 1;
    opts.smoothShading = true;
    opts.clipNear = 0.1;

    Mat4 camToScreen =
//        Mat4();
//        perspectiveProjection(90, opts.clipNear, opts.clipFar);
        screenWindow(0,0.5, 0,0.5);

    // Output variables.
    VarList outVars;
    outVars.push_back(Stdvar::Cs);
    outVars.push_back(Stdvar::z);

    Renderer r(opts, camToScreen, outVars);

//    r.add(createPatch(opts, Vec3(-0.5,-0.5,0), Vec3(0.5,-0.5,0),
//                            Vec3(-0.5,0.5,0), Vec3(0.5,0.5,0),
//                            Mat4().setAxisAngle(Vec3(1,0,0), deg2rad(50))
//                            * Mat4().setTranslation(Vec3(0,0,1))));

    int numPatches = 1;
    if(argc > 1)
        numPatches = 10;

    for(int i = 0; i < numPatches; ++i)
    {
        r.add(createPatch(opts, Vec3(0.2,0.2,5), Vec3(0.5,-0.5,1),
                                Vec3(-0.5,0.5,1), Vec3(0.5,0.5,5)) );
    }

    // Cube geometry
//    addCube(opts, r,
//        Mat4().setAxisAngle(Vec3(0,1,0), deg2rad(45)) *
//        Mat4().setAxisAngle(Vec3(1,0,0), deg2rad(45)) *
//        Mat4().setTranslation(Vec3(0,0,3))
//    );

    r.render();

    return 0;
}
