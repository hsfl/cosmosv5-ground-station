if(NOT DEFINED COSMOS_GROUND_STATION_INCLUDED)
set(COSMOS_GROUND_STATION_INCLUDED TRUE)

get_filename_component(COSMOS_SOURCE_GROUND_STATION "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

if(DEFINED COSMOS_SOURCE)
    include(${COSMOS_SOURCE}/modules/cmake/use_cosmos_from_source.cmake)
else()
    message(FATAL_ERROR "COSMOS_SOURCE not set. Clone the cosmosv5 workspace with --recurse-submodules or set COSMOS_SOURCE to the workspace root.")
endif()

message("Using COSMOS ground-station from " ${COSMOS_SOURCE_GROUND_STATION})

include_directories(${COSMOS_SOURCE_GROUND_STATION}/libraries)
include_directories(${COSMOS_SOURCE_GROUND_STATION}/libraries/device)

add_subdirectory(${COSMOS_SOURCE_GROUND_STATION}/libraries/device/general   ${CMAKE_BINARY_DIR}/ground-station/libraries/device/general)
add_subdirectory(${COSMOS_SOURCE_GROUND_STATION}/libraries/device/netradio  ${CMAKE_BINARY_DIR}/ground-station/libraries/device/netradio)

endif()
