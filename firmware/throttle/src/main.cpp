/**
 * @file main.cpp
 *
 */


#include "arduino_init.h"
#include "debug.h"

#include "init.h"
#include "timers.h"
#include "communications.h"
#include "throttle_control.h"

int main( void )
{
    init_arduino( );

    init_globals( );

    init_devices( );

    init_communication_interfaces( );

    start_timers( );

    DEBUG_PRINTLN( "init complete" );

    while( true )
    {
        check_for_incoming_message( );

        check_for_operator_override( );
    }
}
