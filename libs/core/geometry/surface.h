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
		\brief Declares the base GPrim handling classes.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef SURFACE_H_INCLUDED
//{
#define SURFACE_H_INCLUDED 1

#include	<aqsis/aqsis.h>
#include	<boost/enable_shared_from_this.hpp>
#include	<boost/utility.hpp>

#include	"attributes.h"
#include	"renderer.h"
#include	<aqsis/ri/ri.h>
#include	"transform.h"
#include	<aqsis/util/list.h>
#include	<aqsis/math/matrix.h>
#include	"parameters.h"
#include	"bound.h"
#include	"csgtree.h"
#include	<aqsis/core/isurface.h>
#include	<aqsis/util/logging.h>
#include	"stats.h"
#include	"micropolygon.h"

namespace Aqsis {


//----------------------------------------------------------------------
/** \class CqSurface
 * Abstract base surface class, which provides interfaces to geometry.  
 */

class CqSurface : public IqSurface, private boost::noncopyable, public boost::enable_shared_from_this<CqSurface>
{
	public:
		CqSurface();

		virtual	~CqSurface()
		{
			std::vector<CqParameter*>::iterator iUP;
			for ( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
				if ( NULL != ( *iUP ) )
					delete( *iUP );
			STATS_DEC( GPR_current );
		}

#ifdef _DEBUG
		CqString className() const
		{
			return CqString("CqSurface");
		}
#endif

		enum EqSplitDir
		{
		    SplitDir_U,
		    SplitDir_V,
		};


		virtual	void	Reset()
		{}

		virtual bool	IsMotionBlurMatch( CqSurface* pSurf ) = 0;

		virtual CqString	strName() const;
		virtual	TqInt	Uses() const;

		/**
		* \todo Review: Unused parameter pGrid
		*/
		virtual TqInt	DiceAll( CqMicroPolyGrid* pGrid )
		{
			return(0);
		}

		virtual void	RenderComplete()
		{}

		/** Get the value of the dice size in u, determined during a Diceable() call
		 */
		TqInt uDiceSize() const
		{
			return( m_uDiceSize );
		}

		/** Get the value of the dice size in v, determined during a Diceable() call
		 */
		TqInt vDiceSize() const
		{
			return( m_vDiceSize );
		}

		/** Get the surface paramter values for the given vertex index. Used when constructing a surface
		 * using "Pz" point specification.
		 * \todo Review: Unused parameter index
		 */
		virtual CqVector3D	SurfaceParametersAtVertex( TqInt index )
		{
			return ( CqVector3D( 0, 0, 0 ) );
		}

		/** Get a pointer to the attributes state associated with this GPrim.
		 * \return A pointer to a CqAttributes class.
		 */
		virtual IqAttributesPtr pAttributes() const
		{
			return ( m_pAttributes );
		}
		/** Get a pointer to the transformation state associated with this GPrim.
		 * \return A pointer to a CqTransform class.
		 */
		virtual IqTransformPtr pTransform() const
		{
			return ( boost::static_pointer_cast<IqTransform>( m_pTransform ) );
		}
		virtual	void	SetSurfaceParameters( const CqSurface& From );
		/** Force this GPrim to be undiceable, usually if it crosses the epsilon and eye plane.
		 */
		virtual	void	ForceUndiceable()
		{
			m_fDiceable = false;
		}
		/** Query if this primitive has been marked as undiceable by the eyesplit check.
		 */
		virtual bool	IsUndiceable() const
		{
			return ( !m_fDiceable );
		}
		/** Force this GPrim to be discarded, usually if it has been split too many times due to crossing the epsilon and eye planes..
		 */
		virtual	void	Discard()
		{
			m_fDiscard = true;
		}
		/** Copy the information about splitting and dicing from the specified GPrim.
		 * \param From A CqSurface reference to copy the information from.
		 */
		virtual void CopySplitInfo( const CqSurface* From )
		{
			m_uDiceSize = From->m_uDiceSize;
			m_vDiceSize = From->m_vDiceSize;
			m_SplitDir = From->m_SplitDir;
		}

		/** Determine whether this GPrim is to be discardrd.
		 */
		bool	fDiscard() const
		{
			return ( m_fDiscard );
		}
		/** Get the number of times this GPrim has been split because if crossing the epsilon and eye planes.
		 */
		TqInt	SplitCount() const
		{
			return ( m_SplitCount );
		}
		void	SetSplitCount( TqInt SplitCount )
		{
			m_SplitCount = SplitCount;
		}
		/** Get the precalculated split direction.
		 */
		TqInt	SplitDir() const
		{
			return ( m_SplitDir );
		}
		/** Set the precalculated split direction.
		 */
		void	SetSplitDir( EqSplitDir SplitDir )
		{
			m_SplitDir = SplitDir;
		}

		/** Cache the calculated bound for further reference
		 * \param pBound The calculated bound in hybrid raster/camera space
		 */
		void CacheRasterBound( CqBound& pBound )
		{
			m_Bound = pBound;
			m_CachedBound = true;
		}
		/** Retrieve the cached bound. assert() if it has never been cached.
		 * \return The object bound in hybrid raster/camera space
		 */
		CqBound	GetCachedRasterBound()
		{
			assert(m_CachedBound || !m_fDiceable);
			return m_Bound;
		}

		bool	fCachedBound() const
		{
			return ( m_CachedBound );
		}
		void	AdjustBoundForTransformationMotion( CqBound* B ) const;

		boost::shared_ptr<CqCSGTreeNode>& pCSGNode()
		{
			return ( m_pCSGNode );
		}
 		virtual CqSurface* Clone() const = 0;


		virtual	void	SetDefaultPrimitiveVariables( bool bUseDef_st = true );

		void ClonePrimitiveVariables( const CqSurface& From );

		/** Get a reference the to P default parameter.
		 */
		virtual CqParameterTyped<CqVector4D, CqVector3D>* P()
		{
			if ( m_aiStdPrimitiveVars[ EnvVars_P ] >= 0 )
				return ( static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( m_aUserParams[ m_aiStdPrimitiveVars[ EnvVars_P ] ] ) );
			else
				return ( NULL );
		}
		/** Get a reference the to N default parameter.
		 */
		virtual CqParameterTyped<CqVector3D, CqVector3D>* N()
		{
			if ( m_aiStdPrimitiveVars[ EnvVars_N ] >= 0 )
				return ( static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( m_aUserParams[ m_aiStdPrimitiveVars[ EnvVars_N ] ] ) );
			else
				return ( NULL );
		}
		/** Get a reference the to Cq default parameter.
		 */
		virtual CqParameterTyped<CqColor, CqColor>* Cs()
		{
			if ( m_aiStdPrimitiveVars[ EnvVars_Cs ] >= 0 )
				return ( static_cast<CqParameterTyped<CqColor, CqColor>*>( m_aUserParams[ m_aiStdPrimitiveVars[ EnvVars_Cs ] ] ) );
			else
				return ( NULL );
		}
		/** Get a reference the to Os default parameter.
		 */
		virtual CqParameterTyped<CqColor, CqColor>* Os()
		{
			if ( m_aiStdPrimitiveVars[ EnvVars_Os ] >= 0 )
				return ( static_cast<CqParameterTyped<CqColor, CqColor>*>( m_aUserParams[ m_aiStdPrimitiveVars[ EnvVars_Os ] ] ) );
			else
				return ( NULL );
		}
		/** Get a reference the to s default parameter.
		 */
		virtual CqParameterTyped<TqFloat, TqFloat>* s()
		{
			if ( m_aiStdPrimitiveVars[ EnvVars_s ] >= 0 )
				return ( static_cast<CqParameterTyped<TqFloat, TqFloat>*>( m_aUserParams[ m_aiStdPrimitiveVars[ EnvVars_s ] ] ) );
			else
				return ( NULL );
		}
		/** Get a reference the to t default parameter.
		 */
		virtual CqParameterTyped<TqFloat, TqFloat>* t()
		{
			if ( m_aiStdPrimitiveVars[ EnvVars_t ] >= 0 )
				return ( static_cast<CqParameterTyped<TqFloat, TqFloat>*>( m_aUserParams[ m_aiStdPrimitiveVars[ EnvVars_t ] ] ) );
			else
				return ( NULL );
		}
		/** Get a reference the to u default parameter.
		 */
		virtual CqParameterTyped<TqFloat, TqFloat>* u()
		{
			if ( m_aiStdPrimitiveVars[ EnvVars_u ] >= 0 )
				return ( static_cast<CqParameterTyped<TqFloat, TqFloat>*>( m_aUserParams[ m_aiStdPrimitiveVars[ EnvVars_u ] ] ) );
			else
				return ( NULL );
		}
		/** Get a reference the to v default parameter.
		 */
		virtual CqParameterTyped<TqFloat, TqFloat>* v()
		{
			if ( m_aiStdPrimitiveVars[ EnvVars_v ] >= 0 )
				return ( static_cast<CqParameterTyped<TqFloat, TqFloat>*>( m_aUserParams[ m_aiStdPrimitiveVars[ EnvVars_v ] ] ) );
			else
				return ( NULL );
		}

		/** Get a reference the to P default parameter.
		 */
		virtual const	CqParameterTyped<CqVector4D, CqVector3D>* P() const
		{
			if ( m_aiStdPrimitiveVars[ EnvVars_P ] >= 0 )
				return ( static_cast<const CqParameterTyped<CqVector4D, CqVector3D>*>( m_aUserParams[ m_aiStdPrimitiveVars[ EnvVars_P ] ] ) );
			else
				return ( NULL );
		}
		/** Get a reference the to N default parameter.
		 */
		virtual const	CqParameterTyped<CqVector3D, CqVector3D>* N() const
		{
			if ( m_aiStdPrimitiveVars[ EnvVars_N ] >= 0 )
				return ( static_cast<const CqParameterTyped<CqVector3D, CqVector3D>*>( m_aUserParams[ m_aiStdPrimitiveVars[ EnvVars_N ] ] ) );
			else
				return ( NULL );
		}
		/** Get a reference the to an indexed primitive variable.
		 */
		virtual const	CqParameter* pVar(TqInt index) const
		{
			assert( index >= EnvVars_Cs && index < EnvVars_Last );
			if ( m_aiStdPrimitiveVars[ index ] >= 0 )
				return ( m_aUserParams[ m_aiStdPrimitiveVars[ index ] ] );
			else
				return ( NULL );
		}
		/** Get a reference the to Cq default parameter.
		 */
		virtual const	CqParameterTyped<CqColor, CqColor>* Cs() const
		{
			if ( m_aiStdPrimitiveVars[ EnvVars_Cs ] >= 0 )
				return ( static_cast<const CqParameterTyped<CqColor, CqColor>*>( m_aUserParams[ m_aiStdPrimitiveVars[ EnvVars_Cs ] ] ) );
			else
				return ( NULL );
		}
		/** Get a reference the to Os default parameter.
		 */
		virtual const	CqParameterTyped<CqColor, CqColor>* Os() const
		{
			if ( m_aiStdPrimitiveVars[ EnvVars_Os ] >= 0 )
				return ( static_cast<const CqParameterTyped<CqColor, CqColor>*>( m_aUserParams[ m_aiStdPrimitiveVars[ EnvVars_Os ] ] ) );
			else
				return ( NULL );
		}
		/** Get a reference the to s default parameter.
		 */
		virtual const	CqParameterTyped<TqFloat, TqFloat>* s() const
		{
			if ( m_aiStdPrimitiveVars[ EnvVars_s ] >= 0 )
				return ( static_cast<const CqParameterTyped<TqFloat, TqFloat>*>( m_aUserParams[ m_aiStdPrimitiveVars[ EnvVars_s ] ] ) );
			else
				return ( NULL );
		}
		/** Get a reference the to t default parameter.
		 */
		virtual const	CqParameterTyped<TqFloat, TqFloat>* t() const
		{
			if ( m_aiStdPrimitiveVars[ EnvVars_t ] >= 0 )
				return ( static_cast<const CqParameterTyped<TqFloat, TqFloat>*>( m_aUserParams[ m_aiStdPrimitiveVars[ EnvVars_t ] ] ) );
			else
				return ( NULL );
		}
		/** Get a reference the to u default parameter.
		 */
		virtual const	CqParameterTyped<TqFloat, TqFloat>* u() const
		{
			if ( m_aiStdPrimitiveVars[ EnvVars_u ] >= 0 )
				return ( static_cast<const CqParameterTyped<TqFloat, TqFloat>*>( m_aUserParams[ m_aiStdPrimitiveVars[ EnvVars_u ] ] ) );
			else
				return ( NULL );
		}
		/** Get a reference the to v default parameter.
		 */
		virtual const	CqParameterTyped<TqFloat, TqFloat>* v() const
		{
			if ( m_aiStdPrimitiveVars[ EnvVars_v ] >= 0 )
				return ( static_cast<const CqParameterTyped<TqFloat, TqFloat>*>( m_aUserParams[ m_aiStdPrimitiveVars[ EnvVars_v ] ] ) );
			else
				return ( NULL );
		}
		/** Get a reference the to an indexed primitive variable.
		 */
		virtual CqParameter* pVar(TqInt index)
		{
			assert( index >= EnvVars_Cs && index < EnvVars_Last );
			if ( m_aiStdPrimitiveVars[ index ] >= 0 )
				return ( m_aUserParams[ m_aiStdPrimitiveVars[ index ] ] );
			else
				return ( NULL );
		}

		/** Determine whether this surface has a specified primitive variable based on index
		 */
		virtual const	bool bHasVar(TqInt index) const
		{
			assert( index >= EnvVars_Cs && index < EnvVars_Last );
			// Special case for s & t if "st" is specified.
			if( index == EnvVars_s || index == EnvVars_t )
				return( m_aiStdPrimitiveVars[ index ] >= 0 || FindUserParam("st") );
			else
				return ( m_aiStdPrimitiveVars[ index ] >= 0 );
		}

		/** Get a reference to the user parameter variables array
		 */
		virtual const std::vector<CqParameter*>& aUserParams() const
		{
			return ( m_aUserParams );
		}

		/** Get a reference to the user parameter variables array
		 */
		virtual std::vector<CqParameter*>& aUserParams()
		{
			return ( m_aUserParams );
		}

		virtual CqParameter* FindUserParam( const char* strName ) const;

		/* From IqSurface.
		 */
		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime = 0 );
		virtual TqUint cFaceVertex() const
		{
			return(cFaceVarying());
		}

