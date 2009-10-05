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
		\brief Declares classes and support functionality for the shader virtual machine.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef SHADERVM_H_INCLUDED
#define SHADERVM_H_INCLUDED 1

#include	<vector>
#include	<list>
#include	<boost/shared_ptr.hpp>

#include	<aqsis/aqsis.h>

#include	<aqsis/util/bitvector.h>
#include	<aqsis/math/vector3d.h>
#include	<aqsis/math/color.h>
#include	<aqsis/util/sstring.h>
#include	<aqsis/math/matrix.h>
#include	<aqsis/math/noise.h>
#include	<aqsis/shadervm/ishaderdata.h>
#include	<aqsis/shadervm/ishader.h>
#include	<aqsis/core/irenderer.h>
#include	<aqsis/core/iparameter.h>
#include	"shaderexecenv.h"
#include	"shaderstack.h"
#include	"shadervariable.h"
//#include 	"parameters.h"
#include 	"dsoshadeops.h"
#include	<aqsis/core/itransform.h>
#include	"shadervm_common.h"


namespace Aqsis {


class CqShaderVM;

//----------------------------------------------------------------------
/** \struct SqOpCodeTrans
 * Structure for translating an opcode in an slx file.
 */

struct SqOpCodeTrans
{
	const TqChar*	m_strName;		///< Name of the opcode.
	TqUlong	m_hash;				///< Hash key of the opcode.
	void (CqShaderVM::*m_pCommand ) ();	//< Member pointer to the function.
	TqInt	m_cParams;			///< Number of expected parameters.
	TqInt	m_aParamTypes[10];		///< Array of parameter types (up to ten, can be extended if required).
}
;


class CqShaderVM;
union UsProgramElement;


/** \struct SqLabel
 * Storege for label during slx file reading.
 */

struct SqLabel
{
	UsProgramElement* m_pAddress;	///< The absolute address of the label.
	TqInt	m_Offset;				///< The index offset fromt he start of the program.
}
;


//----------------------------------------------------------------------
/** \union UsProgramElement
 * Union describing the possible types of a single program element.
 */

union UsProgramElement
{
	void( CqShaderVM::*m_Command ) ();		///< Pointer to a function.
	TqFloat	m_FloatVal;				///< Absolute float value.
	TqInt	m_intVal;				///< Absolute int value.
	CqString*	m_pString;			///< Absolute string value.
	TqInt	m_iVariable;				///< Shader variable index.
	SqLabel	m_Label;				///< Program label.
	SqDSOExternalCall *m_pExtCall	;		///< Call a DSO function
};

//----------------------------------------------------------------------
/** \class CqShaderVM
 * Main class handling the execution of a program in shader language bytecodes.
 */

class AQSIS_SHADERVM_SHARE CqShaderVM : public CqShaderStack, public IqShader, public CqDSORepository
{
	public:
		CqShaderVM(IqRenderer* pRenderContext);
		CqShaderVM(const CqShaderVM& From);
		virtual ~CqShaderVM();

		// Overidden from IqShader
		virtual	const CqMatrix&	matCurrent()
		{
			/// \note: We can use time 0 here as shaders aren't affected by motion (currently).
			return ( m_pTransform->matObjectToWorld(0) );
		}
		virtual	const IqTransform*	getTransform() const
		{
			/// \note: We can use time 0 here as shaders aren't affected by motion (currently).
			return ( m_pTransform.get() );
		}

		virtual void SetTransform(IqTransformPtr pTrans)
		{
			m_pTransform = pTrans;
		}

