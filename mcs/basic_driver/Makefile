 # This file (Makefile) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Feb  5, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: .emacs.gnu,v $
 # rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

# Ref. Documentation/kbuild/modules.txt
# Ref. http://www.tldp.org/LDP/lkmpg/2.6/html/  (2007-05-18 ver 2.6.4)
# Module files are specified in the Kbuild file.

# The default "KDIR" is based on the current running kernel (via uname -r)
# Specify another kernel via: make KDIR=/usr/local/src/linux-3.7.1

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
EXTRA_CFLAGS=-I${TRACE_INC}
KBUILD_EXTRA_SYMBOLS=${TRACE_DIR}/module/$(shell uname -r)/Module.symvers

# to allow: make trace_proc.lst    # not working?
#           make trace_proc.o
#           make TRACE.ko
# Ref. Documentation/kbuild/modules.txt
%.o %.lst %.ko:
	$(MAKE) -C $(KDIR) M=$(PWD) EXTRA_CFLAGS=-I$(PWD)/../include $@

all default:
	$(MAKE) -C $(KDIR) M=$(PWD) EXTRA_CFLAGS=$(EXTRA_CFLAGS) KBUILD_EXTRA_SYMBOLS=$(KBUILD_EXTRA_SYMBOLS)

#.PHONY: /
#/:
#	$(MAKE) -C $(KDIR) M=$(PWD) $@

# After testing with just "insmod TRACE.ko". Of course, this needs to be done
# as user root.
install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install

clean help modules modules_install:
	$(MAKE) -C $(KDIR) M=$(PWD) $@