		/** Add a primitive variable to the array.
		 */
		virtual void AddPrimitiveVariable( CqParameter* pParam )
		{
			static const TqUlong RIH_P = CqString::hash("P");
			static const TqUlong RIH_N = CqString::hash("N");
			static const TqUlong RIH_CS = CqString::hash("Cs");
			static const TqUlong RIH_OS = CqString::hash("Os");
			static const TqUlong RIH_S = CqString::hash("s");
			static const TqUlong RIH_T = CqString::hash("t") ;
			static const TqUlong RIH_U = CqString::hash("u");
			static const TqUlong RIH_V = CqString::hash("v");

			m_aUserParams.push_back( pParam );

			if ( pParam->hash() == RIH_P )
			{
				assert( -1 == m_aiStdPrimitiveVars[ EnvVars_P ] );
				m_aiStdPrimitiveVars[ EnvVars_P ] = m_aUserParams.size() - 1;
			}
			else if ( pParam->hash() == RIH_N )
			{
				assert( -1 == m_aiStdPrimitiveVars[ EnvVars_N ] );
				m_aiStdPrimitiveVars[ EnvVars_N ] = m_aUserParams.size() - 1;
			}

			else if ( pParam->hash() == RIH_CS )
			{
				assert( -1 == m_aiStdPrimitiveVars[ EnvVars_Cs ] );
				m_aiStdPrimitiveVars[ EnvVars_Cs ] = m_aUserParams.size() - 1;
			}
			else if ( pParam->hash() == RIH_OS )
			{
				assert( -1 == m_aiStdPrimitiveVars[ EnvVars_Os ] );
				m_aiStdPrimitiveVars[ EnvVars_Os ] = m_aUserParams.size() - 1;
			}
			else if ( pParam->hash() == RIH_S  )
			{
				assert( -1 == m_aiStdPrimitiveVars[ EnvVars_s ] );
				m_aiStdPrimitiveVars[ EnvVars_s ] = m_aUserParams.size() - 1;
			}
			else if ( pParam->hash() == RIH_T)
			{
				assert( -1 == m_aiStdPrimitiveVars[ EnvVars_t ] );
				m_aiStdPrimitiveVars[ EnvVars_t ] = m_aUserParams.size() - 1;
			}
			else if ( pParam->hash() == RIH_U)
			{
				assert( -1 == m_aiStdPrimitiveVars[ EnvVars_u ] );
				m_aiStdPrimitiveVars[ EnvVars_u ] = m_aUserParams.size() - 1;
			}
			else if ( pParam->hash() == RIH_V )
			{
				assert( -1 == m_aiStdPrimitiveVars[ EnvVars_v ] );
				m_aiStdPrimitiveVars[ EnvVars_v ] = m_aUserParams.size() - 1;
			}

		}

