# This module defines
# EVENT_DISPLAY_LIBRARY, the name of the library to link against
# EVENT_DISPLAY_FOUND, if false, do not try to link to EVENT_DISPLAY
# EVENT_DISPLAY_INCLUDE_DIR, where to find EVENT_DISPLAY headers
#
# Note: If you see an empty EVENT_DISPLAY_LIBRARY_TEMP in your configuration
# and no EVENT_DISPLAY_LIBRARY, it means CMake did not find your EVENT_DISPLAY library
# (libEVENT_DISPLAY.dylib, libEVENT_DISPLAY.so, etc).
# These values are used to generate the final EVENT_DISPLAY_LIBRARY variable,
# but when these values are unset, EVENT_DISPLAY_LIBRARY does not get created.
#
#
# $EVENT_DISPLAYDIR is an environment variable that would
# correspond to the CMAKE_INSTALL_PREFIX=$EVENT_DISPLAYDIR
# used in building EVENT_DISPLAY.

SET(EVENT_DISPLAY_SEARCH_PATHS
        /usr/local
        /opt/local
        ${EVENT_DISPLAY_PATH}
        )

FIND_PATH(EVENT_DISPLAY_INCLUDE_DIR EventDisplay.hpp
        HINTS
        $ENV{EVENT_DISPLAYDIR}
        PATH_SUFFIXES include/wagasci/eve include
        PATHS ${EVENT_DISPLAY_SEARCH_PATHS}
        )

FIND_LIBRARY(EVENT_DISPLAY_LIBRARY_TEMP
        NAMES EventDisplay
        HINTS
        $ENV{EVENT_DISPLAYDIR}
        PATH_SUFFIXES lib64/wagasci/eve lib/wagasci/eve lib lib64
        PATHS ${EVENT_DISPLAY_SEARCH_PATHS}
        )

IF(EVENT_DISPLAY_LIBRARY_TEMP)
    # Set the final string here so the GUI reflects the final state.
    SET(EVENT_DISPLAY_LIBRARY ${EVENT_DISPLAY_LIBRARY_TEMP} CACHE STRING "Where the EVENT_DISPLAY Library can be found")
    # Set the temp variable to INTERNAL so it is not seen in the CMake GUI
    SET(EVENT_DISPLAY_LIBRARY_TEMP "${EVENT_DISPLAY_LIBRARY_TEMP}" CACHE INTERNAL "")
ENDIF(EVENT_DISPLAY_LIBRARY_TEMP)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(EVENT_DISPLAY REQUIRED_VARS EVENT_DISPLAY_LIBRARY EVENT_DISPLAY_INCLUDE_DIR)