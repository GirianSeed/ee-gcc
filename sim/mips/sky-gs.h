/*  Copyright (C) 1998, Cygnus Solutions                             */

#ifndef SKY_GS_H_
#define SKY_GS_H_

#include "sim-main.h"

/* GS register address space / boundaries.                           */
#define GS_REGISTER_WINDOW_START 0x12000000
#define GS_REGISTER_WINDOW_END   0x120013ff
#define GS_REGISTER_WINDOW_SIZE (GS_REGISTER_WINDOW_END - GS_REGISTER_WINDOW_START)

/* Register addresses for use when reading / writing.                */
#define GS_REG_PMODE      0x12000000                                 
#define GS_REG_SMODE1     0x12000010    /* NOT VALID IN SIMULATOR    */ 
#define GS_REG_SMODE2     0x12000020    /* NOT VALID IN SIMULATOR    */
#define GS_REG_SRFSH      0x12000030    /* NOT VALID IN SIMULATOR    */
#define GS_REG_SYNCH1     0x12000040    /* NOT VALID IN SIMULATOR    */
#define GS_REG_SYNCH2     0x12000050    /* NOT VALID IN SIMULATOR    */
#define GS_REG_SYNCV      0x12000060    /* NOT VALID IN SIMULATOR    */
#define GS_REG_DISPFB1    0x12000070
#define GS_REG_DISPLAY1   0x12000080
#define GS_REG_DISPFB2    0x12000090
#define GS_REG_DISPLAY2   0x120000a0
#define GS_REG_EXTBUF     0x120000b0    /* NOT VALID IN SIMULATOR    */
#define GS_REG_EXTDATA    0x120000c0    /* NOT VALID IN SIMULATOR    */
#define GS_REG_EXTWRITE   0x120000d0    /* NOT VALID IN SIMULATOR    */
#define GS_REG_BGCOLOR    0x120000e0    /* NOT VALID IN SIMULATOR    */
#define GS_REG_CSR        0x12001000    /* NOT VALID IN SIMULATOR    */
#define GS_REG_IMR        0x12001010    /* NOT VALID IN SIMULATOR    */
#define GS_REG_BUDIR      0x12001040    /* NOT VALID IN SIMULATOR    */
#define GS_REG_SIGID      0x12001080    /* NOT VALID IN SIMULATOR    */
#define GS_REG_LABELID    0x12001090    /* NOT VALID IN SIMULATOR    */
#define GS_REG_SYSCNT     0x120010f0    /* NOT VALID IN SIMULATOR    */

/* GS registers are 64 bits.                                         */
#define GS_REGISTER_BYTES 8
 
/* Complete GS device.                                               */
struct GS_devicefull {
  
  device  dev;                      /* Standard device definition    */
  FILE   *debug_file;               /* GS debug file                 */
  char   *debug_file_name;          /* GS debug file name            */
};

extern struct GS_devicefull gs_device;

#define GS_DEBUG(string,var)                                          \
        {                                                             \
          if (( gs->debug_file == NULL ) &&                           \
              ( gs->debug_file_name != NULL ))                        \
            sky_open_file (&gs->debug_file,gs->debug_file_name,       \
                           (char *) NULL, _IOLBF );                   \
                                                                      \
          fprintf ((gs->debug_file) ? gs->debug_file : stdout,        \
                    string,var);                                      \
        }


/* External function declaration.                                    */
void gs_attach ( SIM_DESC sd );
void gs_options ( struct GS_devicefull *gs, unsigned_4 option,
                  char  *input_string );
void gs_reset ();

#endif
