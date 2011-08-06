// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>

#include <aqsis/ri/ri.h>
#include <aqsis/math/math.h>
#include <aqsis/math/matrix.h>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>

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
	for(PrimVars::iterator var = primVars.begin(),
			end = primVars.end(); var != end; ++var)
	{
		FloatArray& value = *var->value;
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

/** A structure holding all parameters to the hair procedural.
 *
 * The constructor provides for parsing of key-value pairs for parameters held
 * in the struct.
 */
struct HairParams
{
	int numHairs;
	float hairLength;
	float hairWidth;
	std::string emitterFileName;
	std::string hairFileName;
	Aqsis::CqMatrix emitterToHairMatrix;
	HairModifiers hairModifiers;
	bool verbose;

	/** Parse hair parameters from the given input string.
	 *
	 * The parameter string should have the form
	 *
	 * name1=value1; name2=value2; ...
	 *
	 * whitespace (including newlines) is not significant.  Semicolons are
	 * required to separate each (name,value) pair.  Values are specified in
	 * a form compatible with operator>>.
	 */
	HairParams(const std::string& paramString)
		: numHairs(1000),
		hairLength(0.1),
		hairWidth(0.01),
		emitterFileName(),
		hairFileName(),
		emitterToHairMatrix(),
		hairModifiers(),
		verbose(false)
	{
		typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
		Tokenizer tokens(paramString, boost::char_separator<char>(";"));
		for(Tokenizer::iterator tok = tokens.begin(); tok != tokens.end(); ++tok)
		{
			std::string keyValuePair = *tok;
			std::string::size_type pos = keyValuePair.find("=");
			if(pos == std::string::npos)
			{
				boost::algorithm::trim_if(keyValuePair,
						boost::algorithm::is_any_of(" \n\r\t"));
				if(keyValuePair != "")
					g_errStream << "hairgen: value not found in parameter \""
						<< keyValuePair << "\"\n";
				continue;
			}
			std::string name = keyValuePair.substr(0,pos);
			boost::algorithm::trim_if(name, boost::algorithm::is_any_of(" \n\r\t"));
			if(name == "")
				continue;
			std::istringstream valueStream(keyValuePair.substr(pos+1));

			// Read in value for the provided name.
			if(name == "num_hairs")
			{
				valueStream >> numHairs;
			}
			else if(name == "emitter_file_name")
			{
				valueStream >> emitterFileName;
			}
			else if(name == "hair_file_name")
			{
				valueStream >> hairFileName;
			}
			else if(name == "emitter_to_hair_matrix")
			{
				float mInit[16];
				int i = 0;
				while(i < 16 && valueStream >> mInit[i])
					++i;
				if(i == 16)
					emitterToHairMatrix = Aqsis::CqMatrix(mInit);
			}
			else if(name == "verbose")
			{
				valueStream >> std::boolalpha >> verbose;
			}
			else
			{
				if(!hairModifiers.parseParam(name, valueStream))
				{
					g_errStream << "hairgen: WARNING: procedural parameter \""
						<< name << "\" not recognized\n";
				}
			}

			// Warning if value read failed.
			if(!valueStream)
			{
				g_errStream << "hairgen: WARNING: could not parse parameter \""
					<< name << "\"\n";
			}
		}
	}
};

//------------------------------------------------------------------------------
/** Holder for procedural data related to hair generation.
 */
class HairProcedural
{
	private:
		boost::shared_ptr<EmitterMesh> m_emitter;
		boost::shared_ptr<ParentHairs> m_parentHairs;
		HairParams m_params;

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
			const FloatArray& P_emit = curveVars.find("P_emit");
			const FloatArray& Ng_emit = curveVars.find("Ng_emit");
			const FloatArray* Nh_emit = curveVars.findPtr("Nh_emit");
			int numP = P_emit.size()*2;

			// Add "P" to primvar list
			curveVars.append(Aqsis::CqPrimvarToken(Aqsis::class_vertex,
						Aqsis::type_point, 1, "P"));
			FloatArray& P = *curveVars.back().value;
			P.resize(numP, 0);
			for(int i = 0, PmeshSize = P_emit.size(); i+2 < PmeshSize; i += 3)
			{
				P[2*i] = P_emit[i];
				P[2*i+1] = P_emit[i+1];
				P[2*i+2] = P_emit[i+2];
				Vec3 jitterN = 0.1*(Vec3(uRand(), uRand(), uRand()) - 0.5);
				if(Nh_emit)
				{
					const FloatArray& Nh = *Nh_emit;
					float lengthMult = m_params.hairLength / std::sqrt(Nh[i]*Nh[i]
							+ Nh[i+1]*Nh[i+1] + Nh[i+2]*Nh[i+2]);
					P[2*i+3] = P_emit[i] + lengthMult*Nh[i] + jitterN.x();
					P[2*i+4] = P_emit[i+1] + lengthMult*Nh[i+1] + jitterN.y();
					P[2*i+5] = P_emit[i+2] + lengthMult*Nh[i+2] + jitterN.z();
				}
				else
				{
					P[2*i+3] = P_emit[i] + m_params.hairLength*Ng_emit[0] + jitterN.x();
					P[2*i+4] = P_emit[i+1] + m_params.hairLength*Ng_emit[1] + jitterN.y();
					P[2*i+5] = P_emit[i+2] + m_params.hairLength*Ng_emit[2] + jitterN.z();
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
			m_params(initialdata)
		{
			HairgenApiServices apiServices(m_emitter, m_params.numHairs,
										   m_parentHairs,
										   m_params.hairModifiers);
			std::ifstream emitterStream(m_params.emitterFileName.c_str());
			if(emitterStream)
				apiServices.parseRib(emitterStream,
									 m_params.emitterFileName.c_str());
			if(!m_emitter)
				throw std::runtime_error("Could not find PointsPolygons "
										 "emitter mesh in file");

			if(m_params.hairFileName != m_params.emitterFileName)
			{
				std::ifstream curveStream(m_params.hairFileName.c_str());
				if(curveStream)
					apiServices.parseRib(curveStream,
										 m_params.hairFileName.c_str());
			}
			if(!m_parentHairs)
				throw std::runtime_error("Could not find parent Curves in file");

			if(m_params.verbose)
			{
				std::cout << "hairgen: Created hair procedural with "
					<< m_params.numHairs << " hairs\n";
			}
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
			if(m_params.verbose)
				std::cout << "hairgen: Starting hair generation\n";
			for(int faceNum = 0, numFaces = m_emitter->numFaces();
					faceNum < numFaces; ++faceNum)
			{
				boost::shared_ptr<PrimVars> faceVars =
					m_emitter->particlesOnFace(faceNum);
				if(!faceVars)
					continue;

				transformPrimVars(*faceVars, m_params.emitterToHairMatrix);

				m_parentHairs->childInterp(*faceVars);

				// Alternative - generate hairs directly without parent hairs.
//				linearHairsFromPoints(*faceVars);
//				// Add "constantwidth" to primvar list
//				faceVars->append(Aqsis::CqPrimvarToken(Aqsis::class_constant,
//							Aqsis::type_float, 1, "constantwidth"),
//						FloatArray(1, m_hairWidth) );

				// Convert all inherited mesh primvars into a rendeman parameter list.
				ParamList pList(*faceVars);

				int numCurves = faceVars->find("P_emit").size()/3;
				std::vector<int> nVerts(numCurves,
										m_parentHairs->vertsPerCurve());

				RtToken linearStr = const_cast<RtToken>(
						m_parentHairs->linear() ? "linear" : "cubic");

				RiCurvesV(linearStr, numCurves, &nVerts[0],
						  (char*)"nonperiodic",
						  pList.count(), pList.tokens(), pList.values());
			}
			if(m_params.verbose)
				std::cout << "hairgen: Hair generation done.\n";
		}
};


//------------------------------------------------------------------------------
// RiProcDynamicLoad plugin interface functions.

extern "C" AQSIS_EXPORT RtPointer ConvertParameters(char* initialdata)
{
	HairProcedural* params = 0;
	try
	{
		params = new HairProcedural(initialdata);
	}
	catch(std::runtime_error& e)
	{
		g_errStream << "hairgen: ERROR: " << e.what() << "\n";
	}

	return reinterpret_cast<RtPointer>(params);
}

extern "C" AQSIS_EXPORT void Subdivide(RtPointer blinddata, RtFloat detailsize)
{
	const HairProcedural* p = reinterpret_cast<HairProcedural*>(blinddata);

	if(p)
		p->subdivide();
}

extern "C" AQSIS_EXPORT void Free(RtPointer blinddata)
{
	delete reinterpret_cast<HairProcedural*>(blinddata);
}

