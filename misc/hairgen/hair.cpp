// hairgen procedural
// Copyright (C) 2008 Christopher J. Foster [chris42f (at) gmail (d0t) com]
//
// This software is licensed under the GPLv2 - see the file COPYING for details.

#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>

#include <aqsis/ri.h>
#include <aqsis/aqsismath.h>
#include <aqsis/matrix.h>

// project includes
#include "primvar.h"
#include "emitter.h"
#include "parenthairs.h"
#include "rib.h"


// Global error stream.  Aqsis doesn't seem to deal correctly with stderr, so
// we use cout here to report errors instead.
std::ostream& g_errStream = std::cout;


/** Transform the set of primvars by the given transformation.
 *
 * NOTE: Currently this only transforms primvars of type point, so vectors and
 * normals will probably be incorrect after the transform.
 *
 * \param primVars - primitive variables to transform
 * \param pointTrans - transformation to be applied to primvars of type "point"
 */
void transformPrimVars(PrimVars& primVars, const Aqsis::CqMatrix& pointTrans)
{
	for(PrimVars::const_iterator var = primVars.begin(),
			end = primVars.end(); var != end; ++var)
	{
		Aqsis::TqRiFloatArray& value = *var->value;
		switch(var->token.type())
		{
			case Aqsis::type_point:
				for(int i = 0, numVec = value.size()/3; i < numVec; ++i)
				{
					Vec3 v(&value[i*3]);
					v = pointTrans*v;
					value[i*3] = v.x();
					value[i*3+1] = v.y();
					value[i*3+2] = v.z();
				}
				break;
			default:
				continue;
		}
	}
}


//------------------------------------------------------------------------------
/** Holder for procedural data related to hair generation.
 */
class HairProcedural
{
	private:
		boost::shared_ptr<EmitterMesh> m_emitter;
		boost::shared_ptr<ParentHairs> m_parentHairs;
		int m_numChildren;
		float m_hairLength;
		float m_hairWidth;
		Aqsis::CqMatrix m_emitterToHairsTrans;

		/**
		 */
		static void loadEmitter( const std::string& fileName, int numChildren,
				boost::shared_ptr<EmitterMesh>& emitter,
				boost::shared_ptr<ParentHairs>& parentHairs)
		{
			std::ifstream inFile(fileName.c_str());
			if(inFile)
			{
				Aqsis::CqRequestMap requests;
				requests.add("PointsPolygons", new PointsPolygonsRequest(emitter, numChildren));
				requests.add("Curves", new CurvesRequest(parentHairs));
				parseStream(inFile, requests);
			}
		}

		/** Construct a set of linear hairs, given their base points and
		 * normals on the emitting mesh.
		 *
		 * This function simply creates the primvar "P" based on the "P_emit"
		 * base points and normals (either Nh_emit for interpolated hair
		 * normals or Ng_emit for geometric normals).
		 *
		 * This is somewhat of a test function, since it doesn't invoke child
		 * curve interpolation
		 *
		 * \param curveVars - primitive variables for the curves containing
		 * base points and normals from the emitting mesh.  The new variable P
		 * is added to the current set of primvars.
		 */
		void linearHairsFromPoints(PrimVars& curveVars) const
		{
			// construct points
			const Aqsis::TqRiFloatArray& P_emit = curveVars.find("P_emit");
			const Aqsis::TqRiFloatArray& Ng_emit = curveVars.find("Ng_emit");
			const Aqsis::TqRiFloatArray* Nh_emit = curveVars.findPtr("Nh_emit");
			int numP = P_emit.size()*2;

			// Add "P" to primvar list
			curveVars.append(Aqsis::CqPrimvarToken(Aqsis::class_vertex,
						Aqsis::type_point, 1, "P"));
			Aqsis::TqRiFloatArray& P = *curveVars.back().value;
			P.resize(numP, 0);
			for(int i = 0, PmeshSize = P_emit.size(); i+2 < PmeshSize; i += 3)
			{
				P[2*i] = P_emit[i];
				P[2*i+1] = P_emit[i+1];
				P[2*i+2] = P_emit[i+2];
				Vec3 jitterN = 0.1*(Vec3(uRand(), uRand(), uRand()) - 0.5);
				if(Nh_emit)
				{
					const Aqsis::TqRiFloatArray& Nh = *Nh_emit;
					float lengthMult = m_hairLength / std::sqrt(Nh[i]*Nh[i]
							+ Nh[i+1]*Nh[i+1] + Nh[i+2]*Nh[i+2]);
					P[2*i+3] = P_emit[i] + lengthMult*Nh[i] + jitterN.x();
					P[2*i+4] = P_emit[i+1] + lengthMult*Nh[i+1] + jitterN.y();
					P[2*i+5] = P_emit[i+2] + lengthMult*Nh[i+2] + jitterN.z();
				}
				else
				{
					P[2*i+3] = P_emit[i] + m_hairLength*Ng_emit[0] + jitterN.x();
					P[2*i+4] = P_emit[i+1] + m_hairLength*Ng_emit[1] + jitterN.y();
					P[2*i+5] = P_emit[i+2] + m_hairLength*Ng_emit[2] + jitterN.z();
				}
			}
		}

