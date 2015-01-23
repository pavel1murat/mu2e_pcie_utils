# ======================================================================
#  pcie_linux_kernel_module main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/artdaq-demo/ups/setup_for_development
#  buildtool
# ======================================================================


# use cmake 2.8 or later
cmake_minimum_required (VERSION 2.8)


project(pcie_linux_kernel_module)
# cetbuildtools contains our cmake modules
SET ( CETBUILDTOOLS_VERSION $ENV{CETBUILDTOOLS_VERSION} )
IF (NOT CETBUILDTOOLS_VERSION)
    MESSAGE (FATAL_ERROR "ERROR: setup cetbuildtools to get the cmake modules")
ENDIF()

set(CMAKE_MODULE_PATH 
        $ENV{ART_DIR}/Modules
        $ENV{CETBUILDTOOLS_DIR}/Modules
        ${CMAKE_MODULE_PATH})


include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS VIGILANT
 WERROR
 NO_UNDEFINED
 EXTRA_FLAGS -pedantic -Wno-unused-function -Wno-unused-parameter
 )

cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_ups_product( cetbuildtools v3_03_00 )
find_ups_product( swig v3_0_0 )
find_ups_product( TRACE v3_03_00 )

find_library( TRACE trace PATHS $ENV{TRACE_LIB} )

# macros for art_dictionary and simple_plugin
include(CetTest)

# source
add_subdirectory(dtcInterfaceLib)
add_subdirectory(linux_driver)
add_subdirectory(mcs)

# ups - table and config files
add_subdirectory(ups)

# packaging utility
include(UseCPack)
