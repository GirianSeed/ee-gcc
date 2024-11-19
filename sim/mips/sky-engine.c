/*  Copyright (C) 1994-1997, Andrew Cagney <cagney@highland.com.au>
    Copyright (C) 1998, Cygnus Solutions

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

    */

#ifndef _ENGINE_C_
#define _ENGINE_C_

#include "sim-inline.c"

#include "sim-main.h"
#include "itable.h"
#include "idecode.h"
#include "semantics.h"
#include "icache.h"
#include "support.h"

#include "sky-vif.h"
#include "sky-vu.h"

static int icount;

static void
cpu_issue (SIM_DESC sd)
{
  sim_cpu *cpu = STATE_CPU (sd, 0);
  instruction_address cia = CIA_GET (cpu);
  instruction_word instruction_0 = IMEM32 (cia);
  extern int sky_cpcond0, sky_cpcond0A;
  int sky_cpcond0_save;

#if defined (ENGINE_ISSUE_PREFIX_HOOK)
  ENGINE_ISSUE_PREFIX_HOOK();
#endif
  /* Current insn may reference the old and current values of cpcond0.
  Save the current value as it may be udpated and we need it to
  set the old value at the end of the cycle. */
  sky_cpcond0_save = sky_cpcond0;

#if (WITH_SMP)
  cia = idecode_issue(cpu, instruction_0, cia);
#else
  cia = idecode_issue(sd, instruction_0, cia);
#endif

  icount++;
  if (cia  == 0) 
    fprintf(stderr, "idecode_issue returned 0 on %dth call\n", icount);

#if defined (ENGINE_ISSUE_POSTFIX_HOOK)
  ENGINE_ISSUE_POSTFIX_HOOK();
#endif
  sky_cpcond0A = sky_cpcond0_save;

  /* Update the instruction address */
  CIA_SET (cpu, cia);

  ++COP0_COUNT;
  /* ??? This is deemed sufficient for now,
     though we miss the case of the interrupt being generated while
     its masked and then unmasked at a later time.  */
  if (COP0_COUNT == COP0_COMPARE
      && ((SR & (status_IE | status_EIE | status_IM7))
	  == (status_IE | status_EIE | status_IM7))
      && !(SR & (status_EXL | status_ERL)))
    SignalExceptionInterrupt (2);
}

typedef void (*issue_fn) (SIM_DESC);

/* Warning: The device ordering should correspond to the enumerated type 
   txvu_cpu_context (sim-main.h -> shadowed from tm-txvu.h).  */
/* For TARGET_SKY_B we leave things as they are to play it safe, and thus
   have vu0_issue, vif0_issue be nops.  */

static issue_fn device_issue[TXVU_CPU_LAST] = {
  cpu_issue, vu0_issue, vu1_issue, vif0_issue, vif1_issue
};
  
INLINE_ENGINE\
(void) engine_run
(SIM_DESC sd,
 int next_cpu_nr,
 int nr_cpus,
 int siggnal)
{
  sim_cpu *cpu = STATE_CPU (sd, 0);
  register int i;
  enum txvu_cpu_context current_cpu = cpu->cur_device;

  current_state = sd;

  while (1)
    {
      /* Cycle around all the devices starting at the current cpu.
	 We always want to go all the way around so things like single
	 stepping do the right thing.  */

      for (i=0; i<TXVU_CPU_LAST; i++) 
	{
	  device_issue[current_cpu] (sd);

	  current_cpu++;
	  if (current_cpu >= TXVU_CPU_LAST)
	    current_cpu = TXVU_CPU_MASTER;

	  /* Set this here in case we fall out of the loop. We want the
	     event checker to see the what we are about to execute, not
	     what we just did.  */

	  cpu->cur_device = current_cpu;	  
	}	 

      /* process any events */
      if (sim_events_tick (sd))
        {
          sim_events_process (sd);
        }
    }
}

#endif /* _ENGINE_C_*/



