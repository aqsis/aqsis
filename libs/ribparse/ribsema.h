// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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
 *
 * \brief RIB parser implementation for aqsis
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 */

#ifndef RIBREQUESTHANDLER_H_INCLUDED
#define RIBREQUESTHANDLER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <aqsis/ri/ritypes.h>
#include <aqsis/riutil/tokendictionary.h>

#include "ricxx.h"

namespace Aqsis
{

class RibLexer;

//------------------------------------------------------------------------------
/// Parser for standard RIB streams
class RibParser : boost::noncopyable
{
    public:
        RibParser(Ri::RendererServices& services);

        /// Parse a RIB stream, sending requests to the callback interface
        ///
        /// \param ribStream - RIB stream to be parsed.  May be gzipped.
        /// \param streamName - name of the stream, present in error messages
        void parseStream(std::istream& ribStream, const std::string& streamName,
                         Ri::Renderer& context);

    private:
        /// Object ID -> handle map
        typedef std::map<std::string, RtObjectHandle> ObjectMap;
        /// Light ID -> handle map
        typedef std::map<std::string, RtLightHandle> LightMap;
        /// Request handler function type
        typedef void (RibParser::*RequestHandlerType)(Ri::Renderer& renderer);
        /// Request -> handler mapping type
        typedef std::map<std::string, RequestHandlerType> HandlerMap;
        /// Function pointer compatible with {Area}LightSource
        typedef RtLightHandle (Ri::Renderer::*LightSourceFunc)(
                RtConstToken shadername, const Ri::ParamList&);

        // Utilities for handlers
        static bool searchMapStack(const std::vector<LightMap>& maps,
                                   const std::string& name, RtPointer& handle);
        Ri::ParamList readParamList();
        /// Combined handler for LightSource & AreaLightSource
        void handleLightSourceGeneral(LightSourceFunc lightSourceFunc,
                                      Ri::Renderer& renderer);
        RtConstBasis& getBasis();

