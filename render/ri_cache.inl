class RiDeclareCache : public RiCacheBase
{
public:
	RiDeclareCache(RtString name, RtString declaration)
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		int __declaration_length = strlen(declaration);
		m_declaration = new char[ __declaration_length + 1 ];
		strcpy(m_declaration, declaration);
	}
	virtual ~RiDeclareCache()
	{
		delete[](m_name);
		delete[](m_declaration);
	}
	virtual void ReCall()
	{
		RiDeclare(m_name, m_declaration);
	}

private:
	RtString m_name;
	RtString m_declaration;
};

class RiFrameBeginCache : public RiCacheBase
{
public:
	RiFrameBeginCache(RtInt number)
	{
		m_number = number;
	}
	virtual ~RiFrameBeginCache()
	{
	}
	virtual void ReCall()
	{
		RiFrameBegin(m_number);
	}

private:
	RtInt m_number;
};

class RiFrameEndCache : public RiCacheBase
{
public:
	RiFrameEndCache()
	{
	}
	virtual ~RiFrameEndCache()
	{
	}
	virtual void ReCall()
	{
		RiFrameEnd();
	}

private:
};

class RiWorldBeginCache : public RiCacheBase
{
public:
	RiWorldBeginCache()
	{
	}
	virtual ~RiWorldBeginCache()
	{
	}
	virtual void ReCall()
	{
		RiWorldBegin();
	}

private:
};

class RiWorldEndCache : public RiCacheBase
{
public:
	RiWorldEndCache()
	{
	}
	virtual ~RiWorldEndCache()
	{
	}
	virtual void ReCall()
	{
		RiWorldEnd();
	}

private:
};

class RiFormatCache : public RiCacheBase
{
public:
	RiFormatCache(RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio)
	{
		m_xresolution = xresolution;
		m_yresolution = yresolution;
		m_pixelaspectratio = pixelaspectratio;
	}
	virtual ~RiFormatCache()
	{
	}
	virtual void ReCall()
	{
		RiFormat(m_xresolution, m_yresolution, m_pixelaspectratio);
	}

private:
	RtInt m_xresolution;
	RtInt m_yresolution;
	RtFloat m_pixelaspectratio;
};

class RiFrameAspectRatioCache : public RiCacheBase
{
public:
	RiFrameAspectRatioCache(RtFloat frameratio)
	{
		m_frameratio = frameratio;
	}
	virtual ~RiFrameAspectRatioCache()
	{
	}
	virtual void ReCall()
	{
		RiFrameAspectRatio(m_frameratio);
	}

private:
	RtFloat m_frameratio;
};

class RiScreenWindowCache : public RiCacheBase
{
public:
	RiScreenWindowCache(RtFloat left, RtFloat right, RtFloat bottom, RtFloat top)
	{
		m_left = left;
		m_right = right;
		m_bottom = bottom;
		m_top = top;
	}
	virtual ~RiScreenWindowCache()
	{
	}
	virtual void ReCall()
	{
		RiScreenWindow(m_left, m_right, m_bottom, m_top);
	}

private:
	RtFloat m_left;
	RtFloat m_right;
	RtFloat m_bottom;
	RtFloat m_top;
};

class RiCropWindowCache : public RiCacheBase
{
public:
	RiCropWindowCache(RtFloat left, RtFloat right, RtFloat top, RtFloat bottom)
	{
		m_left = left;
		m_right = right;
		m_top = top;
		m_bottom = bottom;
	}
	virtual ~RiCropWindowCache()
	{
	}
	virtual void ReCall()
	{
		RiCropWindow(m_left, m_right, m_top, m_bottom);
	}

private:
	RtFloat m_left;
	RtFloat m_right;
	RtFloat m_top;
	RtFloat m_bottom;
};

