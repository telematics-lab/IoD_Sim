create_test_sourcelist(Tests
  demo_irs-test1.json
  demo_irs-test2.json
  demo_irs-test3.json
  demo_irs-test4.json
  demo_irs_noSN.json
  demo_no_irs.json
  lm_grieco_s2.json
  lm_grieco_s3.json
  paper_irs_s1.json
  paper_irs_s2.json
  paper_s1.json
  paper_s2.json
  paper_s3-lte.json
  paper_s3-wifi.json
  paper_simple.json
  s1_var_patch.json
  s2_c1.json
  s2_c2.json
  s2_c3.json
  s3_defined_1_all.json
  s3_defined_1_uncov.json
  s3_defined_4_all_diff.json
  s3_defined_4_all_same.json
  s3_defined_4_uncov_diff.json
  s3_defined_4_uncov_same.json
  s3_periodic_1_all.json
  s3_periodic_1_uncov.json
  s3_periodic_4_all_diff.json
  s3_periodic_4_all_same.json
  s3_periodic_4_uncov_diff.json
  s3_periodic_4_uncov_same.json
  s3_random_1_all.json
  s3_random_1_uncov.json
  s3_random_4_all.json
  s3_random_4_uncov.json
  s3_u.json
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
