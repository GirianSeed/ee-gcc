/*  Copyright (C) 1998, Cygnus Solutions
*/

#include <stdlib.h>
#include "config.h"

#include "sim-main.h"
#include "sim-assert.h"
#include "sky-device.h"
#include "sky-dma.h"
#include "sky-vif.h"
#include "sky-gpuif.h"
#include "sky-gdb.h"
#include "sky-indebug.h"

#ifdef HAVE_STRING_H
#include <string.h>
#else
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#endif

/* TODO: we need an error handling routine! */
#define error(x) ASSERT(x == NULL)

int sky_cpcond0;                        /* current value of cpcond0 */
int sky_cpcond0A;                       /* previous value of cpcond0 */

static unsigned reg_ctrl, reg_stat, reg_pcr, reg_sqwc, reg_rbsr, reg_rbor, reg_stadr;
static unsigned chnl_regs[DMA_Dn_NUM_CHANNELS][DMA_Dn_REGISTER_QUANTITY];
static unsigned const chnl_reg_masks[DMA_Dn_REGISTER_QUANTITY] =
{
    0x000001ff, 0x7ffffff0, 0x0000ffff, 0x7ffffff0,
    0x7ffffff0, 0x7ffffff0, 0xffffffff
};

static int
dma_io_read_buffer (device *me,
                    void *dest,
                    int space,
                    address_word addr1,
                    unsigned nr_bytes,
                    sim_cpu *processor,
                    sim_cia cia);
static int
dma_io_write_buffer (device *me,
                     const void *source,
                     int space,
                     address_word addr,
                     unsigned nr_bytes,
                     sim_cpu *processor,
                     sim_cia cia);
static void
check_dma_start (sim_cpu *cpu);
static void
check_int1 (void);
static void
check_cpcond0 (void);
static void
do_dma_transfer (SIM_DESC sd, void *data);
static unsigned
do_dma_transfer_tag (SIM_DESC sd,
                     unsigned chnl,
                     SIM_ADDR *pt_addr,
                     SIM_ADDR *pm_addr,
                     unsigned *qwc,
                     SIM_ADDR target_addr);


struct dmac_device dma_device = 
{ 
  { "Dma Controller",      /* Device                 */
    &dma_io_read_buffer,
    &dma_io_write_buffer
  },
  NULL,                    /* Debug file name        */
  NULL                     /* Debug file descriptor  */
};

void 
dma_attach (SIM_DESC sd) 
{
    unsigned i, j;

    sim_core_attach (sd,
                    NULL,
                    0 /*level*/,
                    access_read_write,
                    0 /*space ???*/,
                    DMA_REGISTER_WINDOW_START,
                    DMA_REGISTER_WINDOW_SIZE /*nr_bytes*/,
                    0 /*modulo*/,
                    &dma_device.dev,
                    NULL /*buffer*/);

    dma_reset();
}


void
dma_reset()
{
  int i, j;

  reg_ctrl = reg_stat = reg_pcr = reg_sqwc = 
    reg_rbsr = reg_rbor = reg_stadr = 0;

  for( i=0; i<DMA_Dn_NUM_CHANNELS; ++i)
    {
      for( j=0; j<DMA_Dn_REGISTER_QUANTITY; ++j)
        {
	  chnl_regs[i][j] = 0;
        }
    }
}


