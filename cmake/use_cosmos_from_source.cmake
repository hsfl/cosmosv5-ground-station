if(NOT DEFINED COSMOS_GROUND_STATION_INCLUDED)
set(COSMOS_GROUND_STATION_INCLUDED TRUE)

get_filename_component(COSMOS_SOURCE_GROUND_STATION "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

if(DEFINED COSMOS_SOURCE)
    # Auto-initialize the lower-layer submodule if not present
    if(NOT EXISTS "${COSMOS_SOURCE}/modules/CMakeLists.txt")
        message(STATUS "cosmosv5: initializing modules submodule...")
        execute_process(
            COMMAND git submodule update --init modules
            WORKING_DIRECTORY "${COSMOS_SOURCE}"
            RESULT_VARIABLE _cosmos_git_result
        )
        if(NOT _cosmos_git_result EQUAL 0)
            message(FATAL_ERROR
                "cosmosv5: modules submodule is not initialized and auto-init failed.\n"
                "Run from the cosmosv5 workspace root:\n"
                "  git submodule update --init modules")
        endif()
    endif()
    include(${COSMOS_SOURCE}/modules/cmake/use_cosmos_from_source.cmake)
else()
    message(FATAL_ERROR "COSMOS_SOURCE not set. Set COSMOS_SOURCE to the cosmosv5 workspace root (the directory containing thirdparty/, kernel/, etc.).")
endif()

message("Using COSMOS ground-station from " ${COSMOS_SOURCE_GROUND_STATION})

include_directories(${COSMOS_SOURCE_GROUND_STATION}/libraries)
include_directories(${COSMOS_SOURCE_GROUND_STATION}/libraries/device)

add_subdirectory(${COSMOS_SOURCE_GROUND_STATION}/libraries/device/general   ${CMAKE_BINARY_DIR}/ground-station/libraries/device/general)
add_subdirectory(${COSMOS_SOURCE_GROUND_STATION}/libraries/device/netradio  ${CMAKE_BINARY_DIR}/ground-station/libraries/device/netradio)

endif()
