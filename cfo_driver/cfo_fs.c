/*  This file (cfo_fs.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Feb  5, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: .emacs.gnu,v $
    rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";
    */

#include <linux/fs.h>		/* alloc_chrdev_region */
#include <linux/device.h>	/* class_create */
#include <linux/cdev.h>		/* cdev_add */
#include <linux/types.h>        /* dev_t */

#include "trace.h"		/* TRACE */
#include "cfo_mmap_ioctl.h"	/* CFO_DEV_FILE */
#include "cfo_fs.h"


static struct file_operations cfo_file_ops =
{   .owner=   THIS_MODULE,
    .llseek=  NULL,           	/* lseek        */
    .read=    NULL,		/* read         */
    .write=   NULL,           	/* write        */
    /*.readdir= NULL,              readdir      */
    .poll=    NULL,             /* poll         */
    .IOCTL_FILE_OPS_MEMBER=cfo_ioctl,/* ioctl  */
    .mmap=    cfo_mmap,   	/* mmap         */
    NULL,                       /* open         */
    NULL,                       /* flush        */
    NULL,                       /* release (close?)*/
    NULL,                       /* fsync        */
    NULL,                       /* fasync       */
    NULL,                       /* check_media_change */
    NULL,                       /* revalidate   */
    NULL                        /* lock         */
};

dev_t         cfo_dev_number;
struct class *cfo_dev_class;
struct cdev   cfo_cdev;

int cfo_fs_up()
{
    int sts;
    sts = alloc_chrdev_region( &cfo_dev_number, 0, 1, "cfo_drv" );

    if(sts < 0)
    {   TRACE( 3, "dcm_init(): Failed to get device numbers" );
        return (sts);
    }
    
    cfo_dev_class = class_create( THIS_MODULE, "cfo_dev" );
    
    cdev_init( &cfo_cdev, &cfo_file_ops );

    cfo_cdev.owner = THIS_MODULE;
    cfo_cdev.ops   = &cfo_file_ops;

    sts = cdev_add ( &cfo_cdev, cfo_dev_number, 1 );
    device_create( cfo_dev_class, NULL, cfo_dev_number, NULL, CFO_DEV_FILE );
    // NOTE: permissions set -- use udev - i.e:
    // echo 'KERNEL=="cfo", MODE="0666"' >/etc/udev/rules.d/98-cfo.rules

    return (0);
}   // cfo_fs


void cfo_fs_down()
{
    device_destroy( cfo_dev_class, cfo_dev_number);
    cdev_del( &cfo_cdev );
    class_destroy( cfo_dev_class);
    unregister_chrdev_region( cfo_dev_number, 1 );
}
