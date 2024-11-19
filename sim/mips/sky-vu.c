/* Copyright (C) 1998, Cygnus Solutions */


#include "config.h"

#include "sim-main.h"
#include "sim-endian.h"
#include "sim-assert.h"
#include "sky-vu.h"
#include "sky-gdb.h"
#include "sky-bits.h"
#include "sky-libvpe.h"

#include <stdlib.h>

/* internal functions */

static void vu_attach (SIM_DESC sd, vu_device * me);
static void vu_issue (SIM_DESC sd, vu_device * me);
static void vu_reset (SIM_DESC sd, vu_device * me);

static int vu_io_read_register_window (device * me_, void *dest, int space,
                                       address_word addr, unsigned nr_bytes,
                                       sim_cpu * processor, sim_cia cia);

static int vu_io_write_register_window (device * me_, const void *source,
                                        int space, address_word addr,
                                     unsigned nr_bytes, sim_cpu * processor,
                                        sim_cia cia);

static int read_vu_register_window (vu_device * me, unsigned addr,
                                    unsigned count, void *buf);
static int write_vu_register_window (vu_device * me, unsigned addr,
                                     unsigned count, const void *buf);
static int read_vu_registers (vu_device * me, unsigned addr,
                              unsigned count, char *buf);
static int write_vu_registers (vu_device * me, unsigned addr,
                               unsigned count, const char *buf);

static u_long vpu_stat ();

/* Some helper functions */
static int
f_2_i (float x)
{
  return *(int *) &x;
}

static float
i_2_f (int x)
{
  return *(float *) &x;
}


/* the public static variables */

vu_device vu0_device =
{
  {"vu0", &vu_io_read_register_window, &vu_io_write_register_window},
  {0, VU0_REGISTER_WINDOW_START,
   VU0_MEM0_WINDOW_START, VU0_MEM0_SIZE, VU0_MEM0_SRCADDR_START,
   VU0_MEM1_WINDOW_START, VU0_MEM1_SIZE, VU0_MEM1_SRCADDR_START},  /* config */
  0, 0, 0, 0, 0, 0,                        /* buffers */
  NULL, NULL                               /* debug trace file and name */
};

vu_device vu1_device =
{
  {"vu1", &vu_io_read_register_window, &vu_io_write_register_window},
  {1, VU1_REGISTER_WINDOW_START,
   VU1_MEM0_WINDOW_START, VU1_MEM0_SIZE, VU1_MEM0_SRCADDR_START,
   VU1_MEM1_WINDOW_START, VU1_MEM1_SIZE, VU1_MEM1_SRCADDR_START},  /* config */
  0, 0, 0, 0, 0, 0,                        /* buffers */
  NULL, NULL                               /* debug trace file and name */
};


/* Implementation of external functions */


void
vu0_attach (SIM_DESC sd)
{
#ifndef TARGET_SKY_B
  vu_attach (sd, &vu0_device);
#endif
}

void
vu1_attach (SIM_DESC sd)
{
  vu_attach (sd, &vu1_device);
}


void
vu0_issue (SIM_DESC sd)
{
#ifndef TARGET_SKY_B
  vu_issue (sd, &vu0_device);
#endif
}

void
vu1_issue (SIM_DESC sd)
{
  vu_issue (sd, &vu1_device);
}


void
vu0_reset ()
{
#ifndef TARGET_SKY_B
  vu_reset (current_state, &vu0_device);
#endif
}

void
vu1_reset ()
{
  vu_reset (current_state, &vu1_device);
}

#ifndef TARGET_SKY_B

int
vu0_busy ()
{
  unsigned_4 stat;
  read_vu_special_reg (&vu0_device, VU_REG_STAT, &stat);
  stat = T2H_4 (stat);
  return stat & SINGLE_BIT (VPU_STAT_VBS0_BIT);
}

int
vu0_q_busy ()
{
  /* poking into libvpe internals */
  return vu0_device.pipeline.qpipe.no != 0;
}

#endif

static void
initialize_and_run (vu_device * me, u_long start)
{
  me->run_state = VU_RUN;
  me->junk.pc = start;
  me->junk.eflag = 0;
  me->junk.dflag = 0;
  me->junk.tflag = 0;
  me->junk.peflag = 0;
  me->junk.mflag = 0;
  me->junk.lflag = 0;
}

#ifndef TARGET_SKY_B

void
vu0_macro_issue (unsigned_4 upper, unsigned_4 lower)
{
  /* poking into libvpe internals */
  ASSERT (!vu0_busy ());
  initialize_and_run (&vu0_device, -1);
  vu0_device.junk.mflag = 1;
  vu0_device.junk.peflag = 1;
  vu0_device.junk.eflag = -1;   /* imply END from previous command?; bypass next fetch_inst() */
  vu0_device.junk.instbuf[0] = upper;
  vu0_device.junk.instbuf[1] = lower;
  ASSERT (vu0_busy ());
}


int
vu0_micro_interlock_released ()
{
  /* poking into libvpe internals */
  return vu0_device.junk.lflag != 0;
}


