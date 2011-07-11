// Aqsis
// Copyright (C) 2006, Paul C. Gregory
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
    \brief Implements Blobby polygonizer
    \author Romain Behar (romainbehar@yahoo.com)
    \author Michel Joron (rearrange a bit)
*/
/*    References:
*          K-3D
*
*/

#include "blobby.h"

#include <cstring>
#include <math.h>
#include <vector>
#include <list>
#include <limits>

#include <aqsis/util/file.h>
#include "itexturemap_old.h"
#include "marchingcubes.h"
#include <aqsis/math/matrix.h>
#include <aqsis/util/plugins.h>
#include <aqsis/ri/ri.h>
#include <aqsis/math/vector4d.h>

#if _MSC_VER
#pragma warning(disable:4786)	// hide stl warnings (VS6)
#endif
#if _WIN32
#include <windows.h>
#endif

namespace Aqsis {

#define ZCLAMP  1e-6
#define OPTIMUM_GRID_SIZE 15

typedef struct {
	int BlobbyId;
	int OpId;
} TqState;

typedef void *TqImplicitBound(TqState *, float *, int, int *, int , float *, int, char **);
typedef void *TqImplicitValue(TqState *, float *, float *, int, int *, int, float *, int, char **);
typedef void *TqImplicitRange(TqState *, float *, float *, int, int *, int, float *, int, char **);
typedef void *TqImplicitFree (TqState *);

static CqSimplePlugin DBO;
static TqImplicitBound *pImplicitBound = NULL;
static TqImplicitValue *pImplicitValue = NULL;
static TqImplicitFree  *pImplicitFree  = NULL;
static TqImplicitRange *pImplicitRange = NULL;
static void *DBO_handle = NULL;


//---------------------------------------------------------------------
/**
* When 0<=r<=2, bump(r) is the polynomial of lowest degree with
*      bump(0)=0
*      bump'(0)=0
*      bump"(0)=0
*      bump(1)=1
*      bump(2)=0
*      bump'(2)=0
*      bump"(2)=0
*/
static TqFloat bump(TqFloat r)
{
	if(r<=0.0f || r>=2.0f)
		return 0.0f;
	return (((6.-r)*r-12.)*r+8.)*r*r*r;
}

//---------------------------------------------------------------------
/**
* When 0<=r<=1, ease(r) is the polynomial of lowest degree with
*      ease(0)=0
*      ease'(0)=0
*      ease(1)=1
*      ease'(1)=0
*/
static TqFloat ease(TqFloat r)
{
	if(r<=0.0f)
		return 0.0f;
	if(r>=1.0f)
		return 1.0f;
	return r*r*(3.0f-2.0f*r);
}

//---------------------------------------------------------------------
/** lowest function for Plane opcode.
 */
static TqFloat repulsion(TqFloat z, TqFloat A, TqFloat B, TqFloat C, TqFloat D)
{
	if(z>=A)
		return 0.0f;
	if(z<=ZCLAMP)
		z=ZCLAMP;
	return (D*bump(z/C)-B/z)*(1.0f-ease(z/A));
}

/** \fn CqVector3D nearest_segment_point(const CqVector3D& Point, const CqVector3D& S1, const CqVector3D& S2)
    \brief From a given 3D point, return a segment's closest point.
    \param Point The point to check the distance from.
    \param S1 The first end of the segment.
    \param S2 The second end of the segment.
    \return One of the points belonging to [S1;S2] segment, the nearest to Point.
 */
static CqVector3D nearest_segment_point( const CqVector3D& Point, const CqVector3D& S1,
        const CqVector3D& S2 )
{
	const CqVector3D vector = S2 - S1;
	const CqVector3D w = Point - S1;

	const TqFloat c1 = w * vector;
	if(c1 <= 0)
		return S1;

	const TqFloat c2 = vector * vector;
	if(c2 <= c1)
		return S2;

	const TqFloat b = c1 / c2;
	const CqVector3D middlepoint = S1 + b * vector;
	return middlepoint;
}

/// Blobby virtual machine assembler
/** This class takes RiBlobby parameters as input, and returns:
    - a program that computes the associated implicit values
    - its bounding-box
 
   \param Instructions Reference to the output instruction stack forming the program that computes implicit values.
   \param BBox Reference to the Blobby's bounding-box.
 */
class blobby_vm_assembler
{
	public:
		blobby_vm_assembler(TqInt nleaf, TqInt ncode, TqInt* code, TqInt nfloats, TqFloat* floats, TqInt nstrings, char** strings, CqBlobby::instructions_t& Instructions, CqBound& BBox) :
				m_code(code),
				m_floats(floats),
				m_strings(strings),
				m_instructions(Instructions),
				m_bbox(BBox),
				m_has_bounding_box(false)
		{


			// Decode blobby instructions and store them onto a stack
			for( TqInt c = 0; c < ncode;)
			{
				switch(code[c++])
				{
						case 0:
						{
							opcodes.push_back( opcode( CqBlobby::ADD, c ) );
							const TqInt n_ops = code[c];
							c += n_ops + 1;
						}
						break;
						case 1:
						{
							opcodes.push_back( opcode( CqBlobby::MULTIPLY, c ) );
							const TqInt n_ops = code[c];
							c += n_ops + 1;
						}
						break;
						case 2:
						{
							opcodes.push_back( opcode( CqBlobby::MAX, c ) );
							const TqInt n_ops = code[c];
							c += n_ops + 1;
						}
						break;
						case 3:
						{
							opcodes.push_back( opcode( CqBlobby::MIN, c ) );
							const TqInt n_ops = code[c];
							c += n_ops + 1;
						}
						break;
						case 4:
						{
							opcodes.push_back( opcode( CqBlobby::DIVIDE, c ) );
							c += 2;
						}
						break;
						case 5:
						{
							opcodes.push_back( opcode( CqBlobby::SUBTRACT, c ) );
							c += 2;
						}
						break;
						case 6:
						{
							opcodes.push_back( opcode( CqBlobby::NEGATE, c++ ) );
						}
						break;
						case 7:
						{
							Aqsis::log() << warning << "Unhandled Blobby IDEMPOTENTATE." << std::endl;
							c++;
						}
						break;

						case 9000:
						{
							STATS_INC( GPR_blobbies );
							opcodes.push_back( opcode( CqBlobby::AIR, c ) );
							const TqInt n_ops = code[c];

							c += n_ops + 1;
							Aqsis::log() << info << "Blobby Air with " << n_ops << " parameters" << std::endl;
						}
						break;
						case 1000:
						{
							opcodes.push_back( opcode( CqBlobby::CONSTANT, c++ ) );
							STATS_INC( GPR_blobbies );
							//Aqsis::log() << warning << "Unhandled Blobby CONSTANT." << std::endl;
						}
						break;
						case 1001:
						{
							opcodes.push_back( opcode( CqBlobby::ELLIPSOID, c++ ) );
							STATS_INC( GPR_blobbies );
						}
						break;
						case 1002:
						{
							opcodes.push_back( opcode( CqBlobby::SEGMENT, c++ ) );
							STATS_INC( GPR_blobbies );
						}
						break;
						case 1003:
						{
							opcodes.push_back( opcode( CqBlobby::PLANE, c++ ) );
							c ++;
							STATS_INC( GPR_blobbies );
						}
						break;

						default:
						{
							Aqsis::log() << warning << "Unknown Blobby Opcode #" << code[c-1] << std::endl;

							// Try to skip the unknown opcode
							const TqInt n_ops = code[c];
							c += n_ops + 1;
						}
						break;
				}
			}

			build_program(opcodes.back());
		}