static int
dma_io_read_buffer (device *me,
                   void *dest,
                   int space,
                   address_word addr1,
                   unsigned nr_bytes,
                   sim_cpu *processor,
                   sim_cia cia)
{
    unsigned addr = (unsigned) addr1;
    unsigned *const pmem = (unsigned*) dest;
    unsigned temp;
    struct dmac_device *dma = (struct dmac_device *) me;

    if (indebug("DMAC"))
      {
        DMA_CHECK_OPEN_DEBUG;
        fprintf ((dma->debug_file != NULL) ? dma->debug_file : stdout,
                  "%s: Read from 0x%08x!\n", me->name, addr);
      }            

    ASSERT ((addr & DMA_REG_ADDR_MASK) == 0);   /* sanity check */
    ASSERT (addr >= DMA_REGISTER_WINDOW_START && addr < DMA_REGISTER_WINDOW_END);
    if (nr_bytes != 4)
        error ("DMA read i/o length");

    if (addr < DMA_D2_END)
    {
        unsigned regnum;

        regnum = (addr & DMA_Dn_REGISTER_MASK) >> DMA_Dn_REGISTER_SHIFT;

        if (addr < DMA_D0_END)
            *pmem = H2T_4 (chnl_regs[ 0][ regnum]);
        else if (addr < DMA_D1_END)
            *pmem = H2T_4 (chnl_regs[ 1][ regnum]);
        else
            *pmem = H2T_4 (chnl_regs[ 2][ regnum]);
    }
    else switch (addr)
    {
    case DMA_D_CTRL:
        *pmem = H2T_4 (reg_ctrl);
        break;
    case DMA_D_STAT:
        *pmem = H2T_4 (reg_stat);
        break;
    case DMA_D_PCR:
        temp = reg_pcr;
        if (temp & DMA_D_PCR__PCE)
        {
            temp |= DMA_D_PCR__CDE;
        }
        *pmem = H2T_4 (temp);
        break;
    case DMA_D_SQWC:
        *pmem = H2T_4 (reg_sqwc);
        printf ("DMA warning: read from unsupported register (D_SQWC)\n");
        break;
    case DMA_D_RBSR:
        *pmem = H2T_4 (reg_rbsr);
        printf ("DMA warning: read from unsupported register (D_RBSR)\n");
        break;
    case DMA_D_RBOR:
        *pmem = H2T_4 (reg_rbor);
        printf ("DMA warning: read from unsupported register (D_RBOR)\n");
        break;
    case DMA_D_STADR:
        *pmem = H2T_4 (reg_stadr);
        printf ("DMA warning: read from unsupported register (D_STADR)\n");
        break;
    default:
        ASSERT ("DMA read ctl reg number");
    }

    if (indebug("DMAC"))
      fprintf ((dma->debug_file != NULL) ? dma->debug_file : stdout,
                "\tvalue=0x%08lx\n", (long) T2H_4(*pmem));
    return nr_bytes;
}

static int
dma_io_write_buffer (device *me,
                    const void *source,
                    int space,
                    address_word addr,
                    unsigned nr_bytes,
                    sim_cpu *cpu,
                    sim_cia cia)
{
    const unsigned *pmem = (unsigned *) source;
    unsigned requested, actual;
    char const *req_act_msg = "DMA warning: write reg 0x%08x req=0x%08x actual=0x%08x\n";
    struct dmac_device *dma = (struct dmac_device *) me;

    if (indebug("DMAC"))
      {
        DMA_CHECK_OPEN_DEBUG;
        fprintf ((dma->debug_file != NULL) ? dma->debug_file : stdout,
                 "%s: Write to 0x%08lx with 0x%08lx\n", dma->dev.name,
                 (long) addr, (long) T2H_4(*pmem));
      }           

    ASSERT ((addr & DMA_REG_ADDR_MASK) == 0);   /* sanity check */
    ASSERT (addr >= DMA_REGISTER_WINDOW_START && addr < DMA_REGISTER_WINDOW_END);
    if (nr_bytes != 4)
        error ("DMA write i/o length");

    if (addr < DMA_D2_END)
    {
        unsigned regnum;

        regnum = (addr & DMA_Dn_REGISTER_MASK) >> DMA_Dn_REGISTER_SHIFT;
        requested = T2H_4 (*pmem);
        actual = requested & chnl_reg_masks[regnum];
        if (requested != actual)
            printf( req_act_msg, addr, requested, actual);

        if (addr < DMA_D0_END)
            chnl_regs[0][regnum] = actual;
        else if (addr < DMA_D1_END)
            chnl_regs[1][regnum] = actual;
        else
            chnl_regs[2][regnum] = actual;

        /*
        If any of the CHCR registers were just written,
        We may need to start a DMA operation.
        */
        if (regnum == ((DMA_D0_CHCR & DMA_Dn_REGISTER_MASK) >> DMA_Dn_REGISTER_SHIFT))
            check_dma_start (cpu);
    }
    else switch (addr)
    {
    case DMA_D_CTRL:
        requested = T2H_4 (*pmem);
        actual = requested & 0x000007ff;
        if (requested != actual)
            printf( req_act_msg, addr, requested, actual);
        reg_ctrl = actual;
        check_dma_start (cpu);
        break;
    case DMA_D_STAT:
        requested = T2H_4 (*pmem);
        actual = requested & 0x63ffe3ff;
        if (requested != actual)
            printf( req_act_msg, addr, requested, actual);
        reg_stat ^= actual & DMA_D_STAT__TOGGLE;
        reg_stat &= ~(actual & DMA_D_STAT__CLEAR);
        check_int1 ();
        check_cpcond0 ();
        break;
    case DMA_D_PCR:
        requested = T2H_4 (*pmem);
        actual = requested & 0x83ff03ff;
        if (requested != actual)
            printf( req_act_msg, addr, requested, actual);
        reg_pcr = actual;
        check_cpcond0 ();
        check_dma_start (cpu);
        break;
    case DMA_D_SQWC:
        requested = T2H_4 (*pmem);
        actual = requested & 0x00ff00ff;
        if (requested != actual)
            printf( req_act_msg, addr, requested, actual);
        reg_sqwc = actual;
        printf ("DMA warning: write to unsupported register (D_SQWC)\n");
        break;
    case DMA_D_RBSR:
        requested = T2H_4 (*pmem);
        actual = requested & 0x7ffffff0;
        if (requested != actual)
            printf( req_act_msg, addr, requested, actual);
        reg_rbsr = actual;
        printf ("DMA warning: write to unsupported register (D_RBSR)\n");
        break;
    case DMA_D_RBOR:
        requested = T2H_4 (*pmem);
        actual = requested & 0x7ffffff0;
        if (requested != actual)
            printf( req_act_msg, addr, requested, actual);
        reg_rbor = actual;
        printf ("DMA warning: write to unsupported register (D_RBOR)\n");
        break;
    case DMA_D_STADR:
        requested = T2H_4 (*pmem);
        actual = requested & 0x7ffffff0;
        if (requested != actual)
            printf( req_act_msg, addr, requested, actual);
        reg_stadr = actual;
        printf ("DMA warning: write to unsupported register (D_STADR)\n");
        break;
    default:
        ASSERT ("DMA write ctl reg number");
    }

    return nr_bytes;
}

