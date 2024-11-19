/* Copyright (C) 1998, Cygnus Solutions */

#ifndef VIF_H
#define VIF_H

#define VIF0_STAT		((volatile int *) 0xb0003800)

#define VIF0_STAT_FQC_MASK	0x1F000000
#define VIF0_STAT_PPS_MASK	0x00000003

#define VIF1_STAT		((volatile int *) 0xb0003C00)

#define VIF1_STAT_FQC_MASK	0x1F000000
#define VIF1_STAT_PPS_MASK	0x00000003

#endif /* VIF_H */
