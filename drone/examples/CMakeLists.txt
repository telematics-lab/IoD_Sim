build_lib_example(
  NAME iod-sim
  SOURCE_FILES iod-sim.cc
  LIBRARIES_TO_LINK ${libdrone}
                    ${libcore}
                    ${libapplications}
                    ${libmobility}
                    ${libnetwork}
                    ${libwifi}
                    ${libinternet}
)
