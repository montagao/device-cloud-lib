#
# Copyright (C) 2017-2018 Wind River Systems, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software  distributed
# under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
# OR CONDITIONS OF ANY KIND, either express or implied.
#

if( MSVC )
	add_definitions( "/D_CRT_SECURE_NO_WARNINGS" )
endif( MSVC )

set_compiler_flag_if_supported(
	"-Wno-bad-function-cast"
	"-Wno-disabled-macro-expansion"
	"-Wno-unused-parameter" )

# Generate test support functions
include_directories( "test-support" )
add_subdirectory( "test-support" )

# Add unit tests
add_subdirectory( "unit" )