void
check_dma_start (sim_cpu *cpu)
{

    if (reg_ctrl & DMA_D_CTRL__DMAE)
    {
        const unsigned regnum = (DMA_D0_CHCR - DMA_D0_START) >> DMA_Dn_REGISTER_SHIFT;
        unsigned temp;

        temp = reg_pcr;
        if ((temp & DMA_D_PCR__PCE) == 0)
        {
            temp |= DMA_D_PCR__CDE;
        }

        if ((temp & 0x00010000) && (chnl_regs[0][regnum] & DMA_Dn_CHCR__STR))
        {
            /* Do a DMA transfer before the next instruction. */
            sim_events_schedule (CPU_STATE(cpu), 
                                 0 /*time*/, 
                                 do_dma_transfer,
                                 (void*)0 /*data*/);
        }

        if ((temp & 0x00020000) && (chnl_regs[1][regnum] & DMA_Dn_CHCR__STR))
        {
            /* Do a DMA transfer before the next instruction. */
            sim_events_schedule (CPU_STATE(cpu), 
                                 0 /*time*/, 
                                 do_dma_transfer,
                                 (void*)1 /*data*/);
        }

        if ((temp & 0x00040000) && (chnl_regs[2][regnum] & DMA_Dn_CHCR__STR))
        {
            /* Do a DMA transfer before the next instruction. */
            sim_events_schedule (CPU_STATE(cpu), 
                                 0 /*time*/, 
                                 do_dma_transfer,
                                 (void*)2 /*data*/);
        }
    }
}

void
check_int1 (void)
{
    unsigned temp = reg_stat;
    if ((temp >> 16) & temp & 0x00000007)
    {
        if (indebug("DMAC"))
          {
            DMA_CHECK_OPEN_DEBUG;
            fprintf ((dma_device.debug_file != NULL) ? dma_device.debug_file : stdout,
                     "Dma Interrupt 1!!!\n");
          }           
      
        sky_signal_interrupt();
    }
}

