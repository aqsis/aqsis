// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Declares the classes and support structures for the shader VM stack.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef SHADERSTACK_H_INCLUDED
#define SHADERSTACK_H_INCLUDED 1

#include	<stack>
#include	<vector>

#include	"aqsis.h"

#include	"vector3d.h"
#include	"vector4d.h"
#include	"color.h"
#include	"sstring.h"
#include	"matrix.h"
#include	"ishaderdata.h"
#include	"bitvector.h"

START_NAMESPACE( Aqsis )

#define	OpLSS_FF(a,Res,State)		OpLSS(temp_float,temp_float,a,Res,State)
#define	OpLSS_PP(a,Res,State)		OpLSS(temp_point,temp_point,a,Res,State)
#define	OpLSS_CC(a,Res,State)		OpLSS(temp_color,temp_color,a,Res,State)

#define	OpGRT_FF(a,Res,State)		OpGRT(temp_float,temp_float,a,Res,State)
#define	OpGRT_PP(a,Res,State)		OpGRT(temp_point,temp_point,a,Res,State)
#define	OpGRT_CC(a,Res,State)		OpGRT(temp_color,temp_color,a,Res,State)

#define	OpLE_FF(a,Res,State)		OpLE(temp_float,temp_float,a,Res,State)
#define	OpLE_PP(a,Res,State)		OpLE(temp_point,temp_point,a,Res,State)
#define	OpLE_CC(a,Res,State)		OpLE(temp_color,temp_color,a,Res,State)

#define	OpGE_FF(a,Res,State)		OpGE(temp_float,temp_float,a,Res,State)
#define	OpGE_PP(a,Res,State)		OpGE(temp_point,temp_point,a,Res,State)
#define	OpGE_CC(a,Res,State)		OpGE(temp_color,temp_color,a,Res,State)

#define	OpEQ_FF(a,Res,State)		OpEQ(temp_float,temp_float,a,Res,State)
#define	OpEQ_PP(a,Res,State)		OpEQ(temp_point,temp_point,a,Res,State)
#define	OpEQ_CC(a,Res,State)		OpEQ(temp_color,temp_color,a,Res,State)
#define	OpEQ_SS(a,Res,State)		OpEQ(temp_string,temp_string,a,Res,State)

#define	OpNE_FF(a,Res,State)		OpNE(temp_float,temp_float,a,Res,State)
#define	OpNE_PP(a,Res,State)		OpNE(temp_point,temp_point,a,Res,State)
#define	OpNE_CC(a,Res,State)		OpNE(temp_color,temp_color,a,Res,State)
#define	OpNE_SS(a,Res,State)		OpNE(temp_string,temp_string,a,Res,State)

#define	OpMUL_FF(a,Res,State)		OpMUL(temp_float,temp_float,a,Res,State)
#define	OpDIV_FF(a,Res,State)		OpDIV(temp_float,temp_float,a,Res,State)
#define	OpADD_FF(a,Res,State)		OpADD(temp_float,temp_float,a,Res,State)
#define	OpSUB_FF(a,Res,State)		OpSUB(temp_float,temp_float,a,Res,State)
#define	OpNEG_F(Res,State)			OpNEG(temp_float,Res,State)

#define	OpMUL_PP(a,Res,State)		OpMUL(temp_point,temp_point,a,Res,State)
#define	OpDIV_PP(a,Res,State)		OpDIV(temp_point,temp_point,a,Res,State)
#define	OpADD_PP(a,Res,State)		OpADD(temp_point,temp_point,a,Res,State)
#define	OpSUB_PP(a,Res,State)		OpSUB(temp_point,temp_point,a,Res,State)
#define	OpCRS_PP(a,Res,State)		OpCRS(temp_point,temp_point,a,Res,State)
#define	OpDOT_PP(a,Res,State)		OpDOT(temp_point,temp_point,a,Res,State)
#define	OpNEG_P(Res,State)			OpNEG(temp_point,Res,State)

#define	OpMUL_CC(a,Res,State)		OpMUL(temp_color,temp_color,a,Res,State)
#define	OpDIV_CC(a,Res,State)		OpDIV(temp_color,temp_color,a,Res,State)
#define	OpADD_CC(a,Res,State)		OpADD(temp_color,temp_color,a,Res,State)
#define	OpSUB_CC(a,Res,State)		OpSUB(temp_color,temp_color,a,Res,State)
#define	OpCRS_CC(a,Res,State)		OpCRS(temp_color,temp_color,a,Res,State)
#define	OpDOT_CC(a,Res,State)		OpDOT(temp_color,temp_color,a,Res,State)
#define	OpNEG_C(Res,State)			OpNEG(temp_color,Res,State)

#define	OpMUL_FP(a,Res,State)		OpMUL(temp_float,temp_point,a,Res,State)
#define	OpDIV_FP(a,Res,State)		OpDIV(temp_float,temp_point,a,Res,State)
#define	OpADD_FP(a,Res,State)		OpADD(temp_float,temp_point,a,Res,State)
#define	OpSUB_FP(a,Res,State)		OpSUB(temp_float,temp_point,a,Res,State)

#define	OpMUL_FC(a,Res,State)		OpMUL(temp_float,temp_color,a,Res,State)
#define	OpDIV_FC(a,Res,State)		OpDIV(temp_float,temp_color,a,Res,State)
#define	OpADD_FC(a,Res,State)		OpADD(temp_float,temp_color,a,Res,State)
#define	OpSUB_FC(a,Res,State)		OpSUB(temp_float,temp_color,a,Res,State)

#define	OpLAND_B(a,Res,State)		OpLAND(temp_bool,temp_bool,a,Res,State)
#define	OpLOR_B(a,Res,State)		OpLOR(temp_bool,temp_bool,a,Res,State)