void
vu0_micro_interlock_clear ()
{
  /* poking into libvpe internals */
  ASSERT (vu0_device.junk.lflag != 0);
  vu0_device.junk.lflag = 0;
}

#endif /* TARGET_SKY_B */

int
vu0_read_cop2_register (int reg_num)
{
  ASSERT (0 <= reg_num && reg_num <= 31);

  if (reg_num < 16)
    {
      return vu0_device.regs.VI[reg_num];
    }
  else
    {
      switch (reg_num)
        {
        case COP2_REG_STATUSFLAG:
          return vu0_device.regs.statusflag;
        case COP2_REG_MACFLAG:
          return vu0_device.regs.MACflag;
        case COP2_REG_CLIPFLAG:
          return vu0_device.regs.clipflag;
        case COP2_REG_R:
          return vu0_device.regs.R;
        case COP2_REG_I:
          return vu0_device.regs.I.i;
        case COP2_REG_Q:
          return vu0_device.regs.Q.i;
        case COP2_REG_TPC:
          return vu0_device.regs.MTPC;
        case COP2_REG_CMSAR0:
          return vu0_device.regs.CMSAR0;
        case COP2_REG_FBRST:
          {
            u_long temp = vu0_device.regs.FBRST;
            temp &= VPU_FBRST_TE1_MASK | VPU_FBRST_DE1_MASK
              | VPU_FBRST_TE0_MASK | VPU_FBRST_DE0_MASK;
            return temp;
          }

        case COP2_REG_VPUSTAT:
          return vpu_stat ();
        case COP2_REG_CMSAR1:
          /* Technically, not readable */
          return vu0_device.regs.CMSAR1;
        default:
          return 0;
        }
    }
}

#ifndef TARGET_SKY_B

/* Takes input in host format. */
void
vu0_write_cop2_register (int reg_num, int value)
{
  ASSERT (0 <= reg_num && reg_num <= 31);

  if (reg_num < 16)
    {
      vu0_device.regs.VI[reg_num] = (short) value;
    }
  else
    {
      switch (reg_num)
        {
        case COP2_REG_STATUSFLAG:       /* 7.2.1.3.1  Status flag */
          vu0_device.regs.statusflag = vu0_device.regs.statusflag & 0x0000003f | value & 0x00000fc0;
          return;

        case COP2_REG_MACFLAG:  /* 7.2.1.3.2  MAC flag -- Read Only */
          return;

        case COP2_REG_CLIPFLAG: /* 7.2.1.3.3  Clipping flag */
          vu0_device.regs.clipflag = value & 0x00ffffff;
          return;

        case COP2_REG_R:        /* 7.2.1.3.4  R,I,Q register */
          vu0_device.regs.R = value & 0x07ffffff;
          return;

        case COP2_REG_I:
          vu0_device.regs.I.i = value;
          return;

        case COP2_REG_Q:
          vu0_device.regs.Q.i = value;
          return;

        case COP2_REG_TPC:      /* 7.2.1.3.5  TPC -- Read Only */
          return;

        case COP2_REG_CMSAR0:   /* 7.2.1.3.6  CMSAR0 */
          vu0_device.regs.CMSAR0 = value & 0x0000ffff;
          return;

        case COP2_REG_FBRST:    /* 7.2.1.3.7  FBRST */

          vu0_device.regs.FBRST = value & 0x0000ffff;

          /* RS and FB bits together are undefined.  Have chosen
             to make RS0 bit win. */
          if (value & VPU_FBRST_RS0_MASK)
            vu0_device.run_state = VU_READY;
          else if (value & VPU_FBRST_FB0_MASK)
            vu0_device.run_state = VU_BREAK;

          if (value & VPU_FBRST_RS1_MASK)
            vu1_device.run_state = VU_READY;
          else if (value & VPU_FBRST_FB1_MASK)
            vu1_device.run_state = VU_BREAK;
          return;

        case COP2_REG_VPUSTAT:  /* 7.2.1.3.8  VPU_STAT -- Read Only */
          return;

        case COP2_REG_CMSAR1:   /* 7.2.1.3.9  CMSAR1 */
          vu0_device.regs.CMSAR1 = value & 0x0000ffff;
          initialize_and_run (&vu1_device, vu0_device.regs.CMSAR1);
          return;

        default:
          return;
        }
    }
}

#endif /* TARGET_SKY_B */

/* Internal functions */

