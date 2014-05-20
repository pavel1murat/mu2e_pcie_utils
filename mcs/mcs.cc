 // This file (mcs.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // May 19, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#include <stdio.h>		// printf

int
main(  int	argc
     , char	*argv[] )
{
    if (argc != 2) { printf("usage: %s <mcs_file>\n", basename(argv[0]));return -1;}
    return (0);
}   // main