#define	OpCAST_FC(Res,State)		OpCAST(temp_float,temp_color, Res,State)
#define	OpCAST_FP(Res,State)		OpCAST(temp_float,temp_point, Res,State)
#define	OpCAST_PC(Res,State)		OpCAST(temp_point,temp_color, Res,State)
#define	OpCAST_CP(Res,State)		OpCAST(temp_color,temp_point, Res,State)
#define	OpCAST_FM(Res,State)		OpCAST(temp_float,temp_matrix, Res,State)

#define	OpTRIPLE_C(a,b,c,State)		OpTRIPLE(temp_color,a,b,c,State)
#define	OpTRIPLE_P(a,b,c,State)		OpTRIPLE(temp_point,a,b,c,State)

#define	OpHEXTUPLE_M(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,State)	OpHEXTUPLE(temp_matrix,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,State)

#define	OpCOMP_C(index,Res,State)	OpCOMP(temp_color,index,Res,State)
#define	OpCOMP_P(index,Res,State)	OpCOMP(temp_point,index,Res,State)

#define	OpSETCOMP_C(index,a,State)	OpSETCOMP(temp_color,index,a,State)
#define	OpSETCOMP_P(index,a,State)	OpSETCOMP(temp_point,index,a,State)

/** \enum EqStackEntryType
 */
enum EqStackEntryType
{
    StackEntryType_Float, 	///< Float.
    StackEntryType_Int, 		///< Integer.
    StackEntryType_Bool, 	///< Boolean.
    StackEntryType_Point, 	///< 3D point..
    StackEntryType_HPoint, 	///< 4D Homogenous point.
    StackEntryType_Color, 	///< Color.
    StackEntryType_String, 	///< String.
    StackEntryType_Matrix, 	///< Matrix.
};

struct SqVMStackEntry
{
	SqVMStackEntry( TqFloat f = 0 )
	{
		*this = f;
	}
	SqVMStackEntry( const CqVector3D& v )
	{
		*this = v;
	}
	SqVMStackEntry( const CqColor& c )
	{
		*this = c;
	}
	SqVMStackEntry( const char* s )
	{
		m_Value.m_str = NULL;
		*this = s;
	}
	SqVMStackEntry( const CqString& s )
	{
		m_Value.m_str = NULL;
		*this = s;
	}
	SqVMStackEntry( const CqMatrix& m )
	{
		*this = m;
	}
	~SqVMStackEntry()
	{
		if(	m_Type == StackEntryType_String )
			delete[]( m_Value.m_str );
	}

	// Cast to the various types
	/** Type checked cast to a float
	 */
	void GetValue( TqFloat& res) const
	{
		if ( m_Type == StackEntryType_Int )
			res = static_cast<TqFloat>( m_Value.m_int );
		else
			res = m_Value.m_float;
	}
	/** Type checked cast to an integer
	 */
	void GetValue( TqInt& res ) const
	{
		if ( m_Type == StackEntryType_Float )
			res = static_cast<TqInt>( m_Value.m_float );
		else
		res = m_Value.m_int;
	}
	/** Type checked cast to a boolean
	 */
	void GetValue( bool& res) const
	{
		assert( m_Type == StackEntryType_Bool );
		res = m_Value.m_bool;
	}
	/** Type checked cast to a 3D vector
	 */
	void GetValue( CqVector3D& res ) const
	{
		if ( m_Type == StackEntryType_Float )
		{
			res.x(m_Value.m_float);
			res.y(m_Value.m_float);
			res.z(m_Value.m_float);
		}
		else if ( m_Type == StackEntryType_Int )
		{
			res.x(static_cast<TqFloat>( m_Value.m_int ));
			res.y(static_cast<TqFloat>( m_Value.m_int ));
			res.z(static_cast<TqFloat>( m_Value.m_int ));
		}
		else if ( m_Type == StackEntryType_Bool )
		{
			res.x(static_cast<TqFloat>( m_Value.m_bool ));
			res.y(static_cast<TqFloat>( m_Value.m_bool ));
			res.z(static_cast<TqFloat>( m_Value.m_bool ));
		}
		else if ( m_Type == StackEntryType_HPoint )
		{
			res.x(m_Value.m_hpoint[0]/m_Value.m_hpoint[3]);
			res.y(m_Value.m_hpoint[1]/m_Value.m_hpoint[3]);
			res.z(m_Value.m_hpoint[2]/m_Value.m_hpoint[3]);
		}
		else
		{
			res.x(m_Value.m_point[0]);
			res.y(m_Value.m_point[1]);
			res.z(m_Value.m_point[2]);
		}
	}