static void
vu_attach (SIM_DESC sd, vu_device * me)
{
  char debug_string[20];
  char *debug_console;

  device *me_ = (device *) me;

  /* attach register vector */
  sim_core_attach (sd,
                   NULL,
                   0 /*level */ ,
                   access_read_write,
                   0 /*space ??? */ ,
                   me->config.register_window_start,
                   VU_REG_END /*nr_bytes */ ,
                   0 /*modulo */ ,
                   me_,
                   NULL /*buffer */ );

  /* allocate MEM0 = uMEM = instruction memory buffer */
  me->umem_buffer_tofree = zalloc (32 + me->config.mem0_size);

  /* align & scale for libvpe purposes */
  me->uMEM_buffer = (void *) ALIGN_16 ((unsigned) me->umem_buffer_tofree);
  me->uMEM_size = (me->config.mem0_size) / 8;

  /* attach aligned buffer segment */
  sim_core_attach (sd,
                   NULL,
                   0 /*level */ ,
                   access_read_write,
                   0 /*space ??? */ ,
                   me->config.mem0_window_start,
                   me->config.mem0_size,
                   0 /*modulo */ ,
                   0 /*device */ ,
                   me->uMEM_buffer /*buffer */ );

  /* allocate MEM1 = MEM = data memory buffer */
  me->mem_buffer_tofree = zalloc (32 + (me->config.mem1_size));

  /* align & scale for libvpe purposes */
  me->MEM_buffer = (void *) ALIGN_16 ((unsigned) me->mem_buffer_tofree);
  me->MEM_size = (me->config.mem1_size) / 16;

  /* attach aligned buffer segment */
  sim_core_attach (sd,
                   NULL,
                   0 /*level */ ,
                   access_read_write,
                   0 /*space ??? */ ,
                   me->config.mem1_window_start,
                   me->config.mem1_size,
                   0 /*modulo */ ,
                   0 /*device */ ,
                   me->MEM_buffer /*buffer */ );

  /* VU MEM0 tracking table */
  sim_core_attach (sd, NULL, 0, access_read_write, 0,
                   me->config.mem0_tracking_start,
                   me->config.mem0_size / 2,
                   0 /*modulo*/,
                   NULL,
                   NULL /*buffer*/);

  /* VU MEM1 tracking table */
  sim_core_attach (sd, NULL, 0, access_read_write, 0,
                   me->config.mem1_tracking_start,
                   me->config.mem1_size / 4,
                   0 /*modulo*/,
                   NULL,
                   NULL /*buffer*/);

  sprintf (debug_string, "VU%d_CONSOLE", me->config.vu_number);
  debug_console = getenv (debug_string);
  if (debug_console)
    {
      me->junk._is_dbg = 1;
      fork_terminal (&me->console_out, &me->console_in, debug_string, debug_console);

      STATE_CALLBACK (sd)->fdmap[fileno (me->console_out)] = fileno (me->console_out);
      STATE_CALLBACK (sd)->fdopen[fileno (me->console_out)] = 1;
    }
  else
    {
      me->junk._is_dbg = 0;
      me->console_out = stdout;
      me->console_in = stdin;
    }

  vu_reset(sd, me);
}

static void
vu_reset (SIM_DESC sd, vu_device * me)
{
  unsigned i;

  /* clear memory */
  memset(me->umem_buffer_tofree, 0, 32 + me->config.mem0_size);
  memset(me->mem_buffer_tofree, 0, 32 + me->config.mem1_size);

  /* clear tracking tables */
  for(i = me->config.mem0_tracking_start; i < me->config.mem0_size / 2; i++)
    {
      sim_cpu* cpu = STATE_CPU(CURRENT_STATE, 0);
      sim_core_write_aligned_1(cpu, 0, write_map,
			       (SIM_ADDR) i, (unsigned_1) 0);
    }
  for(i = me->config.mem1_tracking_start; i < me->config.mem1_size / 4; i++)
    {
      sim_cpu* cpu = STATE_CPU(CURRENT_STATE, 0);
      sim_core_write_aligned_1(cpu, 0, write_map,
			       (SIM_ADDR) i, (unsigned_1) 0);
    }

  /* set options */
  me->junk.opc = me->junk.pc = 0;
  me->junk._is_verb = 0;
  me->junk._is_dump = 0;
  me->junk._pgpuif = 4;         /* MEMGPUIF */

  /* initialize libvpe state */
  me->run_state = VU_READY;
  initvpe (me);
  vpecallms_init (me);
  me->junk.bp = 1;              /* Interesting hack... ?!? */
}


static void
vu_issue (SIM_DESC sd, vu_device * me)
{
  if (me->run_state == VU_RUN)
    vpecallms_cycle (sd, me);
}


static int
vu_io_read_register_window (device * me_,
                            void *dest,
                            int space,
                            address_word addr,
                            unsigned nr_bytes,
                            sim_cpu * processor,
                            sim_cia cia)
{
  vu_device *me = (vu_device *) me_;
  address_word base = me->config.register_window_start;

  ASSERT (addr >= base);
  addr -= base;

  /* Adjust nr_bytes if too big */
  if ((addr + nr_bytes) > VU_REG_END)
    nr_bytes -= addr + nr_bytes - VU_REG_END;

  return read_vu_registers (me, addr, nr_bytes, dest);
}


static int
vu_io_write_register_window (device * me_,
                             const void *source,
                             int space,
                             address_word addr,
                             unsigned nr_bytes,
                             sim_cpu * processor,
                             sim_cia cia)
{
  vu_device *me = (vu_device *) me_;
  address_word base = me->config.register_window_start;

  ASSERT (addr >= base);
  addr -= base;

  /* Adjust nr_bytes if too big */
  if ((addr + nr_bytes) > VU_REG_END)
    nr_bytes -= addr + nr_bytes - VU_REG_END;

  return write_vu_registers (me, addr, nr_bytes, source);
}



