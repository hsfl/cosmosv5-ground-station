cmake_minimum_required(VERSION 3.20)

FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
FILE(GLOB INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

# The SOSE archive: paths, names, listing, and the hourly JSON lines writer
add_library(CosmosArchive STATIC ${SOURCES} ${INCLUDES})
target_link_libraries(CosmosArchive CosmosData CosmosTime CosmosSupport ${COSMOS_LIBS})

install(TARGETS CosmosArchive DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include/archive)
