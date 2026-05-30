if(NOT DEFINED APP_BINARY)
    message(FATAL_ERROR "APP_BINARY is required")
endif()

if(NOT EXISTS "${APP_BINARY}")
    message(FATAL_ERROR "App executable does not exist: ${APP_BINARY}")
endif()

get_filename_component(app_binary_name "${APP_BINARY}" NAME)

if(CMAKE_HOST_WIN32)
    if(NOT app_binary_name STREQUAL "HarufushiPatchDependency.exe")
        message(FATAL_ERROR "App executable must be HarufushiPatchDependency.exe, got ${app_binary_name}")
    endif()
else()
    if(NOT app_binary_name STREQUAL "HarufushiPatchDependency")
        message(FATAL_ERROR "App executable must be HarufushiPatchDependency, got ${app_binary_name}")
    endif()
endif()
