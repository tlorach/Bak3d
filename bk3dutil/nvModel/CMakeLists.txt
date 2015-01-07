cmake_minimum_required(VERSION 2.8)

project(nvModel)

include_directories( 
  "include" 
  "nvMath" 
  ${GLEW_INCLUDE_DIR}
)

file(GLOB HFILES "." "include/*.h")
file(GLOB CPPFILES "." "src/*.cpp")

# build a static library
add_library(nvModel STATIC
  ${HFILES}
  ${CPPFILES}
)

set_target_properties( nvModel PROPERTIES FOLDER "nvModel" )

set(NVMODEL_LIB nvModel)
