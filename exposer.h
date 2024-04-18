#pragma once
#include <mutex>
#include <vector>
template<class DataType_t> char const *TypeName;
template<> inline char const *TypeName<char**> = "char**";
template<> inline char const *TypeName<int> = "int";
template<> inline char const *TypeName<float> = "float";
template<> inline char const *TypeName<double> = "double";
template<> inline char const *TypeName<size_t> = "size_t";
template<> inline char const *TypeName<void> = "void";


#ifndef DLL_PUBLIC
#ifdef __GNUC__
//      #define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
#define DLL_PUBLIC
//[[gnu::dllexport]]
//__attribute__ ((dllexport))
#else
      #define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
	#define strdup _strdup
#endif
#endif

//:mutex macros:
#ifndef withlock
#define withlock( object ) if ( std::unique_lock lk( object ) ; true )
//< Execute the following block or statement while holding the lock
#endif

template<class T> void AddToEnd( T *&list, T *pNode )
{
	T **pPred = &list;
	for( T *i = list; i; i = i->m_pNext )
	{
		pPred = &( i->m_pNext );
	}
	*( pPred ) = pNode;
}

struct TypeReference_t
{
	TypeReference_t *m_pNext = nullptr;					//< So that lists of types can be passed
	char const *m_pTypeName = nullptr;								  //< "int" for instance
	char const *m_pUsageName = nullptr;								  //< Can be ignored but could contain things like parameter names

	TypeReference_t( char const *pTypeName )
	{
		m_pTypeName = pTypeName;
	}
};

struct FunctionDeclaration_t
{
	char const *m_pName = nullptr;
	void *m_functionPtr = nullptr;
	TypeReference_t *m_pReturnType = nullptr;				//< nullptr for void functions
	TypeReference_t *m_pArgTypes = nullptr;							//< Linked together by m_pNext
	FunctionDeclaration_t( char const *pName, void *fFunction )
	{
		m_pName = pName;
		m_functionPtr = fFunction;
	}
};

// can't use std::tuple for the multiple return because of c linkage
struct SymbolTableList_t							
{
	size_t m_nNumSymbols;
	FunctionDeclaration_t const *m_pExposedSymbols;
};

class CExposedSymbolTable
{
	std::mutex m_mutex;
public:
	std::mutex m_inProgressMutex; 
	//<  If this mutex is held, Symbol queries will be blocked until it is released.  This can be
	//< used to hold off someone from importing the symbol table before some asynchronous code has
	//< initialized it.
	
	std::vector<FunctionDeclaration_t> m_exposedSymbols;

	void AddFunction( FunctionDeclaration_t const &decl )
	{
		withlock( m_mutex )
		{
			m_exposedSymbols.push_back( decl );
		}
	}

	// Use variadic template syntax to allow any number of args
	template<typename FirstArg, typename...RestArgs>
	void AddParameters( FunctionDeclaration_t &decl )
	{
		AddToEnd( decl.m_pArgTypes, new TypeReference_t( TypeName<FirstArg> ) );
		if constexpr ( sizeof...( RestArgs ) != 0 )
		{
			AddParameters<RestArgs...>( decl );
		}
	}

	template<typename RetType_t, typename...ArgTypes>
	void ExposeFunctionDefinition( char const *pFunctionName, RetType_t (*f)( ArgTypes... ) )
	{
		FunctionDeclaration_t decl( pFunctionName, ( void *) f );
		decl.m_pReturnType = new TypeReference_t( TypeName<RetType_t> );
		if constexpr ( sizeof...( ArgTypes ) != 0 )
		{
			AddParameters<ArgTypes...>( decl );
		}
		AddFunction( decl );
	}
};

extern CExposedSymbolTable g_exposedSymbolTable;

#define EXPOSE_xstr(s) str(s)
#define EXPOSE_str(s) #s
#define EXPOSEFN( name ) g_exposedSymbolTable.ExposeFunctionDefinition( EXPOSE_str( name ), name );

#define EXPOSE_WAITFORME g_exposedSymbolTable.m_inProgressMutex.lock()
//< Call EXPOSE_WAITFORME to prevent anyone from getting the symbol table until we are done popoulating it
#define EXPOSE_DONE g_exposedSymbolTable.m_inProgressMutex.unlock()
//< if EXPOSE_WAITFORME has been called this signals that you are finished and they can ask for the symbol table now


void StartDLL( int argc, char **argv );											// The app must provide this. It should do any setup and add symbols


