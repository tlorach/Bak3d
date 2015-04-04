SET(MODELS_DOWNLOAD_FORCE OFF CACHE BOOL "Forces the download operation of 3D models needed for this demo" )
SET(MODELS_DOWNLOAD_DISABLED OFF CACHE BOOL "DO NOT try to download 3D models needed for this demo... to avoid getting stuck if slow BW" )
#####################################################################################
# Macro for adding a (potentially cuda) executable.
#
if(USE_GLUT OR NOT WIN32)
    macro(_add_glut_executable target)
        if(WIN32)
            # Windows needs some of its dependency dll's copied into the same
            # directory as the executable.
            set( LIBRARIES ${GLUT_LIBRARIES})
            foreach (LIB ${LIBRARIES} )
                string(REPLACE ".lib" ".dll" DLL ${LIB})
                string(REPLACE ".LIB" ".DLL" DLL ${DLL})
                add_custom_command(
                    TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${DLL}
                        $<TARGET_FILE_DIR:${target}>
                )
            endforeach()
        endif()

    endmacro()
endif()
#####################################################################################
# Optional UI
#
if(USE_ANTTWEAKBAR)
    find_package(AntTweakBar)
    if(ANTTWEAKBAR_FOUND)
        add_definitions(-DUSEANTTWEAKBAR)
    endif()
endif()

#####################################################################################
# Optional Device
#
if(USE_SVCSPACEMOUSE)
    find_package(SvcSpaceMouse)
    if(SVCSPACEMOUSE_FOUND)
        add_definitions(-DUSESVCSPACEMOUSE)
    else()
    message(STATUS "
        -------------------------------------------------------------------------------------------
        Optional: SvcSpaceMouse service (dll) not found.
        You can install it from https://github.com/tlorach/SvcSpaceMouse )... and compile it first.
        You can also set SVCSPACEMOUSE_LOCATION in cmake or as Env. variable.
        The samples will run without additional UI. But that's okay ;-)
        -------------------------------------------------------------------------------------------
        "
    )
    endif()
endif()

#####################################################################################
# Keep track of nvImage files
#
#find_path(NV_IMAGE_DIR nvImage.h "${SHARED_DIR}/nvImage")
set(NV_IMAGE_DIR "${SHARED_DIR}/nvImage")
set(NV_IMAGE_HEADERS "${NV_IMAGE_DIR}/nvImage.h" "${NV_IMAGE_DIR}/rgbe.h")
set(NV_IMAGE_SOURCES 
    "${NV_IMAGE_DIR}/rgbe.c"
    "${NV_IMAGE_DIR}/nvImage.cpp"
    "${NV_IMAGE_DIR}/nvImageDDS.cpp"
    "${NV_IMAGE_DIR}/nvImageHdr.cpp"
    #"${NV_IMAGE_DIR}/nvImagePng.cpp" # canceled it because we would then need png library... :-(
)

#####################################################################################
# folder for various models and textures
find_path(RESOURCE_DIR README.md "resources")

##################################################################
# Shared folder
set(EXAMPLES_SHARED_DIR "${PROJECT_SOURCE_DIR}/examples/shared")
#####################################################################################
# nvFx package
find_package(nvFx)
#####################################################################################
#
if( OPENGL_FOUND AND GLEW_FOUND OR (APPLE AND GLUT_FOUND))
    add_subdirectory(bk3dViewer)
    add_subdirectory(simpleOpenGL)
else()
    set(MISSING "")

    if (NOT OPENGL_FOUND)
        list(APPEND MISSING OpenGL)
    endif()

    if (NOT GLEW_FOUND)
        list(APPEND MISSING glew)
    endif()

    message(WARNING
        "The following libraries could not be found : ${MISSING}.  "
    )
endif()

#if(DXSDK_FOUND)
#   add_subdirectory(dxSimple)
#endif()

#####################################################################################
# download model for this demo to run. Avoids using GitHub for this
# TODO: we need to put the models online. Should happen next week (>Dec.22)
#
if(NOT ${MODELS_DOWNLOAD_DISABLED})
  set(FILELIST 
  "RCcar_v134.bk3d.gz"
  )
  _download_files(
    "${FILELIST}"
    "http://developer.download.nvidia.com/ProGraphics/nvpro-samples"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources"
    ${MODELS_DOWNLOAD_FORCE}
  )
endif()
