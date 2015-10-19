if (DEFINED ENV{KDIR})
  set(KDIR "$ENV{KDIR}")
  execute_process(COMMAND cat ${KDIR}/include/config/kernel.release OUTPUT_VARIABLE KVERS OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
  execute_process(COMMAND uname -r OUTPUT_VARIABLE KVERS OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if(EXISTS "$ENV{TRACE_DIR}/module/${KVERS}/TRACE.ko")
  # Using a UPS product version of TRACE
  set(TRACE_KO "$ENV{TRACE_DIR}/module/${KVERS}/TRACE.ko")
  set(ESYMS "$ENV{TRACE_DIR}/module/${KVERS}/Module.symvers")
else()
  # MRB Build or TRACE not available. WILL FAIL if not available!
  set(TRACE_KO "${CMAKE_BINARY_DIR}/TRACE/module/${KVERS}/TRACE.ko")
  set(ESYMS "${CMAKE_BINARY_DIR}/TRACE/module/${KVERS}/Module.symvers")
endif()

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../../bin/pci_devel_main.ko
    COMMAND make CC=/usr/bin/gcc KBUILD_EXTRA_SYMBOLS="${ESYMS}" TRACE_INC="$ENV{TRACE_DIR}/include"
    COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_SOURCE_DIR}/pci_devel_main.ko ${CMAKE_CURRENT_BINARY_DIR}/../../bin/pci_devel_main.ko
    COMMAND make clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/pci_devel_main.c
            ${TRACE_KO}
)

add_custom_target(pci_devel_module ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../../bin/pci_devel_main.ko)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/../../bin/pci_devel_main.ko DESTINATION ${${product}_bin_dir})

install_headers()
install_source()
