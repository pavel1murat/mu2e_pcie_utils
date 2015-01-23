 // This file (mu2e_fs.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Feb  5, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#ifndef MU2E_FS_H
#define MU2E_FS_H

int  mu2e_mmap( struct file *file, struct vm_area_struct *vma );

int  mu2e_ioctl(  struct inode *inode, struct file *filp
		, unsigned int cmd, unsigned long arg );

int  mu2e_fs_up( void );

void mu2e_fs_down( void );

#endif // MU2E_FS_H