	public:
		/** Construct a hair generation procedural from a config string.
		 *
		 * \param initialdata - initialization string provided to the
		 * ProcDynamicLoad RI call.
		 */
		HairProcedural(const char* initialdata)
			: m_emitter(),
			m_parentHairs(),
			m_numChildren(0),
			m_hairLength(0.1),
			m_hairWidth(0.01),
			m_emitterToHairsTrans()
		{
			std::istringstream in(initialdata);
			in >> m_numChildren;
			if(m_numChildren <= 0)
				throw std::runtime_error("Number of child hairs should be positive");
			//in >> m_hairLength;
			//in >> m_hairWidth;

			std::string emitterFileName;
			in >> emitterFileName;
			std::string curveFileName;
			in >> curveFileName;

			float mInit[16];
			// discard leading '[' character
			for(std::istream::int_type c = in.get(); c != '[' && c != EOF; c = in.get());
			// read in transformation matrix.
			for(int i = 0; i < 16; ++i)
				in >> mInit[i];
			m_emitterToHairsTrans = Aqsis::CqMatrix(mInit);

			std::ifstream emitterStream(emitterFileName.c_str());
			if(emitterStream)
			{
				Aqsis::CqRequestMap requests;
				requests.add("PointsPolygons", new PointsPolygonsRequest(m_emitter, m_numChildren));
				parseStream(emitterStream, requests);
			}
			if(!m_emitter)
				throw std::runtime_error("Could not find PointsPolygons emitter mesh in file");

			std::ifstream curveStream(curveFileName.c_str());
			if(curveStream)
			{
				Aqsis::CqRequestMap requests;
				requests.add("Curves", new CurvesRequest(m_parentHairs));
				parseStream(curveStream, requests);
			}
			if(!m_parentHairs)
				throw std::runtime_error("Could not find parent Curves in file");

			g_errStream << "Created hair procedural with " << m_numChildren << " hairs\n";
		}

		/** Subdivide the hair procedural into a set of RiCurves
		 *
		 * subdivide() generates one set of RiCurves per face of the emitting mesh.
		 * In principle it should generate a new procedural for each face so
		 * that hairs can be culled, but presently it just generates all hairs
		 * upfront.
		 */
		void subdivide() const
		{
			for(int faceNum = 0, numFaces = m_emitter->numFaces();
					faceNum < numFaces; ++faceNum)
			{
				boost::shared_ptr<PrimVars> faceVars =
					m_emitter->particlesOnFace(faceNum);
				if(!faceVars)
					continue;

				transformPrimVars(*faceVars, m_emitterToHairsTrans);

				m_parentHairs->childInterp(*faceVars);

				// Alternative - generate hairs directly without parent hairs.
//				linearHairsFromPoints(*faceVars);
//				// Add "constantwidth" to primvar list
//				faceVars->append(Aqsis::CqPrimvarToken(Aqsis::class_constant,
//							Aqsis::type_float, 1, "constantwidth"),
//						Aqsis::TqRiFloatArray(1, m_hairWidth) );

				// Convert all inherited mesh primvars into a rendeman parameter list.
				ParamList pList(*faceVars);

				int numCurves = faceVars->find("P_emit").size()/3;
				Aqsis::TqRiIntArray nVerts(numCurves, m_parentHairs->vertsPerCurve());

				RtToken linearStr = const_cast<RtToken>(
						m_parentHairs->linear() ? "linear" : "cubic");

				RiCurvesV(linearStr, numCurves, &nVerts[0], "nonperiodic",
						pList.count(), pList.tokens(), pList.values());
			}
		}
};


//------------------------------------------------------------------------------
// RiProcDynamicLoad plugin interface functions.

AQSIS_EXPORT extern "C" RtPointer ConvertParameters(char* initialdata)
{
	HairProcedural* params = 0;
	try
	{
		params = new HairProcedural(initialdata);
	}
	catch(std::runtime_error& e)
	{
		g_errStream << "ERROR hairgen: " << e.what() << "\n";
	}

	return reinterpret_cast<RtPointer>(params);
}

AQSIS_EXPORT extern "C" void Subdivide(RtPointer blinddata, RtFloat detailsize)
{
	const HairProcedural* p = reinterpret_cast<HairProcedural*>(blinddata);

	if(p)
		p->subdivide();
}

AQSIS_EXPORT extern "C" void Free(RtPointer blinddata)
{
	delete reinterpret_cast<HairProcedural*>(blinddata);
}

