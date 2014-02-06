 // This file (xdma_user_space.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Dec  9, 2013. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#include <stdio.h>		// printf
#include <fcntl.h>		// open, O_RDWR
#include <stdlib.h>		// exit
#include <unistd.h>		// getopt_long
#include <getopt.h>		// option
#include <stdint.h>		// uint32_t
#include <libgen.h>		// basename
#include <string>		// std::string
#include <iostream>		// std::cout

#define K7_TRD
#include "../include/xpmon_be.h" // TestCmd

#define DEV_FILE	"/dev/xdma_stat"

struct Options
{
    uint32_t    vlvl;
    int         rtprio;
    char *      user;
} gOpts=
    {   vlvl:0x1,
        rtprio:99,
    };

#define USAGE "\
  Usage: %s [options] <cmd> [args]\n\
Example: %s start TEST_START ENABLE_PKTCH ENABLE_PKTGEN ENABLE_LOOPBACK\n\
Options:\n\
 -v             increase verbose print level\n\
 --verbose=<mask>     \n\
 -? -h --help  help/usage\n\
",      basename(argv[0]), basename(argv[0])


int
main(  int	argc
     , char	*argv[] )
{
    std::string cmd;
    int		sts;

    while (1)
    {   int                  option_index=0;
        static struct option long_options[]=
            {   // name,       has_arg,flag,val
                {"help",         0,     0,  'h'},
                {"priority",     1,     0,  'p'},
                {0, 0, 0, 0}
            };
        //                            +=stop parsing at first non-option
        int c = getopt_long(argc,argv,"+?hv",long_options,&option_index);
        if (c == -1)  break;
        switch (c)
        {
        case 'v': gOpts.vlvl = (gOpts.vlvl<<1) | 1; break;
        case '?': case 'h':
            printf( USAGE );  exit( 0 );
            break;
        default:
            printf("?? getopt returned character code %d (0x%x) ??\n", c,c );
        }
    }

    int required_args=1;
    if (optind <= argc-required_args)
    {   // we have args...
	cmd = argv[optind];
	//std::cout << "cmd=" << cmd << '\n';
    } else { printf(USAGE); exit(1); }

    int fd=open(DEV_FILE,O_RDWR);
    if (fd == -1) { perror("open DEV_FILE"); exit(1); }

    TestCmd tc;

    if (cmd == "start")
    {   printf("cmd==start\n"); ++optind;
	tc.TestMode=0;
	while (optind <= argc-required_args)
	{   cmd=argv[optind];
	    if     (cmd == "TEST_START")      tc.TestMode|=TEST_START;
	    else if(cmd == "ENABLE_LOOPBACK") tc.TestMode|=ENABLE_LOOPBACK;
	    else if(cmd == "ENABLE_PKTCHK")   tc.TestMode|=ENABLE_PKTCHK;
	    else if(cmd == "ENABLE_PKTGEN")   tc.TestMode|=ENABLE_PKTGEN;
	    else { printf("shoot; some number?\n");exit(1); }
	    ++optind;
	}
	tc.Engine=0; tc.TestMode=TEST_START|ENABLE_LOOPBACK; tc.MinPktSize=64; tc.MaxPktSize=8*4096;
	sts = ioctl( fd, ISTART_TEST, &tc );
	if (sts == -1) { perror("ISTART_TEST");exit(1); }
	printf("sts=%d\n", sts );
    }
    else if (cmd == "stop")
    {   printf("cmd==stop\n");
	tc.TestMode=0;
	tc.Engine=0; tc.TestMode=TEST_STOP; tc.MinPktSize=64; tc.MaxPktSize=8*4096;
	sts = ioctl( fd, ISTOP_TEST, &tc );
	if (sts == -1) { perror("ISTOP_TEST");exit(1); }
	printf("sts=%d\n", sts );
    }
    else if (cmd == "reg")
    {   //printf("cmd==reg\n");
	++optind;
	// default to read...
	unsigned offset=0;
	if (optind <= argc-required_args) offset=strtoul(argv[optind],NULL,0);
	RegOp regop;
	regop.reg = offset;
	regop.val = 0;  // if read, just init to 0
	regop.loops = 0;
	sts = ioctl( fd, IREG_READ, &regop );
	if (sts == -1) { perror("IREG_READ");exit(1); }
	printf("sts=%d reg=%6d(0x%04x) val=%9d(0x%08x)\n"
	       , sts,offset,offset,regop.val,regop.val );
    }
    else
    {   printf("unknown cmd %s\n", cmd.c_str() );
	exit(1);
    }

    //printf("good bye\n");
    return (0);
}   // main
