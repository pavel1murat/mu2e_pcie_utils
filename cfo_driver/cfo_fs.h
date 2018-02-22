 // This file (cfo_fs.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Feb  5, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#ifndef CFO_FS_H
#define CFO_FS_H

#include <linux/version.h>      /* KERNEL_VERSION */

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,39)
# define IOCTL_ARGS( inode, filep, cmd, arg ) inode, filep, cmd, arg
# define IOCTL_FILE_OPS_MEMBER ioctl
# define IOCTL_RET_TYPE                       int
#else
# define IOCTL_ARGS( inode, filep, cmd, arg )        filep, cmd, arg
# define IOCTL_FILE_OPS_MEMBER unlocked_ioctl
# define IOCTL_RET_TYPE                       long
#endif

IOCTL_RET_TYPE cfo_ioctl(IOCTL_ARGS(  struct inode *inode, struct file *filp
				     , unsigned int cmd,    unsigned long arg));
int  cfo_mmap( struct file *file, struct vm_area_struct *vma );
int  cfo_fs_up( void );

void cfo_fs_down( void );

#endif // CFO_FS_H