		virtual	void	SetstrName( const char* strName )
		{
			m_strName = strName;
		}
		virtual	const CqString& strName() const
		{
			return ( m_strName );
		}
		virtual	void	PrepareShaderForUse( );
		virtual	void	InitialiseParameters( );
		virtual void	SetArgument( const CqString& strName, EqVariableType type, const CqString& strSpace, void* pval);
		virtual	void	SetArgument( IqParameter* pParam, IqSurface* pSurface );
		virtual	IqShaderData*	FindArgument( const CqString& name );
		virtual	bool	GetVariableValue( const char* name, IqShaderData* res ) const;
		virtual	void	Evaluate( IqShaderExecEnv* pEnv )
		{
			Execute( pEnv );
		}
		virtual	void	PrepareDefArgs()
		{
			ExecuteInit();
		}
		virtual void	Initialise( const TqInt uGridRes, const TqInt vGridRes, const TqInt shadingPointCount, IqShaderExecEnv* pEnv );
		virtual	bool	fAmbient() const
		{
			return ( m_fAmbient );
		}
		virtual	boost::shared_ptr<IqShader> Clone() const
		{
			return boost::shared_ptr<IqShader>(new CqShaderVM(*this));
		}
		virtual bool	Uses( TqInt Var ) const
		{
			assert( Var >= 0 && Var < EnvVars_Last );
			return ( Uses( static_cast<EqEnvVars>( Var ) ) );
		}
		virtual TqInt	Uses() const
		{
			return ( m_Uses );
		}
		virtual IqShaderData* CreateVariable( EqVariableType Type, EqVariableClass Class, const CqString& name, bool fArgument = false, bool fOutput = false  );
		virtual IqShaderData* CreateVariableArray( EqVariableType Type, EqVariableClass Class, const CqString& name, TqInt Count, bool fParameter = false, bool fOutput = false  );
		virtual	IqShaderData* CreateTemporaryStorage( EqVariableType type, EqVariableClass _class );
		virtual void DeleteTemporaryStorage( IqShaderData* pData );
		virtual void DefaultSurface();
		virtual	bool IsLayered()
		{
			return(false);
		}
		void AddLayer(const CqString& layername, const boost::shared_ptr<IqShader>& layer)
		{}
		virtual void AddConnection(const CqString& layer1, const CqString& variable1, const CqString& layer2, const CqString& variable2)
		{}

		/// \todo: These should be exposed by the IqShader Interface somehow.
		static	void ShutdownShaderEngine();


		/** \brief Load a compiled shader program from the given stream
		 *
		 * \throw XqBadShader If the program was compiled with a different
		 *   version of aqsis, or is invalid in any other way.
		 */
		void	LoadProgram( std::istream* pFile );
		void	Execute( IqShaderExecEnv* pEnv );
		void	ExecuteInit();

		virtual const std::vector<IqShaderData*>& GetArguments() const;
		void SetType(EqShaderType type);

		virtual EqShaderType Type() const
		{
			return ( m_Type );
		}


		/** Assignment operator.
		 */
		CqShaderVM&	operator=( const CqShaderVM& From );

	private:

		struct SqArgumentRecord
		{
			IqShaderData* m_Value;
			CqString	m_strSpace;
			CqString	m_strName;
		};

		TqInt	m_Uses;			///< Bit vector representing the system variables used by this shader.
		CqString	m_strName;		///< The name of this shader.

		EqShaderType m_Type;							///< Shader type for libslxarge
		TqUint	m_LocalIndex;                   ///<  Local Index to speed up
		IqShaderExecEnv* m_pEnv;							///< Pointer to the current excution environment.
		IqTransformPtr m_pTransform;    ///< Pointer to the transformation at the time the shader was instantiated.

		std::vector<IqShaderData*>	m_LocalVars;		///< Array of local variables.
		std::vector<SqArgumentRecord>	m_StoredArguments;		///< Array of arguments specified during construction.
		std::vector<UsProgramElement>	m_ProgramInit;		///< Bytecodes of the intialisation program.
		std::vector<UsProgramElement>	m_Program;			///< Bytecodes of the main program.
		std::list<CqString*>			m_ProgramStrings;	///< Strings used by the program, which are stored additionally as UsProgramElements.
		TqInt	m_uGridRes;
		TqInt	m_vGridRes;
		TqInt	m_shadingPointCount;
		UsProgramElement*	m_PC;							///< Current program pointer.
		TqInt	m_PO;							///< Current program offset.
		TqInt	m_PE;							///< Offset of the end of the program.
		bool	m_fAmbient;						///< Flag indicating if this is an ambient light source ( if it is indeed a light source ).
		bool	m_outsideWorld;						///< Flag indicating this shader was declared outside the world.
		IqRenderer*	m_pRenderContext;


