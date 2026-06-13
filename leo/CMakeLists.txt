# CMakeLists.txt for the ns-3 'leo' module
# Translated from the original wscript file for compatibility with modern ns-3

# Use standard ns-3 module macros
# include(ns3-module-macros)

include(FetchContent)
FetchContent_Declare(
  sgp4
  GIT_REPOSITORY https://github.com/dnwrnr/sgp4.git
  GIT_TAG d176906dbfaf8361eee1f83bd1dc1bacc6582798
)
FetchContent_MakeAvailable(sgp4)

message(STATUS "SGP4 Source Directory: ${sgp4_SOURCE_DIR}")
include_directories(${sgp4_SOURCE_DIR})

# Fix missing include directories in the upstream target
target_include_directories(sgp4 PUBLIC
  $<BUILD_INTERFACE:${sgp4_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

# Export sgp4 target so it can be used by other modules (like leo) that are exported.
install(TARGETS sgp4 EXPORT ns3ExportTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# List all public header files that should be installed
# Note: Added 'model/leo-module.h' which was missing from the original wscript
# but is required by examples.
set(public_headers
    helper/contrib-leo-orbit-node-helper.h
    helper/leo-ground-node-helper.h
    model/geo-leo-orbit-mobility.h
    model/geo-sgp4-mobility.h
    model/geo-constant-velocity-mobility.h
    model/leo-orbit.h
    model/leo-lat-long.h
)

# List all C++ source files to be compiled
set(sources
    helper/contrib-leo-orbit-node-helper.cc
    helper/leo-ground-node-helper.cc
    model/geo-leo-orbit-mobility.cc
    model/geo-sgp4-mobility.cc
    model/geo-constant-velocity-mobility.cc
    model/leo-orbit.cc
    model/leo-lat-long.cc
)

# List the other ns-3 modules this one depends on
set(dependencies
    ${libcore}
    ${libinternet}
    ${libpropagation}
    ${libstats}
    ${libflow-monitor}
    ${libapplications}
    ${libpoint-to-point}
    ${libnetanim}
    ${libaodv}
    ${libbuildings}
    ${libantenna}
    ${libmobility}
    sgp4
)

build_lib(
  LIBNAME leo
  SOURCE_FILES ${sources}
  HEADER_FILES ${public_headers}
  LIBRARIES_TO_LINK
    ${dependencies}
)

# This tells CMake to also look for examples in the 'examples' subdirectory
if(NS3_EXAMPLES_ENABLED)
    add_subdirectory(examples)
endif()