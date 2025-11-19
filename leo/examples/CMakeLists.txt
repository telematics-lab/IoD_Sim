# CMakeLists.txt for LEO module examples
# Based on the original wscript configuration

# Basic examples that don't require special dependencies
set(basic_examples
    leo-circular-orbit-tracing-example
    leo-car-move-trace-example
)

# Examples that require additional modules (aodv, epidemic-routing)
set(routing_examples
    leo-delay-tracing-example
    leo-bulk-send-example
)

# Examples that require NR module
set(nr_examples
    leo-nr-send-example
)

# Build basic examples
foreach(
  example
  ${basic_examples}
)
  build_lib_example(
    NAME ${example}
    SOURCE_FILES ${example}.cc
    LIBRARIES_TO_LINK ${libleo}
                      ${libcore}
                      ${libmobility}
  )
endforeach()

# Build advanced examples (check if required modules are available)
if(aodv IN_LIST ns3-all-enabled-modules)
  # Check if epidemic-routing is available for routing examples
  if(epidemic-routing IN_LIST ns3-all-enabled-modules)
    foreach(
      example
      ${routing_examples}
    )
      build_lib_example(
        NAME ${example}
        SOURCE_FILES ${example}.cc
        LIBRARIES_TO_LINK ${libleo}
                          ${libcore}
                          ${libmobility}
                          ${libaodv}
                          ${libepidemic-routing}
      )
    endforeach()
  else()
    message(
      WARNING
        "leo-delay-tracing-example and leo-bulk-send-example require epidemic-routing module"
    )
  endif()
else()
  message(
    WARNING
      "leo-delay-tracing-example and leo-bulk-send-example require aodv module"
  )
endif()

# Build NR examples if nr module is available
if(nr IN_LIST ns3-all-enabled-modules)
  foreach(
    example
    ${nr_examples}
  )
    build_lib_example(
      NAME ${example}
      SOURCE_FILES ${example}.cc
      LIBRARIES_TO_LINK ${libleo}
                        ${libcore}
                        ${libmobility}
                        ${libnr}
                        ${libantenna}
                        ${libbuildings}
                        ${libinternet}
                        ${libapplications}
                        ${libpoint-to-point}
    )
  endforeach()
else()
  message(
    WARNING
      "leo-nr-send-example requires nr (5g-lena) module"
  )
endif()
