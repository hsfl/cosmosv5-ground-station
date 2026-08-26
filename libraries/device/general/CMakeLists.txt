cmake_minimum_required(VERSION 3.20)

FILE(GLOB INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

# KISS protocol framing (only needs sliplib + configCosmos)
add_library(CosmosKiss STATIC kisslib.cpp kisslib.h)
target_link_libraries(CosmosKiss CosmosSlip CosmosSupport ${COSMOS_LIBS})
install(TARGETS CosmosKiss DESTINATION lib)

# GPIO (Linux only, configCosmos only)
add_library(CosmosGpio STATIC unixgpio.cpp unixgpio.h)
target_link_libraries(CosmosGpio CosmosSupport ${COSMOS_LIBS})
install(TARGETS CosmosGpio DESTINATION lib)

# Serial/KISS TNC devices (depend on cssl_lib)
add_library(CosmosKissTnc STATIC kisstnc_lib.cpp kisstnc_lib.h)
target_link_libraries(CosmosKissTnc CosmosCssl CosmosSupport ${COSMOS_LIBS})
install(TARGETS CosmosKissTnc DESTINATION lib)

add_library(CosmosKpc9612p STATIC kpc9612p_lib.cpp kpc9612p_lib.h)
target_link_libraries(CosmosKpc9612p CosmosCssl CosmosSupport ${COSMOS_LIBS})
install(TARGETS CosmosKpc9612p DESTINATION lib)

add_library(CosmosMixwTnc STATIC mixwtnc_lib.cpp mixwtnc_lib.h)
target_link_libraries(CosmosMixwTnc CosmosCssl CosmosSupport ${COSMOS_LIBS})
install(TARGETS CosmosMixwTnc DESTINATION lib)

add_library(CosmosGs232b STATIC gs232b_lib.cpp gs232b_lib.h)
target_link_libraries(CosmosGs232b CosmosCssl CosmosMath CosmosSupport ${COSMOS_LIBS})
install(TARGETS CosmosGs232b DESTINATION lib)

add_library(CosmosTs2000 STATIC ts2000_lib.cpp ts2000_lib.h)
target_link_libraries(CosmosTs2000 CosmosCssl CosmosSupport ${COSMOS_LIBS})
install(TARGETS CosmosTs2000 DESTINATION lib)

# Serial-based devices (depend on serialclass)
add_library(CosmosIc9100 STATIC ic9100_lib.cpp ic9100_lib.h)
target_link_libraries(CosmosIc9100 CosmosDeviceSerial CosmosSupport ${COSMOS_LIBS})
install(TARGETS CosmosIc9100 DESTINATION lib)

add_library(CosmosPrkx2suClass STATIC prkx2su_class.cpp prkx2su_class.h)
target_link_libraries(CosmosPrkx2suClass CosmosDeviceSerial CosmosMath CosmosSupport ${COSMOS_LIBS})
install(TARGETS CosmosPrkx2suClass DESTINATION lib)

add_library(CosmosPrkx2su STATIC prkx2su_lib.cpp prkx2su_lib.h)
target_link_libraries(CosmosPrkx2su CosmosDeviceSerial CosmosMath CosmosSupport ${COSMOS_LIBS})
install(TARGETS CosmosPrkx2su DESTINATION lib)

# SDR (depends on socketlib + stringlib)
add_library(CosmosUsrp STATIC usrp_lib.cpp usrp_lib.h)
target_link_libraries(CosmosUsrp CosmosNetwork CosmosString CosmosSupport ${COSMOS_LIBS})
install(TARGETS CosmosUsrp DESTINATION lib)

install(FILES ${INCLUDES} DESTINATION include/device/general)

message("Build ground-station device/general from ${CMAKE_CURRENT_SOURCE_DIR}")
