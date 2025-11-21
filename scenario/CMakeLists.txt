create_test_sourcelist(Tests
  integrated_ntn.json
  irs_demo1.json
  irs_demo2.json
  irs_demo3.json
  irs_demo4.json
  irs_demo_channel.json
  irs_demo_noSN.json
  irs_paper_s1-varpatch.json
  irs_paper_s1.json
  irs_paper_s2-cluster1.json
  irs_paper_s2-cluster2.json
  irs_paper_s2-cluster2only.json
  irs_paper_s2-cluster3.json
  irs_paper_s2.json
  irs_paper_s3-defined_1_all.json
  irs_paper_s3-defined_1_uncov.json
  irs_paper_s3-defined_4_all_diff.json
  irs_paper_s3-defined_4_all_same.json
  irs_paper_s3-defined_4_uncov_diff.json
  irs_paper_s3-defined_4_uncov_same.json
  irs_paper_s3-noIRS.json
  irs_paper_s3-periodic_1_all.json
  irs_paper_s3-periodic_1_uncov.json
  irs_paper_s3-periodic_4_all_diff.json
  irs_paper_s3-periodic_4_all_same.json
  irs_paper_s3-periodic_4_uncov_diff.json
  irs_paper_s3-periodic_4_uncov_same.json
  irs_paper_s3-random_1_all.json
  irs_paper_s3-random_1_uncov.json
  irs_paper_s3-random_4_all.json
  irs_paper_s3-random_4_uncov.json
  lm_grieco_s2.json
  lm_grieco_s3.json
  ntn-hap-big.json
  ntn-hap-static.json
  ntn-hap-sweep.json
  ntn-hap-test.json
  ntn-hap.json
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

set(exec ${CMAKE_SOURCE_DIR}/ns3/build/ns3.45-scenario-debug)

foreach(test ${Tests})
  get_filename_component (TName ${test} NAME_WE)
  set(config ${CMAKE_SOURCE_DIR}/scenario/${test})
  add_test(NAME ${TName} COMMAND ${exec} --config=${config})
  set_tests_properties(${TName} PROPERTIES TIMEOUT 0)
endforeach()
