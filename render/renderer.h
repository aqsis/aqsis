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
		\brief Declares the base CqRenderer class which is the central core of the rendering main loop.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is renderer.h included already?
#ifndef RENDERER_H_INCLUDED
//{
#define RENDERER_H_INCLUDED 1

#ifdef	WIN32
#include	<windows.h>
#endif

#include	<vector>
#include	<iostream>

#include	<time.h>

#include	"ri.h"
#include	"context.h"
#include	"stats.h"
#include	"vector2d.h"
#include	"semaphore.h"
#include	"messages.h"
#include	"scene.h"
#include	"specific.h"
#include	"shaders.h"
#include	"irenderer.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)

class CqImageBuffer;

struct SqCoordSys
{
	SqCoordSys(const char* strName, const CqMatrix& matToWorld, const CqMatrix& matWorldTo) :
					m_strName(strName),
					m_matToWorld(matToWorld),
					m_matWorldTo(matWorldTo)	{}
	SqCoordSys()	{}

	CqMatrix	m_matWorldTo;
	CqMatrix	m_matToWorld;
	CqString	m_strName;
};

enum EqCoordSystems
{
	CoordSystem_Camera=0,
	CoordSystem_Current,
	CoordSystem_World,
	CoordSystem_Screen,
	CoordSystem_NDC,
	CoordSystem_Raster,

	CoordSystem_Last,
};

enum EqRenderMode
{
	RenderMode_Image=0,

	RenderMode_Shadows,
	RenderMode_Reflection,
};

//----------------------------------------------------------------------
/** \class  CqRenderer
 * The main renderer control class.  Contains all information relating to
 * the current image being rendered.  There is only ever one of these,
 * statically defined in the CPP file for this class, and globally available.
 */

class CqRenderer;
extern CqRenderer* pCurrRenderer;

class CqRenderer : public IsRenderer
{
	public:
						CqRenderer();
				virtual	~CqRenderer();

				virtual	void		Destroy()	{delete(this);}

				virtual	CqContext*	CreateMainContext();
				virtual	CqContext*	CreateFrameContext();
				virtual	CqContext*	CreateWorldContext();
				virtual	CqContext*	CreateAttributeContext();
				virtual	CqContext*	CreateTransformContext();
				virtual	CqContext*	CreateSolidContext();
				virtual	CqContext*	CreateObjectContext();
				virtual	CqContext*	CreateMotionContext(TqInt N, TqFloat times[]);

				virtual	void		DeleteMainContext();
				virtual	void		DeleteFrameContext();
				virtual	void		DeleteWorldContext();
				virtual	void		DeleteAttributeContext();
				virtual	void		DeleteTransformContext();
				virtual	void		DeleteSolidContext();
				virtual	void		DeleteObjectContext();
				virtual	void		DeleteMotionContext();

				virtual	CqOptions&		optCurrent();
				virtual	const CqAttributes*	pattrCurrent();
				virtual	CqAttributes*	pattrWriteCurrent();
				virtual	const CqTransform*	ptransCurrent();
				virtual	CqTransform*	ptransWriteCurrent();

				virtual	TqFloat		Time() const;
				virtual	void		AdvanceTime();
	
						/** Get a pointer to the current context.
						 * \return Pointer to a CqContext derived class.
						 */
				virtual	CqContext*	pconCurrent()			{return(m_pconCurrent);}
						/** Get a erad only pointer to the current context.
						 * \return Pointer to a CqContext derived class.
						 */
				virtual const	CqContext*	pconCurrent() const		{return(m_pconCurrent);}
						/** Get a pointer to the current image buffer.
						 * \return A CqImageBuffer pointer.
						 */
				virtual CqImageBuffer* pImage()				{return(m_pImageBuffer);}
						/** Set the pointer to the current image buffer.
						 */
				virtual	void		SetImage(CqImageBuffer* pImage)
															{m_pImageBuffer=pImage;}
						/** Get a pointer to the current scene storage.
						 * \return A CqScene pointer.
						 */
				virtual	CqScene&	Scene()					{return(m_Scene);}
	// Handle various coordinate system transformation requirements.
				virtual	CqMatrix	matSpaceToSpace			(const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld=CqMatrix(), const CqMatrix& matObjectToWorld=CqMatrix());
				virtual	CqMatrix	matVSpaceToSpace		(const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld=CqMatrix(), const CqMatrix& matObjectToWorld=CqMatrix());
				virtual	CqMatrix	matNSpaceToSpace		(const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld=CqMatrix(), const CqMatrix& matObjectToWorld=CqMatrix());
	
						/** Get a read only reference to the current transformation matrix.
						 * \return A constant reference to a CqMatrix.
						 */
				virtual const	CqMatrix&	matCurrent(TqFloat time=0.0f) const		{return(pconCurrent()->matCurrent(time));}

				virtual	TqBool	SetCoordSystem(const char* strName, const CqMatrix& matToWorld);

