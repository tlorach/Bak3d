cmake_minimum_required(VERSION 2.8)

PROJECT( Bak3d )

set(VERSION "1.3.3")

set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for executables")
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers")

set(USE_GLUT OFF CACHE BOOL "For PC only : on if you want to use GLUT instead of Windows")
set(USE_ANTTWEAKBAR ON CACHE BOOL "AntTweakBar UI")
set(USECURVES ON CACHE BOOL "If we want to use Maya curves")
set(USE_SVCSPACEMOUSE ON CACHE BOOL "PC only: 3DConnexion space mouse")
if(WIN32)
  SET( USE_NSIGHT OFF CACHE BOOL "Use NSight" )
  SET( MEMORY_LEAKS_CHECK OFF CACHE BOOL "Build the project for Memory leaks check" )
endif(WIN32)

mark_as_advanced(CLEAR ZLIB_LIBRARY ZLIB_INCLUDE_DIR)

#set (ZLIB_LIBRARY "" CACHE PATH "zlib dir")
#set (ZLIB_INCLUDE_DIR "" CACHE PATH "zlib dir")

# Specify the list of directories to search for cmake modules.
set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
)

set(PUBLIC_HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc )
set(PUBLIC_HELPERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc_helpers )
file(GLOB PUBLIC_HEADERS "." "${PUBLIC_HEADERS_DIR}/*.[hi]*")

set( CMAKE_FIND_ROOT_PATH "")

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set (ARCH "x64" CACHE STRING "CPU Architecture")
  #set( ZLIB_ROOT "$ENV{ProgramFiles}") # strange... still returning x86 folder... grrr
  # I add ZLIB_ROOT because findZLIB uses $ENV{ProgramFiles} and fails, too
  set( ZLIB_ROOT "C:\\Program Files\\zlib")
else ()
  set (ARCH "x86" CACHE STRING "CPU Architecture")
  # I add ZLIB_ROOT because findZLIB uses $ENV{ProgramFiles} and fails, too
  set( ZLIB_ROOT "$ENV{ProgramFiles}/zlib")
endif()

set(LIBRARY_OUTPUT_PATH
    ${PROJECT_BINARY_DIR}/lib
    CACHE PATH
    "Directory where libraries will be stored"
)
set(EXECUTABLE_OUTPUT_PATH
    ${PROJECT_BINARY_DIR}/bin
    CACHE PATH
    "Directory where executables will be stored"
)

#####################################################################################
# Shared folder
set(SHARED_DIR "${PROJECT_SOURCE_DIR}/shared")

#####################################################################################
# Keep track of math files
#
#find_path(NV_MATH_DIR nv_math.h "${SHARED_DIR}/nv_math")
set(NV_MATH_DIR "${SHARED_DIR}/nv_math")
set(NV_MATH_HEADERS "${NV_MATH_DIR}/nv_math.h" "${NV_MATH_DIR}/nv_algebra.h" "${NV_MATH_DIR}/nv_mathdecl.h")
set(NV_MATH_SOURCE "${NV_MATH_DIR}/nv_math.cpp")

#####################################################################################
# Keep track of shared curve files
if(USECURVES)
  set(CV_DIR "${SHARED_DIR}/MayaCurve")
  set(CV_HEADERS "${CV_DIR}/curveEngine.h" "${CV_DIR}/curveReader.h")
  set(CV_SOURCES "${CV_DIR}/curveEngine.cpp" "${CV_DIR}/curveReader.cpp")
else()
    add_definitions(-DNOCURVES)
endif()

