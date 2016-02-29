#! /bin/sh
 # This file (mcs.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Jan  3, 2015. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: .emacs.gnu,v $
 # rev='$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $'
set -u
test $# -eq 1 || { echo "usage: `basename $0` <file>"; exit; }

file=$1; shift

# look for pci_devel_main.ko
fdir= DEVMOD=
if   [ -d "${CETPKG_BUILD-}" ];then
    fdir="${CETPKG_BUILD-}"
elif [ -d "${MRB_BUILDDIR-}" ]; then
    fdir="${MRB_BUILDDIR-}"
elif [ -d "${PCIE_LINUX_KERNEL_MODULE_DIR-}" ];then
    fdir="${PCIE_LINUX_KERNEL_MODULE_DIR-}"
fi
test -n "$fdir" && DEVMOD=`find $fdir -name pci_devel_main.ko | head -1`
if [ -z "$DEVMOD" ];then
   echo "ERROR - can't find pci_devel_main.ko"
   exit
fi

# find the mcs and devl executables
if type mcs >/dev/null;then :;else
    echo "ERROR - mcs and devl executables not found - setup pcie_linux_kernel_driver"
    exit
fi


pids=`lsof -t /dev/mu2e 2>/dev/null` && kill $pids

lsmod | grep pci_devel_main >/dev/null && rmmod pci_devel_main
sleep 1
lsmod | grep mu2e           >/dev/null && rmmod mu2e
lsmod | grep TRACE          >/dev/null && rmmod TRACE

if [ -z $MRB_BUILDDIR ]; then

  # (re)setup TRACE (no get functions defined in this script
  . /mu2e/ups/setup
  test -n "${SETUP_TRACE-}"\
   && { xx=$SETUP_TRACE; unsetup TRACE; eval setup $xx;}\
   || setup TRACE

    lsmod | grep TRACE -q || insmod $TRACE_DIR/module/`uname -r`/TRACE.ko trace_allow_printk=1
  else
    lsmod | grep TRACE -q || insmod $MRB_BUILDDIR/TRACE/module/`uname -r`/TRACE.ko trace_allow_printk=1
    source $TRACE_DIR/script/trace.sh.functions 
  fi

export TRACE_FILE=/proc/trace/buffer
tonSg 0-7; tonMg 0-15


insmod $DEVMOD

echo 'reg 0x9000 value...'
devl uint32 0x9000
echo 'reg 0x9004 value...'
devl uint32 0x9004

export TRACE_NAME=mcs
mcs $file | tee /tmp/mcs.out\
 |{ xx=11;while xx=`expr $xx - 1`;do IFS= read ln;echo "$ln";done;echo ...;tail;}

echo "Now Power Cycle!!!!!!!!!"
