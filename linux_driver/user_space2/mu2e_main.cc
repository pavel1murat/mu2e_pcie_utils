 // This file (mu2e_main.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Feb 13, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

/*
 make mu2e_main LDFLAGS=mu2edev.o
OR
 touch mu2e*.cc;\
 make mu2edev.o CFLAGS='-g -Wall -std=c++0x -pedantic'\
      mu2e_main LDFLAGS=mu2edev.o && ./mu2e_main
 */

#include <stdio.h>		// printf
#include <stdint.h>		// uint16_t
#include <sys/time.h>		// gettimeofday
#include "mu2edev.hh"


uint64_t get_us_timeofday()
{   struct timeval tv;
    gettimeofday( &tv, NULL );
    return (uint64_t)tv.tv_sec*1000000+tv.tv_usec;
}



int
main(  int	argc
     , char	*argv[] )
{
    mu2edev  dev;
    uint64_t total_bytes=0;
    uint64_t delta, mark=get_us_timeofday();
    int      reads_with_data=0;

    //dev.meta_dump( 0, C2S );
    for (unsigned ii=0; ii<10000000; ++ii)
    {
	uint16_t *bufp;
	int       tmo_ms=1000;

	int sts=dev.read_data( 0, (void**)&bufp, tmo_ms );
	if (sts > 0)
	{
	    total_bytes+=sts;
	    reads_with_data++;
	    dev.read_release( 0, 1 );
	}
	//else
	//  printf( "dev.read_data=%d\n", sts );
    }
    delta=get_us_timeofday()-mark;
    printf( "%lu bytes in %lu microseconds reads_with_data=%d\n"
	   ,total_bytes, delta, reads_with_data );
    return (0);
}   // main
