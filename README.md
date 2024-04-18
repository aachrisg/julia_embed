# julia_embed
A simple standalone system for exporting c++ functions so that they can be called from Julia. Does NOT require the Julia SDK.

These standalone files implement a simple system for exporting functions from c++ so that they can be called from Julia. It includes an interactive load/unload ability
so that you can recompile your code and load new versions while debugging.

NOTE that only very basic functionality is provided. In particular not many argument types are supported. This is easily fixable. Even in this primitive state I was easily able
to accomplish what I wanted - the ability to interactively produce a 3d realtime VR visualizatio of data manipulated inside of an interactive notebook.

# Building
This code should build fine on any system. There are no dependencies and no Julia SDK is needed. A trivial CMake file is provided but if you're not using Cmake, all 
you have to do is link the 2 .cpp files into a loadable .dll/.so file.

# using in Julia
You will first include "importcpp.jl" in your julia session. After that you can use ImportCPP( "path/to/dll_name" ). Note that you shuld leave off the .so or .dll extension.

# Notebook interface demo
These screenshots are me graphing a c++ implementation of "A Convenient Generalization of Schlick’s Bias and Gain Functions" (https://arxiv.org/pdf/2010.09714.pdf)
inside of Jupyterlab.
