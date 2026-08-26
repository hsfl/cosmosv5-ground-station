if(NOT DEFINED COSMOS_GROUND_STATION_INCLUDED)
set(COSMOS_GROUND_STATION_INCLUDED TRUE)

include(${COSMOS_SOURCE}/modules/cmake/use_cosmos_from_source.cmake)

set(COSMOS_SOURCE_GROUND_STATION ${COSMOS_SOURCE}/ground-station)
message("Using COSMOS ground-station from " ${COSMOS_SOURCE_GROUND_STATION})

include_directories(${COSMOS_SOURCE_GROUND_STATION}/libraries)
include_directories(${COSMOS_SOURCE_GROUND_STATION}/libraries/device)

add_subdirectory(${COSMOS_SOURCE_GROUND_STATION}/libraries/device/general   ${CMAKE_BINARY_DIR}/ground-station/libraries/device/general)
add_subdirectory(${COSMOS_SOURCE_GROUND_STATION}/libraries/device/netradio  ${CMAKE_BINARY_DIR}/ground-station/libraries/device/netradio)

endif()
