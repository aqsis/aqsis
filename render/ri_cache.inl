class RiDeclareCache : public RiCacheBase
{
public:
	RiDeclareCache(RtString name, RtString declaration) : RiCacheBase()
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
	RiFrameBeginCache(RtInt number) : RiCacheBase()
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
	RiFrameEndCache() : RiCacheBase()
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
	RiWorldBeginCache() : RiCacheBase()
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
	RiWorldEndCache() : RiCacheBase()
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
	RiFormatCache(RtInt xresolution, RtInt yresolution, RtFloat pixelaspectratio) : RiCacheBase()
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
	RiFrameAspectRatioCache(RtFloat frameratio) : RiCacheBase()
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
	RiScreenWindowCache(RtFloat left, RtFloat right, RtFloat bottom, RtFloat top) : RiCacheBase()
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
	RiCropWindowCache(RtFloat left, RtFloat right, RtFloat top, RtFloat bottom) : RiCacheBase()
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

class RiProjectionCache : public RiCacheBase
{
public:
	RiProjectionCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiProjectionCache()
	{
		delete[](m_name);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiProjectionV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	// plist information is stored in the base class.
};

class RiClippingCache : public RiCacheBase
{
public:
	RiClippingCache(RtFloat cnear, RtFloat cfar) : RiCacheBase()
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
	RiDepthOfFieldCache(RtFloat fstop, RtFloat focallength, RtFloat focaldistance) : RiCacheBase()
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
	RiShutterCache(RtFloat opentime, RtFloat closetime) : RiCacheBase()
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
	RiPixelVarianceCache(RtFloat variance) : RiCacheBase()
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
	RiPixelSamplesCache(RtFloat xsamples, RtFloat ysamples) : RiCacheBase()
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
	RiPixelFilterCache(RtFilterFunc function, RtFloat xwidth, RtFloat ywidth) : RiCacheBase()
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
	RiExposureCache(RtFloat gain, RtFloat gamma) : RiCacheBase()
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

class RiImagerCache : public RiCacheBase
{
public:
	RiImagerCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiImagerCache()
	{
		delete[](m_name);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiImagerV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	// plist information is stored in the base class.
};

class RiQuantizeCache : public RiCacheBase
{
public:
	RiQuantizeCache(RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ditheramplitude) : RiCacheBase()
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

class RiDisplayCache : public RiCacheBase
{
public:
	RiDisplayCache(RtToken name, RtToken type, RtToken mode, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
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
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiDisplayCache()
	{
		delete[](m_name);
		delete[](m_type);
		delete[](m_mode);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiDisplayV(m_name, m_type, m_mode, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	RtToken m_type;
	RtToken m_mode;
	// plist information is stored in the base class.
};

class RiHiderCache : public RiCacheBase
{
public:
	RiHiderCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiHiderCache()
	{
		delete[](m_name);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiHiderV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	// plist information is stored in the base class.
};

class RiColorSamplesCache : public RiCacheBase
{
public:
	RiColorSamplesCache(RtInt N, RtFloat nRGB[], RtFloat RGBn[]) : RiCacheBase()
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
	RiRelativeDetailCache(RtFloat relativedetail) : RiCacheBase()
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

class RiOptionCache : public RiCacheBase
{
public:
	RiOptionCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiOptionCache()
	{
		delete[](m_name);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiOptionV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	// plist information is stored in the base class.
};

class RiAttributeBeginCache : public RiCacheBase
{
public:
	RiAttributeBeginCache() : RiCacheBase()
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
	RiAttributeEndCache() : RiCacheBase()
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
	RiColorCache(RtColor Cq) : RiCacheBase()
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
	RiOpacityCache(RtColor Os) : RiCacheBase()
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
	RiTextureCoordinatesCache(RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2, RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4) : RiCacheBase()
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

class RiLightSourceCache : public RiCacheBase
{
public:
	RiLightSourceCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiLightSourceCache()
	{
		delete[](m_name);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiLightSourceV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	// plist information is stored in the base class.
};

class RiAreaLightSourceCache : public RiCacheBase
{
public:
	RiAreaLightSourceCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiAreaLightSourceCache()
	{
		delete[](m_name);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiAreaLightSourceV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	// plist information is stored in the base class.
};

class RiIlluminateCache : public RiCacheBase
{
public:
	RiIlluminateCache(RtLightHandle light, RtBoolean onoff) : RiCacheBase()
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

class RiSurfaceCache : public RiCacheBase
{
public:
	RiSurfaceCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiSurfaceCache()
	{
		delete[](m_name);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiSurfaceV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	// plist information is stored in the base class.
};

class RiAtmosphereCache : public RiCacheBase
{
public:
	RiAtmosphereCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiAtmosphereCache()
	{
		delete[](m_name);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiAtmosphereV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	// plist information is stored in the base class.
};

class RiInteriorCache : public RiCacheBase
{
public:
	RiInteriorCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiInteriorCache()
	{
		delete[](m_name);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiInteriorV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	// plist information is stored in the base class.
};

class RiExteriorCache : public RiCacheBase
{
public:
	RiExteriorCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiExteriorCache()
	{
		delete[](m_name);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiExteriorV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	// plist information is stored in the base class.
};

class RiShadingRateCache : public RiCacheBase
{
public:
	RiShadingRateCache(RtFloat size) : RiCacheBase()
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
	RiShadingInterpolationCache(RtToken type) : RiCacheBase()
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
	RiMatteCache(RtBoolean onoff) : RiCacheBase()
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
	RiBoundCache(RtBound bound) : RiCacheBase()
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
	RiDetailCache(RtBound bound) : RiCacheBase()
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
	RiDetailRangeCache(RtFloat offlow, RtFloat onlow, RtFloat onhigh, RtFloat offhigh) : RiCacheBase()
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
	RiGeometricApproximationCache(RtToken type, RtFloat value) : RiCacheBase()
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
	RiOrientationCache(RtToken orientation) : RiCacheBase()
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
	RiReverseOrientationCache() : RiCacheBase()
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
	RiSidesCache(RtInt nsides) : RiCacheBase()
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
	RiIdentityCache() : RiCacheBase()
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
	RiTransformCache(RtMatrix transform) : RiCacheBase()
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
	RiConcatTransformCache(RtMatrix transform) : RiCacheBase()
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
	RiPerspectiveCache(RtFloat fov) : RiCacheBase()
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
	RiTranslateCache(RtFloat dx, RtFloat dy, RtFloat dz) : RiCacheBase()
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
	RiRotateCache(RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz) : RiCacheBase()
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
	RiScaleCache(RtFloat sx, RtFloat sy, RtFloat sz) : RiCacheBase()
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
	RiSkewCache(RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1, RtFloat dx2, RtFloat dy2, RtFloat dz2) : RiCacheBase()
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

class RiDeformationCache : public RiCacheBase
{
public:
	RiDeformationCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiDeformationCache()
	{
		delete[](m_name);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiDeformationV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	// plist information is stored in the base class.
};

class RiDisplacementCache : public RiCacheBase
{
public:
	RiDisplacementCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiDisplacementCache()
	{
		delete[](m_name);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiDisplacementV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	// plist information is stored in the base class.
};

class RiCoordinateSystemCache : public RiCacheBase
{
public:
	RiCoordinateSystemCache(RtToken space) : RiCacheBase()
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
	RiTransformPointsCache(RtToken fromspace, RtToken tospace, RtInt npoints, RtPoint points[]) : RiCacheBase()
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
	RiTransformBeginCache() : RiCacheBase()
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
	RiTransformEndCache() : RiCacheBase()
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

class RiAttributeCache : public RiCacheBase
{
public:
	RiAttributeCache(RtToken name, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiAttributeCache()
	{
		delete[](m_name);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiAttributeV(m_name, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	// plist information is stored in the base class.
};

class RiPolygonCache : public RiCacheBase
{
public:
	RiPolygonCache(RtInt nvertices, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		m_nvertices = nvertices;
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		varying_size = nvertices;
		vertex_size = nvertices;
		facevarying_size = nvertices;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiPolygonCache()
	{
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiPolygonV(m_nvertices, m_count, m_tokens, m_values);
	}

private:
	RtInt m_nvertices;
	// plist information is stored in the base class.
};

class RiGeneralPolygonCache : public RiCacheBase
{
public:
	RiGeneralPolygonCache(RtInt nloops, RtInt nverts[], RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
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
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		varying_size = 0;
		{
			int __i;
			for(__i=0;__i<nloops;__i++)
				varying_size+=nverts[__i];
		}
		vertex_size = varying_size;
		facevarying_size = varying_size;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiGeneralPolygonCache()
	{
		delete[](m_nverts);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiGeneralPolygonV(m_nloops, m_nverts, m_count, m_tokens, m_values);
	}

private:
	RtInt m_nloops;
	RtInt* m_nverts;
	// plist information is stored in the base class.
};

class RiPointsPolygonsCache : public RiCacheBase
{
public:
	RiPointsPolygonsCache(RtInt npolys, RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
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
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		varying_size=0;
		{
			int __i;
			for(__i=0; __i<__verts_length; __i++)
				if(verts[__i]>varying_size)
					varying_size=verts[__i];
		}
		varying_size+=1;
		vertex_size=varying_size;
		facevarying_size=0;
		{
			int __i;
			for(__i=0; __i<npolys; __i++)
				facevarying_size+=nverts[__i];
		}
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiPointsPolygonsCache()
	{
		delete[](m_nverts);
		delete[](m_verts);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiPointsPolygonsV(m_npolys, m_nverts, m_verts, m_count, m_tokens, m_values);
	}

private:
	RtInt m_npolys;
	RtInt* m_nverts;
	RtInt* m_verts;
	// plist information is stored in the base class.
};

class RiPointsGeneralPolygonsCache : public RiCacheBase
{
public:
	RiPointsGeneralPolygonsCache(RtInt npolys, RtInt nloops[], RtInt nverts[], RtInt verts[], RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
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
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		varying_size=0;
		{
			int __i;
			for(__i=0; __i<__verts_length; __i++)
				if(verts[__i]>varying_size)
					varying_size=verts[__i];
		}
		varying_size+=1;
		vertex_size=varying_size;
		facevarying_size=0;
		{
			int __i;
			for(__i=0; __i<npolys; __i++)
				facevarying_size+=nverts[__i];
		}
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiPointsGeneralPolygonsCache()
	{
		delete[](m_nloops);
		delete[](m_nverts);
		delete[](m_verts);
		// plist gets destroyed by the base class.
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
	// plist information is stored in the base class.
};

class RiBasisCache : public RiCacheBase
{
public:
	RiBasisCache(RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep) : RiCacheBase()
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

class RiPatchCache : public RiCacheBase
{
public:
	RiPatchCache(RtToken type, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		int __type_length = strlen(type);
		m_type = new char[ __type_length + 1 ];
		strcpy(m_type, type);
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		varying_size = 4;
		vertex_size = 4;
		if(strcmp(type, "bicubic")==0)
			vertex_size=16;
		facevarying_size = varying_size;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiPatchCache()
	{
		delete[](m_type);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiPatchV(m_type, m_count, m_tokens, m_values);
	}

private:
	RtToken m_type;
	// plist information is stored in the base class.
};

class RiPatchMeshCache : public RiCacheBase
{
public:
	RiPatchMeshCache(RtToken type, RtInt nu, RtToken uwrap, RtInt nv, RtToken vwrap, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
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
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;

		if(strcmp(type, "bilinear")==0)
		{
			if(strcmp(uwrap, "periodic")==0)
				uniform_size = nu;
			else
				uniform_size = nu-1;
			if(strcmp(vwrap, "periodic")==0)
				uniform_size *= nv;
			else
				uniform_size *= nv-1;
		}
		else
		{
			int ustep = QGetRenderContext() ->pattrCurrent()->GetIntegerAttribute( "System", "BasisStep" ) [ 0 ];
			int vstep = QGetRenderContext() ->pattrCurrent()->GetIntegerAttribute( "System", "BasisStep" ) [ 1 ];
			if(strcmp(uwrap, "periodic")==0)
				uniform_size = nu/ustep;
			else
				uniform_size = (nu-4)/ustep;
			if(strcmp(vwrap, "periodic")==0)
				uniform_size *= nv/vstep;
			else
				uniform_size *= (nv-4)/vstep;
		}

		if(strcmp(type, "bilinear")==0)
			varying_size = nu*nv;
		else
		{
			int ustep = QGetRenderContext() ->pattrCurrent()->GetIntegerAttribute( "System", "BasisStep" ) [ 0 ];
			int vstep = QGetRenderContext() ->pattrCurrent()->GetIntegerAttribute( "System", "BasisStep" ) [ 1 ];
			varying_size = (nu/ustep)*(nv/vstep);
		}
		vertex_size=nu*nv;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiPatchMeshCache()
	{
		delete[](m_type);
		delete[](m_uwrap);
		delete[](m_vwrap);
		// plist gets destroyed by the base class.
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
	// plist information is stored in the base class.
};

class RiNuPatchCache : public RiCacheBase
{
public:
	RiNuPatchCache(RtInt nu, RtInt uorder, RtFloat uknot[], RtFloat umin, RtFloat umax, RtInt nv, RtInt vorder, RtFloat vknot[], RtFloat vmin, RtFloat vmax, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
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
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		uniform_size=(1+nu-uorder+1)*(1+nv-vorder+1);
		uniform_size=(1+nu-uorder+1)*(1+nv-vorder+1);
		uniform_size=nu*nv;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiNuPatchCache()
	{
		delete[](m_uknot);
		delete[](m_vknot);
		// plist gets destroyed by the base class.
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
	// plist information is stored in the base class.
};

class RiTrimCurveCache : public RiCacheBase
{
public:
	RiTrimCurveCache(RtInt nloops, RtInt ncurves[], RtInt order[], RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[]) : RiCacheBase()
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

class RiSphereCache : public RiCacheBase
{
public:
	RiSphereCache(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		m_radius = radius;
		m_zmin = zmin;
		m_zmax = zmax;
		m_thetamax = thetamax;
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		varying_size = 4;
		vertex_size = 4;
		facevarying_size = 4;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiSphereCache()
	{
		// plist gets destroyed by the base class.
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
	// plist information is stored in the base class.
};

class RiConeCache : public RiCacheBase
{
public:
	RiConeCache(RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		m_height = height;
		m_radius = radius;
		m_thetamax = thetamax;
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		varying_size = 4;
		vertex_size = 4;
		facevarying_size = 4;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiConeCache()
	{
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiConeV(m_height, m_radius, m_thetamax, m_count, m_tokens, m_values);
	}

private:
	RtFloat m_height;
	RtFloat m_radius;
	RtFloat m_thetamax;
	// plist information is stored in the base class.
};

class RiCylinderCache : public RiCacheBase
{
public:
	RiCylinderCache(RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		m_radius = radius;
		m_zmin = zmin;
		m_zmax = zmax;
		m_thetamax = thetamax;
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		varying_size = 4;
		vertex_size = 4;
		facevarying_size = 4;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiCylinderCache()
	{
		// plist gets destroyed by the base class.
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
	// plist information is stored in the base class.
};

class RiHyperboloidCache : public RiCacheBase
{
public:
	RiHyperboloidCache(RtPoint point1, RtPoint point2, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		m_point1[0] = point1[0];
		m_point1[1] = point1[1];
		m_point1[2] = point1[2];
		m_point2[0] = point2[0];
		m_point2[1] = point2[1];
		m_point2[2] = point2[2];
		m_thetamax = thetamax;
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		varying_size = 4;
		vertex_size = 4;
		facevarying_size = 4;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiHyperboloidCache()
	{
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiHyperboloidV(m_point1, m_point2, m_thetamax, m_count, m_tokens, m_values);
	}

private:
	RtPoint m_point1;
	RtPoint m_point2;
	RtFloat m_thetamax;
	// plist information is stored in the base class.
};

class RiParaboloidCache : public RiCacheBase
{
public:
	RiParaboloidCache(RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		m_rmax = rmax;
		m_zmin = zmin;
		m_zmax = zmax;
		m_thetamax = thetamax;
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		varying_size = 4;
		vertex_size = 4;
		facevarying_size = 4;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiParaboloidCache()
	{
		// plist gets destroyed by the base class.
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
	// plist information is stored in the base class.
};

class RiDiskCache : public RiCacheBase
{
public:
	RiDiskCache(RtFloat height, RtFloat radius, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		m_height = height;
		m_radius = radius;
		m_thetamax = thetamax;
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		varying_size = 4;
		vertex_size = 4;
		facevarying_size = 4;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiDiskCache()
	{
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiDiskV(m_height, m_radius, m_thetamax, m_count, m_tokens, m_values);
	}

private:
	RtFloat m_height;
	RtFloat m_radius;
	RtFloat m_thetamax;
	// plist information is stored in the base class.
};

class RiTorusCache : public RiCacheBase
{
public:
	RiTorusCache(RtFloat majorrad, RtFloat minorrad, RtFloat phimin, RtFloat phimax, RtFloat thetamax, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		m_majorrad = majorrad;
		m_minorrad = minorrad;
		m_phimin = phimin;
		m_phimax = phimax;
		m_thetamax = thetamax;
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		varying_size = 4;
		vertex_size = 4;
		facevarying_size = 4;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiTorusCache()
	{
		// plist gets destroyed by the base class.
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
	// plist information is stored in the base class.
};

class RiProceduralCache : public RiCacheBase
{
public:
	RiProceduralCache(RtPointer data, RtBound bound, RtProcSubdivFunc refineproc, RtProcFreeFunc freeproc) : RiCacheBase()
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

class RiGeometryCache : public RiCacheBase
{
public:
	RiGeometryCache(RtToken type, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		int __type_length = strlen(type);
		m_type = new char[ __type_length + 1 ];
		strcpy(m_type, type);
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiGeometryCache()
	{
		delete[](m_type);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiGeometryV(m_type, m_count, m_tokens, m_values);
	}

private:
	RtToken m_type;
	// plist information is stored in the base class.
};

class RiSolidBeginCache : public RiCacheBase
{
public:
	RiSolidBeginCache(RtToken type) : RiCacheBase()
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
	RiSolidEndCache() : RiCacheBase()
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
	RiObjectBeginCache() : RiCacheBase()
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
	RiObjectEndCache() : RiCacheBase()
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
	RiObjectInstanceCache(RtObjectHandle handle) : RiCacheBase()
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
	RiMotionBeginVCache(RtInt N, RtFloat times[]) : RiCacheBase()
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
	RiMotionEndCache() : RiCacheBase()
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

class RiMakeTextureCache : public RiCacheBase
{
public:
	RiMakeTextureCache(RtString imagefile, RtString texturefile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
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
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiMakeTextureCache()
	{
		delete[](m_imagefile);
		delete[](m_texturefile);
		delete[](m_swrap);
		delete[](m_twrap);
		// plist gets destroyed by the base class.
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
	// plist information is stored in the base class.
};

class RiMakeBumpCache : public RiCacheBase
{
public:
	RiMakeBumpCache(RtString imagefile, RtString bumpfile, RtToken swrap, RtToken twrap, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
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
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiMakeBumpCache()
	{
		delete[](m_imagefile);
		delete[](m_bumpfile);
		delete[](m_swrap);
		delete[](m_twrap);
		// plist gets destroyed by the base class.
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
	// plist information is stored in the base class.
};

class RiMakeLatLongEnvironmentCache : public RiCacheBase
{
public:
	RiMakeLatLongEnvironmentCache(RtString imagefile, RtString reflfile, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
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
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiMakeLatLongEnvironmentCache()
	{
		delete[](m_imagefile);
		delete[](m_reflfile);
		// plist gets destroyed by the base class.
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
	// plist information is stored in the base class.
};

class RiMakeCubeFaceEnvironmentCache : public RiCacheBase
{
public:
	RiMakeCubeFaceEnvironmentCache(RtString px, RtString nx, RtString py, RtString ny, RtString pz, RtString nz, RtString reflfile, RtFloat fov, RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
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
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiMakeCubeFaceEnvironmentCache()
	{
		delete[](m_px);
		delete[](m_nx);
		delete[](m_py);
		delete[](m_ny);
		delete[](m_pz);
		delete[](m_nz);
		delete[](m_reflfile);
		// plist gets destroyed by the base class.
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
	// plist information is stored in the base class.
};

class RiMakeShadowCache : public RiCacheBase
{
public:
	RiMakeShadowCache(RtString picfile, RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		int __picfile_length = strlen(picfile);
		m_picfile = new char[ __picfile_length + 1 ];
		strcpy(m_picfile, picfile);
		int __shadowfile_length = strlen(shadowfile);
		m_shadowfile = new char[ __shadowfile_length + 1 ];
		strcpy(m_shadowfile, shadowfile);
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiMakeShadowCache()
	{
		delete[](m_picfile);
		delete[](m_shadowfile);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiMakeShadowV(m_picfile, m_shadowfile, m_count, m_tokens, m_values);
	}

private:
	RtString m_picfile;
	RtString m_shadowfile;
	// plist information is stored in the base class.
};

class RiMakeOcclusionCache : public RiCacheBase
{
public:
	RiMakeOcclusionCache(RtInt npics, RtString picfiles[], RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
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
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiMakeOcclusionCache()
	{
		int __picfiles_length = 1;
		int __picfiles_index;
		for(__picfiles_index = 0; __picfiles_index<__picfiles_length; __picfiles_index++)
		{
			delete[](m_picfiles[__picfiles_index]);
		}
		delete[](m_picfiles);
		delete[](m_shadowfile);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiMakeOcclusionV(m_npics, m_picfiles, m_shadowfile, m_count, m_tokens, m_values);
	}

private:
	RtInt m_npics;
	RtString* m_picfiles;
	RtString m_shadowfile;
	// plist information is stored in the base class.
};

class RiErrorHandlerCache : public RiCacheBase
{
public:
	RiErrorHandlerCache(RtErrorFunc handler) : RiCacheBase()
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

class RiClippingPlaneCache : public RiCacheBase
{
public:
	RiClippingPlaneCache(RtFloat x, RtFloat y, RtFloat z, RtFloat nx, RtFloat ny, RtFloat nz) : RiCacheBase()
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
	RiCoordSysTransformCache(RtToken space) : RiCacheBase()
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

class RiBlobbyCache : public RiCacheBase
{
public:
	RiBlobbyCache(RtInt nleaf, RtInt ncode, RtInt code[], RtInt nflt, RtFloat flt[], RtInt nstr, RtToken str[], RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
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
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiBlobbyCache()
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
		// plist gets destroyed by the base class.
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
	// plist information is stored in the base class.
};

class RiPointsCache : public RiCacheBase
{
public:
	RiPointsCache(RtInt npoints, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		m_npoints = npoints;
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		varying_size = npoints;
		vertex_size = npoints;
		facevarying_size = npoints;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiPointsCache()
	{
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiPointsV(m_npoints, m_count, m_tokens, m_values);
	}

private:
	RtInt m_npoints;
	// plist information is stored in the base class.
};

class RiCurvesCache : public RiCacheBase
{
public:
	RiCurvesCache(RtToken type, RtInt ncurves, RtInt nvertices[], RtToken wrap, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
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
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		uniform_size = ncurves;
		varying_size = 0;
		{
			int __i;
			for(__i=0; __i<__nvertices_length; __i++)
			{
				if(strcmp(type, "cubic")==0)
				{
					int step = QGetRenderContext() ->pattrCurrent()->GetIntegerAttribute( "System", "BasisStep" ) [ 0 ];
					if(strcmp(wrap, "periodic")==0)
						varying_size+=nvertices[__i]/step;
					else
						varying_size+=((nvertices[__i]-4)/step)+1;
				}
				else
				{
					if(strcmp(wrap, "periodic")==0)
						varying_size+=nvertices[__i];
					else
						varying_size+=nvertices[__i]-1;
				}
				varying_size+=1;
			}
		}
		vertex_size = 0;
		{
			int __i;
			for(__i=0; __i<ncurves; __i++)
				vertex_size+=nvertices[__i];
		}
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiCurvesCache()
	{
		delete[](m_type);
		delete[](m_nvertices);
		delete[](m_wrap);
		// plist gets destroyed by the base class.
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
	// plist information is stored in the base class.
};

class RiSubdivisionMeshCache : public RiCacheBase
{
public:
	RiSubdivisionMeshCache(RtToken scheme, RtInt nfaces, RtInt nvertices[], RtInt vertices[], RtInt ntags, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[], RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
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
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		varying_size=0;
		{
			int __i;
			for(__i=0; __i<__vertices_length; __i++)
				if(vertices[__i]>varying_size)
					varying_size=vertices[__i];
		}
		varying_size+=1;
		vertex_size=varying_size;
		facevarying_size=0;
		{
			int __i;
			for(__i=0; __i<nfaces; __i++)
				facevarying_size+=nvertices[__i];
		}
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiSubdivisionMeshCache()
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
		// plist gets destroyed by the base class.
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
	// plist information is stored in the base class.
};

class RiReadArchiveCache : public RiCacheBase
{
public:
	RiReadArchiveCache(RtToken name, RtArchiveCallback callback, RtInt count, RtToken tokens[], RtPointer values[]) : RiCacheBase()
	{
		int __name_length = strlen(name);
		m_name = new char[ __name_length + 1 ];
		strcpy(m_name, name);
		m_callback = callback;
		// Copy the plist here.
		int constant_size = 1;
		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);
	}
	virtual ~RiReadArchiveCache()
	{
		delete[](m_name);
		// plist gets destroyed by the base class.
	}
	virtual void ReCall()
	{
		RiReadArchiveV(m_name, m_callback, m_count, m_tokens, m_values);
	}

private:
	RtToken m_name;
	RtArchiveCallback m_callback;
	// plist information is stored in the base class.
};

#define Cache_RiDeclare\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiDeclareCache(name, declaration) ); \
		return(0);	\
	}
	#define Cache_RiFrameBegin\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiFrameBeginCache(number) ); \
		return;	\
	}
	#define Cache_RiFrameEnd\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiFrameEndCache() ); \
		return;	\
	}
	#define Cache_RiWorldBegin\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiWorldBeginCache() ); \
		return;	\
	}
	#define Cache_RiWorldEnd\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiWorldEndCache() ); \
		return;	\
	}
	#define Cache_RiFormat\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiFormatCache(xresolution, yresolution, pixelaspectratio) ); \
		return;	\
	}
	#define Cache_RiFrameAspectRatio\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiFrameAspectRatioCache(frameratio) ); \
		return;	\
	}
	#define Cache_RiScreenWindow\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiScreenWindowCache(left, right, bottom, top) ); \
		return;	\
	}
	#define Cache_RiCropWindow\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiCropWindowCache(left, right, top, bottom) ); \
		return;	\
	}
	#define Cache_RiProjection\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiProjectionCache(name, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiClipping\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiClippingCache(cnear, cfar) ); \
		return;	\
	}
	#define Cache_RiDepthOfField\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiDepthOfFieldCache(fstop, focallength, focaldistance) ); \
		return;	\
	}
	#define Cache_RiShutter\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiShutterCache(opentime, closetime) ); \
		return;	\
	}
	#define Cache_RiPixelVariance\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiPixelVarianceCache(variance) ); \
		return;	\
	}
	#define Cache_RiPixelSamples\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiPixelSamplesCache(xsamples, ysamples) ); \
		return;	\
	}
	#define Cache_RiPixelFilter\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiPixelFilterCache(function, xwidth, ywidth) ); \
		return;	\
	}
	#define Cache_RiExposure\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiExposureCache(gain, gamma) ); \
		return;	\
	}
	#define Cache_RiImager\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiImagerCache(name, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiQuantize\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiQuantizeCache(type, one, min, max, ditheramplitude) ); \
		return;	\
	}
	#define Cache_RiDisplay\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiDisplayCache(name, type, mode, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiHider\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiHiderCache(name, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiColorSamples\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiColorSamplesCache(N, nRGB, RGBn) ); \
		return;	\
	}
	#define Cache_RiRelativeDetail\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiRelativeDetailCache(relativedetail) ); \
		return;	\
	}
	#define Cache_RiOption\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiOptionCache(name, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiAttributeBegin\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiAttributeBeginCache() ); \
		return;	\
	}
	#define Cache_RiAttributeEnd\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiAttributeEndCache() ); \
		return;	\
	}
	#define Cache_RiColor\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiColorCache(Cq) ); \
		return;	\
	}
	#define Cache_RiOpacity\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiOpacityCache(Os) ); \
		return;	\
	}
	#define Cache_RiTextureCoordinates\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiTextureCoordinatesCache(s1, t1, s2, t2, s3, t3, s4, t4) ); \
		return;	\
	}
	#define Cache_RiLightSource\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiLightSourceCache(name, count, tokens, values) ); \
		return(0);	\
	}
	#define Cache_RiAreaLightSource\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiAreaLightSourceCache(name, count, tokens, values) ); \
		return(0);	\
	}
	#define Cache_RiIlluminate\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiIlluminateCache(light, onoff) ); \
		return;	\
	}
	#define Cache_RiSurface\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiSurfaceCache(name, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiAtmosphere\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiAtmosphereCache(name, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiInterior\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiInteriorCache(name, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiExterior\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiExteriorCache(name, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiShadingRate\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiShadingRateCache(size) ); \
		return;	\
	}
	#define Cache_RiShadingInterpolation\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiShadingInterpolationCache(type) ); \
		return;	\
	}
	#define Cache_RiMatte\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiMatteCache(onoff) ); \
		return;	\
	}
	#define Cache_RiBound\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiBoundCache(bound) ); \
		return;	\
	}
	#define Cache_RiDetail\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiDetailCache(bound) ); \
		return;	\
	}
	#define Cache_RiDetailRange\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiDetailRangeCache(offlow, onlow, onhigh, offhigh) ); \
		return;	\
	}
	#define Cache_RiGeometricApproximation\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiGeometricApproximationCache(type, value) ); \
		return;	\
	}
	#define Cache_RiOrientation\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiOrientationCache(orientation) ); \
		return;	\
	}
	#define Cache_RiReverseOrientation\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiReverseOrientationCache() ); \
		return;	\
	}
	#define Cache_RiSides\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiSidesCache(nsides) ); \
		return;	\
	}
	#define Cache_RiIdentity\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiIdentityCache() ); \
		return;	\
	}
	#define Cache_RiTransform\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiTransformCache(transform) ); \
		return;	\
	}
	#define Cache_RiConcatTransform\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiConcatTransformCache(transform) ); \
		return;	\
	}
	#define Cache_RiPerspective\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiPerspectiveCache(fov) ); \
		return;	\
	}
	#define Cache_RiTranslate\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiTranslateCache(dx, dy, dz) ); \
		return;	\
	}
	#define Cache_RiRotate\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiRotateCache(angle, dx, dy, dz) ); \
		return;	\
	}
	#define Cache_RiScale\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiScaleCache(sx, sy, sz) ); \
		return;	\
	}
	#define Cache_RiSkew\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiSkewCache(angle, dx1, dy1, dz1, dx2, dy2, dz2) ); \
		return;	\
	}
	#define Cache_RiDeformation\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiDeformationCache(name, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiDisplacement\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiDisplacementCache(name, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiCoordinateSystem\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiCoordinateSystemCache(space) ); \
		return;	\
	}
	#define Cache_RiTransformPoints\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiTransformPointsCache(fromspace, tospace, npoints, points) ); \
		return(0);	\
	}
	#define Cache_RiTransformBegin\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiTransformBeginCache() ); \
		return;	\
	}
	#define Cache_RiTransformEnd\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiTransformEndCache() ); \
		return;	\
	}
	#define Cache_RiAttribute\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiAttributeCache(name, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiPolygon\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiPolygonCache(nvertices, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiGeneralPolygon\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiGeneralPolygonCache(nloops, nverts, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiPointsPolygons\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiPointsPolygonsCache(npolys, nverts, verts, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiPointsGeneralPolygons\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiPointsGeneralPolygonsCache(npolys, nloops, nverts, verts, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiBasis\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiBasisCache(ubasis, ustep, vbasis, vstep) ); \
		return;	\
	}
	#define Cache_RiPatch\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiPatchCache(type, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiPatchMesh\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiPatchMeshCache(type, nu, uwrap, nv, vwrap, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiNuPatch\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiNuPatchCache(nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiTrimCurve\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiTrimCurveCache(nloops, ncurves, order, knot, min, max, n, u, v, w) ); \
		return;	\
	}
	#define Cache_RiSphere\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiSphereCache(radius, zmin, zmax, thetamax, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiCone\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiConeCache(height, radius, thetamax, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiCylinder\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiCylinderCache(radius, zmin, zmax, thetamax, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiHyperboloid\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiHyperboloidCache(point1, point2, thetamax, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiParaboloid\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiParaboloidCache(rmax, zmin, zmax, thetamax, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiDisk\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiDiskCache(height, radius, thetamax, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiTorus\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiTorusCache(majorrad, minorrad, phimin, phimax, thetamax, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiProcedural\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiProceduralCache(data, bound, refineproc, freeproc) ); \
		return;	\
	}
	#define Cache_RiGeometry\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiGeometryCache(type, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiSolidBegin\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiSolidBeginCache(type) ); \
		return;	\
	}
	#define Cache_RiSolidEnd\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiSolidEndCache() ); \
		return;	\
	}
	#define Cache_RiObjectBegin\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiObjectBeginCache() ); \
		return(0);	\
	}
	#define Cache_RiObjectEnd\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiObjectEndCache() ); \
		return;	\
	}
	#define Cache_RiObjectInstance\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiObjectInstanceCache(handle) ); \
		return;	\
	}
	#define Cache_RiMotionBeginV\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiMotionBeginVCache(N, times) ); \
		return;	\
	}
	#define Cache_RiMotionEnd\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiMotionEndCache() ); \
		return;	\
	}
	#define Cache_RiMakeTexture\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiMakeTextureCache(imagefile, texturefile, swrap, twrap, filterfunc, swidth, twidth, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiMakeBump\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiMakeBumpCache(imagefile, bumpfile, swrap, twrap, filterfunc, swidth, twidth, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiMakeLatLongEnvironment\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiMakeLatLongEnvironmentCache(imagefile, reflfile, filterfunc, swidth, twidth, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiMakeCubeFaceEnvironment\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiMakeCubeFaceEnvironmentCache(px, nx, py, ny, pz, nz, reflfile, fov, filterfunc, swidth, twidth, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiMakeShadow\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiMakeShadowCache(picfile, shadowfile, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiMakeOcclusion\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiMakeOcclusionCache(npics, picfiles, shadowfile, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiErrorHandler\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiErrorHandlerCache(handler) ); \
		return;	\
	}
	#define Cache_RiClippingPlane\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiClippingPlaneCache(x, y, z, nx, ny, nz) ); \
		return;	\
	}
	#define Cache_RiCoordSysTransform\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiCoordSysTransformCache(space) ); \
		return;	\
	}
	#define Cache_RiBlobby\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiBlobbyCache(nleaf, ncode, code, nflt, flt, nstr, str, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiPoints\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiPointsCache(npoints, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiCurves\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiCurvesCache(type, ncurves, nvertices, wrap, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiSubdivisionMesh\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiSubdivisionMeshCache(scheme, nfaces, nvertices, vertices, ntags, tags, nargs, intargs, floatargs, count, tokens, values) ); \
		return;	\
	}
	#define Cache_RiReadArchive\
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new RiReadArchiveCache(name, callback, count, tokens, values) ); \
		return;	\
	}
	