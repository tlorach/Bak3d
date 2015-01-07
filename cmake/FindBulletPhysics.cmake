# Try to find BULLET project dll and include file
#
unset(BULLET_PHYSICS_INCLUDE_DIR CACHE)
unset(BULLET_PHYSICS_FOUND CACHE)
unset(BULLET_PHYSICS_INSTALL_PATH CACHE)

if(WIN32)
if( ARCH STREQUAL "x64" )
  set( BULLET_PHYSICS_INSTALL_PATH "C:/Program Files/BULLET_PHYSICS")
  set( ARCHSUFFIX "64")
else ()
  set( BULLET_PHYSICS_INSTALL_PATH "C:/Program Files (x86)/BULLET_PHYSICS")
endif()
endif(WIN32)

#message(STATUS "SEARCHING for BULLET in ${BULLET_PHYSICS_INSTALL_PATH}" )

find_path( BULLET_PHYSICS_INCLUDE_DIR "bullet/btBulletCollisionCommon.h"
  ${BULLET_PHYSICS_LOCATION}/include
  $ENV{BULLET_PHYSICS_LOCATION}/include
  ${BULLET_PHYSICS_INSTALL_PATH}/include
)

find_path( BULLET_PHYSICS_LIB_DIR "BulletDynamics.lib"
  ${BULLET_PHYSICS_LOCATION}/lib
  $ENV{BULLET_PHYSICS_LOCATION}/lib
  ${BULLET_PHYSICS_INSTALL_PATH}/lib
)

set( BULLET_PHYSICS_FOUND "NO" )

if(BULLET_PHYSICS_INCLUDE_DIR)
  if(BULLET_PHYSICS_LIB_DIR)
    set( BULLET_PHYSICS_FOUND "YES" )
    set( BULLET_PHYSICS_HEADERS 
      "${BULLET_PHYSICS_INCLUDE_DIR}/bullet/btBulletCollisionCommon.h"
      "${BULLET_PHYSICS_INCLUDE_DIR}/bullet/btBulletDynamicsCommon.h"
    )
    set( BULLET_PHYSICS_LIBRARIES
      "${BULLET_PHYSICS_LIB_DIR}/BulletCollision.lib"
      "${BULLET_PHYSICS_LIB_DIR}/BulletDynamics.lib"
      "${BULLET_PHYSICS_LIB_DIR}/BulletSoftBody.lib"
      "${BULLET_PHYSICS_LIB_DIR}/LinearMath.lib"
    )
    set( BULLET_PHYSICS_LIBRARIES_DEBUG
      "${BULLET_PHYSICS_LIB_DIR}/BulletCollision_Debug.lib"
      "${BULLET_PHYSICS_LIB_DIR}/BulletDynamics_Debug.lib"
      "${BULLET_PHYSICS_LIB_DIR}/BulletSoftBody_Debug.lib"
      "${BULLET_PHYSICS_LIB_DIR}/LinearMath_Debug.lib"
    )
  endif()
else()
  Message(WARNING "setup BULLET_PHYSICS_LOCATION in cmake or Env. system Variable of same name (Don't forget to Check INSTALL_LIBS in bullet's cmake)")
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(BULLET DEFAULT_MSG
    BULLET_PHYSICS_INCLUDE_DIR
    BULLET_PHYSICS_LIB_DIR
)

mark_as_advanced( BULLET_PHYSICS_FOUND )