	/** Type checked cast to a 4D vector.
	 */
	void GetValue( CqVector4D& res ) const
	{
		if ( m_Type == StackEntryType_Float )
		{
			res.x(m_Value.m_float);
			res.y(m_Value.m_float);
			res.z(m_Value.m_float);
			res.h(1.0f);
		}
		else if ( m_Type == StackEntryType_Int )
		{
			res.x(static_cast<TqFloat>( m_Value.m_int ));
			res.y(static_cast<TqFloat>( m_Value.m_int ));
			res.z(static_cast<TqFloat>( m_Value.m_int ));
			res.h(1.0f);
		}
		else if ( m_Type == StackEntryType_Bool )
		{
			res.x(static_cast<TqFloat>( m_Value.m_bool ));
			res.y(static_cast<TqFloat>( m_Value.m_bool ));
			res.z(static_cast<TqFloat>( m_Value.m_bool ));
			res.h(1.0f);
		}
		else if ( m_Type == StackEntryType_Point )
		{
			res.x(m_Value.m_point[0]);
			res.y(m_Value.m_point[1]);
			res.z(m_Value.m_point[2]);
			res.h(1.0f);
		}
		else
		{
			res.x(m_Value.m_hpoint[0]);
			res.y(m_Value.m_hpoint[1]);
			res.z(m_Value.m_hpoint[2]);
			res.h(m_Value.m_hpoint[3]);
		}
	}
	/** Type checked cast to a color
	 */
	void GetValue( CqColor& res ) const
	{
		if ( m_Type == StackEntryType_Float )
		{
			res.SetfRed(m_Value.m_float);
			res.SetfGreen(m_Value.m_float);
			res.SetfBlue(m_Value.m_float);
		}
		else if ( m_Type == StackEntryType_Int )
		{
			res.SetfRed(static_cast<TqFloat>( m_Value.m_int ));
			res.SetfGreen(static_cast<TqFloat>( m_Value.m_int ));
			res.SetfBlue(static_cast<TqFloat>( m_Value.m_int ));
		}
		else if ( m_Type == StackEntryType_Bool )
		{
			res.SetfRed(static_cast<TqFloat>( m_Value.m_bool ));
			res.SetfGreen(static_cast<TqFloat>( m_Value.m_bool ));
			res.SetfBlue(static_cast<TqFloat>( m_Value.m_bool ));
		}
		else if ( m_Type == StackEntryType_HPoint )
		{
			res.SetfRed(m_Value.m_hpoint[0]/m_Value.m_hpoint[3]);
			res.SetfGreen(m_Value.m_hpoint[1]/m_Value.m_hpoint[3]);
			res.SetfBlue(m_Value.m_hpoint[2]/m_Value.m_hpoint[3]);
		}
		else
		{
			res.SetfRed(m_Value.m_color[0]);
			res.SetfGreen(m_Value.m_color[1]);
			res.SetfBlue(m_Value.m_color[2]);
		}
	}
	/** Type checked cast to a string
	 */
	void GetValue( CqString& res ) const
	{
		assert( m_Type == StackEntryType_String );
		res = m_Value.m_str;
	}
	/** Type checked cast to a matrix
	 */
	void GetValue( CqMatrix& res ) const
	{
		assert( m_Type == StackEntryType_Matrix );
		res[0][0] = m_Value.m_matrix[0 ];
		res[0][1] = m_Value.m_matrix[1 ];
		res[0][2] = m_Value.m_matrix[2 ];
		res[0][3] = m_Value.m_matrix[3 ];
		res[1][0] = m_Value.m_matrix[4 ];
		res[1][1] = m_Value.m_matrix[5 ];
		res[1][2] = m_Value.m_matrix[6 ];
		res[1][3] = m_Value.m_matrix[7 ];
		res[2][0] = m_Value.m_matrix[8 ];
		res[2][1] = m_Value.m_matrix[9 ];
		res[2][2] = m_Value.m_matrix[10];
		res[2][3] = m_Value.m_matrix[11];
		res[3][0] = m_Value.m_matrix[12];
		res[3][1] = m_Value.m_matrix[13];
		res[3][2] = m_Value.m_matrix[14];
		res[3][3] = m_Value.m_matrix[15];
	}



	/** Assignment from a float
	 */
	SqVMStackEntry& operator=( TqFloat f )
	{
		m_Value.m_float = f; 
		m_Type = StackEntryType_Float; 
		return ( *this );
	}
	/** Assignment from an integer
	 */
	SqVMStackEntry& operator=( TqInt i )
	{
		m_Value.m_int = i; 
		m_Type = StackEntryType_Int; 
		return ( *this );
	}
	/** Assignment from a boolean
	 */
	SqVMStackEntry& operator=( bool b )
	{
		m_Value.m_bool = b; 
		m_Type = StackEntryType_Bool; 
		return ( *this );
	}
	/** Assignment from a 4D vector
	 */
	SqVMStackEntry& operator=( const CqVector4D& v )
	{
		m_Value.m_hpoint[0] = v.x();
		m_Value.m_hpoint[1] = v.y();
		m_Value.m_hpoint[2] = v.z();
		m_Value.m_hpoint[3] = v.h();
		m_Type = StackEntryType_HPoint; 
		return ( *this );
	}
	/** Assignment from a 3D vector
	 */
	SqVMStackEntry& operator=( const CqVector3D& v )
	{
		m_Value.m_point[0] = v.x();
		m_Value.m_point[1] = v.y();
		m_Value.m_point[2] = v.z();
		m_Type = StackEntryType_Point; 
		return ( *this );
	}
	/** Assignment from a color
	 */
	SqVMStackEntry& operator=( const CqColor& c )
	{
		m_Value.m_color[0] = c.fRed();
		m_Value.m_color[1] = c.fGreen();
		m_Value.m_color[2] = c.fBlue();
		m_Type = StackEntryType_Color; 
		return ( *this );
	}
	/** Assignment from a char pointer (string)
	 */
	SqVMStackEntry& operator=( const char* s )
	{
		if( m_Type == StackEntryType_String && m_Value.m_str != NULL )
			delete[]( m_Value.m_str );
		m_Value.m_str = new char[strlen(s)+1]; 
		strcpy(m_Value.m_str, s);
		m_Type = StackEntryType_String; 
		return ( *this );
	}
	/** Assignment from a string
	 */
	SqVMStackEntry& operator=( const CqString& s )
	{
		if( m_Type == StackEntryType_String && m_Value.m_str != NULL )
			delete[]( m_Value.m_str );
		m_Value.m_str = new char[s.size()+1]; 
		strcpy(m_Value.m_str, s.c_str());
		m_Type = StackEntryType_String; 
		return ( *this );
	}
	/** Assignment from a matrix
	 */
	SqVMStackEntry& operator=( const CqMatrix& m )
	{
		m_Value.m_matrix[0 ] = m[0][0];
		m_Value.m_matrix[1 ] = m[0][1];
		m_Value.m_matrix[2 ] = m[0][2];
		m_Value.m_matrix[3 ] = m[1][3];
		m_Value.m_matrix[4 ] = m[1][0];
		m_Value.m_matrix[5 ] = m[1][1];
		m_Value.m_matrix[6 ] = m[1][2];
		m_Value.m_matrix[7 ] = m[2][3];
		m_Value.m_matrix[8 ] = m[2][0];
		m_Value.m_matrix[9 ] = m[2][1];
		m_Value.m_matrix[10] = m[2][2];
		m_Value.m_matrix[11] = m[3][3];
		m_Value.m_matrix[12] = m[3][0];
		m_Value.m_matrix[13] = m[3][1];
		m_Value.m_matrix[14] = m[3][2];
		m_Value.m_matrix[15] = m[3][3];
		m_Type = StackEntryType_Matrix;
		return ( *this );
	}

