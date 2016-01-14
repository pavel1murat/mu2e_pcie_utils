FIND_PACKAGE(SWIG REQUIRED) 

INCLUDE(${SWIG_USE_FILE}) 

function(create_node_package_json NODEJS_ADDON_NAME)
find_file( PACKAGE_JSON_PATH package.json.in PATHS ${CMAKE_MODULE_PATH} NO_DEFAULT_PATH )
        configure_file (${PACKAGE_JSON_PATH} ${CMAKE_CURRENT_BINARY_DIR}/package.json @ONLY)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/package.json
            DESTINATION ${flavorqual_dir}/lib/node_modules/${NODEJS_ADDON_NAME} )
endfunction()

function(create_nodejs_addon NODEJS_ADDON_NAME NODEJS_ADDON_INCLUDES NODEJS_ADDON_LIBS)
    find_path (NODE_ROOT_DIR "node/node.h")
    set (NODE_INCLUDE_DIRS
      ${NODE_ROOT_DIR}/src
      ${NODE_ROOT_DIR}/node
      ${NODE_ROOT_DIR}/deps/v8/include
      ${NODE_ROOT_DIR}/deps/uv/include
    )

    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")

    file(GLOB NODEJS_ADDON_SOURCES  *.i)
    file(GLOB LIB_SOURCES  *.cpp)

    set_source_files_properties (${NODEJS_ADDON_SOURCES} PROPERTIES CPLUSPLUS ON)
    set_source_files_properties (${NODEJS_ADDON_SOURCES} PROPERTIES SWIG_FLAGS "-node")
    
    list(APPEND NODEJS_ADDON_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}) 
    
    swig_add_module (${NODEJS_ADDON_NAME} javascript ${NODEJS_ADDON_SOURCES} ${LIB_SOURCES})

    swig_link_libraries (${NODEJS_ADDON_NAME} ${NODE_LIBRARIES} ${NODEJS_ADDON_LIBS})
    
    target_include_directories ( ${NODEJS_ADDON_NAME} PUBLIC ${NODE_INCLUDE_DIRS} ${NODEJS_ADDON_INCLUDES})
     
    set_target_properties (${NODEJS_ADDON_NAME} PROPERTIES
      COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -DBUILDING_NODE_EXTENSION -DSWIG_V8_VERSION=0x0${V8_DEFINE_STRING}"
      PREFIX ""
      SUFFIX ".node"
    )
    
    create_node_package_json(${NODEJS_ADDON_NAME})

    install (FILES ${LIBRARY_OUTPUT_PATH}/${NODEJS_ADDON_NAME}.node DESTINATION ${flavorqual_dir}/lib/node_modules/${NODEJS_ADDON_NAME})
    
    add_custom_command(TARGET ${NODEJS_ADDON_NAME} POST_BUILD 
      COMMAND echo "**** Exports for ${LIBRARY_OUTPUT_PATH}/${NODEJS_ADDON_NAME}.node"
      COMMAND echo "**** BEGIN"
      COMMAND /usr/bin/nm ${LIBRARY_OUTPUT_PATH}/${NODEJS_ADDON_NAME}.node | /bin/egrep -e \"^[a-f0-9]{1,16} [T]\" | /usr/bin/c++filt  
      COMMAND echo "**** END" )

endfunction()
