#!/usr/bin/env python

'''
Python example for the iot-complete app.

Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.

The right to copy, distribute or otherwise make use of this software
may be licensed only pursuant to the terms of an applicable Wind River
license agreement.  No license to Wind River intellectual property rights is
granted herein.  All rights not licensed by Wind River are reserved by Wind
River.
'''

import os
import sys
import signal
import random
import inspect

from time import sleep
sys.path.append( "../lib" )
from iot_python import *

POLL_INTERVAL_SEC = 10
MAX_LOOP_ITERATIONS = 360
TAG_MAX_LEN = 128

# Set up named parameters for a sample action to validate actions with
# parameters
PARAM_NAME_STR = "param_str"
PARAM_NAME_INT = "param_int"
PARAM_NAME_FLOAT = "param_float"
PARAM_NAME_BOOL = "param_bool"
PARAM_OUT_NAME_BOOL = "param_out"

bool_test = IOT_FALSE
running = True
iot_lib_hdl = None
telemetry_temp = None
telemetry_light = None

#  add one of each telemetry type to test
telemetry_boolean = None
telemetry_location = None
telemetry_string = None
telemetry_int8 = None
telemetry_int16 = None
telemetry_int32 = None
telemetry_int64 = None
telemetry_raw = None

enable_location_cmd = None
enable_telemetry_cmd = None
script_cmd = None
test_params_cmd = None
send_location = False
send_telemetry = False

# Text sample used for random string generation
text_buffer = "Arma virumque cano, Troiae qui primus ab oris" \
        "Italiam, fato profugus, Laviniaque venit" \
        "litora, multum ille et terris iactatus et alto" \
        "vi superum saevae memorem Iunonis ob iram;" \
        "multa quoque et bello passus, dum conderet urbem," \
        "inferretque deos Latio, genus unde Latinum," \
        "Albanique patres, atque altae moenia Romae."

# Text sample length.
MAX_TEXT_SIZE = 64

def generate_random_string( length ):
    '''Random string generation handler'''
    return ( "".join( random.choice( text_buffer )
            for x in range( length ) ) )


def debug_log( log_level, source, msg ):
    '''Debug log wrapper for printing, used for callbacks'''
    i = 0
    prefix = ["FATAL","ALERT","CRITICAL","ERROR","WARNING",
            "NOTICE","INFO","DEBUG","TRACE"]
    # ensure log level is a valid enumeration value
    if ( log_level <= IOT_LOG_TRACE ):
        i = log_level
    print( "{}: {}".format( prefix[i], msg ) )


def IOT_LOG( handle, level, msg ):
    '''Logging function with support for call location'''
    # previous function call
    callerframerecord = inspect.stack()[1]
    # callerframrecord :  1 = function, 3 = file, 2 = line
    iot_log( handle, level, callerframerecord[1], callerframerecord[3],
            callerframerecord[2], msg )


