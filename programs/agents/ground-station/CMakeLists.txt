cmake_minimum_required(VERSION 3.20)

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR} : ")
message(${files})

foreach(file ${files})
    get_filename_component(bfile ${file} NAME_WE)
    add_executable(${bfile} ${bfile}.cpp)
    install(TARGETS ${bfile} DESTINATION bin)
endforeach()

target_link_libraries(kiss_send CosmosKiss CosmosDeviceSerial CosmosSlip)
target_link_libraries(kpc9612p_recv CosmosTime CosmosKpc9612p)
target_link_libraries(kpc9612p_send CosmosKpc9612p)
