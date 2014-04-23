 // This file (dvl_ioctl.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Feb  5, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#ifndef DVL_IOCTL_H
#define DVL_IOCTL_H

#ifdef __KERNEL__
# include <asm/ioctl.h>		// _IOWR
#else
# include <sys/ioctl.h>		// _IOWR
# include <sys/types.h>
# include <unistd.h>		// sysconf
#endif


#define PCIDEVL_DEV_FILE       "pcidev"


/*
 For _IO,_IOR,_IOW,_IORW ref. Documentation/ioctl/ioctl-number.txt
 _IO   - implementation has neither copy_from nor copy_to user (or equivalents)
 _IOR  - implementation has copy_to_user   (or equiv., at end)
 _IOW  - implementation has copy_from_user (or equiv., at beginnning)
 _IOWR - implementaions has both copy_from_user (at beginnning) and
         copy_to_user (at end) 
NOTE: for _IOR, _IOW: the size is only for the data at the address used in the
      ioctl call; NOT for the size at an address contained within the data
      pointed to by the address used in the ioctl call.  So, if a small
      structure is pointed to (to be copied in) which has an address of a
      large buffer, the only thing that these macros should consider is the
      pointer used (directly) in the ioctl call. 
 */
#define DVL_IOC_MAGIC 'D'



#endif // DVL_IOCTL_H