def initialize( exe_path ):
    '''Connects to the agent and registers all actions and telemetry'''
    global iot_lib_hdl, telemetry_light, telemetry_temp, telemetry_location
    global telemetry_boolean, telemetry_string, telemetry_raw, telemetry_int8
    global telemetry_int16, telemetry_int32, telemetry_int64
    global enable_location_cmd, enable_telemetry_cmd, script_cmd
    global test_params_cmd
    result = False
    status = IOT_STATUS_FAILURE

    iot_lib_hdl = iot_initialize( "complete-app-py", None, 0 )
    iot_log_callback_set( iot_lib_hdl, debug_log )
    status = iot_connect( iot_lib_hdl, 0 )
    if ( status == IOT_STATUS_SUCCESS ):
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Connected" )

        if ( os.name == "posix" ):
            script_name = "test_script.sh"
        else:
            script_name = "test_script.cmd"

        script_path = os.path.join( os.path.dirname(exe_path), script_name )

        # Allocate telemetry items
        telemetry_light = iot_telemetry_allocate( iot_lib_hdl,
                "light", IOT_TYPE_FLOAT64 )
        telemetry_temp = iot_telemetry_allocate( iot_lib_hdl,
                "temp", IOT_TYPE_FLOAT64 )
        iot_telemetry_attribute_set( telemetry_temp,
                "udmp:units", IOT_TYPE_STRING, "Celsius" )
        telemetry_boolean = iot_telemetry_allocate( iot_lib_hdl,
                "bool", IOT_TYPE_BOOL )
        telemetry_string = iot_telemetry_allocate( iot_lib_hdl,
                "string", IOT_TYPE_STRING )
        telemetry_int8 = iot_telemetry_allocate( iot_lib_hdl,
                "int8", IOT_TYPE_INT8 )
        telemetry_int16 = iot_telemetry_allocate( iot_lib_hdl,
                "int16", IOT_TYPE_INT16 )
        telemetry_int32 = iot_telemetry_allocate( iot_lib_hdl,
                "int32", IOT_TYPE_INT32 )
        telemetry_int64 = iot_telemetry_allocate( iot_lib_hdl,
                "int64", IOT_TYPE_INT64 )
        telemetry_raw = iot_telemetry_allocate( iot_lib_hdl,
                "raw", IOT_TYPE_RAW )
        telemetry_location = iot_telemetry_allocate( iot_lib_hdl,
                "location", IOT_TYPE_LOCATION )

        # Register telemetry items
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Registering telemetry: {}".format(
                "light" ) )
        iot_telemetry_register( telemetry_light, None, 0 )
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Registering telemetry: {}".format(
                "temp" ) )
        iot_telemetry_register( telemetry_temp, None, 0 )
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Registering telemetry: {}".format(
                "bool" ) )
        iot_telemetry_register( telemetry_boolean, None, 0 )
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Registering telemetry: {}".format(
                "string" ) )
        iot_telemetry_register( telemetry_string, None, 0 )
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Registering telemetry: {}".format(
                "int8" ) )
        iot_telemetry_register( telemetry_int8, None, 0 )
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Registering telemetry: {}".format(
                "int16" ) )
        iot_telemetry_register( telemetry_int16, None, 0 )
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Registering telemetry: {}".format(
                "int32" ) )
        iot_telemetry_register( telemetry_int32, None, 0 )
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Registering telemetry: {}".format(
                "int64" ) )
        iot_telemetry_register( telemetry_int64, None, 0 )
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Registering telemetry: {}".format(
                "raw" ) )
        iot_telemetry_register( telemetry_raw, None, 0 )
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Registering telemetry: {}".format(
                "location" ) )
        iot_telemetry_register( telemetry_location, None, 0 )

        #  Allocate actions
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Registering action: {}".format(
                "telemetry_ON_OFF" ) )
        enable_telemetry_cmd = iot_action_allocate( iot_lib_hdl,
                "telemetry_ON_OFF" )
        status = iot_action_register_callback( enable_telemetry_cmd,
                on_enable_disable_telemetry, None, 0 )
        if ( status != IOT_STATUS_SUCCESS ):
            IOT_LOG( iot_lib_hdl, IOT_LOG_ERROR,
                    "Failed to register command. Reason: {}".format(
                    iot_error( status ) ) )
        sleep( 0.5 )
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Registering action: {}".format(
                "location_ON_OFF" ) )
        enable_location_cmd = iot_action_allocate( iot_lib_hdl,
                "location_ON_OFF" )
        status = iot_action_register_callback( enable_location_cmd,
                on_enable_disable_location, None, 0 )
        if ( status != IOT_STATUS_SUCCESS ):
            IOT_LOG( iot_lib_hdl, IOT_LOG_ERROR,
                    "Failed to register command. Reason: {}".format(
                    iot_error( status ) ) )
        sleep( 0.5 )
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Registering action: {}".format(
                "test_script" ) )
        script_cmd = iot_action_allocate( iot_lib_hdl, "test_script" )
        iot_action_parameter_add( script_cmd,
                "param1", IOT_PARAMETER_IN, IOT_TYPE_STRING, 0 )
        iot_action_parameter_add( script_cmd,
                "param2", IOT_PARAMETER_IN, IOT_TYPE_STRING, 0 )
        status = iot_action_register_command( script_cmd, script_path,
                None, 0 )
        if ( status != IOT_STATUS_SUCCESS ):
            IOT_LOG( iot_lib_hdl, IOT_LOG_ERROR,
                    "Failed to register script. Reason: {}".format(
                    iot_error( status ) ) )
        sleep( 0.5 )

        # create the test params action
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO,
                "Registering action test_parameters\n" )
        test_params_cmd = iot_action_allocate( iot_lib_hdl, "test_parameters" )

        # 1. param str
        iot_action_parameter_add( test_params_cmd,
            PARAM_NAME_STR, IOT_PARAMETER_IN, IOT_TYPE_STRING, 0 )

        # 2. param_int
        iot_action_parameter_add( test_params_cmd,
            PARAM_NAME_INT, IOT_PARAMETER_IN, IOT_TYPE_INT32, 0 )

        # 3. param_float
        iot_action_parameter_add( test_params_cmd,
            PARAM_NAME_FLOAT, IOT_PARAMETER_IN, IOT_TYPE_FLOAT64, 0 )

        # 4. param_bool
        iot_action_parameter_add( test_params_cmd,
            PARAM_NAME_BOOL, IOT_PARAMETER_IN, IOT_TYPE_BOOL, 0 )

        # 5. param to call set on
        iot_action_parameter_add( test_params_cmd,
                PARAM_OUT_NAME_BOOL, IOT_PARAMETER_OUT, IOT_TYPE_BOOL, 0 )

        status = iot_action_register_callback(test_params_cmd,
                on_action_test_parameters, None, 0 )
        if ( status != IOT_STATUS_SUCCESS ):
            IOT_LOG( iot_lib_hdl, IOT_LOG_ERROR,
                    "Failed to register command. Reason: {}".format(
                    iot_error( status ) ) )
    else:
        IOT_LOG( iot_lib_hdl, IOT_LOG_ERROR, "Failed to connect" )
    if ( status == IOT_STATUS_SUCCESS ):
        result = True
    return result


