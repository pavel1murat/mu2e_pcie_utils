/*  This file (mu2e_fs.c) was created by Ron Rechenmacher <ron@fnal.gov> on
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
#include "mu2e_mmap_ioctl.h"	/* MU2E_DEV_FILE */
#include "mu2e_fs.h"


static struct file_operations mu2e_file_ops =
{   .owner=   THIS_MODULE,
    .llseek=  NULL,           	/* lseek        */
    .read=    NULL,		/* read         */
    .write=   NULL,           	/* write        */
    .readdir= NULL,             /* readdir      */
    .poll=    NULL,             /* poll         */
    .ioctl=   mu2e_ioctl,       /* ioctl        */
    .mmap=    mu2e_mmap,   	/* mmap         */
    NULL,                       /* open         */
    NULL,                       /* flush        */
    NULL,                       /* release (close?)*/
    NULL,                       /* fsync        */
    NULL,                       /* fasync       */
    NULL,                       /* check_media_change */
    NULL,                       /* revalidate   */
    NULL                        /* lock         */
};

dev_t         mu2e_dev_number;
struct class *mu2e_dev_class;
struct cdev   mu2e_cdev;

int mu2e_fs_up()
{
    int sts;
    sts = alloc_chrdev_region( &mu2e_dev_number, 0, 1, "mu2e_drv" );

    if(sts < 0)
    {   TRACE( 3, "dcm_init(): Failed to get device numbers" );
        return (sts);
    }
    
    mu2e_dev_class = class_create( THIS_MODULE, "mu2e_dev" );
    
    cdev_init( &mu2e_cdev, &mu2e_file_ops );

    mu2e_cdev.owner = THIS_MODULE;
    mu2e_cdev.ops   = &mu2e_file_ops;

    sts = cdev_add ( &mu2e_cdev, mu2e_dev_number, 1 );
    device_create( mu2e_dev_class, NULL, mu2e_dev_number, NULL, MU2E_DEV_FILE );

    return (0);
}   // mu2e_fs


void mu2e_fs_down()
{
    device_destroy( mu2e_dev_class, mu2e_dev_number);
    cdev_del( &mu2e_cdev );
    class_destroy( mu2e_dev_class);
    unregister_chrdev_region( mu2e_dev_number, 1 );
}
