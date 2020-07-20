#!/bin/bash

# Configuration
PCIE_VERSION=${PCIE_VERSION:-v2_02_06}
PCIE_QUALS=${PCIE_QUALS:-e19:s87:prof}
EPICS_VERSION=${EPICS_VERSION:-v3_15_5}
EPICS_QUALS=${EPICS_QUALS:-e14}
DTC_TEMP_THRESHOLD=${DTC_TEMP_THRESHOLD:-65}
DTC_MAX_TEMP=255
FIREFLY_TEMP_THRESHOLD=${FIREFLY_TEMP_THRESHOLD:-65}
FIREFLY_MAX_TEMP=120
VERBOSE_SET=${VERBOSE+1}
VERBOSE=${VERBOSE:-0}
HOSTID=`hostname|sed 's/^mu2e//'|sed 's/[.].*$//'`
export EPICS_CA_AUTO_ADDR_LIST=NO
export EPICS_CA_ADDR_LIST=192.168.157.0
  
# Default to verbose if run interactively
if [ -z "$VERBOSE_SET" ] && [ $VERBOSE -eq 0 ] && [ -t 0 ];then
  VERBOSE=1
fi

# From NOvA's Loadshed script
MAIL_RECIPIENTS_ERROR="eflumerf@fnal.gov,mu2e_tdaq_developers@fnal.gov" # comma-separated list
function send_error_mail()
{
        echo "$2"| mail -s "`date`: $1" $MAIL_RECIPIENTS_ERROR
}


source /mu2e/ups/setup
setup epics $EPICS_VERSION -q$EPICS_QUALS
setup pcie_linux_kernel_module $PCIE_VERSION -q$PCIE_QUALS

for ii in {0..3}; do

  if [ -e /dev/mu2e$ii ]; then
    
    if [[ $VERBOSE -ne 0 ]]; then
      echo "Reading temperatures for $HOSTNAME DTC $ii"
    fi

    dtctemp=`my_cntl read 0x9010|grep 0x`
    tempvalue=`echo "print int(round(($dtctemp * 503.975 / 4096) - 273.15))"|python -`

    if [[ $VERBOSE -ne 0 ]]; then
      echo "DTC Temperature: $tempvalue"
    fi
    if [[ $tempvalue -gt $DTC_TEMP_THRESHOLD ]] && [[ $tempvalue -le $DTC_MAX_TEMP ]]; then
       errstring="DTC Overtemp $HOSTNAME:/dev/mu2e$ii: $tempvalue!"
    fi
    if [[ ${#EPICS_BASE} -ne 0 ]]; then
	caput Mu2e:CompStatus:$HOSTID:DTC$ii:dtctemp $tempvalue >/dev/null
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
    if [[ $tempvalue -gt $FIREFLY_TEMP_THRESHOLD ]] && [[ $tempvalue -le $FIREFLY_MAX_TEMP ]]; then
      errstring="${errstring+$errstring$'\n'}RX Firefly Overtemp $HOSTNAME:/dev/mu2e$ii: $tempvalue!"
    fi
    if [[ ${#EPICS_BASE} -ne 0 ]]; then
	caput Mu2e:CompStatus:$HOSTID:DTC$ii:rxtemp $tempvalue >/dev/null
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
    if [[ $tempvalue -gt $FIREFLY_TEMP_THRESHOLD ]] && [[ $tempvalue -le $FIREFLY_MAX_TEMP ]]; then
      errstring="${errstring+$errstring$'\n'}TX Firefly Overtemp $HOSTNAME:/dev/mu2e$ii: $tempvalue!"
    fi
    if [[ ${#EPICS_BASE} -ne 0 ]]; then
	caput Mu2e:CompStatus:$HOSTID:DTC$ii:txtemp $tempvalue >/dev/null
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
    if [[ $tempvalue -gt $FIREFLY_TEMP_THRESHOLD ]] && [[ $tempvalue -le $FIREFLY_MAX_TEMP ]]; then
      errstring="${errstring+$errstring$'\n'}TX/RX Firefly Overtemp $HOSTNAME:/dev/mu2e$ii: $tempvalue!"
    fi
    if [[ ${#EPICS_BASE} -ne 0 ]]; then
	caput Mu2e:CompStatus:$HOSTID:DTC$ii:rxtxtemp $tempvalue >/dev/null
    fi

    if [ ${#errstring} -gt 0 ]; then
      send_error_mail "Temperature Errors on DTC $ii on $HOSTNAME" "$errstring"
      if [ $VERBOSE -ne 0 ];then
        echo "Temperature Errors on DTC $ii on $HOSTNAME: $errstring" >&2
      fi
    fi
  fi

done
