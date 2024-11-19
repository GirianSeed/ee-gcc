/*  Copyright (C) 1998, Cygnus Solutions   */

#ifndef SKY_GDB_H_
#define SKY_GDB_H_

#include "sim-main.h"

void gdb_attach ( SIM_DESC sd );

void sky_command_options_open ( SIM_DESC sd );
void sky_command_options_close ( SIM_DESC sd );
void sky_store_file_name (char **, char *);
void sky_open_file ( FILE **, char *, char *, int );

/* Sky specific command line options - common define.  */
enum {
  SKY_OPT_GS_REFRESH1 = 1,
  SKY_OPT_GS_REFRESH2,
  SKY_OPT_GS_ENABLE,
  SKY_OPT_GS_DISABLE,
  SKY_OPT_GIF_REFRESH_ON,
  SKY_OPT_GIF_REFRESH_OFF,
  SKY_OPT_DEBUG_NAME,
  SKY_OPT_TRACE_ON,
  SKY_OPT_TRACE_OFF,
  SKY_OPT_TRACE_NAME,
  SKY_OPT_GIF_OUTPUT_ON,
  SKY_OPT_GIF_OUTPUT_OFF,
  SKY_OPT_GIF_OUTPUT_NAME,
  SKY_OPT_CLOSE 
};

enum { ORDER_XYZW, ORDER_WZYX };

extern int gdb_vu_pipeorder;
  

#endif