		/** \brief Get a string from a program file and interpret escaped chars
		 *
		 * Assumes the leading quote (") has already been removed from the
		 * input, and reads up until the trailing quote
		 *
		 * \param pFile shader stream
		 * \return String
		 */
		static CqString GetString(std::istream* pFile);
		/** \brief Read in and interpret a numeric escape sequence as a character
		 *
		 * \param pFile shader stream
		 * \param s result string
		 * \param c, current character
		 */
		static void GetNumericEscapeChar(std::istream* pFile, CqString &s, char c);
		/** Determine whether the program execution has finished.
		 */
		bool	fDone()
		{
			return ( m_PO >= m_PE );
		}
		/** Get the next program element from storage.
		 * \return Reference to the next program element.
		 */
		UsProgramElement&	ReadNext()
		{
			m_PO++;
			return ( *m_PC++ );
		}
		/** Get a shader variable by index.
		 * \param Index Integer index, top bit indicates system variable.
		 * \return Pointer to a IqShaderData derived class.
		 */
		IqShaderData*	GetVar( TqInt Index )
		{
			if ( Index & 0x8000 )
				return ( m_pEnv->pVar( Index & 0x7FFF ) );
			else
				return ( m_LocalVars[ Index ] );
		}
		/** Add a variable to the list of local ones.
		 * \param pVar Pointer to a IqShaderData derived class.
		 */
		void	AddLocalVariable( IqShaderData* pVar )
		{
			m_LocalVars.push_back( pVar );
		}
		/** Find the index of a named shader variable.
		 * \param strName Character pointer to the name.
		 * \return Index of local variable or -1.
		 */
		TqInt	FindLocalVarIndex( const char* strName )
		{
			TqUint tmp = m_LocalIndex;
			TqUlong hash = CqString::hash(strName);

			for ( ; m_LocalIndex < m_LocalVars.size(); m_LocalIndex++ )
				if ( CqString::hash(m_LocalVars[ m_LocalIndex ] ->strName().c_str()) == hash )
					return ( m_LocalIndex );

			for ( m_LocalIndex = 0; m_LocalIndex < tmp; m_LocalIndex++ )
				if ( CqString::hash(m_LocalVars[ m_LocalIndex ] ->strName().c_str()) == hash )
					return ( m_LocalIndex );
			return ( -1 );
		}
		/** Find the index of a named shader variable.
		 * \param strName Character pointer to the name.
		 * \return Index of local variable or -1.
		 */
		TqInt	FindLocalVarIndex( const char* strName ) const
		{
			TqUlong hash = CqString::hash(strName);

			for ( TqUint m = 0; m < m_LocalVars.size(); m++ )
				if ( CqString::hash(m_LocalVars[ m ] ->strName().c_str()) == hash )
					return ( m );
			return ( -1 );
		}
		void	GetToken( char* token, TqInt l, std::istream* pFile );

		/** Add a command to the program data area.
		 * \param pCommand Pointer to the opcode function.
		 * \param pProgramArea Pointer to the program area, either init, or main code areas.
		 */
		void	AddCommand( void( CqShaderVM::*pCommand ) (), std::vector<UsProgramElement>* pProgramArea )
		{
			UsProgramElement E;
			E.m_Command = pCommand;
			pProgramArea->push_back( E );
		}
		/** Add an absolute float value to the program data area.
		 * \param f Float value to add.
		 * \param pProgramArea Pointer to the program area, either init, or main code areas.
		 */
		void	AddFloat( TqFloat f, std::vector<UsProgramElement>* pProgramArea )
		{
			UsProgramElement E;
			E.m_FloatVal = f;
			pProgramArea->push_back( E );
		}
		/** Add an absolute integer value to the program data area.
		 * \param i Integer value to add.
		 * \param pProgramArea Pointer to the program area, either init, or main code areas.
		 */
		void AddInteger(TqInt i, std::vector<UsProgramElement>* pProgramArea)
		{
			UsProgramElement E;
			E.m_intVal = i;
			pProgramArea->push_back( E );
		}
		/** Add an absolute string value to the program data area.
		 * \param s Character pointer value to add.
		 * \param pProgramArea Pointer to the program area, either init, or main code areas.
		 */
		void	AddString( const char* s, std::vector<UsProgramElement>* pProgramArea )
		{
			CqString* ps = new CqString( s );
			UsProgramElement E;
			E.m_pString = ps;
			pProgramArea->push_back( E );
			m_ProgramStrings.push_back( ps ); // Store here as well to avoid mem leak.
		}
		/** Add an variable index value to the program area.
		 * \param iVar Integer variable index to add, top bit indicates system variable.
		 * \param pProgramArea Pointer to the program area, either init, or main code areas.
		 */
		void	AddVariable( TqInt iVar, std::vector<UsProgramElement>* pProgramArea )
		{
			UsProgramElement E;
			E.m_iVariable = iVar;
			pProgramArea->push_back( E );
		}
		/** Add an an external call descriptor to the program area
		 * \param pExtCall Pointer to the external call descriptor.
		 * \param pProgramArea Pointer to the program area, either init, or main code areas.
		 */
		void	AddDSOExternalCall( SqDSOExternalCall *pExtCall, std::vector<UsProgramElement>* pProgramArea )
		{
			UsProgramElement E;
			E.m_pExtCall = pExtCall;
			pProgramArea->push_back( E );
		}

