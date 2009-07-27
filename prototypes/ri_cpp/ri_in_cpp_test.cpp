#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

#include <boost/assign/std/vector.hpp>

#include <boost/shared_ptr.hpp>

/**
 * Tests of ways to rethink RI as an object oriented interface in C++.
 *
 * The traditional C RI is excellent, but showing its age.  If nothing else,
 * aqsis should consider using something else internally as a connection
 * between RIB and the renderer.
 *
 * Most of what is here investigates ways to pass parameter lists in a typesafe
 * manner, so it's relevant to a possible C++ wrapper for the RI as well as
 * some internal interface.
 *
 *
 * Musings on internal interfaces:
 * -------------------------------
 * After writing this code and thinking some more, I conclude that a truly
 * flexible interface would involve much more than simple fixups involving type
 * safety for parameter lists.
 *
 * Instead, the interface would expose the class interface for geometry types.
 * Primvars can then be attached directly to the geometry, possibly through a
 * typesafe parameter list at construction time.
 *
 * Further, exposing the geometry to the outside world would make it trivial to
 * create procedural geometry which integrates with the renderer pipeline just
 * as tightly as the builtin geometry classes.
 *
 * Exposing such an interface wouldn't be without cost - it suddenly requires
 * internal interfaces to be more stable, potentially constraining the
 * implementation...  so we shouldn't expose it without careful thought.
 */

enum VariableClass
{
    class_invalid,
    class_constant,
    class_uniform,
    class_varying,
    class_vertex,
    class_facevarying,
    class_facevertex
};

enum VariableType
{
    type_invalid,
    type_float,
    type_integer,
    type_point,
    type_string,
    type_color,
    type_triple,
    type_hpoint,
    type_normal,
    type_vector,
    type_void,
    type_matrix,
    type_sixteentuple,
    type_bool,
	type_lookup
};


// Represent a renderman token.
struct Token
{
	VariableClass Class;
	VariableType type;
	int arraySize;
	std::string name;

	Token(VariableClass Class, VariableType type, int arraySize,
			const std::string& name)
		: Class(Class),
		type(type),
		arraySize(arraySize),
		name(name)
	{ }
};


//------------------------------------------------------------------------------
// Classes for managing conversion between user-defined parameters types and
// the underlying types passed to the implementation.

// Manager interface for data allocated during the conversion process.
class ParamManager
{
	public: virtual ~ParamManager() {};
};

// Meta function which determines the underlying type constant (one of
// type_float, type_integer, or type_string) associated with a user-defined
// parameter type.
template<typename T>
struct defaultParamType
{
	static const VariableType value = type_invalid;
};

// Macro to declare that a user-defined type can be converted to a renderman
// parameter value.
//
// typeName - Name of the user-defined type
// defaultType - Type constant specifying which underlying type typeName will
//               be converted to.
#define DECLARE_CONVERT_TO_PARAM_CAPABLE(typeName, defaultType)           \
template<typename T>                                                      \
void convertToParam(const typeName& value, T& valueOut,                   \
		            int& lengthOut, boost::shared_ptr<ParamManager>&)     \
{                                                                         \
	throw std::runtime_error("Cannot convert to requested Param type");   \
}                                                                         \
template<> struct defaultParamType<typeName>                              \
{                                                                         \
	static const VariableType value = defaultType;                        \
};


// Declare some parameter value conversions for std::vector<T>
DECLARE_CONVERT_TO_PARAM_CAPABLE(std::vector<float>, type_float)
void convertToParam(const std::vector<float>& value, const float*& valueOut, 
		            int& lengthOut, boost::shared_ptr<ParamManager>&)
{
	lengthOut = value.size();
	valueOut = (lengthOut == 0) ? 0 : &value[0];
}

DECLARE_CONVERT_TO_PARAM_CAPABLE(std::vector<int>, type_integer)
void convertToParam(const std::vector<int>& value, const int*& valueOut, 
		            int& lengthOut, boost::shared_ptr<ParamManager>&)
{
	lengthOut = value.size();
	valueOut = (lengthOut == 0) ? 0 : &value[0];
}

typedef const char* cstr_type;
DECLARE_CONVERT_TO_PARAM_CAPABLE(std::vector<cstr_type>, type_string)
void convertToParam(const std::vector<cstr_type>& value, const cstr_type*& valueOut, 
		            int& lengthOut, boost::shared_ptr<ParamManager>&)
{
	lengthOut = value.size();
	valueOut = (lengthOut == 0) ? 0 : &value[0];
}

