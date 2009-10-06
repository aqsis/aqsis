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
#include	<io.h>
#endif

#include	<cstring>

#include	"shaderexecenv.h"

namespace Aqsis
{


// helper functions and classes.

//------------------------------------------------------------------------------
/** \brief Contain the extended renderman bake3d,texture3d sampling options
 *
 * The standard renderman bake3d/texture3d options may be passed to calls to the
 * bake3d() and texture3d() builtin functions in the RSL.
 */
const TqInt CqBake3DOptionsMaxParams =  100;

class CqBake3DOptions
{
public:

    /** \brief Set all options to sensible default values
     *
     * The defaults from PtcCloudApi are used this is the place
     * to add more option for bake3d, texture3d in particular later
     * we will add how we look for entries within the pointcloud files.
     */
    CqBake3DOptions()
            : m_Radius(1.0), m_Count(0), m_MaxSize(0)
    {
        TqInt i;
        m_Format[0] = 800.0f;
        m_Format[1] = 600.0f;
        m_Format[2] = 1.0f;
        CqMatrix identity;
        identity.Identity();
        for ( i = 0; i < 16; i++)
        {
            m_Eye2NDC[i] = identity.pElements()[i];
            m_Eye2World[i] = identity.pElements()[i];
        }
        for (TqInt i =0; i < CqBake3DOptionsMaxParams; i++)
        {
            m_VarNames[i] = NULL;
            m_VarTypes[i] = NULL;
            m_UserData[i] = NULL;
            m_VarSizes[i] = 0;
        }

    }
    ~CqBake3DOptions()
    {
        for (TqInt i =0; i < CqBake3DOptionsMaxParams; i++)
        {
            free(m_VarNames[i]);
            free(m_VarTypes[i]);
        }
    }

    void SetRadius(TqFloat a_Radius)
    {
        m_Radius = a_Radius;
    }

    void SetFormat(TqFloat *a_Format)
    {
        for (TqInt i = 0; i < 3; i++)
            m_Format[i] = a_Format[i];
    }

    void SetEye2NDC(CqMatrix a_Eye2NDC)
    {
        for (TqInt i = 0; i < 16; i++)
            m_Eye2NDC[i] = a_Eye2NDC.pElements()[i];
    }

    void SetEye2World(CqMatrix a_Eye2World)
    {
        for (TqInt i = 0; i < 16; i++)
            m_Eye2World[i] = a_Eye2World.pElements()[i];
    }

    void AddUserDefinition(CqString a_Name, CqString a_Type, IqShaderData* value)
    {

        if (m_Count < CqBake3DOptionsMaxParams)
        {
            m_UserData[m_Count] = value;
            m_VarNames[m_Count] = strdup (a_Name.c_str());
            m_VarTypes[m_Count] = strdup (a_Type.c_str());

            if (a_Type == "float" || a_Type == "bool" || a_Type == "integer")
            {
                m_VarSizes[m_Count] = 1;
                m_MaxSize ++;
            }
            else if (a_Type == "matrix" )
            {
                m_VarSizes[m_Count] = 16;
                m_MaxSize += 16;
            }
            else
            {
                m_VarSizes[m_Count] = 3;
                m_MaxSize += 3;
            }

            m_Count ++;
        }
    }

    /// Value to fill a channel with if the associated texture doesn't contain sufficiently many channels.
    TqFloat m_Radius;
    TqFloat m_Format[3];
    TqFloat m_Eye2NDC[16];
    TqFloat m_Eye2World[16];
    TqChar       *m_VarNames[CqBake3DOptionsMaxParams];
    TqChar       *m_VarTypes[CqBake3DOptionsMaxParams];
    IqShaderData *m_UserData[CqBake3DOptionsMaxParams];
    TqUshort      m_VarSizes[CqBake3DOptionsMaxParams];

