cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

SET(sampleName FRR)

project(PrjFRR)

include_directories("C:/Users/Albin/Documents/GitHub/Fast-Realistic-Rendering/deps/include")

link_directories("C:/Users/Albin/Documents/GitHub/Fast-Realistic-Rendering/deps/libs")

add_executable (FRR CubeMap.cpp CubeMap.h main.cpp PLYDrawer.cpp PLYDrawer.h plyloader.cpp plyloader.h shaderLoader.cpp shaderLoader.h)