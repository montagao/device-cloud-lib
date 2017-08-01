#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

set( TARGET "api" )
set( TESTS
#	"iot_action"
	"iot_base"
	"iot_base64"
	"iot_common"
	"iot_json_decode"
	"iot_location"
	"iot_telemetry"
)

include( "mock_api" )
include( "mock_osal" )

#include( "mock_mosquitto" )
#include( "mock_pthread" )
#include( "mock_system" )
#include( "mock_wra_client" )
#include( "mock_wra_ipc" )
#include( "mock_wra_ipc_common" )
#include( "mock_wra_messages" )
#
#set( TELEMETRY_DATA_FUNC
#	"wra_message_command_initialize"
#	"wra_message_command_read"
#	"wra_message_command_read_release"
#	"wra_message_command_response_initialize"
#	"wra_message_command_response_read"
#	"wra_message_command_response_write"
#	"wra_message_command_response_write_size"
#	"wra_message_command_write"
#	"wra_message_command_write_size"
#	"wra_message_connect_initialize"
#	"wra_message_connect_read"
#	"wra_message_connect_read_release"
#	"wra_message_connect_write"
#	"wra_message_connect_write_size"
#	"wra_message_metric_initialize"
#	"wra_message_metric_read"
#	"wra_message_metric_read_release"
#	"wra_message_metric_write"
#	"wra_message_metric_write_size"
#	"wra_message_notification_initialize"
#	"wra_message_notification_read"
#	"wra_message_notification_read_release"
#	"wra_message_notification_write"
#	"wra_message_notification_write_size"
#	"wra_message_sample_initialize"
#	"wra_message_sample_read"
#	"wra_message_sample_read_release"
#	"wra_message_sample_write"
#	"wra_message_sample_write_size"
#)
#
#if ( CMAKE_C_COMPILER_ID MATCHES "(Clang|GNU)" )
#	add_definitions( "-Wno-deprecated-declarations" )
#endif ( CMAKE_C_COMPILER_ID MATCHES "(Clang|GNU)" )
#
#set( MOCK_WRA_CLIENT_CMDS
#	"wra_common_notification_wait"
#	"wra_common_time_relative_to_absolute"
#	"wra_connection_receive"
#	"wra_connection_send"
#	"wra_error"
#	"wra_log"
#	"wra_service_allocate"
#	"wra_service_register"
#)
#
#set( TEST_WRA_COMPAT_MOCK ${MOCK_COMMON_FUNC} ${MOCK_PTHREAD_FUNC} ${MOCK_SYSTEM_FUNC} ${MOCK_WRA_IPC_COMMON_FUNC} ${MOCK_WRA_MESSAGES_FUNC} ${TELEMETRY_DATA_FUNC} )
#set( TEST_WRA_COMPAT_SRCS ${MOCK_COMMON_SRCS} ${MOCK_PTHREAD_SRCS} ${MOCK_SYSTEM_SRCS} ${MOCK_WRA_IPC_COMMON_SRCS} ${MOCK_WRA_MESSAGES_SRCS} "wra_compat_test.c" "wra_test_common.c" )
#set( TEST_WRA_COMPAT_LIBS ${MOCK_COMMON_LIBS} ${MOCK_PTHREAD_LIBS} ${MOCK_SYSTEM_LIBS} ${MOCK_WRA_IPC_COMMON_LIBS} ${MOCK_WRA_MESSAGES_LIBS} )
#set( TEST_WRA_COMPAT_UNIT "wra_command.c" "wra_common.c" "wra_compat.c" "wra_connection.c" "wra_metric.c" "wra_service.c" "wra_source.c" )
#
#set( MOCK_WRA_CLIENT_CMDS
#	"wra_log"
#)
#set( TEST_WRA_CONNECTION_MOCK ${MOCK_WRA_CLIENT_CMDS} ${MOCK_WRA_IPC_COMMON_FUNC} ${MOCK_SYSTEM_FUNC} )
#set( TEST_WRA_CONNECTION_SRCS ${MOCK_WRA_CLIENT_SRCS} ${MOCK_WRA_IPC_COMMON_SRCS} ${MOCK_SYSTEM_SRCS} "wra_connection_test.c" )
#set( TEST_WRA_CONNECTION_LIBS ${MOCK_WRA_CLIENT_LIBS} ${MOCK_WRA_IPC_COMMON_LIBS} ${MOCK_SYSTEM_LIBS} )
#set( TEST_WRA_CONNECTION_UNIT "wra_connection.c" )
#
#set( MOCK_WRA_CLIENT_CMDS
#	"wra_common_notification_wait"
#	"wra_common_time_relative_to_absolute"
#	"wra_connection_receive"
#	"wra_connection_send"
#	"wra_error"
#	"wra_log"
#	"wra_service_allocate"
#	"wra_service_register"
#	"wra_message_command_initialize"
#	"wra_message_command_write_size"
#	"wra_command_request_message_process" 
#	"wra_common_notification_message_process" 
#	"wra_connect_request" 
#	"wra_message_command_initialize" 
#	"wra_message_command_write" 
#	"wra_message_command_write_size" 
#	"wra_message_metric_initialize" 
#	"wra_message_metric_write" 
#	"wra_message_metric_write_size" 
#	"wra_message_notification_initialize" 
#	"wra_message_notification_read" 
#	"wra_message_notification_read_release" 
#	"wra_message_notification_write" 
#	"wra_message_notification_write_size" 
#
#)
#set( TEST_WRA_IPC_MOCK ${MOCK_WRA_CLIENT_CMDS} ${MOCK_WRA_IPC_COMMON_FUNC} ${MOCK_MOSQUITTO_FUNC} ${MOCK_SYSTEM_FUNC} )
#set( TEST_WRA_IPC_SRCS ${MOCK_WRA_CLIENT_SRCS} ${MOCK_WRA_IPC_COMMON_SRCS} ${MOCK_MOSQUITTO_SRCS} ${MOCK_SYSTEM_SRCS} "wra_ipc_test.c" )
#set( TEST_WRA_IPC_LIBS ${MOCK_WRA_CLIENT_LIBS} ${MOCK_WRA_IPC_COMMON_LIBS} ${MOCK_MOSQUITTO_LIBS} ${MOCK_SYSTEM_LIBS} )
#set( TEST_WRA_IPC_UNIT "wra_ipc.c" )
#
#set( TEST_WRA_IPC_COMMON_MOCK ${MOCK_WRA_IPC_FUNC} )
#set( TEST_WRA_IPC_COMMON_SRCS ${MOCK_WRA_IPC_SRCS} "wra_ipc_common_test.c" )
#set( TEST_WRA_IPC_COMMON_LIBS ${MOCK_WRA_IPC_LIBS} )
#set( TEST_WRA_IPC_COMMON_UNIT "wra_ipc_common.c" )
#
#set( TEST_WRA_MESSAGES_MOCK )
#set( TEST_WRA_MESSAGES_SRCS "wra_messages_test.c" )
#set( TEST_WRA_MESSAGES_LIBS )
#set( TEST_WRA_MESSAGES_UNIT "wra_messages.c" "wra_serialization.c" )
#
#set( TEST_WRA_SERIALIZATION_MOCK )
#set( TEST_WRA_SERIALIZATION_SRCS "wra_serialization_test.c" )
#set( TEST_WRA_SERIALIZATION_LIBS )
#set( TEST_WRA_SERIALIZATION_UNIT "wra_serialization.c" )
#
## It is better to use the implementation from wra_command.c for wra_command_register / wra_command_free
#set( MOCK_WRA_CLIENT_CMDS
#	"wra_common_notification_wait"
#	"wra_common_time_relative_to_absolute"
#	"wra_connection_receive"
#	"wra_connection_send"
#	"wra_error"
#	"wra_log"
#)
#set( TEST_WRA_SERVICE_MOCK ${MOCK_WRA_CLIENT_CMDS} ${MOCK_COMMON_FUNC} ${MOCK_SYSTEM_FUNC} ${TELEMETRY_DATA_FUNC} )
#set( TEST_WRA_SERVICE_SRCS ${MOCK_WRA_CLIENT_SRCS} ${MOCK_COMMON_SRCS} ${MOCK_SYSTEM_SRCS} "wra_service_test.c" )
#set( TEST_WRA_SERVICE_LIBS ${MOCK_WRA_CLIENT_LIBS} ${MOCK_COMMON_LIBS} ${MOCK_SYSTEM_LIBS} )
#set( TEST_WRA_SERVICE_UNIT "wra_command.c" "wra_service.c" )
#
#set( TEST_WRA_SOURCE_MOCK ${MOCK_COMMON_FUNC} ${MOCK_PTHREAD_FUNC} ${MOCK_SYSTEM_FUNC} ${MOCK_WRA_IPC_COMMON_FUNC} ${MOCK_WRA_MESSAGES_FUNC} ${TELEMETRY_DATA_FUNC} )
#set( TEST_WRA_SOURCE_SRCS ${MOCK_COMMON_SRCS} ${MOCK_PTHREAD_SRCS} ${MOCK_SYSTEM_SRCS} ${MOCK_WRA_IPC_COMMON_SRCS} ${MOCK_WRA_MESSAGES_SRCS} "wra_source_test.c" "wra_test_common.c" )
#set( TEST_WRA_SOURCE_LIBS ${MOCK_COMMON_LIBS} ${MOCK_PTHREAD_LIBS} ${MOCK_SYSTEM_LIBS} ${MOCK_WRA_IPC_COMMON_LIBS} ${MOCK_WRA_MESSAGES_LIBS} )
#set( TEST_WRA_SOURCE_UNIT "wra_command.c" "wra_common.c" "wra_connection.c" "wra_metric.c" "wra_service.c" "wra_source.c" )
#