void
check_cpcond0 (void)
{
    sky_cpcond0 = 0;
    if (reg_pcr & reg_stat & 0x000003ff)
    {
        sky_cpcond0 = 1;
    }
    if (indebug("DMAC"))
      {
        DMA_CHECK_OPEN_DEBUG;
        fprintf ((dma_device.debug_file != NULL) ? dma_device.debug_file : stdout,
                 "DMA sets CPCOND0=%d\n", sky_cpcond0);
      }           
}
           
static void
do_dma_transfer (SIM_DESC sd, void *data)
{
/*
Defines for chnl_regs[chnl][?].
They must be kept in sync with the defines for DMA_Dn_* in sky-dma.h.
*/
#define CHCR    0
#define MADR    1
#define QWC     2
#define TADR    3
#define ASR0    4
#define ASR1    5
#define PKTFLAG 6

    unsigned chnl = (unsigned) data;
    unsigned chain;                     /* !0 while there is another tag */
    SIM_ADDR t_addr;                    /* address of the (next) tag */
    SIM_ADDR m_addr;                    /* address of the tag's data */
    unsigned qwc = 0;                   /* quadword count fromtag */

    if (indebug("DMAC"))
      {
        DMA_CHECK_OPEN_DEBUG;
        fprintf ((dma_device.debug_file != NULL) ? dma_device.debug_file : stdout,
                 "DMA transfer on channel %d!!!\n", chnl);
      }
                 
    ASSERT (chnl == 0 || chnl == 1 || chnl == 2);

    /* Determine processing mode. */
    chain = chnl_regs[chnl][CHCR] & DMA_Dn_CHCR__MOD;
    switch (chain)
    {
    case DMA_Dn_CHCR__MOD_NORM:
        chain = 0;
        break;
    case DMA_Dn_CHCR__MOD_CHAIN:
        chain = 1;
        break;
    case DMA_Dn_CHCR__MOD_ILEAVE:
        error ("DMA error: Dn_CHCR.MOD=ILEAVE is unsupported.\n");
        break;
    default:
        error ("DMA error: Dn_CHCR.MOD is invalid.\n");
    }

    /* Perform a "normal" or tagged (chained) transfer. */
    do
    {
        const SIM_ADDR target_addr[3] =
            { VIF0_FIFO_ADDR, VIF1_FIFO_ADDR, GIF_PATH3_FIFO_ADDR};

        if ((chnl_regs[chnl][CHCR] & DMA_Dn_CHCR__DIR) == 0)
            error ("DMA transfer to memory is not supported.\n");
        
        if (!chain)
        {
            m_addr = chnl_regs[chnl][MADR];
            qwc = chnl_regs[chnl][QWC];
            if (indebug("DMAC"))
              fprintf ((dma_device.debug_file != NULL) ? dma_device.debug_file : stdout,
                       "madr=0x%08x qwc=0x%08x\n", m_addr, qwc);
        }
        else
        {
            chain = do_dma_transfer_tag (sd, chnl,
                                         &t_addr, &m_addr, &qwc,
                                         target_addr[chnl]);
        }

        /* Transfer the data. */
        for (; qwc; --qwc)
        {
            sim_cpu* cpu = STATE_CPU (CURRENT_STATE, 0);
            unsigned_16 qwbuf;

            /* Pass src addr for debug and whether there is a tag in this quad. */
            chnl_regs[chnl][MADR] = (unsigned) m_addr;
            chnl_regs[chnl][PKTFLAG] = 0;

            qwbuf = sim_core_read_aligned_16 (cpu, CIA_GET(cpu),
                                              read_map, m_addr);
            m_addr += 16;
            sim_core_write_aligned_16 (cpu, CIA_GET(cpu), write_map,
                                       target_addr[chnl], qwbuf);
            if (indebug("DMAC_DATA"))
            {
                /* Note that indebug("DMAC_DATA") implies that
                indebug("DMAC") is also set since the former
                string contains the latter. */
                unsigned word0, word1, word2, word3;
                word0 = V4_16 (qwbuf, 0);
                word1 = V4_16 (qwbuf, 1);
                word2 = V4_16 (qwbuf, 2);
                word3 = V4_16 (qwbuf, 3);
                fprintf ((dma_device.debug_file != NULL) ? dma_device.debug_file : stdout,
                         "DMA data written 0x%08x_%08x_%08x_%08x\n\n",
                         word0, word1, word2, word3);
            }
        }

        /* Now that the transfer is complete, udpate registers to final values. */
        chnl_regs[chnl][MADR] = (unsigned) m_addr;
        chnl_regs[chnl][QWC] = 0;

    } while (chain);

    /* Reset the channel's start bit and set its interrupt status bit. */
    chnl_regs[chnl][CHCR] &= ~ DMA_Dn_CHCR__STR;
    reg_stat |= 1 << chnl;

    /* If enabled, execute a tag interrupt. */
    if (chnl_regs[chnl][CHCR] & DMA_Dn_CHCR__TIE)
    {
        sky_signal_interrupt();
    }
    else
    {
        /* Check because "reg_stat" was set above. */
        check_int1 ();
    }
    check_cpcond0 ();
}
           
