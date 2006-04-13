// Aqsis
// Copyright c 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
    \brief Implements RiBlobbyV
    \author Romain Behar (romainbehar@yahoo.com)
    \author Michel Joron (rearrange a bit)
*/
/*    References:
*          k-3d
*
*/

#include <math.h>
#include <vector>
#include <list>
#include <limits>

#include "aqsis.h"
#include "ri.h"
#include "vector4d.h"
#include "matrix.h"
#include "blobby.h"
#include "itexturemap.h"

START_NAMESPACE( Aqsis )

#define ZCLAMP  1e-6

//---------------------------------------------------------------------
/** return the closest 3d Point between a segment defined by S1, S2.
 *  It is either S1 or S2 or a mix between S1, S2
 */
CqVector3D nearest_segment_point(const CqVector3D& Point, const CqVector3D& S1,
                                 const CqVector3D& S2)
{
	const CqVector3D vector = S2 - S1;
	const CqVector3D w = Point - S1;

	const TqFloat c1 = CqVector3D(w * vector).Magnitude2();
	if(c1 <= 0)
		return S1;

	const TqFloat c2 = CqVector3D(vector * vector).Magnitude2();
	if(c2 <= c1)
		return S2;

	const TqFloat b = c1 / c2;
	const CqVector3D middlepoint = S1 + b * vector;
	return middlepoint;
}


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
inline const TqFloat bump(TqFloat r)
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
inline const TqFloat ease(TqFloat r)
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
TqFloat repulsion(TqFloat z, TqFloat A, TqFloat B, TqFloat C, TqFloat D)
{
	if(z>=A)
		return 0.0f;
	if(z<=ZCLAMP)
		z=ZCLAMP;
	return (D*bump(z/C)-B/z)*(1.0f-ease(z/A));
}

//---------------------------------------------------------------------
/** Blobby virtual machine - calculates the value of an implicit surface at a given 3D point
 */
class blobby_vm_assembler
{
	public:
		blobby_vm_assembler(TqInt nleaf, TqInt ncode, TqInt* code, TqInt nfloats, TqFloat* floats, TqInt nstrings, char** strings, CqBlobby::instructions_t& Instructions, CqBound& BBox, CqBlobby::origins_t& Origins) :
			m_code(code),
			m_floats(floats),
			m_instructions(Instructions),
			m_bbox(BBox),
			m_origins(Origins)
		{
			//
			TqFloat absolute = std::numeric_limits<TqFloat>::max();
			m_bbox = CqBound(absolute, absolute, absolute, -absolute, -absolute, -absolute);

			// Build CqOpcode stack
			CqBlobby::EqOpcodeName current_opcode;
			TqInt int_index = 0;
			TqInt float_index = 0;
			for(TqInt c = 0; c < ncode;)
			{
				// Find out opcode name
				switch(code[c++])
				{
						case 0:
							current_opcode = CqBlobby::ADD;
						break;
						case 1:
							current_opcode = CqBlobby::MULTIPLY;
						break;
						case 2:
							current_opcode = CqBlobby::MAX;
						break;
						case 3:
							current_opcode = CqBlobby::MIN;
						break;
						case 4:
							current_opcode = CqBlobby::DIVIDE;
						break;
						case 5:
							current_opcode = CqBlobby::SUBTRACT;
						break;
						case 6:
							current_opcode = CqBlobby::NEGATE;
						break;
						case 7:
							current_opcode = CqBlobby::IDEMPOTENTATE;
						break;

						case 1000:
							current_opcode = CqBlobby::CONSTANT;
						break;
						case 1001:
							current_opcode = CqBlobby::ELLIPSOID;
						break;
						case 1002:
							current_opcode = CqBlobby::SEGMENT;
						break;
						case 1003:
							current_opcode = CqBlobby::PLANE;
						break;
						default:
							Aqsis::log() << warning << "Unknown Opcode for Blobby " << code[c-1] << std::endl;
							current_opcode =  CqBlobby::UNKNOWN;
						break;
				}

				switch(current_opcode)
				{
						case CqBlobby::CONSTANT:
							opcodes.push_back(CqOpcode(current_opcode, code[c++]));
							STATS_INC( GPR_blobbies );
						break;

						case CqBlobby::ELLIPSOID:
							opcodes.push_back(CqOpcode(current_opcode, code[c++]));
							STATS_INC( GPR_blobbies );
						break;

						case CqBlobby::SEGMENT:
							opcodes.push_back(CqOpcode(current_opcode, code[c++]));
							STATS_INC( GPR_blobbies );
						break;

						case CqBlobby::PLANE:
							opcodes.push_back(CqOpcode(current_opcode, code[c], code[c+1]));
							c += 2;
							STATS_INC( GPR_blobbies );
						break;

						case CqBlobby::ADD:
						case CqBlobby::MULTIPLY:
						case CqBlobby::MAX:
						case CqBlobby::MIN:
						{
							TqInt n_ops = code[c];
							opcodes.push_back(CqOpcode(current_opcode, c));
							c += n_ops + 1;
						}
						break;

						case CqBlobby::DIVIDE:
						case CqBlobby::SUBTRACT:
						{
							opcodes.push_back(CqOpcode(current_opcode, code[c], code[c+1]));
							c += 2;
						}
						break;

						case CqBlobby::NEGATE:
							opcodes.push_back(CqOpcode(current_opcode, code[c++]));
						break;

						case CqBlobby::IDEMPOTENTATE:
							c++;
						break;
						default:
							// Hopefull the following number is to skipped the unknown opcode
							TqInt n_ops = code[c];
							c += n_ops + 1;
						break;
				}
			}

			// Prepare stack for implicit function
			build_stack(opcodes.back());
		}

