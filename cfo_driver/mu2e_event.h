 // This file (mu2e_init.h) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Feb  5, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#ifndef MU2E_EVENT_H
#define MU2E_EVENT_H

#include <linux/wait.h>		// wait_queue_head_t
extern wait_queue_head_t get_info_wait_queue;

int  mu2e_sched_poll( void );
int mu2e_force_poll(void);
int  mu2e_event_up( void );
void mu2e_event_down( void );

#endif // MU2E_EVENT_H
