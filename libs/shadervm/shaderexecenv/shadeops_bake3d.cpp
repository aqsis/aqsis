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


/** \file
		\brief Implements the basic shader operations. (bake3d related)
		\author Michel Joron joron@sympatico.ca
*/


#ifdef AQSIS_SYSTEM_WIN32
#include <io.h>
#endif

#include <cstring>

#include <Partio.h>

#include "shaderexecenv.h"

#include <aqsis/util/autobuffer.h>
#include <aqsis/util/logging.h>

namespace Aqsis
{

// Utils for bake3d() shadeop.
namespace {
struct BakeVar
{
    const IqShaderData* value;
    EqVariableType type;
    Partio::ParticleAttribute attr;

    BakeVar(const IqShaderData* value, EqVariableType type)
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
static void extractBakeVars(float* out, int igrid, IqShaderData* position,
                            IqShaderData* normal, BakeVar* bakeVars,
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
        const BakeVar& var = bakeVars[i];
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

static void releasePartioFile(Partio::ParticlesInfo* file) { file->release(); }

namespace {
/// A cache for open point cloud bake files for bake3d().
class Bake3dCache
{
    public:
        Bake3dCache()
        { }

        ~Bake3dCache()
        {
            for(FileMap::iterator i = m_files.begin(); i != m_files.end(); ++i)
                Partio::write(i->first.c_str(), *i->second);
        }

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

    private:
        typedef std::map<std::string, boost::shared_ptr<Partio::ParticlesDataMutable> > FileMap;
        FileMap m_files;
};
}

// TODO: Make non-global
static Bake3dCache g_bakeCloudCache;

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
    // P, N and r output attributes are always present
    Partio::ParticleAttribute positionAttr, normalAttr, radiusAttr;
    pointFile->attributeInfo("position", positionAttr);
    pointFile->attributeInfo("normal", normalAttr);
    pointFile->attributeInfo("radius", radiusAttr);
    // Extract list of user-specified output vars from arguments
    std::vector<BakeVar> bakeVars;
    bakeVars.reserve(cParams/2);
    CqString paramName;
    // Number of output floats.  Start with space for position and normal data.
    int nOutFloats = 6;
    for(int i = 0; i+1 < cParams; i+=2)
    {
        if(apParams[i]->Type() == type_string)
        {
            apParams[i]->GetString(paramName);
            const IqShaderData* paramValue = apParams[i+1];
            // Parameters with special meanings may be present in the varargs
            // list, but shouldn't be saved to the output file, these include:
            //
            // TODO: also "coordsystem".
            if(paramName == "interpolate")
                paramValue->GetBool(interpolate);
            else if(paramName == "radius")
                radius = paramValue;
            else if(paramName == "radiusscale")
                radiusScale = paramValue;
            else
            {
                // If none of the above special cases, we have an output
                // variable which should be saved to the file.
                int count = 0;
                Partio::ParticleAttributeType parType = Partio::FLOAT;
                EqVariableType type = paramValue->Type();
                switch(type)
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
                bakeVars.push_back(BakeVar(paramValue, type));
                // Find the named attribute in the point file, or create it if
                // it doesn't exist.
                BakeVar& var = bakeVars.back();
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
                iv = igrid / uSize;
                iu = igrid - iv*uSize;
                // Check whether we're off the edge (number of polys in each
                // direction is one less than number of verts)
                if(iv == vSize - 1 || iu == uSize - 1)
                    continue;
            }

            // Extract all baking variables into allData.
            extractBakeVars(allData.get(), igrid, position, normal,
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
                    extractBakeVars(tmpData, interpIndices[i], position, normal,
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
            P[0] = *d++; P[1] = *d++; P[2] = *d++;
            N[0] = *d++; N[1] = *d++; N[2] = *d++;
            r[0] = radiusVal;
            // Save out user-defined attributes
            for(int i = 0, iend = bakeVars.size(); i < iend; ++i)
            {
                BakeVar& var = bakeVars[i];
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
/** \brief Shadeops "texture3d" to restore any parameter from one pointcloud file refer.
 *  \param ptc the name of the pointcloud file
 *  \param point  the P
 *  \param normat its normal
 *  \result result 0 or 1
 *  \param pShader shaderexecenv
 *  \param cParams number of remaining user parameters.
 *  \param aqParams list of user parameters (to save to ptc)
 */
void	CqShaderExecEnv::SO_texture3d(IqShaderData* ptc,
                                   IqShaderData* point,
                                   IqShaderData* normal,
                                   IqShaderData* Result,
                                   IqShader* pShader,
                                   TqInt cParams, IqShaderData** apParams )
{
    // FIXME!

#if 0
    bool __fVarying;
    TqUint __iGrid;

    __fVarying=(point)->Class()==class_varying;
    __fVarying=(normal)->Class()==class_varying||__fVarying;
    __fVarying=(Result)->Class()==class_varying||__fVarying;

    __iGrid = 0;
    const CqBitVector& RS = RunningState();
    CqString _aq_ptc;
    (ptc)->GetString(_aq_ptc,__iGrid);
    TqUshort VarStarts[CqBake3DOptionsMaxParams] = {0};
    PtcPointCloud MyCloudRead = PtcManager.FindCloudRead(_aq_ptc.c_str(), VarStarts);
    CqBake3DOptions     sampleOpts;
    CqBake3DOptionsExtractor optExtractor(apParams, cParams, sampleOpts);


    if (!MyCloudRead)
    {
        const char *varnames[CqBake3DOptionsMaxParams];
        const char *vartypes[CqBake3DOptionsMaxParams];
        TqInt varnumber = 0;
        MyCloudRead = PtcOpenPointCloudFile(_aq_ptc.c_str(), &varnumber, vartypes, varnames);

        TqInt i, j;

        TqInt okay = 0;

        // Double check if this variable exist in this PTC...

        TqInt found = 0;
        for (j=0; j < sampleOpts.m_Count; j++)
        {
            for (i=0; i < varnumber; i ++)
            {
                if (strcmp(sampleOpts.m_VarNames[j], varnames[i]) == 0)
                {
                    found ++;
                    break;
                }
            }
        }
        okay = (found == sampleOpts.m_Count);

        if (!okay)
        {
            PtcClosePointCloudFile(MyCloudRead);
            MyCloudRead = 0;
        }
        else
        {

            for (i=0; i< sampleOpts.m_Count; i++)
            {
                // First we need to find where shader' variable with the userdata
                TqInt found = -1;
                for (j=0; j < varnumber && found == -1; j++)
                {
                    if (strcmp(varnames[j], sampleOpts.m_VarNames[i]) == 0)
                        found = j;
                }

                TqInt where  = 0;
                // If it is existing find its location within the userdata block
                if (found >= 0)
                {

                    for (j=0; j < found; j++)
                    {
                        if ( strcmp(vartypes[j], "float") == 0 ||
                                strcmp(vartypes[j], "integer") == 0 ||
                                strcmp(vartypes[j], "bool") == 0 )
                            where ++;
                        else if ( strcmp(vartypes[j], "matrix") == 0 )
                            where += 16;
                        else
                            where += 3;
                    }
                    VarStarts[i] = where;
                }
            }
            PtcManager.SaveCloudRead(_aq_ptc.c_str(), MyCloudRead, VarStarts);
        }
    }



    TqFloat *userdata = NULL;

    do
    {
        if(RS.Value( __iGrid ) )
        {
            TqInt okay = 1;

            if (MyCloudRead != 0)
            {
                TqFloat radius;
                if (userdata == NULL)
                {
                    TqInt size = 0;
                    PtcGetPointCloudInfo(MyCloudRead, "datasize", &size);

                    userdata = new TqFloat[size];
                }

                /*
                it could to do a simple find using qsort/bsearch based on point
                okay = PtcFindDataPoint(MyCloudRead, pointf, normalf, &radius, userdata);
                */
                // Take all the information and call PtcReadDataPoint()
                TqFloat pointf[3];
                TqFloat normalf[3];
                okay = PtcReadDataPoint(MyCloudRead, pointf, normalf, &radius, userdata);

                CqVector3D _aq_point;
                (point)->GetPoint(_aq_point,__iGrid);
                CqVector3D _aq_normal;
                (normal)->GetPoint(_aq_normal,__iGrid);
                TqFloat fRes = 0.0f;

                // Convert all the information from userdata and save the data into the user parameters
                for (TqInt i=0; i< sampleOpts.m_Count; i++)
                {
                    // First we need to find where shader' variable with the userdata
                    TqInt where  = VarStarts[i];
                    if ( strcmp(sampleOpts.m_VarTypes[i], "float") == 0 ||
                            strcmp(sampleOpts.m_VarTypes[i], "integer") == 0 ||
                            strcmp(sampleOpts.m_VarTypes[i], "bool") == 0 )
                    {
                        TqFloat f;

                        f = userdata[where];
                        sampleOpts.m_UserData[ i ]->SetFloat( f, __iGrid);
                    } else if ( strcmp(sampleOpts.m_VarTypes[i], "vector") == 0)
                    {
                        CqVector3D v;

                        for (TqInt j=0; i < 3; i++)
                        {
                            v[j] = userdata[where + j];
                        }
                        sampleOpts.m_UserData[ i ]->SetVector( v, __iGrid);
                    } else if ( strcmp(sampleOpts.m_VarTypes[i], "color") == 0)
                    {
                        CqColor c;

                        for (TqInt j=0; j < 3; j++)
                        {
                            c[j] = userdata[where + j];
                        }
                        sampleOpts.m_UserData[ i ]->SetColor( c, __iGrid);
                    } else if ( strcmp(sampleOpts.m_VarTypes[i], "point") == 0)
                    {
                        CqVector3D p;

                        for (TqInt j=0; j < 3; j++)
                        {
                            p[j] = userdata[where + j];
                        }
                        sampleOpts.m_UserData[ i ]->SetPoint( p, __iGrid);
                    } else if ( strcmp(sampleOpts.m_VarTypes[i], "normal") == 0)
                    {
                        CqVector3D n;

                        for (TqInt j=0; j < 3; j++)
                        {
                            n[j] = userdata[where + j];
                        }
                        sampleOpts.m_UserData[ i ]->SetNormal( n, __iGrid);
                    } else if ( strcmp(sampleOpts.m_VarTypes[i], "matrix") == 0)
                    {
                        CqMatrix m;

                        for (TqInt j=0; j < 16; j++)
                        {
                            m.pElements()[j] = userdata[where + j];
                        }
                        sampleOpts.m_UserData[ i ]->SetMatrix( m, __iGrid);
                    }
                }
                fRes = okay;
                (Result)->SetFloat(fRes,__iGrid);
            }
        }
    } while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
    if (userdata != NULL)
    {
        delete [] userdata;
    }
#endif
}

//---------------------------------------------------------------------

} // namespace Aqsis