static unsigned
do_dma_transfer_tag (SIM_DESC sd,
                     unsigned chnl,
                     SIM_ADDR *pt_addr,
                     SIM_ADDR *pm_addr,
                     unsigned *pqwc,
                     SIM_ADDR target_addr)
{
    SIM_ADDR t_addr = *pt_addr;         /* address of the (next) tag */
    SIM_ADDR m_addr = *pm_addr;         /* address of the tag's data */
    SIM_ADDR r_addr;                    /* address referenced by the tag */
    unsigned qwc = *pqwc;
    sim_cpu* cpu = STATE_CPU (CURRENT_STATE, 0);
    unsigned chain = 1;                 /* !0 to continue chaining */
    unsigned_16 qwbuf;                  /* buffer for a quad word */
    unsigned tagH, tagL;                /* tag's high & low words */
    unsigned temp;

    t_addr = chnl_regs[chnl][TADR];

    /* read and decode the DMA tag */
    qwbuf = sim_core_read_aligned_16 (cpu, CIA_GET(cpu),
                                      read_map, t_addr);

    tagL = V4_16 (qwbuf, 3);
    tagH = V4_16 (qwbuf, 2);

    if (indebug("DMAC"))
      {
        DMA_CHECK_OPEN_DEBUG;
        fprintf ((dma_device.debug_file != NULL) ? dma_device.debug_file : stdout,
                 "tadr=0x%08x tag=0x%08x_%08x", t_addr, tagH, tagL);
      }           
    t_addr += 16;
    r_addr = tagH & 0x7ffffff0;
    qwc = tagL & 0x0000ffff;
    if (indebug("DMAC"))
      fprintf ((dma_device.debug_file != NULL) ? dma_device.debug_file : stdout,
               " addr=0x%08x qwc=0x%08x\n", r_addr, qwc);

    if (tagH & 0x80000000)
        error ("DMA Scratch Pad RAM is not supported.\n");
    if (tagH & 0x0000000f)
    {
        printf ("DMA WARNING: address in tag (0x%08x_%08x) is not quad word aligned.\n",
                tagH, tagL);
    }

    /* If requested, set or reset D_PCR.PCE. */
    temp = tagL >> 26;
    temp &= 3;
    if (temp == 1)
        printf ("DMA WARNING: tag's PCE field is invalid.\n");
    else if (temp == 2)
        reg_pcr &= ~DMA_D_PCR__PCE;
    else if (temp == 3)
        reg_pcr |= DMA_D_PCR__PCE;

    /* If the tag's IRQ=1, stop after this transfer and
    then interrupt based on Dn_CHCR__TIE. */
    if (tagL & 0x80000000)
        chain = 0;

    /* Set the upper 16 bits of CHCR from the tag. */
    chnl_regs[chnl][CHCR] &= 0x0000ffff;
    chnl_regs[chnl][CHCR] |= tagL & 0xffff0000;

    /* Set t_addr with the address of the next tag and
    m_addr with the address of the data to be transferred. */
    switch (tagL & DMA_TAG_ID)
    {
    case DMA_TAG_ID__CNT:
        m_addr = t_addr;                /* data is inline */
        t_addr += 16 * qwc;             /* next tag is inline (after the data) */
        break;
    case DMA_TAG_ID__NEXT:
        m_addr = t_addr;                /* data is inline */
        t_addr = r_addr;                /* next tag is out-of-line */
        break;
    case DMA_TAG_ID__REF:
    case DMA_TAG_ID__REFS:
    case DMA_TAG_ID__REFE:
        m_addr = r_addr;                /* data is out-of-line */
        ;                               /* next tag is inline */
        break;
    case DMA_TAG_ID__CALL:
        m_addr = t_addr;                /* data is inline */
        t_addr = m_addr + 16 * qwc;     /* return tag is inline (after the data) */
        switch (chnl_regs[chnl][CHCR] & DMA_Dn_CHCR__ASP)
        {
        case DMA_Dn_CHCR__ASP_0:
            chnl_regs[chnl][CHCR] &= ~DMA_Dn_CHCR__ASP;
            chnl_regs[chnl][CHCR] |= DMA_Dn_CHCR__ASP_1;
            chnl_regs[chnl][ASR0] = t_addr;
            t_addr = r_addr;            /* called tag is out-of-line */
            break;
        case DMA_Dn_CHCR__ASP_1:
            chnl_regs[chnl][CHCR] &= ~DMA_Dn_CHCR__ASP;
            chnl_regs[chnl][CHCR] |= DMA_Dn_CHCR__ASP_2;
            chnl_regs[chnl][ASR1] = t_addr;
            t_addr = r_addr;            /* called tag is out-of-line */
            break;
        default:
            error ("DMA error: DMAcall's nested too deeply.\n");
        }
        break;
    case DMA_TAG_ID__RET:
        m_addr = t_addr;                /* data is inline */
        switch (chnl_regs[chnl][CHCR] & DMA_Dn_CHCR__ASP)
        {
        case DMA_Dn_CHCR__ASP_2:
            chnl_regs[chnl][CHCR] &= ~DMA_Dn_CHCR__ASP;
            chnl_regs[chnl][CHCR] |= DMA_Dn_CHCR__ASP_1;
            t_addr = chnl_regs[chnl][ASR1]; /* pop return tag */
            break;
        case DMA_Dn_CHCR__ASP_1:
            chnl_regs[chnl][CHCR] &= ~DMA_Dn_CHCR__ASP;
            chnl_regs[chnl][CHCR] |= DMA_Dn_CHCR__ASP_0;
            t_addr = chnl_regs[chnl][ASR0]; /* pop return tag */
            break;
        default:
            error ("DMA error: too many DMAret's.\n");
        }
        break;
    case DMA_TAG_ID__END:
        chain = 0;                      /* stop after this transfer */
        m_addr = t_addr;                /* data is inline */
    }

    /* If required, transfer the tag. */
    if (chnl_regs[chnl][CHCR] & DMA_Dn_CHCR__TTE)
    {
        /* Pass src addr for debug and whether there is a tag in this quad. */
        chnl_regs[chnl][MADR] = chnl_regs[chnl][TADR];
        chnl_regs[chnl][PKTFLAG] = 1;

        sim_core_write_aligned_16 (cpu, CIA_GET(cpu), write_map,
                                   target_addr, qwbuf);
        if (indebug("DMAC_DATA"))
        {
            /* Note that indebug("DMAC_DATA") implies that
            indebug("DMAC") is also set since the former
            string contains the latter. */
            unsigned word0, word1, word2, word3;
            word0 = V4_16 (qwbuf, 0);
            word1 = V4_16 (qwbuf, 1);
            word2 = V4_16 (qwbuf, 2);
            word3 = V4_16 (qwbuf, 3);
            fprintf ((dma_device.debug_file != NULL) ? dma_device.debug_file : stdout,
                     "DMA tag/VIFs written 0x%08x_%08x_%08x_%08x\n\n",
                     word0, word1, word2, word3);
        }
    }

    /* Update the tag address. */
    chnl_regs[chnl][TADR] = (unsigned) t_addr;

    *pt_addr = t_addr;
    *pm_addr = m_addr;
    *pqwc = qwc;
    return chain;
}

void
dma_options(struct dmac_device *dma, unsigned_4 option, char *option_string)  
{
  switch (option) 
    {
    case SKY_OPT_DEBUG_NAME:
      if ( dma->debug_file != NULL ) 
        {
          fclose (dma->debug_file);
          dma->debug_file = NULL;
        }
      sky_store_file_name (&dma->debug_file_name, option_string);
      break;
      
    case SKY_OPT_CLOSE:
      if (dma->debug_file != NULL) 
        fclose (dma->debug_file);
      break;

    default: 
      ASSERT (0);
      break;
    }

  return;
}
