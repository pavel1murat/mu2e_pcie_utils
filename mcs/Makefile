 # This file (Makefile<2>) was created by Ron Rechenmacher <ron@fnal.gov> on
 # May 19, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: .emacs.gnu,v $
 # rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";


CFLAGS=-I${TRACE_INC} -I${MU2E_PCIE_INC}

# build depends for main exe (not .o file)
%.d: %.cc
	set -e; rm -f $@; \
	$(CXX) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1 $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

mainre       = '(^| |   )main\ *\('
c++exesrc   := $(shell egrep -l $(mainre) *.cc 2>/dev/null)
include $(c++exesrc:.cc=.d)

mcs: mcs.cc
	$(CXX) -std=c++0x ${CFLAGS} -Wall -pedantic -g -o mcs mcs.cc


clean:
	rm -f mcs mcs.[od]
