create_test_sourcelist(Tests
  lm_grieco_s2.json
  lm_grieco_s3.json
  paper_s1.json
  paper_s2.json
  paper_s3-lte.json
  paper_s3-wifi.json
  paper_simple.json
  simple_dual_stack.json
  simple_lte.json
  simple_peripherals.json
  simple_relay.json
  simple_wifi-ofdm.json
  simple_wifi-threelog_pathloss.json
  simple_wifi.json
  test_cadmm.json
  test_periphstream-lte.json
  test_periphstream-wifi.json
)

set(exec ${CMAKE_SOURCE_DIR}/ns3/build/examples/ns3.38-iodsim-debug)

foreach(test ${Tests})
  get_filename_component (TName ${test} NAME_WE)
  set(config ${CMAKE_SOURCE_DIR}/scenario/${test})
  add_test(NAME ${TName} COMMAND ${exec} --config=${config})
  set_tests_properties(${TName} PROPERTIES TIMEOUT 0)
endforeach()
