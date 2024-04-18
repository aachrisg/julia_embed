#include "exposer.h"

#include <math.h>
#include <stdio.h>

// no args
void HelloWorld()
{
	printf( "hello, world\n" );
}

// one arg
auto MySin( float flTheta ) -> float
{
	return sinf( flTheta );
}

// 2 args
auto Mag2( float flX, float flY ) -> float
{
	return flX * flX + flY * flY;
}


void StartDLL( int argc, char **argv )
{
	EXPOSEFN( HelloWorld );
	EXPOSEFN( MySin );
	EXPOSEFN( Mag2 );

	#if 0
	// The way to use this in non-toy apps is to have each of your libraries include an Expose
	// function that you can call at init time to populate the symbol table. But in this test we
	// only export a couple of trivial test functions.
	ExposeGeo();
	#endif
}