def on_enable_disable_location( request ):
    '''Callback function for enabling & disabling sending location'''
    global send_location
    if ( not send_location ):
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Sending location..." )
        send_location = True
    else:
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Disabling location..." )
        send_location = False
    return IOT_STATUS_SUCCESS


def on_enable_disable_telemetry( request ):
    '''Callback function for enabling & disabling sending telemetery'''
    global send_telemetry
    if ( not send_telemetry ):
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Sending telemetry..." )
        send_telemetry = True
    else:
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Disabling telemetry..." )
        send_telemetry = False
    return IOT_STATUS_SUCCESS


def on_action_test_parameters( request ):
    '''Callback function for testing parameters'''
    result = IOT_STATUS_SUCCESS
    status = IOT_STATUS_FAILURE
    param_str = ""
    param_int = 0
    param_float = 0.0
    param_bool = IOT_FALSE

    IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "on_action_test_parameters invoked\n\n")

    # success path test each parameter.
    # 1. string
    ( status, param_str ) = iot_action_parameter_get( request,
            PARAM_NAME_STR, False, IOT_TYPE_STRING )
    if ( status != IOT_STATUS_SUCCESS ):
        result = IOT_STATUS_BAD_PARAMETER
        IOT_LOG( iot_lib_hdl, IOT_LOG_ERROR,
                "get param failed for {}\n".format( PARAM_NAME_STR ) )
    else:
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO,
                "{} success, value = {}\n".format(
                PARAM_NAME_STR, param_str ) )

    # int
    if ( result == IOT_STATUS_SUCCESS ):
        ( status, param_int ) = iot_action_parameter_get( request,
                PARAM_NAME_INT, False, IOT_TYPE_INT32 )
        if ( status != IOT_STATUS_SUCCESS ):
            result = IOT_STATUS_BAD_PARAMETER
            IOT_LOG( iot_lib_hdl, IOT_LOG_ERROR,
                    "get param failed for {}\n".format( PARAM_NAME_INT ) )
        else:
            IOT_LOG( iot_lib_hdl, IOT_LOG_INFO,
                    "{} success, value = {}\n".format(
                    PARAM_NAME_INT, param_int ) )

    # float
    if ( result == IOT_STATUS_SUCCESS ):
        ( status, param_float ) = iot_action_parameter_get( request, PARAM_NAME_FLOAT,
                IOT_FALSE, IOT_TYPE_FLOAT64 )
        if ( status != IOT_STATUS_SUCCESS ):
            result = IOT_STATUS_BAD_PARAMETER
            IOT_LOG( iot_lib_hdl, IOT_LOG_ERROR,
                    "get param failed for {}\n".format( PARAM_NAME_FLOAT ) )
        else:
            IOT_LOG( iot_lib_hdl, IOT_LOG_INFO,
                    "{} success, value = {}\n".format(
                    PARAM_NAME_FLOAT,  param_float ) )

    # bool
    if ( result == IOT_STATUS_SUCCESS ):
        ( status, param_bool ) = iot_action_parameter_get( request, PARAM_NAME_BOOL,
                IOT_FALSE, IOT_TYPE_BOOL )
        if ( status != IOT_STATUS_SUCCESS ):
            result = IOT_STATUS_BAD_PARAMETER
            IOT_LOG( iot_lib_hdl, IOT_LOG_ERROR,
                    "get param failed for {}\n".format( PARAM_NAME_BOOL ) )
        else:
            IOT_LOG( iot_lib_hdl, IOT_LOG_INFO,
                    "{} success, value = {}\n".format(
                    PARAM_NAME_BOOL,  param_bool ) )

    # test setting a param, then validate that the message in the action
    # log in ePO shows this false value.
    iot_action_parameter_set( request, PARAM_OUT_NAME_BOOL,
            IOT_TYPE_BOOL, False )
    return result


