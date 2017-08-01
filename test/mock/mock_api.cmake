#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

set( MOCK_API_LIBS "mock_api" )

set( MOCK_API_FUNC
	"iot_action_process"
	"iot_action_free"
	"iot_base64_encode"
	"iot_base64_encode_size"
	"iot_error"
	"iot_log"
	"iot_protocol"
	"iot_log"
	"iot_plugin_perform"
	"iot_plugin_builtin_load"
	"iot_plugin_builtin_enable"
	"iot_plugin_disable_all"
	"iot_plugin_enable"
	"iot_plugin_initialize"
	"iot_plugin_terminate"
	"iot_telemetry_free"

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

