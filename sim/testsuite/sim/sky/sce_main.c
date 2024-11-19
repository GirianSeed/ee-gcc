
/****************************************************/
/* This is a (Toronto created) wrapper program      */
/* to drive the sce_tests                           */
/*                                                  */
/* Copyright (C) 1998, Cygnus Solutions             */
/****************************************************/ 

#include "vu.h"
#include "vif.h"
#include "dma.h"

extern int printf(const char *, ...);

extern char My_dma_start[];
extern char gpu_refresh;

void DMA_enable(void) {
    *DMA_D_CTRL = 0x01; /* DMA enable */
}

/* If DMA mode is source chain */ 
void start_DMA_ch1_source_chain(void* data) {
    *DMA_D_CTRL  = 0x01; /* DMA enable */
    *DMA_D1_QWC  = 0x00;
    *DMA_D1_TADR = (int)data; 
    *DMA_D1_CHCR = DMA_Dn_CHCR__MODE_CHAIN | DMA_Dn_CHCR__STR |
        DMA_Dn_CHCR__TTE | DMA_Dn_CHCR__DIR;

}

/* If DMA mode is normal */
void start_DMA_ch1_normal(void* data, int qwc) {
    *DMA_D_CTRL  = 0x01; /* DMA enable */
    *DMA_D1_QWC  = qwc; /* 8 is sample */
    *DMA_D1_MADR = (int)data; 
    *DMA_D1_CHCR = DMA_Dn_CHCR__MODE_NORM | DMA_Dn_CHCR__STR |
        DMA_Dn_CHCR__TTE | DMA_Dn_CHCR__DIR;

}

void wait_until_idle() {
    /* Hmmm... Not sure exactly what the right code is for this.  I'll look for
     * VIF_STAT.PPS = 0 && VIF_STAT.FQC == 0 && VPU_STAT.VBS1 == 0 */

    int vif1_stat, vpu_stat;
    do {
	vif1_stat = *VIF1_STAT;
	vpu_stat = *VPU_STAT;
    } while (!(   (vif1_stat & VIF1_STAT_PPS_MASK) == 0
	       && (vif1_stat & VIF1_STAT_FQC_MASK) == 0
	       && (vpu_stat  & VPU_STAT_VBS1_MASK) == 0));
}

void wait_a_while() {
    int i;
    for (i = 0; i<200000; i++) {}
}

int main() {
    start_DMA_ch1_source_chain(&My_dma_start);
    wait_until_idle();
    start_DMA_ch1_source_chain(&gpu_refresh);
    wait_a_while();

    return 0;
}