	private:
		class CqOpcode
		{
			public:
				CqOpcode(CqBlobby::EqOpcodeName Name, const TqInt Index1, const TqInt Index2) :
						name(Name), index1(Index1), index2(Index2)
				{}

				CqOpcode(CqBlobby::EqOpcodeName Name, const TqInt Index1) :
						name(Name), index1(Index1), index2(0)
				{}

				CqOpcode(CqBlobby::EqOpcodeName Name) :
						name(Name), index1(0), index2(0)
				{}

				CqBlobby::EqOpcodeName name;
				TqInt index1;
				TqInt index2;
		};

		std::vector<CqOpcode> opcodes;

		void grow_bound(const CqMatrix& transformation, const TqFloat radius = 1.0)
		{
			TqFloat r = radius * 0.5;
			CqBound unit_box(-r, -r, -r, r, r, r);
			unit_box.Transform(transformation);

			m_bbox.Encapsulate( unit_box);
		}

		void grow_bound(const CqVector3D& vector, const TqFloat radius = 1.0)
		{
			m_bbox.Encapsulate(vector);
		}



		void build_stack(CqOpcode op)
		{
			switch(op.name)
			{
					case CqBlobby::CONSTANT:
					break;

					case CqBlobby::PLANE:
					{
						m_instructions.push_back(CqBlobby::instruction(CqBlobby::PLANE));
						m_instructions.push_back(CqBlobby::instruction(op.index1));
						m_instructions.push_back(CqBlobby::instruction(op.index2));
						Aqsis::log() << info << "id1 " << op.index1 << " id2 " << op.index2 << std::endl;
					}
					break;

					case CqBlobby::ELLIPSOID:
					{
						TqInt f = op.index1;
						CqMatrix transformation(
						    m_floats[f], m_floats[f+1], m_floats[f+2], m_floats[f+3],
						    m_floats[f+4], m_floats[f+5], m_floats[f+6], m_floats[f+7],
						    m_floats[f+8], m_floats[f+9], m_floats[f+10], m_floats[f+11],
						    m_floats[f+12], m_floats[f+13], m_floats[f+14], m_floats[f+15]);

						grow_bound(transformation);
						m_origins.push_back(CqVector3D(m_floats[f+12], m_floats[f+13], m_floats[f+14]));

						m_instructions.push_back(CqBlobby::instruction(CqBlobby::ELLIPSOID));
						m_instructions.push_back(CqBlobby::instruction(transformation.Inverse()));
					}
					break;

					case CqBlobby::SEGMENT:
					{
						TqInt f = op.index1;
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


						m_bbox = CqBound(start.x(),start.y(),start.z(), end.x(), end.y(), end.z());
						grow_bound(transformation, radius);


						m_origins.push_back(transformation * (start + end)/2.0);

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
						TqInt operands = op.index1;
						TqInt n = m_code[operands];
						for(TqInt i = 1; i <= n; ++i)
						{
							build_stack(opcodes[m_code[operands + i]]);
						}

						m_instructions.push_back(CqBlobby::instruction(op.name));
						m_instructions.push_back(CqBlobby::instruction(n));
					}
					break;
					case CqBlobby::SUBTRACT:
					case CqBlobby::DIVIDE:
					{
						build_stack(opcodes[op.index1]);
						build_stack(opcodes[op.index2]);

						m_instructions.push_back(CqBlobby::instruction(op.name));
					}
			}
		}

