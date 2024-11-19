/*  Copyright (C) 1998, Cygnus Solutions

    */

#ifndef DEVICE_H_
#define DEVICE_H_

#include "sim-main.h"

typedef int io_read_buffer_callback_type (device *, void *, int,
                        address_word, unsigned ,
                        sim_cpu *, sim_cia);

typedef int io_write_buffer_callback_type (device *, const void *, int,
                        address_word, unsigned ,
                        sim_cpu *, sim_cia);

void device_error (device *me, char* message, ...);
int device_io_read_buffer(device *me,
                      void *dest,
                      int space,
                      address_word addr,
                      unsigned nr_bytes,
                      sim_cpu *processor,
                      sim_cia cia);
int device_io_write_buffer(device *me,
                       const void *source,
                       int space,
                       address_word addr,
                       unsigned nr_bytes,
                       sim_cpu *processor,
                       sim_cia cia);

struct _device {
    char *name;
    io_read_buffer_callback_type *io_read_buffer_callback;
    io_write_buffer_callback_type *io_write_buffer_callback;
};


/* send an interrupt to the CPU from a sky peripheral */
void sky_signal_interrupt();


#endif