def send_location_sample():
    status = IOT_STATUS_FAILURE
    latitude = random.uniform( -90.0, 90.0 )
    longitude = random.uniform( -180.0, 180.0 )
    accuracy = random.uniform( 0.0, 1000.0 )
    altitude = random.uniform( -15.0, 10000.0 )
    altitude_accuracy = random.uniform( 0.0, 1000.0 )
    heading = random.uniform( 0.0, 360.0 )
    speed = random.uniform( 0.0, 10000.0 )
    source = int( random.uniform( IOT_LOCATION_SOURCE_FIXED,
            IOT_LOCATION_SOURCE_WIFI ) )
    tag_size = int( random.uniform ( 0, TAG_MAX_LEN ) )
    tag = generate_random_string( tag_size-1 )

    LOG_FORMAT = "Location:\n" \
            "\tlatitude         :{}\n" \
            "\tlongitude        :{}\n" \
            "\taccuracy         :{}\n" \
            "\taltitude         :{}\n" \
            "\taltitude_accuracy:{}\n" \
            "\theading          :{}\n" \
            "\tspeed            :{}\n" \
            "\tsource           :{}\n" \
            "\ttag              :{}\n"

    sample = iot_location_allocate( 0.0, 0.0 )
    if ( sample ):
        status = iot_location_set( sample, latitude, longitude )
        if ( status != IOT_STATUS_SUCCESS ):
            IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
                    "Setting latitude and longitude failed: {}\n".format(
                    iot_error( status ) ) )
        status = iot_location_accuracy_set( sample, accuracy )
        if ( status != IOT_STATUS_SUCCESS ):
            IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
                    "Setting accuracy failed: {}\n".format(
                    iot_error( status ) ) )
        status = iot_location_altitude_set( sample, altitude )
        if ( status != IOT_STATUS_SUCCESS ):
            IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
                    "Setting altitude failed: {}\n".format(
                    iot_error( status ) ) )
        status = iot_location_altitude_accuracy_set( sample, altitude_accuracy )
        if ( status != IOT_STATUS_SUCCESS ):
            IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
                    "Setting altitude accuracy failed: {}\n".format(
                    iot_error( status ) ) )
        status = iot_location_heading_set( sample, heading )
        if ( status != IOT_STATUS_SUCCESS ):
            IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
                    "Setting heading failed: {}\n".format(
                    iot_error( status ) ) )
        status = iot_location_speed_set( sample, speed )
        if ( status != IOT_STATUS_SUCCESS ):
            IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
                    "Setting speed failed: {}\n".format(
                    iot_error( status ) ) )
        status = iot_location_source_set( sample, source )
        if ( status != IOT_STATUS_SUCCESS ):
            IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
                    "Setting source failed: {}\n".format(
                    iot_error( status ) ) )
        status = iot_location_tag_set( sample, tag )
        if ( status != IOT_STATUS_SUCCESS ):
            IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
                    "Setting tag failed: {}\n".format(
                    iot_error( status ) ) )

        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO,
                LOG_FORMAT.format(latitude, longitude, accuracy,
                altitude, altitude_accuracy, heading,
                speed, source, tag ) )

        status = iot_telemetry_publish( telemetry_location,
                None, 0, IOT_TYPE_LOCATION, sample )
        if ( status != IOT_STATUS_SUCCESS ):
            IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
                    "Sample publish failed: {}\n".format(
                    iot_error( status ) ) )
    else:
        IOT_LOG( iot_lib_hdl, IOT_LOG_ERROR,
                "Failed to allocate a location sample.\n" )
    if ( sample ):
        iot_location_free( sample )