		/** Determine whether this surface can be trimmed
		 */
		virtual const	bool	bCanBeTrimmed() const
		{
			return ( false );
		}
		/** Determine if the specified point is trimmed.
		 * \todo Review: Unused parameter p
		 */
		virtual	const	bool	bIsPointTrimmed( const CqVector2D& p ) const
		{
			return ( false );
		}
		/** Determine if the specified edge crosses the trimming curves.
		 * \todo Review: Unused parameter v1, v2
		 */
		virtual	const	bool	bIsLineIntersecting( const CqVector2D& v1, const CqVector2D& v2 ) const
		{
			return ( false );
		}
		/** Determine the level at which to split a trim curve according
		 * to its screen size after application to the surface paramters of this
		 * surface.
		 * \todo Review: Unused parameter Curve. Shouldn't it be curve?
		 */
		virtual	TqInt	TrimDecimation( const CqTrimCurve& Curve )
		{
			return ( 0 );
		}
		/** Prepare the trim curve once the surface has been completed.
		 */
		virtual	void	PrepareTrimCurve()
		{}

		void	uSubdivideUserParameters( CqSurface* pA, CqSurface* pB );
		void	vSubdivideUserParameters( CqSurface* pA, CqSurface* pB );

		virtual void	PreDice( TqInt /* uDiceSize */, TqInt /* vDiceSize */ )
		{}
		virtual void	NaturalDice( CqParameter* pParameter, TqInt uDiceSize,
			TqInt vDiceSize, IqShaderData* pData);
		virtual void	PostDice(CqMicroPolyGrid * /* pGrid */)
		{}

