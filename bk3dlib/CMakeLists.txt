project(bk3dlib)

include_directories( 
  "../inc"
  "../inc_helpers"
  ${NV_MATH_DIR}
  ${ZLIB_INCLUDE_DIRS}
)

set(PUBLIC_HEADERS
  ../inc/bk3dBase.h
  ../inc/bk3dDefs.h
  ../inc/bk3dEx.h
)

file(GLOB HFILES "." "*.h")
file(GLOB HFILESLEGACY "." "legacy/*.h")
set(HEADERS
  ${HFILES}
  ${HFILESLEGACY}
)

#let cmake determine linker language
file(GLOB CPPFILES "." "*.cpp")
file(GLOB CPPFILESLEGACY "." "legacy/*.cpp")
set(SOURCES
  ${CPPFILESLEGACY}
  ${CPPFILES}
)
source_group(sources FILES ${SOURCES})
source_group("" FILES ${PUBLIC_HEADERS})

# build a static library
add_library(bk3dlib STATIC
  ${PUBLIC_HEADERS}
  ${HEADERS}
  ${SOURCES}
)

set_target_properties( bk3dlib PROPERTIES FOLDER "bk3dlib" )

##################################################################
# INSTALL
if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    install(TARGETS bk3dlib
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" )
endif()
if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
    install(FILES bk3dLib.h DESTINATION "${INSTALL_LIB_DIR}")
endif()
