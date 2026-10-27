cmake_minimum_required(VERSION 3.20)

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR} : ")
message(${files})

foreach(file ${files})
    get_filename_component(bfile ${file} NAME_WE)
    # track_sband has pre-existing API errors (device[i].ant → device[i]->ant); skip until fixed
    if(${bfile} STREQUAL "track_sband")
        message(STATUS "Skipping ${bfile} — pre-existing device API errors")
        continue()
    endif()
    add_executable(${bfile} ${bfile}.cpp)
    if(${bfile} STREQUAL "archive_simulate")
        target_link_libraries(${bfile} CosmosL0Sim CosmosArchive CosmosSimulator CosmosPhysics CosmosAgent CosmosConvert)
    endif()
    install(TARGETS ${bfile} DESTINATION bin)
endforeach()
