set(source_files
    model/irs.cc
)
set(header_files
    helper/irs_utilities.h
    model/irs.h
)

build_lib(
  LIBNAME irs
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK
    ${libcore}
    ${libnetwork}
    ${libapplications}
    ${libreport}
    ${liblte}
    ${libwifi}
    ${libmobility}
)
