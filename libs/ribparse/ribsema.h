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
 * \brief RIB request handler implementation for aqsis.
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 */

#ifndef RIBREQUESTHANDLER_H_INCLUDED
#define RIBREQUESTHANDLER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <aqsis/ribparser.h>
#include <aqsis/ri/ritypes.h>
#include <aqsis/riutil/tokendictionary.h>

#include "ricxx.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
/// Semantic analyzer to handle RIB requests
class RibSema : public IqRibRequestHandler
{
    public:
        RibSema();

        virtual void handleRequest(const std::string& requestName,
                IqRibParser& parser);

    private:
        /// Object ID -> handle maps
        typedef std::map<int, RtObjectHandle> ObjectMap;
        typedef std::map<std::string, RtObjectHandle> NamedObjectMap;
        /// Light ID -> handle maps
        typedef std::map<int, RtLightHandle> LightMap;
        typedef std::map<std::string, RtLightHandle> NamedLightMap;
        /// Request handler function type
        typedef void (RibSema::*RequestHandlerType)(IqRibParser& parser);
        /// Request -> handler mapping type
        typedef std::map<std::string, RequestHandlerType> HandlerMap;
        /// Function pointer compatible with Ri{Area}LightSourceV
        typedef RtLightHandle (*LightSourceVFunc)(RtToken shadername,
                    RtInt count, RtToken tokens[], RtPointer values[]);

        //--------------------------------------------------
        /// Helpers for handler methods
        static RtBasis* getBasis(IqRibParser& parser);