    TqUshort m_Count;
    TqUshort m_MaxSize;

};

//------------------------------------------------------------------------------
/** \brief Extractor for bake3d/texture3d options
 */
class CqBake3DOptionsExtractor
            : private CqBake3DOptions
{
public:
    CqBake3DOptionsExtractor(IqShaderData** paramList, TqInt numParams,
                             CqBake3DOptions& opts)
    {
        extractUniformAndCacheVarying(paramList, numParams, opts);
    }
    ~CqBake3DOptionsExtractor(){}

protected:

    void handleUniformParam(const CqString& name, IqShaderData* value,
                                    CqBake3DOptions& opts)
    {
        if(name == "radius")
        {
            TqFloat tmp = 0;
            value->GetFloat(tmp, 0);
            opts.SetRadius(tmp);
        }
        else if(name == "format")
        {
            TqFloat *tmp = NULL;
            value->GetFloatPtr(tmp);
            if (NULL != tmp && value->ArrayLength() == 3)
            {
                opts.SetFormat(tmp);
            }
        }
        else if(name == "eye2ndc")
        {
            CqMatrix tmp;
            value->GetMatrix(tmp);
            opts.SetEye2NDC(tmp);
        }
        else if(name == "eye2world")
        {
            CqMatrix tmp;
            value->GetMatrix(tmp);
            opts.SetEye2World(tmp);
        }
    }
    void handleUserParam(const CqString& name, IqShaderData* value,
                                 CqBake3DOptions& opts)
    {
        if((name != "radius") && (name != "format") && (name != "eye2ndc") && (name != "eye2world"))
        {
            CqString type = enumString(value->Type());
            opts.AddUserDefinition(name, type, value);
        }

    }
    void extractUniformAndCacheVarying(IqShaderData** paramList, TqInt numParams,
                                       CqBake3DOptions& opts)
    {
        CqString paramName;
        for(TqInt i = 0; i < numParams; i+=2)
        {
            // Parameter name and data
            paramList[i]->GetString(paramName, 0);
            IqShaderData* param = paramList[i+1];

            handleUniformParam(paramName, param, opts); // This will try to find radius, eye2world, eye2ndc, format
            handleUserParam(paramName, param, opts); // This will cache the rest of parameters
        }
    }
};


#include <aqsis/ri/pointcloud.h>

//------------------------------------------------------------------------------
/** \brief Singleton plain manager of PointCloud files; they are split in two
 * kind the one to write and the one to read from.
 */
class  PtcFileManager
{
public:
    typedef struct
    {
        TqChar filename[1024];
        PtcPointCloud CloudFile;
        TqUchar ReadWrite;
        TqUshort Starts[CqBake3DOptionsMaxParams];
    }
    PtcMapEntries;

    struct
    {
        TqUshort MaxFiles;
        TqUshort CurrentFiles;
        PtcMapEntries *pList;
    }
    MyList;

    PtcPointCloud FindCloudRead(const TqChar *s, TqUshort *Starts)
    {
        TqInt i;
        for (i=0; i < MyList.CurrentFiles; i++)
        {
            if (MyList.pList[i].ReadWrite == 0 && strcmp(MyList.pList[i].filename, s) == 0)
            {
                for (TqInt j =0; j < CqBake3DOptionsMaxParams; j++)
                    Starts[j] = MyList.pList[i].Starts[j];
                return MyList.pList[i].CloudFile;
            }
        }
        return 0;
    }

    PtcPointCloud FindCloudWrite(const TqChar *s)
    {
        TqInt i;
        for (i=0; i < MyList.CurrentFiles; i++)
        {
            if (MyList.pList[i].ReadWrite == 1 && strcmp(MyList.pList[i].filename, s) == 0)
            {
                return MyList.pList[i].CloudFile;
            }
        }
        return 0;
    }

    void SaveCloud(const char *s, PtcPointCloud File)
    {
        if (MyList.MaxFiles == 0)
        {
            MyList.pList = (PtcMapEntries*) calloc( 3, sizeof(PtcMapEntries));
            MyList.MaxFiles = 3;
            MyList.CurrentFiles = 0;
        }
        else if (MyList.CurrentFiles >= MyList.MaxFiles)
        {

            MyList.MaxFiles += 3;
            MyList.pList =(PtcMapEntries*) realloc(MyList.pList, MyList.MaxFiles  * sizeof(PtcMapEntries));
        }

        strcpy(MyList.pList[MyList.CurrentFiles].filename, s);
        MyList.pList[MyList.CurrentFiles].CloudFile = File;

    }

    void SaveCloudRead(const char *s, PtcPointCloud File, TqUshort *Starts)
    {
        SaveCloud(s, File);
        MyList.pList[MyList.CurrentFiles].ReadWrite = 0;
        for (TqInt i = 0; i < CqBake3DOptionsMaxParams; i++)
            MyList.pList[MyList.CurrentFiles].Starts[i] = Starts[i];
        MyList.CurrentFiles ++;
    }

