cmake_minimum_required (VERSION 2.6)
project (hnsw_lib)

include_directories("${PROJECT_BINARY_DIR}")



set(SOURCE_EXE hnsw.cpp)            

set(SOURCE_LIB sift_1b.cpp stats.cpp)

add_library(sift_test STATIC ${SOURCE_LIB})

add_executable(hnsw ${SOURCE_EXE})

SET( CMAKE_CXX_FLAGS  "-Ofast -lrt -DNDEBUG -std=c++11 -DHAVE_CXX0X -openmp -march=native -fpic -w -fopenmp -ftree-vectorize -ftree-vectorizer-verbose=0" )
target_link_libraries(hnsw sift_test) 