	private:
		struct opcode
		{
			opcode( CqBlobby::EqOpcodeName Name, const TqInt Index = 0 ) :
					name( Name ), index( Index )
			{}

			CqBlobby::EqOpcodeName name;
			TqInt index;
		};

		std::vector<opcode> opcodes;

		/// Encapsulate a segment into the bounding-box
		void grow_bound( const CqVector3D& Start, const CqVector3D& End, const TqFloat radius, const CqMatrix& transformation)
		{
			const TqFloat r = radius * 0.72;
			CqBound start_box( Start.x() - r, Start.y() - r, Start.z() - r, Start.x() + r, Start.y() + r, Start.z() + r );
			start_box.Transform( transformation );

			CqBound end_box( End.x() - r, End.y() - r, End.z() - r, End.x() + r, End.y() + r, End.z() + r );
			end_box.Transform( transformation );

			start_box.Encapsulate( &end_box );

			encapsulate( start_box );
		}

		/// Encapsulate an ellipsoid into the bounding-box
		void grow_bound( const CqMatrix& transformation, const TqFloat radius )
		{
			const TqFloat r = radius * 0.72;
			CqBound unit_box( -r, -r, -r, r, r, r );
			unit_box.Transform( transformation );

			encapsulate( unit_box );
		}

		void encapsulate( const CqBound& Bound)
		{
			if(m_has_bounding_box)
			{
				m_bbox.Encapsulate(&Bound);
				return;
			}

			m_bbox = Bound;
			m_has_bounding_box = true;
		}