def send_telemetry_sample():
    '''Send telemetry data to the agent'''
    global bool_test
    light = 0.0
    temperature = 0.0
    string_test = ""
    sample_size = 0
    int_test = int( random.uniform( -3000000000.0, 3000000000.0 ) )
    light = random.uniform( 100.0, 1000.0 )
    temperature = random.uniform( 1.0, 45.0 )
    sample_size = int( random.uniform( 0, MAX_TEXT_SIZE-1 ) )
    string_test = generate_random_string( sample_size-1 )

    IOT_LOG( iot_lib_hdl, IOT_LOG_INFO,
        "{}\n".format("+--------------------------------------------------------+"))

    IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Sending light : {}".format(light) );
    iot_telemetry_publish( telemetry_light, None, 0, IOT_TYPE_FLOAT64, light )

    IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Sending temp  : {}".format(temperature) );
    iot_telemetry_publish( telemetry_temp, None, 0, IOT_TYPE_FLOAT64, temperature )

    IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Sending bool  : {}".format(bool_test) );
    iot_telemetry_publish( telemetry_boolean, None, 0, IOT_TYPE_BOOL, bool_test )

    IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Sending string : {}".format(string_test) );
    iot_telemetry_publish( telemetry_string, None, 0, IOT_TYPE_STRING, string_test )

    IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Sending int8   : {}".format(int_test) );
    iot_telemetry_publish( telemetry_int8, None, 0, IOT_TYPE_INT8, int_test )

    IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Sending int16  : {}".format(int_test) );
    iot_telemetry_publish( telemetry_int16, None, 0, IOT_TYPE_INT16, int_test )

    IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Sending int32  : {}".format(int_test) );
    iot_telemetry_publish( telemetry_int32, None, 0, IOT_TYPE_INT32, int_test )

    IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Sending int64  : {}".format(int_test) );
    iot_telemetry_publish( telemetry_int64, None, 0, IOT_TYPE_INT64, int_test )
    IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Sending raw    : {}".format(string_test) );
    iot_telemetry_publish_raw( telemetry_raw, None, 0, sample_size, string_test )

    # cycle the boolean value for next sample
    # should appear as toggling in the cloud
    if ( str( bool_test ) == str( True ) ):
        bool_test = False
    elif ( str( bool_test ) == str( False ) ):
        bool_test = IOT_TRUE
    elif ( str( bool_test ) == str( IOT_TRUE ) ):
        bool_test = IOT_FALSE
    elif ( str( bool_test ) == str( IOT_FALSE ) ):
        bool_test = True


def sig_handler( signo, frame ):
    '''Handles terminatation signal and tears down gracefully'''
    global running
    if ( signo == signal.SIGINT ):
        IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Received termination signal...\n" )
        running = False


if ( __name__ == '__main__' ):
    if ( initialize( sys.argv[0] ) == IOT_TRUE ):
        signal.signal( signal.SIGINT, sig_handler )

        # If any arg is passed in, then start telemetry
        if ( len( sys.argv ) > 1 ):
            send_telemetry = True
        sleep( 0.5 )

        if ( send_telemetry ):
            IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Sending telemetry..." )
        else:
            IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Disabling telemetry..." )
        while ( running ):
            count = 0
            if ( send_telemetry or send_location ):
                count += 1
                if ( send_telemetry ):
                    send_telemetry_sample()
                if ( send_location ):
                    send_location_sample()
                if ( count > MAX_LOOP_ITERATIONS ):
                    if ( send_telemetry ):
                        on_enable_disable_telemetry( None )
                    if ( send_location ):
                        on_enable_disable_location( None )
                    count = 0
                sleep( POLL_INTERVAL_SEC )
            else:
                sleep( 1 )

    # Test Free API
    iot_action_free( script_cmd, 0 )

    # Test deregister API
    # Unnecessary since iot_terminate will deregister actions
    iot_action_deregister( test_params_cmd, None, 0 )

    # Test Free API for telemetry resources allocated
    # Unnecessary since iot_terminate will free actions
    iot_telemetry_free( telemetry_light, 0 )

    #  Terminate
    IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Exiting..." )
    iot_terminate( iot_lib_hdl, 0 )
    exit( 0 )