#####################################################################################
# NSIGHT
#
if(USE_NSIGHT)
  add_definitions(-DUSE_NSIGHT)
  include_directories(NSight)
  if( ARCH STREQUAL "x64" )
    set(NSIGHT_LIB "${PROJECT_SOURCE_DIR}/NSight/nvToolsExt64_1.lib")
    set(NSIGHT_DLL "${PROJECT_SOURCE_DIR}/NSight/nvToolsExt64_1.dll")
  else()
    set(NSIGHT_LIB "${PROJECT_SOURCE_DIR}/NSight/nvToolsExt32_1.lib")
    set(NSIGHT_DLL "${PROJECT_SOURCE_DIR}/NSight/nvToolsExt32_1.dll")
  endif()
  #MARK_AS_ADVANCED(NSIGHT_LIB)
else()
  set(NSIGHT_LIB, "")
endif()

#####################################################################################
if(UNIX) 
  set(OS "linux")
  add_definitions(-DLINUX)
else(UNIX)
  if(APPLE)
  else(APPLE)
    if(WIN32)
      set(OS "win")
      add_definitions(-DNOMINMAX)
      if(MEMORY_LEAKS_CHECK)
        add_definitions(-DMEMORY_LEAKS_CHECK)
      endif()
    endif(WIN32)
  endif(APPLE)
endif(UNIX)

# Macro for adding files close to the executable
macro(_copy_files_to_target target thefiles)
    if(WIN32)
        foreach (FFF ${thefiles} )
			add_custom_command(
			    TARGET ${target} POST_BUILD
			    COMMAND ${CMAKE_COMMAND} -E copy_if_different
			        ${FFF}
			        $<TARGET_FILE_DIR:${target}>
			        VERBATIM
			)
        endforeach()
    endif()

endmacro()


#####################################################################################
# ZLIB
#
if( ARCH STREQUAL "x64" )
  #set( ZLIB_ROOT "$ENV{ProgramFiles}") # strange... still returning x86 folder... grrr
  # I add ZLIB_ROOT because findZLIB uses $ENV{ProgramFiles} and fails, too
  set( ZLIB_ROOT "C://Program Files//zlib")
else ()
  # I add ZLIB_ROOT because findZLIB uses $ENV{ProgramFiles} and fails, too
  set( ZLIB_ROOT "$ENV{ProgramFiles}/zlib")
endif()
find_package(ZLib)
if(NOT ZLIB_FOUND)
  add_definitions(-DNOGZLIB)
  message(WARNING "
    ZLIB not found : the project will be created without the capability to read compressed models.
    But you can still unzip the models themselves, before using them through the samples."
  )
endif()
MARK_AS_ADVANCED(CLEAR ZLIB_LIBRARY ZLIB_INCLUDE_DIR)


#####################################################################################
# ===============> Bullet
find_package(BulletPhysics)

# ===============> Python
find_package(Python)

# ===============> Maya
find_package(Maya)

# ===============> OpenGL
find_package(OpenGL)
# ===============> GLUT
if(USE_GLUT OR NOT WIN32)
  mark_as_advanced(CLEAR GLUT_LIBRARIES)
  find_package(GLUT)
  if(NOT GLUT_FOUND)
    if(WIN32)
      message(STATUS "No GLUT. Falling back to regular Win32 Windows layout")
    endif()
    message(WARNING "Glut needed on this platform for samples. Try to set GLUT_LOCATION")
  endif()
endif()
# ===============> GLEW
if (NOT APPLE)
  find_package(GLEW REQUIRED)
  if(NOT GLEW_FOUND)
    message(WARNING "Try to set GLEW_LOCATION")
  endif()
endif()

# ===============> GLEW
add_subdirectory(bk3dinfo)
add_subdirectory(bk3dlib)
add_subdirectory(bk3dutil)
add_subdirectory(examples)
add_subdirectory(PyBk3d)
if(MAYA_FOUND)
  add_subdirectory(bk3dExport)
endif()

##################################################################
# INSTALL
if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
    install(FILES ${PUBLIC_HEADERS} DESTINATION "${INSTALL_INC_DIR}")
    install(FILES ${PUBLIC_HEADERS} DESTINATION "${INSTALL_INC_DIR}")
endif()

