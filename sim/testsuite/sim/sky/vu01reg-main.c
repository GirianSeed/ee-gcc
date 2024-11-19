/* Test accessing vu1's registers from vu0.  */

#include "dma.h"
#include "vu.h"
#include "vif.h"

static int errcount;

short *int_buf = (short *) (VU0_MEM1_WINDOW_START + 0x400);
float *fp_buf = (float *) (VU0_MEM1_WINDOW_START + 0x410);

void
error (const char *msg)
{
  printf (msg);
  ++errcount;
}

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

/* If DMA mode is source chain */ 
void
start_dma_ch1_source_chain (void* data)
{
  *DMA_D_CTRL  = 0x01; /* DMA enable */
  *DMA_D1_QWC  = 0x00;
  *DMA_D1_TADR = (int)data;
  *DMA_D1_CHCR = DMA_Dn_CHCR__MODE_CHAIN | DMA_Dn_CHCR__STR |
        DMA_Dn_CHCR__DIR;
}

/* If DMA mode is normal */
void
start_dma_ch1_normal (void* data, int qwc)
{
  *DMA_D_CTRL  = 0x01; /* DMA enable */
  *DMA_D1_QWC  = qwc; /* 8 is sample */
  *DMA_D1_MADR = (int)data;
  *DMA_D1_CHCR = DMA_Dn_CHCR__MODE_NORM | DMA_Dn_CHCR__STR |
        DMA_Dn_CHCR__DIR;
}

extern char dma_start_vu0a[];
extern char dma_start_vu0b[];
extern char dma_start_vu1[];

extern char dma_start_vu0a_ctrl[];
extern char dma_start_vu0b_ctrl[];
extern char dma_start_vu1_ctrl[];

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
validate_intfp ()
{
  /* These addresses and values come from memif0.vu.  */
  if (int_buf[0] != 42)
    error ("read of vu1 int reg failed\n");

  {
    float * f = fp_buf;
    /* FIXME: There's a problem with fp->dp conversions.
       This fails without the `f'.  */
    if (f[0] != 1.0f
	|| f[1] != 2.0f
	|| f[2] != 3.0f
	|| f[3] != 4.0f)
      {
	error ("read 1 of vu1 fp regs failed\n");
      }
    if (f[4] != 1.0f
	|| f[5] != 2.0f
	|| f[6] != 3.0f
	|| f[7] != 4.0f)
      {
	error ("read 2 of vu1 fp regs failed\n");
      }
    if (f[8] != 1.0f
	|| f[9] != 2.0f
	|| f[10] != 3.0f
	|| f[11] != 4.0f)
      {
	error ("read 3 of vu1 fp regs failed\n");
      }
  }
}

static void
enable_cop2 ()
{
  asm volatile ("
	mfc0    $3,$12
	dli	$4,0x40000000
	or	$3,$4,$4
	mtc0	$3,$12
");
}

static void
validate_ctrl ()
{
  short *s = int_buf;
  float *f = fp_buf;
  unsigned int vi06,vi07,vi08,vi09,vi10,vi11,vi12;

  enable_cop2 ();

  /* status flag */
  asm volatile ("cfc2 %0, $vi06" : "=r" (vi06));
  asm volatile ("cfc2 %0, $vi10" : "=r" (vi10));
  if ((vi06 & 0xfc0) != 0xa80 || vi10 != 1)
    error ("status flag failed\n");

  /* mac flag */
  asm volatile("cfc2 %0, $vi07" : "=r" (vi07));
  if (vi07 != 0xe)
    error ("mac flag failed\n");

  /* clipping flag */
  asm volatile ("cfc2 %0, $vi08" : "=r" (vi08));
  asm volatile ("cfc2 %0, $vi11" : "=r" (vi11));
  if (vi08 != 0x5555 || vi11 != 1)
    error ("clipping flag failed\n");

  /* R reg */
  {
    static struct { int x,y,z,w; } __attribute__ ((aligned (16))) rtest asm ("rtest");
    asm volatile ("la $4,rtest\nsqc2 vf10,0($4)" : : : "$4");
    asm volatile ("cfc2 %0, $vi09" : "=r" (vi09));
    if (rtest.x != 0x3f80ffff || vi09 != 0x42)
      error ("R reg failed\n");
  }

  /* I reg */
  {
    static struct { float x,y,z,w; } __attribute__ ((aligned (16))) itest[2] asm ("itest");
    asm volatile ("la $4,itest\nsqc2 vf11,0($4)" : : : "$4");
    asm volatile ("la $4,itest\nsqc2 vf09,16($4)" : : : "$4");
    if (itest[0].x != 1.0f || itest[1].x != -1.0f)
      error ("I reg failed\n");
  }

  /* Q reg */
  {
    static struct { float x,y,z,w; } __attribute__ ((aligned (16))) qtest[2] asm ("qtest");
    asm volatile ("la $4,qtest\nsqc2 vf12,0($4)" : : : "$4");
    asm volatile ("la $4,qtest\nsqc2 vf08,16($4)" : : : "$4");
    if (qtest[0].x != 2.0f || qtest[1].x != -2.0f)
      error ("Q reg failed\n");
  }

  /* P reg */
  {
    static struct { float x,y,z,w; } __attribute__ ((aligned (16))) ptest[2] asm ("ptest");
    asm volatile ("la $4,ptest\nsqc2 vf13,0($4)" : : : "$4");
    asm volatile ("la $4,ptest\nsqc2 vf07,16($4)" : : : "$4");
    if (ptest[0].x != 3.0f || ptest[1].x != -3.0f)
      error ("P reg failed\n");
  }
}

int
main ()
{
  errcount = 0;

  /* Initialize vu0 memory to known values.  */
  memset ((char *) VU0_MEM1_WINDOW_START, 0, VU0_MEM1_SIZE);

  /* First run a program on vu0 to set some vu1 registers.  */
  start_dma_ch0_source_chain (&dma_start_vu0a[0]);
  wait_until_idle (0);

  /* Now run a program on vu1 to do something with those registers.  */
  start_dma_ch1_source_chain (&dma_start_vu1[0]);
  wait_until_idle (1);

  /* Now run a program on vu0 to fetch those values.  */
  start_dma_ch0_source_chain (&dma_start_vu0b[0]);
  wait_until_idle (0);

  /* Validate values in vu0 memory (which came from vu1).  */
  validate_intfp ();

  /* Run a program on vu0 to set some vu1 control registers.  */
  start_dma_ch0_source_chain (&dma_start_vu0a_ctrl[0]);
  wait_until_idle (0);

  /* Now run a program on vu1 to do something with those registers.  */
  start_dma_ch1_source_chain (&dma_start_vu1_ctrl[0]);
  wait_until_idle (1);

  /* Now run a program on vu0 to fetch those values.  */
  start_dma_ch0_source_chain (&dma_start_vu0b_ctrl[0]);
  wait_until_idle (0);

  /* Validate values in vu0 memory (which came from vu1).  */
  validate_ctrl ();

  exit (errcount ? 1 : 0);
}