	union
	{
		TqFloat	m_float;		///< Float value
		TqInt	m_int;			///< Integer value
		bool	m_bool;			///< Boolean value
		char*	m_str;			///< String value
		TqFloat	m_point[3];		///< 3D point value
		TqFloat	m_hpoint[4];	///< 4D homogenous point value
		TqFloat	m_color[3];		///< Color value
		TqFloat	m_matrix[16];	///< Matrix value
	}m_Value;
	TqInt	m_Type;			///< Type identifier, from EqVariableType.
}
;


class CqVMStackEntry : public IqShaderData
{
	public:
		CqVMStackEntry( TqInt size = 1 );
		CqVMStackEntry( IqShaderData* pv )
		{
			m_pVarRef = pv;
		}

		CqVMStackEntry( const CqVMStackEntry& e )
		{
			*this = e;
		}

		CqVMStackEntry( TqFloat f )
		{
			m_Size = 1;
			*this = f;
		}
		CqVMStackEntry( TqBool b )
		{
			m_Size = 1;
			*this = b;
		}
		CqVMStackEntry( const CqVector3D& vec )
		{
			m_Size = 1;
			*this = vec;
		}
		CqVMStackEntry( const char* pstr )
		{
			m_Size = 1;
			*this = pstr;
		}
		CqVMStackEntry( const CqString& str )
		{
			m_Size = 1;
			*this = str;
		}
		CqVMStackEntry( const CqMatrix& mat )
		{
			m_Size = 1;
			*this = mat;
		}
		CqVMStackEntry( const CqColor& col )
		{
			m_Size = 1;
			*this = col;
		}
		~CqVMStackEntry()
		{}

		// Value access functions overridden from IqShaderData
		virtual void GetFloat( TqFloat& f, TqInt index = 0 ) const
		{
			if( m_pVarRef != NULL )	m_pVarRef->GetFloat( f, index );
			else					( Size() == 1 )? m_Value.GetValue( f ) : m_aValues[ index ].GetValue( f );
		}
		virtual void GetBool( TqBool& b, TqInt index = 0 ) const
		{
			if( m_pVarRef != NULL )	assert( false );
			else					( Size() == 1 )? m_Value.GetValue( b ) : m_aValues[ index ].GetValue( b );
		}
		virtual void GetString( CqString& s, TqInt index = 0 ) const
		{
			if( m_pVarRef != NULL )	m_pVarRef->GetString( s, index );
			else					( Size() == 1 )? m_Value.GetValue( s ) : m_aValues[ index ].GetValue( s );
		}
		virtual void GetPoint( CqVector3D& p, TqInt index = 0 ) const
		{
			if( m_pVarRef != NULL )	m_pVarRef->GetPoint( p, index );
			else					( Size() == 1 )? m_Value.GetValue( p ) : m_aValues[ index ].GetValue( p );
		}
		virtual void GetVector( CqVector3D& v, TqInt index = 0 ) const
		{
			if( m_pVarRef != NULL )	m_pVarRef->GetVector( v, index );
			else					( Size() == 1 )? m_Value.GetValue( v ) : m_aValues[ index ].GetValue( v );
		}
		virtual void GetNormal( CqVector3D& n, TqInt index = 0 ) const
		{
			if( m_pVarRef != NULL )	m_pVarRef->GetNormal( n, index );
			else					( Size() == 1 )? m_Value.GetValue( n ) : m_aValues[ index ].GetValue( n );
		}
		virtual void GetColor( CqColor& c, TqInt index = 0 ) const
		{
			if( m_pVarRef != NULL )	m_pVarRef->GetColor( c, index );
			else					( Size() == 1 )? m_Value.GetValue( c ) : m_aValues[ index ].GetValue( c );
		}
		virtual void GetMatrix( CqMatrix& m, TqInt index = 0 ) const
		{
			if( m_pVarRef != NULL )	m_pVarRef->GetMatrix( m, index );
			else					( Size() == 1 )? m_Value.GetValue( m ) : m_aValues[ index ].GetValue( m );
		}

		// Value setters, overridden from IqShaderData
		virtual void SetFloat( const TqFloat& f )
		{
			*this = f;
		}
		virtual void SetBool( const TqBool& b )
		{
			*this = b;
		}
		virtual void SetString( const CqString& s )
		{
			*this = s;
		}
		virtual void SetPoint( const CqVector3D& p )
		{
			*this = p;
		}
		virtual void SetVector( const CqVector3D& v )
		{
			*this = v;
		}
		virtual void SetNormal( const CqVector3D& n )
		{
			*this = n;
		}
		virtual void SetColor( const CqColor& c )
		{
			*this = c;
		}
		virtual void SetMatrix( const CqMatrix& m )
		{
			*this = m;
		}