		/**
		 * \todo Review: Unused parameter aSplits, u
		 */
		virtual TqInt	PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u )
		{
			return ( 0 );
		}
		virtual void	NaturalSubdivide( CqParameter* pParam, CqParameter* pParam1, CqParameter* pParam2, bool u );
		virtual void	PostSubdivide(std::vector<boost::shared_ptr<CqSurface> >& /* aSplits */)
		{}

		virtual	CqMicroPolyGridBase* Dice();
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits );

		virtual bool isMoving() const
		{
			return m_pTransform->isMoving();
		}

		/**
		 * Decide whether the geometry is diceable in the given coordinate system.
		 *
		 * \param diceCoords - The transformation into "dicing coordinates".
		 *                     In normal operation, dicing coordinates are
		 *                     given by the camera->raster transform, but may
		 *                     be something else in special cases, for example,
		 *                     baking to a point cloud where we want to dice in
		 *                     world coordinates.
		 *
		 * \todo Review: Could this be const?
		 */
		virtual bool	Diceable(const CqMatrix& diceCoords)
		{
			return(false);
		}

		/** Get the shading rate, adjusted for DoF and MB
		 *
		 * Without depth of field or motion blur, this function just returns
		 * the shading rate as given by the current attribute set.
		 *
		 * When depth of field is turned on, the shading rate is increased
		 * proportionally to the area of the circle of confusion, multiplied by
		 * the value of the system "GeometricFocusFactor" attribute (as
		 * specified with RiGeometricApproximation("focusfactor", ...) )
		 *
		 * \todo No adjustment is yet implemented for motion blur, but the
		 * intention is to allow RiGeometricApproximation("motionfactor", ...)
		 * to have an effect on the returned shading rate as well.
		 */
		TqFloat AdjustedShadingRate() const;

		bool	m_fDiceable;		///< Flag to indicate that this GPrim is diceable.
		bool	m_fDiscard;			///< Flag to indicate that this GPrim is to be discarded.
		TqInt	m_SplitCount;		///< The number of times this GPrim has been split
	protected:

		/** Protected member function to clone the data, used by the Clone() functions
		 *  on the derived classes.
		 */
		void CloneData(CqSurface* clone) const;
		std::vector<CqParameter*>	m_aUserParams;			///< Storage for user defined paramter variables.
		TqInt	m_aiStdPrimitiveVars[ EnvVars_Last ];		///< Quick lookup index into the primitive variables table for standard variables.

		CqAttributesPtr m_pAttributes;	///< Pointer to the attributes state associated with this GPrim.
		CqTransformPtr m_pTransform;		///< Pointer to the transformation state associated with this GPrim.

		TqInt	m_uDiceSize;		///< Calculated dice size to achieve an appropriate shading rate.
		TqInt	m_vDiceSize;		///< Calculated dice size to achieve an appropriate shading rate.
		EqSplitDir	m_SplitDir;			///< The direction to split this GPrim to achieve best results.
		bool	m_CachedBound;		///< Whether or not the bound has been cached
		CqBound	m_Bound;			///< The cached object bound
		boost::shared_ptr<CqCSGTreeNode>	m_pCSGNode;		///< Pointer to the 'primitive' CSG node this surface belongs to, NULL if not part of a solid.
}
;



