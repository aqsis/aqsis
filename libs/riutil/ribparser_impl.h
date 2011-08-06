// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/** \file
 *
 * \brief RIB parser implementation for aqsis
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 */

#ifndef AQSIS_RIBPARSERIMPL_H_INCLUDED
#define AQSIS_RIBPARSERIMPL_H_INCLUDED

#include <aqsis/aqsis.h>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <aqsis/ri/ritypes.h>
#include <aqsis/riutil/ribparser.h>
#include <aqsis/riutil/ricxx.h>
#include <aqsis/riutil/tokendictionary.h>

namespace Aqsis
{

class RibLexer;

//------------------------------------------------------------------------------
/// Parser for standard RIB streams
class RibParserImpl : public RibParser
{
    public:
        RibParserImpl(Ri::RendererServices& services);

        virtual void parseStream(std::istream& ribStream,
                                 const std::string& streamName,
                                 Ri::Renderer& context);

    private:
        /// Request handler function type
        typedef void (RibParserImpl::*RequestHandlerType)(Ri::Renderer& renderer);
        /// Request -> handler mapping type
        typedef std::map<std::string, RequestHandlerType> HandlerMap;

        Ri::ParamList readParamList();
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
};

} // namespace Aqsis

#endif // AQSIS_RIBPARSERIMPL_H_INCLUDED
// vi: set et:
