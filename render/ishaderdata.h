//------------------------------------------------------------------------------
/**
 *	@file	ishaderdata.h
 *	@author	Paul C. Gregory
 *	@brief	Declare the interface that shader data encapsulation classes must implement.
 *
 *	Last change by:		$Author: pgregory $
 *	Last change date:	$Date: 2002/04/16 07:16:49 $
 */
//------------------------------------------------------------------------------

#ifndef	___ishaderdata_Loaded___
#define	___ishaderdata_Loaded___

#include	"aqsis.h"


START_NAMESPACE( Aqsis )


struct IqShaderVariable;

struct IqShaderData
{
		///	Get the data as a float..
		virtual void GetFloat( TqFloat& res, TqInt index ) const = 0;
		///	Get the data as an integer..
		virtual void GetInteger( TqInt& res, TqInt index ) const = 0;
		///	Get the data as a boolean value..
		virtual void GetBool( TqBool& res, TqInt index ) const = 0;
		///	Get the data as a string..
		virtual void GetString( CqString& res, TqInt index ) const = 0;
		///	Get the data as a point..
		virtual void GetPoint( CqVector3D& res, TqInt index ) const = 0;
		///	Get the data as a vector..
		virtual void GetVector( CqVector3D& res, TqInt index ) const = 0;
		///	Get the data as a normal..
		virtual void GetNormal( CqVector3D& res, TqInt index ) const = 0;
		///	Get the data as a homogeneous 4D point..
		virtual void GetHPoint( CqVector4D& res, TqInt index ) const = 0;
		///	Get the data as a color..
		virtual void GetColor( CqColor& res, TqInt index ) const = 0;
		///	Get the data as a matrix..
		virtual void GetMatrix( CqMatrix& res, TqInt index ) const = 0;

		///	Set the value to the specified float.
		virtual void SetFloat( const TqFloat& val, TqInt index ) = 0;
		///	Set the value to the specified integer.
		virtual void SetInteger( const TqInt& val, TqInt index ) = 0;
		///	Set the value to the specified boolean value.
		virtual void SetBool( const TqBool& val, TqInt index ) = 0;
		///	Set the value to the specified string.
		virtual void SetString( const CqString& val, TqInt index ) = 0;
		///	Set the value to the specified point.
		virtual void SetPoint( const CqVector3D& val, TqInt index ) = 0;
		///	Set the value to the specified vector.
		virtual void SetVector( const CqVector3D& val, TqInt index ) = 0;
		///	Set the value to the specified normal.
		virtual void SetNormal( const CqVector3D& val, TqInt index ) = 0;
		///	Set the value to the specified homogeneous 4D point.
		virtual void SetHPoint( const CqVector4D& val, TqInt index ) = 0;
		///	Set the value to the specified color.
		virtual void SetColor( const CqColor& val, TqInt index ) = 0;
		///	Set the value to the specified matrix.
		virtual void SetMatrix( const CqMatrix& val, TqInt index ) = 0;

		/** Determine whether the value is varying or uniform.
		 */
		virtual	TqBool	fVarying() const = 0;
		virtual TqInt	Size() const = 0;;
		/** Set the size of the SIMD array.
		 * \param size The new size.
		 */
		virtual	void	SetSize( TqUint size ) = 0;
		/** Determine whether the stack entry is in fact a shader variable reference.
		 */
		virtual	TqBool	fVariable() const = 0;
		/** Get a pointer to the referenced shader variable.
		 * Only valid if fVariable returns TqTrue.
		 * \return A pointer to a IqShaderVariable.
		 */
		virtual	IqShaderVariable*	pVariable() const = 0;
		/** Set the pointer to the referenced shader variable.
		 */
		virtual	void	SetpVariable( IqShaderVariable* pv ) = 0;
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	//	___ishaderdata_Loaded___
