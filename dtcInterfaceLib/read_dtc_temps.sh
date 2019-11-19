#!/bin/bash

# Configuration
PCIE_VERSION=${PCIE_VERSION:-v2_02_06}
PCIE_QUALS=${PCIE_QUALS:-e19:s87:prof}
DTC_TEMP_THRESHOLD=${DTC_TEMP_THRESHOLD:-65}
FIREFLY_TEMP_THRESHOLD=${FIREFLY_TEMP_THRESHOLD:-65}
VERBOSE=${VERBOSE:-0}


# From NOvA's Loadshed script
MAIL_RECIPIENTS_ERROR="eflumerf@fnal.gov,mu2e_tdaq_developers@fnal.gov" # comma-separated list
function send_error_mail()
{
        echo "$2"| mail -s "`date`: $1" $MAIL_RECIPIENTS_ERROR
}


source /mu2e/ups/setup
setup pcie_linux_kernel_module $PCIE_VERSION -q$PCIE_QUALS

for ii in {0..3}; do

  if [ -e /dev/mu2e$ii ]; then
    
    if [[ $VERBOSE -ne 0 ]]; then
      echo "Reading temperatures for $HOSTNAME DTC $ii"
    fi

    errstring=
    dtctemp=`my_cntl read 0x9010|grep 0x`
    tempvalue=`echo "print int(round(($dtctemp * 503.975 / 4096) - 273.15))"|python -`

    if [[ $VERBOSE -ne 0 ]]; then
      echo "DTC Temperature: $tempvalue"
    fi
    if [[ $tempvalue -gt $DTC_TEMP_THRESHOLD ]]; then
       errstring="DTC Overtemp $HOSTNAME:/dev/mu2e$ii: $tempvalue!"
    fi

    # RX Firefly
    #enable IIC on Firefly
    my_cntl write 0x93a0 0x00000200 >/dev/null 2>&1
    #Device address, register address, null, null
    my_cntl write 0x9298 0x54160000 >/dev/null 2>&1
    #read enable
    my_cntl write 0x929c 0x00000002 >/dev/null 2>&1
    #disable IIC on Firefly
    my_cntl write 0x93a0 0x00000000 >/dev/null 2>&1
    #read data: Device address, register address, null, temp in 2's compl.
    rxtempval=`my_cntl read 0x9298|grep 0x`
    tempvalue=`echo "print int($rxtempval & 0xFF)"|python -`

    if [[ $VERBOSE -ne 0 ]]; then
      echo "RX Firefly temperature: $tempvalue"
    fi
    if [[ $tempvalue -gt $FIREFLY_TEMP_THRESHOLD ]]; then
      errstring="${errstring+$errstring\n}RX Firefly Overtemp $HOSTNAME:/dev/mu2e$ii: $tempvalue!"
    fi

    # TX Firefly
    my_cntl write 0x93a0 0x00000100 >/dev/null 2>&1
    my_cntl write 0x9288 0x50160000 >/dev/null 2>&1
    my_cntl write 0x928c 0x00000002 >/dev/null 2>&1
    my_cntl write 0x93a0 0x00000000 >/dev/null 2>&1
    txtempval=`my_cntl read 0x9288|grep 0x`
    tempvalue=`echo "print int($txtempval & 0xFF)"|python -`

    if [[ $VERBOSE -ne 0 ]]; then
      echo "TX Firefly temperature: $tempvalue"
    fi
    if [[ $tempvalue -gt $FIREFLY_TEMP_THRESHOLD ]]; then
      errstring="${errstring+$errstring\n}TX Firefly Overtemp $HOSTNAME:/dev/mu2e$ii: $tempvalue!"
    fi
    
    # TX/RX Firefly
    my_cntl write 0x93a0 0x00000400 >/dev/null 2>&1
    my_cntl write 0x92a8 0x50160000 >/dev/null 2>&1
    my_cntl write 0x92ac 0x00000002 >/dev/null 2>&1
    my_cntl write 0x93a0 0x00000000 >/dev/null 2>&1
    txrxtempval=`my_cntl read 0x92a8|grep 0x`
    tempvalue=`echo "print int($txrxtempval & 0xFF)"|python -`

    if [[ $VERBOSE -ne 0 ]]; then
      echo "TX/RX Firefly temperature: $tempvalue"
    fi
    if [[ $tempvalue -gt $FIREFLY_TEMP_THRESHOLD ]]; then
      errstring="${errstring+$errstring\n}TX/RX Firefly Overtemp $HOSTNAME:/dev/mu2e$ii: $tempvalue!"
    fi

    if [ ${#errstring} -gt 0 ]; then
      send_error_mail "Temperature Errors on DTC $ii on $HOSTNAME" "$errstring"
    fi
  fi

done
