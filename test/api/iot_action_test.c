/**
 * @file
 * @brief unit testing for IoT library (action source file)
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "test_support.h"

#include "api/public/iot.h"
#include "api/shared/iot_types.h"
#include "iot_build.h"

#include <string.h>

static iot_status_t test_callback_func( iot_action_request_t *request, void *user_data )
{
	assert_non_null( request );
	return mock_type( iot_status_t );
}

static void test_iot_action_allocate_existing( void **state )
{
	size_t i;
	iot_action_t *action;
	iot_t lib;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	strncpy( lib.action_ptr[0]->name, "1action", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[1]->name, "5action", IOT_NAME_MAX_LEN );
	lib.action_count = 2u;
	action = iot_action_allocate( &lib, "3action" );
	assert_ptr_equal( lib.action_ptr[1], action );
	assert_int_equal( lib.action_count, 3u );
	assert_ptr_equal( action->lib, &lib );
	assert_string_equal( action->name, "3action" );
	assert_string_equal( lib.action_ptr[0]->name, "1action" );
	assert_string_equal( lib.action_ptr[2]->name, "5action" );
}

static void test_iot_action_allocate_first( void **state )
{
	size_t i;
	iot_action_t *action;
	iot_t lib;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 0u;
	action = iot_action_allocate( &lib, "someaction" );
	assert_ptr_equal( lib.action_ptr[0], action );
	assert_int_equal( lib.action_count, 1u );
	assert_string_equal( action->name, "someaction" );
	assert_ptr_equal( action->lib, &lib );
}

static void test_iot_action_allocate_full( void **state )
{
	size_t i;
	iot_action_t *action;
	iot_t lib;
	char name[IOT_NAME_MAX_LEN];
	iot_action_t *stack_actions = NULL;

	stack_actions = (iot_action_t *)test_calloc( IOT_ACTION_MAX - IOT_ACTION_STACK_MAX,
	                                             sizeof( iot_action_t ) );
	assert_non_null( stack_actions );

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_MAX; ++i )
	{
		if ( i < IOT_ACTION_STACK_MAX )
			lib.action_ptr[i] = &lib.action[i];
		else
			lib.action_ptr[i] = &stack_actions[i - IOT_ACTION_STACK_MAX];
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "%luaction", i + 1u );
	}
	lib.action_count = IOT_ACTION_MAX;
	action = iot_action_allocate( &lib, "newaction" );
	assert_null( action );
	assert_int_equal( lib.action_count, IOT_ACTION_MAX );
	for ( i = 0u; i < IOT_ACTION_MAX; ++i )
	{
		snprintf( name, IOT_NAME_MAX_LEN, "%luaction", i + 1u );
		assert_string_equal( lib.action_ptr[i]->name, name );
	}
	test_free( stack_actions );
}

static void test_iot_action_allocate_stack_full( void **state )
{
	size_t i;
	iot_action_t *action;
	iot_t lib;
	char name[IOT_NAME_MAX_LEN];

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		lib.action_ptr[i] = &lib.action[i];
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "%luaction", i + 1u );
	}
	lib.action_count = IOT_ACTION_STACK_MAX;
	if ( IOT_ACTION_MAX > IOT_ACTION_STACK_MAX )
		will_return( __wrap_iot_os_heap_malloc, 1u );
	action = iot_action_allocate( &lib, "newaction" );
	if ( IOT_ACTION_MAX > IOT_ACTION_STACK_MAX )
	{
		assert_non_null( action );
		assert_int_equal( lib.action_count, IOT_ACTION_STACK_MAX + 1u );
	}
	else
	{
		assert_null( action );
		assert_int_equal( lib.action_count, IOT_ACTION_STACK_MAX );
	}
	for ( i = 0u; i < IOT_ACTION_STACK_MAX + 1u && i < IOT_ACTION_MAX; ++i )
	{
		if ( i < IOT_ACTION_STACK_MAX )
			snprintf( name, IOT_NAME_MAX_LEN, "%luaction", i + 1u );
		else
			snprintf( name, IOT_NAME_MAX_LEN, "newaction" );
		assert_string_equal( lib.action_ptr[i]->name, name );
	}

	if ( IOT_ACTION_MAX > IOT_ACTION_STACK_MAX )
		test_free( lib.action_ptr[IOT_ACTION_STACK_MAX] );
}

static void test_iot_action_allocate_null_lib( void **state )
{
	iot_action_t *action;
	action = iot_action_allocate( NULL, "newaction" );
	assert_null( action );
}

static void test_iot_action_option_get_not_there( void **state )
{
	iot_action_t action;
	iot_status_t result;
	iot_int32_t data = 0;

	memset( &action, 0u, sizeof( iot_action_t ) );
	strncpy( action.option[0].name, "someoption", IOT_NAME_MAX_LEN );
	action.option[0].data.type = IOT_TYPE_INT32;
	action.option[0].data.value.int32 = 12345;
	action.option[0].data.has_value = IOT_TRUE;
	strncpy( action.option[1].name, "someotheroption", IOT_NAME_MAX_LEN );
	action.option[1].data.type = IOT_TYPE_FLOAT32;
	action.option[1].data.value.float32 = 123.456f;
	action.option[1].data.has_value = IOT_TRUE;
	action.option_count = 2u;
	result =
	    iot_action_option_get( &action, "yetanotheroption", IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	assert_int_equal( data, 0 );
}

static void test_iot_action_option_get_null_action( void **state )
{
	iot_status_t result;
	iot_int32_t data = 0;

	result = iot_action_option_get( NULL, "someoption", IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( data, 0 );
}

static void test_iot_action_option_get_null_name( void **state )
{
	iot_action_t action;
	iot_status_t result;
	iot_int32_t data = 0;

	memset( &action, 0u, sizeof( iot_action_t ) );
	strncpy( action.option[0].name, "someoption", IOT_NAME_MAX_LEN );
	action.option[0].data.type = IOT_TYPE_INT32;
	action.option[0].data.value.int32 = 12345;
	action.option[0].data.has_value = IOT_TRUE;
	action.option_count = 1u;
	result = iot_action_option_get( &action, NULL, IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( data, 0 );
}

static void test_iot_action_option_get_valid( void **state )
{
	iot_action_t action;
	iot_status_t result;
	iot_int32_t data = 0;

	memset( &action, 0u, sizeof( iot_action_t ) );
	strncpy( action.option[0].name, "someoption", IOT_NAME_MAX_LEN );
	action.option[0].data.type = IOT_TYPE_INT32;
	action.option[0].data.value.int32 = 12345;
	action.option[0].data.has_value = IOT_TRUE;
	action.option_count = 1u;
	result = iot_action_option_get( &action, "someoption", IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 12345 );
}

static void test_iot_action_option_set_add( void **state )
{
	iot_action_t action;
	iot_status_t result;

	memset( &action, 0u, sizeof( iot_action_t ) );
	strncpy( action.option[0].name, "someotheroption", IOT_NAME_MAX_LEN );
	action.option_count = 1u;
	result = iot_action_option_set( &action, "someoption", IOT_TYPE_INT8, 35 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( action.option[1].data.value.int8, 35 );
	assert_string_equal( action.option[0].name, "someotheroption" );
	assert_string_equal( action.option[1].name, "someoption" );
	assert_int_equal( action.option_count, 2u );
}

static void test_iot_action_option_set_full( void **state )
{
	size_t i;
	iot_action_t action;
	iot_status_t result;

	memset( &action, 0u, sizeof( iot_action_t ) );
	for ( i = 0u; i < IOT_OPTION_MAX; ++i )
		snprintf( action.option[i].name, IOT_NAME_MAX_LEN, "option%lu", i + 1 );
	action.option_count = IOT_OPTION_MAX;
	result = iot_action_option_set( &action, "someoption", IOT_TYPE_INT8, 35 );
	assert_int_equal( result, IOT_STATUS_FULL );
	for ( i = 0u; i < IOT_OPTION_MAX; ++i )
		assert_string_not_equal( action.option[i].name, "someoption" );
	assert_int_equal( action.option_count, IOT_OPTION_MAX );
}

static void test_iot_action_option_set_null_action( void **state )
{
	iot_status_t result;

	result = iot_action_option_set( NULL, "someoption", IOT_TYPE_INT8, 35 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_option_set_null_data( void **state )
{
	iot_action_t action;
	iot_status_t result;

	memset( &action, 0u, sizeof( iot_action_t ) );
	action.option_count = 0u;
	result = iot_action_option_set( &action, "someoption", IOT_TYPE_NULL, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( action.option_count, 1u );
}

static void test_iot_action_option_set_update( void **state )
{
	iot_action_t action;
	iot_status_t result;

	memset( &action, 0u, sizeof( iot_action_t ) );
	strncpy( action.option[0].name, "someoption", IOT_NAME_MAX_LEN );
	action.option[0].data.type = IOT_TYPE_FLOAT32;
	action.option[0].data.value.float32 = 12.3f;
	action.option[0].data.has_value = IOT_TRUE;
	action.option_count = 1u;
	result = iot_action_option_set( &action, "someoption", IOT_TYPE_INT8, 35 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( action.option[0].data.value.int8, 35 );
	assert_int_equal( action.option[0].data.type, IOT_TYPE_INT8 );
	assert_string_equal( action.option[0].name, "someoption" );
	assert_int_equal( action.option_count, 1u );
}

static void test_iot_action_option_set_raw_add( void **state )
{
	iot_action_t action;
	iot_status_t result;
	char data[20] = "this is text\0";

	memset( &action, 0u, sizeof( iot_action_t ) );
	strncpy( action.option[0].name, "someotheroption", IOT_NAME_MAX_LEN );
	action.option_count = 1u;
	result = iot_action_option_set_raw( &action, "someoption", sizeof( data ), (void *)data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_string_equal( (const char *)action.option[1].data.value.raw.ptr, "this is text" );
	assert_string_equal( action.option[0].name, "someotheroption" );
	assert_string_equal( action.option[1].name, "someoption" );
	assert_int_equal( action.option_count, 2u );
}

static void test_iot_action_deregister_deregistered( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t *action;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
	action->state = IOT_ITEM_DEREGISTERED;
	result = iot_action_deregister( action, NULL, 0u );
	assert_int_equal( action->state, IOT_ITEM_DEREGISTERED );
	assert_int_equal( result, IOT_STATUS_NOT_INITIALIZED );
}

static void test_iot_action_deregister_null_action( void **state )
{
	iot_status_t result;

	result = iot_action_deregister( NULL, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_deregister_null_lib( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t *action;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = NULL;
	action->state = IOT_ITEM_REGISTERED;
	result = iot_action_deregister( action, NULL, 0u );
	assert_int_equal( action->state, IOT_ITEM_REGISTERED );
	assert_int_equal( result, IOT_STATUS_NOT_INITIALIZED );
}

static void test_iot_action_deregister_transmit_fail( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t *action;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
	action->state = IOT_ITEM_REGISTERED;
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_FAILURE );
	result = iot_action_deregister( action, NULL, 0u );
	assert_int_equal( action->state, IOT_ITEM_DEREGISTER_PENDING );
	assert_int_equal( result, IOT_STATUS_FAILURE );
}

static void test_iot_action_deregister_valid( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t *action;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
	action->state = IOT_ITEM_REGISTERED;
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_deregister( action, NULL, 0u );
	assert_int_equal( action->state, IOT_ITEM_DEREGISTERED );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_action_flags_set_null_action( void **state )
{
	iot_status_t result;

	result = iot_action_flags_set( NULL, 5 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_flags_set_valid( void **state )
{
	iot_action_t action;
	iot_status_t result;

	memset( &action, 0u, sizeof( iot_action_t ) );
	result = iot_action_flags_set( &action, 5 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( action.flags, 5 );
}

static void test_iot_action_free_options( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t *action;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 3u;
	strncpy( lib.action_ptr[0]->name, "action 1", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[1]->name, "action 2", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[2]->name, "action 3", IOT_NAME_MAX_LEN );
	action = lib.action_ptr[1];
	action->lib = &lib;
	action->state = IOT_ITEM_REGISTERED;
	action->callback = &test_callback_func;
	action->option_count = 3u;
	strncpy( action->option[0].name, "option 1", IOT_NAME_MAX_LEN );
	action->option[0].data.type = IOT_TYPE_STRING;
	action->option[0].data.heap_storage = test_malloc( sizeof( char ) * IOT_NAME_MAX_LEN );
	action->option[0].data.value.string = (char *)action->option[0].data.heap_storage;
	strncpy(
	    (char *)action->option[0].data.heap_storage, "here is some text", IOT_NAME_MAX_LEN );
	strncpy( action->option[1].name, "option 2", IOT_NAME_MAX_LEN );
	action->option[1].data.type = IOT_TYPE_STRING;
	action->option[1].data.heap_storage = test_malloc( sizeof( char ) * IOT_NAME_MAX_LEN );
	action->option[1].data.value.string = (char *)action->option[1].data.heap_storage;
	strncpy( (char *)action->option[1].data.heap_storage, "some more text", IOT_NAME_MAX_LEN );
	strncpy( action->option[2].name, "option 3", IOT_NAME_MAX_LEN );
	action->option[2].data.type = IOT_TYPE_RAW;
	action->option[2].data.heap_storage = test_malloc( sizeof( char ) * IOT_NAME_MAX_LEN );
	action->option[2].data.value.raw.ptr = action->option[2].data.heap_storage;
	action->option[2].data.value.raw.length = IOT_NAME_MAX_LEN;
	strncpy(
	    (char *)action->option[2].data.heap_storage, "oh look more text", IOT_NAME_MAX_LEN );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_free( action, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.action_count, 2u );
	assert_ptr_equal( lib.action_ptr[0], &lib.action[0] );
	assert_ptr_equal( lib.action_ptr[1], &lib.action[2] );
	assert_ptr_equal( lib.action_ptr[2], &lib.action[1] );
	assert_int_equal( action->state, IOT_ITEM_DEREGISTERED );
}

static void test_iot_action_free_not_found( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t action;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	memset( &action, 0u, sizeof( iot_action_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 3u;
	strncpy( lib.action_ptr[0]->name, "action 1", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[1]->name, "action 2", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[2]->name, "action 3", IOT_NAME_MAX_LEN );
	strncpy( action.name, "action 4", IOT_NAME_MAX_LEN );
	action.lib = &lib;
	action.state = IOT_ITEM_REGISTERED;
	action.callback = &test_callback_func;
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_free( &action, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.action_count, 3u );
	assert_ptr_equal( lib.action_ptr[0], &lib.action[0] );
	assert_ptr_equal( lib.action_ptr[1], &lib.action[1] );
	assert_ptr_equal( lib.action_ptr[2], &lib.action[2] );
	assert_int_equal( action.state, IOT_ITEM_DEREGISTERED );
}

static void test_iot_action_free_null_action( void **state )
{
	iot_status_t result;

	result = iot_action_free( NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_free_null_handle( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t *action;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 3u;
	strncpy( lib.action_ptr[0]->name, "action 1", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[1]->name, "action 2", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[2]->name, "action 3", IOT_NAME_MAX_LEN );
	action = lib.action_ptr[1];
	action->lib = NULL;
	action->state = IOT_ITEM_REGISTERED;
	action->callback = &test_callback_func;
	result = iot_action_free( action, 0u );
	assert_int_equal( result, IOT_STATUS_NOT_INITIALIZED );
	assert_int_equal( lib.action_count, 3u );
	assert_ptr_equal( lib.action_ptr[0], &lib.action[0] );
	assert_ptr_equal( lib.action_ptr[1], &lib.action[1] );
	assert_ptr_equal( lib.action_ptr[2], &lib.action[2] );
}

static void test_iot_action_free_parameters( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t *action;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 3u;
	strncpy( lib.action_ptr[0]->name, "action 1", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[1]->name, "action 2", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[2]->name, "action 3", IOT_NAME_MAX_LEN );
	action = lib.action_ptr[1];
	action->lib = &lib;
	action->state = IOT_ITEM_REGISTERED;
	action->callback = &test_callback_func;
	action->parameter_count = 3u;
	strncpy( action->parameter[0].name, "parameter 1", IOT_NAME_MAX_LEN );
	action->parameter[0].data.type = IOT_TYPE_STRING;
	action->parameter[0].data.heap_storage = test_malloc( sizeof( char ) * IOT_NAME_MAX_LEN );
	action->parameter[0].data.value.string = (char *)action->parameter[0].data.heap_storage;
	strncpy(
	    (char *)action->parameter[0].data.heap_storage, "here is some text", IOT_NAME_MAX_LEN );
	strncpy( action->parameter[1].name, "parameter 2", IOT_NAME_MAX_LEN );
	action->parameter[1].data.type = IOT_TYPE_STRING;
	action->parameter[1].data.heap_storage = test_malloc( sizeof( char ) * IOT_NAME_MAX_LEN );
	action->parameter[1].data.value.string = (char *)action->parameter[1].data.heap_storage;
	strncpy( (char *)action->parameter[1].data.heap_storage, "some more text", IOT_NAME_MAX_LEN );
	strncpy( action->parameter[2].name, "parameter 3", IOT_NAME_MAX_LEN );
	action->parameter[2].data.type = IOT_TYPE_RAW;
	action->parameter[2].data.heap_storage = test_malloc( sizeof( char ) * IOT_NAME_MAX_LEN );
	action->parameter[2].data.value.raw.ptr = action->parameter[2].data.heap_storage;
	action->parameter[2].data.value.raw.length = IOT_NAME_MAX_LEN;
	strncpy(
	    (char *)action->parameter[2].data.heap_storage, "oh look more text", IOT_NAME_MAX_LEN );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_free( action, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.action_count, 2u );
	assert_ptr_equal( lib.action_ptr[0], &lib.action[0] );
	assert_ptr_equal( lib.action_ptr[1], &lib.action[2] );
	assert_ptr_equal( lib.action_ptr[2], &lib.action[1] );
	assert_int_equal( action->state, IOT_ITEM_DEREGISTERED );
}

static void test_iot_action_free_transmit_fail( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t *action;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 3u;
	strncpy( lib.action_ptr[0]->name, "action 1", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[1]->name, "action 2", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[2]->name, "action 3", IOT_NAME_MAX_LEN );
	action = lib.action_ptr[1];
	action->lib = &lib;
	action->state = IOT_ITEM_REGISTERED;
	action->callback = &test_callback_func;
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_FAILURE );
	result = iot_action_free( action, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.action_count, 2u );
	assert_ptr_equal( lib.action_ptr[0], &lib.action[0] );
	assert_ptr_equal( lib.action_ptr[1], &lib.action[2] );
	assert_ptr_equal( lib.action_ptr[2], &lib.action[1] );
	assert_int_equal( action->state, IOT_ITEM_DEREGISTER_PENDING );
}

static void test_iot_action_parameter_add_bad_name( void **state )
{
	iot_status_t result;
	iot_action_t action;
	iot_t lib;

	memset( &action, 0u, sizeof( iot_action_t ) );
	memset( &lib, 0u, sizeof( iot_t ) );
	action.lib = &lib;
	action.parameter_count = 0u;
	result = iot_action_parameter_add(
	    &action, "new\\ | p&ar;a=meter", IOT_PARAMETER_IN, IOT_TYPE_INT32, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
	assert_int_equal( action.parameter_count, 0u );
}

static void test_iot_action_parameter_add_exists( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;
	char name[IOT_NAME_MAX_LEN];

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
	for ( i = 0u; i < IOT_PARAMETER_MAX / 2; i++ )
		snprintf( action->parameter[i].name, IOT_NAME_MAX_LEN, "parameter name %ld", i );
	action->parameter_count = IOT_PARAMETER_MAX / 2;
	result = iot_action_parameter_add(
	    action, "parameter name 1", IOT_PARAMETER_IN, IOT_TYPE_INT32, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
	assert_int_equal( action->parameter_count, IOT_PARAMETER_MAX / 2 );
	for ( i = 0u; i < IOT_PARAMETER_MAX / 2; i++ )
	{
		snprintf( name, IOT_NAME_MAX_LEN, "parameter name %ld", i );
		assert_string_equal( action->parameter[i].name, name );
	}
}

static void test_iot_action_parameter_add_null_action( void **state )
{
	iot_status_t result;
	iot_t lib;

	memset( &lib, 0u, sizeof( iot_t ) );
	result =
	    iot_action_parameter_add( NULL, "new parameter", IOT_PARAMETER_IN, IOT_TYPE_INT32, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_parameter_add_null_name( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
	action->parameter_count = 0u;
	result = iot_action_parameter_add( action, NULL, IOT_PARAMETER_IN, IOT_TYPE_INT32, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( action->parameter_count, 0u );
}

static void test_iot_action_parameter_add_parameters_empty( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
	action->parameter_count = 0u;
	result =
	    iot_action_parameter_add( action, "new parameter", IOT_PARAMETER_IN, IOT_TYPE_INT32, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( action->parameter_count, 1u );
	assert_string_equal( action->parameter[0].name, "new parameter" );
	assert_int_equal( action->parameter[0].data.type, IOT_TYPE_INT32 );
}

static void test_iot_action_parameter_add_parameters_full( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;
	char name[IOT_NAME_MAX_LEN];

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
	for ( i = 0u; i < IOT_PARAMETER_MAX; i++ )
		snprintf( action->parameter[i].name, IOT_NAME_MAX_LEN, "parameter name %ld", i );
	action->parameter_count = IOT_PARAMETER_MAX;
	result =
	    iot_action_parameter_add( action, "new parameter", IOT_PARAMETER_IN, IOT_TYPE_INT32, 0u );
	assert_int_equal( result, IOT_STATUS_FULL );
	assert_int_equal( action->parameter_count, IOT_PARAMETER_MAX );
	for ( i = 0u; i < IOT_PARAMETER_MAX; i++ )
	{
		snprintf( name, IOT_NAME_MAX_LEN, "parameter name %ld", i );
		assert_string_equal( action->parameter[i].name, name );
	}
}

static void test_iot_action_parameter_add_parameters_half_full( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;
	char name[IOT_NAME_MAX_LEN];

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
	for ( i = 0u; i < IOT_PARAMETER_MAX / 2; i++ )
		snprintf( action->parameter[i].name, IOT_NAME_MAX_LEN, "parameter name %ld", i );
	action->parameter_count = IOT_PARAMETER_MAX / 2;
	result =
	    iot_action_parameter_add( action, "new parameter", IOT_PARAMETER_IN, IOT_TYPE_INT32, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( action->parameter_count, IOT_PARAMETER_MAX / 2 + 1 );
	for ( i = 0u; i < IOT_PARAMETER_MAX / 2; i++ )
	{
		snprintf( name, IOT_NAME_MAX_LEN, "parameter name %ld", i );
		assert_string_equal( action->parameter[i].name, name );
	}
	assert_string_equal( action->parameter[IOT_PARAMETER_MAX / 2].name, "new parameter" );
	assert_int_equal( action->parameter[IOT_PARAMETER_MAX / 2].data.type, IOT_TYPE_INT32 );
}

static void test_iot_action_parameter_get_not_found( void **state )
{
	iot_status_t result;
	iot_action_request_t request;
	iot_uint8_t arg;

	arg = 0u;
	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.value.uint8 = 4u;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_UINT8;
	request.parameter[1].type = IOT_PARAMETER_IN;
	result = iot_action_parameter_get( &request, "param3", IOT_FALSE, IOT_TYPE_UINT8, &arg );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	assert_int_equal( arg, 0u );
}

static void test_iot_action_parameter_get_no_parameters( void **state )
{
	iot_status_t result;
	iot_action_request_t request;
	iot_uint8_t arg;

	arg = 0u;
	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 0u;
	result = iot_action_parameter_get( &request, "param2", IOT_FALSE, IOT_TYPE_UINT8, &arg );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	assert_int_equal( arg, 0u );
}

static void test_iot_action_parameter_get_null_name( void **state )
{
	iot_status_t result;
	iot_action_request_t request;
	iot_uint8_t arg;

	arg = 0u;
	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.value.uint8 = 4u;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_UINT8;
	request.parameter[1].type = IOT_PARAMETER_IN;
	result = iot_action_parameter_get( &request, NULL, IOT_FALSE, IOT_TYPE_UINT8, &arg );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( arg, 0u );
}

static void test_iot_action_parameter_get_null_request( void **state )
{
	iot_status_t result;
	iot_uint8_t arg;

	arg = 0u;
	result = iot_action_parameter_get( NULL, "param2", IOT_FALSE, IOT_TYPE_UINT8, &arg );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( arg, 0u );
}

static void test_iot_action_parameter_get_valid( void **state )
{
	iot_status_t result;
	iot_action_request_t request;
	iot_uint8_t arg;

	arg = 0u;
	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.value.uint8 = 4u;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_UINT8;
	request.parameter[1].type = IOT_PARAMETER_IN;
	result = iot_action_parameter_get( &request, "param2", IOT_FALSE, IOT_TYPE_UINT8, &arg );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( arg, 4u );
}

static void test_iot_action_parameter_get_raw_null_data( void **state )
{
	iot_status_t result;
	size_t length;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	length = 0u;
	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.value.raw.ptr = (const void *)data;
	request.parameter[1].data.value.raw.length = 10u;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_RAW;
	request.parameter[1].type = IOT_PARAMETER_IN;
	result = iot_action_parameter_get_raw( &request, "param2", IOT_FALSE, &length, NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( length, 0u );
}

static void test_iot_action_parameter_get_raw_null_length( void **state )
{
	iot_status_t result;
	iot_action_request_t request;
	const void *arg;
	const char data[10] = "some text\0";

	arg = NULL;
	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.value.raw.ptr = (const void *)data;
	request.parameter[1].data.value.raw.length = 10u;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_RAW;
	request.parameter[1].type = IOT_PARAMETER_IN;
	result = iot_action_parameter_get_raw( &request, "param2", IOT_FALSE, NULL, &arg );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_string_equal( (const char *)arg, "some text" );
}

static void test_iot_action_parameter_get_raw_valid( void **state )
{
	iot_status_t result;
	size_t length;
	iot_action_request_t request;
	const void *arg;
	const char data[10] = "some text\0";

	arg = NULL;
	length = 0u;
	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.value.raw.ptr = (const void *)data;
	request.parameter[1].data.value.raw.length = 10u;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_RAW;
	request.parameter[1].type = IOT_PARAMETER_IN;
	result = iot_action_parameter_get_raw( &request, "param2", IOT_FALSE, &length, &arg );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( length, 10u );
	assert_string_equal( (const char *)arg, "some text" );
}

static void test_iot_action_parameter_set_bad_name( void **state )
{
	iot_status_t result;
	iot_action_request_t request;

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	result = iot_action_parameter_set( &request, "param\\3", IOT_TYPE_UINT16, 13u );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
	assert_int_equal( request.parameter_count, 2u );
}

static void test_iot_action_parameter_set_max_parameters( void **state )
{
	iot_status_t result;
	iot_action_request_t request;

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 7u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[2].name, "param3", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[3].name, "param4", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[4].name, "param5", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[5].name, "param6", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[6].name, "param7", IOT_NAME_MAX_LEN );
	result = iot_action_parameter_set( &request, "param8", IOT_TYPE_UINT16, 13u );
	assert_int_equal( result, IOT_STATUS_FULL );
	assert_int_equal( request.parameter_count, 7u );
}

static void test_iot_action_parameter_set_new_parameter( void **state )
{
	iot_status_t result;
	iot_action_request_t request;

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	result = iot_action_parameter_set( &request, "param3", IOT_TYPE_UINT16, 13u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( request.parameter[2].data.value.uint16, 13u );
	assert_int_equal( request.parameter_count, 3u );
	assert_true( request.parameter[2].data.has_value == IOT_TRUE );
	assert_true( ( request.parameter[2].type & IOT_PARAMETER_OUT ) != 0u );
}

static void test_iot_action_parameter_set_null_name( void **state )
{
	iot_status_t result;
	iot_action_request_t request;

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	result = iot_action_parameter_set( &request, NULL, IOT_TYPE_UINT16, 13u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_parameter_set_null_request( void **state )
{
	iot_status_t result;

	result = iot_action_parameter_set( NULL, "param2", IOT_TYPE_UINT16, 13u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_parameter_set_type_null( void **state )
{
	iot_status_t result;
	iot_action_request_t request;

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.has_value = IOT_FALSE;
	request.parameter[1].data.type = IOT_TYPE_NULL;
	result = iot_action_parameter_set( &request, "param2", IOT_TYPE_UINT16, 13u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( request.parameter[1].data.type, IOT_TYPE_UINT16 );
	assert_int_equal( request.parameter[1].data.value.uint16, 13u );
	assert_true( request.parameter[1].data.has_value == IOT_TRUE );
	assert_true( ( request.parameter[1].type & IOT_PARAMETER_OUT ) != 0 );
}

static void test_iot_action_parameter_set_type_wrong( void **state )
{
	iot_status_t result;
	iot_action_request_t request;
	const iot_int32_t data = 1234;

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.has_value = IOT_FALSE;
	request.parameter[1].data.type = IOT_TYPE_UINT16;
	result = iot_action_parameter_set( &request, "param2", IOT_TYPE_INT32, data );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
	assert_int_not_equal( request.parameter[1].data.value.int32, data );
	assert_true( request.parameter[1].data.has_value == IOT_FALSE );
	assert_true( ( request.parameter[1].type & IOT_PARAMETER_OUT ) == 0u );
}

static void test_iot_action_parameter_set_valid( void **state )
{
	iot_status_t result;
	iot_action_request_t request;

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.has_value = IOT_FALSE;
	request.parameter[1].data.type = IOT_TYPE_UINT16;
	result = iot_action_parameter_set( &request, "param2", IOT_TYPE_UINT16, 13u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( request.parameter[1].data.value.uint16, 13u );
	assert_true( request.parameter[1].data.has_value == IOT_TRUE );
	assert_true( ( request.parameter[1].type & IOT_PARAMETER_OUT ) != 0u );
}

static void test_iot_action_parameter_set_raw_max_parameters( void **state )
{
	iot_status_t result;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 7u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[2].name, "param3", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[3].name, "param4", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[4].name, "param5", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[5].name, "param6", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[6].name, "param7", IOT_NAME_MAX_LEN );
	result = iot_action_parameter_set_raw( &request, "param8", 10u, (const void *)data );
	assert_int_equal( result, IOT_STATUS_FULL );
	assert_int_equal( request.parameter_count, 7u );
}

static void test_iot_action_parameter_set_raw_new_parameter( void **state )
{
	iot_status_t result;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	result = iot_action_parameter_set_raw( &request, "param3", 10u, (const void *)data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_string_equal( (const char *)request.parameter[2].data.value.raw.ptr, "some text" );
	assert_int_equal( request.parameter[2].data.value.raw.length, 10u );
	assert_int_equal( request.parameter_count, 3u );
	assert_true( request.parameter[2].data.has_value == IOT_TRUE );
	assert_true( ( request.parameter[2].type & IOT_PARAMETER_OUT ) != 0u );
}

static void test_iot_action_parameter_set_raw_null_data( void **state )
{
	iot_status_t result;
	iot_action_request_t request;

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.has_value = IOT_FALSE;
	request.parameter[1].data.type = IOT_TYPE_RAW;
	result = iot_action_parameter_set_raw( &request, "param2", 10u, NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_parameter_set_raw_null_name( void **state )
{
	iot_status_t result;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	result = iot_action_parameter_set_raw( &request, NULL, 10u, (const void *)data );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_parameter_set_raw_null_request( void **state )
{
	iot_status_t result;
	const char data[10] = "some text\0";

	result = iot_action_parameter_set_raw( NULL, "param2", 10u, (const void *)data );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_parameter_set_raw_type_null( void **state )
{
	iot_status_t result;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.has_value = IOT_FALSE;
	request.parameter[1].data.type = IOT_TYPE_NULL;
	result = iot_action_parameter_set_raw( &request, "param2", 10u, (const void *)data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_string_equal( (const char *)request.parameter[1].data.value.raw.ptr, "some text" );
	assert_int_equal( request.parameter[1].data.value.raw.length, 10u );
	assert_true( request.parameter[1].data.has_value == IOT_TRUE );
	assert_true( ( request.parameter[1].type & IOT_PARAMETER_OUT ) != 0u );
}

static void test_iot_action_parameter_set_raw_type_wrong( void **state )
{
	iot_status_t result;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.has_value = IOT_FALSE;
	request.parameter[1].data.type = IOT_TYPE_UINT8;
	result = iot_action_parameter_set_raw( &request, "param2", 10u, (const void *)data );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
	assert_true( request.parameter[1].data.has_value == IOT_FALSE );
	assert_true( ( request.parameter[1].type & IOT_PARAMETER_OUT ) == 0u );
}

static void test_iot_action_parameter_set_raw_valid( void **state )
{
	iot_status_t result;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.has_value = IOT_FALSE;
	request.parameter[1].data.type = IOT_TYPE_RAW;
	result = iot_action_parameter_set_raw( &request, "param2", 10u, (const void *)data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_string_equal( (const char *)request.parameter[1].data.value.raw.ptr, "some text" );
	assert_int_equal( request.parameter[1].data.value.raw.length, 10u );
	assert_true( request.parameter[1].data.has_value == IOT_TRUE );
	assert_true( ( request.parameter[1].type & IOT_PARAMETER_OUT ) != 0u );
}

static void test_iot_action_process_actions_empty( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 0u;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	will_return( __wrap_iot_error, "Not Found" );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_NOT_FOUND );
	will_return( __wrap_iot_protocol_transmit, 0u );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_actions_full( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = IOT_ACTION_STACK_MAX;
	for ( i = 0u; i < lib.action_count; i++ )
	{
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "action name %ld", i );
		lib.action_ptr[i]->lib = &lib;
		lib.action_ptr[i]->callback = &test_callback_func;
	}
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	snprintf( lib.request_queue_wait[0]->name,
	          IOT_NAME_MAX_LEN,
	          "action name %d",
	          IOT_ACTION_STACK_MAX / 2 );
	will_return( test_callback_func, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, 0u );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_actions_not_found( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = IOT_ACTION_STACK_MAX - 1u;
	for ( i = 1u; i < lib.action_count; i++ )
	{
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "action name %ld", i );
		lib.action_ptr[i]->lib = &lib;
		lib.action_ptr[i]->callback = &test_callback_func;
	}
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	will_return( __wrap_iot_error, "Not Found" );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_NOT_FOUND );
	will_return( __wrap_iot_protocol_transmit, 0u );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_options( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = IOT_ACTION_STACK_MAX - 1u;
	for ( i = 0u; i < lib.action_count; i++ )
	{
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "action name %ld", i );
		lib.action_ptr[i]->lib = &lib;
		lib.action_ptr[i]->callback = &test_callback_func;
	}
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name 1", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->option_count = 1u;
	strncpy( lib.request_queue_wait[0]->option[0].name, "attr", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->option[0].data.heap_storage =
	    test_malloc( ( IOT_NAME_MAX_LEN + 1 ) * sizeof( char ) );
	lib.request_queue_wait[0]->option[0].data.value.string =
	    (char *)lib.request_queue_wait[0]->option[0].data.heap_storage;
	strncpy( (char *)lib.request_queue_wait[0]->option[0].data.heap_storage,
	         "some text",
	         IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->option[0].data.type = IOT_TYPE_STRING;
	will_return( test_callback_func, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, 0u );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_command_no_return( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->flags = IOT_ACTION_NO_RETURN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	expect_string( __wrap_iot_os_system_run, command, "script_path" );
	will_return( __wrap_iot_os_system_run, 0u );
	will_return( __wrap_iot_os_system_run, IOT_STATUS_INVOKED );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_INVOKED );
	will_return( __wrap_iot_protocol_transmit, 0u );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_command_parameter_bool( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;
	iot_int32_t script_return = 0;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter_count = 1u;
	strncpy( lib.action_ptr[0]->parameter[0].name, "bool", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[0].data.type = IOT_TYPE_BOOL;
	lib.action_ptr[0]->parameter[0].type = IOT_PARAMETER_IN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 1u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "bool", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_BOOL;
	lib.request_queue_wait[0]->parameter[0].data.value.boolean = IOT_TRUE;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	expect_string( __wrap_iot_os_system_run, command, "script_path --bool=1" );
	will_return( __wrap_iot_os_system_run, 0u );
	will_return( __wrap_iot_os_system_run, "this is stdout" );
	will_return( __wrap_iot_os_system_run, "this is stderr" );
	will_return( __wrap_iot_os_system_run, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, 4u );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_INT32 );
	will_return( __wrap_iot_protocol_transmit, &script_return );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stdout" );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stderr" );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_command_parameter_float( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;
	iot_int32_t script_return = 0;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter_count = 2u;
	strncpy( lib.action_ptr[0]->parameter[0].name, "float32", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[0].data.type = IOT_TYPE_FLOAT32;
	lib.action_ptr[0]->parameter[0].type = IOT_PARAMETER_IN;
	strncpy( lib.action_ptr[0]->parameter[1].name, "float64", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[1].data.type = IOT_TYPE_FLOAT64;
	lib.action_ptr[0]->parameter[1].type = IOT_PARAMETER_IN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 2u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "float32", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_FLOAT32;
	lib.request_queue_wait[0]->parameter[0].data.value.float32 = 32.32f;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	strncpy( lib.request_queue_wait[0]->parameter[1].name, "float64", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[1].data.type = IOT_TYPE_FLOAT64;
	lib.request_queue_wait[0]->parameter[1].data.value.float64 = 64.64;
	lib.request_queue_wait[0]->parameter[1].data.has_value = IOT_TRUE;
	expect_string(
	    __wrap_iot_os_system_run, command, "script_path --float32=32.320000 --float64=64.640000" );
	will_return( __wrap_iot_os_system_run, 0u );
	will_return( __wrap_iot_os_system_run, "this is stdout" );
	will_return( __wrap_iot_os_system_run, "this is stderr" );
	will_return( __wrap_iot_os_system_run, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, 5u );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_INT32 );
	will_return( __wrap_iot_protocol_transmit, &script_return );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stdout" );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stderr" );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_command_parameter_int( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;
	iot_int32_t script_return = 0;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter_count = 4u;
	strncpy( lib.action_ptr[0]->parameter[0].name, "int8", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[0].data.type = IOT_TYPE_INT8;
	lib.action_ptr[0]->parameter[0].type = IOT_PARAMETER_IN;
	strncpy( lib.action_ptr[0]->parameter[1].name, "int16", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[1].data.type = IOT_TYPE_INT16;
	lib.action_ptr[0]->parameter[1].type = IOT_PARAMETER_IN;
	strncpy( lib.action_ptr[0]->parameter[2].name, "int32", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[2].data.type = IOT_TYPE_INT32;
	lib.action_ptr[0]->parameter[2].type = IOT_PARAMETER_IN;
	strncpy( lib.action_ptr[0]->parameter[3].name, "int64", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[3].data.type = IOT_TYPE_INT64;
	lib.action_ptr[0]->parameter[3].type = IOT_PARAMETER_IN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 4u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "int8", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_INT8;
	lib.request_queue_wait[0]->parameter[0].data.value.int8 = 8;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	strncpy( lib.request_queue_wait[0]->parameter[1].name, "int16", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[1].data.type = IOT_TYPE_INT16;
	lib.request_queue_wait[0]->parameter[1].data.value.int16 = 16;
	lib.request_queue_wait[0]->parameter[1].data.has_value = IOT_TRUE;
	strncpy( lib.request_queue_wait[0]->parameter[2].name, "int32", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[2].data.type = IOT_TYPE_INT32;
	lib.request_queue_wait[0]->parameter[2].data.value.int32 = 32;
	lib.request_queue_wait[0]->parameter[2].data.has_value = IOT_TRUE;
	strncpy( lib.request_queue_wait[0]->parameter[3].name, "int64", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[3].data.type = IOT_TYPE_INT64;
	lib.request_queue_wait[0]->parameter[3].data.value.int64 = 64;
	lib.request_queue_wait[0]->parameter[3].data.has_value = IOT_TRUE;
	expect_string( __wrap_iot_os_system_run,
	               command,
	               "script_path --int8=8 --int16=16 --int32=32 --int64=64" );
	will_return( __wrap_iot_os_system_run, 0u );
	will_return( __wrap_iot_os_system_run, "this is stdout" );
	will_return( __wrap_iot_os_system_run, "this is stderr" );
	will_return( __wrap_iot_os_system_run, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, 7u );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_INT32 );
	will_return( __wrap_iot_protocol_transmit, &script_return );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stdout" );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stderr" );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_command_parameter_location( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;
	iot_int32_t script_return = 0;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter_count = 1u;
	strncpy( lib.action_ptr[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[0].data.type = IOT_TYPE_LOCATION;
	lib.action_ptr[0]->parameter[0].type = IOT_PARAMETER_IN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 1u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_LOCATION;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	/* NOTE: should probably not add the --param= part. */
	expect_string( __wrap_iot_os_system_run, command, "script_path --param=" );
	will_return( __wrap_iot_os_system_run, 0u );
	will_return( __wrap_iot_os_system_run, "this is stdout" );
	will_return( __wrap_iot_os_system_run, "this is stderr" );
	will_return( __wrap_iot_os_system_run, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, 4u );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_INT32 );
	will_return( __wrap_iot_protocol_transmit, &script_return );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stdout" );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stderr" );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_command_parameter_null( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;
	iot_int32_t script_return = 0;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter_count = 1u;
	strncpy( lib.action_ptr[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[0].data.type = IOT_TYPE_NULL;
	lib.action_ptr[0]->parameter[0].type = IOT_PARAMETER_IN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 1u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_NULL;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	/* NOTE: should probably not add the --param= part. */
	expect_string( __wrap_iot_os_system_run, command, "script_path --param=" );
	will_return( __wrap_iot_os_system_run, 0u );
	will_return( __wrap_iot_os_system_run, "this is stdout" );
	will_return( __wrap_iot_os_system_run, "this is stderr" );
	will_return( __wrap_iot_os_system_run, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, 4u );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_INT32 );
	will_return( __wrap_iot_protocol_transmit, &script_return );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stdout" );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stderr" );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_command_parameter_raw( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;
	iot_int32_t script_return = 0;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter_count = 1u;
	strncpy( lib.action_ptr[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[0].data.type = IOT_TYPE_RAW;
	lib.action_ptr[0]->parameter[0].type = IOT_PARAMETER_IN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 1u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_RAW;
	lib.request_queue_wait[0]->parameter[0].data.heap_storage = test_malloc( sizeof( char ) * 25 );
	lib.request_queue_wait[0]->parameter[0].data.value.raw.ptr =
	    lib.request_queue_wait[0]->parameter[0].data.heap_storage;
	strncpy(
	    (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage, "raw data value", 25 );
	lib.request_queue_wait[0]->parameter[0].data.value.raw.length = 14u;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	will_return( __wrap_iot_base64_encode, 8u );
	expect_string( __wrap_iot_os_system_run, command, "script_path --param=bbbbbbbb" );
	will_return( __wrap_iot_os_system_run, 0u );
	will_return( __wrap_iot_os_system_run, "this is stdout" );
	will_return( __wrap_iot_os_system_run, "this is stderr" );
	will_return( __wrap_iot_os_system_run, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, 4u );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_INT32 );
	will_return( __wrap_iot_protocol_transmit, &script_return );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stdout" );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stderr" );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_command_parameter_string( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;
	iot_int32_t script_return = 0;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter_count = 1u;
	strncpy( lib.action_ptr[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[0].data.type = IOT_TYPE_STRING;
	lib.action_ptr[0]->parameter[0].type = IOT_PARAMETER_IN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 1u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_STRING;
	lib.request_queue_wait[0]->parameter[0].data.heap_storage = test_malloc( sizeof( char ) * 25 );
	lib.request_queue_wait[0]->parameter[0].data.value.string =
	    (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage;
	strncpy( (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage,
	         "string\r\n \\ \"value\"",
	         25 );
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	expect_string(
	    __wrap_iot_os_system_run, command, "script_path --param=\"string \\\\ \\\"value\\\"\"" );
	will_return( __wrap_iot_os_system_run, 0u );
	will_return( __wrap_iot_os_system_run, "this is stdout" );
	will_return( __wrap_iot_os_system_run, "this is stderr" );
	will_return( __wrap_iot_os_system_run, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, 4u );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_INT32 );
	will_return( __wrap_iot_protocol_transmit, &script_return );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stdout" );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stderr" );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_command_parameter_uint( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;
	iot_int32_t script_return = 0;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter_count = 4u;
	strncpy( lib.action_ptr[0]->parameter[0].name, "uint8", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[0].data.type = IOT_TYPE_UINT8;
	lib.action_ptr[0]->parameter[0].type = IOT_PARAMETER_IN;
	strncpy( lib.action_ptr[0]->parameter[1].name, "uint16", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[1].data.type = IOT_TYPE_UINT16;
	lib.action_ptr[0]->parameter[1].type = IOT_PARAMETER_IN;
	strncpy( lib.action_ptr[0]->parameter[2].name, "uint32", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[2].data.type = IOT_TYPE_UINT32;
	lib.action_ptr[0]->parameter[2].type = IOT_PARAMETER_IN;
	strncpy( lib.action_ptr[0]->parameter[3].name, "uint64", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[3].data.type = IOT_TYPE_UINT64;
	lib.action_ptr[0]->parameter[3].type = IOT_PARAMETER_IN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 4u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "uint8", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_UINT8;
	lib.request_queue_wait[0]->parameter[0].data.value.uint8 = 8u;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	strncpy( lib.request_queue_wait[0]->parameter[1].name, "uint16", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[1].data.type = IOT_TYPE_UINT16;
	lib.request_queue_wait[0]->parameter[1].data.value.uint16 = 16u;
	lib.request_queue_wait[0]->parameter[1].data.has_value = IOT_TRUE;
	strncpy( lib.request_queue_wait[0]->parameter[2].name, "uint32", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[2].data.type = IOT_TYPE_UINT32;
	lib.request_queue_wait[0]->parameter[2].data.value.uint32 = 32u;
	lib.request_queue_wait[0]->parameter[2].data.has_value = IOT_TRUE;
	strncpy( lib.request_queue_wait[0]->parameter[3].name, "uint64", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[3].data.type = IOT_TYPE_UINT64;
	lib.request_queue_wait[0]->parameter[3].data.value.uint64 = 64u;
	lib.request_queue_wait[0]->parameter[3].data.has_value = IOT_TRUE;
	expect_string( __wrap_iot_os_system_run,
	               command,
	               "script_path --uint8=8 --uint16=16 --uint32=32 --uint64=64" );
	will_return( __wrap_iot_os_system_run, 0u );
	will_return( __wrap_iot_os_system_run, "this is stdout" );
	will_return( __wrap_iot_os_system_run, "this is stderr" );
	will_return( __wrap_iot_os_system_run, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, 7u );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_INT32 );
	will_return( __wrap_iot_protocol_transmit, &script_return );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stdout" );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stderr" );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_command_script_return_fail( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;
	iot_int32_t script_return = 1;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	expect_string( __wrap_iot_os_system_run, command, "script_path" );
	will_return( __wrap_iot_os_system_run, 1u );
	will_return( __wrap_iot_os_system_run, "this is stdout" );
	will_return( __wrap_iot_os_system_run, "this is stderr" );
	will_return( __wrap_iot_os_system_run, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_EXECUTION_ERROR );
	will_return( __wrap_iot_protocol_transmit, 3u );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_INT32 );
	will_return( __wrap_iot_protocol_transmit, &script_return );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stdout" );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stderr" );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_command_system_run_fail( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	expect_string( __wrap_iot_os_system_run, command, "script_path" );
	will_return( __wrap_iot_os_system_run, -1 );
	will_return( __wrap_iot_os_system_run, "\0" );
	will_return( __wrap_iot_os_system_run, "\0" );
	will_return( __wrap_iot_os_system_run, IOT_STATUS_NOT_EXECUTABLE );
	will_return( __wrap_iot_error, "Not Executable" );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_NOT_EXECUTABLE );
	will_return( __wrap_iot_protocol_transmit, 0u );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_command_valid( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;
	iot_int32_t script_return = 0;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	expect_string( __wrap_iot_os_system_run, command, "script_path" );
	will_return( __wrap_iot_os_system_run, 0u );
	will_return( __wrap_iot_os_system_run, "this is stdout" );
	will_return( __wrap_iot_os_system_run, "this is stderr" );
	will_return( __wrap_iot_os_system_run, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_os_heap_realloc, 1 );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, 3u );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_INT32 );
	will_return( __wrap_iot_protocol_transmit, &script_return );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stdout" );
	will_return( __wrap_iot_protocol_transmit, IOT_TYPE_STRING );
	will_return( __wrap_iot_protocol_transmit, "this is stderr" );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_exclusive( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = &test_callback_func;
	lib.action_ptr[0]->flags = IOT_ACTION_EXCLUSIVE_APP;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	will_return( test_callback_func, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, 0u );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_lib_to_quit( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.to_quit = IOT_TRUE;
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = &test_callback_func;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_NOT_FOUND );
	will_return( __wrap_iot_protocol_transmit, 0u );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_no_handler( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	*lib.action_ptr[0]->command = '\0';
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_FAILURE );
	will_return( __wrap_iot_protocol_transmit, 0u );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_null_lib( void **state )
{
	iot_status_t result;

	result = iot_action_process( NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_process_parameters_bad_type( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = IOT_ACTION_STACK_MAX - 1u;
	for ( i = 0u; i < lib.action_count; i++ )
	{
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "action name %lu", i );
		lib.action_ptr[i]->lib = &lib;
		lib.action_ptr[i]->callback = &test_callback_func;
	}
	lib.action_ptr[1]->parameter_count = 1u;
	strncpy( lib.action_ptr[1]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.action_ptr[1]->parameter[0].type = IOT_PARAMETER_IN_REQUIRED;
	lib.action_ptr[1]->parameter[0].data.type = IOT_TYPE_INT32;
	lib.action_ptr[1]->parameter[0].data.has_value = IOT_FALSE;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name 1", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 1u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.heap_storage =
	    test_malloc( ( IOT_NAME_MAX_LEN + 1 ) * sizeof( char ) );
	lib.request_queue_wait[0]->parameter[0].data.value.string =
	    (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage;
	strncpy( (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage,
	         "some text",
	         IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_STRING;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_BAD_REQUEST );
	will_return( __wrap_iot_protocol_transmit, 1u );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_parameters_missing_required( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = IOT_ACTION_STACK_MAX - 1u;
	for ( i = 0u; i < lib.action_count; i++ )
	{
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "action name %ld", i );
		lib.action_ptr[i]->lib = &lib;
		lib.action_ptr[i]->callback = &test_callback_func;
		lib.action_ptr[i]->parameter_count = 0u;
	}
	lib.action_ptr[1]->parameter_count = 1u;
	strncpy( lib.action_ptr[1]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.action_ptr[1]->parameter[0].type = IOT_PARAMETER_IN_REQUIRED;
	lib.action_ptr[1]->parameter[0].data.type = IOT_TYPE_STRING;
	lib.action_ptr[1]->parameter[0].data.has_value = IOT_FALSE;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name 1", IOT_NAME_MAX_LEN );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_BAD_REQUEST );
	will_return( __wrap_iot_protocol_transmit, 0u );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_parameters_undeclared( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = IOT_ACTION_STACK_MAX - 1u;
	for ( i = 0u; i < lib.action_count; i++ )
	{
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "action name %ld", i );
		lib.action_ptr[i]->lib = &lib;
		lib.action_ptr[i]->callback = &test_callback_func;
	}
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name 1", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 1u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.heap_storage =
	    test_malloc( ( IOT_NAME_MAX_LEN + 1 ) * sizeof( char ) );
	lib.request_queue_wait[0]->parameter[0].data.value.string =
	    (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage;
	strncpy( (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage,
	         "some text",
	         IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_STRING;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_BAD_REQUEST );
	will_return( __wrap_iot_protocol_transmit, 1u );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_parameters_valid( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = IOT_ACTION_STACK_MAX - 1u;
	for ( i = 0u; i < lib.action_count; i++ )
	{
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "action name %lu", i );
		lib.action_ptr[i]->lib = &lib;
		lib.action_ptr[i]->callback = &test_callback_func;
	}
	lib.action_ptr[1]->parameter_count = 1u;
	strncpy( lib.action_ptr[1]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.action_ptr[1]->parameter[0].type = IOT_PARAMETER_IN_REQUIRED;
	lib.action_ptr[1]->parameter[0].data.type = IOT_TYPE_STRING;
	lib.action_ptr[1]->parameter[0].data.has_value = IOT_FALSE;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name 1", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 1u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.heap_storage =
	    test_malloc( ( IOT_NAME_MAX_LEN + 1 ) * sizeof( char ) );
	lib.request_queue_wait[0]->parameter[0].data.value.string =
	    (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage;
	strncpy( (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage,
	         "some text",
	         IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_STRING;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	will_return( test_callback_func, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, 1u );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_valid( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = &test_callback_func;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	will_return( test_callback_func, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, 0u );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_wait_queue_empty( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = &test_callback_func;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 0u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 0u;
	lib.request_queue_free_count = 0u;
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_wait_queue_full( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = &test_callback_func;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 0u; i < IOT_ACTION_QUEUE_MAX; i++ )
		lib.request_queue_wait[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = IOT_ACTION_QUEUE_MAX;
	lib.request_queue_free_count = IOT_ACTION_QUEUE_MAX;
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; i++ )
		snprintf( lib.request_queue_wait[i]->name, IOT_NAME_MAX_LEN, "other action %ld", i );
	will_return( test_callback_func, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_protocol_transmit, 0u );
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, IOT_ACTION_QUEUE_MAX - 1 );
	assert_int_equal( lib.request_queue_free_count, IOT_ACTION_QUEUE_MAX - 1 );
}

static void test_iot_action_register_callback_null_action( void **state )
{
	iot_status_t result;
	char data[10] = "some text\0";

	result = iot_action_register_callback( NULL, &test_callback_func, (void *)data, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_register_callback_null_lib( void **state )
{
	iot_status_t result;
	iot_action_t action;
	char data[10] = "some text\0";

	memset( &action, 0u, sizeof( iot_action_t ) );
	action.state = IOT_ITEM_DEREGISTERED;
	action.lib = NULL;
	result = iot_action_register_callback( &action, &test_callback_func, (void *)data, NULL, 0u );
	assert_int_equal( action.state, IOT_ITEM_DEREGISTERED );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_non_null( action.user_data );
	assert_ptr_equal( action.user_data, data );
	assert_ptr_equal( action.callback, &test_callback_func );
}

static void test_iot_action_register_callback_transmit_fail( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;
	char data[10] = "some text\0";

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
	action->state = IOT_ITEM_DEREGISTERED;
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_FAILURE );
	result = iot_action_register_callback( action, &test_callback_func, (void *)data, NULL, 0u );
	assert_int_equal( action->state, IOT_ITEM_REGISTER_PENDING );
	assert_int_equal( result, IOT_STATUS_FAILURE );
	assert_non_null( action->user_data );
	assert_ptr_equal( action->user_data, data );
	assert_ptr_equal( action->callback, &test_callback_func );
}

static void test_iot_action_register_callback_valid( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;
	char data[10] = "some text\0";

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
	action->state = IOT_ITEM_DEREGISTERED;
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_register_callback( action, &test_callback_func, (void *)data, NULL, 0u );
	assert_int_equal( action->state, IOT_ITEM_REGISTERED );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( action->user_data );
	assert_ptr_equal( action->user_data, data );
	assert_ptr_equal( action->callback, &test_callback_func );
}

static void test_iot_action_register_command_null_action( void **state )
{
	iot_status_t result;

	result = iot_action_register_command( NULL, "script_path", NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_register_command_null_lib( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->state = IOT_ITEM_DEREGISTERED;
	action->lib = NULL;
	result = iot_action_register_command( action, "script_path", NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( action->state, IOT_ITEM_DEREGISTERED );
	assert_null( action->user_data );
	assert_null( action->callback );
	assert_string_equal( action->command, "script_path" );
}

static void test_iot_action_register_command_transmit_fail( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->state = IOT_ITEM_DEREGISTERED;
	action->lib = &lib;
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_FAILURE );
	result = iot_action_register_command( action, "script_path", NULL, 0u );
	assert_int_equal( result, IOT_STATUS_FAILURE );
	assert_int_equal( action->state, IOT_ITEM_REGISTER_PENDING );
	assert_null( action->user_data );
	assert_null( action->callback );
	assert_string_equal( action->command, "script_path" );
}

static void test_iot_action_register_command_valid( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;

	memset( &lib, 0u, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; i++ )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->state = IOT_ITEM_DEREGISTERED;
	action->lib = &lib;
	will_return( __wrap_iot_protocol_transmit, IOT_STATUS_SUCCESS );
	result = iot_action_register_command( action, "script_path", NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( action->state, IOT_ITEM_REGISTERED );
	assert_null( action->user_data );
	assert_null( action->callback );
	assert_string_equal( action->command, "script_path" );
}

static void test_iot_action_request_copy_raw( void **state )
{
	iot_action_request_t dest;
	iot_action_request_t src;
	iot_status_t result;
	char data1[10] = "some text\0";
	char data2[15] = "even more text\0";
	void *data_dest = test_malloc( sizeof( char ) * 25 );

	memset( data_dest, 0u, sizeof( char ) * 25 );
	memset( &dest, 0u, sizeof( iot_action_request_t ) );
	memset( &src, 0u, sizeof( iot_action_request_t ) );
	strncpy( src.name, "thisisarequest", IOT_NAME_MAX_LEN );
	src.parameter_count = 3u;
	src.parameter[0].data.has_value = IOT_FALSE;
	src.parameter[1].data.value.raw.ptr = (void *)data1;
	src.parameter[1].data.value.raw.length = 10u;
	src.parameter[1].data.has_value = IOT_TRUE;
	src.parameter[1].data.type = IOT_TYPE_RAW;
	src.parameter[2].data.value.raw.ptr = (void *)data2;
	src.parameter[2].data.value.raw.length = 15u;
	src.parameter[2].data.has_value = IOT_TRUE;
	src.parameter[2].data.type = IOT_TYPE_RAW;
	result = iot_action_request_copy( &dest, &src, data_dest, 25u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_string_equal( dest.name, "thisisarequest" );
	assert_string_equal( (const char *)dest.parameter[1].data.value.raw.ptr, "some text" );
	assert_string_equal( (const char *)dest.parameter[2].data.value.raw.ptr, "even more text" );
	assert_int_equal( dest.parameter[1].data.value.raw.length, 10u );
	assert_int_equal( dest.parameter[2].data.value.raw.length, 15u );
	test_free( data_dest );
}

static void test_iot_action_request_copy_raw_too_big( void **state )
{
	iot_action_request_t dest;
	iot_action_request_t src;
	iot_status_t result;
	char data1[50] = "gaerioivonerilmtgruiesnchyuiolhtrsthgherhskgownvd\0";
	char data2[50] = "srcshkltbhmirshltilhtnkgfjkhsurthlesghrgjsehrgeor\0";
	char data3[50] = "tyrjeiojtgiohjgidhgfkldjhgkguqiwueifgaehthrshkger\0";
	char data4[50] = "rjeklarieoghirovhieaojruiehafuileghuielghaurilgre\0";
	char data5[50] = "vbtiroangrheaugjrkeanrgjhkealgurheagrtejhnaihruei\0";
	char data6[50] = "grteksxdvfhwjbyrheuknguitrmnuyhtnirsuibgrseukgrfe\0";
	void *data_dest = test_malloc( sizeof( char ) * 200 );

	memset( &dest, 0u, sizeof( iot_action_request_t ) );
	memset( &src, 0u, sizeof( iot_action_request_t ) );
	memset( data_dest, 0u, sizeof( char ) * 200 );
	strncpy( src.name, "thisisarequest", IOT_NAME_MAX_LEN );
	src.parameter_count = 6u;
	src.parameter[0].data.value.raw.ptr = (void *)data1;
	src.parameter[0].data.value.raw.length = 50u;
	src.parameter[0].data.has_value = IOT_TRUE;
	src.parameter[0].data.type = IOT_TYPE_RAW;
	src.parameter[1].data.value.raw.ptr = (void *)data2;
	src.parameter[1].data.value.raw.length = 50u;
	src.parameter[1].data.has_value = IOT_TRUE;
	src.parameter[1].data.type = IOT_TYPE_RAW;
	src.parameter[2].data.value.raw.ptr = (void *)data3;
	src.parameter[2].data.value.raw.length = 50u;
	src.parameter[2].data.has_value = IOT_TRUE;
	src.parameter[2].data.type = IOT_TYPE_RAW;
	src.parameter[3].data.value.raw.ptr = (void *)data4;
	src.parameter[3].data.value.raw.length = 50u;
	src.parameter[3].data.has_value = IOT_TRUE;
	src.parameter[3].data.type = IOT_TYPE_RAW;
	src.parameter[4].data.value.raw.ptr = (void *)data5;
	src.parameter[4].data.value.raw.length = 50u;
	src.parameter[4].data.has_value = IOT_TRUE;
	src.parameter[4].data.type = IOT_TYPE_RAW;
	src.parameter[5].data.value.raw.ptr = (void *)data6;
	src.parameter[5].data.value.raw.length = 50u;
	src.parameter[5].data.has_value = IOT_TRUE;
	src.parameter[5].data.type = IOT_TYPE_RAW;
	result = iot_action_request_copy( &dest, &src, (void *)data_dest, 200u );
	assert_int_equal( result, IOT_STATUS_NO_MEMORY );
	assert_string_equal( dest.name, "thisisarequest" );
	test_free( data_dest );
}

static void test_iot_action_request_copy_string( void **state )
{
	iot_action_request_t dest;
	iot_action_request_t src;
	iot_status_t result;
	char data1[10] = "some text\0";
	char data2[15] = "even more text\0";
	void *data_dest = test_malloc( sizeof( char ) * 25 );

	memset( data_dest, 0u, sizeof( char ) * 25 );
	memset( &dest, 0u, sizeof( iot_action_request_t ) );
	memset( &src, 0u, sizeof( iot_action_request_t ) );
	strncpy( src.name, "thisisarequest", IOT_NAME_MAX_LEN );
	src.parameter_count = 3u;
	src.parameter[0].data.has_value = IOT_FALSE;
	src.parameter[1].data.value.string = data1;
	src.parameter[1].data.has_value = IOT_TRUE;
	src.parameter[1].data.type = IOT_TYPE_STRING;
	src.parameter[2].data.value.string = data2;
	src.parameter[2].data.has_value = IOT_TRUE;
	src.parameter[2].data.type = IOT_TYPE_STRING;
	result = iot_action_request_copy( &dest, &src, data_dest, 25u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_string_equal( dest.name, "thisisarequest" );
	assert_string_equal( dest.parameter[1].data.value.string, "some text" );
	assert_string_equal( dest.parameter[2].data.value.string, "even more text" );
	test_free( data_dest );
}

static void test_iot_action_request_copy_string_too_big( void **state )
{
	iot_action_request_t dest;
	iot_action_request_t src;
	iot_status_t result;
	char data1[50] = "gaerioivonerilmtgruiesnchyuiolhtrsthgherhskgownvd\0";
	char data2[50] = "srcshkltbhmirshltilhtnkgfjkhsurthlesghrgjsehrgeor\0";
	char data3[50] = "tyrjeiojtgiohjgidhgfkldjhgkguqiwueifgaehthrshkger\0";
	char data4[50] = "rjeklarieoghirovhieaojruiehafuileghuielghaurilgre\0";
	char data5[50] = "vbtiroangrheaugjrkeanrgjhkealgurheagrtejhnaihruei\0";
	char data6[50] = "grteksxdvfhwjbyrheuknguitrmnuyhtnirsuibgrseukgrfe\0";
	void *data_dest = test_malloc( sizeof( char ) * 200 );

	memset( &dest, 0u, sizeof( iot_action_request_t ) );
	memset( &src, 0u, sizeof( iot_action_request_t ) );
	memset( data_dest, 0u, sizeof( char ) * 200 );
	strncpy( src.name, "thisisarequest", IOT_NAME_MAX_LEN );
	src.parameter_count = 6u;
	src.parameter[0].data.value.string = data1;
	src.parameter[0].data.has_value = IOT_TRUE;
	src.parameter[0].data.type = IOT_TYPE_STRING;
	src.parameter[1].data.value.string = data2;
	src.parameter[1].data.has_value = IOT_TRUE;
	src.parameter[1].data.type = IOT_TYPE_STRING;
	src.parameter[2].data.value.string = data3;
	src.parameter[2].data.has_value = IOT_TRUE;
	src.parameter[2].data.type = IOT_TYPE_STRING;
	src.parameter[3].data.value.string = data4;
	src.parameter[3].data.has_value = IOT_TRUE;
	src.parameter[3].data.type = IOT_TYPE_STRING;
	src.parameter[4].data.value.string = data5;
	src.parameter[4].data.has_value = IOT_TRUE;
	src.parameter[4].data.type = IOT_TYPE_STRING;
	src.parameter[5].data.value.string = data6;
	src.parameter[5].data.has_value = IOT_TRUE;
	src.parameter[5].data.type = IOT_TYPE_STRING;
	result = iot_action_request_copy( &dest, &src, (void *)data_dest, 200u );
	assert_int_equal( result, IOT_STATUS_NO_MEMORY );
	assert_string_equal( dest.name, "thisisarequest" );
	test_free( data_dest );
}

static void test_iot_action_request_copy_size_no_parameters( void **state )
{
	size_t size;
	iot_action_request_t request;

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 0u;
	size = iot_action_request_copy_size( &request );
	assert_int_equal( size, 0u );
}

static void test_iot_action_request_copy_size_no_pointers( void **state )
{
	size_t size;
	iot_action_request_t request;

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	request.parameter[0].data.value.uint8 = 4u;
	request.parameter[0].data.has_value = IOT_TRUE;
	request.parameter[0].data.type = IOT_TYPE_UINT8;
	request.parameter[1].data.value.float32 = 12.34f;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_FLOAT32;
	size = iot_action_request_copy_size( &request );
	assert_int_equal( size, 0u );
}

static void test_iot_action_request_copy_size_no_values( void **state )
{
	size_t size;
	iot_action_request_t request;

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	request.parameter[0].data.has_value = IOT_FALSE;
	request.parameter[1].data.has_value = IOT_FALSE;
	size = iot_action_request_copy_size( &request );
	assert_int_equal( size, 0u );
}

static void test_iot_action_request_copy_size_null_request( void **state )
{
	size_t size;

	size = iot_action_request_copy_size( NULL );
	assert_int_equal( size, 0u );
}

static void test_iot_action_request_copy_size_raw( void **state )
{
	size_t size;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	request.parameter[0].data.has_value = IOT_FALSE;
	request.parameter[1].data.value.raw.ptr = (const void *)data;
	request.parameter[1].data.value.raw.length = 10u;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_RAW;
	size = iot_action_request_copy_size( &request );
	assert_int_equal( size, 10u );
}

static void test_iot_action_request_copy_size_string( void **state )
{
	size_t size;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	memset( &request, 0u, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
	request.parameter[0].data.has_value = IOT_FALSE;
	request.parameter[1].data.value.string = data;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_STRING;
	size = iot_action_request_copy_size( &request );
	assert_int_equal( size, 10u );
}

int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_iot_action_allocate_existing ),
		cmocka_unit_test( test_iot_action_allocate_first ),
		cmocka_unit_test( test_iot_action_allocate_full ),
		cmocka_unit_test( test_iot_action_allocate_stack_full ),
		cmocka_unit_test( test_iot_action_allocate_null_lib ),
		cmocka_unit_test( test_iot_action_option_get_not_there ),
		cmocka_unit_test( test_iot_action_option_get_null_action ),
		cmocka_unit_test( test_iot_action_option_get_null_name ),
		cmocka_unit_test( test_iot_action_option_get_valid ),
		cmocka_unit_test( test_iot_action_option_set_add ),
		cmocka_unit_test( test_iot_action_option_set_full ),
		cmocka_unit_test( test_iot_action_option_set_null_action ),
		cmocka_unit_test( test_iot_action_option_set_null_data ),
		cmocka_unit_test( test_iot_action_option_set_update ),
		cmocka_unit_test( test_iot_action_option_set_raw_add ),
		cmocka_unit_test( test_iot_action_deregister_deregistered ),
		cmocka_unit_test( test_iot_action_deregister_null_action ),
		cmocka_unit_test( test_iot_action_deregister_null_lib ),
		cmocka_unit_test( test_iot_action_deregister_transmit_fail ),
		cmocka_unit_test( test_iot_action_deregister_valid ),
		cmocka_unit_test( test_iot_action_flags_set_null_action ),
		cmocka_unit_test( test_iot_action_flags_set_valid ),
		cmocka_unit_test( test_iot_action_free_options ),
		cmocka_unit_test( test_iot_action_free_not_found ),
		cmocka_unit_test( test_iot_action_free_null_action ),
		cmocka_unit_test( test_iot_action_free_null_handle ),
		cmocka_unit_test( test_iot_action_free_parameters ),
		cmocka_unit_test( test_iot_action_free_transmit_fail ),
		cmocka_unit_test( test_iot_action_parameter_add_bad_name ),
		cmocka_unit_test( test_iot_action_parameter_add_exists ),
		cmocka_unit_test( test_iot_action_parameter_add_null_action ),
		cmocka_unit_test( test_iot_action_parameter_add_null_name ),
		cmocka_unit_test( test_iot_action_parameter_add_parameters_empty ),
		cmocka_unit_test( test_iot_action_parameter_add_parameters_full ),
		cmocka_unit_test( test_iot_action_parameter_add_parameters_half_full ),
		cmocka_unit_test( test_iot_action_parameter_get_not_found ),
		cmocka_unit_test( test_iot_action_parameter_get_no_parameters ),
		cmocka_unit_test( test_iot_action_parameter_get_null_name ),
		cmocka_unit_test( test_iot_action_parameter_get_null_request ),
		cmocka_unit_test( test_iot_action_parameter_get_valid ),
		cmocka_unit_test( test_iot_action_parameter_get_raw_null_data ),
		cmocka_unit_test( test_iot_action_parameter_get_raw_null_length ),
		cmocka_unit_test( test_iot_action_parameter_get_raw_valid ),
		cmocka_unit_test( test_iot_action_parameter_set_bad_name ),
		cmocka_unit_test( test_iot_action_parameter_set_max_parameters ),
		cmocka_unit_test( test_iot_action_parameter_set_new_parameter ),
		cmocka_unit_test( test_iot_action_parameter_set_null_name ),
		cmocka_unit_test( test_iot_action_parameter_set_null_request ),
		cmocka_unit_test( test_iot_action_parameter_set_type_null ),
		cmocka_unit_test( test_iot_action_parameter_set_type_wrong ),
		cmocka_unit_test( test_iot_action_parameter_set_valid ),
		cmocka_unit_test( test_iot_action_parameter_set_raw_max_parameters ),
		cmocka_unit_test( test_iot_action_parameter_set_raw_new_parameter ),
		cmocka_unit_test( test_iot_action_parameter_set_raw_null_name ),
		cmocka_unit_test( test_iot_action_parameter_set_raw_null_request ),
		cmocka_unit_test( test_iot_action_parameter_set_raw_type_null ),
		cmocka_unit_test( test_iot_action_parameter_set_raw_type_wrong ),
		cmocka_unit_test( test_iot_action_parameter_set_raw_valid ),
		cmocka_unit_test( test_iot_action_process_actions_empty ),
		cmocka_unit_test( test_iot_action_process_actions_full ),
		cmocka_unit_test( test_iot_action_process_actions_not_found ),
		cmocka_unit_test( test_iot_action_process_options ),
		cmocka_unit_test( test_iot_action_process_command_no_return ),
		cmocka_unit_test( test_iot_action_process_command_parameter_bool ),
		cmocka_unit_test( test_iot_action_process_command_parameter_float ),
		cmocka_unit_test( test_iot_action_process_command_parameter_int ),
		cmocka_unit_test( test_iot_action_process_command_parameter_location ),
		cmocka_unit_test( test_iot_action_process_command_parameter_null ),
		cmocka_unit_test( test_iot_action_process_command_parameter_raw ),
		cmocka_unit_test( test_iot_action_process_command_parameter_string ),
		cmocka_unit_test( test_iot_action_process_command_parameter_uint ),
		cmocka_unit_test( test_iot_action_process_command_script_return_fail ),
		cmocka_unit_test( test_iot_action_process_command_system_run_fail ),
		cmocka_unit_test( test_iot_action_process_command_valid ),
		cmocka_unit_test( test_iot_action_process_exclusive ),
		cmocka_unit_test( test_iot_action_process_lib_to_quit ),
		cmocka_unit_test( test_iot_action_process_no_handler ),
		cmocka_unit_test( test_iot_action_process_null_lib ),
		cmocka_unit_test( test_iot_action_process_parameters_bad_type ),
		cmocka_unit_test( test_iot_action_process_parameters_missing_required ),
		cmocka_unit_test( test_iot_action_process_parameters_undeclared ),
		cmocka_unit_test( test_iot_action_process_parameters_valid ),
		cmocka_unit_test( test_iot_action_process_valid ),
		cmocka_unit_test( test_iot_action_process_wait_queue_empty ),
		cmocka_unit_test( test_iot_action_process_wait_queue_full ),
		cmocka_unit_test( test_iot_action_register_callback_null_action ),
		cmocka_unit_test( test_iot_action_register_callback_null_lib ),
		cmocka_unit_test( test_iot_action_register_callback_transmit_fail ),
		cmocka_unit_test( test_iot_action_register_callback_valid ),
		cmocka_unit_test( test_iot_action_register_command_null_action ),
		cmocka_unit_test( test_iot_action_register_command_null_lib ),
		cmocka_unit_test( test_iot_action_register_command_transmit_fail ),
		cmocka_unit_test( test_iot_action_register_command_valid ),
		cmocka_unit_test( test_iot_action_request_copy_raw ),
		cmocka_unit_test( test_iot_action_request_copy_raw_too_big ),
		cmocka_unit_test( test_iot_action_request_copy_string ),
		cmocka_unit_test( test_iot_action_request_copy_string_too_big ),
		cmocka_unit_test( test_iot_action_request_copy_size_no_parameters ),
		cmocka_unit_test( test_iot_action_request_copy_size_no_pointers ),
		cmocka_unit_test( test_iot_action_request_copy_size_no_values ),
		cmocka_unit_test( test_iot_action_request_copy_size_null_request ),
		cmocka_unit_test( test_iot_action_request_copy_size_raw ),
		cmocka_unit_test( test_iot_action_request_copy_size_string )
	};
	MOCK_SYSTEM_ENABLED = 1;
	result = cmocka_run_group_tests( tests, NULL, NULL );
	MOCK_SYSTEM_ENABLED = 0;
	return result;
}
