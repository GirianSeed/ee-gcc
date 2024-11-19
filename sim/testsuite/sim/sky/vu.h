/*  Copyright (C) 1998, Cygnus Solutions  */

#ifndef VU_H
#define VU_H

#define VPU_STAT		((volatile int *) 0xb10073d0)
#define VPU_STAT_VBS0_MASK	0x00000001
#define VPU_STAT_VBS1_MASK	0x00000100

/* MEM0 is instruction memory */
#define VU0_MEM0_WINDOW_START 0xb1000000
#define VU0_MEM0_SIZE         4096

/* MEM1 is data memory */
#define VU0_MEM1_WINDOW_START 0xb1004000
#define VU0_MEM1_SIZE         4096

#endif /* VU_H */
