// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


#ifdef AQSIS_SYSTEM_WIN32
#include <io.h>
#endif

#include <cstring>

#include <Partio.h>

#include "shaderexecenv.h"

#include <aqsis/util/autobuffer.h>
#include <aqsis/util/logging.h>

#include <OpenEXR/ImathVec.h>

namespace Aqsis
{

using Imath::V3f;

// Utils for bake3d() shadeop.
namespace {
struct UserVar
{
    IqShaderData* value;
    EqVariableType type;
    Partio::ParticleAttribute attr;

    UserVar(IqShaderData* value, EqVariableType type,
             const Partio::ParticleAttribute& attr)
        : value(value), type(type), attr(attr) {}
    UserVar(IqShaderData* value, EqVariableType type)
        : value(value), type(type) {}
};
}

/// Extract float data from variables to be baked.
///
/// \param out - all output variables are squashed together into this array.
///              The ordering is P[3] N[3] userdata[...]
/// \param igrid - grid index at which to retrieve the data
/// \param position - position array
/// \param normal - normal array
/// \param bakeVars - array of shader data and associated types
/// \param nBakeVars - length of bakeVars array.
static void extractUserVars(float* out, int igrid, IqShaderData* position,
                            IqShaderData* normal, UserVar* bakeVars,
                            int nBakeVars)
{
    // Temp vars for extracting data from IqShaderData
    TqFloat f;
    CqVector3D p;
    CqColor c;
    CqMatrix m;

    // Extract position and normal; always required
    position->GetPoint(p, igrid);
    *out++ = p.x(); *out++ = p.y(); *out++ = p.z();
    normal->GetNormal(p, igrid);
    *out++ = p.x(); *out++ = p.y(); *out++ = p.z();

    // Get all user-defined parameters and assemble them together into array
    // of floats to pass to Ptc API.
    for(int i = 0; i < nBakeVars; ++i)
    {
        const UserVar& var = bakeVars[i];
        switch(var.type)
        {
            case type_float:
                var.value->GetFloat(f, igrid);
                *out++ = f;
                break;
            case type_point:
                var.value->GetPoint(p, igrid);
                *out++ = p.x(); *out++ = p.y(); *out++ = p.z();
                break;
            case type_normal:
                var.value->GetNormal(p, igrid);
                *out++ = p.x(); *out++ = p.y(); *out++ = p.z();
                break;
            case type_vector:
                var.value->GetVector(p, igrid);
                *out++ = p.x(); *out++ = p.y(); *out++ = p.z();
                break;
            case type_color:
                var.value->GetColor(c, igrid);
                *out++ = c.r(); *out++ = c.g(); *out++ = c.b();
                break;
            case type_matrix:
                var.value->GetMatrix(m, igrid);
                for(int k = 0; k < 4; ++k)
                for(int h = 0; h < 4; ++h)
                    *out++ = m[k][h];
                break;
            default:
                assert(0 && "unexpected type");
                break;
        }
    }
}

//------------------------------------------------------------------------------

static void releasePartioFile(Partio::ParticlesInfo* file)
{
    if(file) file->release();
}

namespace {
/// A cache for open point cloud bake files for bake3d().
class Bake3dCache
{
    public:
        /// Find or create a point cloud with the given name.
        ///
        /// The standard attributes; position, normal, and radius are added on
        /// creation.
        Partio::ParticlesDataMutable* find(const std::string& fileName)
        {
            Partio::ParticlesDataMutable* pointFile = 0;
            FileMap::iterator ptcIter = m_files.find(fileName);
            if(ptcIter == m_files.end())
            {
                // Create new bake file & insert into map.
                pointFile = Partio::create();
                m_files[fileName].reset(pointFile, releasePartioFile);
                if(pointFile)
                {
                    // Add default attributes
                    pointFile->addAttribute("position", Partio::VECTOR, 3);
                    pointFile->addAttribute("normal", Partio::VECTOR, 3);
                    pointFile->addAttribute("radius", Partio::FLOAT, 1);
                }
                else
                {
                    Aqsis::log() << error
                        << "bake3d: Could not open point cloud \"" << fileName
                        << "\" for writing\n";
                }
            }
            else
                pointFile = ptcIter->second.get();
            return pointFile;
        }