		// Value setters, overridden from IqShaderData
		virtual void SetFloat( const TqFloat& f, TqInt index )
		{
			if( m_pVarRef != NULL )	m_pVarRef->SetFloat( f, index );
			else					( Size() == 1 )? m_Value = f : m_aValues[index] = f;
		}
		virtual void SetBool( const TqBool& b, TqInt index )
		{
			if( m_pVarRef != NULL )	assert( false );
			else					( Size() == 1 )? m_Value = b : m_aValues[index] = b;
		}
		virtual void SetString( const CqString& s, TqInt index )
		{
			if( m_pVarRef != NULL )	m_pVarRef->SetString( s, index );
			else					( Size() == 1 )? m_Value = s : m_aValues[index] = s;
		}
		virtual void SetPoint( const CqVector3D& p, TqInt index )
		{
			if( m_pVarRef != NULL )	m_pVarRef->SetPoint( p, index );
			else					( Size() == 1 )? m_Value = p : m_aValues[index] = p;
		}
		virtual void SetVector( const CqVector3D& v, TqInt index )
		{
			if( m_pVarRef != NULL )	m_pVarRef->SetVector( v, index );
			else					( Size() == 1 )? m_Value = v : m_aValues[index] = v;
		}
		virtual void SetNormal( const CqVector3D& n, TqInt index )
		{
			if( m_pVarRef != NULL )	m_pVarRef->SetNormal( n, index );
			else					( Size() == 1 )? m_Value = n : m_aValues[index] = n;
		}
		virtual void SetColor( const CqColor& c, TqInt index )
		{
			if( m_pVarRef != NULL )	m_pVarRef->SetColor( c, index );
			else					( Size() == 1 )? m_Value = c : m_aValues[index] = c;
		}
		virtual void SetMatrix( const CqMatrix& m, TqInt index )
		{
			if( m_pVarRef != NULL )	m_pVarRef->SetMatrix( m, index );
			else					( Size() == 1 )? m_Value = m : m_aValues[index] = m;
		}


		virtual	const CqString&	strName()			{ return(m_strName); }

		virtual	void	Initialise( const TqInt uGridRes, const TqInt vGridRes )
		{
			m_Size = ( uGridRes + 1 ) * ( vGridRes + 1 );
			if ( m_aValues.size() < m_Size && m_Size > 1 )
				m_aValues.resize( m_Size );
		}

		virtual	IqShaderData* Clone() const
		{
			return( new CqVMStackEntry( *this ) );
		}

		virtual	void	SetValueFromVariable( IqShaderData* pVal, TqInt index )
		{
		}

		virtual	void	SetValueFromVariable( IqShaderData* pVal )
		{
		}

		virtual	EqVariableClass	Class() const
		{
			if( m_pVarRef != NULL )	return( m_pVarRef->Class() );
			else					return( ( m_Size == 1 )? class_uniform : class_varying );
		}

		virtual	EqVariableType	Type() const
		{
			if( m_pVarRef != NULL )	return( m_pVarRef->Type() );
			else					return( static_cast<EqVariableType>( ( m_Size == 1 )? m_Value.m_Type : m_aValues[0].m_Type ) );
		}

		virtual TqUint	Size() const;

		virtual TqInt	ArrayLength() const		
		{
			if( m_pVarRef != NULL )	return( m_pVarRef->ArrayLength() );
			else					return( 0 );
		}
		virtual IqShaderData*	ArrayEntry(TqInt i)
		{
			if( m_pVarRef != NULL )	return( m_pVarRef->ArrayEntry(i) );
			else					return( this ); 
		}
		

		/** Assigment operator.
		 */
		CqVMStackEntry&	operator=( const CqVMStackEntry& e )
		{
			if ( e.m_pVarRef != 0 )
				m_pVarRef = e.m_pVarRef;
			else
			{
				m_pVarRef = 0;
				m_Size = e.Size();
				if ( Size() == 1 )
					m_Value = e.m_Value;
				else
				{
					if(m_aValues.size() < m_Size )
						m_aValues.resize( m_Size );
					TqInt i;
					for ( i = Size() - 1; i >= 0; i-- )
						m_aValues[ i ] = e.m_aValues[ i ];
				}
			}
			return ( *this );
		}
		/** Assignment from a float.
		 * Takes care of casting to varying by duplication if neccessary.
		 */
		CqVMStackEntry&	operator=( TqFloat f )
		{
			m_pVarRef = 0;
			TqInt i;
			if ( Size() == 1 ) m_Value = f;
			else
				for ( i = Size() - 1; i >= 0; i-- ) m_aValues[ i ] = f;
			return ( *this );
		}
		/** Assignment from an int.
		 * Takes care of casting to varying by duplication if neccessary.
		 */
		CqVMStackEntry&	operator=( TqInt v )
		{
			m_pVarRef = 0;
			TqInt i;
			if ( Size() == 1 ) m_Value = v;
			else
				for ( i = Size() - 1; i >= 0; i-- ) m_aValues[ i ] = v;
			return ( *this );
		}
		/** Assignment from a boolean.
		 * Takes care of casting to varying by duplication if neccessary.
		 */
		CqVMStackEntry&	operator=( TqBool b )
		{
			m_pVarRef = 0;
			TqInt i;
			if ( Size() == 1 ) m_Value = b;
			else
				for ( i = Size() - 1; i >= 0; i-- ) m_aValues[ i ] = b;
			return ( *this );
		}
		/** Assignment from a 3D vector.
		 * Takes care of casting to varying by duplication if neccessary.
		 */
		CqVMStackEntry&	operator=( const CqVector3D& v )
		{
			m_pVarRef = 0;
			TqInt i;
			if ( Size() == 1 ) m_Value = v;
			else
				for ( i = Size() - 1; i >= 0; i-- ) m_aValues[ i ] = v;
			return ( *this );
		}
		/** Assignment from a color.
		 * Takes care of casting to varying by duplication if neccessary.
		 */
		CqVMStackEntry&	operator=( const CqColor& c )
		{
			m_pVarRef = 0;
			TqInt i;
			if ( Size() == 1 ) m_Value = c;
			else
				for ( i = Size() - 1; i >= 0; i-- ) m_aValues[ i ] = c;
			return ( *this );
		}
		/** Assignment from a char pointer.
		 * Takes care of casting to varying by duplication if neccessary.
		 */
		CqVMStackEntry&	operator=( const char* s )
		{
			m_pVarRef = 0;
			TqInt i;
			if ( Size() == 1 ) m_Value = s;
			else
				for ( i = Size() - 1; i >= 0; i-- ) m_aValues[ i ] = s;
			return ( *this );
		}
		/** Assignment from a string.
		 * Takes care of casting to varying by duplication if neccessary.
		 */
		CqVMStackEntry&	operator=( const CqString& s )
		{
			m_pVarRef = 0;
			TqInt i;
			if ( Size() == 1 ) m_Value = s;
			else
				for ( i = Size() - 1; i >= 0; i-- ) m_aValues[ i ] = s;
			return ( *this );
		}
		/** Assignment from a matrix.
		 * Takes care of casting to varying by duplication if neccessary.
		 */
		CqVMStackEntry&	operator=( const CqMatrix& m )
		{
			m_pVarRef = 0;
			TqInt i;
			if ( Size() == 1 ) m_Value = m;
			else
				for ( i = Size() - 1; i >= 0; i-- ) m_aValues[ i ] = m;
			return ( *this );
		}
		CqVMStackEntry&	operator=( IqShaderData* pv );