    void SaveCloudWrite(const char *s, PtcPointCloud File)
    {
        SaveCloud(s, File);
        MyList.pList[MyList.CurrentFiles].ReadWrite = 1;
        MyList.CurrentFiles ++;
    }
    PtcFileManager()
    {
        MyList.MaxFiles = 0;
        MyList.CurrentFiles = 0;
        MyList.pList = NULL;
    };
    ~PtcFileManager()
    {
        TqInt i;
        for ( i = 0; i < MyList.CurrentFiles; i++)
        {
            if (MyList.pList[i].CloudFile)
            {
                if (MyList.pList[i].ReadWrite)
                    PtcFinishPointCloudFile(MyList.pList[i].CloudFile);
                else
                    PtcClosePointCloudFile( MyList.pList[i].CloudFile);
            }
        }
        MyList.CurrentFiles = 0;
        MyList.MaxFiles = 0;
        free(MyList.pList);
        MyList.pList = 0;
    };
} ;

PtcFileManager PtcManager;


//------------------------------------------------------------------------------
/** \brief Shadeops "bake3d" to save any parameter in one pointcloud file refer.
 *  \param ptc the name of the pointcloud file
 *  \param format the format size w,h,ratio 
 *  \param point  the P
 *  \param normat its normal
 *  \result result 0 or 1
 *  \param pShader shaderexecenv
 *  \param cParams number of remaining user parameters.
 *  \param aqParams list of user parameters (to save to ptc)
 */
void	CqShaderExecEnv::SO_bake3d( IqShaderData* ptc,
                                 IqShaderData* format,
                                 IqShaderData* point,
                                 IqShaderData* normal,
                                 IqShaderData* Result,
                                 IqShader* pShader,
                                 TqInt cParams, IqShaderData** apParams )
{
    bool __fVarying;
    TqUint __iGrid;

    __fVarying=(point)->Class()==class_varying;
    __fVarying=(normal)->Class()==class_varying||__fVarying;
    __fVarying=(Result)->Class()==class_varying||__fVarying;

    __iGrid = 0;
    const CqBitVector& RS = RunningState();
    CqString _aq_ptc;
    (ptc)->GetString(_aq_ptc,__iGrid);
    PtcPointCloud MyCloudWrite = PtcManager.FindCloudWrite(_aq_ptc.c_str());
    CqBake3DOptions     sampleOpts;
    CqBake3DOptionsExtractor optExtractor(apParams, cParams, sampleOpts);

    TqInt i, size = 0;
    size = sampleOpts.m_MaxSize;
    TqFloat *userdata = (TqFloat*) new TqFloat[ size];
    do
    {
        if(RS.Value( __iGrid ) )
        {
            CqString _aq_format;
            (format)->GetString(_aq_format,__iGrid);
            CqVector3D _aq_point;
            (point)->GetPoint(_aq_point,__iGrid);
            CqVector3D _aq_normal;
            (normal)->GetNormal(_aq_normal,__iGrid);

            if (!MyCloudWrite && sampleOpts.m_Count < CqBake3DOptionsMaxParams)
            {
                MyCloudWrite = PtcCreatePointCloudFile(_aq_ptc.c_str(), sampleOpts.m_Count, (const char**)sampleOpts.m_VarTypes, (const char**) sampleOpts.m_VarNames, sampleOpts.m_Eye2NDC, sampleOpts.m_Eye2World, sampleOpts.m_Format);
                PtcManager.SaveCloudWrite(_aq_ptc.c_str(), MyCloudWrite);
            }

            TqFloat pointf[3];
            pointf[0] = _aq_point[0];
            pointf[1] = _aq_point[1];
            pointf[2] = _aq_point[2];

            TqFloat normalf[3];
            normalf[0] = _aq_normal[0];
            normalf[1] = _aq_normal[1];
            normalf[2] = _aq_normal[2];

            TqInt where = 0;

            // Convert all user paramters and assemble them together in userdata array of floats.
            for (i=0; i< sampleOpts.m_Count; i++)
            {
                if ( strcmp(sampleOpts.m_VarTypes[i], "float") == 0 ||
                        strcmp(sampleOpts.m_VarTypes[i], "integer") == 0 ||
                        strcmp(sampleOpts.m_VarTypes[i], "bool") == 0 )
                {
                    TqFloat f;
                    sampleOpts.m_UserData[ i ]->GetFloat( f, __iGrid);
                    userdata[where] = f;
                    where ++;
                } else if ( strcmp(sampleOpts.m_VarTypes[i], "vector") == 0)
                {
                    CqVector3D v;
                    sampleOpts.m_UserData[ i ]->GetVector( v, __iGrid);
                    for (TqInt j=0; j < 3; j++)
                    {
                        userdata[where + j] = v[j];
                    }
                    where +=3;
                } else if ( strcmp(sampleOpts.m_VarTypes[i], "color") == 0)
                {
                    CqColor c;
                    sampleOpts.m_UserData[ i ]->GetColor( c, __iGrid);
                    for (TqInt j=0; j < 3; j++)
                    {
                        userdata[where + j] = c[j];
                    }
                    where +=3;
                } else if ( strcmp(sampleOpts.m_VarTypes[i], "point") == 0)
                {
                    CqVector3D p;
                    sampleOpts.m_UserData[ i ]->GetPoint( p, __iGrid);
                    for (TqInt j=0; j < 3; j++)
                    {
                        userdata[where + j] = p[j];
                    }
                    where +=3;
                } else if ( strcmp(sampleOpts.m_VarTypes[i], "normal") == 0)
                {
                    CqVector3D n;
                    sampleOpts.m_UserData[ i ]->GetNormal( n, __iGrid);
                    for (TqInt j=0; j < 3; j++)
                    {
                        userdata[where + j] = n[j];
                    }
                    where +=3;
                } else if ( strcmp(sampleOpts.m_VarTypes[i], "matrix") == 0)
                {
                    CqMatrix m;
                    sampleOpts.m_UserData[ i ]->GetMatrix( m, __iGrid);
                    for (TqInt j=0; j < 16; j++)
                    {
                        userdata[where + j] = m.pElements()[j];
                    }

                    where += 16;
                }
            }

            // Save all the information with PtcWriteDataPoint()
            TqInt okay = PtcWriteDataPoint(MyCloudWrite, pointf, normalf, sampleOpts.m_Radius, userdata);

            TqFloat fRes = okay;
            (Result)->SetFloat(fRes,__iGrid);
        }
    }
    while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
    delete [] userdata;
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
}

//---------------------------------------------------------------------

} // namespace Aqsis