        /// Flush all files to disk and clear the cache
        void flush()
        {
            for(FileMap::iterator i = m_files.begin(); i != m_files.end(); ++i)
                Partio::write(i->first.c_str(), *i->second);
            m_files.clear();
        }

    private:
        typedef std::map<std::string, boost::shared_ptr<Partio::ParticlesDataMutable> > FileMap;
        FileMap m_files;
};
}


namespace {
/// A cache for open point cloud bake files for texture3d().
class Texture3dCache
{
    public:
        /// Find a point cloud with the given name, or open it from file.
        Partio::ParticlesData* find(const std::string& fileName)
        {
            Partio::ParticlesDataMutable* pointFile = 0;
            FileMap::iterator ptcIter = m_files.find(fileName);
            if(ptcIter == m_files.end())
            {
                // Create new bake file & insert into map.
                pointFile = Partio::read(fileName.c_str());
                m_files[fileName].reset(pointFile, releasePartioFile);
                if(pointFile)
                {
                    // Sort file so that it can be looked up efficiently.
                    pointFile->sort();
                    // TODO: Ensure that radius, position & normal attributes
                    // are present.
                }
                else
                {
                    Aqsis::log() << error
                        << "texture3d: Could not open point cloud \"" << fileName
                        << "\" for reading\n";
                }
            }
            else
                pointFile = ptcIter->second.get();
            return pointFile;
        }

        /// Flush all files from the cache.
        void clear()
        {
            m_files.clear();
        }