		/** Templatised less than operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
		 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
		 * \param Comp The stack entry to use as the second operand.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		template <class A, class B>
		void	OpLSS( A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;
			B vB;

			TqInt i = MAX( MAX( Size(), Comp.Size() ), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					Comp.GetValue( vB, i );
					Res.SetValue( vA < vB, i );
				}
		}
		/** Templatised greater than operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
		 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
		 * \param Comp The stack entry to use as the second operand.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		template <class A, class B>
		void	OpGRT( A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;
			B vB;

			TqInt i = MAX( MAX( Size(), Comp.Size() ), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					Comp.GetValue( vB, i );
					Res.SetValue( vA > vB, i );
				}
		}
		/** Templatised less than or equal to operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
		 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
		 * \param Comp The stack entry to use as the second operand.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		template <class A, class B>
		void	OpLE( A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;
			B vB;

			TqInt i = MAX( MAX( Size(), Comp.Size() ), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					Comp.GetValue( vB, i );
					Res.SetValue( vA <= vB, i );
				}
		}
		/** Templatised greater than or equal to operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
		 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
		 * \param Comp The stack entry to use as the second operand.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		template <class A, class B>
		void	OpGE( A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;
			B vB;

			TqInt i = MAX( MAX( Size(), Comp.Size() ), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					Comp.GetValue( vB, i );
					Res.SetValue( vA >= vB, i );
				}
		}
		/** Templatised equality operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
		 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
		 * \param Comp The stack entry to use as the second operand.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		template <class A, class B>
		void	OpEQ( A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;
			B vB;
			
			TqInt i = MAX( MAX( Size(), Comp.Size() ), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					Comp.GetValue( vB, i );
					Res.SetValue( vA == vB, i );
				}
		}
		/** Templatised inequality operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
		 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
		 * \param Comp The stack entry to use as the second operand.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		template <class A, class B>
		void	OpNE( A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;
			B vB;

			TqInt i = MAX( MAX( Size(), Comp.Size() ), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					Comp.GetValue( vB, i );
					Res.SetValue( vA != vB, i );
				}
		}
		/** Templatised multiplication operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
		 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
		 * \param Comp The stack entry to use as the second operand.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		template <class A, class B>
		void	OpMUL( A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;
			B vB;

			TqInt i = MAX( MAX( Size(), Comp.Size() ), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					Comp.GetValue( vB, i );
					Res.SetValue( vA * vB, i );
				}
		}
		/** Special case vector multiplication operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param Comp The stack entry to use as the second operand.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		void	OpMULV( CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			CqVector3D	vecA, vecB;

			TqInt i = MAX( MAX( Size(), Comp.Size() ), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vecA, i);
					Comp.GetValue( vecB, i);
					Res.SetValue( CqVector3D( vecA.x() * vecB.x(),
					                          vecA.y() * vecB.y(),
					                          vecA.z() * vecB.z() ), i );
				}
		}
		/** Templatised division operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
		 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
		 * \param Comp The stack entry to use as the second operand.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		template <class A, class B>
		void	OpDIV( A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;
			B vB;
			
			TqInt i = MAX( MAX( Size(), Comp.Size() ), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					Comp.GetValue( vB, i );
					Res.SetValue( vA / vB, i );
				}
		}
		/** Templatised addition operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
		 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
		 * \param Comp The stack entry to use as the second operand.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		template <class A, class B>
		void	OpADD( A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;
			B vB;
			
			TqInt i = MAX( MAX( Size(), Comp.Size() ), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					Comp.GetValue( vB, i );
					Res.SetValue( vA + vB, i );
				}
		}
		/** Templatised subtraction operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
		 * \param a The type of the second operand, used to determine templateisation, needed by VC++..
		 * \param Comp The stack entry to use as the second operand.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		template <class A, class B>
		void	OpSUB( A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;
			B vB;

			TqInt i = MAX( MAX( Size(), Comp.Size() ), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					Comp.GetValue( vB, i );
					Res.SetValue( vA - vB, i );
				}
		}
		/** Templatised dot operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
		 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
		 * \param Comp The stack entry to use as the second operand.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 * \attention Should only ever be called with vector based operands.
		 */
		template <class A, class B>
		void	OpDOT( A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;
			B vB;

			TqInt i = MAX( MAX( Size(), Comp.Size() ), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					Comp.GetValue( vB, i );
					Res.SetValue( vA * vB, i );
				}
		}
		/** Templatised cross product operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
		 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
		 * \param Comp The stack entry to use as the second operand.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 * \attention Should only ever be called with vector based operands.
		 */
		template <class A, class B>
		void	OpCRS( A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;
			B vB;
			
			TqInt i = MAX( MAX( Size(), Comp.Size() ), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					Comp.GetValue( vB, i );
					Res.SetValue( vA % vB, i );
				}
		}
		/** Templatised logical AND operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
		 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
		 * \param Comp The stack entry to use as the second operand.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		template <class A, class B>
		void	OpLAND( A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;
			B vB;

			TqInt i = MAX( MAX( Size(), Comp.Size() ), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					Comp.GetValue( vB, i );
					Res.SetValue( vA && vB, i );
				}
		}
		/** Templatised logical OR operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
		 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
		 * \param Comp The stack entry to use as the second operand.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		template <class A, class B>
		void	OpLOR( A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;
			B vB;

			TqInt i = MAX( MAX( Size(), Comp.Size() ), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					Comp.GetValue( vB, i );
					Res.SetValue( vA || vB, i );
				}
		}
		/** Templatised negation operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
		 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
		 * \param Comp The stack entry to use as the second operand.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		template <class A>
		void	OpNEG( A& a, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;

			TqInt i = MAX( Size(), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					Res.SetValue( -vA, i );
				}
		}
		/** Templatised cast operator, cast the current stack entry to the spcified type.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
		 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
		 * \param Comp The stack entry to use as the second operand.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		template <class A, class B>
		void	OpCAST( A& a, B& b, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;

			TqInt i = MAX( Size(), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					Res.SetValue( static_cast<B>( vA ), i );
				}
		}
		/** Templatised cast three operands to a single triple type (vector/normal/color etc.) and store the result in this stack entry
		 * \param z The type to combine the float values into.
		 * \param a Float first operand 
		 * \param b Float second operand 
		 * \param c Float third operand 
		 * \param RunningState The current SIMD state.
		 */
		template <class A>
		void	OpTRIPLE( A&, CqVMStackEntry& a, CqVMStackEntry& b, CqVMStackEntry& c, CqBitVector& RunningState )
		{
			TqFloat x,y,z;

			TqInt i = MAX( MAX( a.Size(), b.Size() ), c.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
					a.GetValue( x, i );
					b.GetValue( y, i );
					c.GetValue( z, i );
					SetValue( A( x, y, z ), i );
				}
		}
		/** Templatised cast sixteen operands to a single matrix type and store the result in this stack entry
		 * The parameters a-p are the float values to combine.
		 * \param z The type to combine the float values into (currenlty only matrix supported).
		 * \param RunningState The current SIMD state.
		 */
		template <class A>
		void	OpHEXTUPLE( A& z, CqVMStackEntry& a, CqVMStackEntry& b, CqVMStackEntry& c, CqVMStackEntry& d,
		                 CqVMStackEntry& e, CqVMStackEntry& f, CqVMStackEntry& g, CqVMStackEntry& h,
		                 CqVMStackEntry& i, CqVMStackEntry& j, CqVMStackEntry& k, CqVMStackEntry& l,
		                 CqVMStackEntry& m, CqVMStackEntry& n, CqVMStackEntry& o, CqVMStackEntry& p, CqBitVector& RunningState )
		{
			TqFloat a1,a2,a3,a4;
			TqFloat b1,b2,b3,b4;
			TqFloat c1,c2,c3,c4;
			TqFloat d1,d2,d3,d4;
			
			TqInt ii = MAX( MAX( a.Size(), b.Size() ), c.Size() ) - 1;
			TqBool __fVarying = ii > 0;
			for ( ; ii >= 0; ii-- )
			{
				if ( !__fVarying || RunningState.Value( ii ) )
				{
					a.GetValue( a1, ii );	b.GetValue( a2, ii );	c.GetValue( a3, ii );	d.GetValue( a4, ii );
					e.GetValue( b1, ii );	f.GetValue( b2, ii );	g.GetValue( b3, ii );	h.GetValue( b4, ii );
					i.GetValue( c1, ii );	j.GetValue( c2, ii );	k.GetValue( c3, ii );	l.GetValue( c4, ii );
					m.GetValue( d1, ii );	n.GetValue( d2, ii );	o.GetValue( d3, ii );	p.GetValue( d4, ii );
					A tt( a1, a2, a3, a4,
					      b1, b2, b3, b4,
					      c1, c2, c3, c4,
					      d1, d2, d3, d4);
					SetValue( tt, ii );
				}
			}
		}
		/** Templatised component access operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param z The type to cast this stackentry to.
		 * \param index Integer index. 
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		template <class A>
		void	OpCOMP( A& z, int index, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;
			
			TqInt i = MAX( Size(), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					Res.SetValue( vA [ index ], i );
				}
		}
		/** Templatised component access operator.
		 * The template classes decide the cast used, there must be an appropriate operator between the two types.
		 * \param z The type to cast this stackentry to.
		 * \param index Integer type stackentry index. 
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		template <class A>
		void	OpCOMP( A& z, CqVMStackEntry& index, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			A vA;
			TqFloat fi;

			TqInt i = MAX( MAX( Size(), Res.Size() ), index.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					index.GetValue( fi, i );
					Res.SetValue( vA [ static_cast<TqInt>( fi ) ], i );
				}
		}
		/** Templatised component set operator.
		 * \param z The type to cast this to.
		 * \param index Integer index. 
		 * \param a Float type stackentry to set the index to.
		 * \param RunningState The current SIMD state.
		 */
		template <class A>
		void	OpSETCOMP( A& z, int index, CqVMStackEntry& a, CqBitVector& RunningState )
		{
			A vA;
			TqFloat val;

			TqInt i = MAX( Size(), a.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
			{
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					a.GetValue( val, i );
					vA [ index ] = val;
					SetValue( vA, i );
				}
			}
		}
		/** Templatised component set operator.
		 * \param z The type to cast this to.
		 * \param index Integer type stackentry index. 
		 * \param a Float type stackentry to set the index to.
		 * \param RunningState The current SIMD state.
		 */
		template <class A>
		void	OpSETCOMP( A& z, CqVMStackEntry& index, CqVMStackEntry& a, CqBitVector& RunningState )
		{
			A vA;
			TqFloat val, fi;

			TqInt i = MAX( MAX( Size(), a.Size() ), index.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
			{
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( vA, i );
					a.GetValue( val, i );
					index.GetValue( fi, i );
					vA [ static_cast<TqInt>( fi ) ] = val;
					SetValue( vA, i );
				}
			}
		}