//----------------------------------------------------------------------
/** \class CqDeformingSurface
 * Templatised class containing a series of motion stages of a specific surface type for motion blurring.
 */

class CqDeformingSurface : public CqSurface, public CqMotionSpec<boost::shared_ptr<CqSurface> >
{
	public:
		CqDeformingSurface( boost::shared_ptr<CqSurface> const& a ) : CqSurface(), CqMotionSpec<boost::shared_ptr<CqSurface> >( a )
		{}
		virtual	~CqDeformingSurface()
		{}

		/** Get combined bound for all times
		 * \return CqBound representing the geometric boundary of this GPrim over all time slots.
		 */
		virtual	void	Bound(CqBound* bound) const
		{
			bound->vecMin() = CqVector3D(FLT_MAX, FLT_MAX, FLT_MAX);
			bound->vecMax() = CqVector3D(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			CqBound B;
			TqInt i;
			for ( i = 0; i < cTimes(); i++ )
			{
				GetMotionObject( Time( i ) )->Bound(&B);
				bound->Encapsulate( &B );
			}
		}
		/** Dice this GPrim, creating a CqMotionMicroPolyGrid with all times in.
		 */
		virtual	CqMicroPolyGridBase* Dice()
		{
			CqMotionMicroPolyGrid* pGrid = new CqMotionMicroPolyGrid;
			TqInt i;
			for ( i = 0; i < cTimes(); i++ )
			{
				CqMicroPolyGridBase* pGrid2 = GetMotionObject( Time( i ) ) ->Dice();
				pGrid->AddTimeSlot( Time( i ), pGrid2 );
				ADDREF(pGrid2);
				pGrid->SetfTriangular( pGrid2->fTriangular() );
			}
			pGrid->Initialise(m_uDiceSize, m_vDiceSize, shared_from_this());
			return ( pGrid );
		}
		/** Split this GPrim, creating a series of CqDeformingSurface with all times in.
		 */
		virtual	TqInt	Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
		{
			std::vector<std::vector<boost::shared_ptr<CqSurface> > > aaMotionSplits;
			aaMotionSplits.resize( cTimes() );
			TqInt cSplits = 0;
			TqInt i;

			cSplits = GetMotionObject( Time( 0 ) ) ->Split( aaMotionSplits[ 0 ] );
			for ( i = 1; i < cTimes(); i++ )
			{
#ifdef _DEBUG
				TqInt numsplits = GetMotionObject( Time( i ) ) ->Split( aaMotionSplits[ i ] );
				assert( numsplits == cSplits);
#else

				GetMotionObject( Time( i ) ) ->Split( aaMotionSplits[ i ] );
#endif

			}

			// Now build motion surfaces from the splits and pass them back.
			for ( i = 0; i < cSplits; i++ )
			{
				boost::shared_ptr<CqDeformingSurface> pNewMotion( new CqDeformingSurface( boost::shared_ptr<CqSurface>() ) );
				pNewMotion->m_fDiceable = true;
				pNewMotion->m_SplitCount = m_SplitCount + 1;
				TqInt j;
				for ( j = 0; j < cTimes(); j++ )
					pNewMotion->AddTimeSlot( Time( j ), aaMotionSplits[ j ][ i ] );
				aSplits.push_back( boost::static_pointer_cast<CqSurface>( pNewMotion ) );
			}
			return ( cSplits );
		}
		/** Determine if the prmary time slot is diceable, this is the one that is shaded, so
		 * determines the dicing rate, which is then copied to the other times.
		 * \return Boolean indicating GPrim is diceable.
		 */
		virtual bool	Diceable(const CqMatrix& matCtoR)
		{
			bool f = GetMotionObject( Time( 0 ) ) ->Diceable(matCtoR);
			// Copy the split info so that at each time slot, the gprims split the same.
			TqInt i;
			for ( i = 1; i < cTimes(); i++ )
				GetMotionObject( Time( i ) ) ->CopySplitInfo( GetMotionObject( Time( 0 ) ).get() );
			return ( f );
		}

		virtual CqSurface* Clone() const
		{
/* 			CqDeformingSurface* clone = new CqDeformingSurface(GetDefaultObject());
 * 			clone->CqMotionSpec<boost::shared_ptr<CqSurface> >::operator=(*this);
 * 			return(clone);
 */
			return(NULL);
		}


		/** Determine whether the passed surface is valid to be used as a
		 *  frame in motion blur for this surface.
		 */
		virtual bool	IsMotionBlurMatch( CqSurface* pSurf )
		{
			return( false );
		}

		/** Transform all GPrims by the specified transformation matrices.
		 * \param matTx Reference to the transformation matrix.
		 * \param matITTx Reference to the inverse transpose of the transformation matrix, used to transform normals.
		 * \param matRTx Reference to the rotation only transformation matrix, used to transform vectors.
		 * \param iTime The frame time at which to apply the transformation.
		 */
		virtual void	Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime = 0 )
		{
			TqInt i;
			for ( i = 0; i < cTimes(); i++ )
			{
				Aqsis::log() << debug << "Transforming deforming surface at time : " << i << " : [" << cTimes() << "]" << std::endl;
				GetMotionObject( Time(i) ) ->Transform( matTx, matITTx, matRTx, i );
			}
		}

