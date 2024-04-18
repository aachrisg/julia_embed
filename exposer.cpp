#include "exposer.h"
#include <string.h>



CExposedSymbolTable g_exposedSymbolTable;

extern "C" inline DLL_PUBLIC SymbolTableList_t GetExposedSymbols()
//< Called by Julia to get a pointer to the symbol table
{
	withlock( g_exposedSymbolTable.m_inProgressMutex )
	{
		SymbolTableList_t ret;
		ret.m_nNumSymbols = g_exposedSymbolTable.m_exposedSymbols.size();
		ret.m_pExposedSymbols = g_exposedSymbolTable.m_exposedSymbols.data();
		return ret;
	}
}

extern "C" inline DLL_PUBLIC void Setup( int argc, char **argv )
//< Called by Julia after we load the dll, to get the export symbol table built.
{
	// we are going to duplicate the args to avoid them being freed from under us by julia during GC
	static std::vector<char *> argVector;
	argVector.resize( argc );
	for( int i =0 ; i < argc; i++ )
	{
		argVector[i] = strdup( argv[i] );
	}
	// prevent adding symbols over and over during testing of import code
	static bool s_bAlreadyDidIt = false;
	if ( ! s_bAlreadyDidIt )
	{
		s_bAlreadyDidIt = true;
		StartDLL( argc, argVector.data() );
	}
}
