// Aqsis
// Copyright © 2006, Paul C. Gregory
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
    \brief Implements Blobby polygonizer
    \author Romain Behar (romainbehar@yahoo.com)
    \author Michel Joron (rearrange a bit)
*/
/*    References:
*          K-3D
*
*/
#include <stdio.h>
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
#include "marchingcubes.h"


START_NAMESPACE( Aqsis )

#define ZCLAMP  1e-6


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
			m_instructions(Instructions),
			m_bbox(BBox)
		{
			// Initialize bounding-box
			TqFloat absolute = std::numeric_limits<TqFloat>::max();
			m_bbox = CqBound(absolute, absolute, absolute, -absolute, -absolute, -absolute);

			// Decode blobby instructions and store them onto a stack
			for(TqInt c = 0; c < ncode;)
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
							opcodes.push_back( opcode( CqBlobby::DIVIDE, c ) );
							c += 2;
						break;
						case 5:
							opcodes.push_back( opcode( CqBlobby::SUBTRACT, c ) );
							c += 2;
						break;
						case 6:
							opcodes.push_back( opcode( CqBlobby::NEGATE, c++ ) );
						break;
						case 7:
							Aqsis::log() << warning << "Unhandled Blobby IDEMPOTENTATE." << std::endl;
							c++;
						break;

						case 1000:
							opcodes.push_back( opcode( CqBlobby::CONSTANT, c++ ) );
							STATS_INC( GPR_blobbies );
							Aqsis::log() << warning << "Unhandled Blobby CONSTANT." << std::endl;
						break;
						case 1001:
							opcodes.push_back( opcode( CqBlobby::ELLIPSOID, c++ ) );
							STATS_INC( GPR_blobbies );
						break;
						case 1002:
							opcodes.push_back( opcode( CqBlobby::SEGMENT, c++ ) );
							STATS_INC( GPR_blobbies );
						break;
						case 1003:
							opcodes.push_back( opcode( CqBlobby::PLANE, c ) );
							c += 2;
							STATS_INC( GPR_blobbies );
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

		void grow_bound( const CqMatrix& transformation, const TqFloat radius = 1.0 )
		{
			TqFloat r = radius * 0.5;
			CqBound unit_box( -r, -r, -r, r, r, r );
			unit_box.Transform( transformation );

			m_bbox.Encapsulate( unit_box );
		}

		void grow_bound( const CqVector3D& vector, const TqFloat radius = 1.0 )
		{
			m_bbox.Encapsulate( vector );
		}


		/// Build implicit value evaluation program: store opcodes and parameters in execution order
		void build_program( opcode op )
		{
			switch( op.name )
			{
					case CqBlobby::CONSTANT:
						Aqsis::log() << warning << "RiBlobby's Constant not supported." << std::endl;
					break;

					case CqBlobby::NEGATE:
						Aqsis::log() << warning << "RiBlobby's Negate operator not supported." << std::endl;
					break;

					case CqBlobby::IDEMPOTENTATE:
						Aqsis::log() << warning << "RiBlobby's Idempotate operator not supported." << std::endl;
					break;

					case CqBlobby::PLANE:
					{
						m_instructions.push_back(CqBlobby::instruction(CqBlobby::PLANE));
						m_instructions.push_back(CqBlobby::instruction(m_code[op.index]));
						m_instructions.push_back(CqBlobby::instruction(m_code[op.index + 1]));
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

						grow_bound(transformation);

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


						m_bbox = CqBound(start.x(),start.y(),start.z(), end.x(), end.y(), end.z());
						grow_bound(transformation, radius);

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

	TqInt* m_code;
	TqFloat* m_floats;
	CqBlobby::instructions_t& m_instructions;
	CqBound& m_bbox;
};

//---------------------------------------------------------------------
/** Constructor.
 */
CqBlobby::CqBlobby(TqInt nleaf, TqInt ncode, TqInt* code, TqInt nfloats, TqFloat* floats, TqInt nstrings, char** strings) : m_nleaf(nleaf), m_ncode(ncode), m_code(code), m_nfloats(nfloats), m_floats(floats), m_nstrings(nstrings), m_strings(strings)
{
	blobby_vm_assembler(nleaf, ncode, code, nfloats, floats, nstrings, strings, instructions, bbox);
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
	//TqFloat z = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Clipping" ) [ 0 ];


	return 0;
}

/** \fn CqVector3D nearest_segment_point(const CqVector3D& Point, const CqVector3D& S1, const CqVector3D& S2)
    \brief From a given 3D point, return a segment's closest point.
    \param Point The point to check the distance from.
    \param S1 The first end of the segment.
    \param S2 The second end of the segment.
    \return One of the points belonging to [S1;S2] segment, the nearest to Point.
 */
CqVector3D nearest_segment_point( const CqVector3D& Point, const CqVector3D& S1,
                                 const CqVector3D& S2 )
{
	const CqVector3D vector = S2 - S1;
	const CqVector3D w = Point - S1;

	const TqFloat c1 = CqVector3D( w * vector ).Magnitude2();
	if(c1 <= 0)
		return S1;

	const TqFloat c2 = CqVector3D( vector * vector ).Magnitude2();
	if(c2 <= c1)
		return S2;

	const TqFloat b = c1 / c2;
	const CqVector3D middlepoint = S1 + b * vector;
	return middlepoint;
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
TqFloat CqBlobby::implicit_value( CqVector3D &Point, TqInt n, std::vector <TqFloat> &splits )
{
	TqFloat sum = 0.0f;
	TqFloat result;
	std::stack<TqFloat> stack;
	TqInt int_index = 0;
	stack.push(0);

	for(unsigned long pc = 0; pc < instructions.size(); )
	{
		switch(instructions[pc++].opcode)
		{
				case CqBlobby::CONSTANT:
					result = instructions[pc++].value;
					sum += result;
					splits[int_index++] = result;
				break;
				case CqBlobby::ELLIPSOID:
				{
					const TqFloat r2 = (instructions[pc++].get_matrix() * Point).Magnitude2();
					result = r2 <= 1 ? 1 - 3*r2 + 3*r2*r2 - r2*r2*r2 : 0;
					sum += result;
					splits[int_index++] = result;
				}
				break;
				case CqBlobby::PLANE:
				{
					TqInt which = (TqInt) instructions[pc++].value;
					TqInt n = (TqInt) instructions[pc++].value;

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

					result = repulsion(fv[0], A, B, C, D);
					sum += result;
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


					result = r2 <= 1 ? (1 - 3*r2 + 3*r2*r2 - r2*r2*r2) : 0;
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
 *  This is the most important method it is used by jules_bloomenthal.cpp to
 *  polygonize the primitives
 */
TqFloat CqBlobby::implicit_value( const CqVector3D& Point )
{
	std::stack<TqFloat> stack;
	stack.push(0);
	TqFloat result;

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
					result = r2 <= 1 ? 1 - 3*r2 + 3*r2*r2 - r2*r2*r2 : 0;
					//Aqsis::log() << info << "Ellipsoid: result " << result << std::endl;
					stack.push(result);
				}
				break;
				case PLANE:
				{
					TqInt which = (TqInt) instructions[pc++].value;
					TqInt n = (TqInt) instructions[pc++].value;

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
					result = repulsion(fv[0], A, B, C, D);
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


					result = r2 <= 1 ? (1 - 3*r2 + 3*r2*r2 - r2*r2*r2) : 0;
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

void CqBlobby::polygonize( TqInt& NPoints, TqInt& NPolys, TqInt*& NVertices, TqInt*& Vertices, TqFloat*& Points, TqFloat Multiplier )
{
	MarchingCubes mc;
	TqInt Divider = 50;
	if (Multiplier)
        	Divider = (TqInt)ceil(0.5/(Multiplier));
	mc.set_resolution( Divider, Divider, Divider );
	mc.init_all();

	const CqVector3D centre = ( bbox.vecMax() + bbox.vecMin() )/2.0;
	const CqVector3D length = ( bbox.vecMax() - bbox.vecMin() );
	const TqFloat length_x = length.x();
	const TqFloat length_y = length.y();
	const TqFloat length_z = length.z();
	const TqFloat max = std::max( length_x, std::max( length_y, length_z ) );
	const TqFloat voxel_size = max / (Divider - 1.0);

	Aqsis::log() << info << "Divider Blobby: " << Divider << std::endl;
	const TqFloat x_start = centre.x() - max / 2.0;
	const TqFloat y_start = centre.y() - max / 2.0;
	const TqFloat z_start = centre.z() - max / 2.0;

	// Compute implicit values
	TqFloat z = z_start;
	for( int k = 0 ; k < mc.size_z() ; k++, z += voxel_size )
	{
		TqFloat y = y_start;
		for( int j = 0 ; j < mc.size_y() ; j++, y += voxel_size )
		{
			TqFloat x = x_start;
			for( int i = 0 ; i < mc.size_x() ; i++, x += voxel_size )
			{
				const TqFloat iv = implicit_value( CqVector3D( x, y, z ) );
				mc.set_data( static_cast<float>( iv - 0.421875 ), i, j, k );
			}
		}
	}

	// Run Marching Cubes
	mc.run() ;
	mc.clean_temps() ;

	const int nverts = mc.nverts();
	const int ntrigs = mc.ntrigs();
	const Vertex* vertices = mc.vertices();
	const Triangle* triangles = mc.triangles();

	// Allocate RiPointsPolygons structure
	NPoints = nverts;
	NPolys = ntrigs;
	NVertices = new TqInt[NPolys];
	Vertices = new TqInt[3 * NPolys];
	Points = new TqFloat[3 * NPoints];

	// Set vertex indices
	TqInt* nvert = NVertices;
	TqInt* vert = Vertices;
	for ( int i = 0; i < ntrigs; ++i )
	{
		*nvert++ = 3;
		*vert++ = triangles[i].v1;
		*vert++ = triangles[i].v2;
		*vert++ = triangles[i].v3;
	}

	// Compute vertex positions in the blobbies world (they were returned in grid coordinates)
	TqFloat* point = Points;
	for ( int i = 0; i < nverts; i++ )
	{
		*point++ = x_start + voxel_size * vertices[i].x;
		*point++ = y_start + voxel_size * vertices[i].y;
		*point++ = z_start + voxel_size * vertices[i].z;
	}

	// Cleanup
	mc.clean_all() ;
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------


