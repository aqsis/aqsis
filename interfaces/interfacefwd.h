//------------------------------------------------------------------------------
/**
 *	@file	interfacefwd.h
 *	@author	Paul Gregory
 *	@brief	Declare the interface pointers for various interface classes.
 *
 *	Last change by:		$Author:$
 *	Last change date:	$Date:$
 */
//------------------------------------------------------------------------------


#ifndef	___interfacefwd_Loaded___
#define	___interfacefwd_Loaded___

#include <boost/shared_ptr.hpp>

namespace Aqsis {

struct IqAttributes;
typedef boost::shared_ptr<IqAttributes> IqAttributesPtr;
typedef boost::shared_ptr<const IqAttributes> IqConstAttributesPtr;

struct IqSurface;
typedef boost::shared_ptr<IqSurface> IqSurfacePtr;
typedef boost::shared_ptr<const IqSurface> IqConstSurfacePtr;

struct IqTransform;
typedef boost::shared_ptr<IqTransform> IqTransformPtr;
typedef boost::shared_ptr<const IqTransform> IqConstTransformPtr;

struct IqLightsource;
typedef boost::shared_ptr<IqLightsource> IqLightsourcePtr;
typedef boost::shared_ptr<const IqLightsource> IqConstLightsourcePtr;

struct IqOptions;
typedef boost::shared_ptr<IqOptions> IqOptionsPtr;
typedef boost::shared_ptr<const IqOptions> IqConstOptionsPtr;

} // namespace Aqsis

#endif	//	___interfacefwd_Loaded___