        //--------------------------------------------------
        /// Handler methods definitions
        /*[[[cog
        from codegenutils import *
        riXml = parseXml(riXmlPath)
        for p in filter(lambda p: p.findall('Rib'), riXml.findall('Procedures/Procedure')):
            cog.outl('void handle%s(Ri::Renderer& renderer);' % (p.findtext('Name'),))

        ]]]*/
        void handleDeclare(Ri::Renderer& renderer);
        void handleFrameBegin(Ri::Renderer& renderer);
        void handleFrameEnd(Ri::Renderer& renderer);
        void handleWorldBegin(Ri::Renderer& renderer);
        void handleWorldEnd(Ri::Renderer& renderer);
        void handleIfBegin(Ri::Renderer& renderer);
        void handleElseIf(Ri::Renderer& renderer);
        void handleElse(Ri::Renderer& renderer);
        void handleIfEnd(Ri::Renderer& renderer);
        void handleFormat(Ri::Renderer& renderer);
        void handleFrameAspectRatio(Ri::Renderer& renderer);
        void handleScreenWindow(Ri::Renderer& renderer);
        void handleCropWindow(Ri::Renderer& renderer);
        void handleProjection(Ri::Renderer& renderer);
        void handleClipping(Ri::Renderer& renderer);
        void handleClippingPlane(Ri::Renderer& renderer);
        void handleDepthOfField(Ri::Renderer& renderer);
        void handleShutter(Ri::Renderer& renderer);
        void handlePixelVariance(Ri::Renderer& renderer);
        void handlePixelSamples(Ri::Renderer& renderer);
        void handlePixelFilter(Ri::Renderer& renderer);
        void handleExposure(Ri::Renderer& renderer);
        void handleImager(Ri::Renderer& renderer);
        void handleQuantize(Ri::Renderer& renderer);
        void handleDisplay(Ri::Renderer& renderer);
        void handleHider(Ri::Renderer& renderer);
        void handleColorSamples(Ri::Renderer& renderer);
        void handleRelativeDetail(Ri::Renderer& renderer);
        void handleOption(Ri::Renderer& renderer);
        void handleAttributeBegin(Ri::Renderer& renderer);
        void handleAttributeEnd(Ri::Renderer& renderer);
        void handleColor(Ri::Renderer& renderer);
        void handleOpacity(Ri::Renderer& renderer);
        void handleTextureCoordinates(Ri::Renderer& renderer);
        void handleLightSource(Ri::Renderer& renderer);
        void handleAreaLightSource(Ri::Renderer& renderer);
        void handleIlluminate(Ri::Renderer& renderer);
        void handleSurface(Ri::Renderer& renderer);
        void handleDisplacement(Ri::Renderer& renderer);
        void handleAtmosphere(Ri::Renderer& renderer);
        void handleInterior(Ri::Renderer& renderer);
        void handleExterior(Ri::Renderer& renderer);
        void handleShaderLayer(Ri::Renderer& renderer);
        void handleConnectShaderLayers(Ri::Renderer& renderer);
        void handleShadingRate(Ri::Renderer& renderer);
        void handleShadingInterpolation(Ri::Renderer& renderer);
        void handleMatte(Ri::Renderer& renderer);
        void handleBound(Ri::Renderer& renderer);
        void handleDetail(Ri::Renderer& renderer);
        void handleDetailRange(Ri::Renderer& renderer);
        void handleGeometricApproximation(Ri::Renderer& renderer);
        void handleOrientation(Ri::Renderer& renderer);
        void handleReverseOrientation(Ri::Renderer& renderer);
        void handleSides(Ri::Renderer& renderer);
        void handleIdentity(Ri::Renderer& renderer);
        void handleTransform(Ri::Renderer& renderer);
        void handleConcatTransform(Ri::Renderer& renderer);
        void handlePerspective(Ri::Renderer& renderer);
        void handleTranslate(Ri::Renderer& renderer);
        void handleRotate(Ri::Renderer& renderer);
        void handleScale(Ri::Renderer& renderer);
        void handleSkew(Ri::Renderer& renderer);
        void handleCoordinateSystem(Ri::Renderer& renderer);
        void handleCoordSysTransform(Ri::Renderer& renderer);
        void handleTransformBegin(Ri::Renderer& renderer);
        void handleTransformEnd(Ri::Renderer& renderer);
        void handleResource(Ri::Renderer& renderer);
        void handleResourceBegin(Ri::Renderer& renderer);
        void handleResourceEnd(Ri::Renderer& renderer);
        void handleAttribute(Ri::Renderer& renderer);
        void handlePolygon(Ri::Renderer& renderer);
        void handleGeneralPolygon(Ri::Renderer& renderer);
        void handlePointsPolygons(Ri::Renderer& renderer);
        void handlePointsGeneralPolygons(Ri::Renderer& renderer);
        void handleBasis(Ri::Renderer& renderer);
        void handlePatch(Ri::Renderer& renderer);
        void handlePatchMesh(Ri::Renderer& renderer);
        void handleNuPatch(Ri::Renderer& renderer);
        void handleTrimCurve(Ri::Renderer& renderer);
        void handleSubdivisionMesh(Ri::Renderer& renderer);
        void handleSphere(Ri::Renderer& renderer);
        void handleCone(Ri::Renderer& renderer);
        void handleCylinder(Ri::Renderer& renderer);
        void handleHyperboloid(Ri::Renderer& renderer);
        void handleParaboloid(Ri::Renderer& renderer);
        void handleDisk(Ri::Renderer& renderer);
        void handleTorus(Ri::Renderer& renderer);
        void handlePoints(Ri::Renderer& renderer);
        void handleCurves(Ri::Renderer& renderer);
        void handleBlobby(Ri::Renderer& renderer);
        void handleProcedural(Ri::Renderer& renderer);
        void handleGeometry(Ri::Renderer& renderer);
        void handleSolidBegin(Ri::Renderer& renderer);
        void handleSolidEnd(Ri::Renderer& renderer);
        void handleObjectBegin(Ri::Renderer& renderer);
        void handleObjectEnd(Ri::Renderer& renderer);
        void handleObjectInstance(Ri::Renderer& renderer);
        void handleMotionBegin(Ri::Renderer& renderer);
        void handleMotionEnd(Ri::Renderer& renderer);
        void handleMakeTexture(Ri::Renderer& renderer);
        void handleMakeLatLongEnvironment(Ri::Renderer& renderer);
        void handleMakeCubeFaceEnvironment(Ri::Renderer& renderer);
        void handleMakeShadow(Ri::Renderer& renderer);
        void handleMakeOcclusion(Ri::Renderer& renderer);
        void handleErrorHandler(Ri::Renderer& renderer);
        void handleReadArchive(Ri::Renderer& renderer);
        void handleArchiveBegin(Ri::Renderer& renderer);
        void handleArchiveEnd(Ri::Renderer& renderer);
        //[[[end]]]

        void handleVersion(Ri::Renderer& renderer);

        //--------------------------------------------------
        /// Renderer instance to which requests will be forwarded
        Ri::RendererServices& m_services;

        /// Lexer instance
        boost::shared_ptr<RibLexer> m_lex;

        /// Request name -> handler mapping.
        HandlerMap m_requestHandlerMap;

        // Storage for parameter list parsing
        std::vector<Ri::Param> m_paramListStorage;
        std::vector<std::string> m_paramNameStorage;

        /// Number of color components
        int m_numColorComps;
        /// Dictionary of declared tokens
        CqTokenDictionary m_tokenDict;

        /// Mapping from light numbers to handles
        std::vector<LightMap> m_lightMaps;

        /// Mapping from object numbers to handles
        std::vector<ObjectMap> m_objectMaps;
};

} // namespace Aqsis

#endif // RIBREQUESTHANDLER_H_INCLUDED
// vi: set et:
