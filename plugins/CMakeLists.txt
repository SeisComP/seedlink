INCLUDE_DIRECTORIES(../libs/slutils)

SET(
	_DIR_LIST
		mseedfifo_plugin
		chain_plugin
)

IF ( NOT MACOSX )
	SET(
		_DIR_LIST 
			${_DIR_LIST}
			caps_plugin
			echopro_plugin
			ewexport_plugin
			ewexport_pasv_plugin
			fs_plugin
			minilogger_plugin
			mseedscan_plugin
			naqsplugin
			optodas_plugin
			q330plugin
			reftek_plugin
			scream_plugin
			scream2_plugin
			serial_plugin
			sockplugin
			wave24_plugin
			win_plugin
	) 
ENDIF ( NOT MACOSX )

IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../common)
	SET(_DIR_LIST ${_DIR_LIST} gdrt_plugin)
ENDIF()

SET(SEEDLINK_PLUGIN_OUTPUT_DIR ${SC3_PACKAGE_SHARE_DIR}/plugins/seedlink)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
INSTALL(FILES plugin.py DESTINATION ${SEEDLINK_PLUGIN_OUTPUT_DIR})

SUBDIRS(${_DIR_LIST})