DECLARE_CONVERT_TO_PARAM_CAPABLE(std::vector<std::string>, type_string)
class CharVecManager : public ParamManager
{
	private:
		std::vector<cstr_type> m_cstrVec;
	public:
		CharVecManager(const std::vector<std::string>& strings)
			: m_cstrVec(strings.size(), 0)
		{
			for(int i = 0, end=strings.size(); i < end; ++i)
				m_cstrVec[i] = strings[i].c_str();
		}
		cstr_type* value()
		{
			return m_cstrVec.empty() ? 0 : &m_cstrVec[0];
		}
};
void convertToParam(const std::vector<std::string>& value, const cstr_type*& valueOut, 
		            int& lengthOut, boost::shared_ptr<ParamManager>& manager)
{
	boost::shared_ptr<CharVecManager> cManager(new CharVecManager(value));
	manager = cManager;
	valueOut = cManager->value();
	lengthOut = value.size();
}

/// Holder for a parameter value
class ParamValue
{
	private:
		// One of: type_float, type_integer or type_string, corresponding to
		// m_value pointing at float, int or cstr_type.
		VariableType m_type;
		// The value array is held in m_value with type indicated by m_type.
		const void* m_value;
		// Length of the value array.
		int m_length;
		// Any management of the data pointed to by m_value is managed by m_valueManager.
		boost::shared_ptr<ParamManager> m_valueManager;

	public:
		// Extract one of the underlying types from the user-defined input
		// value by making use of an appropriate user-defined convertToParam
		// function.
		template<typename T>
		ParamValue(const T& value)
			: m_type(defaultParamType<T>::value),
			m_value(0),
			m_length(0),
			m_valueManager()
		{
			switch(m_type)
			{
				case type_float:
					{
						const float* outValue = reinterpret_cast<const float*>(m_value);
						convertToParam(value, outValue, m_length, m_valueManager);
						m_value = reinterpret_cast<const void*>(outValue);
					}
					break;
				case type_integer:
					{
						const int* outValue = reinterpret_cast<const int*>(m_value);
						convertToParam(value, outValue, m_length, m_valueManager);
						m_value = reinterpret_cast<const void*>(outValue);
					}
					break;
				case type_string:
					{
						cstr_type const* outValue = reinterpret_cast<const cstr_type*>(m_value);
						convertToParam(value, outValue, m_length, m_valueManager);
						m_value = reinterpret_cast<const void*>(outValue);
					}
					break;
				default:
					assert(0);
					return;
			}
		}

		VariableType type() { return m_type; }
		int length() const { return m_length; }
		const void* value() const { return m_value; }

		// Typesafe visitation of the parameter value.
		template<typename VisitorT>
		void visit(VisitorT visitor) const
		{
			switch(m_type)
			{
				case type_float:
					visitor(reinterpret_cast<const float*>(m_value), m_length);
					break;
				case type_integer:
					visitor(reinterpret_cast<const int*>(m_value), m_length);
					break;
				case type_string:
					visitor(reinterpret_cast<const cstr_type*>(m_value), m_length);
					break;
				default:
					assert(0);
					return;
			}
		}
};


/// Wrapper around a (Token, ParamValue) pair.
class Param
{
	private:
		Token m_token;
		ParamValue m_value;

		template<typename VisitorT>
		class ParamVisitor
		{
			private:
				const Token& m_token;
				VisitorT& m_paramVisitor;
			public:
				ParamVisitor(const Token& token, VisitorT& paramVisitor)
					: m_token(token), m_paramVisitor(paramVisitor)
				{}
				template<typename T>
				void operator()(const T* value, int length)
				{
					m_paramVisitor(m_token, value, length);
				}
		};
	public:
		// Various constructor functions for convenience.
		template<typename T>
		Param(const Token& token, const T& value)
			: m_token(token), m_value(value) { }

		template<typename T>
		Param(VariableType type, const std::string& name, const T& value)
			: m_token(class_uniform, type, 1, name), m_value(value) { }

		template<typename T>
		Param(const std::string& name, const T& value)
			: m_token(class_invalid, type_lookup, 1, name), m_value(value) { }

		template<typename T>
		Param(VariableClass Class, VariableType type, const std::string& name, const T& value)
			: m_token(Class, type, 1, name), m_value(value) { }

		template<typename T>
		Param(VariableType type, int arraySize, const std::string& name, const T& value)
			: m_token(class_uniform, type, arraySize, name), m_value(value) { }

