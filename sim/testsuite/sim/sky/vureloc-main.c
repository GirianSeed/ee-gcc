/* Test vu relocs.  */

#include "dma.h"
#include "vu.h"
#include "vif.h"

static int errcount;

void
DMA_enable(void)
{
  *DMA_D_CTRL = 0x01; /* DMA enable */
}

/* If DMA mode is source chain */ 
void
start_dma_ch0_source_chain (void* data)
{
  *DMA_D_CTRL  = 0x01; /* DMA enable */
  *DMA_D0_QWC  = 0x00;
  *DMA_D0_TADR = (int)data;
  *DMA_D0_CHCR = DMA_Dn_CHCR__MODE_CHAIN | DMA_Dn_CHCR__STR |
        DMA_Dn_CHCR__DIR;
}

/* If DMA mode is normal */
void
start_dma_ch0_normal (void* data, int qwc)
{
  *DMA_D_CTRL  = 0x01; /* DMA enable */
  *DMA_D0_QWC  = qwc; /* 8 is sample */
  *DMA_D0_MADR = (int)data;
  *DMA_D0_CHCR = DMA_Dn_CHCR__MODE_NORM | DMA_Dn_CHCR__STR |
        DMA_Dn_CHCR__DIR;
}

extern char dma_start_vu0[];

void
wait_until_idle (int vu_num)
{
  /* Hmmm... Not sure exactly what the right code is for this.  I'll look for
   * VIF_STAT.PPS = 0 && VIF_STAT.FQC == 0 && VPU_STAT.VBS1 == 0 */
  int vif_stat, vpu_stat;

  if (vu_num == 1)
    {
      do {
	vif_stat = *VIF1_STAT;
	vpu_stat = *VPU_STAT;
      } while (! ((vif_stat & VIF1_STAT_PPS_MASK) == 0
		  && (vif_stat & VIF1_STAT_FQC_MASK) == 0
		  && (vpu_stat  & VPU_STAT_VBS1_MASK) == 0));
    }
  else
    {
      do {
	vif_stat = *VIF0_STAT;
	vpu_stat = *VPU_STAT;
      } while (! ((vif_stat & VIF0_STAT_PPS_MASK) == 0
		  && (vif_stat & VIF0_STAT_FQC_MASK) == 0
		  && (vpu_stat  & VPU_STAT_VBS0_MASK) == 0));
    }
}

static void
validate ()
{
  /* These addresses and values come from vureloc.dvpasm.  */

  /* Validate R_MIPS_DVP_11_S4.  */
  if (*(short*) (VU0_MEM1_WINDOW_START + 0x400) != 42)
    {
      printf ("ilw/isw failed\n");
      ++errcount;
    }
}

int
main ()
{
  errcount = 0;

  /* Initialize vu0 memory to known values.  */
  memset ((char *) VU0_MEM1_WINDOW_START, 0, VU0_MEM1_SIZE);

  /* Run a program on vu0 to perform the tests.  */
  start_dma_ch0_source_chain (&dma_start_vu0[0]);
  wait_until_idle (0);

  /* Finally, validate values in vu0 memory.  */
  validate ();

  exit (errcount ? 1 : 0);
}
