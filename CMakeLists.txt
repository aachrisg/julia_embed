cmake_minimum_required(VERSION 3.26)

project (embed_test)

add_library(embed_test SHARED exposer.cpp embed_test.cpp )

set_property( TARGET embed_test PROPERTY CXX_STANDARD 20 )