		template<typename T>
		Param(VariableClass Class, VariableType type, int arraySize,
			  const std::string& name, const T& value)
			: m_token(Class, type, arraySize, name), m_value(value) { }

		const Token& token() const { return m_token; }

		// Typesafe visitation of the contents of the Param
		template<typename VisitorT>
		void visit(VisitorT visitor) const
		{
			m_value.visit(ParamVisitor<VisitorT>(m_token, visitor));
		}
};


/// Printer visitor for data in a Param class, for use in operator<<()
class PrintParamVisitor
{
	private:
		std::ostream& m_out;

		template<typename T> void print(std::ostream& out, const T& val) { out << val; }
		void print(std::ostream& out, cstr_type val) { out << '"' << val << '"'; }
	public:
		PrintParamVisitor(std::ostream& out) : m_out(out) {}

		template<typename T>
		void operator()(const Token& token, const T* values, int length)
		{
			m_out << '"' << token.name << "\": [";
			for(int i = 0; i < length; ++i)
			{
				print(m_out, values[i]);
				if(i != length-1)
					m_out << " ";
			}
			m_out << "]";
		}
};

/// Stream insertion for Param classes.
std::ostream& operator<<(std::ostream& out, const Param& p)
{
	p.visit(PrintParamVisitor(out));
	return out;
}

// Encapsulate a token and overload an = operator for keyword-parameter style
// initialisation in the ParamList class constructor.
class TokenKeyword
{
	private:
		Token m_token;
	public:
		// Convenience constructors allowing various aspects of the token
		// to be defaults.
		TokenKeyword(const Token& token)
			: m_token(token) { }
		TokenKeyword(VariableType type, const std::string& name)
			: m_token(class_uniform, type, 1, name) { }
		TokenKeyword(const std::string& name)
			: m_token(class_invalid, type_lookup, 1, name) { }
		TokenKeyword(VariableClass Class, VariableType type, const std::string& name)
			: m_token(Class, type, 1, name) { }
		TokenKeyword(VariableType type, int arraySize, const std::string& name)
			: m_token(class_uniform, type, arraySize, name) { }
		TokenKeyword(VariableClass Class, VariableType type, int arraySize, const std::string& name)
			: m_token(Class, type, arraySize, name) { }

		template<typename T>
		Param operator=(const T& value)
		{
			return Param(m_token, value);
		}
};

// Predeclared keyword objects to be used in creating parameter lists.
namespace Ri
{
	TokenKeyword P (class_vertex,  type_point, 1, "P" );
	TokenKeyword Cs(class_varying, type_color, 1, "Cs");
	TokenKeyword st(class_varying, type_float, 2, "st");
}

// Renderman parameter list for passing to Renderer methods.
class ParamList
{
	private:
		typedef std::vector<Param> PlistT;
		PlistT m_pList;
	public:
		typedef PlistT::const_iterator const_iterator;

		// Constructors with varying numbers of parameter arguments, up to five
		// for now.
		ParamList() {};
		ParamList(const Param& p1)
		{
			m_pList.push_back(p1);
		}
		ParamList(const Param& p1, const Param& p2)
		{
			m_pList.reserve(2);
			m_pList.push_back(p1); m_pList.push_back(p2);
		}
		ParamList(const Param& p1, const Param& p2, const Param& p3)
		{
			m_pList.reserve(3);
			m_pList.push_back(p1); m_pList.push_back(p2);
			m_pList.push_back(p3);
		}
		ParamList(const Param& p1, const Param& p2, const Param& p3,
				  const Param& p4)
		{
			m_pList.reserve(4);
			m_pList.push_back(p1); m_pList.push_back(p2);
			m_pList.push_back(p3); m_pList.push_back(p4);
		}
		ParamList(const Param& p1, const Param& p2, const Param& p3,
				  const Param& p4, const Param& p5)
		{
			m_pList.reserve(5);
			m_pList.push_back(p1); m_pList.push_back(p2);
			m_pList.push_back(p3); m_pList.push_back(p4);
			m_pList.push_back(p5);
		}

		ParamList& operator<<(const Param& param)
		{
			m_pList.push_back(param);
			return *this;
		}

		//--------------------------------------------------
		// Iterator access
		const_iterator begin() const
		{
			return m_pList.begin();
		}
		const_iterator end() const
		{
			return m_pList.end();
		}
};


