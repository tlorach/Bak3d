Project(PyBk3d)

include_directories(
    ${NV_MATH_DIR}
    ${PUBLIC_HEADERS_DIR}
    ${PUBLIC_HELPERS_DIR}
    ${CURVE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${bk3dlib_SOURCE_DIR}
    ${PYTHON_INCLUDE_DIR}
)

set(SOURCE_FILES
    dllmain.cpp
    PyBk3d.cpp
    PyBk3dBuffer.cpp
    PyBk3dHeader.cpp
    PyBk3dMaterial.cpp
    PyBk3dMesh.cpp
    PyBk3dTransform.cpp
)
set(MEL_SCRIPTS
    ${PROJECT_SOURCE_DIR}/bk3dExportOptions.mel
)

set(HEADER_FILES
    PyBk3d.h
    stdafx.h
    targetver.h
    ${bk3dlib_SOURCE_DIR}/bk3dLib.h
)

if(UNIX)
    set(PLATFORM_COMPILE_FLAGS
        -D_BOOL
        -DREQUIRE_IOSTREAM
        -DLINUX
    )
    set(PLATFORM_LIBRARIES
        ${ZLIB_LIBRARIES}
        ${PYTHON_LIBRARY}
        nvModel
        bk3dlib
    )
    set(PLATFORM_PLUGIN_EXTENSION
        .so
    )
    set(PLATFORM_LINK_FLAGS
    )
endif(UNIX)

if(WIN32)
    set(PLATFORM_COMPILE_FLAGS
        /D_AFXDLL
        /DNT_PLUGIN
        /DREQUIRE_IOSTREAM
    )
    set(PLATFORM_LIBRARIES
        ${ZLIB_LIBRARIES}
        ${PYTHON_LIBRARY}
        nvModel
        bk3dlib
    )
    set(PLATFORM_PLUGIN_EXTENSION
        .pyd
    )
    #set(PLATFORM_LINK_FLAGS "/export:PyInit_bk3d" )
endif(WIN32)

add_definitions(
    ${PLATFORM_COMPILE_FLAGS}
)

source_group(nv_math FILES
    ${NV_MATH_SOURCE}
    ${NV_MATH_HEADERS}
)

add_library(PyBk3d SHARED
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${NV_MATH_SOURCE}
    ${NV_MATH_HEADERS}
)

set_target_properties(PyBk3d
    PROPERTIES
    OUTPUT_NAME "bk3d"
    SUFFIX ${PLATFORM_PLUGIN_EXTENSION}
    LINK_FLAGS "${PLATFORM_LINK_FLAGS}"
)

target_link_libraries(PyBk3d
    ${PLATFORM_LIBRARIES}
)

if(ZLIB_FOUND)
  _copy_files_to_target( PyBk3d "${ZLIB_BIN}")
endif()

##################################################################
# INSTALL
if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    install(TARGETS PyBk3d
        RUNTIME DESTINATION "${PYTHON_INCLUDE_DIR}/../Lib/site-packages/bk3d")
    if(ZLIB_FOUND)
        install(FILES ${ZLIB_BIN} DESTINATION "${PYTHON_INCLUDE_DIR}/../Lib/site-packages/bk3d")
    endif()
endif()

