PROJECT( bk3dutil )

add_subdirectory(nvModel)

set(PLATFORM_LIBRARIES
    ${ZLIB_LIBRARIES}
    nvModel
    bk3dlib
)

file(GLOB HFILES "." "*.h")
file(GLOB CPPFILES "." "*.cpp")

#SET(REGEXP 
#  "regexp/Matcher.cpp"
#  "regexp/Pattern.cpp"
#  "regexp/regexp/Matcher.h"
#  "regexp/regexp/Pattern.h"
#)

include_directories(
    ${PUBLIC_HEADERS_DIR}
    ${PUBLIC_HELPERS_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${nvModel_SOURCE_DIR}/include
    #"regexp"
    ${bk3dlib_SOURCE_DIR}
    ${NV_MATH_DIR}
)

#source_group(regexp FILES
#  ${REGEXP}
#)
source_group(public_headers FILES
  ${PUBLIC_HEADERS}
  ${PUBLIC_HELPERS_DIR}/bk3dDbgFuncs.inl
)
source_group(nv_math FILES
  ${NV_MATH_SOURCE}
  ${NV_MATH_HEADERS}
)
add_executable(bk3dutil
  ${CPPFILES}
  ${HFILES}
  ${PUBLIC_HEADERS}
  ${PUBLIC_INL}
  #${REGEXP}
  ${nvModel_SOURCE_DIR}/include/nvModel.h
  ${bk3dlib_SOURCE_DIR}/bk3dLib.h
  ${NV_MATH_SOURCE}
  ${NV_MATH_HEADERS}
)

if(NOT ZLIB_FOUND)
  add_definitions(-DNOGZLIB)
endif()

target_link_libraries(bk3dutil
    ${PLATFORM_LIBRARIES}
)

##################################################################
# INSTALL
if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    install(TARGETS bk3dutil
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" )
endif()

if(ZLIB_FOUND)
  _copy_files_to_target( bk3dutil "${ZLIB_BIN}")
endif()
