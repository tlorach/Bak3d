Project(simpleOpenGL)

set(PLATFORM_LIBRARIES
    ${OPENGL_LIBRARY}
    ${GLEW_LIBRARY}
    ${CUDA_LIBRARIES}
    ${ZLIB_LIBRARIES}
)
include_directories(
    ../../inc
    ${GLEW_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${SHARED_DIR}
    ${EXAMPLES_SHARED_DIR}
    ${NV_MATH_DIR}
)
set(SOURCE_FILES 
  simpleOpenGL.cpp
  GLSLProgram.cpp
  GLSLProgram.h
   ../../inc/bk3dBase.h
   ../../inc/bk3dEx.h
   ../../inc/bk3dDefs.h
  ${EXAMPLES_SHARED_DIR}/main_win32.cpp
)
#####################################################################################
# Glew : source or lib
if(GLEW_SOURCE)
  message(STATUS "found Glew source code. Using it instead of library")
  LIST(APPEND SOURCE_FILES ${GLEW_SOURCE})
  add_definitions(-DGLEW_STATIC)
else()
    message(STATUS "using GLEW library")
    LIST(APPEND PLATFORM_LIBRARIES ${GLEW_LIBRARY})
endif()
#####################################################################################
# GLUT
if(USE_GLUT AND GLUT_FOUND)
  LIST(APPEND PLATFORM_LIBRARIES ${GLUT_LIBRARIES} )
  include_directories(${GLUT_INCLUDE_DIR})
endif()
source_group(nv_math FILES
  ${NV_MATH_SOURCE}
  ${NV_MATH_HEADERS}
)
source_group(bk3d_headers FILES
  ${BK3D_HEADERS}
)

#if(APPLE)
#    _add_glut_executable(simpleOpenGL simpleOpenGL.cpp)
#    add_definitions(-DAPPLE)
#else()
#_add_glut_executable(simpleOpenGL
#    simpleOpenGL.cpp
#    ${INC_FILES}
#)
#endif()

if(NOT USE_GLUT)
  set(WIN_SRC
    ${EXAMPLES_SHARED_DIR}/resources.h
    ${EXAMPLES_SHARED_DIR}/resources.rc
  )
endif()

if(NOT ZLIB_FOUND)
  add_definitions(-DNOGZLIB)
endif()
if(WIN32)
  add_executable(simpleOpenGL WIN32
    ${SOURCE_FILES}
    ${PUBLIC_HEADERS}
    ${SOURCES}
    ${NV_MATH_SOURCE}
    ${NV_MATH_HEADERS}
    ${BK3D_HEADERS}
    ${WIN_SRC}
    ${EXAMPLES_SHARED_DIR}/Camera.h
    ${EXAMPLES_SHARED_DIR}/Realtime.h
  )
  #remove the printf warning
  add_definitions(/wd4996)
  if((NOT USE_GLUT) OR (NOT GLUT_FOUND))
    # strange: not working... but WIN32 above solve the issue
    #set_target_properties(simpleOpenGL PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    #set_target_properties(simpleOpenGL PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
    add_definitions(-DNOGLUT)
  endif()
else()
  add_executable(simpleOpenGL
    ${SOURCE_FILES}
    ${PUBLIC_HEADERS}
    ${SOURCES}
    ${NV_MATH_SOURCE}
    ${NV_MATH_HEADERS}
    ${BK3D_HEADERS}
    ${WIN_SRC}
    ${EXAMPLES_SHARED_DIR}/Camera.h
    ${EXAMPLES_SHARED_DIR}/Realtime.h
  )
endif()
target_link_libraries(simpleOpenGL
    ${PLATFORM_LIBRARIES}
)

# if we wanted to have the effect close to the exe
# but let's use the effect from the resource directory instead : 
# the sample will pull the effect from RESOURCE_DIR
#_copy_files_to_target(simpleOpenGL ${PROJECT_SOURCE_DIR}/effect.glslfx)

if(ZLIB_FOUND)
  _copy_files_to_target( simpleOpenGL "${ZLIB_BIN}")
endif()
#_copy_files_to_target( simpleOpenGL "${RESOURCE_DIR}/NV_Shaderball_v133.bk3d.gz")
add_definitions(-DRESOURCE_DIR="${RESOURCE_DIR}")

