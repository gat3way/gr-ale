INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_ALE ale)

FIND_PATH(
    ALE_INCLUDE_DIRS
    NAMES ale/api.h
    HINTS $ENV{ALE_DIR}/include
        ${PC_ALE_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    ALE_LIBRARIES
    NAMES gnuradio-ale
    HINTS $ENV{ALE_DIR}/lib
        ${PC_ALE_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ALE DEFAULT_MSG ALE_LIBRARIES ALE_INCLUDE_DIRS)
MARK_AS_ADVANCED(ALE_LIBRARIES ALE_INCLUDE_DIRS)

