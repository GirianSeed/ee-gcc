/****************************************************************************/
/*                                                                          */
/*             Sony Computer Entertainment CONFIDENTIAL                     */
/*      (C) 1997 Sony Computer Entertainment Inc. All Rights Reserved       */
/*                                                                          */
/*      VPE1 simulator (part of VU1) global variables                       */
/*                                                                          */
/****************************************************************************/

#ifndef LIBVPE_H_
#define LIBVPE_H_

#include "sky-vu.h"

void initvpe (vu_device * me);

void vpecallms_init (vu_device * me);
void vpecallms_cycle (SIM_DESC sd, vu_device * me);

#endif
