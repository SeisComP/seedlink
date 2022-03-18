CHECK_FUNCTION_EXISTS(cfmakeraw HAVE_CFMAKERAW)
IF(HAVE_CFMAKERAW)
        MESSAGE(STATUS "Found function cfmakeraw")
        ADD_DEFINITIONS(-DHAVE_CFMAKERAW)
ENDIF(HAVE_CFMAKERAW)

SET(SERIALPLUGIN_SOURCES
	serial_plugin.cc
	rw2300.c
	linux2300.c
	proto_dr24.cc
	proto_edata.cc
	proto_edata2.cc
	proto_hrd24.cc
	proto_mws.cc
	proto_prema.cc
	proto_sadc.cc
	proto_solar.cc
	proto_vaisala.cc
	proto_ws2300.cc
	proto_modbus.cc
	proto_miscString.cc
	proto_miscSerial.cc
	proto_miscScript.cc
	proto_gmeteo.cc
)

INCLUDE_DIRECTORIES(../../libs/plugin)
INCLUDE_DIRECTORIES(../../libs/slutils)

SET(CONFIG_FILE ${SC3_PACKAGE_INSTALL_PREFIX}/acquisition/config/plugins.ini)

SET(OPTIONS -DSYSLOG_FACILITY=LOG_LOCAL0 -DCONFIG_FILE="${CONFIG_FILE}")
SET(WSOPTIONS -DVERSION="1.10")

ADD_DEFINITIONS(${OPTIONS} ${WSOPTIONS})

ADD_EXECUTABLE(serial_plugin ${SERIALPLUGIN_SOURCES})

TARGET_LINK_LIBRARIES(
	serial_plugin
		slplugin
		slutils
		pthread
)

INSTALL(TARGETS serial_plugin RUNTIME DESTINATION ${SEEDLINK_PLUGIN_OUTPUT_DIR})

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
