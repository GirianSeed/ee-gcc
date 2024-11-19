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

#include "sim-main.h"
#include "sky-device.h"

void 
device_error (device *me, char* fmt, ...) 
{
  char* name = "<none>";
  va_list ap;

  if(me)
    name = me->name;
  fprintf(stderr, "device_error: %s ", name);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr,"\n\n\n");
  exit(1);
}

int
device_io_read_buffer(device *me,
                      void *dest,
                      int space,
                      address_word addr,
                      unsigned nr_bytes,
                      sim_cpu *processor,
                      sim_cia cia)
{
  if (me->io_read_buffer_callback == NULL)
    device_error(me, "no io_read_buffer_callback method");
  return me->io_read_buffer_callback(me, dest, space,
                                      addr, nr_bytes,
                                      processor, cia);
}

int
device_io_write_buffer(device *me,
                       const void *source,
                       int space,
                       address_word addr,
                       unsigned nr_bytes,
                       sim_cpu *processor,
                       sim_cia cia)
{
  if (me->io_write_buffer_callback == NULL)
    device_error(me, "no io_write_buffer_callback method");
  return me->io_write_buffer_callback(me, source, space,
                                       addr, nr_bytes,
                                       processor, cia);
}


/* Send an interrupt event to the CPU from a sky peripheral. */
void
sky_signal_interrupt()
{
  extern void interrupt_event(SIM_DESC, void*);

  SIM_DESC sd = current_state;
  sim_events_schedule(sd, 1, interrupt_event, NULL);
}
