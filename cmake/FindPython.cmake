#
# Try to find Python library and include path.
# Once done this will define
#
# PYTHON_FOUND
# PYTHON_INCLUDE_DIR
# PYTHON_LIBRARY
#

include(FindPackageHandleStandardArgs)

if (WIN32)
    find_path( PYTHON_INCLUDE_DIR
        NAMES
            Python.h
        PATHS
            ${PYTHON_LOCATION}/include
            $ENV{PYTHON_LOCATION}/include
            ${PYTHONHOME}/include
            $ENV{PYTHONHOME}/include
            DOC "The directory where Python.h resides" )
    # NOTE: for now, now way to check x86 vs. x64 python...
    # TODO
    if(ARCH STREQUAL "x86")
      find_library( PYTHON_LIBRARY
          NAMES
              python26 python27 python30 python31 python32 python33 python34
          PATHS
              ${PYTHON_LOCATION}/libs
              $ENV{PYTHON_LOCATION}/libs
              ${PYTHONHOME}/libs
              $ENV{PYTHONHOME}/libs
              DOC "The PYTHON library")
    else()
      find_library( PYTHON_LIBRARY
          NAMES
              python26 python27 python30 python31 python32 python33 python34
          PATHS
              ${PYTHON_LOCATION}/libs
              $ENV{PYTHON_LOCATION}/libs
              ${PYTHONHOME}/libs
              $ENV{PYTHONHOME}/libs
              DOC "The PYTHON library")
    endif()
endif ()

if (${CMAKE_HOST_UNIX})
    find_path( PYTHON_INCLUDE_DIR
        NAMES
            Python.h
        PATHS
            ${PYTHON_LOCATION}/include
            $ENV{PYTHON_LOCATION}/include
            ${PYTHONHOME}/include
            $ENV{PYTHONHOME}/include
            /usr/include
            /usr/local/include
            /sw/include
            /opt/local/include
            NO_DEFAULT_PATH
            DOC "The directory where Python.h resides"
    )
    find_library( PYTHON_LIBRARY
        NAMES
            python
        PATHS
            ${PYTHON_LOCATION}/lib
            $ENV{PYTHON_LOCATION}/lib
            ${PYTHONHOME}/lib
            $ENV{PYTHONHOME}/lib
            /usr/lib64
            /usr/lib
            /usr/local/lib64
            /usr/local/lib
            /sw/lib
            /opt/local/lib
            NO_DEFAULT_PATH
            DOC "The PYTHON library")
endif ()

find_package_handle_standard_args(PYTHON DEFAULT_MSG
    PYTHON_INCLUDE_DIR
    PYTHON_LIBRARY
)

mark_as_advanced( PYTHON_FOUND )
