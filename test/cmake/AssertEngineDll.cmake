if(NOT DEFINED ENGINE_BINARY)
    message(FATAL_ERROR "ENGINE_BINARY is required")
endif()

get_filename_component(engine_binary_name "${ENGINE_BINARY}" NAME)

if(CMAKE_HOST_WIN32)
    if(NOT engine_binary_name STREQUAL "harufushi_engine.dll")
        message(FATAL_ERROR "harufushi_engine must build as a DLL, got ${engine_binary_name}")
    endif()
else()
    if(NOT engine_binary_name MATCHES "^libharufushi_engine\\.(so|dylib)$")
        message(FATAL_ERROR "harufushi_engine must build as a shared library, got ${engine_binary_name}")
    endif()
endif()

if(NOT EXISTS "${ENGINE_BINARY}")
    message(FATAL_ERROR "Engine binary does not exist: ${ENGINE_BINARY}")
endif()