/* Copies integer register regno (VI00-VI15) to buf in target-endian order */
int
read_vu_int_reg (vu_regs * regs, int regno, void *buf)
{
  if (regno >= 0 && regno <= 15)
    {
      *((unsigned short *) buf) = H2T_2 (regs->VI[regno]);
      return 2;
    }

  return -1;
}

/* Copies vector register regno (VF00x-VF31w) to buf in target-endian order */
int
read_vu_vec_reg (vu_regs * regs, int regno, int xyzw, void *buf)
{
  if (regno >= 0 && regno <= 31 && xyzw >= 0 && xyzw <= 3)
    {
      *((int *) buf) = H2T_4 (*((int *) &(regs->VF[regno][xyzw])));
      return 4;
    }

  return -1;
}

/* Copies integer register regno (VI00-VI15) to buf in target-endian order */
int
read_vu_acc_reg (vu_regs * regs, int xyzw, void *buf)
{
  if (xyzw >= 0 && xyzw < 4)
    {
      *((int *) buf) = H2T_4 (*((int *) &(regs->acc[xyzw])));
      return 4;
    }

  return -1;
}

/* Copies register at regaddr to buf in target-endian order.  The registers
   handled here are those that are neither VI nor VF, but which do not require
   the general VU state to handle.  */
int
read_vu_misc_reg (vu_regs * regs, int regaddr, void *buf)
{
  switch (regaddr)
    {
    case VU_REG_MST:
      *((u_long *) buf) = H2T_4 (regs->MST);
      return 4;
    case VU_REG_MMC:
      *((u_long *) buf) = H2T_4 (regs->MMC);
      return 4;
    case VU_REG_MCP:
      *((u_long *) buf) = H2T_4 (regs->MCP);
      return 4;
    case VU_REG_MR:
      *((u_long *) buf) = H2T_4 (regs->R);
      return 4;
    case VU_REG_MI:
      *((u_long *) buf) = H2T_4 (*((u_long *) & (regs->I)));
      return 4;
    case VU_REG_MQ:
      *((u_long *) buf) = H2T_4 (*((u_long *) & (regs->Q)));
      return 4;
    case VU_REG_MP:
      *((u_long *) buf) = H2T_4 (*((u_long *) & (regs->VN[4])));
      return 4;
    case VU_REG_MTPC:
      *((u_long *) buf) = H2T_4 (regs->MTPC);
      return 4;
    default:
      *((u_long *) buf) = 0;    /* read 0 if undefined/reserved */
    }

  return -1;
}


/*
   (11) The VU1 control registers is not mapped to the main memory space
   too. The control registers written above are mapped only to VPU-MEM0.
   The VPU1-STAT is become including in COP2 Control registers.

   The changes in COP2 Control registers is following.

   VPU-STAT

   31              23              15               7              0
   +---------------+---------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |               |               | |E|D|V|V|V|V|V|P| |D| |V|V|V|V|
   |   00000000    |   00000000    |0|F|I|G|F|T|D|B|B|0|I|0|F|T|D|B|
   |               |               | |U|V|W|S|S|S|S|S| |V| |S|S|S|S|
   |               |               | |1|1|1|1|1|1|1|0| |0| |0|0|0|0|
   +---------------+---------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   Bit     |            Definition
   --------+-----------------------------------------
   VBS0(r) | VU0 busy
   | 0: idle, 1: busy
   | Is busy during micro subroutine execution.
   | E bit termination, is made idle by the Reset state.
   --------+-----------------------------------------
   VDS0(r) | VU0 D bit stop
   | 0: No D bit stop, 1: D bit stop
   --------+-----------------------------------------
   VTS0(r) | VU0 T bit stop
   | 0: No T bit stop, 1: T bit stop
   --------+-----------------------------------------
   VFS0(r) | VU0 ForceBreak stop
   | 0: No ForceBreak stop, 1: ForceBreak stop
   --------+-----------------------------------------
   DIV0(r) | VU0 DIV BUSY
   | 0: idle, 1: busy
   --------+-----------------------------------------
   PBS0(r) | VIF0 busy
   | 0: idle, 1: busy
   --------+-----------------------------------------
   VBS1(r) | VU1 busy
   | 0: idle, 1: busy
   | Is busy during micro subroutine execution.
   | E bit termination, is made idle by the Reset state.
   --------+-----------------------------------------
   VDS1(r) | VU1 D bit stop
   | 0: No D bit stop, 1: D bit stop
   --------+-----------------------------------------
   VTS1(r) | VU1 T bit stop
   | 0: No T bit stop, 1: T bit stop
   --------+-----------------------------------------
   VFS1(r) | VU1 ForceBreak stop
   | 0: No ForceBreak stop, 1: ForceBreak stop
   --------+-----------------------------------------
   VGS1(r) | VU1 XGKICK wait
   | 0: No wait,, 1: wait
   --------+-----------------------------------------
   DIV1(r) | VU1 DIV BUSY
   | 0: idle, 1: busy
   --------+-----------------------------------------
   EFU1(r) | VU1 EFU busy
   | 0: idle, 1: busy
   --------+-----------------------------------------
 */

