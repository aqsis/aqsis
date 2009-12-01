struct StorageCount
{
	int constant;
	int uniform;
	int varying;
	int vertex;
	int fvarying;
	int fvertex;

	StorageCount(int constant, int uniform, int varying, int vertex, int fvarying, int fvertex)
		: constant(constant), uniform(uniform), varying(varying),
		vertex(vertex), fvarying(fvarying), fvertex(fvertex),
	{ }

	int operator[](variable_class Class)
	{
		switch(Class)
		{
			case class_constant:    return constant;
			case class_uniform:     return uniform;
			case class_varying:     return varying;
			case class_vertex:      return vertex;
			case class_facevarying: return fvarying;
			case class_facevertex:  return fvertex;
		}
	}
};


class PrimvarList
{
	private:
		StorageCount m_count;
		StorageCount m_numVars;
		// Storage for all primvars
		std::vector<float> m_data;
		// std::vector<int> m_intData; not necessary ???
		std::vector<std::string> m_stringData;

		PrimvarList(const StorageCount& varCount, const StorageCount& numVars)
			: m_count(varCount),
			m_numVars(numVars)
		{ }
	public:
		void add(VarType)
		{
		}
};


PrimvarList* createPrimvarList(const StorageCount& storageSize, int count,
		                       RtToken* tokens, RtPointer* values)
{
	// An array to hold the tokens identifying the primvars
	std::vector<PrimvarToken> parsedTokens;
	parsedTokens.reserve(count);
	std::vector<int> validTokens;
	validTokens.reserve(count);
	int numFloatTokens = 0;
	int numStringTokens = 0;
	int floatSize = 0;
	int stringSize = 0;
	for(int i = 0; i < count; ++i)
	{
		PrimvarToken tok(tokens[i]);
		if(/* primvar is used in a shader or an AOV and primvar is not an int or bool
			  and any std primvar is as expected (Eg, "P" has type "vertex point[1]")
		    */)
		{
			parsedTokens.push_back(tok);
			validTokens.push_back(i);
			int size = tok.storageCount() * storageSize[tok.Class()];
			if(tok.type == type_string)
				stringSize += size;
			else
				floatSize += size;
		}
	}
	// Allocate primvar storage.
	std::vector<float> floatData(floatSize);
	std::vector<std::string> stringData(stringSize);
	// Copy the values into the storage space.
	int floatOffset = 0;
	int stringOffset = 0;
	for(int i = 0; i < parsedTokens.size(); ++i)
	{
		const PrimvarToken& tok = parsedTokens[i];
		int size = tok.storageCount() * storageSize[tok.Class()];
		switch(tok.type)
		{
			case type_string:
				// Copy strings
				for(int j = 0; j < size; ++j)
					stringData[stringOffset + j] = reinterpret_cast<const char*>(values[validTokens[i]])[j];
				stringOffset += size;
				break;
			default:
				// Possible token types here use float storage.
				memcpy(&floatData[floatOffset], values[validTokens[i]], size*sizeof(float));
				floatOffset += size;
				break;
		}
	}
	return new PrimvarList(parsedTokens, floatData, stringData);
}


