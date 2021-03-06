/**
 * @file
 * @brief unit testing for IoT library (alarm source file)
 *
 * @copyright Copyright (C) 2017-2018 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied."
 */

#include "test_support.h"

#include "api/public/iot.h"
#include "api/shared/iot_types.h"
#include "iot_build.h"

#include <strings.h> /* for bzero */

static void test_iot_alarm_register_empty( void **state )
{
	size_t i;
	iot_t lib;
	const char *name = "alarm";
	iot_alarm_t *result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_STACK_MAX; i++ )
		lib.alarm_ptr[i] = &lib.alarm[i];
	lib.alarm_count = 0u;
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1 ); /* alarm name */
#endif
	result = iot_alarm_register( &lib, name );
	assert_non_null( result );
	assert_int_equal( lib.alarm_count, 1u );
	assert_ptr_equal( result, lib.alarm_ptr[0] );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( lib.alarm_ptr[0]->name );
#endif
}

static void test_iot_alarm_register_full( void **state )
{
	size_t i;
	iot_t lib;
	char name[IOT_NAME_MAX_LEN + 1u];
	char *t_names;
	iot_alarm_t *result;
	iot_alarm_t *stack_alarm;

	t_names = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) * IOT_ALARM_MAX );
	assert_non_null( t_names );

	stack_alarm = (iot_alarm_t *)test_calloc( IOT_ALARM_MAX - IOT_ALARM_STACK_MAX,
	                                                  sizeof( iot_alarm_t ) );
	assert_non_null( stack_alarm );
	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_MAX; i++ )
	{
		if ( i < IOT_ALARM_STACK_MAX )
			lib.alarm_ptr[i] = &lib.alarm[i];
		else
			lib.alarm_ptr[i] = &stack_alarm[i - IOT_ALARM_STACK_MAX];
		lib.alarm_ptr[i]->name = &t_names[( IOT_NAME_MAX_LEN + 1u ) * i];
		snprintf( lib.alarm_ptr[i]->name, IOT_NAME_MAX_LEN, "alarm %03lu", i );
	}
	snprintf( name, IOT_NAME_MAX_LEN, "alarm %03d.5", IOT_ALARM_MAX / 2u );
	lib.alarm_count = IOT_ALARM_MAX;
	result = iot_alarm_register( &lib, name );
	assert_null( result );
	assert_int_equal( lib.alarm_count, IOT_ALARM_MAX );
	test_free( t_names );
	test_free( stack_alarm );
}

static void test_iot_alarm_register_stack_full( void **state )
{
	size_t i;
	iot_t lib;
	char name[IOT_NAME_MAX_LEN + 1u];
	char *t_names;
	iot_alarm_t *result;

	t_names = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) * IOT_ALARM_STACK_MAX );
	assert_non_null( t_names );
	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_STACK_MAX; i++ )
	{
		lib.alarm_ptr[i] = &lib.alarm[i];
		lib.alarm_ptr[i]->name = &t_names[( IOT_NAME_MAX_LEN + 1u ) * i];
		snprintf( lib.alarm_ptr[i]->name, IOT_NAME_MAX_LEN, "alarm %03lu", i );
	}
	snprintf( name, IOT_NAME_MAX_LEN, "alarm %03d.5", IOT_ALARM_STACK_MAX / 2u );
	lib.alarm_count = IOT_ALARM_STACK_MAX;
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1 ); /* alarm object */
	will_return( __wrap_os_malloc, 1 ); /* alarm name */
#endif
	result = iot_alarm_register( &lib, name );
	if ( IOT_ALARM_MAX > IOT_ALARM_STACK_MAX )
	{
#ifdef IOT_STACK_ONLY
		assert_null( result );
		assert_int_equal( lib.alarm_count, IOT_ALARM_STACK_MAX );
#else
		assert_non_null( result );
		assert_int_equal( lib.alarm_count, IOT_ALARM_STACK_MAX + 1u );
		assert_int_equal( result->is_in_heap, 1 );
		os_free( result->name );
		os_free( result );
#endif
	}
	else
	{
		assert_null( result );
		assert_int_equal( lib.alarm_count, IOT_ALARM_MAX );
	}
	test_free( t_names );
}

static void test_iot_alarm_register_null_lib( void **state )
{
	char name[] = "alarm";
	iot_alarm_t *result = NULL;

	result = iot_alarm_register( NULL, name );
	assert_null( result );
}

static void test_iot_alarm_register_null_name( void **state )
{
	size_t i;
	iot_t lib;
	iot_alarm_t *result = NULL;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_STACK_MAX; i++ )
		lib.alarm_ptr[i] = &lib.alarm[i];
	lib.alarm_count = 0u;
	result = iot_alarm_register( &lib, NULL );
	assert_null( result );
	assert_int_equal( lib.alarm_count, 0u );
}

static void test_iot_alarm_register_no_memory_obj( void **state )
{
	iot_t lib;
	iot_alarm_t *result = NULL;

	bzero( &lib, sizeof( iot_t ) );
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 0u ); /* for new alarm */
#endif
	result = iot_alarm_register( &lib, "new alarm" );
	assert_null( result );
	assert_int_equal( lib.alarm_count, 0u );
}

static void test_iot_alarm_register_no_memory_name( void **state )
{
	iot_t lib;
	iot_alarm_t *result = NULL;

	bzero( &lib, sizeof( iot_t ) );
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1u ); /* for new alarm */
	will_return( __wrap_os_malloc, 0u ); /* for name */
#endif
	result = iot_alarm_register( &lib, "new alarm" );
	assert_null( result );
	assert_int_equal( lib.alarm_count, 0u );
}

static void test_iot_alarm_register_valid( void **state )
{
	size_t i;
	iot_t lib;
	char name[IOT_NAME_MAX_LEN + 2u];
	iot_alarm_t *result = NULL;
	char *t_names;

	t_names = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) * IOT_ALARM_STACK_MAX );
	assert_non_null( t_names );

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_STACK_MAX; i++ )
	{
		lib.alarm_ptr[i] = &lib.alarm[i];
		lib.alarm_ptr[i]->name = &t_names[( IOT_NAME_MAX_LEN + 1u ) * i];
		snprintf( lib.alarm_ptr[i]->name, IOT_NAME_MAX_LEN, "%u alarm %03lu", (unsigned int)(i * 2u), i );
	}
	lib.alarm_count = IOT_ALARM_STACK_MAX - 1u;
	test_generate_random_string( name, IOT_NAME_MAX_LEN + 2u );
	name[0] = (char)(IOT_ALARM_STACK_MAX/2u) + '0' ; /* ensure first letter in middle of list*/
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1 ); /* for alarm message */
#endif
	result = iot_alarm_register( &lib, name );
	assert_non_null( result );
	assert_int_equal( lib.alarm_count, IOT_ALARM_STACK_MAX );
	assert_ptr_equal( result, lib.alarm_ptr[IOT_ALARM_STACK_MAX / 2u] );

	/* clean up */
	test_free( t_names );
#ifndef IOT_STACK_ONLY
	test_free( lib.alarm[IOT_ALARM_STACK_MAX / 2u + 1 ].name );
#endif
}

static void test_iot_alarm_deregister_null_alarm( void **state )
{
	iot_status_t result;
	result = iot_alarm_deregister( NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_alarm_deregister_null_lib( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_alarm_t *alarm;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_STACK_MAX; i++ )
		lib.alarm_ptr[i] = &lib.alarm[i];
	lib.alarm_count = 2u;
	alarm = lib.alarm_ptr[1];
	alarm->lib = NULL;
	result = iot_alarm_deregister( alarm );
	assert_int_equal( result, IOT_STATUS_NOT_INITIALIZED );
	assert_int_equal( lib.alarm_count, 2u );
}

static void test_iot_alarm_deregister_valid( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_alarm_t *alarm;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_STACK_MAX; i++ )
		lib.alarm_ptr[i] = &lib.alarm[i];
	lib.alarm_count = 2u;
	alarm = lib.alarm_ptr[1];
	alarm->lib = &lib;
	result = iot_alarm_deregister( alarm );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.alarm_count, 1u );
}

static void test_iot_alarm_deregister_valid_in_heap( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_alarm_t *alarm;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_STACK_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.alarm_ptr[i] = &lib.alarm[i];
		bzero( lib.alarm_ptr[i], sizeof( struct iot_alarm ) );
#else
		will_return( __wrap_os_malloc, 1u );
		lib.alarm_ptr[i] = os_malloc( sizeof( struct iot_alarm ) );
		bzero( lib.alarm_ptr[i], sizeof( struct iot_alarm ) );
		lib.alarm_ptr[i]->is_in_heap = IOT_TRUE;
#endif
		lib.alarm_ptr[i]->lib = &lib;
#ifdef IOT_STACK_ONLY
		lib.alarm_ptr[i]->name = lib.alarm_ptr[i]->_name;
#else
		will_return( __wrap_os_malloc, 1u );
		lib.alarm_ptr[i]->name = os_malloc( 10u );
#endif
		snprintf( lib.alarm_ptr[i]->name, 10u, "alarm #%u", (unsigned int)i );
		++lib.alarm_count;
	}

	for ( i = 0u; i < IOT_ALARM_STACK_MAX; ++i )
	{
		alarm = lib.alarm_ptr[IOT_ALARM_STACK_MAX - i - 1u];
		assert_int_equal( lib.alarm_count, IOT_ALARM_STACK_MAX - i);
		result = iot_alarm_deregister( alarm );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
		assert_int_equal( lib.alarm_count, IOT_ALARM_STACK_MAX - i - 1u);
	}
}

