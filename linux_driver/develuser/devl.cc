 // This file (devl.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Apr 23, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#include <stdio.h>		// printf
#include <unistd.h>             /* getopt */
#include <getopt.h>             /* getopt */
#include "trace.h"		// TRACE
#include "pcidevl_ioctl.h"

#define USAGE "\
    usage: %s\n\
", basename(argv[0])

int
main(  int	argc
     , char	*argv[] )
{
extern  int        optind;         /* for getopt */

    if (argc == 1) { printf( USAGE ); return (0); }

    int fd=open( "/dev/" PCIDEVL_DEV_FILE, O_RDONLY );
    if (fd == -1) { perror("open /dev/" PCIDEVL_DEV_FILE ); return (1); }

    optind = 1;

    if (strcmp(argv[optind],"init") == 0)
    {	TRACE( 2, "init" );
    }
    return (0);
}   // main