static u_long
vpu_stat ()
{
  u_long stat = 0;

  if (vu0_device.run_state == VU_RUN || vu0_device.run_state == VU_BREAK)
    SET_BIT (stat, VPU_STAT_VBS0_BIT);
  if (vu0_device.junk.dflag)
    SET_BIT (stat, VPU_STAT_VDS0_BIT);
  if (vu0_device.junk.tflag)
    SET_BIT (stat, VPU_STAT_VTS0_BIT);
  if (0 /* TODO: force-break */ )
    SET_BIT (stat, VPU_STAT_VFS0_BIT);
  if (vu0_device.pipeline.qpipe.no != 0)
    SET_BIT (stat, VPU_STAT_DIV0_BIT);
  if (0 /* TODO: vif0 busy */ )
    SET_BIT (stat, VPU_STAT_PBS0_BIT);

  if (vu1_device.run_state == VU_RUN || vu1_device.run_state == VU_BREAK)
    SET_BIT (stat, VPU_STAT_VBS1_BIT);
  if (vu1_device.junk.dflag)
    SET_BIT (stat, VPU_STAT_VDS1_BIT);
  if (vu1_device.junk.tflag)
    SET_BIT (stat, VPU_STAT_VTS1_BIT);
  if (0 /* TODO: force-break */ )
    SET_BIT (stat, VPU_STAT_VFS1_BIT);
  if (0 /* XGKICK wait */ )
    SET_BIT (stat, VPU_STAT_VGS1_BIT);
  if (vu1_device.pipeline.qpipe.no != 0)
    SET_BIT (stat, VPU_STAT_DIV1_BIT);
  if (vu1_device.pipeline.spipe.no != 0)
    SET_BIT (stat, VPU_STAT_EFU1_BIT);

  return stat;
}

/* Copies special register at regaddr to buf in target-endian order.
   Registers are considered special if they interact with the state of the
   machine directly.  */
int
read_vu_special_reg (vu_device * me, int regaddr, void *buf)
{
  switch (regaddr)
    {
    case VU_REG_CMSAR0:
#ifndef TARGET_SKY_B
      if (me->config.vu_number == 0)
        {
          *((u_long *) buf) = H2T_4 (vu0_read_cop2_register (COP2_REG_CMSAR0));
        }
      else
#endif
        {
          *((u_long *) buf) = H2T_4 (0);
        }
      return 4;

    case VU_REG_FBRST:
      {
#ifndef TARGET_SKY_B
        if (me->config.vu_number == 0)
          {
            *((u_long *) buf) = H2T_4 (vu0_read_cop2_register (COP2_REG_FBRST));
          }
        else
#endif
          {
            *((u_long *) buf) = H2T_4 (0);
          }
        return 4;
      }

    case VU_REG_STAT:
      {
#if 0
        if (me->config.vu_number == 0)
          {
#endif
            *((u_long *) buf) = H2T_4 (vu0_read_cop2_register (COP2_REG_VPUSTAT));
#if 0
          }
        else
          {
            *((u_long *) buf) = H2T_4 (0);
          }
#endif
        return 4;
      }

    case VU_REG_CIA:
      *((u_long *) buf) = H2T_4 (me->junk.pc);
      return 4;

    case VU_REG_CMSAR1:
#ifndef TARGET_SKY_B
      if (me->config.vu_number == 0)
        {
          *((u_long *) buf) = H2T_4 (vu0_read_cop2_register (COP2_REG_CMSAR1));
        }
      else
#endif
        {
          *((u_long *) buf) = H2T_4 (0);
        }
      return 4;

    default:
    }

  sim_io_error (current_state, "Reading unknown VU special_reg %d\n", regaddr);
  return -1;
}

/* Copies buf to integer register regno (VI00-VI15) in host-endian order */
int
write_vu_int_reg (vu_regs * regs, int regno, const void *buf)
{
  if (regno >= 0 && regno <= 15)
    {
      regs->VI[regno] = T2H_2 (*((unsigned short *) buf));
      return 2;
    }

  return -1;
}

/* Copies buf to vector register regno (VF00x-VF31w) in host-endian order */
int
write_vu_vec_reg (vu_regs * regs, int regno, int xyzw, const void *buf)
{
  if (regno >= 0 && regno <= 31 && xyzw >= 0 && xyzw <= 3)
    {
      *((int *) &(regs->VF[regno][xyzw])) = T2H_4 (*((int *) buf));
      return 4;
    }

  return -1;
}

/* Copies buf to vector register regno (VF00x-VF31w) in host-endian order */
int
write_vu_acc_reg (vu_regs * regs, int xyzw, const void *buf)
{
  if (xyzw >= 0 && xyzw < 4)
    {
      *((int *) &(regs->acc[xyzw])) = T2H_4 (*((int *) buf));
      return 4;
    }

  return -1;
}

/* Copies buf to special register at regaddr in host-endian order */
int
write_vu_misc_reg (vu_regs * regs, int regaddr, const void *buf)
{
  switch (regaddr)
    {
    case VU_REG_MST:
      regs->MST = T2H_4 (*((u_long *) buf));
      return 4;
    case VU_REG_MMC:
      regs->MMC = T2H_4 (*((u_long *) buf));
      return 4;
    case VU_REG_MCP:
      regs->MCP = T2H_4 (*((u_long *) buf));
      return 4;
    case VU_REG_MR:
      regs->R = T2H_4 (*((u_long *) buf));
      return 4;
    case VU_REG_MI:
      *((u_long *) & (regs->I)) = T2H_4 (*((u_long *) buf));
      return 4;
    case VU_REG_MQ:
      *((u_long *) & (regs->Q)) = T2H_4 (*((u_long *) buf));
      return 4;
    case VU_REG_MP:
      *((u_long *) & (regs->VN[4])) = T2H_4 (*((u_long *) buf));
      return 4;
    case VU_REG_MTPC:
      regs->MTPC = T2H_4 (*((u_long *) buf));
      return 4;
    default:
      /* Ignore write if undefined/reserved */
    }

  return -1;
}

/* Copies special register at regaddr to buf in target-endian order.
   Registers are considered special if they interact with the state of the
   machine directly. 

   This part of the memory map no longer exists.  Anything here is for
   the use of the simulator internals, but does not reflect the "real"
   hardware.
 */

int
write_vu_special_reg (vu_device * me, int regaddr, const void *buf)
{
  u_long temp;
  temp = T2H_4 (*((u_long *) buf));

  switch (regaddr)
    {
    case VU_REG_FBRST:
      if (me->config.vu_number != 0)
        return 4;
#ifndef TARGET_SKY_B
      vu0_write_cop2_register (COP2_REG_FBRST, temp);
#endif
      return 4;

    case VU_REG_STAT:
      /* This is a read-only register. Writes are allowed but ignored. */
      return 4;

    case VU_REG_CIA:
      /* This is not on the real hardware, but we've implemented a write to the CIA as
       * the mechanism by which the VU is restarted. */
      initialize_and_run (me, temp);
      return 4;

    case VU_REG_CMSAR0:
      if (me->config.vu_number != 0)
        return 4;
#ifndef TARGET_SKY_B
      vu0_write_cop2_register (COP2_REG_CMSAR0, temp);
#endif
      return 4;

    case VU_REG_CMSAR1:
      if (me->config.vu_number != 0)
        return 4;
#ifndef TARGET_SKY_B
      vu0_write_cop2_register (COP2_REG_CMSAR1, temp);
#endif
      return 4;

    default:
    }

  sim_io_error (current_state, "Writing unknown VU special_reg %d\n", regaddr);
  return -1;
}

/* Read from the memory to which the registers are mapped.  This function
   does not assume anything about alignment/size of addr/count.  To support
   misaligned and/or partial register reads, the function builds a local
   memory image and dumps the registers into it in target order.  The local
   image is used to satisfy the actual addr/count request. */

/* addr is Relative to start of register window */

static int
read_vu_register_window (vu_device * me, unsigned addr, unsigned count, void *buf)
{
  int i, j;
  char mem_image[VU_REG_END*2];
  char *src = mem_image;

  /* Be a little bit smart about which portions of the memory image to fill.
     In particular, divide the range (addr,add+count) into 3 register sections:
     vector, integer, and special. */

  if (addr < VU_REG_VI)
    {
      for (i = 0; i < 32; i++)
        for (j = 0; j < 4; j++)
          {
            read_vu_vec_reg (&(me->regs), i, j, src);
            src += 4;
          }
    }

  if (addr < VU_REG_MST && (addr + count) >= VU_REG_VI)
    {
      src = mem_image + VU_REG_VI;
      for (i = 0; i < 16; i++)
        {
          read_vu_int_reg (&(me->regs), i, src);
          src += 16;
        }
    }

  if (addr < VU_REG_CMSAR0 && (addr + count) >= VU_REG_MST)
    {
      /* Misc regs are enumerated by their memory mapped offsets, which
         happen to be quad-word aligned (today). */

      src = mem_image + VU_REG_MST;
      for (i = VU_REG_MST; i <= VU_REG_MTPC; i += 16)
        {
          read_vu_misc_reg (&(me->regs), i, src);
          src += 16;
        }
    }

  if (addr + count >= VU_REG_CMSAR0)
    {
      read_vu_special_reg (me, VU_REG_CMSAR0, &mem_image[VU_REG_CMSAR0]);
      read_vu_special_reg (me, VU_REG_FBRST, &mem_image[VU_REG_FBRST]);
      read_vu_special_reg (me, VU_REG_STAT, &mem_image[VU_REG_STAT]);
      read_vu_special_reg (me, VU_REG_CIA, &mem_image[VU_REG_CIA]);
      read_vu_special_reg (me, VU_REG_CMSAR1, &mem_image[VU_REG_CMSAR1]);
    }

  memcpy (buf, &mem_image[addr], count);
  return count;
}

