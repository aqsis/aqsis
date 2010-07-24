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
        RibParser(Ri::Renderer& renderer);

        /// Parse a RIB stream, sending requests to the callback interface
        ///
        /// \param ribStream - RIB stream to be parsed.  May be gzipped.
        /// \param streamName - name of the stream, present in error messages
        void parseStream(std::istream& ribStream,
                         const std::string& streamName);

    private:
        /// Object ID -> handle maps
        typedef std::map<int, RtObjectHandle> ObjectMap;
        typedef std::map<std::string, RtObjectHandle> NamedObjectMap;
        /// Light ID -> handle maps
        typedef std::map<int, RtLightHandle> LightMap;
        typedef std::map<std::string, RtLightHandle> NamedLightMap;
        /// Request handler function type
        typedef void (RibParser::*RequestHandlerType)(RibLexer& lex);
        /// Request -> handler mapping type
        typedef std::map<std::string, RequestHandlerType> HandlerMap;
        /// Function pointer compatible with {Area}LightSource
        typedef RtLightHandle (Ri::Renderer::*LightSourceFunc)(
                RtConstToken shadername, const Ri::ParamList&);

        // Utilities for handlers
        Ri::ParamList readParamList(RibLexer& lex);
        /// Combined handler for LightSource & AreaLightSource
        void handleLightSourceGeneral(LightSourceFunc lightSourceFunc,
                                      RibLexer& lex);
        RtConstBasis& getBasis(RibLexer& lex) const;

        //--------------------------------------------------
        /// Handler methods definitions
        /*[[[cog
        from cogutils import *
        riXml = parseXmlTree('ri.xml')
        for p in filter(lambda p: p.haschild('Rib'), riXml.findall('Procedures/Procedure')):
            cog.outl('void handle%s(RibLexer& lex);' % (p.findtext('Name'),))

        ]]]*/
        void handleDeclare(RibLexer& lex);
        void handleFrameBegin(RibLexer& lex);
        void handleFrameEnd(RibLexer& lex);
        void handleWorldBegin(RibLexer& lex);
        void handleWorldEnd(RibLexer& lex);
        void handleIfBegin(RibLexer& lex);
        void handleElseIf(RibLexer& lex);
        void handleElse(RibLexer& lex);
        void handleIfEnd(RibLexer& lex);
        void handleFormat(RibLexer& lex);
        void handleFrameAspectRatio(RibLexer& lex);
        void handleScreenWindow(RibLexer& lex);
        void handleCropWindow(RibLexer& lex);
        void handleProjection(RibLexer& lex);
        void handleClipping(RibLexer& lex);
        void handleClippingPlane(RibLexer& lex);
        void handleDepthOfField(RibLexer& lex);
        void handleShutter(RibLexer& lex);
        void handlePixelVariance(RibLexer& lex);
        void handlePixelSamples(RibLexer& lex);
        void handlePixelFilter(RibLexer& lex);
        void handleExposure(RibLexer& lex);
        void handleImager(RibLexer& lex);
        void handleQuantize(RibLexer& lex);
        void handleDisplay(RibLexer& lex);
        void handleHider(RibLexer& lex);
        void handleColorSamples(RibLexer& lex);
        void handleRelativeDetail(RibLexer& lex);
        void handleOption(RibLexer& lex);
        void handleAttributeBegin(RibLexer& lex);
        void handleAttributeEnd(RibLexer& lex);
        void handleColor(RibLexer& lex);
        void handleOpacity(RibLexer& lex);
        void handleTextureCoordinates(RibLexer& lex);
        void handleLightSource(RibLexer& lex);
        void handleAreaLightSource(RibLexer& lex);
        void handleIlluminate(RibLexer& lex);
        void handleSurface(RibLexer& lex);
        void handleDisplacement(RibLexer& lex);
        void handleAtmosphere(RibLexer& lex);
        void handleInterior(RibLexer& lex);
        void handleExterior(RibLexer& lex);
        void handleShaderLayer(RibLexer& lex);
        void handleConnectShaderLayers(RibLexer& lex);
        void handleShadingRate(RibLexer& lex);
        void handleShadingInterpolation(RibLexer& lex);
        void handleMatte(RibLexer& lex);
        void handleBound(RibLexer& lex);
        void handleDetail(RibLexer& lex);
        void handleDetailRange(RibLexer& lex);
        void handleGeometricApproximation(RibLexer& lex);
        void handleOrientation(RibLexer& lex);
        void handleReverseOrientation(RibLexer& lex);
        void handleSides(RibLexer& lex);
        void handleIdentity(RibLexer& lex);
        void handleTransform(RibLexer& lex);
        void handleConcatTransform(RibLexer& lex);
        void handlePerspective(RibLexer& lex);
        void handleTranslate(RibLexer& lex);
        void handleRotate(RibLexer& lex);
        void handleScale(RibLexer& lex);
        void handleSkew(RibLexer& lex);
        void handleCoordinateSystem(RibLexer& lex);
        void handleCoordSysTransform(RibLexer& lex);
        void handleTransformBegin(RibLexer& lex);
        void handleTransformEnd(RibLexer& lex);
        void handleResource(RibLexer& lex);
        void handleResourceBegin(RibLexer& lex);
        void handleResourceEnd(RibLexer& lex);
        void handleAttribute(RibLexer& lex);
        void handlePolygon(RibLexer& lex);
        void handleGeneralPolygon(RibLexer& lex);
        void handlePointsPolygons(RibLexer& lex);
        void handlePointsGeneralPolygons(RibLexer& lex);
        void handleBasis(RibLexer& lex);
        void handlePatch(RibLexer& lex);
        void handlePatchMesh(RibLexer& lex);
        void handleNuPatch(RibLexer& lex);
        void handleTrimCurve(RibLexer& lex);
        void handleSubdivisionMesh(RibLexer& lex);
        void handleSphere(RibLexer& lex);
        void handleCone(RibLexer& lex);
        void handleCylinder(RibLexer& lex);
        void handleHyperboloid(RibLexer& lex);
        void handleParaboloid(RibLexer& lex);
        void handleDisk(RibLexer& lex);
        void handleTorus(RibLexer& lex);
        void handlePoints(RibLexer& lex);
        void handleCurves(RibLexer& lex);
        void handleBlobby(RibLexer& lex);
        void handleProcedural(RibLexer& lex);
        void handleGeometry(RibLexer& lex);
        void handleSolidBegin(RibLexer& lex);
        void handleSolidEnd(RibLexer& lex);
        void handleObjectBegin(RibLexer& lex);
        void handleObjectEnd(RibLexer& lex);
        void handleObjectInstance(RibLexer& lex);
        void handleMotionBegin(RibLexer& lex);
        void handleMotionEnd(RibLexer& lex);
        void handleMakeTexture(RibLexer& lex);
        void handleMakeLatLongEnvironment(RibLexer& lex);
        void handleMakeCubeFaceEnvironment(RibLexer& lex);
        void handleMakeShadow(RibLexer& lex);
        void handleMakeOcclusion(RibLexer& lex);
        void handleErrorHandler(RibLexer& lex);
        void handleReadArchive(RibLexer& lex);
        //[[[end]]]

        void handleVersion(RibLexer& lex);

        //--------------------------------------------------
        /// Renderer instance to which requests will be forwarded
        Ri::Renderer& m_renderer;

        /// Lexer instance
        boost::shared_ptr<RibLexer> m_lexer;

        /// Request name -> handler mapping.
        HandlerMap m_requestHandlerMap;

        // Storage for parameter list parsing
        std::vector<Ri::Param> m_paramListStorage;

        /// Number of color components
        int m_numColorComps;
        /// Dictionary of declared tokens
        CqTokenDictionary m_tokenDict;

        /// Mapping from light numbers to handles
        LightMap m_lightMap;
        /// Mapping from light names to handles
        NamedLightMap m_namedLightMap;

        /// Mapping from object numbers to handles
        ObjectMap m_objectMap;
        /// Mapping from object names to handles
        NamedObjectMap m_namedObjectMap;
};

} // namespace Aqsis

#endif // RIBREQUESTHANDLER_H_INCLUDED
// vi: set et:
