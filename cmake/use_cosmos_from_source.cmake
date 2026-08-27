if(NOT DEFINED COSMOS_GROUND_STATION_INCLUDED)
set(COSMOS_GROUND_STATION_INCLUDED TRUE)

get_filename_component(COSMOS_SOURCE_GROUND_STATION "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

if(EXISTS "${COSMOS_SOURCE_GROUND_STATION}/deps/modules/cmake/use_cosmos_from_source.cmake")
    include(${COSMOS_SOURCE_GROUND_STATION}/deps/modules/cmake/use_cosmos_from_source.cmake)
elseif(DEFINED COSMOS_SOURCE)
    include(${COSMOS_SOURCE}/modules/cmake/use_cosmos_from_source.cmake)
else()
    message(FATAL_ERROR "Cannot find cosmosv5-modules. Clone with --recurse-submodules or set COSMOS_SOURCE.")
endif()

message("Using COSMOS ground-station from " ${COSMOS_SOURCE_GROUND_STATION})

include_directories(${COSMOS_SOURCE_GROUND_STATION}/libraries)
include_directories(${COSMOS_SOURCE_GROUND_STATION}/libraries/device)

add_subdirectory(${COSMOS_SOURCE_GROUND_STATION}/libraries/device/general   ${CMAKE_BINARY_DIR}/ground-station/libraries/device/general)
add_subdirectory(${COSMOS_SOURCE_GROUND_STATION}/libraries/device/netradio  ${CMAKE_BINARY_DIR}/ground-station/libraries/device/netradio)

endif()
