/*  Copyright (C) 1998, Cygnus Solutions  */

#ifndef DMA_H
#define DMA_H

#define DMA_D0_CHCR	(volatile int*)0xb0008000
#define DMA_D0_MADR	(volatile int*)0xb0008010
#define DMA_D0_QWC	(volatile int*)0xb0008020
#define DMA_D0_TADR	(volatile int*)0xb0008030
#define DMA_D0_ASR0	(volatile int*)0xb0008040
#define DMA_D0_ASR1	(volatile int*)0xb0008050
#define DMA_D0_PKTFLAG	(volatile int*)0xb0008060	/* virtual reg to indicate presence of tag in data */

#define DMA_D1_CHCR	(volatile int*)0xb0009000
#define DMA_D1_MADR	(volatile int*)0xb0009010
#define DMA_D1_QWC	(volatile int*)0xb0009020
#define DMA_D1_TADR	(volatile int*)0xb0009030
#define DMA_D1_ASR0	(volatile int*)0xb0009040
#define DMA_D1_ASR1	(volatile int*)0xb0009050
#define DMA_D1_PKTFLAG	(volatile int*)0xb0009060	/* virtual reg to indicate presence of tag in data */ 

#define DMA_D2_CHCR	(volatile int*)0xb000a000
#define DMA_D2_MADR	(volatile int*)0xb000a010
#define DMA_D2_QWC	(volatile int*)0xb000a020
#define DMA_D2_TADR	(volatile int*)0xb000a030
#define DMA_D2_ASR0	(volatile int*)0xb000a040
#define DMA_D2_ASR1	(volatile int*)0xb000a050
#define DMA_D2_PKTFLAG	(volatile int*)0xb000a060	/* virtual reg to indicate presence of tag in data */ 

#define DMA_D_CTRL	(volatile int*)0xb000e000
#define DMA_D_CTRL__DMAE	0x00000001
#define DMA_D_STAT	(volatile int*)0xb000e010
#define DMA_D_STAT__TOGGLE	0x63ff0000
#define DMA_D_STAT__CLEAR	0x0000e3ff
#define DMA_D_PCR	(volatile int*)0xb000e020
#define DMA_D_PCR__PCE		0x80000000
#define DMA_D_PCR__CDE		0x03ff0000
#define DMA_D_SQWC	(volatile int*)0xb000e030
#define DMA_D_RBSR	(volatile int*)0xb000e040
#define DMA_D_RBOR	(volatile int*)0xb000e050
#define DMA_D_STADR	(volatile int*)0xb000e060

/* Defines for DMA tag fields. */    
#define DMA_TAG_ID	0x70000000
#define DMA_TAG_ID__REFE	0
#define DMA_TAG_ID__CNT		1
#define DMA_TAG_ID__NEXT	2
#define DMA_TAG_ID__REF		3
#define DMA_TAG_ID__REFS	4
#define DMA_TAG_ID__CALL	5
#define DMA_TAG_ID__RET		6
#define DMA_TAG_ID__END		7

/* Dn_CHCR definition values */
#define DMA_Dn_CHCR__STR	0x00000100
#define DMA_Dn_CHCR__TIE	0x00000080
#define DMA_Dn_CHCR__TTE	0x00000040
#define DMA_Dn_CHCR__MODE	0x0000000c
#define DMA_Dn_CHCR__MODE_NORM	0x00000000
#define DMA_Dn_CHCR__MODE_CHAIN	0x00000004
#define DMA_Dn_CHCR__DIR	0x00000001

#endif /* DMA_H */