		/// Build implicit value evaluation program: store opcodes and parameters in execution order
		void build_program( opcode op )
		{
			switch( op.name )
			{
					case CqBlobby::CONSTANT:
					{
						m_instructions.push_back(CqBlobby::instruction(CqBlobby::CONSTANT));
						m_instructions.push_back(CqBlobby::instruction(m_floats[m_code[op.index]]));
					}
					break;

					case CqBlobby::NEGATE:
					{
					Aqsis::log() << warning << "RiBlobby's Negate operator not supported." << std::endl;
					}
					break;

					case CqBlobby::IDEMPOTENTATE:
					{
					Aqsis::log() << warning << "RiBlobby's Idempotate operator not supported." << std::endl;
					}
					break;

					case CqBlobby::AIR:
					{
						/*
						A dynamic blob op can be used like any other primitive blob in a Blobby object.  DBOs have two required parameters and can have an arbitrary number of float, string, or integer parameters that arepassed to the DBO functions.

						9000 2  nameix transformix
						9000 4 nameix transformix nfloat floatix
						9000 6 nameix transformix nfloat floatix nstring stringix 
						9000 (7+nint) nameix transformix nfloat floatix nstring stringix nintint_1...int_nint 

						nameix is an index into the string array for the name of the DBO.  
						AIR will search the proceduresearch path for a DLL or shared object with the corresponding name. 
						transformix is an index into the float array for a matrix giving the blob-to-object space transformation.
						floatix (if present) is the index in the float array of the first of the nfloat float parameters. 
						stringix (if present) is the index in the string array of the first of the nstring string parameters.
						*/
						m_instructions.push_back(CqBlobby::instruction(CqBlobby::AIR));
						m_instructions.push_back(CqBlobby::instruction(op.index)); // idx to count

						TqInt f =  (TqInt) m_code[op.index+2]; //idx for the inverse matrix;

						CqMatrix transformation(
						    m_floats[f], m_floats[f+1], m_floats[f+2], m_floats[f+3],
						    m_floats[f+4], m_floats[f+5], m_floats[f+6], m_floats[f+7],
						    m_floats[f+8], m_floats[f+9], m_floats[f+10], m_floats[f+11],
						    m_floats[f+12], m_floats[f+13], m_floats[f+14], m_floats[f+15]);

						TqFloat bounds[6];
						TqInt g =  (TqInt) m_code[op.index+3];
						TqInt h =  (TqInt) m_code[op.index+4];
						TqState s;

						// Load the DBO plugin
						if (!DBO_handle)
						{
							std::string dboName = std::string(m_strings[m_code[op.index+1]])
										+ SHARED_LIBRARY_SUFFIX;
							try
							{
								CqString fullName = native(QGetRenderContext()->poptCurrent()
														   ->findRiFile(dboName, "procedural"));
								DBO_handle = DBO.SimpleDLOpen(&fullName);
							}
							catch(XqInvalidFile& /*e*/)
							{
								Aqsis::log() << error
									<< "Could not find dynamic blob object \""
									<< dboName << "\"\n";
							}
						}

						// Attach each API DBO functions
						// Even if we attach them all only ImplicitBound, ImplicitValue, ImplicitFree will be used.
						if (DBO_handle)
						{
							if (!pImplicitBound)
							{
								CqString implicitbound("ImplicitBound");
								pImplicitBound = (TqImplicitBound *) DBO.SimpleDLSym(DBO_handle, &implicitbound);
							}
							if (!pImplicitValue)
							{
								CqString implicitvalue("ImplicitValue");
								pImplicitValue = (TqImplicitValue *) DBO.SimpleDLSym(DBO_handle, &implicitvalue );
							}
							if (!pImplicitFree)
							{
								CqString implicitfree("ImplicitFree");
								pImplicitFree = (TqImplicitFree *) DBO.SimpleDLSym(DBO_handle, &implicitfree);
							}
							if (!pImplicitRange)
							{
								CqString implicitrange("ImplicitRange");
								pImplicitRange = (TqImplicitRange *) DBO.SimpleDLSym(DBO_handle, &implicitrange);
							}
						} else 
						{
							Aqsis::log() << warning << "Cannot load dbo plugin: " << m_strings[m_code[op.index+1]] << std::endl;
						}

						bounds[0] = bounds[1] = bounds[2] = bounds[3] = bounds[4] = bounds[5] = 0.0f;

						if (pImplicitBound)
						{
							Aqsis::log() << info << "Using the dbo plugin ..." << std::endl;
							(*pImplicitBound)(&s, bounds,
							                  0, m_code,
							                  g, &m_floats[h],
							                  0, 0);
						}

						CqVector3D mn = CqVector3D(bounds[0], bounds[2], bounds[4]);
						CqVector3D mx = CqVector3D(bounds[1], bounds[3], bounds[5]);
						CqVector3D mid = (mn + mx) / 2.0; 
						grow_bound( mn,
						            mx,
						            1.0,
						            transformation);

						// Push the inverse matrix
						m_instructions.push_back(CqBlobby::instruction(transformation.Inverse()));

						// Push the center of this blobby according to its bbox
						m_instructions.push_back(CqBlobby::instruction(mid));
						m_instructions.push_back(CqBlobby::instruction(mx));
						m_instructions.push_back(CqBlobby::instruction(mn));


					}
					break;

					case CqBlobby::PLANE:
					{
						m_instructions.push_back(CqBlobby::instruction(CqBlobby::PLANE));
						m_instructions.push_back(CqBlobby::instruction((TqFloat) m_code[op.index]));
						m_instructions.push_back(CqBlobby::instruction((TqFloat) m_code[op.index + 1]));
						Aqsis::log() << info << "id1 " << m_code[op.index] << " id2 " << m_code[op.index + 1] << std::endl;
					}
					break;

					case CqBlobby::ELLIPSOID:
					{
						TqInt f = m_code[op.index];
						CqMatrix transformation(
						    m_floats[f], m_floats[f+1], m_floats[f+2], m_floats[f+3],
						    m_floats[f+4], m_floats[f+5], m_floats[f+6], m_floats[f+7],
						    m_floats[f+8], m_floats[f+9], m_floats[f+10], m_floats[f+11],
						    m_floats[f+12], m_floats[f+13], m_floats[f+14], m_floats[f+15]);

						grow_bound(transformation, 1.0);

						m_instructions.push_back(CqBlobby::instruction(CqBlobby::ELLIPSOID));
						m_instructions.push_back(CqBlobby::instruction(transformation.Inverse()));
					}
					break;

					case CqBlobby::SEGMENT:
					{
						TqInt f = m_code[op.index];
						CqVector3D start(m_floats[f], m_floats[f+1], m_floats[f+2]);
						f += 3;
						CqVector3D end(m_floats[f], m_floats[f+1], m_floats[f+2]);
						f += 3;
						TqFloat radius = m_floats[f];
						f++;
						CqMatrix transformation(
						    m_floats[f], m_floats[f+1], m_floats[f+2], m_floats[f+3],
						    m_floats[f+4], m_floats[f+5], m_floats[f+6], m_floats[f+7],
						    m_floats[f+8], m_floats[f+9], m_floats[f+10], m_floats[f+11],
						    m_floats[f+12], m_floats[f+13], m_floats[f+14], m_floats[f+15]);

						grow_bound(start, end, radius, transformation);

						m_instructions.push_back(CqBlobby::instruction(CqBlobby::SEGMENT));
						m_instructions.push_back(CqBlobby::instruction(transformation));
						m_instructions.push_back(CqBlobby::instruction(start));
						m_instructions.push_back(CqBlobby::instruction(end));
						m_instructions.push_back(CqBlobby::instruction(radius));
					}
					break;

					case CqBlobby::ADD:
					case CqBlobby::MULTIPLY:
					case CqBlobby::MIN:
					case CqBlobby::MAX:
					{
						TqInt operands = op.index;
						TqInt n = m_code[operands];
						for(TqInt i = 1; i <= n; ++i)
						{
							build_program(opcodes[m_code[operands + i]]);
						}

						m_instructions.push_back(CqBlobby::instruction(op.name));
						m_instructions.push_back(CqBlobby::instruction(n));
					}
					break;
					case CqBlobby::SUBTRACT:
					case CqBlobby::DIVIDE:
					{
						build_program(opcodes[m_code[op.index]]);
						build_program(opcodes[m_code[op.index + 1]]);

						m_instructions.push_back(CqBlobby::instruction(op.name));
					}
			}
		}