class RiProjectionVCache : public RiCacheBase
{
public:
	RiProjectionVCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
	}
	virtual ~RiProjectionVCache()
	{
		delete[](m_name);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiProjectionV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiClippingCache : public RiCacheBase
{
public:
	RiClippingCache(RtFloat cnear, RtFloat cfar)
	{
		m_cnear = cnear;
		m_cfar = cfar;
	}
	virtual ~RiClippingCache()
	{
	}
	virtual void ReCall()
	{
		RiClipping(m_cnear, m_cfar);
	}

private:
	RtFloat m_cnear;
	RtFloat m_cfar;
};

class RiDepthOfFieldCache : public RiCacheBase
{
public:
	RiDepthOfFieldCache(RtFloat fstop, RtFloat focallength, RtFloat focaldistance)
	{
		m_fstop = fstop;
		m_focallength = focallength;
		m_focaldistance = focaldistance;
	}
	virtual ~RiDepthOfFieldCache()
	{
	}
	virtual void ReCall()
	{
		RiDepthOfField(m_fstop, m_focallength, m_focaldistance);
	}

private:
	RtFloat m_fstop;
	RtFloat m_focallength;
	RtFloat m_focaldistance;
};

class RiShutterCache : public RiCacheBase
{
public:
	RiShutterCache(RtFloat opentime, RtFloat closetime)
	{
		m_opentime = opentime;
		m_closetime = closetime;
	}
	virtual ~RiShutterCache()
	{
	}
	virtual void ReCall()
	{
		RiShutter(m_opentime, m_closetime);
	}

private:
	RtFloat m_opentime;
	RtFloat m_closetime;
};

class RiPixelVarianceCache : public RiCacheBase
{
public:
	RiPixelVarianceCache(RtFloat variance)
	{
		m_variance = variance;
	}
	virtual ~RiPixelVarianceCache()
	{
	}
	virtual void ReCall()
	{
		RiPixelVariance(m_variance);
	}

private:
	RtFloat m_variance;
};

class RiPixelSamplesCache : public RiCacheBase
{
public:
	RiPixelSamplesCache(RtFloat xsamples, RtFloat ysamples)
	{
		m_xsamples = xsamples;
		m_ysamples = ysamples;
	}
	virtual ~RiPixelSamplesCache()
	{
	}
	virtual void ReCall()
	{
		RiPixelSamples(m_xsamples, m_ysamples);
	}

private:
	RtFloat m_xsamples;
	RtFloat m_ysamples;
};

class RiPixelFilterCache : public RiCacheBase
{
public:
	RiPixelFilterCache(RtFilterFunc function, RtFloat xwidth, RtFloat ywidth)
	{
		m_function = function;
		m_xwidth = xwidth;
		m_ywidth = ywidth;
	}
	virtual ~RiPixelFilterCache()
	{
	}
	virtual void ReCall()
	{
		RiPixelFilter(m_function, m_xwidth, m_ywidth);
	}

private:
	RtFilterFunc m_function;
	RtFloat m_xwidth;
	RtFloat m_ywidth;
};

class RiExposureCache : public RiCacheBase
{
public:
	RiExposureCache(RtFloat gain, RtFloat gamma)
	{
		m_gain = gain;
		m_gamma = gamma;
	}
	virtual ~RiExposureCache()
	{
	}
	virtual void ReCall()
	{
		RiExposure(m_gain, m_gamma);
	}

private:
	RtFloat m_gain;
	RtFloat m_gamma;
};

class RiImagerVCache : public RiCacheBase
{
public:
	RiImagerVCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
	}
	virtual ~RiImagerVCache()
	{
		delete[](m_name);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiImagerV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiQuantizeCache : public RiCacheBase
{
public:
	RiQuantizeCache(RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ditheramplitude)
	{
		int __type_length = strlen(type);
		m_type = new char[ __type_length + 1 ];
		strcpy(m_type, type);
		m_one = one;
		m_min = min;
		m_max = max;
		m_ditheramplitude = ditheramplitude;
	}
	virtual ~RiQuantizeCache()
	{
		delete[](m_type);
	}
	virtual void ReCall()
	{
		RiQuantize(m_type, m_one, m_min, m_max, m_ditheramplitude);
	}

private:
	RtToken m_type;
	RtInt m_one;
	RtInt m_min;
	RtInt m_max;
	RtFloat m_ditheramplitude;
};

class RiDisplayVCache : public RiCacheBase
{
public:
	RiDisplayVCache(RtToken name, RtToken type, RtToken mode, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		int __type_length = strlen(type);
		m_type = new char[ __type_length + 1 ];
		strcpy(m_type, type);
		int __mode_length = strlen(mode);
		m_mode = new char[ __mode_length + 1 ];
		strcpy(m_mode, mode);
		// Copy the plist here.
	}
	virtual ~RiDisplayVCache()
	{
		delete[](m_name);
		delete[](m_type);
		delete[](m_mode);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiDisplayV(m_name, m_type, m_mode, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	RtToken m_type;
	RtToken m_mode;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiHiderVCache : public RiCacheBase
{
public:
	RiHiderVCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
	}
	virtual ~RiHiderVCache()
	{
		delete[](m_name);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiHiderV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiColorSamplesCache : public RiCacheBase
{
public:
	RiColorSamplesCache(RtInt N, RtFloat nRGB[], RtFloat RGBn[])
	{
		m_N = N;
		int __nRGB_length = N;
		m_nRGB = new RtFloat[__nRGB_length];
		int __nRGB_index;
		for(__nRGB_index = 0; __nRGB_index<__nRGB_length; __nRGB_index++)
		{
			m_nRGB[__nRGB_index] = nRGB[__nRGB_index];
		}
		int __RGBn_length = N;
		m_RGBn = new RtFloat[__RGBn_length];
		int __RGBn_index;
		for(__RGBn_index = 0; __RGBn_index<__RGBn_length; __RGBn_index++)
		{
			m_RGBn[__RGBn_index] = RGBn[__RGBn_index];
		}
	}
	virtual ~RiColorSamplesCache()
	{
		delete[](m_nRGB);
		delete[](m_RGBn);
	}
	virtual void ReCall()
	{
		RiColorSamples(m_N, m_nRGB, m_RGBn);
	}

private:
	RtInt m_N;
	RtFloat* m_nRGB;
	RtFloat* m_RGBn;
};

class RiRelativeDetailCache : public RiCacheBase
{
public:
	RiRelativeDetailCache(RtFloat relativedetail)
	{
		m_relativedetail = relativedetail;
	}
	virtual ~RiRelativeDetailCache()
	{
	}
	virtual void ReCall()
	{
		RiRelativeDetail(m_relativedetail);
	}

private:
	RtFloat m_relativedetail;
};

class RiOptionVCache : public RiCacheBase
{
public:
	RiOptionVCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
	}
	virtual ~RiOptionVCache()
	{
		delete[](m_name);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiOptionV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiAttributeBeginCache : public RiCacheBase
{
public:
	RiAttributeBeginCache()
	{
	}
	virtual ~RiAttributeBeginCache()
	{
	}
	virtual void ReCall()
	{
		RiAttributeBegin();
	}

private:
};

class RiAttributeEndCache : public RiCacheBase
{
public:
	RiAttributeEndCache()
	{
	}
	virtual ~RiAttributeEndCache()
	{
	}
	virtual void ReCall()
	{
		RiAttributeEnd();
	}

private:
};

class RiColorCache : public RiCacheBase
{
public:
	RiColorCache(RtColor Cq)
	{
		m_Cq[0] = Cq[0];
		m_Cq[1] = Cq[1];
		m_Cq[2] = Cq[2];
	}
	virtual ~RiColorCache()
	{
	}
	virtual void ReCall()
	{
		RiColor(m_Cq);
	}

private:
	RtColor m_Cq;
};

class RiOpacityCache : public RiCacheBase
{
public:
	RiOpacityCache(RtColor Os)
	{
		m_Os[0] = Os[0];
		m_Os[1] = Os[1];
		m_Os[2] = Os[2];
	}
	virtual ~RiOpacityCache()
	{
	}
	virtual void ReCall()
	{
		RiOpacity(m_Os);
	}

private:
	RtColor m_Os;
};

class RiTextureCoordinatesCache : public RiCacheBase
{
public:
	RiTextureCoordinatesCache(RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4)
	{
		m_s1 = s1;
		m_t1 = t1;
		m_s2 = s2;
		m_t2 = t2;
		m_s3 = s3;
		m_t3 = t3;
		m_s4 = s4;
		m_t4 = t4;
	}
	virtual ~RiTextureCoordinatesCache()
	{
	}
	virtual void ReCall()
	{
		RiTextureCoordinates(m_s1, m_t1, m_s2, m_t2, m_s3, m_t3, m_s4, m_t4);
	}

private:
	RtFloat m_s1;
	RtFloat m_t1;
	RtFloat m_s2;
	RtFloat m_t2;
	RtFloat m_s3;
	RtFloat m_t3;
	RtFloat m_s4;
	RtFloat m_t4;
};

class RiLightSourceVCache : public RiCacheBase
{
public:
	RiLightSourceVCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
	}
	virtual ~RiLightSourceVCache()
	{
		delete[](m_name);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiLightSourceV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiAreaLightSourceVCache : public RiCacheBase
{
public:
	RiAreaLightSourceVCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
	}
	virtual ~RiAreaLightSourceVCache()
	{
		delete[](m_name);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiAreaLightSourceV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiIlluminateCache : public RiCacheBase
{
public:
	RiIlluminateCache(RtLightHandle light, RtBoolean onoff)
	{
		m_light = light;
		m_onoff = onoff;
	}
	virtual ~RiIlluminateCache()
	{
	}
	virtual void ReCall()
	{
		RiIlluminate(m_light, m_onoff);
	}

private:
	RtLightHandle m_light;
	RtBoolean m_onoff;
};

class RiSurfaceVCache : public RiCacheBase
{
public:
	RiSurfaceVCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
	}
	virtual ~RiSurfaceVCache()
	{
		delete[](m_name);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiSurfaceV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiAtmosphereVCache : public RiCacheBase
{
public:
	RiAtmosphereVCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
	}
	virtual ~RiAtmosphereVCache()
	{
		delete[](m_name);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiAtmosphereV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiInteriorVCache : public RiCacheBase
{
public:
	RiInteriorVCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
	}
	virtual ~RiInteriorVCache()
	{
		delete[](m_name);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiInteriorV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiExteriorVCache : public RiCacheBase
{
public:
	RiExteriorVCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
	}
	virtual ~RiExteriorVCache()
	{
		delete[](m_name);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiExteriorV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiShadingRateCache : public RiCacheBase
{
public:
	RiShadingRateCache(RtFloat size)
	{
		m_size = size;
	}
	virtual ~RiShadingRateCache()
	{
	}
	virtual void ReCall()
	{
		RiShadingRate(m_size);
	}

private:
	RtFloat m_size;
};

class RiShadingInterpolationCache : public RiCacheBase
{
public:
	RiShadingInterpolationCache(RtToken type)
	{
		int __type_length = strlen(type);
		m_type = new char[ __type_length + 1 ];
		strcpy(m_type, type);
	}
	virtual ~RiShadingInterpolationCache()
	{
		delete[](m_type);
	}
	virtual void ReCall()
	{
		RiShadingInterpolation(m_type);
	}

private:
	RtToken m_type;
};

class RiMatteCache : public RiCacheBase
{
public:
	RiMatteCache(RtBoolean onoff)
	{
		m_onoff = onoff;
	}
	virtual ~RiMatteCache()
	{
	}
	virtual void ReCall()
	{
		RiMatte(m_onoff);
	}

private:
	RtBoolean m_onoff;
};

class RiBoundCache : public RiCacheBase
{
public:
	RiBoundCache(RtBound bound)
	{
		m_bound[0] = bound[0];
		m_bound[1] = bound[1];
		m_bound[2] = bound[2];
		m_bound[3] = bound[3];
		m_bound[4] = bound[4];
		m_bound[5] = bound[5];
	}
	virtual ~RiBoundCache()
	{
	}
	virtual void ReCall()
	{
		RiBound(m_bound);
	}

private:
	RtBound m_bound;
};

class RiDetailCache : public RiCacheBase
{
public:
	RiDetailCache(RtBound bound)
	{
		m_bound[0] = bound[0];
		m_bound[1] = bound[1];
		m_bound[2] = bound[2];
		m_bound[3] = bound[3];
		m_bound[4] = bound[4];
		m_bound[5] = bound[5];
	}
	virtual ~RiDetailCache()
	{
	}
	virtual void ReCall()
	{
		RiDetail(m_bound);
	}

private:
	RtBound m_bound;
};

class RiDetailRangeCache : public RiCacheBase
{
public:
	RiDetailRangeCache(RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh)
	{
		m_offlow = offlow;
		m_onlow = onlow;
		m_onhigh = onhigh;
		m_offhigh = offhigh;
	}
	virtual ~RiDetailRangeCache()
	{
	}
	virtual void ReCall()
	{
		RiDetailRange(m_offlow, m_onlow, m_onhigh, m_offhigh);
	}

private:
	RtFloat m_offlow;
	RtFloat m_onlow;
	RtFloat m_onhigh;
	RtFloat m_offhigh;
};

class RiGeometricApproximationCache : public RiCacheBase
{
public:
	RiGeometricApproximationCache(RtToken type, RtFloat value)
	{
		int __type_length = strlen(type);
		m_type = new char[ __type_length + 1 ];
		strcpy(m_type, type);
		m_value = value;
	}
	virtual ~RiGeometricApproximationCache()
	{
		delete[](m_type);
	}
	virtual void ReCall()
	{
		RiGeometricApproximation(m_type, m_value);
	}

private:
	RtToken m_type;
	RtFloat m_value;
};

class RiOrientationCache : public RiCacheBase
{
public:
	RiOrientationCache(RtToken orientation)
	{
		int __orientation_length = strlen(orientation);
		m_orientation = new char[ __orientation_length + 1 ];
		strcpy(m_orientation, orientation);
	}
	virtual ~RiOrientationCache()
	{
		delete[](m_orientation);
	}
	virtual void ReCall()
	{
		RiOrientation(m_orientation);
	}

private:
	RtToken m_orientation;
};

class RiReverseOrientationCache : public RiCacheBase
{
public:
	RiReverseOrientationCache()
	{
	}
	virtual ~RiReverseOrientationCache()
	{
	}
	virtual void ReCall()
	{
		RiReverseOrientation();
	}

private:
};

class RiSidesCache : public RiCacheBase
{
public:
	RiSidesCache(RtInt nsides)
	{
		m_nsides = nsides;
	}
	virtual ~RiSidesCache()
	{
	}
	virtual void ReCall()
	{
		RiSides(m_nsides);
	}

private:
	RtInt m_nsides;
};

class RiIdentityCache : public RiCacheBase
{
public:
	RiIdentityCache()
	{
	}
	virtual ~RiIdentityCache()
	{
	}
	virtual void ReCall()
	{
		RiIdentity();
	}

private:
};

class RiTransformCache : public RiCacheBase
{
public:
	RiTransformCache(RtMatrix transform)
	{
		int __transform_i, __transform_j;
		for(__transform_j = 0; __transform_j<4; __transform_j++)
			for(__transform_i = 0; __transform_i<4; __transform_i++)
				m_transform[__transform_j][__transform_i] = transform[__transform_j][__transform_i];
	}
	virtual ~RiTransformCache()
	{
	}
	virtual void ReCall()
	{
		RiTransform(m_transform);
	}

private:
	RtMatrix m_transform;
};

class RiConcatTransformCache : public RiCacheBase
{
public:
	RiConcatTransformCache(RtMatrix transform)
	{
		int __transform_i, __transform_j;
		for(__transform_j = 0; __transform_j<4; __transform_j++)
			for(__transform_i = 0; __transform_i<4; __transform_i++)
				m_transform[__transform_j][__transform_i] = transform[__transform_j][__transform_i];
	}
	virtual ~RiConcatTransformCache()
	{
	}
	virtual void ReCall()
	{
		RiConcatTransform(m_transform);
	}

private:
	RtMatrix m_transform;
};

class RiPerspectiveCache : public RiCacheBase
{
public:
	RiPerspectiveCache(RtFloat fov)
	{
		m_fov = fov;
	}
	virtual ~RiPerspectiveCache()
	{
	}
	virtual void ReCall()
	{
		RiPerspective(m_fov);
	}

private:
	RtFloat m_fov;
};

class RiTranslateCache : public RiCacheBase
{
public:
	RiTranslateCache(RtFloat dx, RtFloat dy, RtFloat dz)
	{
		m_dx = dx;
		m_dy = dy;
		m_dz = dz;
	}
	virtual ~RiTranslateCache()
	{
	}
	virtual void ReCall()
	{
		RiTranslate(m_dx, m_dy, m_dz);
	}

private:
	RtFloat m_dx;
	RtFloat m_dy;
	RtFloat m_dz;
};

class RiRotateCache : public RiCacheBase
{
public:
	RiRotateCache(RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz)
	{
		m_angle = angle;
		m_dx = dx;
		m_dy = dy;
		m_dz = dz;
	}
	virtual ~RiRotateCache()
	{
	}
	virtual void ReCall()
	{
		RiRotate(m_angle, m_dx, m_dy, m_dz);
	}

private:
	RtFloat m_angle;
	RtFloat m_dx;
	RtFloat m_dy;
	RtFloat m_dz;
};

class RiScaleCache : public RiCacheBase
{
public:
	RiScaleCache(RtFloat sx, RtFloat sy, RtFloat sz)
	{
		m_sx = sx;
		m_sy = sy;
		m_sz = sz;
	}
	virtual ~RiScaleCache()
	{
	}
	virtual void ReCall()
	{
		RiScale(m_sx, m_sy, m_sz);
	}

private:
	RtFloat m_sx;
	RtFloat m_sy;
	RtFloat m_sz;
};

class RiSkewCache : public RiCacheBase
{
public:
	RiSkewCache(RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1, RtFloat dx2, RtFloat dy2, RtFloat dz2)
	{
		m_angle = angle;
		m_dx1 = dx1;
		m_dy1 = dy1;
		m_dz1 = dz1;
		m_dx2 = dx2;
		m_dy2 = dy2;
		m_dz2 = dz2;
	}
	virtual ~RiSkewCache()
	{
	}
	virtual void ReCall()
	{
		RiSkew(m_angle, m_dx1, m_dy1, m_dz1, m_dx2, m_dy2, m_dz2);
	}

private:
	RtFloat m_angle;
	RtFloat m_dx1;
	RtFloat m_dy1;
	RtFloat m_dz1;
	RtFloat m_dx2;
	RtFloat m_dy2;
	RtFloat m_dz2;
};

class RiDeformationVCache : public RiCacheBase
{
public:
	RiDeformationVCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
	}
	virtual ~RiDeformationVCache()
	{
		delete[](m_name);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiDeformationV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiDisplacementVCache : public RiCacheBase
{
public:
	RiDisplacementVCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
	}
	virtual ~RiDisplacementVCache()
	{
		delete[](m_name);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiDisplacementV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiCoordinateSystemCache : public RiCacheBase
{
public:
	RiCoordinateSystemCache(RtToken space)
	{
		int __space_length = strlen(space);
		m_space = new char[ __space_length + 1 ];
		strcpy(m_space, space);
	}
	virtual ~RiCoordinateSystemCache()
	{
		delete[](m_space);
	}
	virtual void ReCall()
	{
		RiCoordinateSystem(m_space);
	}

private:
	RtToken m_space;
};

class RiTransformPointsCache : public RiCacheBase
{
public:
	RiTransformPointsCache(RtToken fromspace, RtToken tospace, RtInt npoints, RtPoint points[])
	{
		int __fromspace_length = strlen(fromspace);
		m_fromspace = new char[ __fromspace_length + 1 ];
		strcpy(m_fromspace, fromspace);
		int __tospace_length = strlen(tospace);
		m_tospace = new char[ __tospace_length + 1 ];
		strcpy(m_tospace, tospace);
		m_npoints = npoints;
		int __points_length = npoints;
		m_points = new RtPoint[__points_length];
		int __points_index;
		for(__points_index = 0; __points_index<__points_length; __points_index++)
		{
			m_points[__points_index][0] = points[__points_index][0];
			m_points[__points_index][1] = points[__points_index][1];
			m_points[__points_index][2] = points[__points_index][2];
		}
	}
	virtual ~RiTransformPointsCache()
	{
		delete[](m_fromspace);
		delete[](m_tospace);
		delete[](m_points);
	}
	virtual void ReCall()
	{
		RiTransformPoints(m_fromspace, m_tospace, m_npoints, m_points);
	}

private:
	RtToken m_fromspace;
	RtToken m_tospace;
	RtInt m_npoints;
	RtPoint* m_points;
};

class RiTransformBeginCache : public RiCacheBase
{
public:
	RiTransformBeginCache()
	{
	}
	virtual ~RiTransformBeginCache()
	{
	}
	virtual void ReCall()
	{
		RiTransformBegin();
	}

private:
};

class RiTransformEndCache : public RiCacheBase
{
public:
	RiTransformEndCache()
	{
	}
	virtual ~RiTransformEndCache()
	{
	}
	virtual void ReCall()
	{
		RiTransformEnd();
	}

private:
};

class RiAttributeVCache : public RiCacheBase
{
public:
	RiAttributeVCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
	}
	virtual ~RiAttributeVCache()
	{
		delete[](m_name);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiAttributeV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiPolygonVCache : public RiCacheBase
{
public:
	RiPolygonVCache(RtInt nvertices, RtInt count, RtToken tokens[], RtPointer values[])
	{
		m_nvertices = nvertices;
		// Copy the plist here.
	}
	virtual ~RiPolygonVCache()
	{
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiPolygonV(m_nvertices, m_count, m_tokens, m_values);
	}

private:
	RtInt m_nvertices;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiGeneralPolygonVCache : public RiCacheBase
{
public:
	RiGeneralPolygonVCache(RtInt nloops, RtInt nverts[], RtInt count, RtToken tokens[], RtPointer values[])
	{
		m_nloops = nloops;
		int __nverts_length = nloops;
		m_nverts = new RtInt[__nverts_length];
		int __nverts_index;
		for(__nverts_index = 0; __nverts_index<__nverts_length; __nverts_index++)
		{
			m_nverts[__nverts_index] = nverts[__nverts_index];
		}
		// Copy the plist here.
	}
	virtual ~RiGeneralPolygonVCache()
	{
		delete[](m_nverts);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiGeneralPolygonV(m_nloops, m_nverts, m_count, m_tokens, m_values);
	}

private:
	RtInt m_nloops;
	RtInt* m_nverts;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiPointsPolygonsVCache : public RiCacheBase
{
public:
	RiPointsPolygonsVCache(RtInt npolys, RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[])
	{
		m_npolys = npolys;
		int __nverts_length = npolys;
		m_nverts = new RtInt[__nverts_length];
		int __nverts_index;
		for(__nverts_index = 0; __nverts_index<__nverts_length; __nverts_index++)
		{
			m_nverts[__nverts_index] = nverts[__nverts_index];
		}
		int __verts_length = 0;
		{
			int __i;
			for(__i=0; __i<npolys; __i++)
				__verts_length+=nverts[__i];
		}
		m_verts = new RtInt[__verts_length];
		int __verts_index;
		for(__verts_index = 0; __verts_index<__verts_length; __verts_index++)
		{
			m_verts[__verts_index] = verts[__verts_index];
		}
		// Copy the plist here.
	}
	virtual ~RiPointsPolygonsVCache()
	{
		delete[](m_nverts);
		delete[](m_verts);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiPointsPolygonsV(m_npolys, m_nverts, m_verts, m_count, m_tokens, m_values);
	}

private:
	RtInt m_npolys;
	RtInt* m_nverts;
	RtInt* m_verts;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiPointsGeneralPolygonsVCache : public RiCacheBase
{
public:
	RiPointsGeneralPolygonsVCache(RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[])
	{
		m_npolys = npolys;
		int __nloops_length = npolys;
		m_nloops = new RtInt[__nloops_length];
		int __nloops_index;
		for(__nloops_index = 0; __nloops_index<__nloops_length; __nloops_index++)
		{
			m_nloops[__nloops_index] = nloops[__nloops_index];
		}
		int __nverts_length = 0;
		{
			int __i;
			for(__i=0; __i<npolys; __i++)
				__nverts_length+=nloops[__i];
		}
		m_nverts = new RtInt[__nverts_length];
		int __nverts_index;
		for(__nverts_index = 0; __nverts_index<__nverts_length; __nverts_index++)
		{
			m_nverts[__nverts_index] = nverts[__nverts_index];
		}
		int __verts_length = 0;
		{
			int __i;
			for(__i=0; __i<__nverts_length; __i++)
				__verts_length+=nverts[__i];
		}
		m_verts = new RtInt[__verts_length];
		int __verts_index;
		for(__verts_index = 0; __verts_index<__verts_length; __verts_index++)
		{
			m_verts[__verts_index] = verts[__verts_index];
		}
		// Copy the plist here.
	}
	virtual ~RiPointsGeneralPolygonsVCache()
	{
		delete[](m_nloops);
		delete[](m_nverts);
		delete[](m_verts);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiPointsGeneralPolygonsV(m_npolys, m_nloops, m_nverts, m_verts, m_count, m_tokens, m_values);
	}

private:
	RtInt m_npolys;
	RtInt* m_nloops;
	RtInt* m_nverts;
	RtInt* m_verts;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiBasisCache : public RiCacheBase
{
public:
	RiBasisCache(RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep)
	{
		int __ubasis_i, __ubasis_j;
		for(__ubasis_j = 0; __ubasis_j<4; __ubasis_j++)
			for(__ubasis_i = 0; __ubasis_i<4; __ubasis_i++)
				m_ubasis[__ubasis_j][__ubasis_i] = ubasis[__ubasis_j][__ubasis_i];
		m_ustep = ustep;
		int __vbasis_i, __vbasis_j;
		for(__vbasis_j = 0; __vbasis_j<4; __vbasis_j++)
			for(__vbasis_i = 0; __vbasis_i<4; __vbasis_i++)
				m_vbasis[__vbasis_j][__vbasis_i] = vbasis[__vbasis_j][__vbasis_i];
		m_vstep = vstep;
	}
	virtual ~RiBasisCache()
	{
	}
	virtual void ReCall()
	{
		RiBasis(m_ubasis, m_ustep, m_vbasis, m_vstep);
	}

private:
	RtBasis m_ubasis;
	RtInt m_ustep;
	RtBasis m_vbasis;
	RtInt m_vstep;
};

class RiPatchVCache : public RiCacheBase
{
public:
	RiPatchVCache(RtToken type, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __type_length = strlen(type);
		m_type = new char[ __type_length + 1 ];
		strcpy(m_type, type);
		// Copy the plist here.
	}
	virtual ~RiPatchVCache()
	{
		delete[](m_type);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiPatchV(m_type, m_count, m_tokens, m_values);
	}

private:
	RtToken m_type;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiPatchMeshVCache : public RiCacheBase
{
public:
	RiPatchMeshVCache(RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __type_length = strlen(type);
		m_type = new char[ __type_length + 1 ];
		strcpy(m_type, type);
		m_nu = nu;
		int __uwrap_length = strlen(uwrap);
		m_uwrap = new char[ __uwrap_length + 1 ];
		strcpy(m_uwrap, uwrap);
		m_nv = nv;
		int __vwrap_length = strlen(vwrap);
		m_vwrap = new char[ __vwrap_length + 1 ];
		strcpy(m_vwrap, vwrap);
		// Copy the plist here.
	}
	virtual ~RiPatchMeshVCache()
	{
		delete[](m_type);
		delete[](m_uwrap);
		delete[](m_vwrap);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiPatchMeshV(m_type, m_nu, m_uwrap, m_nv, m_vwrap, m_count, m_tokens, m_values);
	}

private:
	RtToken m_type;
	RtInt m_nu;
	RtToken m_uwrap;
	RtInt m_nv;
	RtToken m_vwrap;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiNuPatchVCache : public RiCacheBase
{
public:
	RiNuPatchVCache(RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, RtInt count, RtToken tokens[], RtPointer values[])
	{
		m_nu = nu;
		m_uorder = uorder;
		int __uknot_length = nu + uorder;
		m_uknot = new RtFloat[__uknot_length];
		int __uknot_index;
		for(__uknot_index = 0; __uknot_index<__uknot_length; __uknot_index++)
		{
			m_uknot[__uknot_index] = uknot[__uknot_index];
		}
		m_umin = umin;
		m_umax = umax;
		m_nv = nv;
		m_vorder = vorder;
		int __vknot_length = nv + vorder;
		m_vknot = new RtFloat[__vknot_length];
		int __vknot_index;
		for(__vknot_index = 0; __vknot_index<__vknot_length; __vknot_index++)
		{
			m_vknot[__vknot_index] = vknot[__vknot_index];
		}
		m_vmin = vmin;
		m_vmax = vmax;
		// Copy the plist here.
	}
	virtual ~RiNuPatchVCache()
	{
		delete[](m_uknot);
		delete[](m_vknot);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiNuPatchV(m_nu, m_uorder, m_uknot, m_umin, m_umax, m_nv, m_vorder, m_vknot, m_vmin, m_vmax, m_count, m_tokens, m_values);
	}

private:
	RtInt m_nu;
	RtInt m_uorder;
	RtFloat* m_uknot;
	RtFloat m_umin;
	RtFloat m_umax;
	RtInt m_nv;
	RtInt m_vorder;
	RtFloat* m_vknot;
	RtFloat m_vmin;
	RtFloat m_vmax;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiTrimCurveCache : public RiCacheBase
{
public:
	RiTrimCurveCache(RtInt nloops, RtInt ncurves[], RtInt order[], RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[])
	{
		m_nloops = nloops;
		int __ncurves_length = nloops;
		m_ncurves = new RtInt[__ncurves_length];
		int __ncurves_index;
		for(__ncurves_index = 0; __ncurves_index<__ncurves_length; __ncurves_index++)
		{
			m_ncurves[__ncurves_index] = ncurves[__ncurves_index];
		}
		int __order_length = 0;
		{
			int __i;
			for(__i=0; __i<nloops; __i++)
				__order_length+=ncurves[__i];
		}
		m_order = new RtInt[__order_length];
		int __order_index;
		for(__order_index = 0; __order_index<__order_length; __order_index++)
		{
			m_order[__order_index] = order[__order_index];
		}
		int __knot_length = 0;
		{
			int __i;
			for(__i=0; __i<__order_length; __i++)
				__knot_length+=order[__i]+n[__i];
		}
		m_knot = new RtFloat[__knot_length];
		int __knot_index;
		for(__knot_index = 0; __knot_index<__knot_length; __knot_index++)
		{
			m_knot[__knot_index] = knot[__knot_index];
		}
		int __min_length = __order_length;
		m_min = new RtFloat[__min_length];
		int __min_index;
		for(__min_index = 0; __min_index<__min_length; __min_index++)
		{
			m_min[__min_index] = min[__min_index];
		}
		int __max_length = __order_length;
		m_max = new RtFloat[__max_length];
		int __max_index;
		for(__max_index = 0; __max_index<__max_length; __max_index++)
		{
			m_max[__max_index] = max[__max_index];
		}
		int __n_length = __order_length;
		m_n = new RtInt[__n_length];
		int __n_index;
		for(__n_index = 0; __n_index<__n_length; __n_index++)
		{
			m_n[__n_index] = n[__n_index];
		}
		int __u_length = 0;
		{
			int __i;
			for(__i=0; __i<__order_length; __i++)
				__u_length+=n[__i];
		}
		m_u = new RtFloat[__u_length];
		int __u_index;
		for(__u_index = 0; __u_index<__u_length; __u_index++)
		{
			m_u[__u_index] = u[__u_index];
		}
		int __v_length = __u_length;
		m_v = new RtFloat[__v_length];
		int __v_index;
		for(__v_index = 0; __v_index<__v_length; __v_index++)
		{
			m_v[__v_index] = v[__v_index];
		}
		int __w_length = __u_length;
		m_w = new RtFloat[__w_length];
		int __w_index;
		for(__w_index = 0; __w_index<__w_length; __w_index++)
		{
			m_w[__w_index] = w[__w_index];
		}
	}
	virtual ~RiTrimCurveCache()
	{
		delete[](m_ncurves);
		delete[](m_order);
		delete[](m_knot);
		delete[](m_min);
		delete[](m_max);
		delete[](m_n);
		delete[](m_u);
		delete[](m_v);
		delete[](m_w);
	}
	virtual void ReCall()
	{
		RiTrimCurve(m_nloops, m_ncurves, m_order, m_knot, m_min, m_max, m_n, m_u, m_v, m_w);
	}

private:
	RtInt m_nloops;
	RtInt* m_ncurves;
	RtInt* m_order;
	RtFloat* m_knot;
	RtFloat* m_min;
	RtFloat* m_max;
	RtInt* m_n;
	RtFloat* m_u;
	RtFloat* m_v;
	RtFloat* m_w;
};

class RiSphereVCache : public RiCacheBase
{
public:
	RiSphereVCache(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[])
	{
		m_radius = radius;
		m_zmin = zmin;
		m_zmax = zmax;
		m_thetamax = thetamax;
		// Copy the plist here.
	}
	virtual ~RiSphereVCache()
	{
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiSphereV(m_radius, m_zmin, m_zmax, m_thetamax, m_count, m_tokens, m_values);
	}

private:
	RtFloat m_radius;
	RtFloat m_zmin;
	RtFloat m_zmax;
	RtFloat m_thetamax;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiConeVCache : public RiCacheBase
{
public:
	RiConeVCache(RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[])
	{
		m_height = height;
		m_radius = radius;
		m_thetamax = thetamax;
		// Copy the plist here.
	}
	virtual ~RiConeVCache()
	{
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiConeV(m_height, m_radius, m_thetamax, m_count, m_tokens, m_values);
	}

private:
	RtFloat m_height;
	RtFloat m_radius;
	RtFloat m_thetamax;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiCylinderVCache : public RiCacheBase
{
public:
	RiCylinderVCache(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[])
	{
		m_radius = radius;
		m_zmin = zmin;
		m_zmax = zmax;
		m_thetamax = thetamax;
		// Copy the plist here.
	}
	virtual ~RiCylinderVCache()
	{
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiCylinderV(m_radius, m_zmin, m_zmax, m_thetamax, m_count, m_tokens, m_values);
	}

private:
	RtFloat m_radius;
	RtFloat m_zmin;
	RtFloat m_zmax;
	RtFloat m_thetamax;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiHyperboloidVCache : public RiCacheBase
{
public:
	RiHyperboloidVCache(RtPoint point1, RtPoint point2, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[])
	{
		m_point1[0] = point1[0];
		m_point1[1] = point1[1];
		m_point1[2] = point1[2];
		m_point2[0] = point2[0];
		m_point2[1] = point2[1];
		m_point2[2] = point2[2];
		m_thetamax = thetamax;
		// Copy the plist here.
	}
	virtual ~RiHyperboloidVCache()
	{
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiHyperboloidV(m_point1, m_point2, m_thetamax, m_count, m_tokens, m_values);
	}

private:
	RtPoint m_point1;
	RtPoint m_point2;
	RtFloat m_thetamax;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiParaboloidVCache : public RiCacheBase
{
public:
	RiParaboloidVCache(RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[])
	{
		m_rmax = rmax;
		m_zmin = zmin;
		m_zmax = zmax;
		m_thetamax = thetamax;
		// Copy the plist here.
	}
	virtual ~RiParaboloidVCache()
	{
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiParaboloidV(m_rmax, m_zmin, m_zmax, m_thetamax, m_count, m_tokens, m_values);
	}

private:
	RtFloat m_rmax;
	RtFloat m_zmin;
	RtFloat m_zmax;
	RtFloat m_thetamax;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiDiskVCache : public RiCacheBase
{
public:
	RiDiskVCache(RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[])
	{
		m_height = height;
		m_radius = radius;
		m_thetamax = thetamax;
		// Copy the plist here.
	}
	virtual ~RiDiskVCache()
	{
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiDiskV(m_height, m_radius, m_thetamax, m_count, m_tokens, m_values);
	}

private:
	RtFloat m_height;
	RtFloat m_radius;
	RtFloat m_thetamax;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiTorusVCache : public RiCacheBase
{
public:
	RiTorusVCache(RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[])
	{
		m_majorrad = majorrad;
		m_minorrad = minorrad;
		m_phimin = phimin;
		m_phimax = phimax;
		m_thetamax = thetamax;
		// Copy the plist here.
	}
	virtual ~RiTorusVCache()
	{
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiTorusV(m_majorrad, m_minorrad, m_phimin, m_phimax, m_thetamax, m_count, m_tokens, m_values);
	}

private:
	RtFloat m_majorrad;
	RtFloat m_minorrad;
	RtFloat m_phimin;
	RtFloat m_phimax;
	RtFloat m_thetamax;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiProceduralCache : public RiCacheBase
{
public:
	RiProceduralCache(RtPointer data, RtBound bound, RtProcSubdivFunc refineproc, RtProcFreeFunc freeproc)
	{
		m_data = data;
		m_bound[0] = bound[0];
		m_bound[1] = bound[1];
		m_bound[2] = bound[2];
		m_bound[3] = bound[3];
		m_bound[4] = bound[4];
		m_bound[5] = bound[5];
		m_refineproc = refineproc;
		m_freeproc = freeproc;
	}
	virtual ~RiProceduralCache()
	{
	}
	virtual void ReCall()
	{
		RiProcedural(m_data, m_bound, m_refineproc, m_freeproc);
	}

private:
	RtPointer m_data;
	RtBound m_bound;
	RtProcSubdivFunc m_refineproc;
	RtProcFreeFunc m_freeproc;
};

class RiGeometryVCache : public RiCacheBase
{
public:
	RiGeometryVCache(RtToken type, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __type_length = strlen(type);
		m_type = new char[ __type_length + 1 ];
		strcpy(m_type, type);
		// Copy the plist here.
	}
	virtual ~RiGeometryVCache()
	{
		delete[](m_type);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiGeometryV(m_type, m_count, m_tokens, m_values);
	}

private:
	RtToken m_type;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiSolidBeginCache : public RiCacheBase
{
public:
	RiSolidBeginCache(RtToken type)
	{
		int __type_length = strlen(type);
		m_type = new char[ __type_length + 1 ];
		strcpy(m_type, type);
	}
	virtual ~RiSolidBeginCache()
	{
		delete[](m_type);
	}
	virtual void ReCall()
	{
		RiSolidBegin(m_type);
	}

private:
	RtToken m_type;
};

class RiSolidEndCache : public RiCacheBase
{
public:
	RiSolidEndCache()
	{
	}
	virtual ~RiSolidEndCache()
	{
	}
	virtual void ReCall()
	{
		RiSolidEnd();
	}

private:
};

class RiObjectBeginCache : public RiCacheBase
{
public:
	RiObjectBeginCache()
	{
	}
	virtual ~RiObjectBeginCache()
	{
	}
	virtual void ReCall()
	{
		RiObjectBegin();
	}

private:
};

class RiObjectEndCache : public RiCacheBase
{
public:
	RiObjectEndCache()
	{
	}
	virtual ~RiObjectEndCache()
	{
	}
	virtual void ReCall()
	{
		RiObjectEnd();
	}

private:
};

class RiObjectInstanceCache : public RiCacheBase
{
public:
	RiObjectInstanceCache(RtObjectHandle handle)
	{
		m_handle = handle;
	}
	virtual ~RiObjectInstanceCache()
	{
	}
	virtual void ReCall()
	{
		RiObjectInstance(m_handle);
	}

private:
	RtObjectHandle m_handle;
};

class RiMotionBeginVCache : public RiCacheBase
{
public:
	RiMotionBeginVCache(RtInt N, RtFloat times[])
	{
		m_N = N;
		int __times_length = N;
		m_times = new RtFloat[__times_length];
		int __times_index;
		for(__times_index = 0; __times_index<__times_length; __times_index++)
		{
			m_times[__times_index] = times[__times_index];
		}
	}
	virtual ~RiMotionBeginVCache()
	{
		delete[](m_times);
	}
	virtual void ReCall()
	{
		RiMotionBeginV(m_N, m_times);
	}

private:
	RtInt m_N;
	RtFloat* m_times;
};

class RiMotionEndCache : public RiCacheBase
{
public:
	RiMotionEndCache()
	{
	}
	virtual ~RiMotionEndCache()
	{
	}
	virtual void ReCall()
	{
		RiMotionEnd();
	}

private:
};

class RiMakeTextureVCache : public RiCacheBase
{
public:
	RiMakeTextureVCache(RtString imagefile, RtString texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __imagefile_length = strlen(imagefile);
		m_imagefile = new char[ __imagefile_length + 1 ];
		strcpy(m_imagefile, imagefile);
		int __texturefile_length = strlen(texturefile);
		m_texturefile = new char[ __texturefile_length + 1 ];
		strcpy(m_texturefile, texturefile);
		int __swrap_length = strlen(swrap);
		m_swrap = new char[ __swrap_length + 1 ];
		strcpy(m_swrap, swrap);
		int __twrap_length = strlen(twrap);
		m_twrap = new char[ __twrap_length + 1 ];
		strcpy(m_twrap, twrap);
		m_filterfunc = filterfunc;
		m_swidth = swidth;
		m_twidth = twidth;
		// Copy the plist here.
	}
	virtual ~RiMakeTextureVCache()
	{
		delete[](m_imagefile);
		delete[](m_texturefile);
		delete[](m_swrap);
		delete[](m_twrap);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiMakeTextureV(m_imagefile, m_texturefile, m_swrap, m_twrap, m_filterfunc, m_swidth, m_twidth, m_count, m_tokens, m_values);
	}

private:
	RtString m_imagefile;
	RtString m_texturefile;
	RtToken m_swrap;
	RtToken m_twrap;
	RtFilterFunc m_filterfunc;
	RtFloat m_swidth;
	RtFloat m_twidth;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiMakeBumpVCache : public RiCacheBase
{
public:
	RiMakeBumpVCache(RtString imagefile, RtString bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __imagefile_length = strlen(imagefile);
		m_imagefile = new char[ __imagefile_length + 1 ];
		strcpy(m_imagefile, imagefile);
		int __bumpfile_length = strlen(bumpfile);
		m_bumpfile = new char[ __bumpfile_length + 1 ];
		strcpy(m_bumpfile, bumpfile);
		int __swrap_length = strlen(swrap);
		m_swrap = new char[ __swrap_length + 1 ];
		strcpy(m_swrap, swrap);
		int __twrap_length = strlen(twrap);
		m_twrap = new char[ __twrap_length + 1 ];
		strcpy(m_twrap, twrap);
		m_filterfunc = filterfunc;
		m_swidth = swidth;
		m_twidth = twidth;
		// Copy the plist here.
	}
	virtual ~RiMakeBumpVCache()
	{
		delete[](m_imagefile);
		delete[](m_bumpfile);
		delete[](m_swrap);
		delete[](m_twrap);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiMakeBumpV(m_imagefile, m_bumpfile, m_swrap, m_twrap, m_filterfunc, m_swidth, m_twidth, m_count, m_tokens, m_values);
	}

private:
	RtString m_imagefile;
	RtString m_bumpfile;
	RtToken m_swrap;
	RtToken m_twrap;
	RtFilterFunc m_filterfunc;
	RtFloat m_swidth;
	RtFloat m_twidth;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiMakeLatLongEnvironmentVCache : public RiCacheBase
{
public:
	RiMakeLatLongEnvironmentVCache(RtString imagefile, RtString reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __imagefile_length = strlen(imagefile);
		m_imagefile = new char[ __imagefile_length + 1 ];
		strcpy(m_imagefile, imagefile);
		int __reflfile_length = strlen(reflfile);
		m_reflfile = new char[ __reflfile_length + 1 ];
		strcpy(m_reflfile, reflfile);
		m_filterfunc = filterfunc;
		m_swidth = swidth;
		m_twidth = twidth;
		// Copy the plist here.
	}
	virtual ~RiMakeLatLongEnvironmentVCache()
	{
		delete[](m_imagefile);
		delete[](m_reflfile);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiMakeLatLongEnvironmentV(m_imagefile, m_reflfile, m_filterfunc, m_swidth, m_twidth, m_count, m_tokens, m_values);
	}

private:
	RtString m_imagefile;
	RtString m_reflfile;
	RtFilterFunc m_filterfunc;
	RtFloat m_swidth;
	RtFloat m_twidth;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiMakeCubeFaceEnvironmentVCache : public RiCacheBase
{
public:
	RiMakeCubeFaceEnvironmentVCache(RtString px, RtString nx, RtString py, RtString ny, RtString pz, RtString nz, RtString reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __px_length = strlen(px);
		m_px = new char[ __px_length + 1 ];
		strcpy(m_px, px);
		int __nx_length = strlen(nx);
		m_nx = new char[ __nx_length + 1 ];
		strcpy(m_nx, nx);
		int __py_length = strlen(py);
		m_py = new char[ __py_length + 1 ];
		strcpy(m_py, py);
		int __ny_length = strlen(ny);
		m_ny = new char[ __ny_length + 1 ];
		strcpy(m_ny, ny);
		int __pz_length = strlen(pz);
		m_pz = new char[ __pz_length + 1 ];
		strcpy(m_pz, pz);
		int __nz_length = strlen(nz);
		m_nz = new char[ __nz_length + 1 ];
		strcpy(m_nz, nz);
		int __reflfile_length = strlen(reflfile);
		m_reflfile = new char[ __reflfile_length + 1 ];
		strcpy(m_reflfile, reflfile);
		m_fov = fov;
		m_filterfunc = filterfunc;
		m_swidth = swidth;
		m_twidth = twidth;
		// Copy the plist here.
	}
	virtual ~RiMakeCubeFaceEnvironmentVCache()
	{
		delete[](m_px);
		delete[](m_nx);
		delete[](m_py);
		delete[](m_ny);
		delete[](m_pz);
		delete[](m_nz);
		delete[](m_reflfile);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiMakeCubeFaceEnvironmentV(m_px, m_nx, m_py, m_ny, m_pz, m_nz, m_reflfile, m_fov, m_filterfunc, m_swidth, m_twidth, m_count, m_tokens, m_values);
	}

private:
	RtString m_px;
	RtString m_nx;
	RtString m_py;
	RtString m_ny;
	RtString m_pz;
	RtString m_nz;
	RtString m_reflfile;
	RtFloat m_fov;
	RtFilterFunc m_filterfunc;
	RtFloat m_swidth;
	RtFloat m_twidth;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiMakeShadowVCache : public RiCacheBase
{
public:
	RiMakeShadowVCache(RtString picfile, RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __picfile_length = strlen(picfile);
		m_picfile = new char[ __picfile_length + 1 ];
		strcpy(m_picfile, picfile);
		int __shadowfile_length = strlen(shadowfile);
		m_shadowfile = new char[ __shadowfile_length + 1 ];
		strcpy(m_shadowfile, shadowfile);
		// Copy the plist here.
	}
	virtual ~RiMakeShadowVCache()
	{
		delete[](m_picfile);
		delete[](m_shadowfile);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiMakeShadowV(m_picfile, m_shadowfile, m_count, m_tokens, m_values);
	}

private:
	RtString m_picfile;
	RtString m_shadowfile;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiMakeOcclusionVCache : public RiCacheBase
{
public:
	RiMakeOcclusionVCache(RtInt npics, RtString picfiles[], RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[])
	{
		m_npics = npics;
		int __picfiles_length = npics;
		m_picfiles = new RtString[__picfiles_length];
		int __picfiles_index;
		for(__picfiles_index = 0; __picfiles_index<__picfiles_length; __picfiles_index++)
		{
			int __picfiles_slength = strlen(picfiles[__picfiles_index]);
			m_picfiles[__picfiles_index] = new char[ __picfiles_slength + 1 ];
			strcpy(m_picfiles[__picfiles_index], picfiles[__picfiles_index]);
		}
		int __shadowfile_length = strlen(shadowfile);
		m_shadowfile = new char[ __shadowfile_length + 1 ];
		strcpy(m_shadowfile, shadowfile);
		// Copy the plist here.
	}
	virtual ~RiMakeOcclusionVCache()
	{
		int __picfiles_length = 1;
		int __picfiles_index;
		for(__picfiles_index = 0; __picfiles_index<__picfiles_length; __picfiles_index++)
		{
			delete[](m_picfiles[__picfiles_index]);
		}
		delete[](m_picfiles);
		delete[](m_shadowfile);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiMakeOcclusionV(m_npics, m_picfiles, m_shadowfile, m_count, m_tokens, m_values);
	}

private:
	RtInt m_npics;
	RtString* m_picfiles;
	RtString m_shadowfile;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiErrorHandlerCache : public RiCacheBase
{
public:
	RiErrorHandlerCache(RtErrorFunc handler)
	{
		m_handler = handler;
	}
	virtual ~RiErrorHandlerCache()
	{
	}
	virtual void ReCall()
	{
		RiErrorHandler(m_handler);
	}

private:
	RtErrorFunc m_handler;
};

class RiGetContextCache : public RiCacheBase
{
public:
	RiGetContextCache()
	{
	}
	virtual ~RiGetContextCache()
	{
	}
	virtual void ReCall()
	{
		RiGetContext();
	}

private:
};

class RiContextCache : public RiCacheBase
{
public:
	RiContextCache(RtContextHandle handle)
	{
		m_handle = handle;
	}
	virtual ~RiContextCache()
	{
	}
	virtual void ReCall()
	{
		RiContext(m_handle);
	}

private:
	RtContextHandle m_handle;
};

class RiClippingPlaneCache : public RiCacheBase
{
public:
	RiClippingPlaneCache(RtFloat x, RtFloat y, RtFloat z, RtFloat nx, RtFloat ny, RtFloat nz)
	{
		m_x = x;
		m_y = y;
		m_z = z;
		m_nx = nx;
		m_ny = ny;
		m_nz = nz;
	}
	virtual ~RiClippingPlaneCache()
	{
	}
	virtual void ReCall()
	{
		RiClippingPlane(m_x, m_y, m_z, m_nx, m_ny, m_nz);
	}

private:
	RtFloat m_x;
	RtFloat m_y;
	RtFloat m_z;
	RtFloat m_nx;
	RtFloat m_ny;
	RtFloat m_nz;
};

class RiCoordSysTransformCache : public RiCacheBase
{
public:
	RiCoordSysTransformCache(RtToken space)
	{
		int __space_length = strlen(space);
		m_space = new char[ __space_length + 1 ];
		strcpy(m_space, space);
	}
	virtual ~RiCoordSysTransformCache()
	{
		delete[](m_space);
	}
	virtual void ReCall()
	{
		RiCoordSysTransform(m_space);
	}

private:
	RtToken m_space;
};

class RiBlobbyVCache : public RiCacheBase
{
public:
	RiBlobbyVCache(RtInt nleaf, RtInt ncode, RtInt code[], RtInt nflt, RtFloat flt[], RtInt nstr, RtToken str[], RtInt count, RtToken tokens[], RtPointer values[])
	{
		m_nleaf = nleaf;
		m_ncode = ncode;
		int __code_length = ncode;
		m_code = new RtInt[__code_length];
		int __code_index;
		for(__code_index = 0; __code_index<__code_length; __code_index++)
		{
			m_code[__code_index] = code[__code_index];
		}
		m_nflt = nflt;
		int __flt_length = nflt;
		m_flt = new RtFloat[__flt_length];
		int __flt_index;
		for(__flt_index = 0; __flt_index<__flt_length; __flt_index++)
		{
			m_flt[__flt_index] = flt[__flt_index];
		}
		m_nstr = nstr;
		int __str_length = nstr;
		m_str = new RtToken[__str_length];
		int __str_index;
		for(__str_index = 0; __str_index<__str_length; __str_index++)
		{
			int __str_slength = strlen(str[__str_index]);
			m_str[__str_index] = new char[ __str_slength + 1 ];
			strcpy(m_str[__str_index], str[__str_index]);
		}
		// Copy the plist here.
	}
	virtual ~RiBlobbyVCache()
	{
		delete[](m_code);
		delete[](m_flt);
		int __str_length = 1;
		int __str_index;
		for(__str_index = 0; __str_index<__str_length; __str_index++)
		{
			delete[](m_str[__str_index]);
		}
		delete[](m_str);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiBlobbyV(m_nleaf, m_ncode, m_code, m_nflt, m_flt, m_nstr, m_str, m_count, m_tokens, m_values);
	}

private:
	RtInt m_nleaf;
	RtInt m_ncode;
	RtInt* m_code;
	RtInt m_nflt;
	RtFloat* m_flt;
	RtInt m_nstr;
	RtToken* m_str;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiPointsVCache : public RiCacheBase
{
public:
	RiPointsVCache(RtInt npoints, RtInt count, RtToken tokens[], RtPointer values[])
	{
		m_npoints = npoints;
		// Copy the plist here.
	}
	virtual ~RiPointsVCache()
	{
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiPointsV(m_npoints, m_count, m_tokens, m_values);
	}

private:
	RtInt m_npoints;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiCurvesVCache : public RiCacheBase
{
public:
	RiCurvesVCache(RtToken type, RtInt ncurves, RtInt nvertices[], RtToken wrap, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __type_length = strlen(type);
		m_type = new char[ __type_length + 1 ];
		strcpy(m_type, type);
		m_ncurves = ncurves;
		int __nvertices_length = ncurves;
		m_nvertices = new RtInt[__nvertices_length];
		int __nvertices_index;
		for(__nvertices_index = 0; __nvertices_index<__nvertices_length; __nvertices_index++)
		{
			m_nvertices[__nvertices_index] = nvertices[__nvertices_index];
		}
		int __wrap_length = strlen(wrap);
		m_wrap = new char[ __wrap_length + 1 ];
		strcpy(m_wrap, wrap);
		// Copy the plist here.
	}
	virtual ~RiCurvesVCache()
	{
		delete[](m_type);
		delete[](m_nvertices);
		delete[](m_wrap);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiCurvesV(m_type, m_ncurves, m_nvertices, m_wrap, m_count, m_tokens, m_values);
	}

private:
	RtToken m_type;
	RtInt m_ncurves;
	RtInt* m_nvertices;
	RtToken m_wrap;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiSubdivisionMeshVCache : public RiCacheBase
{
public:
	RiSubdivisionMeshVCache(RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __scheme_length = strlen(scheme);
		m_scheme = new char[ __scheme_length + 1 ];
		strcpy(m_scheme, scheme);
		m_nfaces = nfaces;
		int __nvertices_length = nfaces;
		m_nvertices = new RtInt[__nvertices_length];
		int __nvertices_index;
		for(__nvertices_index = 0; __nvertices_index<__nvertices_length; __nvertices_index++)
		{
			m_nvertices[__nvertices_index] = nvertices[__nvertices_index];
		}
		int __vertices_length = 0;
		{
			int __i;
			for(__i=0; __i<nfaces; __i++)
				__vertices_length+=nvertices[__i];
		}
		m_vertices = new RtInt[__vertices_length];
		int __vertices_index;
		for(__vertices_index = 0; __vertices_index<__vertices_length; __vertices_index++)
		{
			m_vertices[__vertices_index] = vertices[__vertices_index];
		}
		m_ntags = ntags;
		int __tags_length = ntags;		m_tags = new RtToken[__tags_length];
		int __tags_index;
		for(__tags_index = 0; __tags_index<__tags_length; __tags_index++)
		{
			int __tags_slength = strlen(tags[__tags_index]);
			m_tags[__tags_index] = new char[ __tags_slength + 1 ];
			strcpy(m_tags[__tags_index], tags[__tags_index]);
		}
		int __nargs_length = ntags*2;
		m_nargs = new RtInt[__nargs_length];
		int __nargs_index;
		for(__nargs_index = 0; __nargs_index<__nargs_length; __nargs_index++)
		{
			m_nargs[__nargs_index] = nargs[__nargs_index];
		}
		int __intargs_length = 0;
		{
			int __i;
			for(__i=0; __i<ntags*2; __i+=2)
				__intargs_length+=nargs[__i];
		}
		m_intargs = new RtInt[__intargs_length];
		int __intargs_index;
		for(__intargs_index = 0; __intargs_index<__intargs_length; __intargs_index++)
		{
			m_intargs[__intargs_index] = intargs[__intargs_index];
		}
		int __floatargs_length = 0;
		{
			int __i;
			for(__i=0; __i<ntags*2; __i+=2)
				__floatargs_length+=nargs[__i+1];
		}
		m_floatargs = new RtFloat[__floatargs_length];
		int __floatargs_index;
		for(__floatargs_index = 0; __floatargs_index<__floatargs_length; __floatargs_index++)
		{
			m_floatargs[__floatargs_index] = floatargs[__floatargs_index];
		}
		// Copy the plist here.
	}
	virtual ~RiSubdivisionMeshVCache()
	{
		delete[](m_scheme);
		delete[](m_nvertices);
		delete[](m_vertices);
		int __tags_length = 1;
		int __tags_index;
		for(__tags_index = 0; __tags_index<__tags_length; __tags_index++)
		{
			delete[](m_tags[__tags_index]);
		}
		delete[](m_tags);
		delete[](m_nargs);
		delete[](m_intargs);
		delete[](m_floatargs);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiSubdivisionMeshV(m_scheme, m_nfaces, m_nvertices, m_vertices, m_ntags, m_tags, m_nargs, m_intargs, m_floatargs, m_count, m_tokens, m_values);
	}

private:
	RtToken m_scheme;
	RtInt m_nfaces;
	RtInt* m_nvertices;
	RtInt* m_vertices;
	RtInt m_ntags;
	RtToken* m_tags;
	RtInt* m_nargs;
	RtInt* m_intargs;
	RtFloat* m_floatargs;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

class RiReadArchiveVCache : public RiCacheBase
{
public:
	RiReadArchiveVCache(RtToken name, RtArchiveCallback callback, RtInt count, RtToken tokens[], RtPointer values[])
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		m_callback = callback;
		// Copy the plist here.
	}
	virtual ~RiReadArchiveVCache()
	{
		delete[](m_name);
		// Delete the plist here.
	}
	virtual void ReCall()
	{
		RiReadArchiveV(m_name, m_callback, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	RtArchiveCallback m_callback;
	RtInt m_count;
	RtToken* m_tokens;
	RtPointer* m_values;
};