		/** Special case matrix component access.
		 * \param r Integer type stackentry row index.
		 * \param c Integer type stackentry column index.
		 * \param Res The stack entry to store the result in.
		 * \param RunningState The current SIMD state.
		 */
		void	OpCOMPM( CqVMStackEntry& r, CqVMStackEntry& c, CqVMStackEntry& Res, CqBitVector& RunningState )
		{
			CqMatrix m;
			TqFloat fr, fc;

			TqInt i = MAX( Size(), Res.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( m, i );
					r.GetValue( fr, i );
					c.GetValue( fc, i );
					Res.SetValue( m [ static_cast<TqInt>( fr ) ][ static_cast<TqInt>( fc ) ], i );
				}
		}

		/** Special case matrix component access.
		 * \param r Integer type stackentry row index.
		 * \param c Integer type stackentry column index.
		 * \param v Float type stackentry value to set index to.
		 * \param RunningState The current SIMD state.
		 */
		void	OpSETCOMPM( CqVMStackEntry& r, CqVMStackEntry& c, CqVMStackEntry& v, CqBitVector& RunningState )
		{
			CqMatrix m;
			TqFloat fr, fc, val;

			TqInt i = MAX( Size(), v.Size() ) - 1;
			TqBool __fVarying = i > 0;
			for ( ; i >= 0; i-- )
			{
				if ( !__fVarying || RunningState.Value( i ) )
				{
				GetValue( m, i );
					r.GetValue( fr, i );
					c.GetValue( fc, i );
					v.GetValue( val, i );
					m[ static_cast<TqInt>( fr ) ][ static_cast<TqInt>( fc ) ] = val;
					SetValue( m, i );
				}
			}
		}

