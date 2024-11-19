/*  Copyright (C) 1998, Cygnus Solutions

    */

#ifndef DMA_H_
#define DMA_H_

#include "sim-main.h"

void dma_attach(SIM_DESC sd);
 
#define DMA_REGISTER_WINDOW_START 0x10008000

#define DMA_D0_START    0x10008000
#define DMA_D0_CHCR     0x10008000
#define DMA_Dn_CHCR__STR        0x00000100
#define DMA_Dn_CHCR__TIE        0x00000080
#define DMA_Dn_CHCR__TTE        0x00000040
#define DMA_Dn_CHCR__ASP        0x00000030
#define DMA_Dn_CHCR__ASP_0      0x00000000
#define DMA_Dn_CHCR__ASP_1      0x00000010
#define DMA_Dn_CHCR__ASP_2      0x00000020
#define DMA_Dn_CHCR__MOD        0x0000000c
#define DMA_Dn_CHCR__MOD_NORM   0x00000000
#define DMA_Dn_CHCR__MOD_CHAIN  0x00000004
#define DMA_Dn_CHCR__MOD_ILEAVE 0x00000008
#define DMA_Dn_CHCR__DIR        0x00000001
#define DMA_D0_MADR     0x10008010
#define DMA_D0_QWC      0x10008020
#define DMA_D0_TADR     0x10008030
#define DMA_D0_ASR0     0x10008040
#define DMA_D0_ASR1     0x10008050
#define DMA_D0_PKTFLAG  0x10008060      /* virtual reg to indicate presence of tag in data */
#define DMA_D0_END      0x10008070
#define DMA_Dn_REGISTER_MASK 0x00000ff0
#define DMA_Dn_REGISTER_SHIFT 4
#define DMA_Dn_REGISTER_QUANTITY ((DMA_D0_END - DMA_D0_START) >> DMA_Dn_REGISTER_SHIFT)
#define DMA_Dn_NUM_CHANNELS 3

#define DMA_D1_START    0x10009000
#define DMA_D1_CHCR     0x10009000
#define DMA_D1_MADR     0x10009010
#define DMA_D1_QWC      0x10009020
#define DMA_D1_TADR     0x10009030
#define DMA_D1_ASR0     0x10009040
#define DMA_D1_ASR1     0x10009050
#define DMA_D1_PKTFLAG  0x10009060      /* virtual reg to indicate presence of tag in data */ 
#define DMA_D1_END      0x10009070

#define DMA_D2_START    0x1000a000
#define DMA_D2_CHCR     0x1000a000
#define DMA_D2_MADR     0x1000a010
#define DMA_D2_QWC      0x1000a020
#define DMA_D2_TADR     0x1000a030
#define DMA_D2_ASR0     0x1000a040
#define DMA_D2_ASR1     0x1000a050
#define DMA_D2_PKTFLAG  0x1000a060      /* virtual reg to indicate presence of tag in data */ 
#define DMA_D2_END      0x1000a070

#define DMA_D_CTRL      0x1000e000
#define DMA_D_CTRL__DMAE        0x00000001
#define DMA_D_STAT      0x1000e010
#define DMA_D_STAT__TOGGLE      0x63ff0000
#define DMA_D_STAT__CLEAR       0x0000e3ff
#define DMA_D_PCR       0x1000e020
#define DMA_D_PCR__PCE          0x80000000
#define DMA_D_PCR__CDE          0x03ff0000
#define DMA_D_SQWC      0x1000e030
#define DMA_D_RBSR      0x1000e040
#define DMA_D_RBOR      0x1000e050
#define DMA_D_STADR     0x1000e060

#define DMA_REGISTER_WINDOW_END 0x1000e070
#define DMA_REGISTER_WINDOW_SIZE (DMA_REGISTER_WINDOW_END - DMA_REGISTER_WINDOW_START)

#define DMA_REG_ADDR_MASK 0xefff0f0f    /* for sanity check on register numbers */

/* Defines for DMA tag fields. */    
#define DMA_TAG_ID      0x70000000
#define DMA_TAG_ID__REFE        0x00000000
#define DMA_TAG_ID__CNT         0x10000000
#define DMA_TAG_ID__NEXT        0x20000000
#define DMA_TAG_ID__REF         0x30000000
#define DMA_TAG_ID__REFS        0x40000000
#define DMA_TAG_ID__CALL        0x50000000
#define DMA_TAG_ID__RET         0x60000000
#define DMA_TAG_ID__END         0x70000000


/* DMAC device.                                         */       
struct dmac_device
{
  device  dev;             /* Common device definition  */
  char   *debug_file_name; /* Debug file name           */
  FILE   *debug_file;      /* Debug file descriptor     */
};  

extern struct dmac_device dma_device;

#define DMA_CHECK_OPEN_DEBUG                             \
           if (( dma_device.debug_file == NULL ) &&      \
               ( dma_device.debug_file_name != NULL ))   \
             sky_open_file( &dma_device.debug_file,      \
                            dma_device.debug_file_name,  \
                            (char *) NULL, _IOLBF );              
                            
void dma_reset ();
void dma_options (struct dmac_device *dma, unsigned_4 option, char *option_string);

#endif
