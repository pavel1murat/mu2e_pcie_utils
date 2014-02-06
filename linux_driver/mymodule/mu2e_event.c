/*  This file (mu2e_event.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Feb  5, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: .emacs.gnu,v $
    rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";
    */

#include <linux/timer.h>	/* del_timer_sync  */
#include <linux/mm.h>

#include "../trace/trace.h"	/* TRACE */
#include "mu2e_event.h"

#define PACKET_POLL_HZ 100

struct timer_list packets_timer;


static void poll_packets(unsigned long __opaque)
{
    int offset;
    TRACE( 2, "poll_packets" );


    // Reschedule poll routine.
    offset = HZ / PACKET_POLL_HZ;
    packets_timer.expires = jiffies + offset;
    add_timer( &packets_timer );
}


int mu2e_event_up( void )
{
    init_timer( &packets_timer );
    packets_timer.expires = jiffies + (HZ/PACKET_POLL_HZ);
    //timer->data=(unsigned long) pdev;
    packets_timer.function = poll_packets;
    add_timer( &packets_timer );

    return (0);
}

void mu2e_event_down( void )
{
    del_timer_sync( &packets_timer );
}
