#test `whoami` == root || { echo 'You are not root and must be; script returning.'; return; }
test "$0" == "$BASH_SOURCE" && { echo 'You must source this script; script exiting'; exit; }

ROOT_MODE=0
if [[ `whoami` == root ]] || [[ `whoami` == ron2 ]]; then
   ROOT_MODE=1
fi

if [ $ROOT_MODE -eq 1 ]; then
  echo "Cleaning up kernel modules..."
  killall node         2>/dev/null
  rmmod pci_devel_main 2>/dev/null
  rmmod mu2ecfo        2>/dev/null
  
  echo "Reinserting kernel modules..."
  PCIE_KO_PATH=$PCIE_LINUX_KERNEL_MODULE_DIR/drivers/`uname -r`/mu2ecfo.ko
  TRACE_KO_PATH=$TRACE_DIR/module/`uname -r`/TRACE.ko

  if [ ! -e $PCIE_KO_PATH -a ! -z "${MRB_BUILDDIR-}" ]; then
    PCIE_KO_PATH=$MRB_BUILDDIR/pcie_linux_kernel_module/drivers/`uname -r`/mu2ecfo.ko
  fi

  if [ ! -e $TRACE_KO_PATH -a ! -z "${MRB_BUILDDIR-}" ]; then
    TRACE_KO_PATH=$MRB_BUILDDIR/TRACE/module/`uname -r`/TRACE.ko
  fi

  lsmod | grep TRACE -q || insmod $TRACE_KO_PATH trace_allow_printk=1
  lsmod | grep mu2ecfo -q || insmod $PCIE_KO_PATH
  source $TRACE_DIR/script/trace.sh.functions  || source $TRACE_FQ_DIR/bin/trace.sh.functions
  chmod 666 /dev/mu2ecfo
fi

MU2EHOST=`hostname|grep -v mu2edaq01|grep -c mu2edaq`
if [ $MU2EHOST -gt 0 ];then
  test -e /dev/mu2ecfo || { echo 'CFO device file not found. Please re-run this script as root!'; return; }

  echo "Setting up TRACE module"
  export TRACE_FILE=/proc/trace/buffer;tonM -nKERNEL 0-19;toffM -nKERNEL 4  # poll noise is on lvls 22-23
  tonSg 0
  export CFOLIB_SIM_ENABLE=N
else
  export TRACE_FILE=/tmp/trace_buffer_$USER
  export CFOLIB_SIM_ENABLE=1
fi
echo "Doing \"Super\" Reset Chants"
cfo_cntl write 0x9100 0xa0000000  >/dev/null # reset CFO  reset serdes osc
cfo_cntl write 0x9100 0x00000000  >/dev/null  # clear reset

TRACE_NAME=MU2ECFO tonM -nMU2ECFO 0-31; tonM 0-31

CFO_Reset()
{
  cfo_cntl write 0x9100 0xa0000000  >/dev/null # reset CFO  reset serdes osc
  cfo_cntl write 0x9118 0x0000003f  >/dev/null  # SERDES resets
  cfo_cntl write 0x9100 0x00000000  >/dev/null  # clear reset
  cfo_cntl write 0x9100 0x10000000  >/dev/null  # select 2.5Gb/select
  cfo_cntl write 0x9100 0x30000000  >/dev/null # Reset SERDES Osc
  cfo_cntl write 0x9100 0x10000000  >/dev/null
  cfo_cntl write 0x9118 0x00000000  >/dev/null  # clear SERDES reset on link 0
}

echo 'To Reset the CFO, use CFO_Reset'
echo 'To see the current state of all of the CFO Registers, use CFORegDump'
