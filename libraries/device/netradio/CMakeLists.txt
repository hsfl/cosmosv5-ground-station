cmake_minimum_required(VERSION 3.20)

FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
FILE(GLOB INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(CosmosNetRadio STATIC ${SOURCES} ${INCLUDES})
target_link_libraries(CosmosNetRadio CosmosPacket CosmosNetwork CosmosSupport ${COSMOS_LIBS})

install(TARGETS CosmosNetRadio DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include/device/netradio)

message("Build CosmosNetRadio from ${CMAKE_CURRENT_SOURCE_DIR}")
