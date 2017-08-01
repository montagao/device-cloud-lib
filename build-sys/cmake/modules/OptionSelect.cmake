#
# Copyright (C) 2015 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

include( CMakeParseArguments )

function( OPTION_SELECT OUTPUT_VAR )
	cmake_parse_arguments( DROPDOWN "" "DESCRIPTION;DEFAULT" "" ${ARGN} )
	set( DROPDOWN_OPTIONS ${DROPDOWN_UNPARSED_ARGUMENTS} )
	if ( DROPDOWN_DEFAULT )
		if ( NOT ";${DROPDOWN_OPTIONS};" MATCHES ";${DROPDOWN_DEFAULT};" )
			set( DROPDOWN_OPTIONS ${DROPDOWN_OPTIONS} ${DROPDOWN_DEFAULT} )
		endif ( NOT ";${DROPDOWN_OPTIONS};" MATCHES ";${DROPDOWN_DEFAULT};" )
	endif ( DROPDOWN_DEFAULT )
	if ( NOT DROPDOWN_DESCRIPTION )
		set( DROPDOWN_DESCRIPTION "Value for ${OUPUT_VAR}" )
	endif ( NOT DROPDOWN_DESCRIPTION )

	if ( NOT ${OUTPUT_VAR} )
		set( ${OUTPUT_VAR} "${DROPDOWN_DEFAULT}" CACHE STRING "${DROPDOWN_DESCRIPTION}" FORCE )
	endif ( NOT ${OUTPUT_VAR} )
	set_property( CACHE ${OUTPUT_VAR} PROPERTY STRINGS ${DROPDOWN_OPTIONS} )

	string( TOUPPER "${DROPDOWN_OPTIONS}" DROPDOWN_OPTIONS_UPPER )
	string( TOUPPER "${${OUTPUT_VAR}}" CURRENT_OPTION_UPPER )
	if ( NOT ";${DROPDOWN_OPTIONS_UPPER};" MATCHES ";${CURRENT_OPTION_UPPER};" )
		string( REPLACE ";" ", " VALID_OPTIONS "${DROPDOWN_OPTIONS}" )
		message( FATAL_ERROR
			"The value of \"${${OUTPUT_VAR}}\" for \"${OUTPUT_VAR}\" is not valid. "
			"Valid values are: ${VALID_OPTIONS}" )
	endif ( NOT ";${DROPDOWN_OPTIONS_UPPER};" MATCHES ";${CURRENT_OPTION_UPPER};" )
endfunction( OPTION_SELECT )