		void	SO_nop();
		void	SO_dup();
		void	SO_drop();
		void	SO_debug_break();
		void	SO_pushif();
		void	SO_puship();
		void	SO_pushis();
		void	SO_pushv();
		void	SO_ipushv();
		void	SO_pop();
		void	SO_ipop();
		void	SO_mergef();
		void	SO_merges();
		void	SO_mergep();
		void	SO_mergec();
		void	SO_setfc();
		void	SO_setfp();
		void	SO_setfm();
		void	SO_settc();
		void	SO_settp();
		void	SO_setpc();
		void	SO_setcp();
		void	SO_setwm();
		void	SO_RS_PUSH();
		void	SO_RS_POP();
		void	SO_RS_GET();
		void	SO_RS_INVERSE();
		void	SO_S_CLEAR();
		void	SO_S_GET();
		void	SO_RS_JZ();
		void	SO_RS_BREAK();
		void	SO_S_JZ();
		void	SO_jnz();
		void	SO_jz();
		void	SO_jmp();
		void	SO_lsff();
		void	SO_lspp();
		void	SO_lscc();
		void	SO_gtff();
		void	SO_gtpp();
		void	SO_gtcc();
		void	SO_geff();
		void	SO_gepp();
		void	SO_gecc();
		void	SO_leff();
		void	SO_lepp();
		void	SO_lecc();
		void	SO_eqff();
		void	SO_eqpp();
		void	SO_eqcc();
		void	SO_eqss();
		void	SO_eqmm();
		void	SO_neff();
		void	SO_nepp();
		void	SO_necc();
		void	SO_ness();
		void	SO_nemm();
		void	SO_mulff();
		void	SO_divff();
		void	SO_addff();
		void	SO_subff();
		void	SO_negf();
		void	SO_mulpp();
		void	SO_divpp();
		void	SO_addpp();
		void	SO_subpp();
		void	SO_crspp();
		void	SO_dotpp();
		void	SO_negp();
		void	SO_mulcc();
		void	SO_divcc();
		void	SO_addcc();
		void	SO_subcc();
		void	SO_dotcc();
		void	SO_negc();
		void	SO_mulfp();
		void	SO_divfp();
		void	SO_addfp();
		void	SO_subfp();
		void	SO_mulfc();
		void	SO_divfc();
		void	SO_addfc();
		void	SO_subfc();
		void	SO_mulmm();
		void	SO_divmm();
		void	SO_land();
		void	SO_lor();
		void	SO_radians();
		void	SO_degrees();
		void	SO_sin();
		void	SO_asin();
		void	SO_cos();
		void	SO_acos();
		void	SO_tan();
		void	SO_atan();
		void	SO_atan2();
		void	SO_pow();
		void	SO_exp();
		void	SO_sqrt();
		void	SO_log();
		void	SO_log2();
		void	SO_mod();
		void	SO_abs();
		void	SO_sign();
		void	SO_min();
		void	SO_max();
		void	SO_pmin();
		void	SO_pmax();
		void	SO_vmin();
		void	SO_vmax();
		void	SO_nmin();
		void	SO_nmax();
		void	SO_cmin();
		void	SO_cmax();
		void	SO_clamp();
		void	SO_pclamp();
		void	SO_cclamp();
		void	SO_floor();
		void	SO_ceil();
		void	SO_round();
		void	SO_step();
		void	SO_smoothstep();
		void	SO_fspline();
		void	SO_cspline();
		void	SO_pspline();
		void	SO_sfspline();
		void	SO_scspline();
		void	SO_spspline();
		void	SO_fDu();
		void	SO_fDv();
		void	SO_fDeriv();
		void	SO_cDu();
		void	SO_cDv();
		void	SO_cDeriv();
		void	SO_pDu();
		void	SO_pDv();
		void	SO_pDeriv();
		void	SO_frandom();
		void	SO_crandom();
		void	SO_prandom();
		void	SO_noise1();
		void	SO_noise2();
		void	SO_noise3();
		void	SO_noise4();
		void	SO_cnoise1();
		void	SO_cnoise2();
		void	SO_cnoise3();
		void	SO_cnoise4();
		void	SO_pnoise1();
		void	SO_pnoise2();
		void	SO_pnoise3();
		void	SO_pnoise4();
		void	SO_xcomp();
		void	SO_ycomp();
		void	SO_zcomp();
		void	SO_setxcomp();
		void	SO_setycomp();
		void	SO_setzcomp();
		void	SO_length();
		void	SO_distance();
		void	SO_area();
		void	SO_normalize();
		void	SO_faceforward();
		void	SO_faceforward2();
		void	SO_reflect();
		void	SO_refract();
		void	SO_fresnel();
		void	SO_fresnel2();
		void	SO_transform2();
		void	SO_transform();
		void	SO_transformm();
		void	SO_vtransform2();
		void	SO_vtransform();
		void	SO_vtransformm();
		void	SO_ntransform2();
		void	SO_ntransform();
		void	SO_ntransformm();
		void	SO_mtransform2();
		void	SO_mtransform();
		void	SO_depth();
		void	SO_calculatenormal();
		void	SO_comp();
		void	SO_setcomp();
		void	SO_cmix();
		void	SO_cmixc();
		void	SO_fmix();
		void	SO_pmix();
		void	SO_vmix();
		void	SO_nmix();
		void	SO_pmixc();
		void	SO_vmixc();
		void	SO_nmixc();
		void	SO_ambient();
		void	SO_diffuse();
		void	SO_specular();
		void	SO_phong();
		void	SO_trace();
		void	SO_shadow();
		void	SO_shadow1();
		void	SO_ftexture1();
		void	SO_ftexture2();
		void	SO_ftexture3();
		void	SO_textureinfo();
		void	SO_ctexture1();
		void	SO_ctexture2();
		void	SO_ctexture3();
		void	SO_fenvironment2();
		void	SO_fenvironment3();
		void	SO_cenvironment2();
		void	SO_cenvironment3();
		void	SO_bump1();
		void	SO_bump2();
		void	SO_bump3();
		void	SO_illuminate();
		void	SO_illuminate2();
		void	SO_init_illuminance();
		void	SO_init_illuminance2();
		void	SO_advance_illuminance();
		void	SO_illuminance();
		void	SO_illuminance2();
		void	SO_solar();
		void	SO_solar2();
		void	SO_init_gather();
		void	SO_advance_gather();
		void	SO_gather();
		void	SO_printf();
		void	SO_atmosphere();
		void	SO_displacement();
		void	SO_lightsource();
		void	SO_surface();
		void	SO_attribute();
		void	SO_option();
		void	SO_rendererinfo();
		void	SO_incident();
		void	SO_opposite();
		void	SO_fcellnoise1();
		void	SO_fcellnoise2();
		void	SO_fcellnoise3();
		void	SO_fcellnoise4();
		void	SO_ccellnoise1();
		void	SO_ccellnoise2();
		void	SO_ccellnoise3();
		void	SO_ccellnoise4();
		void	SO_pcellnoise1();
		void	SO_pcellnoise2();
		void	SO_pcellnoise3();
		void	SO_pcellnoise4();
		void	SO_fpnoise1();
		void	SO_fpnoise2();
		void	SO_fpnoise3();
		void	SO_fpnoise4();
		void	SO_cpnoise1();
		void	SO_cpnoise2();
		void	SO_cpnoise3();
		void	SO_cpnoise4();
		void	SO_ppnoise1();
		void	SO_ppnoise2();
		void	SO_ppnoise3();
		void	SO_ppnoise4();
		void	SO_ctransform2();
		void	SO_ctransform();
		void	SO_ptlined();
		void	SO_inversesqrt();
		void	SO_concat();
		void	SO_format();
		void	SO_match();
		void	SO_rotate();
		void	SO_filterstep();
		void	SO_filterstep2();
		void	SO_specularbrdf();
		void	SO_mcomp();
		void	SO_setmcomp();
		void	SO_determinant();
		void	SO_mtranslate();
		void	SO_mrotate();
		void	SO_mscale();
		void	SO_fsplinea();
		void	SO_csplinea();
		void	SO_psplinea();
		void	SO_sfsplinea();
		void	SO_scsplinea();
		void	SO_spsplinea();
		void	SO_shadername();
		void	SO_shadername2();
		void	SO_bake_f();
		void	SO_bake_3c();
		void	SO_bake_3p();
		void	SO_bake_3v();
		void	SO_bake_3n();
		void	SO_external();
		void	SO_occlusion();
		void	SO_occlusion_rt();
		void	SO_rayinfo();
		void	SO_bake3d();
		void	SO_texture3d();
      
		static	SqOpCodeTrans	m_TransTable[];		///< Static opcode translation table.
		static	TqInt	m_cTransSize;		///< Size of translation table.
}
;


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !SHADERVM_H_INCLUDED