/* Read from the memory to which the registers are mapped. This routine is
   meant to optimize the assumed common case of program reads of particular
   registers. As such, the read alignment/length is expected to correspond
   exactly with one or more registers. If either the length or alignment is
   skewed, the routine falls back to the more generic read_register_window.
 */
static int
read_vu_registers (vu_device * me, unsigned addr, unsigned count, char *dest)
{
  int i;
  int start_reg, end_reg;
  char *buf = dest;             /* running pointer */
  unsigned pos = addr;          /* running offset */
  int remaining = count;        /* running count */

  /* If the address is not at least word aligned and the count is smaller 
     than the smallest register, make them pay. */
  if ((addr & 0x3) || (count & 0x1))
    return read_vu_register_window (me, addr, count, dest);

  if (addr < VU_REG_VI)
    {
      int row, col;

      /* Read one or more Vector regs. These regs are contiguous in mapped 
         memory, so we will attempt to return the data for several if needed.
         However, don't bother if count is such that it forces a partial 
         register read. */

      /* start_reg and end_reg should range from 0 - 32*4 = 128, and then
         row and col are *_reg/4 and *_reg%4, respectively */
      start_reg = addr >> 2;    /* 4 bytes per register */
      end_reg = start_reg + (count >> 2);
      col = start_reg & 0x3;    /* 4 register per row */

      if (count & 0x3)
        return read_vu_register_window (me, addr, count, dest);

      for (row = start_reg >> 2; row < 32; row += 4)
        {
          for (; col < 4 && (row * 4 + col) < end_reg; col++)
            {
              read_vu_vec_reg (&(me->regs), row, col, buf);
              buf += 4;
              pos += 4;
              remaining -= 4;
            }
          col = 0;
        }

      if (remaining <= 0)
        return count;
    }

  /* From here on, all regs are on quad-word boundaries */
  if (pos & 0x0f)
    return read_vu_register_window (me, addr, count, dest);

  if (pos < VU_REG_MST)
    {
      /* Read one or more integer regs. */

      start_reg = (pos - VU_REG_VI) >> 4;
      end_reg = ((pos + remaining) - VU_REG_VI) >> 4;

      for (i = start_reg; i <= end_reg && i <= 16; i++)
        {
          read_vu_int_reg (&(me->regs), i, buf);
          buf += 16;
          pos += 16;
          remaining -= 16;
        }

      if (remaining <= 0)
        return count;
    }

  if (count & 3)                /* Avoid counts that force a partial reg read */
    return read_vu_register_window (me, addr, count, dest);

  if (pos < VU_REG_CMSAR0)
    {
      while (pos < VU_REG_CMSAR0 && remaining > 0)
        {
          read_vu_misc_reg (&(me->regs), pos, buf);
          buf += 16;
          pos += 16;
          remaining -= 16;
        }

      if (remaining <= 0)
        return count;
    }

  /* Only the special registers are left */
  while (remaining > 0)
    {
      read_vu_special_reg (me, pos, buf);
      buf += 16;
      pos += 16;
      remaining -= 16;
    }

  return count;
}

/* Write to the memory to which the registers are mapped.  This function
   does not assume anything about alignment/size of addr/count.  To support
   misaligned and/or partial register writes, the function builds a local
   memory image and dumps the registers into it in target order.  The local
   image is used to satisfy the actual addr/count request.  Finally, the
   local buffer is copied back into the registers.  */

static int
write_vu_register_window (vu_device * me, unsigned addr, unsigned count, const void *source)
{
  int i, j;
  char mem_image[VU_REG_END*2];
  char *src = mem_image;

  /* Be a little bit smart about which portions of the memory image to copy
     back to the registers.  In particular, divide the range (addr,add+count) 
     into 3 register sections: vector, integer, and special. */

  if (addr < VU_REG_VI)
    {
      int amount;

      /* Build the memory image for the fp regs, and copy the modifications 
         from source into the memory image. */
      if (addr + count < VU_REG_VI)
        amount = count;
      else
        amount = addr + count - VU_REG_VI;

      read_vu_register_window (me, addr, amount, mem_image);
      memcpy (&mem_image[addr], source, amount);

      for (i = 0; i < 32; i++)
        for (j = 0; j < 4; j++)
          {
            write_vu_vec_reg (&(me->regs), i, j, src);
            src += 4;
          }
    }

  if (addr < VU_REG_MST && (addr + count) >= VU_REG_VI)
    {
      read_vu_register_window (me, VU_REG_VI, 16 * 16, mem_image + VU_REG_VI);
      memcpy (&mem_image[addr], source, count);

      src = mem_image + VU_REG_VI;
      for (i = 0; i < 16; i++)
        {
          write_vu_int_reg (&(me->regs), i, src);
          src += 16;
        }
    }

  if (addr < VU_REG_CMSAR0 && (addr + count) >= VU_REG_MST)
    {
      read_vu_register_window (me, VU_REG_MST, VU_REG_STAT - VU_REG_MST,
                               mem_image + VU_REG_MST);
      memcpy (&mem_image[addr], source, count);

      src = mem_image + VU_REG_MST;
      for (i = VU_REG_MST; i <= VU_REG_MTPC; i += 16)
        {
          write_vu_misc_reg (&(me->regs), i, src);
          src += 16;
        }
    }

  if ((addr + count) >= VU_REG_CMSAR0)
    {
      read_vu_special_reg (me, VU_REG_CMSAR0, &mem_image[VU_REG_CMSAR0]);
      read_vu_special_reg (me, VU_REG_FBRST, &mem_image[VU_REG_FBRST]);
      read_vu_special_reg (me, VU_REG_STAT, &mem_image[VU_REG_STAT]);
      read_vu_special_reg (me, VU_REG_CIA, &mem_image[VU_REG_CIA]);
      read_vu_special_reg (me, VU_REG_CMSAR1, &mem_image[VU_REG_CMSAR1]);
      memcpy (&mem_image[addr], source, count);

      write_vu_special_reg (me, VU_REG_CMSAR0, &mem_image[VU_REG_CMSAR0]);
      write_vu_special_reg (me, VU_REG_FBRST, &mem_image[VU_REG_FBRST]);
      write_vu_special_reg (me, VU_REG_STAT, &mem_image[VU_REG_STAT]);
      write_vu_special_reg (me, VU_REG_CIA, &mem_image[VU_REG_CIA]);
      write_vu_special_reg (me, VU_REG_CMSAR1, &mem_image[VU_REG_CMSAR1]);
    }

  return count;
}

