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
 * \brief Shadeops which call external plugins.
 */

#include	"shaderexecenv.h"

#include	<cstring>

#include	<aqsis/ri/shadeop.h>

namespace Aqsis {

void CqShaderExecEnv::SO_external( DSOMethod method, void *initData, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(Result)->Class()==class_varying;
	int p;
	for ( p = 0;p < cParams;p++ )
	{
		__fVarying=(apParams[ p ])->Class()==class_varying||__fVarying;
	};

	int dso_argc = cParams + 1; // dso_argv[0] is used for the return value
	void **dso_argv = new void * [ dso_argc ] ;

	// \todo <b>Code Review</b> SO_external is a pretty huge function - it should be split.

	// create storage for the returned value
	switch ( Result->Type() )
	{
		case type_float:
			dso_argv[ 0 ] = ( void* ) new TqFloat;
			break;
		case type_point:
		case type_color:
		case type_triple:
		case type_vector:
		case type_normal:
		case type_hpoint:
			dso_argv[ 0 ] = ( void* ) new TqFloat[ 3 ];
			break;
		case type_string:
			dso_argv[ 0 ] = ( void* ) new STRING_DESC;
			( ( STRING_DESC* ) dso_argv[ 0 ] ) ->s = NULL;
			( ( STRING_DESC* ) dso_argv[ 0 ] ) ->bufflen = 0;
			break;
		case type_matrix:
		case type_sixteentuple:
			dso_argv[ 0 ] = ( void* ) new TqFloat[ 16 ];
			break;
		default:
			// Unhandled TYpe
			break;
	};

	// Allocate space for the arguments
	for ( p = 1;p <= cParams;p++ )
	{
		switch ( apParams[ p - 1 ] ->Type() )
		{
			case type_float:
				dso_argv[ p ] = ( void* ) new TqFloat;
				break;
			case type_hpoint:
			case type_point:
			case type_triple:  // This seems reasonable
			case type_vector:
			case type_normal:
			case type_color:
				dso_argv[ p ] = ( void* ) new TqFloat[ 3 ];
				break;
			case type_string:
				dso_argv[ p ] = ( void* ) new STRING_DESC;
				( ( STRING_DESC* ) dso_argv[ p ] ) ->s = NULL;
				( ( STRING_DESC* ) dso_argv[ p ] ) ->bufflen = 0;
				break;
			case type_matrix:
			case type_sixteentuple:
				dso_argv[ p ] = ( void* ) new TqFloat[ 16 ];
				break;
			default:
				// Unhandled TYpe
				break;
		};
	};


	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{

			// Convert the arguments to the required format for the DSO
			for ( p = 1;p <= cParams;p++ )
			{

				switch ( apParams[ p - 1 ] ->Type() )
				{
					case type_float:
						apParams[ p - 1 ] ->GetFloat( *( ( float* ) dso_argv[ p ] ), __iGrid );
						break;
					case type_hpoint:
					case type_point:
						{
							CqVector3D v;
							apParams[ p - 1 ] ->GetPoint( v, __iGrid );
							( ( float* ) dso_argv[ p ] ) [ 0 ] = v[ 0 ];
							( ( float* ) dso_argv[ p ] ) [ 1 ] = v[ 1 ];
							( ( float* ) dso_argv[ p ] ) [ 2 ] = v[ 2 ];
						}
						break;
					case type_triple:  // This seems reasonable
					case type_vector:
						{
							CqVector3D v;
							apParams[ p - 1 ] ->GetVector( v, __iGrid );
							( ( float* ) dso_argv[ p ] ) [ 0 ] = v[ 0 ];
							( ( float* ) dso_argv[ p ] ) [ 1 ] = v[ 1 ];
							( ( float* ) dso_argv[ p ] ) [ 2 ] = v[ 2 ];
						}
						break;
					case type_normal:
						{
							CqVector3D v;
							apParams[ p - 1 ] ->GetNormal( v, __iGrid );
							( ( float* ) dso_argv[ p ] ) [ 0 ] = v[ 0 ];
							( ( float* ) dso_argv[ p ] ) [ 1 ] = v[ 1 ];
							( ( float* ) dso_argv[ p ] ) [ 2 ] = v[ 2 ];
						}
						break;
					case type_color:
						{
							CqColor c;
							apParams[ p - 1 ] ->GetColor( c, __iGrid );
							( ( float* ) dso_argv[ p ] ) [ 0 ] = c[ 0 ];
							( ( float* ) dso_argv[ p ] ) [ 1 ] = c[ 1 ];
							( ( float* ) dso_argv[ p ] ) [ 2 ] = c[ 2 ];
						}
						break;
					case type_string:
						{
							CqString s;
							apParams[ p - 1 ] ->GetString( s, __iGrid );
							TqInt requiredSize = s.size() + 1;
							STRING_DESC* dsoStr
								= reinterpret_cast<STRING_DESC*>(dso_argv[p]);
							if(requiredSize > dsoStr->bufflen)
							{
								// We use malloc() intentionally here - since
								// the DSO interface is C-based, DSO shadeops
								// may presumably want to delete the memory
								// using free() rather than delete.
								free(dsoStr->s);
								dsoStr->s = reinterpret_cast<char*>(
										malloc(sizeof(char) * requiredSize));
								dsoStr->bufflen = requiredSize;
							}
							strncpy(dsoStr->s, s.c_str(), requiredSize );
						}
						break;
					case type_matrix:
					case type_sixteentuple:
						{
							CqMatrix m;
							int r, c;
							apParams[ p - 1 ] ->GetMatrix( m, __iGrid );
							for ( r = 0; r < 4; r++ )
								for ( c = 0; c < 4; c++ )
									( ( TqFloat* ) dso_argv[ p ] ) [ ( r * 4 ) + c ] = m[ r ][ c ];
						}
						break;
					default:
						// Unhandled TYpe
						break;
				};
			};

			// Atlast, we call the shadeop method, looks rather dull after all this effort.
			method( initData, dso_argc, dso_argv );

			// Pass the returned value back to aqsis
			switch ( Result->Type() )
			{
				case type_float:
					{
						TqFloat val = *( ( float* ) ( dso_argv[ 0 ] ) );
						Result->SetFloat( val, __iGrid );
					}
					break;
				case type_hpoint:
				case type_point:
					{
						CqVector3D v;
						v[ 0 ] = ( ( float* ) dso_argv[ 0 ] ) [ 0 ];
						v[ 1 ] = ( ( float* ) dso_argv[ 0 ] ) [ 1 ];
						v[ 2 ] = ( ( float* ) dso_argv[ 0 ] ) [ 2 ];
						Result->SetPoint( v, __iGrid );
					}
					break;
				case type_triple:  // This seems reasonable
				case type_vector:
					{
						CqVector3D v;
						v[ 0 ] = ( ( float* ) dso_argv[ 0 ] ) [ 0 ];
						v[ 1 ] = ( ( float* ) dso_argv[ 0 ] ) [ 1 ];
						v[ 2 ] = ( ( float* ) dso_argv[ 0 ] ) [ 2 ];
						Result->SetVector( v, __iGrid );
					}
					break;
				case type_normal:
					{
						CqVector3D v;
						v[ 0 ] = ( ( float* ) dso_argv[ 0 ] ) [ 0 ];
						v[ 1 ] = ( ( float* ) dso_argv[ 0 ] ) [ 1 ];
						v[ 2 ] = ( ( float* ) dso_argv[ 0 ] ) [ 2 ];
						Result->SetNormal( v, __iGrid );
					}
					break;
				case type_color:
					{
						CqColor c;
						c[ 0 ] = ( ( float* ) dso_argv[ 0 ] ) [ 0 ];
						c[ 1 ] = ( ( float* ) dso_argv[ 0 ] ) [ 1 ];
						c[ 2 ] = ( ( float* ) dso_argv[ 0 ] ) [ 2 ];
						Result->SetColor( c, __iGrid );
					}
					break;
				case type_string:
					{
						CqString s( ( ( STRING_DESC* ) dso_argv[ 0 ] ) ->s );
						Result->SetString( s, __iGrid );
					}
					break;
				case type_matrix:
				case type_sixteentuple:
					{
						CqMatrix m( ( float* ) dso_argv[ 0 ] );
						Result->SetMatrix( m, __iGrid );
					}
					break;
				default:
					// Unhandled TYpe
					std::cout << "Unsupported type" << std::endl;
					break;
			};


			// Set the values that were altered by the Shadeop
			for ( p = 1;p <= cParams;p++ )
			{
				switch ( apParams[ p - 1 ] ->Type() )
				{
					case type_float:
						{
							TqFloat val = *( ( float* ) dso_argv[ p ] ) ;
							apParams[ p - 1 ] ->SetFloat( val, __iGrid );
						}
						break;
					case type_hpoint:
					case type_point:
						{
							CqVector3D v;
							v[ 0 ] = ( ( float* ) dso_argv[ p ] ) [ 0 ];
							v[ 1 ] = ( ( float* ) dso_argv[ p ] ) [ 1 ];
							v[ 2 ] = ( ( float* ) dso_argv[ p ] ) [ 2 ];
							apParams[ p - 1 ] ->SetPoint( v, __iGrid );
						}
						break;
					case type_triple:  // This seems reasonable
					case type_vector:
						{
							CqVector3D v;
							v[ 0 ] = ( ( float* ) dso_argv[ p ] ) [ 0 ];
							v[ 1 ] = ( ( float* ) dso_argv[ p ] ) [ 1 ];
							v[ 2 ] = ( ( float* ) dso_argv[ p ] ) [ 2 ];
							apParams[ p - 1 ] ->SetVector( v, __iGrid );
						}
						break;
					case type_normal:
						{
							CqVector3D v;
							v[ 0 ] = ( ( float* ) dso_argv[ p ] ) [ 0 ];
							v[ 1 ] = ( ( float* ) dso_argv[ p ] ) [ 1 ];
							v[ 2 ] = ( ( float* ) dso_argv[ p ] ) [ 2 ];
							apParams[ p - 1 ] ->SetNormal( v, __iGrid );
						}
						break;
					case type_color:
						{
							CqColor c;
							c[ 0 ] = ( ( float* ) dso_argv[ p ] ) [ 0 ];
							c[ 1 ] = ( ( float* ) dso_argv[ p ] ) [ 1 ];
							c[ 2 ] = ( ( float* ) dso_argv[ p ] ) [ 2 ];
							apParams[ p - 1 ] ->SetColor( c, __iGrid );
						}
						break;
					case type_string:
						{
							CqString s( ( ( STRING_DESC* ) dso_argv[ p ] ) ->s );
							apParams[ p - 1 ] ->SetString( s, __iGrid );
						}
						break;
					case type_matrix:
					case type_sixteentuple:
						{
							CqMatrix m( ( float* ) dso_argv[ p ] );
							apParams[ p - 1 ] ->SetMatrix( m, __iGrid );
						}
						break;
					default:
						// Unhandled TYpe
						break;
				};
			};

		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);

	// Free up the storage allocated for the return type
	switch ( Result->Type() )
	{
		case type_float:
			delete ( float* ) dso_argv[ 0 ];
			break;
		case type_point:
		case type_triple:  // This seems reasonable
		case type_vector:
		case type_normal:
		case type_color:
		case type_hpoint:
			delete[] ( float* ) dso_argv[ 0 ];
			break;
		case type_string:  // Need to look into these
			delete ( STRING_DESC* ) dso_argv[ 0 ];
			break;
		case type_matrix:
		case type_sixteentuple:
			delete[] ( float* ) dso_argv[ 0 ];
			break;
		default:
			// Unhandled TYpe
			break;
	};

	// Free up the storage allocated for the args
	for ( p = 1;p <= cParams;p++ )
	{
		switch ( apParams[ p - 1 ] ->Type() )
		{
			case type_float:
				delete ( float* ) dso_argv[ p ];
				break;
			case type_point:
			case type_triple:
			case type_vector:
			case type_normal:
			case type_color:
			case type_hpoint:
				delete[] ( float* ) dso_argv[ p ];
				break;
			case type_string:
				{
					STRING_DESC* dsoStr = reinterpret_cast<STRING_DESC*>(dso_argv[p]);
					// Using free() rather than delete[] is intentional - see
					// malloc usage above.
					free(dsoStr->s);
					delete dsoStr;
				}
				break;
			case type_matrix:
			case type_sixteentuple:
				delete[] ( float* ) dso_argv[ p ];
				break;
			default:
				// Unhandled TYpe
				break;
		};
	};

	delete[] dso_argv;
}

} // namespace Aqsis