    private:
        typedef std::map<std::string, boost::shared_ptr<Partio::ParticlesDataMutable> > FileMap;
        FileMap m_files;
};
}


// TODO: Make non-global
static Bake3dCache g_bakeCloudCache;
static Texture3dCache g_texture3dCloudCache;

void flushBake3dCache()
{
    g_bakeCloudCache.flush();
    g_texture3dCloudCache.clear();
}

//------------------------------------------------------------------------------
/// Shadeop to bake vertices to point cloud
///
/// bake3d(fileName, P, N, format, ...)
///
/// \param ptc      - the name of the pointcloud file
/// \param channels - not used.  For PRMan compatibility.)
/// \param position - vertex position
/// \param normal   - normal at vertex
/// \result result  - 0 or 1 for failure or success
/// \param pShader  - unused
/// \param cParams  - number of extra user parameters.
/// \param apParams - list of extra parameters to control output or save to ptc.
///
void CqShaderExecEnv::SO_bake3d( IqShaderData* ptc,
                                 IqShaderData* channels,
                                 IqShaderData* position,
                                 IqShaderData* normal,
                                 IqShaderData* Result,
                                 IqShader* pShader,
                                 TqInt cParams, IqShaderData** apParams )
{
    const CqBitVector& RS = RunningState();
    CqString ptcName;
    ptc->GetString(ptcName);
    // Find point cloud in cache, or create it if it doesn't exist.
    Partio::ParticlesDataMutable* pointFile = g_bakeCloudCache.find(ptcName);
    bool varying = position->Class() == class_varying ||
                   normal->Class() == class_varying ||
                   Result->Class() == class_varying;
    if(!pointFile)
    {
        // Error; set result to false and return.
        int npoints = varying ? shadingPointCount() : 1;
        for(int igrid = 0; igrid < npoints; ++igrid)
            if(!varying || RS.Value(igrid))
                Result->SetFloat(0.0f, igrid);
        return;
    }
    // Optional output control variables
    bool interpolate = false;
    const IqShaderData* radius = 0;
    const IqShaderData* radiusScale = 0;
    CqString coordSystem = "world";
    // P, N and r output attributes are always present
    Partio::ParticleAttribute positionAttr, normalAttr, radiusAttr;
    pointFile->attributeInfo("position", positionAttr);
    pointFile->attributeInfo("normal", normalAttr);
    pointFile->attributeInfo("radius", radiusAttr);
    // Extract list of user-specified output vars from arguments
    std::vector<UserVar> bakeVars;
    bakeVars.reserve(cParams/2);
    CqString paramName;
    // Number of output floats.  Start with space for position and normal data.
    int nOutFloats = 6;
    for(int i = 0; i+1 < cParams; i+=2)
    {
        if(apParams[i]->Type() == type_string)
        {
            apParams[i]->GetString(paramName);
            IqShaderData* paramValue = apParams[i+1];
            EqVariableType paramType = paramValue->Type();
            // Parameters with special meanings may be present in the varargs
            // list, but shouldn't be saved to the output file, these include:
            if(paramName == "interpolate" && paramType == type_float)
                paramValue->GetBool(interpolate);
            else if(paramName == "radius" && paramType == type_float)
                radius = paramValue;
            else if(paramName == "radiusscale" && paramType == type_float)
                radiusScale = paramValue;
            else if(paramName == "coordsystem" && paramType == type_string)
                paramValue->GetString(coordSystem);
            else
            {
                // If none of the above special cases, we have an output
                // variable which should be saved to the file.
                int count = 0;
                Partio::ParticleAttributeType parType = Partio::FLOAT;
                switch(paramType)
                {
                    case type_float:  count = 1;  parType = Partio::FLOAT;  break;
                    case type_point:  count = 3;  parType = Partio::VECTOR; break;
                    case type_color:  count = 3;  parType = Partio::FLOAT;  break;
                    case type_normal: count = 3;  parType = Partio::VECTOR; break;
                    case type_vector: count = 3;  parType = Partio::VECTOR; break;
                    case type_matrix: count = 16; parType = Partio::FLOAT;  break;
                    default:
                        Aqsis::log() << warning
                            << "bake3d: Can't save non-float argument \""
                            << paramName << "\"\n";
                        continue;
                }
                bakeVars.push_back(UserVar(paramValue, paramType));
                // Find the named attribute in the point file, or create it if
                // it doesn't exist.
                UserVar& var = bakeVars.back();
                if(pointFile->attributeInfo(paramName.c_str(), var.attr))
                {
                    if(var.attr.count != count)
                    {
                        Aqsis::log() << warning
                            << "bake3d: can't bake variable \"" << paramName
                            << "\"; previously baked with different type\n";
                        bakeVars.pop_back();
                    }
                }
                else
                    var.attr = pointFile->addAttribute(paramName.c_str(),
                                                       parType, count);
                nOutFloats += count;
            }
        }
        else
            Aqsis::log() << "unexpected non-string for parameter name "
                            "in bake3d()\n";
    }

    /// Compute transformations
    CqMatrix positionTrans;
    getRenderContext()->matSpaceToSpace("current", coordSystem.c_str(),
                                        pShader->getTransform(),
                                        pTransform().get(), 0, positionTrans);
    CqMatrix normalTrans = normalTransform(positionTrans);

    CqAutoBuffer<TqFloat, 100> allData(interpolate ?
                                       2*nOutFloats : nOutFloats);

    // Number of vertices in the grid
    int uSize = m_uGridRes+1;
    int vSize = m_vGridRes+1;

    TqUint igrid = 0;
    do
    {
        if(RS.Value( igrid ) )
        {
            int iu = 0, iv = 0;
            if(interpolate)
            {
                // Get micropoly position on 2D grid.
                // TODO: What if the grid is 1D or 0D?
                iv = igrid / uSize;
                iu = igrid - iv*uSize;
                // Check whether we're off the edge (number of polys in each
                // direction is one less than number of verts)
                if(iv == vSize - 1 || iu == uSize - 1)
                    continue;
            }

            // Extract all baking variables into allData.
            extractUserVars(allData.get(), igrid, position, normal,
                            &bakeVars[0], bakeVars.size());

            // Get radius if it's avaliable, otherwise compute automatically
            // below.
            float radiusVal = 0;
            if(radius)
                radius->GetFloat(radiusVal, igrid);

            if(interpolate)
            {
                int interpIndices[3] = {
                    iv*uSize       + iu + 1,
                    (iv + 1)*uSize + iu,
                    (iv + 1)*uSize + iu + 1
                };
                float* tmpData = allData.get() + nOutFloats;
                float* outData = allData.get();
                CqVector3D P[4]; // current micropoly vertex positions.
                P[0] = CqVector3D(outData);
                // Extract data from the three other verts on the current
                // micropolygon & merge into outData.
                for(int i = 0; i < 3; ++i)
                {
                    extractUserVars(tmpData, interpIndices[i], position, normal,
                                    &bakeVars[0], bakeVars.size());
                    for(int j = 0; j < nOutFloats; ++j)
                        outData[j] += tmpData[j];
                    P[i+1] = CqVector3D(tmpData);
                }
                // normalize averages
                for(int j = 0; j < nOutFloats; ++j)
                    outData[j] *= 0.25f;
                if(!radius)
                {
                    CqVector3D Pmid = CqVector3D(outData);
                    // Compute radius using vertex positions.  Radius is the
                    // maximum distance from the centre of the micropolygon to
                    // a vertex.
                    radiusVal = (Pmid - P[0]).Magnitude2();
                    for(int i = 1; i < 4; ++i)
                        radiusVal = std::max(radiusVal,
                                             (Pmid - P[i]).Magnitude2());
                    radiusVal = std::sqrt(radiusVal);
                }
            }
            else
            {
                if(!radius)
                {
                    // Extract radius, non-interpolation case.
                    CqVector3D e1 = diffU<CqVector3D>(position, igrid);
                    CqVector3D e2 = diffV<CqVector3D>(position, igrid);
                    // Distances from current vertex to diagonal neighbours.
                    float d1 = (e1 + e2).Magnitude2();
                    float d2 = (e1 - e2).Magnitude2();
                    // Choose distance to furtherest diagonal neighbour so
                    // that the disks just overlap to produce a surface
                    // without holes.  The factor of 0.5 gives the radius
                    // rather than diameter.
                    radiusVal = 0.5f*std::sqrt(std::max(d1, d2));
                }
            }

            // Scale radius if desired.
            if(radiusScale)
            {
                float scale = 1;
                radiusScale->GetFloat(scale, igrid);
                radiusVal *= scale;
            }

            // Save current point data to the point file
            float* d = &allData[0];
            Partio::ParticleIndex ptIdx = pointFile->addParticle();
            // Save out standard attributes
            float* P = pointFile->dataWrite<float>(positionAttr, ptIdx);
            float* N = pointFile->dataWrite<float>(normalAttr, ptIdx);
            float* r = pointFile->dataWrite<float>(radiusAttr, ptIdx);
            CqVector3D cqP = positionTrans * CqVector3D(d[0], d[1], d[2]); d += 3;
            P[0] = cqP.x(); P[1] = cqP.y(); P[2] = cqP.z();
            CqVector3D cqN = normalTrans * CqVector3D(d[0], d[1], d[2]); d += 3;
            N[0] = cqN.x(); N[1] = cqN.y(); N[2] = cqN.z();
            r[0] = radiusVal;
            // Save out user-defined attributes
            for(int i = 0, iend = bakeVars.size(); i < iend; ++i)
            {
                UserVar& var = bakeVars[i];
                float* out = pointFile->dataWrite<float>(var.attr, ptIdx);
                for(int j = 0; j < var.attr.count; ++j)
                    out[j] = *d++;
            }
            Result->SetFloat(1, igrid);
        }
    }
    while( ( ++igrid < shadingPointCount() ) && varying);
}


//------------------------------------------------------------------------------
// Texture3d stuff

// Parse and validate the varargs part of the texture3d argument list.
//
// nargs and args specify the varargs list.
static bool parseTexture3dVarargs(int nargs, IqShaderData** args,
                                  const Partio::ParticlesData* pointFile,
                                  CqString& coordSystem,
                                  std::vector<UserVar>& userVars)
{
    userVars.reserve(nargs/2);
    CqString paramName;
    for(int i = 0; i+1 < nargs; i+=2)
    {
        if(args[i]->Type() != type_string)
        {
            Aqsis::log() << error
                << "unexpected non-string for parameter name in texture3d()\n";
            return false;
        }
        args[i]->GetString(paramName);
        IqShaderData* paramValue = args[i+1];
        EqVariableType paramType = paramValue->Type();
        // Parameters with special meanings may be present in the varargs
        // list, and shouldn't be looked up in the file:
        if(paramName == "coordsystem" && paramType == type_string)
            paramValue->GetString(coordSystem);
        else if(paramName == "filterradius")
            ; // For brick maps; ignored for now
        else if(paramName == "filterscale")
            ; // For brick maps; ignored for now
        else if(paramName == "maxdepth")
            ; // For brick maps; ignored for now
        else
        {
            // If none of the above special cases, we have a user-defined
            // variable.  Look it up in the point file, and check whether the
            // type corresponds with the provided shader variable.
            Partio::ParticleAttribute attr;
            pointFile->attributeInfo(paramName.c_str(), attr);
            if(attr.type != Partio::FLOAT && attr.type != Partio::VECTOR)
            {
                Aqsis::log() << warning
                    << "texture3d: Can't load non-float data \""
                    << paramName << "\"\n";
                continue;
            }
            int desiredCount = 0;
            switch(paramType)
            {
                case type_float:  desiredCount = 1;  break;
                case type_point:  desiredCount = 3;  break;
                case type_color:  desiredCount = 3;  break;
                case type_normal: desiredCount = 3;  break;
                case type_vector: desiredCount = 3;  break;
                case type_matrix: desiredCount = 16; break;
                default:
                    Aqsis::log() << warning
                        << "texture3d: Can't load non float-based argument \""
                        << paramName << "\"\n";
                    continue;
            }
            if(desiredCount != attr.count)
            {
                Aqsis::log() << warning
                    << "texture3d: variable \"" << paramName
                    << "\" mismatched in point file\n";
                continue;
            }
            userVars.push_back(UserVar(paramValue, paramType, attr));
        }
    }
    return userVars.size() > 0;
}


/** \brief Shadeops "texture3d" to restore any parameter from one pointcloud file refer.
 *  \param ptc the name of the pointcloud file
 *  \param position
 *  \param normal
 *  \result result 0 or 1
 *  \param pShader shaderexecenv
 *  \param cParams number of remaining user parameters.
 *  \param aqParams list of user parameters (to save to ptc)
 */
void CqShaderExecEnv::SO_texture3d(IqShaderData* ptc,
                                   IqShaderData* position,
                                   IqShaderData* normal,
                                   IqShaderData* Result,
                                   IqShader* pShader,
                                   TqInt cParams, IqShaderData** apParams )
{
    const CqBitVector& RS = RunningState();
    CqString ptcName;
    ptc->GetString(ptcName);

    Partio::ParticlesData* pointFile = g_texture3dCloudCache.find(ptcName);
    bool varying = position->Class() == class_varying ||
                   normal->Class() == class_varying ||
                   Result->Class() == class_varying;
    int npoints = varying ? shadingPointCount() : 1;

    CqString coordSystem = "world";
    std::vector<UserVar> userVars;

    if(!pointFile || !parseTexture3dVarargs(cParams, apParams,
                                            pointFile, coordSystem, userVars))
    {
        // Error - no point file or no arguments to look up: set result to 0
        // and return.
        for(int igrid = 0; igrid < npoints; ++igrid)
            if(!varying || RS.Value(igrid))
                Result->SetFloat(0, igrid);
        return;
    }

    /// Compute transformations
    CqMatrix positionTrans;
    getRenderContext()->matSpaceToSpace("current", coordSystem.c_str(),
                                        pShader->getTransform(),
                                        pTransform().get(), 0, positionTrans);
    CqMatrix normalTrans = normalTransform(positionTrans);

    // Grab the standard attributes for computing filter weights
    Partio::ParticleAttribute positionAttr, normalAttr, radiusAttr;
    pointFile->attributeInfo("position", positionAttr);
    pointFile->attributeInfo("normal", normalAttr);
    pointFile->attributeInfo("radius", radiusAttr);

    for(int igrid = 0; igrid < npoints; ++igrid)
    {
        if(varying && !RS.Value(igrid))
            continue;
        CqVector3D cqP, cqN;
        position->GetPoint(cqP, igrid);
        cqP = positionTrans*cqP;
        normal->GetNormal(cqN, igrid);
        cqN = normalTrans*cqN;

        // Using the four nearest neighbours for filtering is roughly the
        // minimum we can get away with for a surface made out of
        // quadrilaterals.  This isn't exactly perfect near edges but it seems
        // to be good enough.
        //
        // Since the lookups happen without prefiltering, no effort is made to
        // avoid aliasing by using a filter radius based on the size of the
        // current shading element.
        const int nfilter = 4;
        Partio::ParticleIndex indices[nfilter];
        float distSquared[nfilter];
        float maxRadius2 = 0;
        V3f P(cqP.x(), cqP.y(), cqP.z());
        // The reinterpret_casts are ugly here of course, but V3f is basically
        // a POD type, so they work ok.
        int pointsFound = pointFile->findNPoints(reinterpret_cast<float*>(&P),
                                                 nfilter, FLT_MAX, indices,
                                                 distSquared, &maxRadius2);
        if(pointsFound < nfilter)
        {
            Result->SetFloat(0.0f, igrid);
            Aqsis::log() << error << "Not enough points found to filter!";
        }
        // Read position, normal and radius
        V3f foundP[nfilter];
        pointFile->dataAsFloat(positionAttr, nfilter, indices, false,
                               reinterpret_cast<float*>(foundP));
        V3f foundN[nfilter];
        pointFile->dataAsFloat(normalAttr, nfilter, indices, false,
                               reinterpret_cast<float*>(foundN));
        float foundRadius[nfilter];
        pointFile->dataAsFloat(radiusAttr, nfilter, indices, false, foundRadius);

        // Compute filter weights for nearby points.  inverseWidthSquared
        // decides the blurryness of the gaussian filter.  The value was chosen
        // by eyeballing the results of various widths.  As usual it's a trade
        // off: Too small a value will be too blurry, (and artifacty if nfilter
        // is 4); too large a value and it ends up looking like nearest
        // neighbour filtering.
        const float inverseWidthSquared = 1.3f;
        float weights[nfilter];
        float totWeight = 0;
        V3f N(cqN.x(), cqN.y(), cqN.z());
        for(int i = 0; i < nfilter; ++i)
        {
            // The weights depend on how well the normals are aligned, and the
            // distance between the current shading point and the points found
            // in the point cloud.
            float wN = std::max(0.0f, N.dot(foundN[i]));
            // TODO: Speed this up using a lookup table for exp?
            float wP = std::exp(-inverseWidthSquared * distSquared[i]
                                / (foundRadius[i]*foundRadius[i]));
            // Clamping to a minimum of 1e-7 means the weights don't quite
            // become zero as distSquared becomes large, so texture lookups
            // even far from any point in the cloud return something nonzero.
            //
            // Not sure if this is a good idea or not, so commented out for now
            // wP = std::max(1e-7f, wP);
            float w = wN*wP;
            weights[i] = w;
            totWeight += w;
        }
        // Normalize the weights
        float renorm = totWeight != 0 ? 1/totWeight : 0;
        for(int i = 0; i < nfilter; ++i)
            weights[i] *= renorm;

        for(std::vector<UserVar>::const_iterator var = userVars.begin();
            var != userVars.end(); ++var)
        {
            // Read and filter each piece of user-defined data
            float varData[16*nfilter];
            pointFile->dataAsFloat(var->attr, nfilter, indices, false, varData);
            int varSize = var->attr.count;
            float accum[16];
            for(int c = 0; c < varSize; ++c)
                accum[c] = 0;
            for(int i = 0; i < nfilter; ++i)
                for(int c = 0; c < varSize; ++c)
                    accum[c] += weights[i] * varData[i*varSize + c];
            // Ah, if only we could get at the raw floats stored by
            // IqShaderData, we wouldn't need this switch
            switch(var->type)
            {
                case type_float:
                    var->value->SetFloat(accum[0], igrid);
                    break;
                case type_color:
                    var->value->SetColor(CqColor(accum[0], accum[1],
                                                 accum[2]), igrid);
                    break;
                case type_point:
                    var->value->SetPoint(CqVector3D(accum[0], accum[1],
                                                    accum[2]), igrid);
                    break;
                case type_normal:
                    var->value->SetNormal(CqVector3D(accum[0], accum[1],
                                                     accum[2]), igrid);
                    break;
                case type_vector:
                    var->value->SetVector(CqVector3D(accum[0], accum[1],
                                                     accum[2]), igrid);
                    break;
                case type_matrix:
                    var->value->SetMatrix(CqMatrix(accum), igrid);
                    break;
                default: assert(0 && "unknown type");
            }
        }
        // Return success for this point
        Result->SetFloat(1.0f, igrid);
    }
}

//---------------------------------------------------------------------

} // namespace Aqsis

