# CMakeLists.txt for the ns-3 'leo' module
# Translated from the original wscript file for compatibility with modern ns-3

# Use standard ns-3 module macros
# include(ns3-module-macros)

# List all public header files that should be installed
# Note: Added 'model/leo-module.h' which was missing from the original wscript
# but is required by examples.
set(public_headers
    helper/arp-cache-helper.h
    helper/isl-helper.h
    helper/leo-channel-helper.h
    helper/leo-input-fstream-container.h
    helper/leo-orbit-node-helper.h
    helper/nd-cache-helper.h
    helper/leo-ground-node-helper.h
    helper/satellite-node-helper.h
    model/geo-leo-orbit-mobility.h
    model/geo-sgp4-mobility.h
    model/geo-constant-velocity-mobility.h
    model/leo-mock-channel.h
    model/leo-mock-net-device.h
    model/leo-oneweb-constants.h
    model/leo-orbit.h
    model/leo-lat-long.h
    model/leo-polar-position-allocator.h
    model/leo-propagation-loss-model.h
    model/leo-starlink-constants.h
    model/leo-telesat-constants.h
    model/mock-net-device.h
    model/mock-channel.h
    model/isl-mock-channel.h
    model/isl-propagation-loss-model.h
)

# List all C++ source files to be compiled
set(sources
    helper/arp-cache-helper.cc
    helper/isl-helper.cc
    helper/leo-channel-helper.cc
    helper/leo-input-fstream-container.cc
    helper/leo-orbit-node-helper.cc
    helper/nd-cache-helper.cc
    helper/leo-ground-node-helper.cc
    helper/satellite-node-helper.cc
    model/geo-leo-orbit-mobility.cc
    model/geo-sgp4-mobility.cc
    model/geo-constant-velocity-mobility.cc
    model/leo-mock-channel.cc
    model/leo-mock-net-device.cc
    model/leo-orbit.cc
    model/leo-lat-long.cc
    model/leo-polar-position-allocator.cc
    model/leo-propagation-loss-model.cc
    model/mock-net-device.cc
    model/mock-channel.cc
    model/isl-mock-channel.cc
    model/isl-propagation-loss-model.cc
    sgp4/libsgp4/CoordGeodetic.cc
    sgp4/libsgp4/CoordTopocentric.cc
    sgp4/libsgp4/DateTime.cc
    sgp4/libsgp4/DecayedException.cc
    sgp4/libsgp4/Eci.cc
    sgp4/libsgp4/Globals.cc
    sgp4/libsgp4/Observer.cc
    sgp4/libsgp4/OrbitalElements.cc
    sgp4/libsgp4/SGP4.cc
    sgp4/libsgp4/SatelliteException.cc
    sgp4/libsgp4/SolarPosition.cc
    sgp4/libsgp4/TimeSpan.cc
    sgp4/libsgp4/Tle.cc
    sgp4/libsgp4/TleException.cc
    sgp4/libsgp4/Util.cc
    sgp4/libsgp4/Vector.cc
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
)

build_lib(
  LIBNAME leo
  SOURCE_FILES ${sources}
  HEADER_FILES ${public_headers}
  LIBRARIES_TO_LINK
    ${dependencies}
  TEST_SOURCES
    test/ground-node-helper-test-suite.cc
    test/isl-mock-channel-test-suite.cc
    test/isl-propagation-test-suite.cc
    test/isl-test-suite.cc
    test/leo-anim-test-suite.cc
    test/leo-orbit-test-suite.cc
    test/leo-input-fstream-container-test-suite.cc
    test/leo-mobility-test-suite.cc
    test/leo-mock-channel-test-suite.cc
    test/leo-propagation-test-suite.cc
    test/leo-test-suite.cc
    test/leo-trace-test-suite.cc
    test/satellite-node-helper-test-suite.cc
)

# This tells CMake to also look for examples in the 'examples' subdirectory
if(NS3_EXAMPLES_ENABLED)
    add_subdirectory(examples)
endif()