	private:

		std::vector < SqVMStackEntry /*,CqVMStackEntryAllocator*/ > m_aValues;	///< Array of SqVMStackEntry storage stuctures.
		SqVMStackEntry	m_Value;		///< Single value in the case of a uniforn stack entry.
		IqShaderData*	m_pVarRef;		///< Pointer to a referenced variable if a variable stack entry.
		TqUint	m_Size;			///< Size of the SIMD data.

	static	CqString m_strName;
}
;



//----------------------------------------------------------------------
/** \class CqShaderStack
 * Class handling the shader execution stack.
 */

class _qShareC CqShaderStack
{
	public:
		_qShareM CqShaderStack() : m_iTop( 0 )
		{
			m_Stack.resize( 48 );
		}
		virtual _qShareM ~CqShaderStack()
		{
			m_Stack.clear();
		}

		/** Get a reference to the next item on the stack to be pushed.
		 */
		CqVMStackEntry& GetPush()
		{
			if ( m_iTop >= m_Stack.size() )
				m_Stack.resize( m_Stack.size() + 1 );

			return ( m_Stack[ m_iTop++ ] );
		}

		/** Push a new stack entry onto the stack.
		 */
		void	Push( const CqVMStackEntry& E )
		{
			GetPush() = E;
		}
		/** Push a new float onto the stack.
		 */
		void	Push( const TqFloat f )
		{
			CqVMStackEntry & s = GetPush();
			s.Initialise( 0, 0 );
			s = f;
		}
		/** Push a new boolean onto the stack.
		 */
		void	Push( const TqBool b )
		{
			CqVMStackEntry & s = GetPush();
			s.Initialise( 0, 0 );
			s = b;
		}
		/** Push a new 3D vector onto the stack.
		 */
		void	Push( const CqVector3D& v )
		{
			CqVMStackEntry & s = GetPush();
			s.Initialise( 0, 0 );
			s = v;
		}
		/** Push a new color onto the stack, passed as 3 floats.
		 */
		void	Push( const TqFloat r, TqFloat g, TqFloat b )
		{
			CqVMStackEntry & s = GetPush();
			s.Initialise( 0, 0 );
			s = CqColor( r, g, b );
		}
		/** Push a new color onto the stack.
		 */
		void	Push( const CqColor& c )
		{
			CqVMStackEntry & s = GetPush();
			s.Initialise( 0, 0 );
			s = c;
		}
		/** Push a new string onto the stack.
		 */
		void	Push( const CqString& str )
		{
			CqVMStackEntry & s = GetPush();
			s.Initialise( 0, 0 );
			s = str;
		}
		/** Push a new string, passed as a character onto the stack.
		 */
		void	Push( const char* ps )
		{
			CqVMStackEntry & s = GetPush();
			s.Initialise( 0, 0 );
			s = ps;
		}
		/** Push a new matrix onto the stack.
		 */
		void	Push( const CqMatrix& mat )
		{
			CqVMStackEntry & s = GetPush();
			s.Initialise( 0, 0 );
			s = mat;
		}
		/** Push a new shader variable reference onto the stack.
		 */
		void	Push( IqShaderData* pv )
		{
			CqVMStackEntry & s = GetPush();
			s = pv;
		}
		/** Pop the top stack entry.
		 * \param f Boolean value to update if this is varying. If not varying, leaves f unaltered.
		 * \return Reference to the top stack entry.
		 */
		CqVMStackEntry& Pop( TqBool& f )
		{
			if ( m_iTop ) m_iTop--;
			CqVMStackEntry& val = m_Stack[ m_iTop ];
			f = val.Size() > 1 || f;
			return ( val );
		}
		/** Duplicate the top stack entry.
		 */
		void	Dup()
		{
			TqInt iTop = m_iTop;
			GetPush() = m_Stack[ iTop ];
		}
		/** Drop the top stack entry.
		 */
		void	Drop()
		{
			if ( m_iTop )
				m_iTop--;
		}
	protected:
		std::vector<CqVMStackEntry>	m_Stack;		///< Array of stacke entries.
		TqUint	m_iTop;			///< Index of the top entry.
}
;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !SHADERSTACK_H_INCLUDED
