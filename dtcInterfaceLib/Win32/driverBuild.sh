#!/bin/bash

if [ $# -lt 1 ];then
    echo "ERROR: You must specify a command (build, rebuild, clean)"
    exit 1
fi
command=$1;shift

source /home/eflumerf/products/setups
source /home/eflumerf/Desktop/mu2e-mrb-base/products/setups
setup mrb
source /home/eflumerf/Desktop/mu2e-mrb-base/localProducts_mu2e_*/setup
source mrbSetEnv

cd $MRB_BUILDDIR/pcie_linux_kernel_module/mu2e_driver
if [[ "$command" == "build" ]];then
make
elif [[ "$command" == "rebuild" ]];then
make clean;make
elif [[ "$command" == "clean" ]];then
make clean
fi
