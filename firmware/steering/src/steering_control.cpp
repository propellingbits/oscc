/**
 * @file steering_control.cpp
 *
 */


#include <Arduino.h>
#include <stdlib.h>
#include <stdint.h>
#include "debug.h"
#include "oscc_dac.h"
#include "can_protocols/steering_can_protocol.h"
#include "dtc.h"
#include "vehicles.h"

#include "communications.h"
#include "steering_control.h"
#include "globals.h"


/*
 * @brief Number of consecutive faults that can occur when reading the
 *        torque sensor before control is disabled.
 *
 */
#define SENSOR_VALIDITY_CHECK_FAULT_COUNT ( 4 )


static void read_torque_sensor(
    steering_torque_s * value );


void check_for_operator_override( void )
{
    if( g_steering_control_state.enabled == true
        || g_steering_control_state.operator_override == true )
    {
        steering_torque_s torque;

        read_torque_sensor( &torque );

        if ( (abs(torque.high) >= OVERRIDE_WHEEL_THRESHOLD_IN_DEGREES_PER_USEC)
            || (abs(torque.low) >= OVERRIDE_WHEEL_THRESHOLD_IN_DEGREES_PER_USEC) )
        {
            disable_control( );

            publish_fault_report( );

            g_steering_control_state.operator_override = true;

            DEBUG_PRINTLN( "Operator override" );
        }
        else
        {
            g_steering_control_state.operator_override = false;
        }
    }
}


void check_for_sensor_faults( void )
{
    if ( (g_steering_control_state.enabled == true)
        || DTC_CHECK(g_steering_control_state.dtcs, OSCC_STEERING_DTC_INVALID_SENSOR_VAL) )
    {
        static int fault_count = 0;

        steering_torque_s torque;

        read_torque_sensor(&torque);

        // sensor pins tied to ground - a value of zero indicates disconnection
        if( (torque.high == 0)
            || (torque.low == 0) )
        {
            ++fault_count;

            if( fault_count >= SENSOR_VALIDITY_CHECK_FAULT_COUNT )
            {
                disable_control( );

                publish_fault_report( );

                DTC_SET(
                    g_steering_control_state.dtcs,
                    OSCC_STEERING_DTC_INVALID_SENSOR_VAL );

                DEBUG_PRINTLN( "Bad value read from torque sensor" );
            }
        }
        else
        {
            DTC_CLEAR(
                    g_steering_control_state.dtcs,
                    OSCC_STEERING_DTC_INVALID_SENSOR_VAL );

            fault_count = 0;
        }
    }
}


void update_steering(
    uint16_t spoof_command_high,
    uint16_t spoof_command_low )
{
    if ( g_steering_control_state.enabled == true )
    {
        uint16_t spoof_high =
            constrain(
                spoof_command_high,
                STEERING_SPOOF_HIGH_SIGNAL_RANGE_MIN,
                STEERING_SPOOF_HIGH_SIGNAL_RANGE_MAX );

        uint16_t spoof_low =
            constrain(
                spoof_command_low,
                STEERING_SPOOF_LOW_SIGNAL_RANGE_MIN,
                STEERING_SPOOF_LOW_SIGNAL_RANGE_MAX );

        cli();
        g_dac.outputA( spoof_high );
        g_dac.outputB( spoof_low );
        sei();
     }
}


void enable_control( void )
{
    if( g_steering_control_state.enabled == false
        && g_steering_control_state.operator_override == false )
    {
        const uint16_t num_samples = 20;
        prevent_signal_discontinuity(
            g_dac,
            num_samples,
            PIN_TORQUE_SENSOR_HIGH,
            PIN_TORQUE_SENSOR_LOW );

        cli();
        digitalWrite( PIN_SPOOF_ENABLE, HIGH );
        sei();

        g_steering_control_state.enabled = true;

        DEBUG_PRINTLN( "Control enabled" );
    }
}


void disable_control( void )
{
    if( g_steering_control_state.enabled == true )
    {
        const uint16_t num_samples = 20;
        prevent_signal_discontinuity(
            g_dac,
            num_samples,
            PIN_TORQUE_SENSOR_HIGH,
            PIN_TORQUE_SENSOR_LOW );

        cli();
        digitalWrite( PIN_SPOOF_ENABLE, LOW );
        sei();

        g_steering_control_state.enabled = false;

        DEBUG_PRINTLN( "Control disabled" );
    }
}

static void read_torque_sensor(
    steering_torque_s * value )
{
    cli();
    value->high = analogRead( PIN_TORQUE_SENSOR_HIGH );
    value->low = analogRead( PIN_TORQUE_SENSOR_LOW );
    sei();
}