// Simple non-abstract prototype for the Renderer interface.
class Renderer
{
	public:
		// RiSphere has an optional param list.
		void Sphere(float radius, float minz, float maxz, float angle,
				const ParamList& params/* = ParamList() avoid ambiguity */)
		{
			// Figure out how to extract stuff from the ParamList...
			std::cout << "Sphere:\n"
				<< "  rad = " << radius << "\n"
				<< "  minz = " << minz << "\n"
				<< "  maxz = " << maxz << "\n"
				<< "  angle = " << angle << "\n";

			std::cout << "  params = {\n";
			for(ParamList::const_iterator i = params.begin(); i != params.end(); ++i)
				std::cout << "    " << *i << "\n";
			std::cout << "  }\n";

			/*
			// Something like what would be used to convert to the traditional RI:
			RiParamList riParams(params);
			RiSphere(radius, minz, maxz, angle,
			         riParams.count(), riParams.tokens(), riParams.values());
			*/
		}

		// RiGeneralPolygon has an array argument;  We need to decide what to
		// do with this.
		void GeneralPolygon(int nloops, int nverts[],
                            const ParamList& pList = ParamList())
		{
		}
};


int main()
{
	using namespace boost::assign;

	// Declare some parameter values.  (Initialisation using boost::assign)

	std::vector<float> P;
	P += 1,1,1, 0,0,0, 3,3,3, 0,0,0, 4,4,4;
	std::vector<float> Cs;
	Cs += 0, 0.5, 1;
	std::vector<float> st;
	st += 0, -1.5;

	// Parameters can potentially be of any type.  vectors of std::string and
	// const char* both work.
	std::vector<std::string> textureNames;
	textureNames += "asdf", "qwer", "wasd", "uiop";
	std::vector<const char*> textureNames2;
	textureNames2 += "asdf_2", "qwer_2", "wasd_2", "uiop_2";


	// Here's the usage for a class-based renderer context based on using the
	// ParamList object above for creating typesafe parameter lists.

	Renderer ri;

	ri.Sphere(1, -1, 1, 360, ParamList()
		<< Param(type_point, "P", P)
		<< Param(class_varying, type_color, "Cs", Cs)
		<< Param("st", st)
		<< Param(class_uniform, type_string, "texturename", textureNames)
		<< Param(class_uniform, type_string, "texturename2", textureNames2)
	);


	// Here's a simple "keyword arguments" style invocation:

	ri.Sphere(2, -2, 2, 360, ParamList(Ri::P=P, Ri::Cs=Cs, Ri::st=st));

	// More complex keyword argument usage with user-defined keywords are
	// possible too:

	TokenKeyword TxName(type_string, "texturename");

	ri.Sphere(3, -3, 3, 360,
	          ParamList(Ri::P=P, Ri::Cs=Cs, Ri::st=st,
	                    TxName=textureNames,
	                    TokenKeyword(type_string, "texturename2")=textureNames2));

	
	// Alternative interfaces
	// ----------------------

	/*

	// With some extra work we can get the following (TBH this probably has a
	// poor benefit to effort/complexity ratio.)

	ri.Sphere(2, -1.5, 1.5, 360)
		<< Param(type_point, "P", P)
		<< Param(class_varying, type_color, "Cs", Cs)
		<< Param(type_float, 2, "ua", ua);


	// Gelato-style ?

	ri.param(type_point, "P", P);
	ri.param(class_varying, type_color, "Cs", Cs);
	ri.param(type_float, 2, "ua", ua);
	ri.param(class_uniform, type_string, "texturename", textureName);
	ri.Sphere(1, -1, 1, 360);

	*/

	return 0;
}



#if 0

/** Class to convert a ParamList into the (count, tokens, values) triple needed
 * by the traditional C API.
 */
class RiParamList
{
	private:
		std::vector<std::string> m_tokenStorage;
		std::vector<RtToken> m_tokens;
		std::vector<RtPointer> m_values;

	public:
		RiParamList(const ParamList& params)
			: m_tokens(),
			m_values()
		{
			int count = params.end() - params.begin();
			m_tokens.reserve(count);
			m_tokenStorage.reserve(count);
			m_values.reserve(count);
			for(ParamList::const_iterator i = params.begin(), end = params.end();
					i != end; ++i)
			{
				m_tokenStorage.push_back(i->token->asString());
				m_tokens.push_back(const_cast<RtToken>(m_tokenStorage.back().c_str()));
				// FIXME!!  This won't really work.
				m_values.push_back(i->value->getPtr());
			}
		}

		RtToken* tokens() const
		{
			return &m_tokens[0];
		}

		RtPointer* values() const
		{
			return &m_values[0];
		}

		int count() const
		{
			return m_tokens.size();
		}
};
#endif


