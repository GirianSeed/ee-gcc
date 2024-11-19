/*  Copyright (C) 1998, Cygnus Solutions

    */

#ifndef SKY_PSIO_H_
#define SKY_PSIO_H_

#include "sim-main.h"

void vio_attach(SIM_DESC sd);
 
typedef struct _ioRegister {
        volatile unsigned int reg;
} IOREGISTER;

#define SIO_DONE        0
#define SIO_START       1

#define PSIO_REGISTER_WINDOW_START 0x1000f160
#define PSIO_SI_CSR 0x1000f160
#define PSIO_SI_CDR 0x1000f170
#define PSIO_SO_CSR 0x1000f1b0
#define PSIO_SO_CDR 0x1000f1f0
#define PSIO_REGISTER_WINDOW_END   0x1000f200

#define PSIO_REGISTER_WINDOW_SIZE (PSIO_REGISTER_WINDOW_END - PSIO_REGISTER_WINDOW_START)

/* DMAC device.                                         */       
struct psio_device
{
  device  dev;             /* Common device definition  */
};  

#endif