	TqInt* m_code;
	TqFloat* m_floats;
	CqBlobby::instructions_t& m_instructions;
	CqBound& m_bbox;
	CqBlobby::origins_t& m_origins;
};

//---------------------------------------------------------------------
/** Constructor.
 */
CqBlobby::CqBlobby(TqInt nleaf, TqInt ncode, TqInt* code, TqInt nfloats, TqFloat* floats, TqInt nstrings, char** strings) : m_nleaf(nleaf), m_ncode(ncode), m_code(code), m_nfloats(nfloats), m_floats(floats), m_nstrings(nstrings), m_strings(strings)
{
	blobby_vm_assembler(nleaf, ncode, code, nfloats, floats, nstrings, strings, instructions, bbox, origins);
}

//---------------------------------------------------------------------
/** Clone in advent of copy an already diced/splitted primitive
 */
CqSurface* CqBlobby::Clone() const
{
	CqBlobby* clone = new CqBlobby(m_nleaf, m_ncode, m_code, m_nfloats, m_floats, m_nstrings, m_strings);

	return ( clone );
}

//---------------------------------------------------------------------
/** Split the primitive and ignore it algother if the prim is behind the Z plane
 */
TqInt CqBlobby::Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
{
	// Get near clipping plane in Z (here, primitives are in camera space)
	TqFloat z = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Clipping" ) [ 0 ];


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
TqFloat CqBlobby::implicit_value(CqVector3D &Point, TqInt n, std::vector <TqFloat> &splits)
{
	TqFloat result = 0.0f;
	std::stack<TqFloat> stack;
	TqInt int_index = 0;
	stack.push(0);

	for(unsigned long pc = 0; pc < instructions.size(); )
	{
		switch(instructions[pc++].opcode)
		{
				case CqBlobby::CONSTANT:
				splits[int_index++] = instructions[pc++].value;
				break;
				case CqBlobby::ELLIPSOID:
				{
					const TqFloat r2 = (instructions[pc++].get_matrix() * Point).Magnitude2();
					TqFloat result = r2 <= 1 ? 1 - 3*r2 + 3*r2*r2 - r2*r2*r2 : 0;
					splits[int_index++] = result;
				}
				break;
				case CqBlobby::PLANE:
				{
					TqInt which = instructions[pc++].value;
					TqInt n = instructions[pc++].value;

					CqString depthname = m_strings[which];
					IqTextureMap* pMap = QGetRenderContextI() ->GetShadowMap( depthname );

					TqFloat A, B, C, D;
					std::valarray<TqFloat> fv;
					fv.resize(1);
					fv[0]= 0.0f;


					A = m_floats[n];
					B = m_floats[n+1];
					C = m_floats[n+2];
					D = m_floats[n+3];

					if ( pMap != 0 && pMap->IsValid() )
					{
						CqVector3D swidth = 0.0f, twidth = 0.0f;
						CqVector3D _aq_P = Point;
						pMap->SampleMap( _aq_P, swidth, twidth, fv, 0 );
					}
					else
					{
						fv[0] = A + 1.0;
					}

					TqFloat result = repulsion(fv[0], A, B, C, D);
					splits[int_index++] = result;
				}
				break;
				case CqBlobby::SEGMENT:
				{
					const CqMatrix m = instructions[pc++].get_matrix();
					const CqVector3D start = instructions[pc++].get_vector();
					const CqVector3D end = instructions[pc++].get_vector();
					const TqFloat radius = instructions[pc++].value;


					CqVector3D nearpt = nearest_segment_point(Point, start, end);


					CqVector3D probably  =   m * (Point -  nearpt) ;
					TqFloat r2 = probably.Magnitude2();


					TqFloat result = r2 <= 1 ? (1 - 3*r2 + 3*r2*r2 - r2*r2*r2) : 0;
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

	for (TqInt i=0; i < n; i++)
		result +=  splits[i];

	return result;
}

//---------------------------------------------------------------------
/** Return the float value based on each opcodes. 
 *  This is the most important method it is used by jules_bloomenthal.cpp to 
 *  polygonize the primitives
 */
TqFloat CqBlobby::implicit_value(const CqVector3D& Point)
{
	std::stack<TqFloat> stack;
	stack.push(0);

	for(unsigned long pc = 0; pc < instructions.size(); )
	{
		switch(instructions[pc++].opcode)
		{
				case CONSTANT:
					stack.push(instructions[pc++].value);
				break;
				case ELLIPSOID:
				{
					const TqFloat r2 = (instructions[pc++].get_matrix() * Point).Magnitude2();
					TqFloat result = r2 <= 1 ? 1 - 3*r2 + 3*r2*r2 - r2*r2*r2 : 0;
					Aqsis::log() << info << "Ellipsoid: result " << result << std::endl;
					stack.push(result);
				}
				break;
				case PLANE:
				{
					TqInt which = instructions[pc++].value;
					TqInt n = instructions[pc++].value;

					CqString depthname = m_strings[which];
					IqTextureMap* pMap = QGetRenderContextI() ->GetShadowMap( depthname );

					TqFloat A, B, C, D;
					std::valarray<TqFloat> fv;
					fv.resize(1);
					fv[0]= 0.0f;


					A = m_floats[n];
					B = m_floats[n+1];
					C = m_floats[n+2];
					D = m_floats[n+3];

					if ( pMap != 0 && pMap->IsValid() )
					{
						CqVector3D swidth = 0.0f, twidth = 0.0f;
						CqVector3D _aq_P = Point;
						pMap->SampleMap( _aq_P, swidth, twidth, fv, 0 );
					}
					else
					{
						Aqsis::log() << warning << depthname << " is not found " << std::endl;
						fv[0] = A + 1.0;
					}

					Aqsis::log() << info << "A " << A << " B " << B << " C " << C << " D " << D << std::endl;
					TqFloat result = repulsion(fv[0], A, B, C, D);
					Aqsis::log() << info << "Plane: result " << result << std::endl;
					stack.push(result);
				}
				break;
				case SEGMENT:
				{
					const CqMatrix m = instructions[pc++].get_matrix();
					const CqVector3D start = instructions[pc++].get_vector();
					const CqVector3D end = instructions[pc++].get_vector();
					const TqFloat radius = instructions[pc++].value;


					CqVector3D nearpt = nearest_segment_point(Point, start, end);


					CqVector3D probably  =   m * (Point -  nearpt) ;
					TqFloat r2 = probably.Magnitude2();


					TqFloat result = r2 <= 1 ? (1 - 3*r2 + 3*r2*r2 - r2*r2*r2) : 0;
					Aqsis::log() << info << "Segment: result " << result << std::endl;
					stack.push(result);
				}
				break;
				case SUBTRACT:
				{
					TqFloat a = stack.top();
					stack.pop();
					TqFloat b = stack.top();
					stack.pop();
					Aqsis::log() << info << "idex a " << a << " idex b " << b << std::endl;
					if (a != 0.0)
						stack.push(b / a);
					else
						stack.push(0.0f);
				}
				break;
				case DIVIDE:
				{
					TqFloat a = stack.top();
					stack.pop();
					TqFloat b = stack.top();
					stack.pop();

					stack.push(b - a);
				}
				break;
				case ADD:
				{
					const TqInt count = instructions[pc++].count;
					TqFloat sum = 0;
					for(TqInt i = 0; i != count; ++i)
					{
						sum += stack.top();
						stack.pop();
					}
					stack.push(sum);
				}
				break;
				case MULTIPLY:
				{
					const TqInt count = instructions[pc++].count;
					TqFloat product = 1;
					for(TqInt i = 0; i != count; ++i)
					{
						product *= stack.top();
						stack.pop();
					}
					stack.push(product);
				}
				break;
				case MIN:
				{
					const TqInt count = instructions[pc++].count;
					TqFloat minimum = std::numeric_limits<TqFloat>::max();
					for(TqInt i = 0; i != count; ++i)
					{
						minimum = std::min(minimum, stack.top());
						stack.pop();
					}
					stack.push(minimum);
				}
				break;
				case MAX:
				{
					const TqInt count = instructions[pc++].count;
					TqFloat maximum = -std::numeric_limits<TqFloat>::max();
					for(TqInt i = 0; i != count; ++i)
					{
						maximum = std::max(maximum, stack.top());
						stack.pop();
					}
					stack.push(maximum);
				}
				break;
		}
	}

	return stack.top();
}

//---------------------------------------------------------------------
/** polygonize this is where everything is going on..
*/

void CqBlobby::polygonize(std::vector<CqVector3D>& Vertices, std::vector<CqVector3D>& Normals, std::vector<std::vector<TqInt> >& Polygons, TqFloat Multiplier)
{
	// Set up polygonizer

	const CqVector3D mid = (bbox.vecMax() + bbox.vecMin())/2.0;
	const CqVector3D length =  (bbox.vecMax() - bbox.vecMin());


	TqDouble voxel_size = Multiplier * 4.0;

	TqInt n_x_over_2 = static_cast<TqInt>(2 * (fabs(length.x())+0.5) / voxel_size ) + 1;
	TqInt n_y_over_2 = static_cast<TqInt>(2 * (fabs(length.y())+0.5) / voxel_size ) + 1;
	TqInt n_z_over_2 = static_cast<TqInt>(2 * (fabs(length.z())+0.5) / voxel_size ) + 1;

	Aqsis::log() << info << "n_x_over_2, n_y_over_2, n_z_over_2: (" << n_x_over_2 << ", " << n_y_over_2 << ", " << n_z_over_2 << "), voxel_size (" << voxel_size << ")" << std::endl;

	bloomenthal_polygonizer::polygonization_t which = bloomenthal_polygonizer::MARCHINGCUBES;

	bloomenthal_polygonizer polygonizer(
	    which, // Ask for marching cube
	    voxel_size, // Voxel size
	    0.421875, // Threshold (blobby specific)
	    -n_x_over_2, n_x_over_2,// Lattice X min-max
	    -n_y_over_2, n_y_over_2, // Lattice Y min-max
	    -n_z_over_2, n_z_over_2, // Lattice Z min-max
	    mid, // Lattice center
	    static_cast<implicit_functor&>(*this),
	    Vertices, Normals, Polygons);

	polygonizer.cross_limits();


	// Polygonize blobbies
	TqBool missed_blobbies = TqFalse;
	std::vector<CqVector3D>::const_iterator p;
	for(p = origins.begin(); p != origins.end(); ++p)
	{
		if(!polygonizer.polygonize_from_inside_point(*p))
		{
			missed_blobbies = TqTrue;
			break;
		}
	}

	// Second chance for missed blobbies
	if(missed_blobbies == TqTrue)
		polygonizer.polygonize_whole_grid();

	missed_blobbies = TqFalse;
	for( p = origins.begin(); p != origins.end(); ++p)
		if(!polygonizer.polygonize_from_inside_point(*p))
			missed_blobbies = TqTrue;

	if (missed_blobbies)
		polygonizer.polygonize_whole_grid();

}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------