		TqInt  * m_code;
		TqFloat* m_floats;
		char  ** m_strings;
		CqBlobby::instructions_t& m_instructions;
		CqBound& m_bbox;
		bool m_has_bounding_box;
};

//---------------------------------------------------------------------
/** Constructor.
 */
CqBlobby::CqBlobby(TqInt nleaf, TqInt ncode, TqInt* code, TqInt nfloats, TqFloat* floats, TqInt nstrings, char** strings) : m_nleaf(nleaf), m_ncode(ncode), m_code(code), m_nfloats(nfloats), m_floats(floats), m_nstrings(nstrings), m_strings(strings)
{
	blobby_vm_assembler(nleaf, ncode, code, nfloats, floats, nstrings, strings, m_instructions, m_bbox);
}

//---------------------------------------------------------------------
/** Clone in advent of copy an already diced/splitted primitive
 */
CqSurface* CqBlobby::Clone() const
{
	CqBlobby* clone = new CqBlobby( m_nleaf, m_ncode, m_code, m_nfloats, m_floats, m_nstrings, m_strings );

	return ( clone );
}

//---------------------------------------------------------------------
/** Split the primitive and ignore it algother if the prim is behind the Z plane
 */
TqInt CqBlobby::Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
{
	// Get near clipping plane in Z (here, primitives are in camera space)
	//TqFloat z = QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "Clipping" ) [ 0 ];


	return 0;
}

//---------------------------------------------------------------------
/** Return the float value based on each operands. In particular Add will
 *  accumulate the result of each opcode together. Now this function is
 *  particulary important since it is used to weight each operand and deduce
 *  how well will be split the parent blobby' parameters (via ri.cpp).
 *  Not particular intelligent since it is redoing what the next method is
 *  computing however more keen eyes could combine both methods together to
 *  raise the performance significantly.
 */
/** Blobby virtual machine program execution - calculates the value of an implicit surface at a given 3D point */
TqFloat CqBlobby::implicit_value( const CqVector3D& Point, TqInt n, std::vector <TqFloat> &splits )
{
	register TqFloat sum = 0.0f;
	register TqFloat result;
	std::stack<TqFloat> stack;
	TqInt int_index = 0;
	stack.push(0);

	register unsigned long pc;

	for(pc = 0; pc < m_instructions.size(); )
	{
		switch(m_instructions[pc++].opcode)
		{
				case NEGATE:
				case IDEMPOTENTATE:
					break;
				case CONSTANT:
				{
					result = m_instructions[pc++].value;
					sum += result;
					splits[int_index++] = result;
				}
				break;

				case ELLIPSOID:
				{
					const TqFloat r2 = (m_instructions[pc++].get_matrix() * Point).Magnitude2();
					result = r2 <= 1 ? 1 - 3*r2 + 3*r2*r2 - r2*r2*r2 : 0;
					sum += result;
					splits[int_index++] = result;
				}
				break;

				case PLANE:
				{
					TqInt which = (TqInt) m_instructions[pc++].value;
					TqInt n = (TqInt) m_instructions[pc++].value;

					CqString depthname = m_strings[which];
					/** \todo Fix to use the new-style texture maps.  Using
					 * GetOcclusionMap happens to access the old texture
					 * sampling machinary through GetShadowMap()
					 */
					IqTextureMapOld* pMap = QGetRenderContextI() ->GetOcclusionMap( depthname );

					TqFloat A, B, C, D;
					std::valarray<TqFloat> fv;
					TqFloat avg, depth;

					depth = -Point.z();

					fv.resize(1);
					fv[0]= 0.0f;


					A = m_floats[n];
					B = m_floats[n+1];
					C = m_floats[n+2];
					D = m_floats[n+3];

					if ( pMap != 0 && pMap->IsValid() )
					{
						CqVector3D swidth(0.0f);
						CqVector3D twidth(0.0f);
						CqVector3D aq_P = Point;
						pMap->SampleMap( aq_P, swidth, twidth, fv, 0, &avg, &depth );
					}


					result = repulsion(depth, A, B, C, D);
					sum += result;
					splits[int_index++] = result;
				}
				break;

				case AIR:
				{
					/*
					A dynamic blob op can be used like any other primitive blob in a Blobby object.  DBOs have two required parameters and can have an arbitrary number of float, string, or integer parameters that arepassed to the DBO functions.

					9000 2 nameix transformix
					9000 4 nameix transformix nfloat floatix
					9000 6 nameix transformix nfloat floatix nstring stringix 
					9000 (7+nint) nameix transformix nfloat floatix nstring stringix nintint_1...int_nint 

					nameix is an index into the string array for the name of the DBO.  
					AIR will search the proceduresearch path for a DLL or shared object with the corresponding name. 
					transformix is an index into the float array for a matrix giving the blob-to-object space transformation.
					floatix (if present) is the index in the float array of the first of the nfloat float parameters. 
					stringix (if present) is the index in the string array of the first of the nstring string parameters.

					 on the stack you will find

					m_instructions.push_back(CqBlobby::instruction(CqBlobby::AIR));
					m_instructions.push_back(CqBlobby::instruction(op.index)); // idx to Count
					m_instructions.push_back(transformation.Inverse()); // Push the inverse matrix
					m_instructions.push_back(mid); // Push the center  of this blobby according to its bbox
					m_instructions.push_back(mx); // Push the max  of this blobby according to its bbox
					m_instructions.push_back(mn); // Push the min  of this blobby according to its bbox

					*/

					TqInt count, e, f, g, h, i, j;

					e = f = g = h = i = j = 0;
					count = m_instructions[pc++].count;

					if (m_code[count] >= 7)
					{
						e = 7 - m_code[count]; // How many strings
						f = count + 7;
					}
					if (m_code[count] >= 4)
					{
						g = m_code[count + 3]; // How many floats
						h = m_code[count + 4]; // Idx to the floats
					}
					if (m_code[count] >= 6)
					{
						i = m_code[count + 5]; // How many strings
						j = m_code[count + 6]; // Idx to the strings
					}



					TqFloat point[3];
					const CqMatrix transformation = m_instructions[pc++].get_matrix();
					const CqVector3D mid = m_instructions[pc++].get_vector();
					const CqVector3D mx = m_instructions[pc++].get_vector();
					const CqVector3D mn = m_instructions[pc++].get_vector();
					const CqBound bound(mn, mx);

					TqState s;
					CqVector3D tmp = transformation * Point;
					point[0] = tmp.x();
					point[1] = tmp.y();
					point[2] = tmp.z();

					if ((point[2]>= 0.0) && bound.Contains3D(tmp) && pImplicitValue )
					{
						(*pImplicitValue)(&s, &result, point,
							             e, &m_code[f],
							             g, &m_floats[h],
							             i, &m_strings[j]);
						result = 1.0 - result;
					}

					sum += result;
					splits[int_index++] = result;
				}
				break;

				case SEGMENT:
				{
					const CqMatrix m = m_instructions[pc++].get_matrix();
					const CqVector3D start = m_instructions[pc++].get_vector();
					const CqVector3D end = m_instructions[pc++].get_vector();
					const TqFloat radius = m_instructions[pc++].value;

					// Nearest segment point
					const CqVector3D segment_point = nearest_segment_point(Point, start, end);
					// Translation( segment_point ) * Scaling ( radius ) * m
					const CqMatrix transformation = CqMatrix( segment_point ) * CqMatrix( radius, radius, radius ) * m;
					// Distance
					const TqFloat r2 = (transformation.Inverse() * Point).Magnitude2();
					// Value
					const TqFloat result = (r2 <= 1) ? (1 - 3*r2 + 3*r2*r2 - r2*r2*r2) : 0;

					sum += result;
					splits[int_index++] = result;
				}
				break;

				case CqBlobby::SUBTRACT:
				case CqBlobby::DIVIDE:
				case CqBlobby::ADD:
				case CqBlobby::MULTIPLY:
				case CqBlobby::MIN:
				case CqBlobby::MAX:
				break;

		}
		if (int_index >= n)
			break;
	}

	return sum;
}

//---------------------------------------------------------------------
/** Return the float value based on each opcodes.
 *  This is the most important method it is used by MarchingCubes.cpp to
 *  polygonize the primitives
 */
TqFloat CqBlobby::implicit_value( const CqVector3D& Point )
{
	std::stack<TqFloat> stack;
	stack.push(0);
	register TqFloat result;
	register unsigned long pc;

	for(pc = 0; pc < m_instructions.size(); )
	{
		switch(m_instructions[pc++].opcode)
		{
				case NEGATE:
				case IDEMPOTENTATE:
					break;
				case CONSTANT:
				{
					stack.push(m_instructions[pc++].value);
				}
				break;

				case ELLIPSOID:
				{
					const TqFloat r2 = (m_instructions[pc++].get_matrix() * Point).Magnitude2();
					result = r2 <= 1 ? 1 - 3*r2 + 3*r2*r2 - r2*r2*r2 : 0;

					//Aqsis::log() << info << "Ellipsoid: result " << result << std::endl;
					stack.push(result);
				}
				break;

				case PLANE:
				{
					TqInt which = (TqInt) m_instructions[pc++].value;
					TqInt n = (TqInt) m_instructions[pc++].value;

					CqString depthname = m_strings[which];
					/** \todo Fix to use the new-style texture maps.  Using
					 * GetOcclusionMap happens to access the old texture
					 * sampling machinary through GetShadowMap()
					 */
					IqTextureMapOld* pMap = QGetRenderContextI() ->GetOcclusionMap( depthname );

					TqFloat A, B, C, D;
					std::valarray<TqFloat> fv;
					TqFloat avg, depth;
					depth = -Point.z();

					fv.resize(1);
					fv[0]= 0.0f;


					A = m_floats[n];
					B = m_floats[n+1];
					C = m_floats[n+2];
					D = m_floats[n+3];

					if ( pMap != 0 && pMap->IsValid() )
					{
						CqVector3D swidth(0.0f);
						CqVector3D twidth(0.0f);
						CqVector3D aq_P = Point;
						pMap->SampleMap( aq_P, swidth, twidth, fv, 0, &avg, &depth);
					}

					//Aqsis::log() << info << "A " << A << " B " << B << " C " << C << " D " << D << std::endl;
					result = repulsion(depth, A, B, C, D);

					//Aqsis::log() << info << "Plane: result " << result << std::endl;
					stack.push(result);
				}
				break;

				case AIR:
				{
					TqInt count, e, f, g, h, i, j;

					e = f = g = h = i = j = 0;
					count = m_instructions[pc++].count;

					if (m_code[count] >= 7)
					{
						e = 7 - m_code[count]; // How many strings
						f = count + 7;
					}
					if (m_code[count] >= 4)
					{
						g = m_code[count + 3]; // How many floats
						h = m_code[count + 4]; // Idx to the floats
					}
					if (m_code[count] >= 6)
					{
						i = m_code[count + 5]; // How many strings
						j = m_code[count + 6]; // Idx to the strings
					}


					TqFloat point[3];
					const CqMatrix transformation = m_instructions[pc++].get_matrix();
					const CqVector3D mid = m_instructions[pc++].get_vector();
					const CqVector3D mx = m_instructions[pc++].get_vector();
					const CqVector3D mn = m_instructions[pc++].get_vector();
					const CqBound bound(mn, mx);

					TqState s;
					CqVector3D tmp = transformation * Point;
					point[0] = tmp.x();
					point[1] = tmp.y();
					point[2] = tmp.z();

					if ((point[2]>= 0.0) && bound.Contains3D(tmp) && pImplicitValue )
					{
						(*pImplicitValue)(&s, &result, point,
							          e, &m_code[f],
							          g, &m_floats[h],
							         i, &m_strings[j]);
						result = 1.0 - result;
					}

					//Aqsis::log() << info << " Result " << result << std::endl;

					stack.push(result);
				}
				break;

				case SEGMENT:
				{
					const CqMatrix m = m_instructions[pc++].get_matrix();
					const CqVector3D start = m_instructions[pc++].get_vector();
					const CqVector3D end = m_instructions[pc++].get_vector();
					const TqFloat radius = m_instructions[pc++].value;

					// Nearest segment point
					const CqVector3D segment_point = nearest_segment_point(Point, start, end);
					// Translation( segment_point ) * Scaling ( radius ) * m
					const CqMatrix transformation = CqMatrix( segment_point ) * CqMatrix( radius, radius, radius ) * m;
					// Distance
					const TqFloat r2 = (transformation.Inverse() * Point).Magnitude2();

					// Value
					result = (r2 <= 1) ? (1 - 3*r2 + 3*r2*r2 - r2*r2*r2) : 0;

					//Aqsis::log() << info << "Segment: result " << result << std::endl;
					stack.push(result);
				}
				break;

				case SUBTRACT:
				{
					TqFloat a = stack.top();
					stack.pop();
					TqFloat b = stack.top();
					stack.pop();
					//Aqsis::log() << info << "idex a " << a << " idex b " << b << std::endl;
					result = 0.0;
					if (a != 0.0)
						result = b/a;

					stack.push(result);
				}
				break;

				case DIVIDE:
				{
					TqFloat a = stack.top();
					stack.pop();
					TqFloat b = stack.top();
					stack.pop();

					result = b - a;
					stack.push(result);
				}
				break;

				case ADD:
				{
					const TqInt count = m_instructions[pc++].count;
					result = 0.0;
					for(TqInt i = 0; i != count; ++i)
					{
						result += stack.top();
						stack.pop();
					}
					stack.push(result);
				}
				break;

				case MULTIPLY:
				{
					const TqInt count = m_instructions[pc++].count;
					result = stack.top();
					stack.pop();
					for(TqInt i = 1; i != count; ++i)
					{
						result *= stack.top();
						stack.pop();
					}
					stack.push(result);
				}
				break;

				case MIN:
				{
					const TqInt count = m_instructions[pc++].count;
					result = stack.top();
					stack.pop();
					for(TqInt i = 1; i != count; ++i)
					{
						result = min(result, stack.top());
						stack.pop();
					}
					stack.push(result);
				}
				break;
				case MAX:
				{
					const TqInt count = m_instructions[pc++].count;
					result = stack.top();
					stack.pop();
					for(TqInt i = 1; i != count; ++i)
					{
						result = max(result, stack.top());
						stack.pop();
					}
					stack.push(result);
				}
				break;
		}
	}

	return stack.top();
}


/** \fn TqInt polygonize( TqInt& NPoints, TqInt& NPolys, TqInt*& NVertices, TqInt*& Vertices, TqFloat*& Points, TqFloat PixelsWidth, TqFloat PixelsHeight )
    \brief Polygonizes RiBlobby and outputs RiPointsPolygons data.
    \param PixelWidth Blobby's bounding-box width in pixels.
    \param PixelHeight Blobby's bounding-box height in pixels.
    \param NPoints Resulting point count.
    \param NPolys Resulting polygon count (triangles).
    \param NVertices Polygon vertex counts array.
    \param Vertices Polygons array.
    \param Vertices Point Points array.
 */
TqInt CqBlobby::polygonize( TqInt PixelsWidth, TqInt PixelsHeight, TqInt& NPoints, TqInt& NPolys, TqInt*& NVertices, TqInt*& Vertices, TqFloat*& Points )
{
	register TqInt i,j,k;

	// Make sure the blobby is big enough to show
	if(PixelsWidth <= 0 || PixelsHeight <= 0)
		return 0;

	PixelsWidth /= 2;
	PixelsHeight /= 2;


	// Get bounding-box center and sizes
	const CqVector3D center = ( m_bbox.vecMax() + m_bbox.vecMin() ) / 2.0;
	const CqVector3D length = ( m_bbox.vecMax() - m_bbox.vecMin() );

	// Calculate voxel sizes and polygonization resolution
	const TqFloat x_voxel_size = length.x() /  PixelsWidth;
	const TqFloat y_voxel_size = length.y() /  PixelsHeight;
	const TqFloat z_voxel_size = ( x_voxel_size + y_voxel_size ) / 2.0;

	const TqInt x_resolution = PixelsWidth;
	const TqInt y_resolution = PixelsHeight;
	const TqInt z_resolution = static_cast<TqInt>( ceil( length.z() / z_voxel_size) );


	const TqFloat x_start = center.x() - length.x()/2.0;
	const TqFloat y_start = center.y() - length.y()/2.0;
	const TqFloat z_start = center.z() - length.z()/2.0;

	const TqInt div_z = z_resolution/OPTIMUM_GRID_SIZE + 1;
	const TqInt div_y = y_resolution/OPTIMUM_GRID_SIZE + 1;
	const TqInt div_x = x_resolution/OPTIMUM_GRID_SIZE + 1;

	int nverts = 0;
	int ntrigs = 0;
	Vertex* vertices = 0;
	Triangle* triangles = 0;

	Aqsis::log() << info << "We will need to call mc " << div_x  * div_y * div_z << std::endl;
	TqInt cnt = 1;

	for (TqInt k1=0; k1 < div_z; k1 ++)
	{
		for (TqInt y1=0; y1 < div_y; y1++)
		{
			for (TqInt x1=0; x1 < div_x; x1++)
			{

				TqFloat x,y,z;

				z = z_start + (TqFloat) k1 * (TqFloat) OPTIMUM_GRID_SIZE * z_voxel_size;
				bool isrequired = false;

				// Initialize Marching Cubes algorithm
				MarchingCubes mc(OPTIMUM_GRID_SIZE+1, OPTIMUM_GRID_SIZE+1, OPTIMUM_GRID_SIZE+1);

				mc.init_all();

				for( k = 0 ; k < OPTIMUM_GRID_SIZE+1; k++, z += z_voxel_size )
				{
					y = y_start + (TqFloat) y1 * (TqFloat)OPTIMUM_GRID_SIZE * y_voxel_size;
					for( j = 0 ; j < OPTIMUM_GRID_SIZE+1; j++, y += y_voxel_size )
					{

						x = x_start + (TqFloat) x1 * (TqFloat)OPTIMUM_GRID_SIZE * x_voxel_size;
						for( i = 0 ; i < OPTIMUM_GRID_SIZE+1; i++, x += x_voxel_size )
						{
							const TqFloat iv = implicit_value( CqVector3D( x, y, z ) );
							isrequired |= (iv != 0.0);
							//Aqsis::log() << info << iv << std::endl;
							mc.set_data( static_cast<TqFloat>( iv - 0.421875 ), i, j, k );
						}
					}
				}

				z = z_start + (TqFloat) k1 * (TqFloat)OPTIMUM_GRID_SIZE * z_voxel_size;
				y = y_start + (TqFloat) y1 * (TqFloat)OPTIMUM_GRID_SIZE * y_voxel_size;
				x = x_start + (TqFloat) x1 * (TqFloat)OPTIMUM_GRID_SIZE * x_voxel_size;


				if (!isrequired)
				{
					Aqsis::log() << info << "Don't need to call mc " << cnt++ << std::endl;
					continue;
				}

				// Run Marching Cubes
				// when we are sure it is required.


				{
					// Inform the status class how far we have got, and update UI.
					float Complete = ( float ) ( cnt++ );
					Complete /= (div_x  * div_y * div_z);
					Complete *= 100.0f;
					Aqsis::log() << info << "Polygonize a blobby " << Complete << "% complete" << std::endl;
				}

				mc.run() ;

				if ((mc.ntrigs() == 0) || mc.nverts() == 0)
				{
					Aqsis::log() << info << "Don't merge the vertices they are empty " << cnt-1 << std::endl;
					continue;
				}

				TqInt overts = nverts;
				if (nverts == 0)
				{
					nverts = mc.nverts();
					vertices = (Vertex*) malloc(nverts * sizeof(Vertex));

				}
				else
				{
					nverts += mc.nverts();
					vertices = (Vertex*) realloc(vertices, nverts * sizeof(Vertex));

				}

				// Compute vertex positions in the blobbies world (they were returned in grid coordinates)
				for (TqInt tmp = 0; tmp < mc.nverts(); tmp++)
				{
					vertices[overts+tmp].x = x + x_voxel_size * mc.vertices()[tmp].x;
					vertices[overts+tmp].y = y + y_voxel_size * mc.vertices()[tmp].y;
					vertices[overts+tmp].z = z + z_voxel_size * mc.vertices()[tmp].z;
				}

				TqInt otrigs = ntrigs;

				if (ntrigs == 0)
				{
					ntrigs = mc.ntrigs();
					triangles = (Triangle *) malloc(ntrigs * sizeof(Triangle));
					memcpy(triangles, mc.triangles(), ntrigs * sizeof(Triangle));
				}
				else
				{

					ntrigs += mc.ntrigs();
					triangles = (Triangle *) realloc(triangles,  ntrigs * sizeof(Triangle));
					for (TqInt tmp = 0; tmp < mc.ntrigs(); tmp++)
					{
						triangles[otrigs+tmp].v1 = mc.triangles()[tmp].v1 + overts;
						triangles[otrigs+tmp].v2 = mc.triangles()[tmp].v2 + overts;
						triangles[otrigs+tmp].v3 = mc.triangles()[tmp].v3 + overts;
					}
				}



			}
		}
	}


	NPoints = nverts;
	NPolys = ntrigs;
	NVertices = new TqInt[NPolys];
	Vertices = new TqInt[3 * NPolys];
	Points = new TqFloat[3 * NPoints];

	// Set vertex indices
	TqInt* nvert = NVertices;
	TqInt* vert = Vertices;
	for ( i = 0; i < ntrigs; ++i )
	{
		*nvert++ = 3;
		*vert++ = triangles[i].v1;
		*vert++ = triangles[i].v2;
		*vert++ = triangles[i].v3;
	}


	TqFloat* point = Points;
	for ( i = 0; i < nverts; i++ )
	{
		*point++ = vertices[i].x;
		*point++ = vertices[i].y;
		*point++ = vertices[i].z;
	}

	free(vertices);
	free(triangles);

	// Cleanup the DBO i/f
	if (DBO_handle)
	{
		TqState s;
		if (pImplicitFree)
			(*pImplicitFree)(&s);
		pImplicitBound = NULL;
		pImplicitValue = NULL;
		pImplicitFree  = NULL;
		pImplicitRange = NULL;
		DBO.SimpleDLClose(DBO_handle);
		DBO_handle = NULL;
	}
	return div_x * div_y * div_z;
}


} // namespace Aqsis
//---------------------------------------------------------------------