	// Function which can be overridden by the derived class.
				virtual	void		Initialise();
				virtual	void		RenderWorld();
				virtual	void		LoadDisplayLibrary();
				virtual	void		Quit();
				virtual	void		UpdateStatus()		{}
						/** Get the global statistics class.
						 * \return A reference to the CqStats class on this renderer.
						 */
				virtual	CqStats&	Stats()				{return(m_Stats);}
						/** Print a message to stdout, along with any relevant message codes.
						 * \param msg A SqMessage structure to print.
						 */
				virtual	void		PrintMessage(const SqMessage& msg)
															{ 
																if(msg.Code()>0)
																	std::cout << msg.Code() << " : " << 
																				 msg.Severity() << " : ";
																std::cout << msg.strMessage().data() << std::endl; 
															}
	// Contect change callbacks
				virtual	void		OnBegin()			{}
				virtual	void		OnFrameBegin()		{}
				virtual	void		OnWorldBegin()		{}
				virtual	void		OnAttributeBegin()	{}
				virtual	void		OnTransformBegin()	{}
				virtual	void		OnSolidBegin()		{}
				virtual	void		OnObjectBegin()		{}
				virtual	void		OnMotionBegin()		{}
				virtual	void		OnEnd()				{}
				virtual	void		OnFrameEnd()		{}
				virtual	void		OnWorldEnd()		{}
				virtual	void		OnAttributeEnd()	{}
				virtual	void		OnTransformEnd()	{}
				virtual	void		OnSolidEnd()		{}
				virtual	void		OnObjectEnd()		{}
				virtual	void		OnMotionEnd()		{}

				virtual	SqParameterDeclaration FindParameterDecl(const char* strDecl);
				virtual	void		AddParameterDecl(const char* strName, const char* strType);
				virtual	void		ClearSymbolTable()	{m_Symbols.clear();}

						/** Get the list of currently registered shaders.
						 * \return A reference to a list of CqShaderRegister classes.
						 */
				virtual	CqList<CqShaderRegister>& Shaders()	{return(m_Shaders);}

						/** Flush any registered shaders.
						 */
				virtual	void		FlushShaders()			{
																while(m_Shaders.pFirst()!=0)	
																	delete(m_Shaders.pFirst());
															}

						/** Get a reference to the current transformation matrix.
						 * \return A reference to a CqMatrix.
						 */
				virtual	CqMatrix&	matWorldToCamera()		{return(m_aCoordSystems[CoordSystem_Camera].m_matWorldTo);}
						/** Set the world to screen matrix.
						 * \param mat The new matrix to use as the world to screen transformation.
						 */
				virtual	void		SetmatScreen(const CqMatrix& mat)	{
																	m_aCoordSystems[CoordSystem_Screen].m_matWorldTo=mat;
																}
						/** Set the world to NDC matrix.
						 * \param mat The new matrix to use as the world to NDC transformation.
						 */
				virtual	void		SetmatNDC(const CqMatrix& mat)	{
																	m_aCoordSystems[CoordSystem_NDC].m_matWorldTo=mat;
																}
						/** Set the world to raster matrix.
						 * \param mat The new matrix to use as the world to raster transformation.
						 */
				virtual	void		SetmatRaster(const CqMatrix& mat)	{
																	m_aCoordSystems[CoordSystem_Raster].m_matWorldTo=mat;
																}
						/** Set the world to camera matrix.
						 * \param mat The new matrix to use as the world to camera transformation.
						 */
				virtual	void		SetmatCamera(const CqMatrix& mat)
																{
																	m_aCoordSystems[CoordSystem_Camera] .m_matWorldTo=
																	m_aCoordSystems[CoordSystem_Current].m_matWorldTo=mat;
																	m_aCoordSystems[CoordSystem_Camera] .m_matToWorld=
																	m_aCoordSystems[CoordSystem_Current].m_matToWorld=mat.Inverse();
																}

							/** Get the current transformation stack.
							 * \return A reference to a vector of CqTransform class pointers.
							 */
				virtual std::vector<CqTransform*>&	TransformStack()	{return(m_TransformStack);}

							/** Signal from the display driver to indicate that it has finished.
							 *\param reason The reason for finishing. 
							 */
				virtual	void		SignalDisplayDriverFinished(TqInt reason)
																{
																	m_semDisplayDriverFinished.Signal();
																}

						/** Set the locally stored render time.
						 * \param time System time value representing the seconds taken to render.
						 */
						void	SettimeTaken(time_t time)	{m_timeTaken=time;}
						/** Get the time taken to complete the last render in seconds.
						 */
						time_t	timeTaken() const			{return(m_timeTaken);}

						void		PrintStats(TqInt level);




			CqSemaphore		m_semDisplayDriverReady;		///< Semaphore used to check for safe startup of a display driver.
			CqSemaphore		m_semDisplayDriverFinished;		///< Semaphore used to check for safe close down of a display driver.
	
	private:
			CqContext*		m_pconCurrent;					///< Pointer to the current context.
			CqStats		m_Stats;						///< Global statistics.
			CqOptions		m_optDefault;					///< Default options.
			CqAttributes	m_attrDefault;					///< Default attributes.
			CqTransform		m_transDefault;					///< Default transformation.
			CqImageBuffer*	m_pImageBuffer;					///< Pointer to the current image buffer.
			HANDLE			m_hDisplayThread;				///< Handle to the display driver thread.
			EqRenderMode	m_Mode;							
			CqScene			m_Scene;						///< The global scene storage.
			CqList<CqShaderRegister> m_Shaders;				///< List of registered shaders.
			TqBool			m_fSaveGPrims;
			std::vector<CqTransform*>	m_TransformStack;	///< The global transformation stack.
			std::vector<SqParameterDeclaration>	m_Symbols;	///< Symbol table.

	protected:
			time_t			m_timeTaken;					///< The stored time taken to complete the last full render.

	public:
			std::vector<SqCoordSys>	m_aCoordSystems;		///< List of reistered coordinate systems.
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

//}  // End of #ifdef RENDERER_H_INCLUDED
#endif
