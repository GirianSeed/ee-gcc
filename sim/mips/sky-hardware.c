/*  Copyright (C) 1998, Cygnus Solutions
*/

#include "sky-gpuif.h"
#include "sky-dma.h"
#include "sky-vif.h"
#include "sky-vu.h"
#include "sky-gs.h"
#include "sky-gdb.h"
#include "sky-psio.h"
#include "sky-hardware.h"

void 
register_devices(SIM_DESC sd)
{
  /* Attach a bunch of devices... */
  gif_attach(sd);
  dma_attach(sd);
  vu0_attach(sd);
  vu1_attach(sd);
  vif0_attach(sd);
  vif1_attach(sd);
  gs_attach(sd);
  gdb_attach(sd);
  psio_attach(sd);
}