        //--------------------------------------------------
        /// Handler methods definitions
        /*[[[cog
        from cogutils import *
        riXml = parseXmlTree('ri.xml')
        for p in filter(lambda p: p.haschild('Rib'), riXml.findall('Procedures/Procedure')):
            cog.outl('void handle%s(IqRibParser& parser);' % (p.findtext('Name'),))

        ]]]*/
        void handleDeclare(IqRibParser& parser);
        void handleFrameBegin(IqRibParser& parser);
        void handleFrameEnd(IqRibParser& parser);
        void handleWorldBegin(IqRibParser& parser);
        void handleWorldEnd(IqRibParser& parser);
        void handleIfBegin(IqRibParser& parser);
        void handleElseIf(IqRibParser& parser);
        void handleElse(IqRibParser& parser);
        void handleIfEnd(IqRibParser& parser);
        void handleFormat(IqRibParser& parser);
        void handleFrameAspectRatio(IqRibParser& parser);
        void handleScreenWindow(IqRibParser& parser);
        void handleCropWindow(IqRibParser& parser);
        void handleProjection(IqRibParser& parser);
        void handleClipping(IqRibParser& parser);
        void handleClippingPlane(IqRibParser& parser);
        void handleDepthOfField(IqRibParser& parser);
        void handleShutter(IqRibParser& parser);
        void handlePixelVariance(IqRibParser& parser);
        void handlePixelSamples(IqRibParser& parser);
        void handlePixelFilter(IqRibParser& parser);
        void handleExposure(IqRibParser& parser);
        void handleImager(IqRibParser& parser);
        void handleQuantize(IqRibParser& parser);
        void handleDisplay(IqRibParser& parser);
        void handleHider(IqRibParser& parser);
        void handleColorSamples(IqRibParser& parser);
        void handleRelativeDetail(IqRibParser& parser);
        void handleOption(IqRibParser& parser);
        void handleAttributeBegin(IqRibParser& parser);
        void handleAttributeEnd(IqRibParser& parser);
        void handleColor(IqRibParser& parser);
        void handleOpacity(IqRibParser& parser);
        void handleTextureCoordinates(IqRibParser& parser);
        void handleLightSource(IqRibParser& parser);
        void handleAreaLightSource(IqRibParser& parser);
        void handleIlluminate(IqRibParser& parser);
        void handleSurface(IqRibParser& parser);
        void handleDisplacement(IqRibParser& parser);
        void handleAtmosphere(IqRibParser& parser);
        void handleInterior(IqRibParser& parser);
        void handleExterior(IqRibParser& parser);
        void handleShaderLayer(IqRibParser& parser);
        void handleConnectShaderLayers(IqRibParser& parser);
        void handleShadingRate(IqRibParser& parser);
        void handleShadingInterpolation(IqRibParser& parser);
        void handleMatte(IqRibParser& parser);
        void handleBound(IqRibParser& parser);
        void handleDetail(IqRibParser& parser);
        void handleDetailRange(IqRibParser& parser);
        void handleGeometricApproximation(IqRibParser& parser);
        void handleOrientation(IqRibParser& parser);
        void handleReverseOrientation(IqRibParser& parser);
        void handleSides(IqRibParser& parser);
        void handleIdentity(IqRibParser& parser);
        void handleTransform(IqRibParser& parser);
        void handleConcatTransform(IqRibParser& parser);
        void handlePerspective(IqRibParser& parser);
        void handleTranslate(IqRibParser& parser);
        void handleRotate(IqRibParser& parser);
        void handleScale(IqRibParser& parser);
        void handleSkew(IqRibParser& parser);
        void handleCoordinateSystem(IqRibParser& parser);
        void handleCoordSysTransform(IqRibParser& parser);
        void handleTransformBegin(IqRibParser& parser);
        void handleTransformEnd(IqRibParser& parser);
        void handleResource(IqRibParser& parser);
        void handleResourceBegin(IqRibParser& parser);
        void handleResourceEnd(IqRibParser& parser);
        void handleAttribute(IqRibParser& parser);
        void handlePolygon(IqRibParser& parser);
        void handleGeneralPolygon(IqRibParser& parser);
        void handlePointsPolygons(IqRibParser& parser);
        void handlePointsGeneralPolygons(IqRibParser& parser);
        void handleBasis(IqRibParser& parser);
        void handlePatch(IqRibParser& parser);
        void handlePatchMesh(IqRibParser& parser);
        void handleNuPatch(IqRibParser& parser);
        void handleTrimCurve(IqRibParser& parser);
        void handleSubdivisionMesh(IqRibParser& parser);
        void handleSphere(IqRibParser& parser);
        void handleCone(IqRibParser& parser);
        void handleCylinder(IqRibParser& parser);
        void handleHyperboloid(IqRibParser& parser);
        void handleParaboloid(IqRibParser& parser);
        void handleDisk(IqRibParser& parser);
        void handleTorus(IqRibParser& parser);
        void handlePoints(IqRibParser& parser);
        void handleCurves(IqRibParser& parser);
        void handleBlobby(IqRibParser& parser);
        void handleProcedural(IqRibParser& parser);
        void handleGeometry(IqRibParser& parser);
        void handleSolidBegin(IqRibParser& parser);
        void handleSolidEnd(IqRibParser& parser);
        void handleObjectBegin(IqRibParser& parser);
        void handleObjectEnd(IqRibParser& parser);
        void handleObjectInstance(IqRibParser& parser);
        void handleMotionBegin(IqRibParser& parser);
        void handleMotionEnd(IqRibParser& parser);
        void handleMakeTexture(IqRibParser& parser);
        void handleMakeLatLongEnvironment(IqRibParser& parser);
        void handleMakeCubeFaceEnvironment(IqRibParser& parser);
        void handleMakeShadow(IqRibParser& parser);
        void handleMakeOcclusion(IqRibParser& parser);
        void handleErrorHandler(IqRibParser& parser);
        void handleReadArchive(IqRibParser& parser);
        //[[[end]]]

        void handleVersion(IqRibParser& parser);

        //--------------------------------------------------
        // Instance data
        /// Request name -> handler mapping.
        HandlerMap m_requestHandlerMap;

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
