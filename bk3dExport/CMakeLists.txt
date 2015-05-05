Project(bk3dExporter)

include_directories(
    ${MAYA_INCLUDE_DIRS}
    ${NV_MATH_DIR}
    ${PUBLIC_HEADERS_DIR}
    ${PUBLIC_HELPERS_DIR}
    ${CURVE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${bk3dlib_SOURCE_DIR}
)

set(SOURCE_FILES
    bk3dExport.cpp
    blendshapes.cpp
    CMakeLists.txt
    curves.cpp
    extractors.cpp
    MayaHelpers.cpp
    skinning.cpp
)
set(MEL_SCRIPTS
    ${PROJECT_SOURCE_DIR}/bk3dExportOptions.mel
)

set(HEADER_FILES
    bk3dExport.h
    MayaHelpers.h
    MiscHelpers.h
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
        nvModel
        bk3dlib
    )
    set(PLATFORM_PLUGIN_EXTENSION
        .mll
    )
    set(PLATFORM_LINK_FLAGS
        "/export:initializePlugin /export:uninitializePlugin"
    )
endif(WIN32)

add_definitions(
    ${PLATFORM_COMPILE_FLAGS}
)

source_group(nv_math FILES
    ${NV_MATH_SOURCE}
    ${NV_MATH_HEADERS}
)

add_library(bk3dExporter SHARED
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${NV_MATH_SOURCE}
    ${NV_MATH_HEADERS}
)

set_target_properties(bk3dExporter
    PROPERTIES
    OUTPUT_NAME "bk3dExporter"
    SUFFIX ${PLATFORM_PLUGIN_EXTENSION}
    LINK_FLAGS "${PLATFORM_LINK_FLAGS}"
)

target_link_libraries(bk3dExporter
    ${MAYA_Foundation_LIBRARY}
    ${MAYA_OpenMaya_LIBRARY}
    ${MAYA_OpenMayaUI_LIBRARY}
    ${MAYA_OpenMayaAnim_LIBRARY}
    ${MAYA_tbb_LIBRARY}
    ${PLATFORM_LIBRARIES}
)

_copy_files_to_target( bk3dExporter "${MEL_SCRIPTS}")

# TODO: export the Env. Var to where is the mll plugin MAYA_PLUG_IN_PATH