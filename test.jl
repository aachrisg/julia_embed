# test loading embed_test. If this prints hello, world, it worked.

include( "importcpp.jl" )

ImportCPP( "Release/embed_test" );      # or wherever it is. Use libembedtest for linux.

HelloWorld();


