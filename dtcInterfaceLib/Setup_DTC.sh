test `whoami` == root || { echo 'You are not root and must be; script returning.'; return; }

echo First, cleanup...
killall node         2>/dev/null
rmmod pci_devel_main 2>/dev/null
rmmod mu2e           2>/dev/null
rmmod TRACE          2>/dev/null
. /mu2e/ups/setup
unsetup_all           >/dev/null

echo -e '\nNow, (re)setup...'
. ~ron/.profile >/dev/null 2>&1

#setup -j TRACE -qe6 v3_05_00
setup gcc v4_9_2
setup TRACE -qe7 v3_05_00 # with -j, the mrbsetenv then does not setup gcc
lsmod | grep TRACE -q || insmod $TRACE_DIR/module/`uname -r`/TRACE.ko trace_allow_printk=1
export TRACE_FILE=/proc/trace/buffer;tonM -nKERNEL 0-19  # poll noise is on lvls 22-23
tonSg 0

if true;then
  Base=/home/ron/work/mu2ePrj/mrb              # this is the development directory
  setup mrb                                    # $MRB_DIR used in next setup script
  . $Base/local*/setup                         # "base" variable used by this script and is unset; MRB_BUILDDIR, etc set.
  mrbsetenv                                    # add (most) BUILDDIR dirs to PATHs. Overrrides UPS_OVERRIDE.
  #setup gcc v4_9_1
  LD_LIBRARY_PATH=$MRB_BUILDDIR/lib:$LD_LIBRARY_PATH  # mrb bug? fix
  lsmod | grep mu2e  -q || insmod $MRB_BUILDDIR/pcie_linux_kernel_module/drivers/`uname -r`/mu2e.ko
else
  setup pcie_linux_kernel_module -q e7:debug
  lsmod | grep mu2e -q || insmod $PCIE_LINUX_KERNEL_MODULE_FQ_DIR/drivers/`uname -r`/mu2e.ko
fi

echo "Doing \"Super\" Reset Chants"
my_cntl write 0x9100 0xa0000000  >/dev/null # reset DTC  reset serdes osc
my_cntl write 0x9100 0x00000000  >/dev/null  # clear reset
rocUtil toggle_serdes
rocUtil reset_roc
rocUtil write_register -a 14 -w 0x010

TRACE_NAME=MU2EDEV tonM -nMU2EDEV 0-31; tonM 0-31

DTC_Test()
{
    treset;tmodeM 1;DTCLIB_SIM_ENABLE=N mu2eUtil buffer_test -a 0 "$@"
}

DTC_Test2()
{
    treset;tmodeM 1;DTCLIB_SIM_ENABLE=N mu2eUtil buffer_test -a 0 -n 1 -c 50 -S -q -t "$@"
}
DTC_Test3()
{
    treset;tmodeM 1;DTCLIB_SIM_ENABLE=N mu2eUtil buffer_test -a 0 -n 1 -c 200 -S -Q -T 2 
}
DTC_ReadFile()
{
    od -x $@|less
}
DTC_Test_ROC_emulation()
{   # sim modes -- see DTC.cc:DTC
    treset;tmodeM 1;DTCLIB_SIM_ENABLE=R mu2eUtil "$@"
}

DTC_Reset()
{
#    my_cntl write 0x9100 0x30000000 >/dev/null;: Oscillator resets;\
#    my_cntl write 0x9100 0x80000000 >/dev/null;: DTC reset, Clear Oscillator resets;\
#    my_cntl write 0x9100 0x00000000 >/dev/null;: Clear DTC reset;\
#    my_cntl write 0x9118 0x0000003f >/dev/null;: Reset all links;\
#    my_cntl write 0x9118 0x00000000 >/dev/null;: Clear Link Resets

  my_cntl write 0x9100 0xa0000000  >/dev/null # reset DTC  reset serdes osc
  my_cntl write 0x9118 0x0000003f  >/dev/null  # SERDES resets
  my_cntl write 0x9100 0x00000000  >/dev/null  # clear reset
  my_cntl write 0x9100 0x10000000  >/dev/null  # select 2.5Gb/select
  my_cntl write 0x9100 0x30000000  >/dev/null # Reset SERDES Osc
  my_cntl write 0x9100 0x10000000  >/dev/null
  my_cntl write 0x9118 0x00000000  >/dev/null  # clear SERDES reset on link 0
}

ROC_Reset()
{
    rocUtil toggle_serdes
    rocUtil reset_roc
    rocUtil write_register -a 14 -w 0x010
}

DTC_TestDDR() { 
     treset;
     tonM1;
     DTCLIB_SIM_ENABLE=N mu2eUtil buffer_test -a 0 -n 1 -c 10 -S -q -T 4
 }

DTC_TestSRAM() { 
    treset;
    tonM1;
    DTCLIB_SIM_ENABLE=N mu2eUtil buffer_test -a 0 -n 1 -c 10 -S -q -T 3 
}

DTC_Links() { 
    DTCRegDump|grep -A1 "RX Buffer Status" 
}

echo '\nIf there were no errors, you should now be able to test with: DTC_Test [NumReqs]'
echo '       or DTC_Test_ROC_emulation [NumReqs]'
echo 'Example: DTC_Test -n 1       # send       1 read-out/data request pair'
echo '         DTC_Test -n 1000000 -f /tmp/mu2etest.bin # send 1000000 read-out/data request pair, log binary data to /tmp/mu2etest.bin'
echo '         DTC_Test2 -f /tmp/mu2etest.bin # Same as DTC_Test -n 1 -q -c 50 -t -f /tmp/mu2etest.bin'
echo '         DTC_Test3 -f /tmp/mu2eraw.bin # Same as DTC_Test -n 1 -c 200 -S -Q -T 2 -f /tmp/mu2eraw.bin'
echo 'Use DTC_TestDDR and DTC_TestSRAM for RAM Error Checking modes'
echo 'Use DTC_Links to see which of Ring0/1 are currently connected'
echo 'and reset with DTC_Reset and ROC_Reset'
echo 'Use "DTC_Test -h" to see other options.'
echo
echo 'To see the current state of all of the DTC Registers, use DTCRegDump'
echo
echo 'To view a file saved with DTC_Test -f, use DTC_ReadFile <filename>'
echo