		/** Set the surface parameters of all GPrims to match those on the spefified one.
		 * \param From GPrim to copy parameters from.
		 */
		virtual	void	SetSurfaceParameters( const CqSurface& From )
		{
			TqInt i;
			for ( i = 0; i < cTimes(); i++ )
				GetMotionObject( Time( i ) ) ->SetSurfaceParameters( From );
		}
		/** Force all GPrims to be undiceable.
		 */
		virtual	void	ForceUndiceable()
		{
			CqSurface::ForceUndiceable();
			TqInt i;
			for ( i = 0; i < cTimes(); i++ )
				GetMotionObject( Time( i ) ) ->ForceUndiceable();
		}

		/** Mark all GPrims to be discarded.
		 */
		virtual	void	Discard()
		{
			CqSurface::Discard();
			TqInt i;
			for ( i = 0; i < cTimes(); i++ )
				GetMotionObject( Time( i ) ) ->Discard();
		}

		virtual	TqUint	cUniform() const
		{
			return ( GetMotionObject( Time( 0 ) ) ->cUniform() );
		}
		virtual	TqUint	cVarying() const
		{
			return ( GetMotionObject( Time( 0 ) ) ->cVarying() );
		}
		virtual	TqUint	cVertex() const
		{
			return ( GetMotionObject( Time( 0 ) ) ->cVertex() );
		}
		virtual	TqUint	cFaceVarying() const
		{
			return ( GetMotionObject( Time( 0 ) ) ->cFaceVarying() );
		}
		// Overrides from CqMotionSpec
		virtual	void	ClearMotionObject( boost::shared_ptr<CqSurface>& A ) const
			{}
		;
		virtual	boost::shared_ptr<CqSurface>	ConcatMotionObjects( boost::shared_ptr<CqSurface> const& A, boost::shared_ptr<CqSurface> const& B ) const
		{
			return ( A );
		}
		virtual	boost::shared_ptr<CqSurface>	LinearInterpolateMotionObjects( TqFloat Fraction, boost::shared_ptr<CqSurface> const& A, boost::shared_ptr<CqSurface> const& B ) const
		{
			return ( A );
		}
	protected:
		/** Protected member function to clone the data, used by the Clone() functions
		 *  on the derived classes.
		 */
		void CloneData(CqDeformingSurface* clone) const;

};


//-----------------------------------------------------------------------

} // namespace Aqsis

//}  // End of #ifdef SURFACE_H_INCLUDED
#endif
