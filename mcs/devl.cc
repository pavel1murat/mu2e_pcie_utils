 // This file (devl.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Apr 23, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#include <stdio.h>		// printf
#include <unistd.h>             /* getopt */
#include <getopt.h>             /* getopt */
#include <stdint.h>		// uint64_t
#include <stdlib.h>		// setenv
#include "trace.h"		// TRACE
#include "pcidevl_ioctl.h"

#define USAGE "\
usage: %s <cmd>\n\
<cmd> is one of:\n\
  hello           - TRACE hello\n\
  register        - pci_register_driver, if necessary\n\
  unregister      - iounmap, pci_unregister_driver\n\
  ioremap         - \n\
  iounmap         - \n\
  uint32 <offset> - \n\
  read_loop       - \n\
", basename(argv[0])

int
main(  int	argc
     , char	*argv[] )
{
	int		sts;
extern  int     	optind;	/* for getopt */
	const char     *cmd;

    if (argc == 1) { printf( USAGE ); return (0); }
    setenv( "TRACE_FILE", "/proc/trace/buffer", 0 ); // allow user to set in env.
    setenv( "TRACE_LVLS", "0xff", 0 ); // levels 0,1,2 but no-overwrite -- allow user to set in env.
    TRACE_CNTL( "lvlmskS", (uint64_t)0xff ); TRACE_CNTL( "modeS", (uint64_t)1 );

    int fd=open( "/dev/" PCIDEVL_DEV_FILE, O_RDONLY );
    if (fd == -1) { perror("open /dev/" PCIDEVL_DEV_FILE ); return (1); }

    optind = 1;
    cmd = argv[optind++];

    if      (strcmp(cmd,"hello") == 0)
    {	TRACE( 2, "hello" );
	sts = ioctl( fd, IOC_HELLO );
	if (sts == -1) { perror("ioctl HELLO"); return (1); }
    }
    else if (strcmp(cmd,"register") == 0)
    {	TRACE( 2, "register" );
	sts = ioctl( fd, IOC_REGISTER );
	if (sts == -1) { perror("ioctl REGISTER"); return (1); }
    }
    else if (strcmp(cmd,"unregister") == 0)
    {	TRACE( 2, "unregister" );
	sts = ioctl( fd, IOC_UNREGISTER );
	if (sts == -1) { perror("ioctl UNREGISTER"); return (1); }
    }
    else if (strcmp(cmd,"ioremap") == 0)
    {	TRACE( 2, "ioremap" );
	sts = ioctl( fd, IOC_IOREMAP );
	if (sts == -1) { perror("ioctl IOREMAP"); return (1); }
    }
    else if (strcmp(cmd,"iounmap") == 0)
    {	TRACE( 2, "iounmap" );
	sts = ioctl( fd, IOC_IOUNMAP );
	if (sts == -1) { perror("ioctl IOUNMAP"); return (1); }
    }
    else if (strcmp(cmd,"uint32") == 0)
    {	if ((argc-optind) < 1) { printf("cmd \"uint32\" needs an arg (offset)\n"); return (1); }
	TRACE( 2, "uint32" );
	uint32_t in_out=strtoul(argv[optind],NULL,0);
	sts = ioctl( fd, IOC_UINT32, &in_out );
	if (sts == -1) { perror("ioctl UINT32"); return (1); }
	TRACE( 2, "val at off=0x%08x", in_out );
    }
    else
    {   TRACE( 0, "invalid cmd" );
    }
    return (0);
}   // main
