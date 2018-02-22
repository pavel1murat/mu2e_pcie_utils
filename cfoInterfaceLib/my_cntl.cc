// This file (my_cntl.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Feb  6, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: .emacs.gnu,v $
static char* rev=(char*)"$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#include <stdio.h>		// printf
#include <fcntl.h>		// open, O_RDONLY
#include <libgen.h>		// basename
#include <string.h>		// strcmp
#include <stdlib.h>		// strtoul
#include <getopt.h>             // getopt_long

#include "cfodev.h"		// cfodev
#include "cfo_driver/cfo_mmap_ioctl.h"	// m_ioc_cmd_t

#define USAGE "\
   usage: %s <start|stop|dump>\n\
          %s read <offset>\n\
          %s write <offset> <val>\n\
          %s write_loopback_data [-p<packet_cnt>] [data]...\n\
examples: %s start\n\
", basename(argv[0]), basename(argv[0]), basename(argv[0]), basename(argv[0]), basename(argv[0])




int
main(  int	argc
	   , char	*argv[] )
{
  int                sts=0;
  int                fd;
  char              *cmd;
  m_ioc_cmd_t        ioc_cmd;
  m_ioc_reg_access_t reg_access; 
  cfodev            dev;

  int         opt_v=0;
  int         opt;
  int		opt_packets=8;
  while (1)
    {   int option_index = 0;
	  static struct option long_options[] =
        {   /* name,   has_arg, int* flag, val_for_flag */
		  {"mem-to-malloc",1,    0,         'm'},
		  {0,            0,      0,          0},
        };
	  opt = getopt_long (argc, argv, "?hm:Vv",
						 long_options, &option_index);
	  if (opt == -1) break;
	  switch (opt)
        {
        case '?': case 'h':  printf( USAGE );  exit( 0 ); break;
        case 'V': printf("%s\n",rev);return(0);           break;
        case 'v': opt_v++;                                break;
		case 'p': opt_packets=strtoul(optarg,NULL,0);     break;
        default:  printf ("?? getopt returned character code 0%o ??\n", opt);
        }
    }
  if (argc - optind < 1)
    {   printf( "Need cmd\n" );
	  printf( USAGE ); exit( 0 );
    }    
  cmd = argv[optind++];
  printf( "cmd=%s\n", cmd );
  printf( "opt_packets=%i\n", opt_packets);

  fd = open( "/dev/" CFO_DEV_FILE, O_RDONLY );
  if (fd == -1) { perror("open /dev/" CFO_DEV_FILE); return (1); }

  if      (strcmp(cmd,"start") == 0)
    {
	  sts = ioctl( fd, M_IOC_TEST_START, &ioc_cmd );
    }
  else if (strcmp(cmd,"stop") == 0)
    {
	  sts = ioctl( fd, M_IOC_TEST_STOP, &ioc_cmd );
    }
  else if (strcmp(cmd,"read") == 0)
    {
	  if (argc < 3) { printf(USAGE); return (1); }
	  reg_access.reg_offset = strtoul(argv[2],NULL,0);
	  reg_access.access_type = 0;
	  sts = ioctl( fd, M_IOC_REG_ACCESS, &reg_access );
	  if (sts) { perror("ioctl M_IOC_REG_ACCESS read"); return (1); }
	  printf( "0x%08x\n", reg_access.val );
    }
  else if (strcmp(cmd,"write") == 0)
    {
	  if (argc < 4) { printf(USAGE); return (1); }
	  reg_access.reg_offset  = strtoul(argv[2],NULL,0);
	  reg_access.access_type = 1;
	  reg_access.val         = strtoul(argv[3],NULL,0);
	  sts = ioctl( fd, M_IOC_REG_ACCESS, &reg_access );
	  if (sts) { perror("ioctl M_IOC_REG_ACCESS write"); return (1); }
    }
  else if (strcmp(cmd,"dump") == 0)
    {
	  sts = ioctl( fd, M_IOC_DUMP );
	  if (sts) { perror("ioctl M_IOC_REG_ACCESS write"); return (1); }
    }
  else
    {
	  printf("unknown cmd %s\n", cmd ); return (1);
    }

  printf( "sts=%d\n", sts );
  return (0);
}   // main
