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
		\brief Declares the interface to a renderer class..
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is renderer.h included already?
#ifndef IRENDERER_H_INCLUDED
//{
#define IRENDERER_H_INCLUDED 1

#include	"ri.h"
#include	"context.h"
#include	"stats.h"
#include	"messages.h"
#include	"scene.h"
#include	"specific.h"
#include	"matrix.h"
#include	"shaders.h"
#include	"symbols.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)

class CqImageBuffer;

//----------------------------------------------------------------------
/** \struct IsRenderer
 * Interface to the common functions on the renderer class.
 */

struct IsRenderer
{
	public:
		_qShareM static IsRenderer* Create();
		
		virtual void		Destroy()=0;

		virtual	CqContext*	CreateMainContext()=0;
		virtual	CqContext*	CreateFrameContext()=0;
		virtual	CqContext*	CreateWorldContext()=0;
		virtual	CqContext*	CreateAttributeContext()=0;
		virtual	CqContext*	CreateTransformContext()=0;
		virtual	CqContext*	CreateSolidContext()=0;
		virtual	CqContext*	CreateObjectContext()=0;
		virtual	CqContext*	CreateMotionContext(TqInt N, TqFloat times[])=0;

		virtual	void		DeleteMainContext()=0;
		virtual	void		DeleteFrameContext()=0;
		virtual	void		DeleteWorldContext()=0;
		virtual	void		DeleteAttributeContext()=0;
		virtual	void		DeleteTransformContext()=0;
		virtual	void		DeleteSolidContext()=0;
		virtual	void		DeleteObjectContext()=0;
		virtual	void		DeleteMotionContext()=0;

		virtual	CqOptions&		optCurrent()=0;
		virtual	const CqAttributes*	pattrCurrent()=0;
		virtual	CqAttributes*	pattrWriteCurrent()=0;
		virtual	const CqTransform*	ptransCurrent()=0;
		virtual	CqTransform*	ptransWriteCurrent()=0;

		virtual	TqFloat		Time() const=0;
		virtual	void		AdvanceTime()=0;
	
						/** Get a pointer to the current context.
						 * \return Pointer to a CqContext derived class.
						 */
		virtual	CqContext*	pconCurrent()=0;
						/** Get a erad only pointer to the current context.
						 * \return Pointer to a CqContext derived class.
						 */
		virtual const CqContext*	pconCurrent() const=0;
						/** Get a pointer to the current image buffer.
						 * \return A CqImageBuffer pointer.
						 */
		virtual	CqImageBuffer* pImage()=0;
						/** Set the pointer to the current image buffer.
						 */
		virtual	void		SetImage(CqImageBuffer* pImage)=0;
						/** Get a pointer to the current scene storage.
						 * \return A CqScene pointer.
						 */
		virtual	CqScene&	Scene()=0;

	// Handle various coordinate system transformation requirements.
		virtual	CqMatrix	matSpaceToSpace			(const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld=CqMatrix(), const CqMatrix& matObjectToWorld=CqMatrix())=0;
		virtual	CqMatrix	matVSpaceToSpace		(const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld=CqMatrix(), const CqMatrix& matObjectToWorld=CqMatrix())=0;
		virtual	CqMatrix	matNSpaceToSpace		(const char* strFrom, const char* strTo, const CqMatrix& matShaderToWorld=CqMatrix(), const CqMatrix& matObjectToWorld=CqMatrix())=0;
	
						/** Get a read only reference to the current transformation matrix.
						 * \return A constant reference to a CqMatrix.
						 */
		virtual const	CqMatrix&	matCurrent(TqFloat time=0.0f) const=0;
						/** Get the list of currently registered shaders.
						 * \return A reference to a list of CqShaderRegister classes.
						 */
		virtual	CqList<CqShaderRegister>& Shaders()=0;

						/** Flush any registered shaders.
						 */
		virtual	void	FlushShaders()=0;
						/** Get a reference to the current transformation matrix.
						 * \return A reference to a CqMatrix.
						 */
		virtual	CqMatrix& matWorldToCamera()=0;
						/** Set the world to screen matrix.
						 * \param mat The new matrix to use as the world to screen transformation.
						 */
		virtual	void	SetmatScreen(const CqMatrix& mat)=0;
						/** Set the world to NDC matrix.
						 * \param mat The new matrix to use as the world to NDC transformation.
						 */
		virtual	void	SetmatNDC(const CqMatrix& mat)=0;
						/** Set the world to raster matrix.
						 * \param mat The new matrix to use as the world to raster transformation.
						 */
		virtual	void	SetmatRaster(const CqMatrix& mat)=0;
						/** Set the world to camera matrix.
						 * \param mat The new matrix to use as the world to camera transformation.
						 */
		virtual	void	SetmatCamera(const CqMatrix& mat)=0;
						/** Set the world to screen matrix.
						 * \param mat The new matrix to use as the world to screen transformation.
						 */
		virtual	TqBool	SetCoordSystem(const char* strName, const CqMatrix& matToWorld)=0;
		virtual	std::vector<CqTransform*>&	TransformStack()=0;

	// Function which can be overridden by the derived class.
		virtual	void		Initialise()=0;
		virtual	void		RenderWorld()=0;
		virtual	void		LoadDisplayLibrary()=0;
		virtual	void		Quit()=0;
		virtual	void		UpdateStatus()=0;
						/** Get the global statistics class.
						 * \return A reference to the CqStats class on this renderer.
						 */
		virtual	CqStats&	Stats()=0;
						/** Print a message to stdout, along with any relevant message codes.
						 * \param msg A SqMessage structure to print.
						 */
		virtual	void		PrintMessage(const SqMessage& msg)=0;

	// Contect change callbacks
		virtual	void		OnBegin()=0;
		virtual	void		OnFrameBegin()=0;
		virtual	void		OnWorldBegin()=0;
		virtual	void		OnAttributeBegin()=0;
		virtual	void		OnTransformBegin()=0;
		virtual	void		OnSolidBegin()=0;
		virtual	void		OnObjectBegin()=0;
		virtual	void		OnMotionBegin()=0;
		virtual	void		OnEnd()=0;
		virtual	void		OnFrameEnd()=0;
		virtual	void		OnWorldEnd()=0;
		virtual	void		OnAttributeEnd()=0;
		virtual	void		OnTransformEnd()=0;
		virtual	void		OnSolidEnd()=0;
		virtual	void		OnObjectEnd()=0;
		virtual	void		OnMotionEnd()=0;

		virtual	SqParameterDeclaration FindParameterDecl(const char* strDecl)=0;
		virtual	void		AddParameterDecl(const char* strName, const char* strType)=0;
		virtual	void		ClearSymbolTable()=0;

		virtual	void		SignalDisplayDriverFinished(TqInt reason)=0;
};

_qShareM	IsRenderer*	CreateDefRenderer();
_qShareM	IsRenderer* pCurrentRenderer();

//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)


#endif // RENDERER_H_INCLUDED
