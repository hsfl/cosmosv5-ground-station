cmake_minimum_required(VERSION 3.20)

FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
FILE(GLOB INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

# Simulated science data: archived imaging tiles to L0 ENVI files (task 004 pass 2)
add_library(CosmosL0Sim STATIC ${SOURCES} ${INCLUDES})
target_link_libraries(CosmosL0Sim CosmosArchive CosmosAgent CosmosConvert CosmosDem CosmosEnvi CosmosData CosmosSupport ${COSMOS_LIBS})

install(TARGETS CosmosL0Sim DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include/science)