set( MOCK_API_PART ${MOCK_API_FUNC} )
list( REMOVE_ITEM MOCK_API_PART
	"iot_action_process"
)
set( TEST_IOT_ACTION_MOCK ${MOCK_API_PART} ${MOCK_OSAL_FUNC} )
set( TEST_IOT_ACTION_SRCS ${MOCK_API_SRCS} ${MOCK_OSAL_SRCS} "iot_action_test.c" )
set( TEST_IOT_ACTION_LIBS ${MOCK_API_LIBS} ${MOCK_OSAL_LIBS} )
set( TEST_IOT_ACTION_UNIT "iot_action.c" "iot_common.c" )

set( MOCK_API_PART ${MOCK_API_FUNC} )
list( REMOVE_ITEM MOCK_API_PART
	"iot_error"
	"iot_log"
)
set( TEST_IOT_BASE_MOCK ${MOCK_API_PART} ${MOCK_OSAL_FUNC} )
set( TEST_IOT_BASE_SRCS ${MOCK_API_SRCS} ${MOCK_OSAL_SRCS} "iot_base_test.c" )
set( TEST_IOT_BASE_LIBS ${MOCK_API_LIBS} ${MOCK_OSAL_LIBS} )
set( TEST_IOT_BASE_UNIT "iot_base.c" "iot_common.c" )