static void test_iot_alarm_publish_null_alarm( void **state )
{
	iot_status_t result;

	result = iot_alarm_publish( NULL, NULL, NULL, 1u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_alarm_publish_null_lib( void **state )
{
	struct iot_alarm alarm;
	iot_status_t result;

	bzero( &alarm, sizeof( struct iot_alarm ) );
	result = iot_alarm_publish( &alarm, NULL, NULL, 1u );
	assert_int_equal( result, IOT_STATUS_NOT_INITIALIZED );
}

static void test_iot_alarm_publish_plugin_failure( void **state )
{
	struct iot_alarm alarm;
	struct iot lib;
	iot_status_t result;

	bzero( &alarm, sizeof( struct iot_alarm ) );
	bzero( &lib, sizeof( struct iot ) );
	alarm.lib = &lib;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_FAILURE );
	result = iot_alarm_publish( &alarm, NULL, NULL, 1u );
	assert_int_equal( result, IOT_STATUS_FAILURE );
}

static void test_iot_alarm_publish_valid( void **state )
{
	struct iot_alarm alarm;
	struct iot lib;
	iot_status_t result;

	bzero( &alarm, sizeof( struct iot_alarm ) );
	bzero( &lib, sizeof( struct iot ) );
	alarm.lib = &lib;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_alarm_publish( &alarm, NULL, NULL, 1u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_alarm_publish_string_null_lib( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_alarm_t *alarm;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_MAX; i++ )
		lib.alarm_ptr[i] = &lib.alarm[i];
	lib.alarm_count = 1u;
	alarm = lib.alarm_ptr[0];
	alarm->lib = NULL;
	result = iot_alarm_publish_string( alarm, NULL, NULL, 1u, "msg" );
	assert_int_equal( result, IOT_STATUS_NOT_INITIALIZED );
}

static void test_iot_alarm_publish_string_null_alarm( void **state )
{
	iot_status_t result;

	result = iot_alarm_publish_string( NULL, NULL, NULL, 1u, "msg" );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_alarm_publish_string_valid( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_alarm_t *alarm;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_MAX; i++ )
		lib.alarm_ptr[i] = &lib.alarm[i];
	lib.alarm_count = 1u;
	alarm = lib.alarm_ptr[0];
	alarm->lib = &lib;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_alarm_publish_string( alarm, NULL, NULL, 1u, "msg" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_alarm_publish_string_null_message( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_alarm_t *alarm;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_MAX; i++ )
		lib.alarm_ptr[i] = &lib.alarm[i];
	lib.alarm_count = 1u;
	alarm = lib.alarm_ptr[0];
	alarm->lib = &lib;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_alarm_publish_string( alarm, NULL, NULL, 1u, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_alarm_publish_string_empty_message( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_alarm_t *alarm;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ALARM_MAX; i++ )
		lib.alarm_ptr[i] = &lib.alarm[i];
	lib.alarm_count = 1u;
	alarm = lib.alarm_ptr[0];
	alarm->lib = &lib;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_alarm_publish_string( alarm, NULL, NULL, 1u, "" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_iot_alarm_register_empty ),
		cmocka_unit_test( test_iot_alarm_register_full ),
		cmocka_unit_test( test_iot_alarm_register_stack_full ),
		cmocka_unit_test( test_iot_alarm_register_null_lib ),
		cmocka_unit_test( test_iot_alarm_register_null_name ),
		cmocka_unit_test( test_iot_alarm_register_no_memory_obj ),
		cmocka_unit_test( test_iot_alarm_register_no_memory_name ),
		cmocka_unit_test( test_iot_alarm_register_valid ),
		cmocka_unit_test( test_iot_alarm_deregister_null_alarm ),
		cmocka_unit_test( test_iot_alarm_deregister_null_lib ),
		cmocka_unit_test( test_iot_alarm_deregister_valid ),
		cmocka_unit_test( test_iot_alarm_deregister_valid_in_heap ),
		cmocka_unit_test( test_iot_alarm_publish_null_alarm ),
		cmocka_unit_test( test_iot_alarm_publish_null_lib ),
		cmocka_unit_test( test_iot_alarm_publish_plugin_failure ),
		cmocka_unit_test( test_iot_alarm_publish_valid ),
		cmocka_unit_test( test_iot_alarm_publish_string_null_lib ),
		cmocka_unit_test( test_iot_alarm_publish_string_null_alarm ),
		cmocka_unit_test( test_iot_alarm_publish_string_valid ),
		cmocka_unit_test( test_iot_alarm_publish_string_null_message ),
		cmocka_unit_test( test_iot_alarm_publish_string_empty_message )
	};
	test_initialize( argc, argv );
	result = cmocka_run_group_tests( tests, NULL, NULL );
	test_finalize( argc, argv );
	return result;
}