/* Write the memory to which the registers are mapped. This routine is
   meant to optimize the assumed common case of program writes of particular
   registers. As such, the alignment/length is expected to correspond exactly 
   with one or more registers. If either the length or alignment is skewed, 
   the routine falls back to the more generic write_register_window.
 */
static int
write_vu_registers (vu_device * me, unsigned addr, unsigned count, const char *source)
{
  int i;
  int start_reg, end_reg;
  const char *buf = source;     /* running pointer */
  unsigned pos = addr;          /* running offset */
  int remaining = count;        /* running count */

  /* If the address is not at least word aligned and the count is smaller 
     than the smallest register, make them pay. */
  if ((addr & 0x3) || (count & 0x1))
    return write_vu_register_window (me, addr, count, source);

  if (addr < VU_REG_VI)
    {
      int row, col;

      /* Read one or more Vector regs. These regs are contiguous in mapped 
         memory, so we will attempt to return the data for several if needed.
         However, don't bother if count is such that it forces a partial 
         register read. */

      /* start_reg and end_reg should range from 0 - 32*4 = 128, and then
         row and col are *_reg/4 and *_reg%4, respectively */
      start_reg = addr >> 2;    /* 4 bytes per register */
      end_reg = start_reg + (count >> 2);
      col = start_reg & 0x3;    /* 4 register per row */

      if (count & 0x3)
        return write_vu_register_window (me, addr, count, source);

      for (row = start_reg >> 2; row < 32; row += 4)
        {
          for (; col < 4 && (row * 4 + col) < end_reg; col++)
            {
              write_vu_vec_reg (&(me->regs), row, col, buf);
              buf += 4;
              pos += 4;
              remaining -= 4;
            }
          col = 0;
        }

      if (remaining <= 0)
        return count;
    }

  /* From here on, all regs are on quad-word boundaries */
  if (pos & 0x0f)
    return write_vu_register_window (me, addr, count, source);

  if (pos < VU_REG_MST)
    {
      /* Write one or more integer regs. */

      start_reg = (pos - VU_REG_VI) >> 4;
      end_reg = ((pos + remaining) - VU_REG_VI) >> 4;

      for (i = start_reg; i <= end_reg && i <= 16; i++)
        {
          write_vu_int_reg (&(me->regs), i, buf);
          buf += 16;
          pos += 16;
          remaining -= 16;
        }

      if (remaining <= 0)
        return count;
    }

  if (count & 3)                /* Avoid counts that force a partial reg read */
    return write_vu_register_window (me, addr, count, source);

  if (pos < VU_REG_STAT)
    {
      while (pos < VU_REG_STAT && remaining > 0)
        {
          write_vu_misc_reg (&(me->regs), pos, buf);
          buf += 16;
          pos += 16;
          remaining -= 16;
        }

      if (remaining <= 0)
        return count;
    }

  /* Only the special registers are left */
  while (remaining > 0)
    {
      write_vu_special_reg (me, pos, buf);
      buf += 16;
      pos += 16;
      remaining -= 16;
    }

  return count;
}

void
vu_options (vu_device *me, int option, char *option_string)  
{
  switch (option) 
    {
    case SKY_OPT_DEBUG_NAME:
      if ( me->debug_file != NULL ) 
        {
          fclose (me->debug_file);
          me->debug_file = NULL;
        }
      sky_store_file_name (&me->debug_file_name, option_string);
      break;

    case SKY_OPT_CLOSE:
      if (me->debug_file != NULL) 
        fclose (me->debug_file);
      break;

    default: 
      ASSERT (0);
      break;
    }

  return;
}

