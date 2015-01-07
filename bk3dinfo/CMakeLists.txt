PROJECT( bk3dinfo )

set(PLATFORM_LIBRARIES
    ${ZLIB_LIBRARIES}
)

set(SOURCE_FILES "bk3dInfo.cpp")

include_directories(
    ${PUBLIC_HEADERS_DIR}
    ${PUBLIC_HELPERS_DIR}
    ${ZLIB_INCLUDE_DIRS}
)

add_executable(bk3dinfo
  ${SOURCE_FILES}
  ${PUBLIC_HEADERS}
  ${PUBLIC_HELPERS_DIR}/bk3dDbgFuncs.inl
)

if(NOT ZLIB_FOUND)
  add_definitions(-DNOGZLIB)
endif()

target_link_libraries(bk3dinfo
    ${PLATFORM_LIBRARIES}
)

##################################################################
# INSTALL
if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    install(TARGETS bk3dinfo
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" )
endif()
