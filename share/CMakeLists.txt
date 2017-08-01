#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

include( CMakeParseArguments )
#
# EXAMPLE_NAME     name of the example
# ...              source files for the example
# DESCRIPTION   ...  description of the example
# DISPLAY_NAME  ... (optional) name of the example is within installer
# README        ... (optional) readme file for the example
# SCRIPTS       ... (optional) script files for the example
# FILES          ... (optional) file for the example
macro( ADD_SAMPLE_FILES EXAMPLE_NAME )
	cmake_parse_arguments( APP "" "DESCRIPTION;DISPLAY_NAME"
		"README;SCRIPTS;FILES"
		${ARGN} )
	string( REPLACE " " "_" _EXAMPLE_NAME "${EXAMPLE_NAME}" )
	set( APP_COMPONENT "${_EXAMPLE_NAME}" )
	# Component names can only be [a-z][A-Z][0-9] or '_'
	string( REGEX REPLACE "[^a-zA-Z0-9_]" "_" APP_COMPONENT "${APP_COMPONENT}" )
	string( TOUPPER "${APP_COMPONENT}" APP_COMPONENT_UPPER )

	set( IOT_APPS ${IOT_APPS} "${APP_COMPONENT}" )

	# Set optional attributes for the sample application component
	if ( APP_DESCRIPTION )
		set( IOT_APP_${APP_COMPONENT_UPPER}_DESCRIPTION "${APP_DESCRIPTION}" CACHE
			INTERNAL "Description for ${EXAMPLE_NAME}" FORCE )
	endif ( APP_DESCRIPTION )
	if ( APP_DISPLAY_NAME )
		set( IOT_APP_${APP_COMPONENT_UPPER}_DISPLAY_NAME "${APP_DISPLAY_NAME}" CACHE
			INTERNAL "Name to use for ${EXAMPLE_NAME}" FORCE )
	endif ( APP_DISPLAY_NAME )

	# Add example source files
	set( DEST_TEXT_FILES "${OUT_MAKE_FILE}" ${APP_README} ${APP_FILES} )
	if ( DEST_TEXT_FILES )
		install( FILES ${DEST_TEXT_FILES}
			DESTINATION "${INSTALL_EXAMPLES_DIR}/${_EXAMPLE_NAME}"
			COMPONENT "${APP_COMPONENT}"
		)
	endif( DEST_TEXT_FILES )

	# Add any required script files to sources
	set( APP_SCRIPTS ${APP_SCRIPTS} ${APP_SCRIPTS} )
	set( DEST_SCRIPT_FILES ${APP_SCRIPTS} )
	if ( DEST_SCRIPT_FILES )
		install( PROGRAMS ${DEST_SCRIPT_FILES}
			DESTINATION "${INSTALL_EXAMPLES_DIR}/${_EXAMPLE_NAME}"
			COMPONENT "${APP_COMPONENT}"
		)
	endif( DEST_SCRIPT_FILES )

	# Add example files to installation
	if ( OUT_TARGETS OR APP_SCRIPTS )
		install( PROGRAMS ${OUT_TARGETS} ${APP_SCRIPTS}
			DESTINATION "${INSTALL_BIN_DIR}"
			COMPONENT "${APP_COMPONENT}"
		)
	endif ( OUT_TARGETS OR APP_SCRIPTS )
endmacro( ADD_SAMPLE_FILES )

# Subdirectories containing example files
add_subdirectory( "apps" )
#add_subdirectory( "ota-examples" )

# Update the list of example applications to the parent CMakeLists.txt file
# (this is used for packaging)
#set( IOT_APPS ${IOT_APPS} PARENT_SCOPE )
#install_set_variables( PARENT_SCOPE )
