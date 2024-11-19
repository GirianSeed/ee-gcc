/*  Copyright (C) 1998, Cygnus Solutions

    */

#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#include "sim-main.h"

/* in sky-hardware.h */
void register_devices(SIM_DESC sd);

/* in sky-gdb.c */
SIM_RC vu_cmd_install (SIM_DESC sd);
SIM_RC vif_cmd_install (SIM_DESC sd);
SIM_RC log_cmd_install (SIM_DESC sd);

#endif
