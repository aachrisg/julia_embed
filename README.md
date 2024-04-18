# julia_embed
A simple standalone system for exporting c++ functions so that they can be called from Julia. Does NOT require the Julia SDK.

These standalone files implement a simple system for exporting functions from c++ so that they can be called from Julia. It includes an interactive load/unload ability
so that you can recompile your code and load new versions while debugging.

NOTE that only very basic functionality is provided. In particular not many argument types are supported. This is easily fixable. Even in this primitive state I was easily able
to accomplish what I wanted - the ability to interactively produce a 3d realtime VR visualization of data manipulated inside of an interactive notebook.

# Building
This code should build fine on any system. There are no dependencies and no Julia SDK is needed. A trivial CMake file is provided but if you're not using Cmake, all 
you have to do is link the 2 .cpp files into a loadable .dll/.so file.

# Using in Julia
You will first include "importcpp.jl" in your julia session. After that you can use ImportCPP( "path/to/dll_name" ). Note that you should leave off the .so or .dll extension.
When you load the dll it will tell you what to type when you unload it. If you call an expoted function that has been unloaded, you will get an error.

You can run the included test.jl by typing "julia test.jl".

ImportCpp works by loading a symbol table of exported functions and parameter types from the dll. It then uses Julia's ability to construct and compile code 
at run time to build an AST tree defining the function binding, and then compiles it to native code. I'd never written this kind of code in Julia or for that
matter much of any Julia code. I'm sure it could be a lot better but it didn't take long to write, it works, and yer gets what yer pay for :-).

# Notebook interface demo
These screenshots are me graphing a c++ implementation of "A Convenient Generalization of Schlick’s Bias and Gain Functions" (https://arxiv.org/pdf/2010.09714.pdf)
inside of Jupyterlab.

![image](https://github.com/aachrisg/julia_embed/assets/43356352/02f80cfa-9252-45d8-b3bb-d01d8d9d55b3)
*C++ code in Emacs*

![image](https://github.com/aachrisg/julia_embed/assets/43356352/ecc762a8-d78a-49f1-9a20-5a3a4e0ba378)
*Plotting the c++ function interactively*