set( TEST_IOT_BASE64_MOCK )
set( TEST_IOT_BASE64_SRCS ${MOCK_API_SRCS} "iot_base64_test.c" )
set( TEST_IOT_BASE64_LIBS ${MOCK_API_LIBS} )
set( TEST_IOT_BASE64_UNIT "iot_base64.c" )

set( TEST_IOT_COMMON_MOCK ${MOCK_API_FUNC} )
set( TEST_IOT_COMMON_SRCS ${MOCK_API_SRCS} "iot_common_test.c" )
set( TEST_IOT_COMMON_LIBS ${MOCK_API_LIBS} )
set( TEST_IOT_COMMON_UNIT "iot_common.c" )

set( MOCK_API_PART ${MOCK_API_FUNC} )
list( REMOVE_ITEM MOCK_API_PART
	"iot_json_decode_array_at"
	"iot_json_decode_array_iterator"
	"iot_json_decode_array_iterator_next"
	"iot_json_decode_array_iterator_value"
	"iot_json_decode_array_size"
	"iot_json_decode_bool"
	"iot_json_decode_initialize"
	"iot_json_decode_integer"
	"iot_json_decode_number"
	"iot_json_decode_object_find"
	"iot_json_decode_object_iterator"
	"iot_json_decode_object_iterator_key"
	"iot_json_decode_object_iterator_next"
	"iot_json_decode_object_iterator_value"
	"iot_json_decode_object_size"
	"iot_json_decode_parse"
	"iot_json_decode_real"
	"iot_json_decode_string"
	"iot_json_decode_terminate"
	"iot_json_decode_type"
)
set( TEST_IOT_JSON_DECODE_MOCK ${MOCK_API_PART} )
set( TEST_IOT_JSON_DECODE_DEFS "${JSON_DEFS}" )
set( TEST_IOT_JSON_DECODE_INCS "${JSON_INCLUDE_DIR}" )
set( TEST_IOT_JSON_DECODE_SRCS ${MOCK_API_SRCS} "iot_json_decode_test.c" )
set( TEST_IOT_JSON_DECODE_LIBS ${MOCK_API_LIBS} ${JSON_LIBRARIES} )
set( TEST_IOT_JSON_DECODE_UNIT "json/iot_json_decode.c" "json/iot_json_base.c" )

set( TEST_IOT_LOCATION_MOCK ${MOCK_API_FUNC} )
set( TEST_IOT_LOCATION_SRCS ${MOCK_API_SRCS} "iot_location_test.c" )
set( TEST_IOT_LOCATION_LIBS ${MOCK_API_LIBS} )
set( TEST_IOT_LOCATION_UNIT "iot_location.c" )

set( MOCK_API_PART ${MOCK_API_FUNC} )
list( REMOVE_ITEM MOCK_API_PART
	"iot_telemetry_free"
)
set( TEST_IOT_TELEMETRY_MOCK ${MOCK_API_PART} ${MOCK_OSAL_FUNC} )
set( TEST_IOT_TELEMETRY_SRCS ${MOCK_API_SRCS} ${MOCK_OSAL_SRCS} "iot_telemetry_test.c" )
set( TEST_IOT_TELEMETRY_LIBS ${MOCK_API_LIBS} ${MOCK_OSAL_LIBS} )
set( TEST_IOT_TELEMETRY_UNIT "iot_telemetry.c" "iot_common.c" )

include( TestSupport )
add_tests( ${TARGET} ${TESTS} )

