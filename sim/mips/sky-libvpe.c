/****************************************************************************/
/*                                                                          */
/*             Sony Computer Entertainment CONFIDENTIAL                     */
/*      (C) 1997 Sony Computer Entertainment Inc. All Rights Reserved       */
/*                                                                          */
/*      VU simulator                                                        */
/*                                                                          */
/****************************************************************************/

#include "config.h"

#include "sky-vpe.h"
#include "sky-libvpe.h"
#include "sky-gpuif.h"
#include "sky-indebug.h"
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <sim-assert.h>

#include "sim-main.h"
#include "sim-endian.h"
#include "sky-vudis.h"
#include "sky-interact.h"

#ifdef HAVE_FPU_CONTROL_H
#include <fpu_control.h>
#endif

#define H2T_I4(x)       ((long)(H2T_4((x))))
#define T2H_I4(x)       ((long)(T2H_4((x))))

static floatie
T2H_F4(unsigned x)
{
  floatie result;
  result.i = T2H_4 (x);
  return result;
}


#ifndef min
#define min(x,y) ((x)<(y)?(x):(y))
#endif

#ifndef max
#define max(x,y) ((x)>(y)?(x):(y))
#endif


/*
 * Interface to accurate floating point library.
 */

#ifdef SKY_FUNIT
#define ACCURATE_FP() (STATE_FP_TYPE_OPT(CURRENT_STATE) & STATE_FP_TYPE_OPT_ACCURATE)

/* Flag bits, set as a side-effect of calling accurate_fmacN() */
static bit fmac_O = 0, fmac_U = 0, fmac_S = 0, fmac_Z = 0;
static bit fmac_moflw = 0, fmac_muflw = 0, fmac_msign = 0, fmac_mzero = 0; /* set by MAC mul part */
static bit fmac_acc_oflw = 0;
#else
#define ACCURATE_FP() 0
typedef unsigned int bit;
enum operation {F_ADD,   F_SUB,   F_MUL,    F_MADD,   F_MSUB, 
                F_ADDA,  F_SUBA,  F_MULA,   F_MADDA,  F_MSUBA,
                F_MAX,   F_MINI,  F_ABS,    F_CLIP,
                F_ITOF0, F_ITOF4, F_ITOF12, F_ITOF15,
                F_FTOI0, F_FTOI4, F_FTOI12, F_FTOI15,
                F_MOVE,  F_NEG,   F_CMP};
enum div_operation
  {
    F_DIV, F_SQRT, F_RSQRT
  };

/* These dummy variables simplify funit-dependent logic later. */
static bit fmac_acc_oflw = 0;
static bit fmac_O = 0;
#endif


/* An odd little test.  -0.0 is negative.  NaN is not negative. */
static int
is_signed (floatie arg)
{
  if (arg.i & 0x80000000)
    return 1;
  else
    return 0;
}


static floatie
accurate_fmac3 (enum operation op, floatie acc_in, bit acc_oflw, floatie lhs, floatie rhs)
{
#ifdef SKY_FUNIT
  word ops, opt;
  word rst, acc;
  bit acc_zero;
  bit cplus, cminus;
  bit ceq, cgt, clt;		/* flags for FPU */
  bit opszero, optzero;
  floatie result;

  ops.i = lhs.i;
  opt.i = rhs.i;
  acc.i = acc_in.i;
  fmac_mzero = fmac_msign = fmac_muflw = fmac_moflw = 0;
  fmac_acc_oflw = acc_oflw;
  acc_zero = (acc.u == 0) ? 1 : 0;
  wallace_flag = 1;
  if (indebug("inst_trace_fmac"))
    {
      printf( "FMAC setup: acc=0x%08x acc-OZ=%d%d ops=0x%08x opt=0x%08x\n",
              acc.u, fmac_acc_oflw, acc_zero, ops.u, opt.u);
    }
  fmac (op, ops, opt, &rst, &acc, &fmac_acc_oflw, &acc_zero,
        &fmac_O, &fmac_U, &fmac_S, &fmac_Z,
        &fmac_moflw, &fmac_muflw, &fmac_msign, &fmac_mzero,
        &cplus, &cminus,
        &ceq, &cgt, &clt,
        &opszero, &optzero);
  if (indebug("inst_trace_fmac"))
    {
      printf( "FMAC rslts: acc=0x%08x acc-OZ=%d%d\n", acc.u, fmac_acc_oflw, acc_zero);
      printf( "\"           rst=0x%08x mult-OUSZ=%d%d%d%d OUSZ=%d%d%d%d\n",
              rst, fmac_moflw, fmac_muflw, fmac_msign, fmac_mzero, fmac_O, fmac_U, fmac_S, fmac_Z);
    }
  ASSERT (sizeof (fp_word) == sizeof (float));
  result.i = rst.i;
  return result;
#else
  floatie junk;
  ASSERT (0);
  return junk;
#endif
}
  
static floatie
accurate_fmac2 (enum operation op, floatie lhs, floatie rhs)
{
#ifdef SKY_FUNIT
  floatie acc;
  bit acc_oflw;

  acc.f = 0.0;
  acc_oflw = 0;
  return accurate_fmac3 (op, acc, acc_oflw, lhs, rhs);
#else
  floatie junk;
  ASSERT (0);
  return junk;
#endif
}

static floatie
accurate_fmac1 (enum operation op, floatie lhs)
{
  floatie rhs;
  rhs.f = 0.0;
  return accurate_fmac2 (op, lhs, rhs);
}

/* Flag bits, set as a side-effect of calling accurate_fdiv() */
static bit fdiv_I, fdiv_D;

static floatie
accurate_fdiv (enum div_operation op, floatie lhs, floatie rhs)
{
  floatie result;
#ifdef SKY_FUNIT
  word fs, ft;
  word qt;
  bit oflw, uflw, sign, zero;

  ASSERT (sizeof (fp_word) == sizeof (float));

  fs.i = lhs.i;
  ft.i = rhs.i;
  fdiv (op, fs, ft, &qt, &fdiv_D, &fdiv_I, &oflw, &uflw, &sign, &zero);
  result.i = qt.i;
  return result;
#else
  ASSERT (0);
  return result;
#endif
}


static floatie
FIntConv (enum operation op, floatie rhs)
{
  floatie lhs;
  if (ACCURATE_FP ())
    {
      lhs = accurate_fmac1 (op, rhs);
    }
  else
    {
      switch(op)
	{
        case F_ITOF0:   lhs.f = rhs.i;                  break;
        case F_ITOF4:   lhs.f = rhs.i / 16.0;           break;
        case F_ITOF12:  lhs.f = rhs.i / 4096.0;         break;
        case F_ITOF15:  lhs.f = rhs.i / 32768.0;        break;
        case F_FTOI0:   lhs.i = rhs.f;                  break;
        case F_FTOI4:   lhs.i = rhs.f * 16.0;           break;
        case F_FTOI12:  lhs.i = rhs.f * 4096.0;         break;
        case F_FTOI15:  lhs.i = rhs.f * 32768.0;        break;
	default:        abort();
	}
    }
  return lhs;
}


static int
FCmp (floatie lhs, char op, floatie rhs)  /* op: '<', '=', '>' */
{
  if (ACCURATE_FP ())
    {
#ifdef SKY_FUNIT
      /* The 1998-02 era funit library ignores the input numbers'
         signs, so claims -2 < -4!!  We have to work around this by
	 logic that post-processes the comparison output flags. */

      unsigned ls = is_signed(lhs);
      unsigned rs = is_signed(rhs);

      /* almost all of these are dummy variables */
      word ops, opt;
      word rst, acc;
      bit acc_oflw, acc_zero;
      bit moflw, muflw, msign, mzero;	/* flags for MAC mul part */
      bit cplus, cminus;
      bit opszero, optzero;
      bit fmacO, fmacU, fmacS, fmacZ;
      /* use only these: */
      bit ceq, cgt, clt;		/* flags for FPU */

      if(indebug("inst_trace2"))
	{
	  fprintf (stdout, "[%c %08x] ?%c? [%c %08x]",
		   (ls ? '-' : ' '),
		   lhs.i,
		   op,
		   (rs ? '-' : ' '),
		   rhs.i);
	}

      ops.i = lhs.i & 0x7fffffff;
      opt.i = rhs.i & 0x7fffffff;
      acc.f = 0.0;
      rst.f = 0.0;
      acc_oflw = 0;
      acc_zero = 1;
      fmac (F_CMP, ops, opt, &rst, &acc,
	    &acc_oflw, &acc_zero,
	    &fmacO, &fmacU, &fmacS, &fmacZ,
	    &moflw, &muflw, &msign, &mzero,
	    &cplus, &cminus,
	    &ceq, &cgt, &clt,
	    &opszero, &optzero);

      /* post-processing time */
      if(!ls && !rs) /* both positive */
	{
	  ; /* nothing to do */
	}
      else if(ls && !rs) /* negative OP positive -> set .lt. */
	{
	  cgt = 0;
	  ceq = 0;
	  clt = 1;
	}
      else if(!ls && rs) /* positive OP negative -> set .gt. */
	{
	  cgt = 1;
	  ceq = 0;
	  clt = 0;
	}
      else /* negative OP negative -> exchange .gt. and .lt. */
	{
	  int tmp;
	  tmp = clt;
	  clt = cgt;
	  cgt = tmp;
	}			

      if(indebug("inst_trace2"))
	{
	  fprintf (stdout, " ==> %f[%08x] ?%c? %f[%08x]: %c %c %c\n",
		   lhs.f, lhs.i, op, rhs.f, rhs.i,
		   clt ? '<' : ' ',
		   ceq ? '=' : ' ',
		   cgt ? '>' : ' ');
	}

      switch(op)
	{
	case '<': return clt;
	case '=': return ceq;
	case '>': return cgt;
	}
#else
      ASSERT (0);
#endif
    }
  else
    {
      switch(op)
	{
	case '<': return (lhs.f < rhs.f);
	case '=': return (lhs.f == rhs.f);
	case '>': return (lhs.f > rhs.f);
	}
    }
  abort ();
}



static floatie
FAbs (floatie rhs)
{
  if (ACCURATE_FP ())
    {
      return accurate_fmac1 (F_ABS, rhs);
    }
  else
    {
      floatie result;
      result.i = rhs.i & 0x7fffffff;
      return result;
    }
}

static floatie
FAdd (floatie lhs, floatie rhs)
{
  if (ACCURATE_FP ())
    return accurate_fmac2 (F_ADD, lhs, rhs);
  else
    {
      floatie result;
      result.f = lhs.f + rhs.f;
      return result;
    }
}

static floatie
FSub (floatie lhs, floatie rhs)
{
  if (ACCURATE_FP ())
    return accurate_fmac2 (F_SUB, lhs, rhs);
  else
    {
      floatie result;
      result.f = lhs.f - rhs.f;
      return result;
    }
}

static floatie
FMul (floatie lhs, floatie rhs)
{
  if (ACCURATE_FP ())
    return accurate_fmac2 (F_MUL, lhs, rhs);
  else
    {
      floatie result;
      result.f = lhs.f * rhs.f;
      return result;
    }
}

static floatie
FMAdd (floatie acc, bit acc_oflw, floatie lhs, floatie rhs)
{
  if (ACCURATE_FP ())
    return accurate_fmac3 (F_MADD, acc, acc_oflw, lhs, rhs);
  else
    {
      floatie result;
      /* Do not remove "temp", Ian found that it is required to get
      the correct results for SCEI on Linux.  jlemke 1998-09-24 */
      volatile float temp;
      temp = lhs.f * rhs.f;
      result.f = acc.f + temp;
      return result;
    }
}

static floatie
FMSub (floatie acc, bit acc_oflw, floatie lhs, floatie rhs)
{
  if (ACCURATE_FP ())
    return accurate_fmac3 (F_MSUB, acc, acc_oflw, lhs, rhs);
  else
    {
      floatie result;
      /* Do not remove "temp", Ian found that it is required to get
      the correct results for SCEI on Linux.  jlemke 1998-09-24 */
      volatile float temp;
      temp = lhs.f * rhs.f;
      result.f = acc.f - temp;
      return result;
    }
}

static floatie
FDiv (floatie lhs, floatie rhs)
{
  if (ACCURATE_FP ())
    return accurate_fdiv (F_DIV, lhs, rhs);
  else
    {
      floatie result;
      result.f = lhs.f / rhs.f;
      return result;
    }
}

static floatie
FDiv_Stat (floatie lhs, floatie rhs, u_long * status)
{
  *status = 0;
  if (ACCURATE_FP ())
    {
      floatie result;
      result = accurate_fdiv (F_DIV, lhs, rhs);
      *status &= ~0x30;
      if (fdiv_I)
	*status |= 0x10;	/* Inavlid Bit */
      if (fdiv_D)
	*status |= 0x20;	/* Divide by zero Bit */
      return result;
    }
  else
    {
      floatie result;

      if (rhs.f == 0.0)
	{
	  if (lhs.f == 0.0)
	    {
	      *status |= 0x10;	/* Invalid bit */
	      result.i = 0x7fffffff;
	    }
	  else
	    {
	      *status |= 0x20;	/* Divide by Zero Bit */
	      result.i = 0x7fffffff;
	    }
	}
      else
	result.f = lhs.f / rhs.f;

      return result;
    }
}

static floatie
FSqrt (floatie rhs)
{
  if (ACCURATE_FP ())
    {
      floatie lhs;
      lhs.f = 1.0;
      return accurate_fdiv (F_SQRT, lhs, rhs);
    }
  else
    {
      floatie result;
      result.f = sqrt (rhs.f);
      return result;
    }
}

static floatie
FSqrt_Stat (floatie rhs, u_long * status)
{
  *status = 0;
  if (ACCURATE_FP ())
    {
      floatie result;
      floatie lhs;
      lhs.f = 1.0;
      result = accurate_fdiv (F_SQRT, lhs, rhs);
      *status &= ~0x30;
      if (fdiv_I)
	*status |= 0x10;	/* Inavlid Bit */
      if (fdiv_D)
	*status |= 0x20;	/* Divide by zero Bit */
      return result;
    }
  else
    {
      floatie result;
      if (rhs.f < 0.0)
	{
	  *status |= 0x10;
	  result.f = sqrt (-rhs.f);
	}
      else
	{
	  result.f = sqrt (rhs.f);
	}
      return result;
    }
}

static floatie
FRSqrt (floatie lhs, floatie rhs)
{
  if (ACCURATE_FP ())
    return accurate_fdiv (F_RSQRT, lhs, rhs);
  else
    {
      floatie result;
      result.f = lhs.f / sqrt (rhs.f);
      return result;
    }
}


static floatie
FRSqrt_Stat (floatie lhs, floatie rhs, u_long * status)
{
  *status = 0;
  if (ACCURATE_FP ())
    {
      floatie result;
      result = accurate_fdiv (F_RSQRT, lhs, rhs);
      *status &= ~0x30;
      if (fdiv_I)
	*status |= 0x10;	/* Invalid Bit */
      if (fdiv_D)
	*status |= 0x20;	/* Divide by zero Bit */
      return result;
    }
  else
    {
      float fval;
      floatie result;

      fval = rhs.f;
      if (rhs.f < 0.0)
	{
	  fval = -fval;
	  *status |= 0x10;
	}
      fval = sqrt (fval);

      if (fval == 0.0)
	{
	  *status |= 0x20;
	  result.i = 0x7fffffff;
	}
      else
	{
	  result.f = lhs.f / fval;
	}

      return result;
    }
}

static void
trace_x (vu_device *me, char * opcode, char * target, int value) 
{
  if(indebug("inst_trace2"))
    {
      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
               "\t%s %s=0x%08x\n", opcode, target, value);
    }
}

static void
trace_vi_x (vu_device *me, char * opcode, int regnum, int value)
{
  if(indebug("inst_trace2"))
    {
      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
               "\t%s VI%02d=0x%08x\n", opcode, regnum, value);
    }
}

void
trace_ACC_2 (u_long opcode, floatie x, floatie y, floatie z, floatie w)
{
  /* ??? */
}


/****************************************************************************/
/* Some forward declarations ... */
static void GpuIfKick (vu_device * me, int addr);

static int
get_TOP (vu_device * me)
{
  sim_cpu *cpu;
  cpu = STATE_CPU (CURRENT_STATE, 0);
  return sim_core_read_aligned_4 (cpu, NULL_CIA, read_map, 0x10003ce0);
}

static int
get_ITOP (vu_device * me)
{
  sim_cpu *cpu;
  cpu = STATE_CPU (CURRENT_STATE, 0);
  return sim_core_read_aligned_4 (cpu, NULL_CIA, read_map, 0x10003cd0);
}

#define range_check mem_range_check

/* Signal a memory access error, address ADDR is invalid.  */

static void
mem_range_error (vu_device * me, int addr)
{
  fprintf (stderr, "Out of bounds memory reference!\n");
  /* ??? Later might wish to do something more clever than print
     an error message (or even just print a more intelligent message).  */
}

static void
mem_range_check (vu_device * me, int i)
{
  if (i < 0 || i > 2047)
    mem_range_error (me, i);
}

/* These implement mere memory address wrapping.  VU0 / VU1 cross-mapped
   memories aren't yet implemented. */

#define MEM(index,word) (me->MEM_buffer[(index) % me->MEM_size][(word)])
#define uMEM(index,word) (me->uMEM_buffer[(index) % me->uMEM_size][(word)])

#define VF me->regs.VF
#define VI me->regs.VI

#define pipe me->pipeline.pipe
#define qpipe me->pipeline.qpipe
#define ipipe me->pipeline.ipipe
#define apipe me->pipeline.apipe
#define Ipipe me->pipeline.Ipipe
#define spipe me->pipeline.spipe

#define _pgpuif         me->junk._pgpuif

#define _is_dbg         me->junk._is_dbg
#define _is_verb        me->junk._is_verb
#define _is_dump        me->junk._is_dump
#define _fp_gpu         me->junk._fp_gpu

#define _GIF_SIM_OFF    me->junk._GIF_SIM_OFF
#define _GIF_BUSY       me->junk._GIF_BUSY
#define _GIF_VUCALL     me->junk._GIF_VUCALL
#define _GIF_VUADDR     me->junk._GIF_VUADDR

#define instbuf         me->junk.instbuf
#define pc              me->junk.pc
#define opc             me->junk.opc
#define jaddr           me->junk.jaddr

#ifdef ACC
#undef ACC
#endif

#define ACC             me->regs.acc
#define ACC_OFLW        me->regs.acc_oflw
#define Q               me->regs.Q
#define I               me->regs.I
#define R               me->regs.R
#define VN              me->regs.VN
#define MACflag         me->regs.MACflag
#define statusflag      me->regs.statusflag
#define clipflag        me->regs.clipflag

#define eflag           me->junk.eflag
#define jflag           me->junk.jflag
#define peflag          me->junk.peflag
#define sflag           me->junk.sflag
#define lflag           me->junk.lflag
#define mflag           me->junk.mflag
#define dflag           me->junk.dflag
#define tflag           me->junk.tflag

#define bp              me->junk.bp
#define ecount          me->junk.ecount
#define intr_mode       me->junk.intr_mode
#define verb_mode       me->junk.verb_mode
#define dpr_mode        me->junk.dpr_mode
#define all_count       me->junk.all_count
#define hc_count        me->junk.hc_count

#define MEM_UPPER 1
#define MEM_LOWER 0

#undef S1
#undef S2
#undef S3
#undef S4
#undef S5
#undef T1
#undef T2
#undef T3
#undef T4
#undef T5
#undef T6
#undef T7
#undef T8
#undef PI_4

#define S1 VN[9]
#define S2 VN[10]
#define S3 VN[11]
#define S4 VN[12]
#define S5 VN[13]
#define T1 VN[14]
#define T2 VN[15]
#define T3 VN[16]
#define T4 VN[17]
#define T5 VN[18]
#define T6 VN[19]
#define T7 VN[20]
#define T8 VN[21]
#define PI_4 VN[22]
#define E1 VN[23]
#define E2 VN[24]
#define E3 VN[25]
#define E4 VN[26]
#define E5 VN[27]
#define E6 VN[28]

void
initvpe (vu_device * me)
{
/* 
   [name]               initvpe
   [desc]               Initialize internal status of VU simulator.
   [args]               void
   [return]     void
 */
  int i, j;

  _fp_gpu = stdout;
  VF[0][0].f = 0.0;
  VF[0][1].f = 0.0; /* was: 0.5 */
  VF[0][2].f = 0.0; /* was: -1.0 */
  VF[0][3].f = 1.0;
  VI[0] = 0;
  eflag = 0;
  jflag = 0;
  peflag = 0;
  sflag = 0;
  dflag = 0;
  tflag = 0;
  jaddr = 0;
  bp = 0xffffffff;
  for (i = 0; i < 4; i++)
    {
      for (j = 0; j < 2; j++)
	{
	  pipe[i][j].flag = P_EMPTY;
	}
    }
  ipipe.old_reg = -1;
  ipipe.flag = 0;
  apipe.flag = 0;
  Ipipe.flag = 0;

  spipe.no = 0;

  qpipe.no = 0;

  for (i = 1; i < 16; i++)
    {
      VI[i] = 0;
    }
  for (i = 1; i < 32; i++)
    {
      for (j = 0; j < 4; j++)
	{
	  VF[i][j].f = 0.0;
	}
    }
  /* EFU internal regstars */

  for (i = 0; i < 32; i++)
    VN[i].f = 0.0;

  VN[8].f = 1.0;
  VN[9].f = 1.0;			/* S1 */
  VN[10].f = -0.166666567325592;	/* S2 */
  VN[11].f = 0.008333025500178;	/* S3 */
  VN[12].f = -0.000198074136279;	/* S4 */
  VN[13].f = 0.000002601886990;	/* S5 */
  VN[14].f = 0.999999344348907;	/* T1 */
  VN[15].f = -0.333298563957214;	/* T2 */
  VN[16].f = 0.199465364217758;	/* T3 */
  VN[17].f = -0.139085337519646;	/* T4 */
  VN[18].f = 0.096420042216778;	/* T5 */
  VN[19].f = -0.055909886956215;	/* T6 */
  VN[20].f = 0.021861229091883;	/* T7 */
  VN[21].f = -0.004054057877511;	/* T8 */
  VN[22].f = 0.7853981633974483;	/* PI/4 */
  VN[23].f = 0.2499986842;	/* E1 */
  VN[24].f = 0.0312575832;	/* E2 */
  VN[25].f = 0.0025913712;	/* E3 */
  VN[26].f = 0.0001715620;	/* E4 */
  VN[27].f = 0.0000054302;	/* E5 */
  VN[28].f = 0.0000006906;	/* E6 */

  ACC[0].f = ACC[1].f = ACC[2].f = ACC[3].f = 0.0;
  Q.f = I.f = 0.0;
  R = MACflag = statusflag = clipflag = 0;

  _GIF_SIM_OFF = 0;
  _GIF_BUSY = 0;
  _GIF_VUCALL = 0;
  _GIF_VUADDR = 0;
  intr_mode = _is_dbg;
  ecount = 0;
}

/* Cover function to access VF registers.
   ??? This is needed because VF is both a macro and the member name.  */

static floatie *
get_VF (vu_device * me, int regno)
{
  return &VF[regno][0];
}

/* Cover function to access VI registers.
   ??? This is needed because VI is both a macro and the member name.  */

static short *
get_VI (vu_device * me, int regno)
{
  return &VI[regno];
}

static int
get_dest (u_long opcode)
{
/*  
   [name]               get_dest
   [desc.]              get dest field of instruction.
   [args]               opcode: instruction(32bits)
   [return]     dest field value(4bits)
 */
  return ((int) ((opcode >> 21) & 0xf));
}

static int
get_ft (u_long opcode)
{
/*
   [name]               get_ft
   [desc.]              get ft field of instruction 
   [args]               opcode: instruction(32bits)
   [return]     ft field value(5bits)
 */
  return ((int) ((opcode >> 16) & 0x1f));
}

static int
get_fs (u_long opcode)
{
/*
   [name]               get_fs
   [desc.]              get fs field of instruction 
   [args]               opcode: instruction(32bits)
   [return]     fs field value(5bits)
 */
  return ((int) ((opcode >> 11) & 0x1f));
}

static int
get_fd (u_long opcode)
{
/*
   [name]               get_fd
   [desc.]              get fd field of instruction 
   [args]               opcode: instruction(32bits)
   [return]     fd field value(5bits)
 */
  return ((int) ((opcode >> 6) & 0x1f));
}

static int
get_bc (u_long opcode)
{
/*
   [name]               get_bc
   [desc.]              get bc field of instruction
   [args]               opcode: instruction(32bit)
   [return]     bc field value(2bits);
 */
  return ((int) (opcode & 0x3));
}

static int
get_ic (u_long opcode)
{
/*
   [name]               get_ic
   [desc.]              get bit25 and bit26 in instruction
   [args]               opcode: instruction(32bit)
   [return]     bit 25 and bit26 value(2bits)
 */
  return ((int) ((opcode >> 25) & 0x3));
}

static int
get_fsf (u_long opcode)
{
/*
   [name]               get_fsf
   [desc.]              get fsf field of instruction
   [args]               opcode: instruction(32bit)
   [return]     fsf field value(2bits)
 */
  return ((opcode >> 21) & 0x3);
}

static int
get_ftf (u_long opcode)
{
/*
   [name]               get_ftf
   [desc.]              get ftf field of instruction
   [args]               opcode: instruction(32bit)
   [return]     ftf field value(2bits)
 */
  return ((opcode >> 23) & 0x3);
}

#define BURST_UPPER0(opcode, dest, ft, fs, fd, bc) \
        dest = get_dest(opcode); \
        ft = get_ft(opcode); \
        fs = get_fs(opcode); \
        fd = get_fd(opcode); \
        bc = get_bc(opcode)

#define BURST_UPPER2(opcode, dest, ft, fs, bc) \
        dest = get_dest(opcode); \
        ft = get_ft(opcode); \
        fs = get_fs(opcode); \
        bc = get_bc(opcode)

/* vu0/vu1 reg access support */

/* Return value of a ctrl reg.
   ADDR is defined by the vu0/vu1 interface.  */

static unsigned int
get_ctrlreg (vu_device * me, int addr)
{
  unsigned int val = 0;

  addr -= VU0_VU1_CTRLREG_START;
  switch (addr)
    {
    case 0: /* status flag */
      val = statusflag & 0xfff;
      break;
    case 1: /* MAC flag */
      val = MACflag & 0xffff;
      break;
    case 2: /* clipping flag */
      val = clipflag & 0xffffff;
      break;
    case 4: /* R register */
      val = R & 0x7fffff;
      break;
    case 5: /* I register */
      val = I.i;
      break;
    case 6: /* Q register */
      val = Q.i;
      break;
    case 7: /* P register */
      val = VN[4].i;
      break;
    case 0xa: /* TPC */
      val = pc & 0xffff;
      break;
    default:
      mem_range_error (me, addr);
      break;
    }

  return val;
}

/* Return value of a ctrl reg.
   ADDR is defined by the vu0/vu1 interface.  */

static void
set_ctrlreg (vu_device * me, int addr, unsigned int val)
{
  addr -= VU0_VU1_CTRLREG_START;
  switch (addr)
    {
    case 0: /* status flag */
      /* values written to lower 6 bits are ignored */
      statusflag = (statusflag & 0x3f) | (val & 0xfc0);
      break;
    case 1: /* MAC flag */
      /* Can't write MAC.  See VU Spec 2.10, page 7-9.  */
      /* ??? Perhaps we should print an error message here.  */
      break;
    case 2: /* clipping flag */
      clipflag = val & 0xffffff;
      break;
    case 4: /* R register */
      R = val & 0x7fffff;
      break;
    case 5: /* I register */
      I.i = val;
      break;
    case 6: /* Q register */
      Q.i = val;
      break;
    case 7: /* P register */
      VN[4].i = val;
      break;
    case 0xa: /* TPC */
      /* Can't write TPC.  See VU Spec 2.10, page 7-9.  */
      /* ??? Perhaps we should print an error message here.  */
      break;
    default:
      mem_range_error (me, addr);
      break;
    }
}


static void
statuscheck_fmac (floatie arg, u_long * status, int no)
{
/*
   [name]               statuscheck_fmac
   [desc.]              FMAC status decide when FMAC calculates.
   [args]               arg: calculated value by FMAC.
   status: FMAC status pointer.
   no: FMAC number
   [return]     void
 */
    u_long old_status = *status;

  /* write status flag's value */

#ifdef SKY_FUNIT
  if (ACCURATE_FP ())
    {
      if (fmac_Z)
	*status = *status | 0x001;
      if (fmac_S)
	*status = *status | 0x002;
      if (fmac_U)
	*status = *status | 0x004;
      if (fmac_O)
	*status = *status | 0x008;
      if (fmac_mzero)
        *status = *status | 0x040;
      if (fmac_msign)
	*status = *status | 0x080;
      if (fmac_muflw)
	*status = *status | 0x100;
      if (fmac_moflw)
	*status = *status | 0x200;
      switch (no)
	{
	case 0:
	  if (fmac_Z)
	    *status |= 0x80000;
	  if (fmac_S)
	    *status |= 0x800000;
	  if (fmac_U)
	    *status |= 0x8000000;
	  if (fmac_O)
	    *status |= 0x80000000;
	  break;
	case 1:
	  if (fmac_Z)
	    *status |= 0x40000;
	  if (fmac_S)
	    *status |= 0x400000;
	  if (fmac_U)
	    *status |= 0x4000000;
	  if (fmac_O)
	    *status |= 0x40000000;
	  break;
	case 2:
	  if (fmac_Z)
	    *status |= 0x20000;
	  if (fmac_S)
	    *status |= 0x200000;
	  if (fmac_U)
	    *status |= 0x2000000;
	  if (fmac_O)
	    *status |= 0x20000000;
	  break;
	case 3:
	  if (fmac_Z)
	    *status |= 0x10000;
	  if (fmac_S)
	    *status |= 0x100000;
	  if (fmac_U)
	    *status |= 0x1000000;
	  if (fmac_O)
	    *status |= 0x10000000;
	  break;
	}
    }
  else
#endif /* SKY_FUNIT */
    {
      if (arg.f == 0.0)
	*status = *status | 1;
      if (is_signed (arg))
	*status = *status | 2;
      /* write MACflag's value */
      switch (no)
	{
	case 0:
	  if (arg.f == 0.0)
	    *status |= 0x80000;
	  if (is_signed (arg))
	    *status |= 0x800000;
	  break;
	case 1:
	  if (arg.f == 0.0)
	    *status |= 0x40000;
	  if (is_signed (arg))
	    *status |= 0x400000;
	  break;
	case 2:
	  if (arg.f == 0.0)
	    *status |= 0x20000;
	  if (is_signed (arg))
	    *status |= 0x200000;
	  break;
	case 3:
	  if (arg.f == 0.0)
	    *status |= 0x10000;
	  if (is_signed (arg))
	    *status |= 0x100000;
	  break;
	}
    }

  if (indebug ("inst_trace2"))
    {
      fprintf (stdout, /* XXX */
	       "statuscheck_fmac: arg %f, no %d, status %08x-->%08x\n",
	       arg.f, no, old_status, *status);
    }
}

static int
hazardcheck (vu_device * me, int fs, int ft, int dest, int type)
{
/*
   [name]               hazardcheck

   If destinatin registers in pipeline is same as source register,
   data hazard occured.
   [args]               fs: If fs field assigns source register, set source register number
   in this argment.
   ft: If ft field assigns source register, set souce register number
   in this argment
   dest: specify which units calculate
   type: source register type
   1: source register is ft field
   2: source register is fs field
   3: source registers are ft and fs field
   [return]     0: hazard not occured
   1: hazard occured
 */

  int i, j;

  for (i = 3; i > 0; i--) /* exclude current pipeline stage */
    {
      for (j = 0; j < 2; j++)
	{
	  if ((pipe[i][j].flag == P_VF_REG || pipe[i][j].flag == P_VF_REG_NO_STATUS) || (pipe[i][j].flag == P_MFP))
	    {
	      if (type & 0x1)
		{
		  if ((pipe[i][j].no != 0)
		      && (pipe[i][j].no == ft) && (pipe[i][j].mask & dest))
		    {
		      sflag = 1;
		      hc_count++;
		      if (verb_mode)
			printf ("[data hazard 1 (%d,%d) %04ld:V%c%02d]\n", 
				i, j, pc - 1, ((ft > 31) ? 'I' : 'F'), ft & 0x1f);
		      return 1;
		    }
		}
	      if (type & 0x2)
		{
		  if ((pipe[i][j].no != 0)
		      && (pipe[i][j].no == fs) && (pipe[i][j].mask & dest))
		    {
		      sflag = 1;
		      hc_count++;
		      if (verb_mode)
			printf ("[data hazard 2 (%d,%d) %04ld:V%c%02d]\n", 
				i, j, pc - 1, ((fs > 31) ? 'I' : 'F'), fs & 0x1f);
		      return 1;
		    }
		}
	    }
	}
    }
  sflag = 0;
  return 0;
}

static int
hazardcheckMEM (vu_device * me, int addr)
{
/*
   [name]               hazardcheckMEM

   If given address is in the load-store unit write queue, a load/store hazard occurred.
   [return]     0: hazard not occurred
   1: hazard occurred
 */

  int i;

  for (i = 3; i > 0; i--) /* exclude current pipeline stage */
    {
      if (pipe[i][1].flag == P_MEMORY &&
	  pipe[i][1].addr == addr)
	{
	  sflag = 1;
	  hc_count++;
	  if (verb_mode)
	    printf ("[load/store hazard (%d) %04ld: addr %d]\n", i, pc - 1, addr);
	  return 1;
	}
    }

  return 0;
}


static int
hazardcheckVI (vu_device * me, int fs, int ft, int type)
{
/*
   [name]               hazardcheckVI
   [desc.]              data hazarde check. (VI register)
   If destinatin registers in pipeline is same as source register,
   data hazard occured.
   [args]               fs: If fs field assigns source register, set source register number
   in this argment.
   ft: If ft field assigns source register, set souce register number
   in this argment
   type: source register type
   1: source register is ft field
   2: source register is fs field
   3: source registers are ft and fs field
   [return]     0: hazard not occured
   1: hazard occured
 */

  int i;

  if (ipipe.flag != 0)
    {
      if (type & 0x1)
	{
	  if (ipipe.no != 0
	      && ipipe.no == ft)
	    {
	      hc_count++;
	      if (verb_mode)
		printf ("[data hazard %04ld:VI%02d]\n", pc - 1, ft);
	      sflag = 1;
	      return 1;
	    }
	}
      if (type & 0x2)
	{
	  if (ipipe.no != 0
	      && ipipe.no == fs)
	    {
	      hc_count++;
	      if (verb_mode)
		printf ("[data hazard %04ld:VI%02d]\n", pc - 1, fs);
	      sflag = 1;
	      return 1;
	    }
	}
    }

  /* check also the FMAC pipeline for a pending write to VI */
  if(hazardcheck(me, fs + 32, ft + 32, 0xf, type))
    return 1;

  sflag = 0;
  return 0;
}

static int
hazardcheckFTF (vu_device * me, int fs, int ft, int fsf, int ftf, int type)
{
/*
   [name]               hazardcheckFTF
   [desc.]              data hazarde check. (VF register)
   If destinatin registers in pipeline is same as source register,
   data hazard occured.
   [args]               fs: If fs field assigns source register, set source register number
   in this argment.
   ft: If ft field assigns source register, set souce register number
   in this argment
   fsf: If fsf field assigns source register, set source register
   number in this argment.
   ftf: If ftf field assigns source register, set souce register
   number in this argment
   type: source register type
   1: source register is ft field
   2: source register is fs field
   3: source registers are ft and fs field
   [return]     0: hazard not occured
   1: hazard occured
 */

  int i, j;

  for (i = 3; i >= 0; i--)
    {
      for (j = 0; j < 2; j++)
	{
	  if ((pipe[i][j].flag == P_VF_REG || pipe[i][j].flag == P_VF_REG_NO_STATUS) || (pipe[i][j].flag == P_MFP))
	    {
	      if (type & 0x1)
		{
		  if (pipe[i][j].no != 0
		      && pipe[i][j].no == ft)
		    {
		      switch (ftf)
			{
			case 0:
			  if (pipe[i][j].mask & DEST_X)
			    {
			      hc_count++;
			      if (verb_mode)
				printf ("[data hazard %04ld:V%c%02d]\n",
					pc - 1, ((ft > 31) ? 'I' : 'F'), ft & 0x1f);
			      sflag = 1;
			      return 1;
			    }
			  break;
			case 1:
			  if (pipe[i][j].mask & DEST_Y)
			    {
			      hc_count++;
			      if (verb_mode)
				printf ("[data hazard %04ld:V%c%02d]\n",
					pc - 1, ((ft > 31) ? 'I' : 'F'), ft & 0x1f);
			      sflag = 1;
			      return 1;
			    }
			  break;
			case 2:
			  if (pipe[i][j].mask & DEST_Z)
			    {
			      hc_count++;
			      if (verb_mode)
				printf ("[data hazard %04ld:V%c%02d]\n",
					pc - 1, ((ft > 31) ? 'I' : 'F'), ft & 0x1f);
			      sflag = 1;
			      return 1;
			    }
			  break;
			case 3:
			  if (pipe[i][j].mask & DEST_W)
			    {
			      hc_count++;
			      if (verb_mode)
				printf ("[data hazard %04ld:V%c%02d]\n",
					pc - 1, ((ft > 31) ? 'I' : 'F'), ft & 0x1f);
			      sflag = 1;
			      return 1;
			    }
			  break;
			}
		    }
		}
	      if (type & 0x2)
		{
		  if (pipe[i][j].no != 0
		      && pipe[i][j].no == fs)
		    {
		      switch (fsf)
			{
			case 0:
			  if (pipe[i][j].mask & DEST_X)
			    {
			      hc_count++;
			      if (verb_mode)
				printf ("[data hazard %04ld:V%c%02d]\n",
					pc - 1, ((fs > 31) ? 'I' : 'F'), fs & 0x1f);
			      sflag = 1;
			      return 1;
			    }
			  break;
			case 1:
			  if (pipe[i][j].mask & DEST_Y)
			    {
			      hc_count++;
			      if (verb_mode)
				printf ("[data hazard %04ld:V%c%02d]\n",
					pc - 1, ((fs > 31) ? 'I' : 'F'), fs & 0x1f);
			      sflag = 1;
			      return 1;
			    }
			  break;
			case 2:
			  if (pipe[i][j].mask & DEST_Z)
			    {
			      hc_count++;
			      if (verb_mode)
				printf ("[data hazard %04ld:V%c%02d]\n",
					pc - 1, ((fs > 31) ? 'I' : 'F'), fs & 0x1f);
			      sflag = 1;
			      return 1;
			    }
			  break;
			case 3:
			  if (pipe[i][j].mask & DEST_W)
			    {
			      hc_count++;
			      if (verb_mode)
				printf ("[data hazard %04ld:V%c%02d]\n",
					pc - 1, ((fs > 31) ? 'I' : 'F'), fs & 0x1f);
			      sflag = 1;
			      return 1;
			    }
			  break;
			}
		    }
		}
	    }
	}
    }
  sflag = 0;
  return 0;
}

static int
hazardcheckBC (vu_device * me, int fs, int ft, int dest, int bc, int type)
{
/*
   [name]               hazardcheckBC
   [desc.]              data hazarde check. (VF register)
   If destinatin registers in pipeline is same as source register,
   data hazard occured.
   [args]               fs: If fs field assigns source register, set source register number
   in this argment.
   ft: If ft field assigns source register, set souce register number
   in this argment
   dest: specify which units calculate
   bc: specify which broadcast units
   type: source register type
   1: source register is ft field
   2: source register is fs field
   3: source registers are ft and fs field
   [return]     0: hazard not occured
   1: hazard occured
 */

  int i, j;

  for (i = 3; i >= 0; i--)
    {
      for (j = 0; j < 2; j++)
	{
	  if ((pipe[i][j].flag == P_VF_REG || pipe[i][j].flag == P_VF_REG_NO_STATUS) || (pipe[i][j].flag == P_MFP))
	    {
	      if (type & 0x1)
		{
		  if (pipe[i][j].no != 0
		      && pipe[i][j].no == ft)
		    {
		      switch (bc)
			{
			case 0:
			  if (pipe[i][j].mask & DEST_X)
			    {
			      hc_count++;
			      if (verb_mode)
				printf ("[data hazard %04ld:VF%02d]\n",
					pc - 1, ft);
			      sflag = 1;
			      return 1;
			    }
			  break;
			case 1:
			  if (pipe[i][j].mask & DEST_Y)
			    {
			      hc_count++;
			      if (verb_mode)
				printf ("[data hazard %04ld:VF%02d]\n",
					pc - 1, ft);
			      sflag = 1;
			      return 1;
			    }
			  break;
			case 2:
			  if (pipe[i][j].mask & DEST_Z)
			    {
			      hc_count++;
			      if (verb_mode)
				printf ("[data hazard %04ld:VF%02d]\n",
					pc - 1, ft);
			      sflag = 1;
			      return 1;
			    }
			  break;
			case 3:
			  if (pipe[i][j].mask & DEST_W)
			    {
			      hc_count++;
			      if (verb_mode)
				printf ("[data hazard %04ld:VF%02d]\n",
					pc - 1, ft);
			      sflag = 1;
			      return 1;
			    }
			  break;
			}
		    }
		}
	      if (type & 0x2)
		{
		  if ((pipe[i][j].no != 0)
		      && (pipe[i][j].no == fs) && (pipe[i][j].mask & dest))
		    {
		      hc_count++;
		      if (verb_mode)
			printf ("[data hazard %04ld:VF%02d]\n", pc - 1, ft);
		      sflag = 1;
		      return 1;
		    }
		}
	    }
	}
    }
  sflag = 0;
  return 0;
}

static int
hazardcheckQ (vu_device * me)
{
/*
   [name]               hazardcheckQ
   [desc.]              resource hazarde check. (FDIV unit)
   If instruction use FDIV unit when FDIV is calculating,
   resource hazard occured.
   [args]               void
   [return]     0: hazard not occured
   1: hazard occured
 */
  int i;

  if (qpipe.no != 0)
    {
      hc_count++;
      if (verb_mode)
	printf ("[resource hazard %04ld:FDIV]\n", pc - 1);
      sflag = 1;
      return 1;
    }
  sflag = 0;
  return 0;
}

static int
hazardcheckP (vu_device * me, int fromWaitP)
{
/*
   [name]               hazardcheckP
   [desc.]              resource hazarde check. (EFU)
   If instruction use EFU unit when EFU is calculating,
   resource hazard occured.
   [args]               fromWaitP: WAITP criterion vs. ordinary EFU insn criterion
   [return]     0: hazard not occured
   1: hazard occured
 */
  if ((!fromWaitP && (spipe.no != 0)) 
      || (fromWaitP && (spipe.no >= 2)))
    {
      hc_count++;
      if (verb_mode)
	printf ("[resource hazard %04ld:EFU, %d]\n", pc - 1, spipe.no);
      sflag = 1;
      return 1;
    }
  sflag = 0;
  return 0;
}

static int
Lower_special (vu_device * me, u_long iL)
{
/*
   [name]               Lower_special
   [desc.]              Micro Special OPCODE (Lower instruction) execute
   [args]               iL: lower instruction(32bits)
   [return]     0: executed
   1: stall ( hazard occured)
 */

  double dval, ddval;
  floatie fval, fval2;
  floatie x, y, z, w, acc, tmp1, tmp2, tmp3;
  u_long ival;
  bit acc_oflw = 0;
  int tmp;
  floatie one, zero;

  one.f = 1.0;
  zero.f = 0.0;

  if (indebug ("inst_trace2"))
    VU_CHECK_OPEN_DEBUG (me);

  switch ((iL >> 6) & 0x1f)
    {
    case 12:			/* MOVE, MR32, ---, --- */
      switch (get_bc (iL))
	{
	case 0:		/* MOVE */
	  if ((get_fs (iL) != 0) || (get_ft (iL) != 0))
	    {
	      if (hazardcheck (me, get_fs (iL), 0, get_dest (iL), 2))
		{
		  pipe[0][0].flag = P_EMPTY;
		  apipe.flag = 0;
		  return 1;
		}
	      pipe[0][1].no = get_ft (iL);
	      pipe[0][1].mask = get_dest (iL);
	      pipe[0][1].flag = P_VF_REG;
	      pipe[0][1].code = I_MOVE;
	      pipe[0][1].code_addr = opc;
	      if (get_dest (iL) & DEST_X)
		pipe[0][1].vf[0] = VF[get_fs (iL)][0];
	      if (get_dest (iL) & DEST_Y)
		pipe[0][1].vf[1] = VF[get_fs (iL)][1];
	      if (get_dest (iL) & DEST_Z)
		pipe[0][1].vf[2] = VF[get_fs (iL)][2];
	      if (get_dest (iL) & DEST_W)
		pipe[0][1].vf[3] = VF[get_fs (iL)][3];
	    }
	  break;
	case 1:		/* MR32 */
	  /* source fields are rotated w.r.t. dest fields */
	  tmp = get_dest (iL);
	  tmp = ((tmp & 0xe) >> 1) | ((tmp & 0x1) << 3);
	  if (hazardcheck (me, get_fs (iL), 0, tmp, 2))
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
	  pipe[0][1].no = get_ft (iL);
	  pipe[0][1].mask = get_dest (iL);
	  pipe[0][1].flag = P_VF_REG;
	  pipe[0][1].code = I_MR32;
	  pipe[0][1].code_addr = opc;
	  if (get_dest (iL) & DEST_X)
	    pipe[0][1].vf[0] = VF[get_fs (iL)][1];
	  if (get_dest (iL) & DEST_Y)
	    pipe[0][1].vf[1] = VF[get_fs (iL)][2];
	  if (get_dest (iL) & DEST_Z)
	    pipe[0][1].vf[2] = VF[get_fs (iL)][3];
	  if (get_dest (iL) & DEST_W)
	    pipe[0][1].vf[3] = VF[get_fs (iL)][0];
	  break;
	default:
	  fprintf (stderr, "Undefined opcode(%08lx)\n", iL);
	  exit (1);
	}
      break;
    case 13:			/* LQI, SQI, LQD, SQD */
      switch (get_bc (iL))
	{
	case 0:		/* LQI */
	  {
	    int addr = VI[get_fs (iL)];
	    int dest = get_dest (iL);

	    if (hazardcheckVI (me, get_fs (iL), 0, 2) ||
		hazardcheckMEM (me, addr))
	      {
		pipe[0][0].flag = P_EMPTY;
		apipe.flag = 0;
		return 1;
	      }
	    if (indebug ("inst_trace2"))
	      {
		fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
			 "\tLQI VF%02d=(%d)=<%08x,%08x,%08x,%08x>=<%f,%f,%f,%f> ",
			 get_ft(iL), addr,
			 T2H_I4 (MEM (addr, 0)),
			 T2H_I4 (MEM (addr, 1)),
			 T2H_I4 (MEM (addr, 2)),
			 T2H_I4 (MEM (addr, 3)),
			 T2H_F4 (MEM (addr, 0)),
			 T2H_F4 (MEM (addr, 1)),
			 T2H_F4 (MEM (addr, 2)),
			 T2H_F4 (MEM (addr, 3)));
	      }
	    pipe[0][1].no = get_ft (iL);
	    pipe[0][1].mask = dest;
	    pipe[0][1].flag = P_VF_REG;
	    pipe[0][1].code = I_LQI;
	    pipe[0][1].code_addr = opc;
	    /* For vu0 we need to handle memory accesses used to fetch
	       vu1 regs.  We handle the common case of memory first though.  */
	    if (me->config.vu_number == 1
		|| addr < VU0_VU1_REG_START)
	      {
		range_check (me, addr);
		if (dest & DEST_X)
		  pipe[0][1].vf[0] = T2H_F4 (MEM (addr, 0));
		if (dest & DEST_Y)
		  pipe[0][1].vf[1] = T2H_F4 (MEM (addr, 1));
		if (dest & DEST_Z)
		  pipe[0][1].vf[2] = T2H_F4 (MEM (addr, 2));
		if (dest & DEST_W)
		  pipe[0][1].vf[3] = T2H_F4 (MEM (addr, 3));
	      }
	    /* Testing vu_number again isn't technically necessary. 
	       Sue me.  */
	    else if (me->config.vu_number == 0
		     && addr >= VU0_VU1_FPREG_START
		     && addr < (VU0_VU1_FPREG_START + VU0_VU1_FPREG_SIZE))
	      {
		int vu1_regno = addr - VU0_VU1_FPREG_START;
		floatie *vf = get_VF (&vu1_device, vu1_regno);

		if (dest & DEST_X)
		  pipe[0][1].vf[0] = vf[0];
		if (dest & DEST_Y)
		  pipe[0][1].vf[1] = vf[1];
		if (dest & DEST_Z)
		  pipe[0][1].vf[2] = vf[2];
		if (dest & DEST_W)
		  pipe[0][1].vf[3] = vf[3];
	      }
	    else if (me->config.vu_number == 0
		     && addr >= VU0_VU1_CTRLREG_START
		     && addr < (VU0_VU1_CTRLREG_START + VU0_VU1_CTRLREG_SIZE))
	      {
		unsigned val = get_ctrlreg (&vu1_device, addr);

		if (dest & DEST_X)
		  pipe[0][1].vf[0].i = val;
		if (dest & DEST_Y)
		  pipe[0][1].vf[1].i = 0;
		if (dest & DEST_Z)
		  pipe[0][1].vf[2].i = 0;
		if (dest & DEST_W)
		  pipe[0][1].vf[3].i = 0;
	      }
	    else
	      mem_range_error (me, addr);

	    if (indebug ("inst_trace2"))
	      {
		fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
			 " => <%08x,%08x,%08x,%08x>\n",
			 pipe[0][1].vf[0].i,
			 pipe[0][1].vf[1].i,
			 pipe[0][1].vf[2].i,
			 pipe[0][1].vf[3].i);
	      }

	    VI[get_fs (iL)] += 1;
	    break;
	  }
	case 1:		/* SQI */
	  if (indebug ("inst_trace2"))
	    {
#if 0
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		"\tSQI (%d) = <%08x, %08x, %08x, %08x> = <%f, %f, %f, %f>\n",
		       (int) VI[get_ft (iL)],
		       f_2_i (VF[get_fs (iL)][0]),
		       f_2_i (VF[get_fs (iL)][1]),
		       f_2_i (VF[get_fs (iL)][2]),
		       f_2_i (VF[get_fs (iL)][3]),
		       VF[get_fs (iL)][0],
		       VF[get_fs (iL)][1],
		       VF[get_fs (iL)][2],
		       VF[get_fs (iL)][3]);
#endif
	    }
	  if (hazardcheck (me, 0, get_fs (iL), get_dest (iL), 1) ||	/* ihc: ft->fs */
	      hazardcheckVI (me, get_ft (iL), 0, 2))
	    {			/* ihc: fs->ft */
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
	  pipe[0][1].mask = get_dest (iL);
	  pipe[0][1].flag = P_MEMORY;
	  pipe[0][1].addr = VI[get_ft (iL)];
	  pipe[0][1].code = I_SQI;
	  pipe[0][1].code_addr = opc;
	  range_check (me, VI[get_ft (iL)]);	/* ft,fs right way round? TODO -=ihc=- */
	  if (get_dest (iL) & DEST_X)
	    pipe[0][1].vf[0] = VF[get_fs (iL)][0];
	  if (get_dest (iL) & DEST_Y)
	    pipe[0][1].vf[1] = VF[get_fs (iL)][1];
	  if (get_dest (iL) & DEST_Z)
	    pipe[0][1].vf[2] = VF[get_fs (iL)][2];
	  if (get_dest (iL) & DEST_W)
	    pipe[0][1].vf[3] = VF[get_fs (iL)][3];
	  VI[get_ft (iL)] += 1;
	  break;
	case 2:		/* LQD */
	  {
	    int addr = VI[get_fs (iL)] - 1;
	    int dest = get_dest (iL);

	    if (hazardcheckVI (me, get_fs (iL), 0, 2) ||
		hazardcheckMEM (me, addr))
	      {
		pipe[0][0].flag = P_EMPTY;
		apipe.flag = 0;
		return 1;
	      }
	    pipe[0][1].no = get_ft (iL);
	    pipe[0][1].mask = dest;
	    pipe[0][1].flag = P_VF_REG;
	    pipe[0][1].code = I_LQD;
	    pipe[0][1].code_addr = opc;
	    /* For vu0 we need to handle memory accesses used to fetch
	       vu1 regs.  We handle the common case of memory first though.  */
	    if (me->config.vu_number == 1
		|| addr < VU0_VU1_REG_START)
	      {
		range_check (me, addr);
		if (dest & DEST_X)
		  pipe[0][1].vf[0] = T2H_F4 (MEM (addr, 0));
		if (dest & DEST_Y)
		  pipe[0][1].vf[1] = T2H_F4 (MEM (addr, 1));
		if (dest & DEST_Z)
		  pipe[0][1].vf[2] = T2H_F4 (MEM (addr, 2));
		if (dest & DEST_W)
		  pipe[0][1].vf[3] = T2H_F4 (MEM (addr, 3));
	      }
	    /* Testing vu_number again isn't technically necessary. 
	       Sue me.  */
	    else if (me->config.vu_number == 0
		     && addr >= VU0_VU1_FPREG_START
		     && addr < (VU0_VU1_FPREG_START + VU0_VU1_FPREG_SIZE))
	      {
		int vu1_regno = addr - VU0_VU1_FPREG_START;
		floatie *vf = get_VF (&vu1_device, vu1_regno);

		if (dest & DEST_X)
		  pipe[0][1].vf[0] = vf[0];
		if (dest & DEST_Y)
		  pipe[0][1].vf[1] = vf[1];
		if (dest & DEST_Z)
		  pipe[0][1].vf[2] = vf[2];
		if (dest & DEST_W)
		  pipe[0][1].vf[3] = vf[3];
	      }
	    else if (me->config.vu_number == 0
		     && addr >= VU0_VU1_CTRLREG_START
		     && addr < (VU0_VU1_CTRLREG_START + VU0_VU1_CTRLREG_SIZE))
	      {
		unsigned val = get_ctrlreg (&vu1_device, addr);

		if (dest & DEST_X)
		  pipe[0][1].vf[0].i = val;
		if (dest & DEST_Y)
		  pipe[0][1].vf[1].i = 0;
		if (dest & DEST_Z)
		  pipe[0][1].vf[2].i = 0;
		if (dest & DEST_W)
		  pipe[0][1].vf[3].i = 0;
	      }
	    else
	      mem_range_error (me, addr);
	    VI[get_fs (iL)] -= 1;
	    break;
	  }
	case 3:		/* SQD */
	  if (hazardcheck (me, 0, get_fs (iL), get_dest (iL), 1) ||	/* ihc: ft->fs */
	      hazardcheckVI (me, get_ft (iL), 0, 2))
	    {			/* ihc: fs->ft */
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
	  if (indebug ("inst_trace2"))
	    {
#if 0
	      fprintf ((me->debug_file) != NULL ? me->debug_file : stdout,
		"\tSQD (%d) = <%08x, %08x, %08x, %08x> = <%f, %f, %f, %f>\n",
		       (int) (VI[get_ft (iL)] - 1),
		       f_2_i (VF[get_fs (iL)][0]),
		       f_2_i (VF[get_fs (iL)][1]),
		       f_2_i (VF[get_fs (iL)][2]),
		       f_2_i (VF[get_fs (iL)][3]),
		       VF[get_fs (iL)][0],
		       VF[get_fs (iL)][1],
		       VF[get_fs (iL)][2],
		       VF[get_fs (iL)][3]);
#endif
	    }
	  pipe[0][1].mask = get_dest (iL);
	  pipe[0][1].flag = P_MEMORY;
	  pipe[0][1].addr = VI[get_ft (iL)] - 1;
	  range_check (me, VI[get_ft (iL)] - 1);
	  pipe[0][1].code = I_SQD;
	  pipe[0][1].code_addr = opc;
	  if (get_dest (iL) & DEST_X)
	    pipe[0][1].vf[0] = VF[get_fs (iL)][0];
	  if (get_dest (iL) & DEST_Y)
	    pipe[0][1].vf[1] = VF[get_fs (iL)][1];
	  if (get_dest (iL) & DEST_Z)
	    pipe[0][1].vf[2] = VF[get_fs (iL)][2];
	  if (get_dest (iL) & DEST_W)
	    pipe[0][1].vf[3] = VF[get_fs (iL)][3];
	  VI[get_ft (iL)] -= 1;
	  break;
	}
      break;
    case 14:			/* DIV, SQRT, RSQRT, WAITQ */
      switch (get_bc (iL))
	{
	case 0:		/* DIV */
	  if (hazardcheckFTF (me, get_fs (iL), get_ft (iL),
			      get_fsf (iL), get_ftf (iL), 3) ||
	      hazardcheckQ (me))
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
	  if (indebug ("inst_trace2"))
	    {
#if 0
	      fprintf ((me->debug_file) != NULL ? me->debug_file : stdout,
		       "\tDIV Q = %f<%08x> / %f<%08x> = %f<%08x>\n",
		       VF[get_fs (iL)][get_fsf (iL)],
		       f_2_i (VF[get_fs (iL)][get_fsf (iL)]),
		       VF[get_ft (iL)][get_ftf (iL)],
		       f_2_i (VF[get_ft (iL)][get_ftf (iL)]),
		    (VF[get_ft (iL)][get_ftf (iL)] == 0 ? i_2_f (0x7fffffff)
		     : (FDiv (VF[get_fs (iL)][get_fsf (iL)], VF[get_ft (iL)][get_ftf (iL)]))),
		       (VF[get_ft (iL)][get_ftf (iL)] == 0 ? 0x7fffffff
			: f_2_i (FDiv (VF[get_fs (iL)][get_fsf (iL)], VF[get_ft (iL)][get_ftf (iL)])))
		);
#endif
	    }
	  qpipe.no = 7;
	  qpipe.vf = FDiv_Stat (VF[get_fs (iL)][get_fsf (iL)],
				VF[get_ft (iL)][get_ftf (iL)],
				&qpipe.status);
	  qpipe.code = I_DIV;
	  qpipe.code_addr = opc;
	  break;
	case 1:		/* SQRT */
	  if (hazardcheckFTF (me, 0, get_ft (iL), 0, get_ftf (iL), 1) ||
	      hazardcheckQ (me))
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
	  qpipe.no = 7;
	  fval = VF[get_ft (iL)][get_ftf (iL)];
	  qpipe.vf = FSqrt_Stat (fval, &qpipe.status);
	  qpipe.code = I_SQRT;
	  qpipe.code_addr = opc;
	  break;
	case 2:		/* RSQRT */
	  if (hazardcheckFTF (me, get_fs(iL), get_ft (iL), get_fsf(iL), get_ftf (iL), 3) ||
	      hazardcheckQ (me))
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
	  qpipe.no = 13;
	  fval = VF[get_fs (iL)][get_fsf (iL)];	/* lhs */
	  fval2 = VF[get_ft (iL)][get_ftf (iL)];	/* rhs */
	  qpipe.vf = FRSqrt_Stat (fval, fval2, &qpipe.status);
	  qpipe.code = I_RSQRT;
	  qpipe.code_addr = opc;
	  break;
	case 3:		/* WAITQ */
	  if (hazardcheckQ (me))
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
	  break;
	}
      break;
    case 15:			/* MTIR, MFIR, ILWR, ISWR */
      switch (get_bc (iL))
	{
	case 0:		/* MTIR */
	  {
	    int mask = 1 << (3 - get_fsf (iL));
	    if (hazardcheckFTF (me, get_fs (iL), 0, get_fsf (iL), 0, 2))
	      {
		pipe[0][0].flag = P_EMPTY;
		apipe.flag = 0;
		return 1;
	      }
	    pipe[0][1].no = get_ft (iL) + 32;
	    pipe[0][1].mask = mask;
	    pipe[0][1].flag = P_VF_REG;
	    pipe[0][1].code = I_MTIR;
	    pipe[0][1].code_addr = opc;
	    if (mask & DEST_X)
	      pipe[0][1].vf[0] = VF[get_fs (iL)][0];
	    if (mask & DEST_Y)
	      pipe[0][1].vf[1] = VF[get_fs (iL)][1];
	    if (mask & DEST_Z)
	      pipe[0][1].vf[2] = VF[get_fs (iL)][2];
	    if (mask & DEST_W)
	      pipe[0][1].vf[3] = VF[get_fs (iL)][3];
	    break;
	  }
	case 1:		/* MFIR */
	  if (hazardcheckVI (me, get_fs (iL), 0, 2))
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
	  pipe[0][1].no = get_ft (iL);
	  pipe[0][1].mask = get_dest (iL);
	  pipe[0][1].flag = P_VF_REG;
	  pipe[0][1].code = I_MFIR;
	  pipe[0][1].code_addr = opc;
	  ival = VI[get_fs (iL)]; /* sign extended */
	  if (get_dest (iL) & DEST_X)
	    pipe[0][1].vf[0].i = ival;
	  if (get_dest (iL) & DEST_Y)
	    pipe[0][1].vf[1].i = ival;
	  if (get_dest (iL) & DEST_Z)
	    pipe[0][1].vf[2].i = ival;
	  if (get_dest (iL) & DEST_W)
	    pipe[0][1].vf[3].i = ival;
	  break;
	case 2:		/* ILWR */
	  {
	    int addr = VI[get_fs (iL)];
	    int dest = get_dest (iL);

	    if (hazardcheckVI (me, get_fs (iL), 0, 2) ||
		hazardcheckMEM (me, addr))
	      {
		pipe[0][0].flag = P_EMPTY;
		apipe.flag = 0;
		return 1;
	      }
	    /* For vu0 we need to handle memory accesses used to fetch
	       vu1 regs.  We handle the common case of memory first though.  */
	    if (me->config.vu_number == 1
		|| addr < VU0_VU1_REG_START)
	      {
		range_check (me, addr);
		if (dest & DEST_X)
		  pipe[0][1].vf[0].i = T2H_I4 (MEM (addr, 0));
		if (dest & DEST_Y)
		  pipe[0][1].vf[1].i = T2H_I4 (MEM (addr, 1));
		if (dest & DEST_Z)
		  pipe[0][1].vf[2].i = T2H_I4 (MEM (addr, 2));
		if (dest & DEST_W)
		  pipe[0][1].vf[3].i = T2H_I4 (MEM (addr, 3));
	      }
	    /* Testing vu_number again isn't technically necessary. 
	       Sue me.  */
	    else if (me->config.vu_number == 0
		     && addr >= VU0_VU1_INTREG_START
		     && addr < (VU0_VU1_INTREG_START + VU0_VU1_INTREG_SIZE))
	      {
		int vu1_regno = addr - VU0_VU1_INTREG_START;
		short *vi = get_VI (&vu1_device, vu1_regno);

		if (dest & DEST_X)
		  pipe[0][1].vf[0].i = *(unsigned short *) vi;
		if (dest & DEST_Y)
		  pipe[0][1].vf[1].i = 0;
		if (dest & DEST_Z)
		  pipe[0][1].vf[2].i = 0;
		if (dest & DEST_W)
		  pipe[0][1].vf[3].i = 0;
	      }
	    else if (me->config.vu_number == 0
		     && addr >= VU0_VU1_CTRLREG_START
		     && addr < (VU0_VU1_CTRLREG_START + VU0_VU1_CTRLREG_SIZE))
	      {
		unsigned short val = get_ctrlreg (&vu1_device, addr);

		if (dest & DEST_X)
		  pipe[0][1].vf[0].i = val;
		if (dest & DEST_Y)
		  pipe[0][1].vf[1].i = 0;
		if (dest & DEST_Z)
		  pipe[0][1].vf[2].i = 0;
		if (dest & DEST_W)
		  pipe[0][1].vf[3].i = 0;
	      }
	    else
	      mem_range_error (me, addr);
	    pipe[0][1].no = get_ft (iL) + 32;
	    pipe[0][1].mask = get_dest (iL);
	    pipe[0][1].flag = P_VF_REG;
	    pipe[0][1].code = I_ILWR;
	    pipe[0][1].code_addr = opc;
	    break;
	  }
	case 3:		/* ISWR */
	  if (hazardcheckVI (me, get_fs (iL), get_ft (iL), 3))
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
	  pipe[0][1].mask = get_dest (iL);
	  pipe[0][1].flag = P_MEMORY;
	  pipe[0][1].addr = VI[get_fs (iL)];
	  range_check (me, VI[get_fs (iL)]);
	  pipe[0][1].code = I_ISWR;
	  pipe[0][1].code_addr = opc;
	  if (get_dest (iL) & DEST_X)
	    pipe[0][1].vf[0].i = (VI[get_ft (iL)]) & 0xffff;
	  if (get_dest (iL) & DEST_Y)
	    pipe[0][1].vf[1].i = (VI[get_ft (iL)]) & 0xffff;
	  if (get_dest (iL) & DEST_Z)
	    pipe[0][1].vf[2].i = (VI[get_ft (iL)]) & 0xffff;
	  if (get_dest (iL) & DEST_W)
	    pipe[0][1].vf[3].i = (VI[get_ft (iL)]) & 0xffff;
	  if (indebug ("inst_trace2"))
	    {
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\tISWR dest=%x mem[%x]=%04x\n",
		       get_dest(iL), VI[get_fs(iL)], (unsigned)(VI[get_ft(iL)] & 0xffff));
	    }
	  break;
	}
      break;
    case 16:			/* RNEXT, RGET, RINIT, RXOR */
      switch (get_bc (iL))
	{
	case 0:		/* RNEXT */
	  pipe[0][1].no = get_ft (iL);
	  pipe[0][1].mask = get_dest (iL);
	  pipe[0][1].flag = P_VF_REG;
	  pipe[0][1].code = I_RNEXT;
	  pipe[0][1].code_addr = opc;
	  ival = 0;
	  if (((R >> 22) & 0x1) ^ ((R >> 4) & 0x1))
	    ival = 1;
	  R = ((R << 1) + ival) & 0x007fffff;
	  if (get_dest (iL) & DEST_X)
	    pipe[0][1].vf[0].i = 0x3f800000 | R;
	  if (get_dest (iL) & DEST_Y)
	    pipe[0][1].vf[1].i = 0x3f800000 | R;
	  if (get_dest (iL) & DEST_Z)
	    pipe[0][1].vf[2].i = 0x3f800000 | R;
	  if (get_dest (iL) & DEST_W)
	    pipe[0][1].vf[3].i = 0x3f800000 | R;
	  break;
	case 1:		/* RGET */
	  pipe[0][1].no = get_ft (iL);
	  pipe[0][1].mask = get_dest (iL);
	  pipe[0][1].flag = P_VF_REG;
	  pipe[0][1].code = I_RGET;
	  pipe[0][1].code_addr = opc;
	  if (get_dest (iL) & DEST_X)
	    pipe[0][1].vf[0].i = 0x3f800000 | R;
	  if (get_dest (iL) & DEST_Y)
	    pipe[0][1].vf[1].i = 0x3f800000 | R;
	  if (get_dest (iL) & DEST_Z)
	    pipe[0][1].vf[2].i = 0x3f800000 | R;
	  if (get_dest (iL) & DEST_W)
	    pipe[0][1].vf[3].i = 0x3f800000 | R;
	  break;
	case 2:		/* RINIT */
	  if (hazardcheckFTF (me, get_fs (iL), 0, get_fsf (iL), 0, 2))
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
	  R = VF[get_fs (iL)][get_fsf (iL)].i & 0x7fffff;
	  break;
	case 3:		/* RXOR */
	  if (hazardcheckFTF (me, get_fs (iL), 0, get_fsf (iL), 0, 2))
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
	  R = R ^ (VF[get_fs (iL)][get_fsf (iL)].i & 0x7fffff);
	  break;
	}
      break;
    case 25:			/* MFP, ---, ---, --- */
      switch (get_bc (iL))
	{
	case 0:		/* MFP */
	  if (indebug ("inst_trace2"))
	    {
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\tMFP VF%02d.%x = <%f>\n",
		       get_ft (iL), get_dest (iL), VN[4]);
	    }
	  pipe[0][1].no = get_ft (iL);
	  pipe[0][1].mask = get_dest (iL);
	  pipe[0][1].flag = P_MFP;
	  pipe[0][1].code = I_MFP;
	  pipe[0][1].code_addr = opc;
	  if (get_dest (iL) & DEST_X)
	    pipe[0][1].vf[0] = VN[4];
	  if (get_dest (iL) & DEST_Y)
	    pipe[0][1].vf[1] = VN[4];
	  if (get_dest (iL) & DEST_Z)
	    pipe[0][1].vf[2] = VN[4];
	  if (get_dest (iL) & DEST_W)
	    pipe[0][1].vf[3] = VN[4];
	  break;
	default:
	  fprintf (stderr, "Undefined opcode(LowerOp:%08lx)\n", iL);
	  exit (1);
	  break;
	}
      break;
    case 26:			/* XITOP, XTOP */

      switch (get_bc (iL))
	{
	case 0:		/* XTOP */

	  if (indebug ("inst_trace2"))
	    {
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		    "\tXTOP VI[%d]: >>> %d <<<\n", get_ft (iL), get_TOP (me));
	    }
	  ipipe.no = get_ft (iL);
	  ipipe.flag = 1;
	  ipipe.vi = (short) get_TOP (me);
	  ipipe.code = I_XTOP;
	  ipipe.code_addr = opc;
	  break;
	case 1:		/* XITOP */

	  if (indebug ("inst_trace2"))
	    {
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		 "\tXITOP VI[%d]: >>> %ld <<<\n", get_ft (iL), get_ITOP (me));
	    }
	  ipipe.no = get_ft (iL);
	  ipipe.flag = 1;
	  ipipe.vi = (short) get_ITOP (me);
	  ipipe.code = I_XITOP;
	  ipipe.code_addr = opc;
	  break;
	}
      break;
    case 27:			/* XGKICK */
      if (indebug ("inst_trace2") || indebug ("gpuif"))
	{
	  VU_CHECK_OPEN_DEBUG (me);	/* just in case gpuif is the conditional. */
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
	       "\tXGKICK VI[%d]: >>> %d <<<\n", get_fs (iL), VI[get_fs (iL)]);
	}
      GpuIfKick (me, VI[get_fs (iL)]);
      break;
    case 28:			/* ESADD, ERSADD, ELENG, ERLENG */
      switch (get_bc (iL))
	{
	case 0:		/* ESADD */
	  if (hazardcheckP (me, 0) || hazardcheck (me, 0, get_fs (iL), 0xe, 1))
	    {			/* ihc: ft->fs */
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
          acc_oflw = fmac_acc_oflw;     /* save */
          fmac_acc_oflw = 0;
	  x = VF[get_fs (iL)][0];
	  y = VF[get_fs (iL)][1];
	  z = VF[get_fs (iL)][2];
	  acc = FMul (x, x);	/* MULA  ACC, x, x */
	  acc = FMAdd (acc, fmac_acc_oflw, y, y);	/* MADDA ACC, y, y */
	  spipe.vn = FMAdd (acc, fmac_acc_oflw, z, z);		/* MADD  P, z, z */
	  spipe.no = 9;
	  spipe.code = I_ESADD;
	  spipe.code_addr = opc;
          fmac_acc_oflw = acc_oflw;     /* restore */
	  break;

	case 1:		/* ERSADD */
	  if (hazardcheckP (me, 0) || hazardcheck (me, 0, get_fs (iL), 0xe, 1))
	    {			/* ihc: ft->fs */
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
          acc_oflw = fmac_acc_oflw;     /* save */
          fmac_acc_oflw = 0;
	  x = VF[get_fs (iL)][0];
	  y = VF[get_fs (iL)][1];
	  z = VF[get_fs (iL)][2];
	  acc = FMul (x, x);	/* MULA  ACC, x, x */
	  acc = FMAdd (acc, fmac_acc_oflw, y, y);	/* MADDA ACC, y, y */
	  fval = FMAdd (acc, fmac_acc_oflw, z, z);	/* MADD  P,   z, z */
	  spipe.vn = FDiv (one, fval);
	  spipe.no = 16;
	  spipe.code = I_ERSADD;
	  spipe.code_addr = opc;
          fmac_acc_oflw = acc_oflw;     /* restore */
	  break;

	case 2:		/* ELENG */
	  if (hazardcheckP (me, 0) || hazardcheck (me, 0, get_fs (iL), 0xe, 1))
	    {			/* ihc: ft->fs */
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
          acc_oflw = fmac_acc_oflw;     /* save */
          fmac_acc_oflw = 0;
	  x = VF[get_fs (iL)][0];
	  y = VF[get_fs (iL)][1];
	  z = VF[get_fs (iL)][2];

	  acc = FMul (x, x);	/* MULA  ACC, x, x */
	  acc = FMAdd (acc, fmac_acc_oflw, y, y);	/* MADDA ACC, y, y */
	  fval = FMAdd (acc, fmac_acc_oflw, z, z);	/* MADD  P,   z, z */
	  spipe.vn = FSqrt (fval);	/* SQRT  P,   1, P */
	  spipe.no = 16;
	  spipe.code = I_ELENG;
	  spipe.code_addr = opc;
          fmac_acc_oflw = acc_oflw;     /* restore */
	  break;

	case 3:		/* ERLENG */
	  if (hazardcheckP (me, 0) || hazardcheck (me, 0, get_fs (iL), 0xe, 1))
	    {			/* ihc: ft->fs */
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
          acc_oflw = fmac_acc_oflw;     /* save */
          fmac_acc_oflw = 0;
	  x = VF[get_fs (iL)][0];
	  y = VF[get_fs (iL)][1];
	  z = VF[get_fs (iL)][2];

	  acc = FMul (x, x);	/* MULA  ACC, x, x */
	  acc = FMAdd (acc, fmac_acc_oflw, y, y);	/* MADDA ACC, y, y */
	  fval = FMAdd (acc, fmac_acc_oflw, z, z);	/* MADD  P,   z, z */
	  spipe.vn = FRSqrt (one, fval);	/* RSQRT P,   1, P */
	  spipe.no = 22;
	  spipe.code = I_ERLENG;
	  spipe.code_addr = opc;
          fmac_acc_oflw = acc_oflw;     /* restore */
	  if (indebug ("inst_trace2"))
	    {
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\tERLENG P = <%f> == 1 / |<%f,%f,%f>|\n",
		       spipe.vn, x, y, z);
	    }
	  break;
	}
      break;
    case 29:			/* EATANxy, EATANxz, ESUM, --- */
      switch (get_bc (iL))
	{
	case 0:		/* EATANxy */
	  if (hazardcheckP (me, 0) || hazardcheck (me, 0, get_fs (iL), 0xc, 1))
	    {			/* ihc: ft->fs */
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
          acc_oflw = fmac_acc_oflw;     /* save */
          fmac_acc_oflw = 0;
	  x = VF[get_fs (iL)][0];
	  y = VF[get_fs (iL)][1];
	  tmp1 = FAdd (y, x);	/* ADD     tmp1, y, x           */
	  tmp2 = FSub (y, x);	/* SUB     tmp2, y, x           */
	  x = FDiv (tmp2, tmp1);	/* DIV     x, tmp2, tmp1        */
	  tmp3 = FMul (x, x);	/* MUL     tmp3, x, x           */
	  acc = FMul (T1, x);	/* MULA    ACC, T1, x           */
	  tmp1 = FMul (tmp3, x);	/* MUL     tmp1, tmp3, x        */
	  tmp2 = FMul (tmp1, tmp3);	/* MUL     tmp2, tmp1, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp1, T2);	/* MADDA   ACC, tmp1, T2        */
	  tmp1 = FMul (tmp2, tmp3);	/* MUL     tmp1, tmp2, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp2, T3);	/* MADDA   ACC, tmp2, T3        */
	  tmp2 = FMul (tmp1, tmp3);	/* MUL     tmp2, tmp1, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp1, T4);	/* MADDA   ACC, tmp1, T4        */
	  tmp1 = FMul (tmp2, tmp3);	/* MUL     tmp1, tmp2, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp2, T5);	/* MADDA   ACC, tmp2, T5        */
	  tmp2 = FMul (tmp1, tmp3);	/* MUL     tmp2, tmp1, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp1, T6);	/* MADDA   ACC, tmp1, T6        */
	  tmp1 = FMul (tmp2, tmp3);	/* MUL     tmp1, tmp2, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp2, T7);	/* MADDA   ACC, tmp2, T7        */
	  acc = FMAdd (acc, fmac_acc_oflw, one, PI_4);		/* MADDA   ACC, 1, PI/4         */
	  spipe.vn = FMAdd (acc, fmac_acc_oflw, tmp1, T8);	/* MADD  P, tmp1, T8          */
	  spipe.no = 52;
	  spipe.code = I_EATANxy;
	  spipe.code_addr = opc;
          fmac_acc_oflw = acc_oflw;     /* restore */
	  break;
	case 1:		/* EATANxz */
	  if (hazardcheckP (me, 0) || hazardcheck (me, 0, get_fs (iL), 0xa, 1))
	    {			/* ihc: ft->fs */
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
          acc_oflw = fmac_acc_oflw;     /* save */
          fmac_acc_oflw = 0;
	  x = VF[get_fs (iL)][0];
	  z = VF[get_fs (iL)][2];
	  tmp1 = FAdd (z, x);	/* ADD     tmp1, z, x           */
	  tmp2 = FSub (z, x);	/* SUB     tmp2, z, x           */
	  x = FDiv (tmp2, tmp1);	/* DIV     x, tmp2, tmp1        */
	  tmp3 = FMul (x, x);	/* MUL     tmp3, x, x           */
	  acc = FMul (T1, x);	/* MULA    ACC, T1, x           */
	  tmp1 = FMul (tmp3, x);	/* MUL     tmp1, tmp3, x        */
	  tmp2 = FMul (tmp1, tmp3);	/* MUL     tmp2, tmp1, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp1, T2);	/* MADDA   ACC, tmp1, T2        */
	  tmp1 = FMul (tmp2, tmp3);	/* MUL     tmp1, tmp2, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp2, T3);	/* MADDA   ACC, tmp2, T3        */
	  tmp2 = FMul (tmp1, tmp3);	/* MUL     tmp2, tmp1, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp1, T4);	/* MADDA   ACC, tmp1, T4        */
	  tmp1 = FMul (tmp2, tmp3);	/* MUL     tmp1, tmp2, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp2, T5);	/* MADDA   ACC, tmp2, T5        */
	  tmp2 = FMul (tmp1, tmp3);	/* MUL     tmp2, tmp1, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp1, T6);	/* MADDA   ACC, tmp1, T6        */
	  tmp1 = FMul (tmp2, tmp3);	/* MUL     tmp1, tmp2, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp2, T7);	/* MADDA   ACC, tmp2, T7        */
	  acc = FMAdd (acc, fmac_acc_oflw, one, PI_4);		/* MADDA   ACC, 1, PI/4         */
	  spipe.vn = FMAdd (acc, fmac_acc_oflw, tmp1, T8);	/* MADD  P, tmp1, T8          */
	  spipe.no = 52;
	  spipe.code = I_EATANxz;
	  spipe.code_addr = opc;
          fmac_acc_oflw = acc_oflw;     /* restore */
	  break;
	case 2:		/* ESUM */
	  if (hazardcheckP (me, 0) || hazardcheck (me, 0, get_fs (iL), 0xf, 1))
	    {			/* ihc: ft->fs */
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
          acc_oflw = fmac_acc_oflw;     /* save */
          fmac_acc_oflw = 0;
	  x = VF[get_fs (iL)][0];
	  y = VF[get_fs (iL)][1];
	  z = VF[get_fs (iL)][2];
	  w = VF[get_fs (iL)][3];

	  acc = FMul (x, one);	/* MULA    ACC, x, 1 */
	  acc = FMAdd (acc, fmac_acc_oflw, y, one);	/* MADDA   ACC, y, 1 */
	  acc = FMAdd (acc, fmac_acc_oflw, z, one);	/* MADDA   ACC, z, 1 */
	  spipe.vn = FMAdd (acc, fmac_acc_oflw, w, one);	/* MADD    P, w, 1 */
	  spipe.no = 10;
	  spipe.code = I_ESUM;
	  spipe.code_addr = opc;
          fmac_acc_oflw = acc_oflw;     /* restore */
	  break;
	default:
	  fprintf (stderr, "Undefined opcode(LowerOp:%08lx)\n", iL);
	  exit (1);
	  break;
	}
      break;
    case 30:			/* ESQRT, ERSQRT, ERCPR, WAITP */
      switch (get_bc (iL))
	{
	case 0:		/* ESQRT */
	  if (hazardcheckP (me, 0) ||
	      hazardcheckFTF (me, get_fs (iL), 0, get_fsf (iL), 0, 2))
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
          acc_oflw = fmac_acc_oflw;     /* save */
	  x = VF[get_fs (iL)][get_fsf (iL)];	/* x */
	  spipe.vn = FSqrt (x);	/* SQRT P, 1, x */
	  spipe.no = 10;
	  spipe.code = I_ESQRT;
	  spipe.code_addr = opc;
          fmac_acc_oflw = acc_oflw;     /* restore */
	  break;
	case 1:		/* ERSQRT */
	  if (hazardcheckP (me, 0) ||
	      hazardcheckFTF (me, get_fs (iL), 0, get_fsf (iL), 0, 2))
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
          acc_oflw = fmac_acc_oflw;     /* save */
	  x = VF[get_fs (iL)][get_fsf (iL)];	/* x */
	  spipe.vn = FRSqrt (one, x);	/* ERSQRT P, 1, x */
	  spipe.no = 16;
	  spipe.code = I_ERSQRT;
	  spipe.code_addr = opc;
          fmac_acc_oflw = acc_oflw;     /* restore */
	  break;

	case 2:		/* ERCPR */
	  if (hazardcheckP (me, 0) ||
	      hazardcheckFTF (me, get_fs (iL), 0, get_fsf (iL), 0, 2))
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
          acc_oflw = fmac_acc_oflw;     /* save */
	  x = VF[get_fs (iL)][get_fsf (iL)];	/* x */
	  spipe.vn = FDiv (one, x);	/* DIV P, 1, x */
	  spipe.no = 10;
	  spipe.code = I_ERCPR;
	  spipe.code_addr = opc;
          fmac_acc_oflw = acc_oflw;     /* restore */
	  break;
	case 3:		/* WAITP */
	  if (hazardcheckP (me, 1))
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
	  break;
	}
      break;
    case 31:			/* ESIN, EATAN, EEXP, --- */
      switch (get_bc (iL))
	{
	case 0:		/* SIN */

	  if (hazardcheckP (me, 0) ||
	      hazardcheckFTF (me, get_fs (iL), 0, get_fsf (iL), 0, 2))
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
          acc_oflw = fmac_acc_oflw;     /* save */
          fmac_acc_oflw = 0;
	  x = VF[get_fs (iL)][get_fsf (iL)];	/* x */
	  tmp3 = FMul (x, x);	/* MUL tmp3, x, x */
	  acc = FMul (x, S1);	/* MULA ACC, x, S1 */
	  tmp1 = FMul (tmp3, x);	/* MUL tmp1, tmp3, x */
	  tmp2 = FMul (tmp1, tmp3);	/* MUL tmp2, tmp1, tmp3 */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp1, S2);	/* MADDA ACC, tmp1, S2 */
	  tmp1 = FMul (tmp2, tmp3);	/* MUL tmp1, tmp2, tmp3 */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp2, S3);	/* MADDA ACC, tmp2, S3 */
	  tmp2 = FMul (tmp1, tmp3);	/* MUL tmp2, tmp1, tmp3 */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp1, S4);	/* MADDA ACC, tmp1, S4 */
	  spipe.vn = FMAdd (acc, fmac_acc_oflw, tmp2, S5);	/* MADD P, tmp2, S5 */
	  spipe.no = 27;
	  spipe.code = I_ESIN;
	  spipe.code_addr = opc;
          fmac_acc_oflw = acc_oflw;     /* restore */
	  break;
	case 1:		/* EATAN */
	  if (hazardcheckP (me, 0) ||
	      hazardcheckFTF (me, get_fs (iL), 0, get_fsf (iL), 0, 2))
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
          acc_oflw = fmac_acc_oflw;     /* save */
          fmac_acc_oflw = 0;
	  x = VF[get_fs (iL)][get_fsf (iL)];
	  tmp1 = FAdd (x, one);	/* ADD     tmp1, x, 1           */
	  tmp2 = FSub (x, one);	/* SUB     tmp2, x, 1           */
	  x = FDiv (tmp2, tmp1);	/* DIV     x, tmp2, tmp1        */
	  tmp3 = FMul (x, x);	/* MUL     tmp3, x, x           */
	  acc = FMul (T1, x);	/* MULA    ACC, T1, x           */
	  tmp1 = FMul (tmp3, x);	/* MUL     tmp1, tmp3, x        */
	  tmp2 = FMul (tmp1, tmp3);	/* MUL     tmp2, tmp1, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp1, T2);	/* MADDA   ACC, tmp1, T2        */
	  tmp1 = FMul (tmp2, tmp3);	/* MUL     tmp1, tmp2, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp2, T3);	/* MADDA   ACC, tmp2, T3        */
	  tmp2 = FMul (tmp1, tmp3);	/* MUL     tmp2, tmp1, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp1, T4);	/* MADDA   ACC, tmp1, T4        */
	  tmp1 = FMul (tmp2, tmp3);	/* MUL     tmp1, tmp2, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp2, T5);	/* MADDA   ACC, tmp2, T5        */
	  tmp2 = FMul (tmp1, tmp3);	/* MUL     tmp2, tmp1, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp1, T6);	/* MADDA   ACC, tmp1, T6        */
	  tmp1 = FMul (tmp2, tmp3);	/* MUL     tmp1, tmp2, tmp3     */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp2, T7);	/* MADDA   ACC, tmp2, T7        */
	  acc = FMAdd (acc, fmac_acc_oflw, one, PI_4);		/* MADDA   ACC, 1, PI/4         */
	  spipe.vn = FMAdd (acc, fmac_acc_oflw, tmp1, T8);	/* MADD  P, tmp1, T8          */
	  spipe.no = 52;
	  spipe.code = I_EATAN;
	  spipe.code_addr = opc;
          fmac_acc_oflw = acc_oflw;     /* restore */
	  break;
	case 2:		/* EEXP */
	  if (hazardcheckP (me, 0) ||
	      hazardcheckFTF (me, get_fs (iL), 0, get_fsf (iL), 0, 2))
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
          acc_oflw = fmac_acc_oflw;     /* save */
          fmac_acc_oflw = 0;
	  x = VF[get_fs (iL)][get_fsf (iL)];
	  tmp1 = FMul (x, x);	/* MUL     tmp1, x, x */
	  acc = FMul (x, E1);	/* MULA    ACC, x, E1 */
	  tmp2 = FMul (tmp1, x);	/* MUL     tmp2, tmp1, x */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp1, E2);	/* MADDA   ACC, tmp1, E2 */
	  tmp1 = FMul (tmp2, x);	/* MUL     tmp1, tmp2, x */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp2, E3);	/* MADDA   ACC, tmp2, E3 */
	  tmp2 = FMul (tmp1, x);	/* MUL     tmp2, tmp1, x */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp1, E4);	/* MADDA   ACC, tmp1, E4 */
	  tmp1 = FMul (tmp2, x);	/* MUL     tmp1, tmp2, x */
	  acc = FMAdd (acc, fmac_acc_oflw, tmp2, E5);	/* MADDA   ACC, tmp2, E5 */
	  acc = FMAdd (acc, fmac_acc_oflw, one, one);	/* MADDA   ACC, 1, 1 */
	  fval = FMAdd (acc, fmac_acc_oflw, tmp1, E6);		/* MADD    P, tmp1, E6 */
	  fval = FMul (fval, fval);	/* MUL     P, P, P */
	  fval = FMul (fval, fval);	/* MUL     P, P, P */
	  spipe.vn = FDiv (one, fval);	/* DIV     P, 1, P */
	  spipe.no = 42;
	  spipe.code = I_EEXP;
	  spipe.code_addr = opc;
          fmac_acc_oflw = acc_oflw;     /* restore */
	  break;
	default:
	  fprintf (stderr, "Undefined opcode(LowerOp:%08lx)\n", iL);
	  exit (1);
	  break;
	}
      break;
    default:
      fprintf (stderr, "Undefined opcode(LowerOp:%08lx)\n", iL);
      exit (1);
    }
  return 0;
}

static int
Upper_special (vu_device * me, u_long iH, u_long iLword)
{
/*
   [name]               Upper_special
   [desc.]              Micro Special OPCODE (Upper instruction) execute
   [args]               iH: Upper instruction(32bits)
   iL: lower instruction(32bits)
   [return]     0: executed
   1: stall ( hazard occured)
 */

  floatie w;
  floatie iL;
  /* double dval, dval2; -=UNUSED=- */
  long ix, iy, iz, iw;
  int dest, ft, fs, bc;
  floatie zero;

  iL.i = iLword;
  zero.f = 0.0;

  if (indebug ("inst_trace2"))
    VU_CHECK_OPEN_DEBUG (me);

  switch ((iH >> 6) & 0x1f)
    {				/* bit42-38 */
    case 0:			/* ADDAx, ADDAy, ADDAz, ADDAw */
      BURST_UPPER2 (iH, dest, ft, fs, bc);

      if (hazardcheckBC (me, fs, ft, dest, bc, 3))
	return 1;
      pipe[0][0].flag = P_STATUS_REG;
      pipe[0][0].status = 0;
      pipe[0][0].code = I_ADDA;
      pipe[0][0].code_addr = opc;
      apipe.flag = 1;
      apipe.mask = dest;
      if (dest & DEST_X)
	{
	  apipe.acc[0] = FAdd (VF[fs][0], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[0] = fmac_O;
	  statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
	}
      if (dest & DEST_Y)
	{
	  apipe.acc[1] = FAdd (VF[fs][1], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[1] = fmac_O;
	  statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
	}
      if (dest & DEST_Z)
	{
	  apipe.acc[2] = FAdd (VF[fs][2], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[2] = fmac_O;
	  statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
	}
      if (dest & DEST_W)
	{
	  apipe.acc[3] = FAdd (VF[fs][3], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[3] = fmac_O;
	  statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
	}
      break;
    case 1:			/* SUBAx, SUBAy, SUBAz, SUBAw */
      BURST_UPPER2 (iH, dest, ft, fs, bc);
      if (hazardcheckBC (me, fs, ft, dest, bc, 3))
	return 1;
      pipe[0][0].flag = P_STATUS_REG;
      pipe[0][0].status = 0;
      pipe[0][0].code = I_SUBA;
      pipe[0][0].code_addr = opc;
      apipe.flag = 1;
      apipe.mask = dest;
      if (dest & DEST_X)
	{
	  apipe.acc[0] = FSub (VF[fs][0], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[0] = fmac_O;
	  statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
	}
      if (dest & DEST_Y)
	{
	  apipe.acc[1] = FSub (VF[fs][1], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[1] = fmac_O;
	  statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
	}
      if (dest & DEST_Z)
	{
	  apipe.acc[2] = FSub (VF[fs][2], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[2] = fmac_O;
	  statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
	}
      if (dest & DEST_W)
	{
	  apipe.acc[3] = FSub (VF[fs][3], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[3] = fmac_O;
	  statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
	}
      break;
    case 2:			/* MADDAx, MADDAy, MADDAz, MADDAw */
      BURST_UPPER2 (iH, dest, ft, fs, bc);
      if (hazardcheckBC (me, fs, ft, dest, bc, 3))
	return 1;
      if (indebug ("inst_trace2"))
	{
#if 0
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
	  "\tMADDAv ACC = <%08x, %08x, %08x, %08x> = <%f, %f, %f, %f>\n",
		   f_2_i (FMAdd (ACC[0], ACC_OFLW[0], VF[fs][0], VF[ft][bc])),
		   f_2_i (FMAdd (ACC[1], ACC_OFLW[1], VF[fs][1], VF[ft][bc])),
		   f_2_i (FMAdd (ACC[2], ACC_OFLW[2], VF[fs][2], VF[ft][bc])),
		   f_2_i (FMAdd (ACC[3], ACC_OFLW[3], VF[fs][3], VF[ft][bc])),
		   FMAdd (ACC[0], ACC_OFLW[0], VF[fs][0], VF[ft][bc]),
		   FMAdd (ACC[1], ACC_OFLW[1], VF[fs][1], VF[ft][bc]),
		   FMAdd (ACC[2], ACC_OFLW[2], VF[fs][2], VF[ft][bc]),
		   FMAdd (ACC[3], ACC_OFLW[3], VF[fs][3], VF[ft][bc])
	    );
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		   "\t     <%f,%f,%f,%f> + <%f,%f,%f,%f>*<%f>\n",
		   ACC[0], ACC[1], ACC[2], ACC[3],
		   VF[fs][0],
		   VF[fs][1],
		   VF[fs][2],
		   VF[fs][3],
		   VF[ft][bc]);
#endif
	}
      pipe[0][0].flag = P_STATUS_REG;
      pipe[0][0].status = 0;
      pipe[0][0].code = I_MADDA;
      pipe[0][0].code_addr = opc;
      apipe.flag = 1;
      apipe.mask = dest;
      if (dest & DEST_X)
	{
	  apipe.acc[0] = FMAdd (ACC[0], ACC_OFLW[0], VF[fs][0], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[0] = fmac_O;
	  statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
	}
      if (dest & DEST_Y)
	{
	  apipe.acc[1] = FMAdd (ACC[1], ACC_OFLW[1], VF[fs][1], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[1] = fmac_O;
	  statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
	}
      if (dest & DEST_Z)
	{
	  apipe.acc[2] = FMAdd (ACC[2], ACC_OFLW[2], VF[fs][2], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[2] = fmac_O;
	  statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
	}
      if (dest & DEST_W)
	{
	  apipe.acc[3] = FMAdd (ACC[3], ACC_OFLW[3], VF[fs][3], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[3] = fmac_O;
	  statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
	}
      break;
    case 3:			/* MSUBAx, MSUBAy, MSUBAz, MSUBAw */
      BURST_UPPER2 (iH, dest, ft, fs, bc);
      if (hazardcheckBC (me, fs, ft, dest, bc, 3))
	return 1;
      pipe[0][0].flag = P_STATUS_REG;
      pipe[0][0].status = 0;
      pipe[0][0].code = I_MSUBA;
      pipe[0][0].code_addr = opc;
      apipe.flag = 1;
      apipe.mask = dest;
      if (dest & DEST_X)
	{
	  apipe.acc[0] = FMSub (ACC[0], ACC_OFLW[0], VF[fs][0], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[0] = fmac_O;
	  statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
	}
      if (dest & DEST_Y)
	{
	  apipe.acc[1] = FMSub (ACC[1], ACC_OFLW[1], VF[fs][1], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[1] = fmac_O;
	  statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
	}
      if (dest & DEST_Z)
	{
	  apipe.acc[2] = FMSub (ACC[2], ACC_OFLW[2], VF[fs][2], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[2] = fmac_O;
	  statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
	}
      if (dest & DEST_W)
	{
	  apipe.acc[3] = FMSub (ACC[3], ACC_OFLW[3], VF[fs][3], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[3] = fmac_O;
	  statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
	}
      break;
    case 4:			/* ITOF0, ITOF4, ITOF12, ITOF15 */
      BURST_UPPER2 (iH, dest, ft, fs, bc);
      if (hazardcheck (me, fs, 0, dest, 2))
	return 1;
      pipe[0][0].no = ft;
      pipe[0][0].mask = dest;
      pipe[0][0].flag = P_VF_REG_NO_STATUS;
      switch (bc)
	{
	case 0:		/* ITOF0 */
	  pipe[0][0].code = I_ITOF0;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    pipe[0][0].vf[0] = FIntConv(F_ITOF0, VF[fs][0]);
	  if (dest & DEST_Y)
	    pipe[0][0].vf[1] = FIntConv(F_ITOF0, VF[fs][1]);
	  if (dest & DEST_Z)
	    pipe[0][0].vf[2] = FIntConv(F_ITOF0, VF[fs][2]);
	  if (dest & DEST_W)
	    pipe[0][0].vf[3] = FIntConv(F_ITOF0, VF[fs][3]);
	  break;
	case 1:		/* ITOF4 */
	  pipe[0][0].code = I_ITOF4;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    pipe[0][0].vf[0] = FIntConv(F_ITOF4, VF[fs][0]);
	  if (dest & DEST_Y)
	    pipe[0][0].vf[1] = FIntConv(F_ITOF4, VF[fs][1]);
	  if (dest & DEST_Z)
	    pipe[0][0].vf[2] = FIntConv(F_ITOF4, VF[fs][2]);
	  if (dest & DEST_W)
	    pipe[0][0].vf[3] = FIntConv(F_ITOF4, VF[fs][3]);
	  break;
	case 2:		/* ITOF12 */
	  pipe[0][0].code = I_ITOF12;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    pipe[0][0].vf[0] = FIntConv(F_ITOF12, VF[fs][0]);
	  if (dest & DEST_Y)
	    pipe[0][0].vf[1] = FIntConv(F_ITOF12, VF[fs][1]);
	  if (dest & DEST_Z)
	    pipe[0][0].vf[2] = FIntConv(F_ITOF12, VF[fs][2]);
	  if (dest & DEST_W)
	    pipe[0][0].vf[3] = FIntConv(F_ITOF12, VF[fs][3]);
	  break;
	case 3:		/* ITOF15 */
	  pipe[0][0].code = I_ITOF15;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    pipe[0][0].vf[0] = FIntConv(F_ITOF15, VF[fs][0]);
	  if (dest & DEST_Y)
	    pipe[0][0].vf[1] = FIntConv(F_ITOF15, VF[fs][1]);
	  if (dest & DEST_Z)
	    pipe[0][0].vf[2] = FIntConv(F_ITOF15, VF[fs][2]);
	  if (dest & DEST_W)
	    pipe[0][0].vf[3] = FIntConv(F_ITOF15, VF[fs][3]);
	  break;
	}
      break;
    case 5:			/* FTOI0, FTOI4, FTOI12, FTOI15 */
      BURST_UPPER2 (iH, dest, ft, fs, bc);
      if (hazardcheck (me, fs, 0, dest, 2))
	return 1;
      pipe[0][0].no = ft;
      pipe[0][0].mask = dest;
      pipe[0][0].flag = P_VF_REG_NO_STATUS;
      switch (bc)
	{
	case 0:		/* FTOI0 */
	  if (indebug ("inst_trace2"))
	    {
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\tFTOI0 VF%02d = <%08x, %08x, %08x, %08x>\n",
		       (int) ft,
		       VF[fs][0].i,
		       VF[fs][1].i,
		       VF[fs][2].i,
		       VF[fs][3].i);
	    }
	  pipe[0][0].code = I_FTOI0;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    pipe[0][0].vf[0] = FIntConv(F_FTOI0, VF[fs][0]);
	  if (dest & DEST_Y)
	    pipe[0][0].vf[1] = FIntConv(F_FTOI0, VF[fs][1]);
	  if (dest & DEST_Z)
	    pipe[0][0].vf[2] = FIntConv(F_FTOI0, VF[fs][2]);
	  if (dest & DEST_W)
	    pipe[0][0].vf[3] = FIntConv(F_FTOI0, VF[fs][3]);
	  break;
	case 1:		/* FTOI4 */
	  if (indebug ("inst_trace2"))
	    {
#if 0
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\tFTOI4 VF%02d = <%08x, %08x, %08x, %08x>\n",
		       (int) ft,
		       (int) (FMul (VF[fs][0], 16)),
		       (int) (FMul (VF[fs][1], 16)),
		       (int) (FMul (VF[fs][2], 16)),
		       (int) (FMul (VF[fs][3], 16)));
#endif
	    }
	  pipe[0][0].code = I_FTOI4;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    pipe[0][0].vf[0] = FIntConv(F_FTOI4, VF[fs][0]);
	  if (dest & DEST_Y)
	    pipe[0][0].vf[1] = FIntConv(F_FTOI4, VF[fs][1]);
	  if (dest & DEST_Z)
	    pipe[0][0].vf[2] = FIntConv(F_FTOI4, VF[fs][2]);
	  if (dest & DEST_W)
	    pipe[0][0].vf[3] = FIntConv(F_FTOI4, VF[fs][3]);
	  break;
	case 2:		/* FTOI12 */
	  if (indebug ("inst_trace2"))
	    {
#if 0
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\tFTOI12 VF%02d = <%08x, %08x, %08x, %08x>\n",
		       (int) ft,
		       (int) (FMul (VF[fs][0], 4096)),
		       (int) (FMul (VF[fs][1], 4096)),
		       (int) (FMul (VF[fs][2], 4096)),
		       (int) (FMul (VF[fs][3], 4096)));
#endif
	    }
	  pipe[0][0].code = I_FTOI12;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    pipe[0][0].vf[0] = FIntConv(F_FTOI12, VF[fs][0]);
	  if (dest & DEST_Y)
	    pipe[0][0].vf[1] = FIntConv(F_FTOI12, VF[fs][1]);
	  if (dest & DEST_Z)
	    pipe[0][0].vf[2] = FIntConv(F_FTOI12, VF[fs][2]);
	  if (dest & DEST_W)
	    pipe[0][0].vf[3] = FIntConv(F_FTOI12, VF[fs][3]);
	  break;
	case 3:		/* FTOI15 */
	  if (indebug ("inst_trace2"))
	    {
#if 0
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\tFTOI15 VF%02d = <%08x, %08x, %08x, %08x>\n",
		       (int) ft,
		       (int) (FMul (VF[fs][0], 32768)),
		       (int) (FMul (VF[fs][1], 32768)),
		       (int) (FMul (VF[fs][2], 32768)),
		       (int) (FMul (VF[fs][3], 32768)));
#endif
	    }
	  pipe[0][0].code = I_FTOI15;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    pipe[0][0].vf[0] = FIntConv(F_FTOI15, VF[fs][0]);
	  if (dest & DEST_Y)
	    pipe[0][0].vf[1] = FIntConv(F_FTOI15, VF[fs][1]);
	  if (dest & DEST_Z)
	    pipe[0][0].vf[2] = FIntConv(F_FTOI15, VF[fs][2]);
	  if (dest & DEST_W)
	    pipe[0][0].vf[3] = FIntConv(F_FTOI15, VF[fs][3]);
	  break;
	}
      break;
    case 6:			/* MULAx, MULAy, MULAz, MULAw */
      BURST_UPPER2 (iH, dest, ft, fs, bc);
      if (hazardcheckBC (me, fs, ft, dest, bc, 3))
	return 1;
      if (indebug ("inst_trace2"))
	{
#if 0
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
	   "\tMULAv ACC = <%08x, %08x, %08x, %08x> = <%f, %f, %f, %f>\n",
		   f_2_i (FMul (VF[fs][0], VF[ft][bc])),
		   f_2_i (FMul (VF[fs][1], VF[ft][bc])),
		   f_2_i (FMul (VF[fs][2], VF[ft][bc])),
		   f_2_i (FMul (VF[fs][3], VF[ft][bc])),
		   FMul (VF[fs][0], VF[ft][bc]),
		   FMul (VF[fs][1], VF[ft][bc]),
		   FMul (VF[fs][2], VF[ft][bc]),
		   FMul (VF[fs][3], VF[ft][bc])
	    );
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		   "\t     <%f,%f,%f,%f>*<%f>\n",
		   VF[fs][0],
		   VF[fs][1],
		   VF[fs][2],
		   VF[fs][3],
		   VF[ft][bc]);
#endif
	}
      pipe[0][0].flag = P_STATUS_REG;
      pipe[0][0].status = 0;
      pipe[0][0].code = I_MULA;
      pipe[0][0].code_addr = opc;
      apipe.flag = 1;
      apipe.mask = dest;
      if (dest & DEST_X)
	{
	  apipe.acc[0] = FMul (VF[fs][0], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[0] = fmac_O;
	  statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
	}
      if (dest & DEST_Y)
	{
	  apipe.acc[1] = FMul (VF[fs][1], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[1] = fmac_O;
	  statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
	}
      if (dest & DEST_Z)
	{
	  apipe.acc[2] = FMul (VF[fs][2], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[2] = fmac_O;
	  statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
	}
      if (dest & DEST_W)
	{
	  apipe.acc[3] = FMul (VF[fs][3], VF[ft][bc]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[3] = fmac_O;
	  statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
	}
      break;
    case 7:			/* MULAq, ABS, MULAi, CLIP */
      BURST_UPPER2 (iH, dest, ft, fs, bc);
      switch (bc)
	{
	case 0:		/* MULAq */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  if (indebug ("inst_trace2"))
	    {
#if 0
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
	               "\tMULAv ACC = <%08x, %08x, %08x, %08x> = <%f, %f, %f, %f>\n",
		       f_2_i (FMul (VF[fs][0], Q)),
		       f_2_i (FMul (VF[fs][1], Q)),
		       f_2_i (FMul (VF[fs][2], Q)),
		       f_2_i (FMul (VF[fs][3], Q)),
		       FMul (VF[fs][0], Q),
		       FMul (VF[fs][1], Q),
		       FMul (VF[fs][2], Q),
		       FMul (VF[fs][3], Q)
		);
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\t <%f,%f,%f,%f>*<%f>\n",
		       VF[fs][0],
		       VF[fs][1],
		       VF[fs][2],
		       VF[fs][3],
		       Q);
#endif
	    }
	  pipe[0][0].flag = P_STATUS_REG;
          pipe[0][0].status = 0;
	  pipe[0][0].code = I_MULAq;
	  pipe[0][0].code_addr = opc;
	  apipe.flag = 1;
	  apipe.mask = dest;
	  if (dest & DEST_X)
	    {
	      apipe.acc[0] = FMul (VF[fs][0], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[0] = fmac_O;
	      statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      apipe.acc[1] = FMul (VF[fs][1], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[1] = fmac_O;
	      statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      apipe.acc[2] = FMul (VF[fs][2], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[2] = fmac_O;
	      statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      apipe.acc[3] = FMul (VF[fs][3], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[3] = fmac_O;
	      statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 1:		/* ABS */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  if (indebug ("inst_trace2"))
	    {
#if 0
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
	               "\tVF%02d.%x = <%08x, %08x, %08x, %08x> = <%f, %f, %f, %f>\n",
		       ft, dest,
		       f_2_i (FAbs (VF[fs][0])),
		       f_2_i (FAbs (VF[fs][1])),
		       f_2_i (FAbs (VF[fs][2])),
		       f_2_i (FAbs (VF[fs][3])),
		       FAbs (VF[fs][0]),
		       FAbs (VF[fs][1]),
		       FAbs (VF[fs][2]),
		       FAbs (VF[fs][3])
		);
#endif
	    }
	  pipe[0][0].no = ft;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG_NO_STATUS;
	  pipe[0][0].code = I_ABS;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    {
	      pipe[0][0].vf[0] = FAbs (VF[fs][0]);
	    }
	  if (dest & DEST_Y)
	    {
	      pipe[0][0].vf[1] = FAbs (VF[fs][1]);
	    }
	  if (dest & DEST_Z)
	    {
	      pipe[0][0].vf[2] = FAbs (VF[fs][2]);
	    }
	  if (dest & DEST_W)
	    {
	      pipe[0][0].vf[3] = FAbs (VF[fs][3]);
	    }
	  break;
	case 2:		/* MULAi */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  pipe[0][0].flag = P_STATUS_REG;
          pipe[0][0].status = 0;
	  pipe[0][0].code = I_MULAi;
	  pipe[0][0].code_addr = opc;
	  apipe.flag = 1;
	  apipe.mask = dest;
	  if (iH & 0x80000000)
	    {
	      if (dest & DEST_X)
		{
		  apipe.acc[0] = FMul (VF[fs][0], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[0] = fmac_O;
		  statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
		}
	      if (dest & DEST_Y)
		{
		  apipe.acc[1] = FMul (VF[fs][1], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[1] = fmac_O;
		  statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
		}
	      if (dest & DEST_Z)
		{
		  apipe.acc[2] = FMul (VF[fs][2], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[2] = fmac_O;
		  statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
		}
	      if (dest & DEST_W)
		{
		  apipe.acc[3] = FMul (VF[fs][3], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[3] = fmac_O;
		  statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
		}
	    }
	  else
	    {
	      if (dest & DEST_X)
		{
		  apipe.acc[0] = FMul (VF[fs][0], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[0] = fmac_O;
		  statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
		}
	      if (dest & DEST_Y)
		{
		  apipe.acc[1] = FMul (VF[fs][1], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[1] = fmac_O;
		  statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
		}
	      if (dest & DEST_Z)
		{
		  apipe.acc[2] = FMul (VF[fs][2], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[2] = fmac_O;
		  statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
		}
	      if (dest & DEST_W)
		{
		  apipe.acc[3] = FMul (VF[fs][3], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[3] = fmac_O;
		  statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
		}
	    }
	  break;
	case 3:		/* CLIP */
	  if (hazardcheck (me, fs, 0, 0xf, 2))
	    return 1;
          pipe[0][0].status = 0;
	  pipe[0][0].flag = P_CLIP_REG;
	  pipe[0][0].status = 0;
	  pipe[0][0].code = I_CLIP;
	  pipe[0][0].code_addr = opc;
#if 0
	  if (FCmp(VF[ft][3], '<', zero))
	    w = FSub (0, VF[ft][3]);
	  else
	    w = VF[ft][3];
#endif
	  w = FAbs (VF[ft][3]);
	  if (FCmp(VF[fs][0], '>', w))
	    pipe[0][0].status |= 0x1;
	  if (FCmp(VF[fs][0], '<', FSub (zero, w)))
	    pipe[0][0].status |= 0x2;
	  if (FCmp(VF[fs][1], '>', w))
	    pipe[0][0].status |= 0x4;
	  if (FCmp(VF[fs][1], '<', FSub (zero, w)))
	    pipe[0][0].status |= 0x8;
	  if (FCmp(VF[fs][2], '>', w))
	    pipe[0][0].status |= 0x10;
	  if (FCmp(VF[fs][2], '<', FSub (zero, w)))
	    pipe[0][0].status |= 0x20;
	  break;
	}
      break;
    case 8:			/* ADDAq, MADDAq, ADDAi, MADDAi */
      BURST_UPPER2 (iH, dest, ft, fs, bc);
      switch (bc)
	{
	case 0:		/* ADDAq */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  pipe[0][0].flag = P_STATUS_REG;
          pipe[0][0].status = 0;
	  pipe[0][0].code = I_ADDAq;
	  pipe[0][0].code_addr = opc;
	  apipe.flag = 1;
	  apipe.mask = dest;
	  if (dest & DEST_X)
	    {
	      apipe.acc[0] = FAdd (VF[fs][0], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[0] = fmac_O;
	      statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      apipe.acc[1] = FAdd (VF[fs][1], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[1] = fmac_O;
	      statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      apipe.acc[2] = FAdd (VF[fs][2], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[2] = fmac_O;
	      statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      apipe.acc[3] = FAdd (VF[fs][3], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[3] = fmac_O;
	      statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 1:		/* MADDAq */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  pipe[0][0].flag = P_STATUS_REG;
          pipe[0][0].status = 0;
	  pipe[0][0].code = I_MADDAq;
	  pipe[0][0].code_addr = opc;
	  apipe.flag = 1;
	  apipe.mask = dest;
	  if (dest & DEST_X)
	    {
	      apipe.acc[0] = FMAdd (ACC[0], ACC_OFLW[0], VF[fs][0], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[0] = fmac_O;
	      statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      apipe.acc[1] = FMAdd (ACC[1], ACC_OFLW[1], VF[fs][1], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[1] = fmac_O;
	      statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      apipe.acc[2] = FMAdd (ACC[2], ACC_OFLW[2], VF[fs][2], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[2] = fmac_O;
	      statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      apipe.acc[3] = FMAdd (ACC[3], ACC_OFLW[3], VF[fs][3], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[3] = fmac_O;
	      statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 2:		/* ADDAi */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  pipe[0][0].flag = P_STATUS_REG;
          pipe[0][0].status = 0;
	  pipe[0][0].code = I_ADDAi;
	  pipe[0][0].code_addr = opc;
	  apipe.flag = 1;
	  apipe.mask = dest;
	  if (iH & 0x80000000)
	    {
	      if (dest & DEST_X)
		{
		  apipe.acc[0] = FAdd (VF[fs][0], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[0] = fmac_O;
		  statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
		}
	      if (dest & DEST_Y)
		{
		  apipe.acc[1] = FAdd (VF[fs][1], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[1] = fmac_O;
		  statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
		}
	      if (dest & DEST_Z)
		{
		  apipe.acc[2] = FAdd (VF[fs][2], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[2] = fmac_O;
		  statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
		}
	      if (dest & DEST_W)
		{
		  apipe.acc[3] = FAdd (VF[fs][3], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[3] = fmac_O;
		  statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
		}
	    }
	  else
	    {
	      if (dest & DEST_X)
		{
		  apipe.acc[0] = FAdd (VF[fs][0], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[0] = fmac_O;
		  statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
		}
	      if (dest & DEST_Y)
		{
		  apipe.acc[1] = FAdd (VF[fs][1], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[1] = fmac_O;
		  statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
		}
	      if (dest & DEST_Z)
		{
		  apipe.acc[2] = FAdd (VF[fs][2], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[2] = fmac_O;
		  statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
		}
	      if (dest & DEST_W)
		{
		  apipe.acc[3] = FAdd (VF[fs][3], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[3] = fmac_O;
		  statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
		}
	    }
	  break;
	case 3:		/* MADDAi */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  pipe[0][0].flag = P_STATUS_REG;
          pipe[0][0].status = 0;
	  pipe[0][0].code = I_MADDAi;
	  pipe[0][0].code_addr = opc;
	  apipe.flag = 1;
	  apipe.mask = dest;
	  if (iH & 0x80000000)
	    {
	      if (dest & DEST_X)
		{
		  apipe.acc[0] = FMAdd (ACC[0], ACC_OFLW[0], VF[fs][0], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[0] = fmac_O;
		  statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
		}
	      if (dest & DEST_Y)
		{
		  apipe.acc[1] = FMAdd (ACC[1], ACC_OFLW[1], VF[fs][1], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[1] = fmac_O;
		  statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
		}
	      if (dest & DEST_Z)
		{
		  apipe.acc[2] = FMAdd (ACC[2], ACC_OFLW[2], VF[fs][2], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[2] = fmac_O;
		  statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
		}
	      if (dest & DEST_W)
		{
		  apipe.acc[3] = FMAdd (ACC[3], ACC_OFLW[0], VF[fs][3], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[3] = fmac_O;
		  statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
		}
	    }
	  else
	    {
	      if (dest & DEST_X)
		{
		  apipe.acc[0] = FMAdd (ACC[0], ACC_OFLW[0], VF[fs][0], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[0] = fmac_O;
		  statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
		}
	      if (dest & DEST_Y)
		{
		  apipe.acc[1] = FMAdd (ACC[1], ACC_OFLW[1], VF[fs][1], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[1] = fmac_O;
		  statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
		}
	      if (dest & DEST_Z)
		{
		  apipe.acc[2] = FMAdd (ACC[2], ACC_OFLW[2], VF[fs][2], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[2] = fmac_O;
		  statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
		}
	      if (dest & DEST_W)
		{
		  apipe.acc[3] = FMAdd (ACC[3], ACC_OFLW[3], VF[fs][3], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[3] = fmac_O;
		  statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
		}
	    }
	  break;
	}
      break;
    case 9:			/* SUBAq, MSUBAq, SUBAi, MSUBAi */
      BURST_UPPER2 (iH, dest, ft, fs, bc);
      switch (bc)
	{
	case 0:		/* SUBAq */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  pipe[0][0].flag = P_STATUS_REG;
          pipe[0][0].status = 0;
	  pipe[0][0].code = I_SUBAq;
	  pipe[0][0].code_addr = opc;
	  apipe.flag = 1;
	  apipe.mask = dest;
	  if (dest & DEST_X)
	    {
	      apipe.acc[0] = FSub (VF[fs][0], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[0] = fmac_O;
	      statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      apipe.acc[1] = FSub (VF[fs][1], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[1] = fmac_O;
	      statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      apipe.acc[2] = FSub (VF[fs][2], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[2] = fmac_O;
	      statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      apipe.acc[3] = FSub (VF[fs][3], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[3] = fmac_O;
	      statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 1:		/* MSUBAq */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  pipe[0][0].flag = P_STATUS_REG;
          pipe[0][0].status = 0;
	  pipe[0][0].code = I_MSUBAq;
	  pipe[0][0].code_addr = opc;
	  apipe.flag = 1;
	  apipe.mask = dest;
	  if (dest & DEST_X)
	    {
	      apipe.acc[0] = FMSub (ACC[0], ACC_OFLW[0], VF[fs][0], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[0] = fmac_O;
	      statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      apipe.acc[1] = FMSub (ACC[1], ACC_OFLW[1], VF[fs][1], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[1] = fmac_O;
	      statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      apipe.acc[2] = FMSub (ACC[2], ACC_OFLW[2], VF[fs][2], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[2] = fmac_O;
	      statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      apipe.acc[3] = FMSub (ACC[3], ACC_OFLW[3], VF[fs][3], Q);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[3] = fmac_O;
	      statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 2:		/* SUBAi */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  pipe[0][0].flag = P_STATUS_REG;
          pipe[0][0].status = 0;
	  pipe[0][0].code = I_SUBAi;
	  pipe[0][0].code_addr = opc;
	  apipe.flag = 1;
	  apipe.mask = dest;
	  if (iH & 0x80000000)
	    {
	      if (dest & DEST_X)
		{
		  apipe.acc[0] = FSub (VF[fs][0], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[0] = fmac_O;
		  statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
		}
	      if (dest & DEST_Y)
		{
		  apipe.acc[1] = FSub (VF[fs][1], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[1] = fmac_O;
		  statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
		}
	      if (dest & DEST_Z)
		{
		  apipe.acc[2] = FSub (VF[fs][2], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[2] = fmac_O;
		  statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
		}
	      if (dest & DEST_W)
		{
		  apipe.acc[3] = FSub (VF[fs][3], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[3] = fmac_O;
		  statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
		}
	    }
	  else
	    {
	      if (dest & DEST_X)
		{
		  apipe.acc[0] = FSub (VF[fs][0], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[0] = fmac_O;
		  statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
		}
	      if (dest & DEST_Y)
		{
		  apipe.acc[1] = FSub (VF[fs][1], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[1] = fmac_O;
		  statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
		}
	      if (dest & DEST_Z)
		{
		  apipe.acc[2] = FSub (VF[fs][2], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[2] = fmac_O;
		  statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
		}
	      if (dest & DEST_W)
		{
		  apipe.acc[3] = FSub (VF[fs][3], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[3] = fmac_O;
		  statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
		}
	    }
	  break;
	case 3:		/* MSUBAi */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  pipe[0][0].flag = P_STATUS_REG;
          pipe[0][0].status = 0;
	  pipe[0][0].code = I_MSUBAi;
	  pipe[0][0].code_addr = opc;
	  apipe.flag = 1;
	  apipe.mask = dest;
	  if (iH & 0x80000000)
	    {
	      if (dest & DEST_X)
		{
		  apipe.acc[0] = FMSub (ACC[0], ACC_OFLW[0], VF[fs][0], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[0] = fmac_O;
		  statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
		}
	      if (dest & DEST_Y)
		{
		  apipe.acc[1] = FMSub (ACC[1], ACC_OFLW[1], VF[fs][1], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[1] = fmac_O;
		  statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
		}
	      if (dest & DEST_Z)
		{
		  apipe.acc[2] = FMSub (ACC[2], ACC_OFLW[2], VF[fs][2], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[2] = fmac_O;
		  statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
		}
	      if (dest & DEST_W)
		{
		  apipe.acc[3] = FMSub (ACC[3], ACC_OFLW[3], VF[fs][3], iL);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[3] = fmac_O;
		  statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
		}
	    }
	  else
	    {
	      if (dest & DEST_X)
		{
		  apipe.acc[0] = FMSub (ACC[0], ACC_OFLW[0], VF[fs][0], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[0] = fmac_O;
		  statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
		}
	      if (dest & DEST_Y)
		{
		  apipe.acc[1] = FMSub (ACC[1], ACC_OFLW[1], VF[fs][1], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[1] = fmac_O;
		  statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
		}
	      if (dest & DEST_Z)
		{
		  apipe.acc[2] = FMSub (ACC[2], ACC_OFLW[2], VF[fs][2], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[2] = fmac_O;
		  statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
		}
	      if (dest & DEST_W)
		{
		  apipe.acc[3] = FMSub (ACC[3], ACC_OFLW[3], VF[fs][3], I);
                  fmac_O |= fmac_acc_oflw;
                  apipe.acc_oflw[3] = fmac_O;
		  statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
		}
	    }
	  break;
	}
      break;
    case 10:			/* ADDA, MADDA, MULA, --- */
      BURST_UPPER2 (iH, dest, ft, fs, bc);
      switch (bc)
	{
	case 0:		/* ADDA */
	  if (hazardcheck (me, fs, ft, dest, 3))
	    return 1;
	  pipe[0][0].flag = P_STATUS_REG;
          pipe[0][0].status = 0;
	  pipe[0][0].code = I_ADDA;
	  pipe[0][0].code_addr = opc;
	  apipe.flag = 1;
	  apipe.mask = dest;
	  if (dest & DEST_X)
	    {
	      apipe.acc[0] = FAdd (VF[fs][0], VF[ft][0]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[0] = fmac_O;
	      statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      apipe.acc[1] = FAdd (VF[fs][1], VF[ft][1]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[1] = fmac_O;
	      statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      apipe.acc[2] = FAdd (VF[fs][2], VF[ft][2]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[2] = fmac_O;
	      statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      apipe.acc[3] = FAdd (VF[fs][3], VF[ft][3]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[3] = fmac_O;
	      statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 1:		/* MADDA */
	  if (hazardcheck (me, fs, ft, dest, 3))
	    return 1;
	  pipe[0][0].flag = P_STATUS_REG;
          pipe[0][0].status = 0;
	  pipe[0][0].code = I_MADDA;
	  pipe[0][0].code_addr = opc;
	  apipe.flag = 1;
	  apipe.mask = dest;
	  if (dest & DEST_X)
	    {
	      apipe.acc[0] = FMAdd (ACC[0], ACC_OFLW[0], VF[fs][0], VF[ft][0]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[0] = fmac_O;
	      statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      apipe.acc[1] = FMAdd (ACC[1], ACC_OFLW[1], VF[fs][1], VF[ft][1]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[1] = fmac_O;
	      statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      apipe.acc[2] = FMAdd (ACC[2], ACC_OFLW[2], VF[fs][2], VF[ft][2]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[2] = fmac_O;
	      statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      apipe.acc[3] = FMAdd (ACC[3], ACC_OFLW[3], VF[fs][3], VF[ft][3]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[3] = fmac_O;
	      statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 2:		/* MULA */
	  if (hazardcheck (me, fs, ft, dest, 3))
	    return 1;
	  pipe[0][0].flag = P_STATUS_REG;
          pipe[0][0].status = 0;
	  pipe[0][0].code = I_MULA;
	  pipe[0][0].code_addr = opc;
	  apipe.flag = 1;
	  apipe.mask = dest;
	  if (dest & DEST_X)
	    {
	      apipe.acc[0] = FMul (VF[fs][0], VF[ft][0]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[0] = fmac_O;
	      statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      apipe.acc[1] = FMul (VF[fs][1], VF[ft][1]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[1] = fmac_O;
	      statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      apipe.acc[2] = FMul (VF[fs][2], VF[ft][2]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[2] = fmac_O;
	      statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      apipe.acc[3] = FMul (VF[fs][3], VF[ft][3]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[3] = fmac_O;
	      statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 3:
	  fprintf (stderr, "Undefined opcode(UpperOp:%08lx)\n", iH);
	  break;
	}
      break;
    case 11:			/* SUBA, MSUBA, OPMULA, NOP */
      BURST_UPPER2 (iH, dest, ft, fs, bc);
      switch (bc)
	{
	case 0:		/* SUBA */
	  if (hazardcheck (me, fs, ft, dest, 3))
	    return 1;
	  pipe[0][0].flag = P_STATUS_REG;
          pipe[0][0].status = 0;
	  pipe[0][0].code = I_SUBA;
	  pipe[0][0].code_addr = opc;
	  apipe.flag = 1;
	  apipe.mask = dest;
	  if (dest & DEST_X)
	    {
	      apipe.acc[0] = FSub (VF[fs][0], VF[ft][0]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[0] = fmac_O;
	      statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      apipe.acc[1] = FSub (VF[fs][1], VF[ft][1]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[1] = fmac_O;
	      statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      apipe.acc[2] = FSub (VF[fs][2], VF[ft][2]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[2] = fmac_O;
	      statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      apipe.acc[3] = FSub (VF[fs][3], VF[ft][3]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[3] = fmac_O;
	      statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 1:		/* MSUBA */
	  if (hazardcheck (me, fs, ft, dest, 3))
	    return 1;
	  pipe[0][0].flag = P_STATUS_REG;
          pipe[0][0].status = 0;
	  pipe[0][0].code = I_MSUBA;
	  pipe[0][0].code_addr = opc;
	  apipe.flag = 1;
	  apipe.mask = dest;
	  if (dest & DEST_X)
	    {
	      apipe.acc[0] = FMSub (ACC[0], ACC_OFLW[0], VF[fs][0], VF[ft][0]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[0] = fmac_O;
	      statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      apipe.acc[1] = FMSub (ACC[1], ACC_OFLW[1], VF[fs][1], VF[ft][1]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[1] = fmac_O;
	      statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      apipe.acc[2] = FMSub (ACC[2], ACC_OFLW[2], VF[fs][2], VF[ft][2]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[2] = fmac_O;
	      statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      apipe.acc[3] = FMSub (ACC[3], ACC_OFLW[3], VF[fs][3], VF[ft][3]);
              fmac_O |= fmac_acc_oflw;
              apipe.acc_oflw[3] = fmac_O;
	      statuscheck_fmac (apipe.acc[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 2:		/* OPMULA */
	  if (hazardcheck (me, fs, ft, 0xe, 3))
	    return 1;
	  pipe[0][0].flag = P_STATUS_REG;
          pipe[0][0].status = 0;
	  apipe.flag = 1;
	  apipe.mask = dest;
	  pipe[0][0].code = I_OPMULA;
	  pipe[0][0].code_addr = opc;
	  apipe.acc[0] = FMul (VF[fs][1], VF[ft][2]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[0] = fmac_O;
	  statuscheck_fmac (apipe.acc[0], &pipe[0][0].status, 0);
	  apipe.acc[1] = FMul (VF[fs][2], VF[ft][0]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[1] = fmac_O;
	  statuscheck_fmac (apipe.acc[1], &pipe[0][0].status, 1);
	  apipe.acc[2] = FMul (VF[fs][0], VF[ft][1]);
          fmac_O |= fmac_acc_oflw;
          apipe.acc_oflw[2] = fmac_O;
	  statuscheck_fmac (apipe.acc[2], &pipe[0][0].status, 2);
	  break;
	case 3:		/* NOP */
	  break;
	}
      break;
    default:
      fprintf (stderr, "Undefined opcode(%08lx)\n", iH);
      exit (1);
    }
  return 0;
}


static int
exec_inst (vu_device * me)
{
/*
   [name]               exec_inst
   [desc.]              execute instruction ( second stage of pipeline)
   [args]               void
   [return]     0: finished
   1: stall (hazard occured)
   2: executed
 */
  /* double dval; -=UNUSED */
  floatie fval;
  u_long iH, iL;
  floatie iLf;

  long imm12, imm11, imm15, imm5, imm24;
  int dest, ft, fs, fd, bc;

/* ihc */
  if (indebug ("inst_trace"))
    {
      static char *insn_prefix = NULL;

      if (insn_prefix == NULL)
        {
          insn_prefix = "\t";
          if (indebug ("inst_trace_columns"))
              insn_prefix = "\n\"\t";
        }

      VU_CHECK_OPEN_DEBUG (me);
      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
	       "%03lx: %08lx %08lx%s", opc, instbuf[0], instbuf[1], insn_prefix);
      opcode_analyze (stdout, instbuf, insn_prefix);
      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout, "\n");
      fflush (stdout);
    }
/* -=ihc=- */

  if (peflag == -1)
    return 0;

  if (peflag == 1)
    peflag = -1;

  iH = instbuf[0];
  iLf.i = iL = instbuf[1];
  if (iH & 0x40000000)
    eflag = 1;			/* [e] detected */
  if (iH & 0x20000000)
    lflag = 1;			/* [m] detected */
#ifndef TARGET_SKY_B
  if (iH & 0x10000000)
    {
      /* [d] detected */
      if (me->config.vu_number == 0 && (vu0_device.regs.FBRST & VPU_FBRST_DE0_MASK)
	  || me->config.vu_number == 1 && (vu0_device.regs.FBRST & VPU_FBRST_DE1_MASK))
	{
	  dflag = 1;
	  peflag = -1;		/* stop after this insn */
	}
    }
  if (iH & 0x08000000)
    {
      /* [t] detected */
      if (me->config.vu_number == 0 && (vu0_device.regs.FBRST & VPU_FBRST_TE0_MASK)
	  || me->config.vu_number == 1 && (vu0_device.regs.FBRST & VPU_FBRST_TE1_MASK))
	{
	  tflag = 1;
	  peflag = -1;		/* stop after this insn */
	}
    }
#endif /* ! TARGET_SKY_B */

  pipe[0][0].status = pipe[0][1].status = 0;
  if (iH & 0x80000000)
    {
      Ipipe.flag = 1;
      Ipipe.val = iLf;
    }
  /* UpperOp execute */

  switch ((iH >> 2) & 0xf)
    {				/* bit37-34 */
    case 0:			/* ADDx,y,z,w */

      BURST_UPPER0 (iH, dest, ft, fs, fd, bc);

      if (hazardcheckBC (me, fs, ft, dest, bc, 3))
	return 1;
      pipe[0][0].no = fd;
      pipe[0][0].mask = dest;
      pipe[0][0].flag = P_VF_REG;
      pipe[0][0].code = I_ADD;
      pipe[0][0].code_addr = opc;
      if (dest & DEST_X)
	{
	  pipe[0][0].vf[0] = FAdd (VF[fs][0], VF[ft][bc]);
	  statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	}
      if (dest & DEST_Y)
	{
	  pipe[0][0].vf[1] = FAdd (VF[fs][1], VF[ft][bc]);
	  statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	}
      if (dest & DEST_Z)
	{
	  pipe[0][0].vf[2] = FAdd (VF[fs][2], VF[ft][bc]);
	  statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	}
      if (dest & DEST_W)
	{
	  pipe[0][0].vf[3] = FAdd (VF[fs][3], VF[ft][bc]);
	  statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	}
      break;
    case 1:			/* SUBx,y,z,w */
      BURST_UPPER0 (iH, dest, ft, fs, fd, bc);

      if (hazardcheckBC (me, fs, ft, dest, bc, 3))
	return 1;
      pipe[0][0].no = fd;
      pipe[0][0].mask = dest;
      pipe[0][0].flag = P_VF_REG;
      pipe[0][0].code = I_SUB;
      pipe[0][0].code_addr = opc;
      if (dest & DEST_X)
	{
	  pipe[0][0].vf[0] = FSub (VF[fs][0], VF[ft][bc]);
	  statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	}
      if (dest & DEST_Y)
	{
	  pipe[0][0].vf[1] = FSub (VF[fs][1], VF[ft][bc]);
	  statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	}
      if (dest & DEST_Z)
	{
	  pipe[0][0].vf[2] = FSub (VF[fs][2], VF[ft][bc]);
	  statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	}
      if (dest & DEST_W)
	{
	  pipe[0][0].vf[3] = FSub (VF[fs][3], VF[ft][bc]);
	  statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	}
      break;
    case 2:			/* MADDx,y,z,w */
      BURST_UPPER0 (iH, dest, ft, fs, fd, bc);

      if (hazardcheckBC (me, fs, ft, dest, bc, 3))
	return 1;
      if (indebug ("inst_trace2"))
	{
#if 0
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		   "\tMADDv VF%02d = <%08x, %08x, %08x, %08x> = <%f, %f, %f, %f>\n",
		   fd,
		   f_2_i (FMAdd (ACC[0], ACC_OFLW[0], VF[fs][0], VF[ft][bc])),
		   f_2_i (FMAdd (ACC[1], ACC_OFLW[1], VF[fs][1], VF[ft][bc])),
		   f_2_i (FMAdd (ACC[2], ACC_OFLW[2], VF[fs][2], VF[ft][bc])),
		   f_2_i (FMAdd (ACC[3], ACC_OFLW[3], VF[fs][3], VF[ft][bc])),
		   FMAdd (ACC[0], ACC_OFLW[0], VF[fs][0], VF[ft][bc]),
		   FMAdd (ACC[1], ACC_OFLW[1], VF[fs][1], VF[ft][bc]),
		   FMAdd (ACC[2], ACC_OFLW[2], VF[fs][2], VF[ft][bc]),
		   FMAdd (ACC[3], ACC_OFLW[3], VF[fs][3], VF[ft][bc]));
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		   "\t        <%f,%f,%f,%f>+<%f,%f,%f,%f>*<%f>\n",
		   ACC[0], ACC[1], ACC[2], ACC[3],
		   VF[fs][0], VF[fs][1], VF[fs][2], VF[fs][3],
		   VF[ft][bc]);
#endif
	}
      pipe[0][0].no = fd;
      pipe[0][0].mask = dest;
      pipe[0][0].flag = P_VF_REG;
      pipe[0][0].code = I_MADD;
      pipe[0][0].code_addr = opc;
      if (dest & DEST_X)
	{
	  pipe[0][0].vf[0] = FMAdd (ACC[0], ACC_OFLW[0], VF[fs][0], VF[ft][bc]);
          apipe.acc_oflw[0] = fmac_acc_oflw;
	  statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	}
      if (dest & DEST_Y)
	{
	  pipe[0][0].vf[1] = FMAdd (ACC[1], ACC_OFLW[1], VF[fs][1], VF[ft][bc]);
          apipe.acc_oflw[1] = fmac_acc_oflw;
	  statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	}
      if (dest & DEST_Z)
	{
	  pipe[0][0].vf[2] = FMAdd (ACC[2], ACC_OFLW[2], VF[fs][2], VF[ft][bc]);
          apipe.acc_oflw[2] = fmac_acc_oflw;
	  statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	}
      if (dest & DEST_W)
	{
	  pipe[0][0].vf[3] = FMAdd (ACC[3], ACC_OFLW[3], VF[fs][3], VF[ft][bc]);
          apipe.acc_oflw[3] = fmac_acc_oflw;
	  statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	}
      break;
    case 3:			/* MSUBx,y,z,w */
      BURST_UPPER0 (iH, dest, ft, fs, fd, bc);

      if (hazardcheckBC (me, fs, ft, dest, bc, 3))
	return 1;
      pipe[0][0].no = fd;
      pipe[0][0].mask = dest;
      pipe[0][0].flag = P_VF_REG;
      pipe[0][0].code = I_MSUB;
      pipe[0][0].code_addr = opc;
      if (dest & DEST_X)
	{
	  pipe[0][0].vf[0] = FMSub (ACC[0], ACC_OFLW[0], VF[fs][0], VF[ft][bc]);
	  statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	}
      if (dest & DEST_Y)
	{
	  pipe[0][0].vf[1] = FMSub (ACC[1], ACC_OFLW[1], VF[fs][1], VF[ft][bc]);
	  statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	}
      if (dest & DEST_Z)
	{
	  pipe[0][0].vf[2] = FMSub (ACC[2], ACC_OFLW[2], VF[fs][2], VF[ft][bc]);
	  statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	}
      if (dest & DEST_W)
	{
	  pipe[0][0].vf[3] = FMSub (ACC[3], ACC_OFLW[3], VF[fs][3], VF[ft][bc]);
	  statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	}
      break;
    case 4:			/* MAXx,y,z,w */
      BURST_UPPER0 (iH, dest, ft, fs, fd, bc);

      if (hazardcheckBC (me, fs, ft, dest, bc, 3))
	return 1;
      if (indebug ("inst_trace2"))
	{
#if 0
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		   "\tMAXv VF%02d=max(<%f,%f,%f,%f>, %f)=<%08x,%08x,%08x,%08x>=<%f,%f,%f,%f>\n",
		   (int) fd,
		   VF[fs][0],
		   VF[fs][1],
		   VF[fs][2],
		   VF[fs][3],
		   VF[ft][bc],
		   f_2_i (max (VF[fs][0], VF[ft][bc])),
		   f_2_i (max (VF[fs][1], VF[ft][bc])),
		   f_2_i (max (VF[fs][2], VF[ft][bc])),
		   f_2_i (max (VF[fs][3], VF[ft][bc])),
		   max (VF[fs][0], VF[ft][bc]),
		   max (VF[fs][1], VF[ft][bc]),
		   max (VF[fs][2], VF[ft][bc]),
		   max (VF[fs][3], VF[ft][bc])
	    );
#endif
	}
      pipe[0][0].no = fd;
      pipe[0][0].mask = dest;
      pipe[0][0].flag = P_VF_REG_NO_STATUS;
      pipe[0][0].code = I_MAX;
      pipe[0][0].code_addr = opc;
      if (dest & DEST_X)
	{
	  if (FCmp(VF[fs][0], '>', VF[ft][bc]))
	    pipe[0][0].vf[0] = VF[fs][0];
	  else
	    pipe[0][0].vf[0] = VF[ft][bc];
	}
      if (dest & DEST_Y)
	{
	  if (FCmp(VF[fs][1], '>', VF[ft][bc]))
	    pipe[0][0].vf[1] = VF[fs][1];
	  else
	    pipe[0][0].vf[1] = VF[ft][bc];
	}
      if (dest & DEST_Z)
	{
	  if (FCmp(VF[fs][2], '>', VF[ft][bc]))
	    pipe[0][0].vf[2] = VF[fs][2];
	  else
	    pipe[0][0].vf[2] = VF[ft][bc];
	}
      if (dest & DEST_W)
	{
	  if (FCmp(VF[fs][3], '>', VF[ft][bc]))
	    pipe[0][0].vf[3] = VF[fs][3];
	  else
	    pipe[0][0].vf[3] = VF[ft][bc];
	}
      break;
    case 5:			/* MINIx,y,z,w */
      BURST_UPPER0 (iH, dest, ft, fs, fd, bc);

      if (hazardcheckBC (me, fs, ft, dest, bc, 3))
	return 1;
      if (indebug ("inst_trace2"))
	{
#if 0
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		   "\tMINIv VF%02d=min(<%f,%f,%f,%f>, %f)=<%08x,%08x,%08x,%08x>=<%f,%f,%f,%f>\n",
		   (int) fd,
		   VF[fs][0],
		   VF[fs][1],
		   VF[fs][2],
		   VF[fs][3],
		   VF[ft][bc],
		   f_2_i (min (VF[fs][0], VF[ft][bc])),
		   f_2_i (min (VF[fs][1], VF[ft][bc])),
		   f_2_i (min (VF[fs][2], VF[ft][bc])),
		   f_2_i (min (VF[fs][3], VF[ft][bc])),
		   min (VF[fs][0], VF[ft][bc]),
		   min (VF[fs][1], VF[ft][bc]),
		   min (VF[fs][2], VF[ft][bc]),
		   min (VF[fs][3], VF[ft][bc])
	    );
#endif
	}
      pipe[0][0].no = fd;
      pipe[0][0].mask = dest;
      pipe[0][0].flag = P_VF_REG_NO_STATUS;
      pipe[0][0].code = I_MINI;
      pipe[0][0].code_addr = opc;
      if (dest & DEST_X)
	{
	  if (FCmp(VF[fs][0], '<', VF[ft][bc]))
	    pipe[0][0].vf[0] = VF[fs][0];
	  else
	    pipe[0][0].vf[0] = VF[ft][bc];
	}
      if (dest & DEST_Y)
	{
	  if (FCmp(VF[fs][1], '<', VF[ft][bc]))
	    pipe[0][0].vf[1] = VF[fs][1];
	  else
	    pipe[0][0].vf[1] = VF[ft][bc];
	}
      if (dest & DEST_Z)
	{
	  if (FCmp(VF[fs][2], '<', VF[ft][bc]))
	    pipe[0][0].vf[2] = VF[fs][2];
	  else
	    pipe[0][0].vf[2] = VF[ft][bc];
	}
      if (dest & DEST_W)
	{
	  if (FCmp(VF[fs][3], '<', VF[ft][bc]))
	    pipe[0][0].vf[3] = VF[fs][3];
	  else
	    pipe[0][0].vf[3] = VF[ft][bc];
	}
      break;
    case 6:			/* MULx,y,z,w */
      BURST_UPPER0 (iH, dest, ft, fs, fd, bc);

      if (hazardcheckBC (me, fs, ft, dest, bc, 3))
	return 1;
      if (indebug ("inst_trace2"))
	{
#if 0
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
	       "\tMULq VF%02d.%x = <%08x,%08x,%08x,%08x>==<%f,%f,%f,%f>\n",
		   fd,
		   dest,
		   f_2_i (FMul (VF[fs][0], VF[ft][bc])),
		   f_2_i (FMul (VF[fs][1], VF[ft][bc])),
		   f_2_i (FMul (VF[fs][2], VF[ft][bc])),
		   f_2_i (FMul (VF[fs][3], VF[ft][bc])),
		   FMul (VF[fs][0], VF[ft][bc]),
		   FMul (VF[fs][1], VF[ft][bc]),
		   FMul (VF[fs][2], VF[ft][bc]),
		   FMul (VF[fs][3], VF[ft][bc])
	    );
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		   "\t     <%f,%f,%f,%f>*<%f>\n",
		   VF[fs][0],
		   VF[fs][1],
		   VF[fs][2],
		   VF[fs][3],
		   VF[ft][bc]);
#endif
	}
      pipe[0][0].no = fd;
      pipe[0][0].mask = dest;
      pipe[0][0].flag = P_VF_REG;
      pipe[0][0].code = I_MUL;
      pipe[0][0].code_addr = opc;
      if (dest & DEST_X)
	{
	  pipe[0][0].vf[0] =
	    FMul (VF[fs][0], VF[ft][bc]);
	  statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	}
      if (dest & DEST_Y)
	{
	  pipe[0][0].vf[1] =
	    FMul (VF[fs][1], VF[ft][bc]);
	  statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	}
      if (dest & DEST_Z)
	{
	  pipe[0][0].vf[2] =
	    FMul (VF[fs][2], VF[ft][bc]);
	  statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	}
      if (dest & DEST_W)
	{
	  pipe[0][0].vf[3] =
	    FMul (VF[fs][3], VF[ft][bc]);
	  statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	}
      break;
    case 7:			/* MULq, MAXi, MULi, MINIi */
      BURST_UPPER0 (iH, dest, ft, fs, fd, bc);

      switch (bc)
	{
	case 0:		/* MULq */
	  if (indebug ("inst_trace2"))
	    {
#if 0
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\tMULq VF%02d = <%08x,%08x,%08x,%08x>==<%f,%f,%f,%f>\n",
		       fd,
		       f_2_i (FMul (VF[fs][0], Q)),
		       f_2_i (FMul (VF[fs][1], Q)),
		       f_2_i (FMul (VF[fs][2], Q)),
		       f_2_i (FMul (VF[fs][3], Q)),
		       FMul (VF[fs][0], Q),
		       FMul (VF[fs][1], Q),
		       FMul (VF[fs][2], Q),
		       FMul (VF[fs][3], Q)
		);
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\t <%f,%f,%f,%f>*<%f>\n",
		       VF[fs][0],
		       VF[fs][1],
		       VF[fs][2],
		       VF[fs][3],
		       Q);
#endif
	    }
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG;
	  pipe[0][0].code = I_MULq;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    {
	      pipe[0][0].vf[0] = FMul (VF[fs][0], Q);
	      statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      pipe[0][0].vf[1] = FMul (VF[fs][1], Q);
	      statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      pipe[0][0].vf[2] = FMul (VF[fs][2], Q);
	      statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      pipe[0][0].vf[3] = FMul (VF[fs][3], Q);
	      statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 1:		/* MAXi */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG_NO_STATUS;
	  pipe[0][0].code = I_MAXi;
	  pipe[0][0].code_addr = opc;
	  fval = I;
	  if (dest & DEST_X)
	    {
	      if (FCmp(VF[fs][0], '>', fval))
		{
		  pipe[0][0].vf[0] = VF[fs][0];
		}
	      else
		{
		  pipe[0][0].vf[0] = fval;
		}
	    }
	  if (dest & DEST_Y)
	    {
	      if (FCmp(VF[fs][1], '>', fval))
		{
		  pipe[0][0].vf[1] = VF[fs][1];
		}
	      else
		{
		  pipe[0][0].vf[1] = fval;
		}
	    }
	  if (dest & DEST_Z)
	    {
	      if (FCmp(VF[fs][2], '>', fval))
		{
		  pipe[0][0].vf[2] = VF[fs][2];
		}
	      else
		{
		  pipe[0][0].vf[2] = fval;
		}
	    }
	  if (dest & DEST_W)
	    {
	      if (FCmp(VF[fs][3], '>', fval))
		{
		  pipe[0][0].vf[3] = VF[fs][3];
		}
	      else
		{
		  pipe[0][0].vf[3] = fval;
		}
	    }
	  break;
	case 2:		/* MULi */
	  /* if(iH & 0x80000000){
	     if(hazardcheckI())
	     return 1; 
	     } */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;

	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG;
	  pipe[0][0].code = I_MULi;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    {
	      pipe[0][0].vf[0] = FMul (VF[fs][0], I);
	      statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      pipe[0][0].vf[1] = FMul (VF[fs][1], I);
	      statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      pipe[0][0].vf[2] = FMul (VF[fs][2], I);
	      statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      pipe[0][0].vf[3] = FMul (VF[fs][3], I);
	      statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 3:		/* MINIi */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG_NO_STATUS;
	  pipe[0][0].code = I_MINIi;
	  pipe[0][0].code_addr = opc;
	  fval = I;
	  if (dest & DEST_X)
	    {
	      if (FCmp(VF[fs][0], '<', fval))
		{
		  pipe[0][0].vf[0] = VF[fs][0];
		}
	      else
		{
		  pipe[0][0].vf[0] = fval;
		}
	    }
	  if (dest & DEST_Y)
	    {
	      if (FCmp(VF[fs][1], '<', fval))
		{
		  pipe[0][0].vf[1] = VF[fs][1];
		}
	      else
		{
		  pipe[0][0].vf[1] = fval;
		}
	    }
	  if (dest & DEST_Z)
	    {
	      if (FCmp(VF[fs][2], '<', fval))
		{
		  pipe[0][0].vf[2] = VF[fs][2];
		}
	      else
		{
		  pipe[0][0].vf[2] = fval;
		}
	    }
	  if (dest & DEST_W)
	    {
	      if (FCmp(VF[fs][3], '<', fval))
		{
		  pipe[0][0].vf[3] = VF[fs][3];
		}
	      else
		{
		  pipe[0][0].vf[3] = fval;
		}
	    }
	  break;
	}
      break;
    case 8:			/* ADDq, MADDq, ADDi, MADDi */
      BURST_UPPER0 (iH, dest, ft, fs, fd, bc);

      switch (bc)
	{
	case 0:		/* ADDq */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG;
	  pipe[0][0].code = I_ADDq;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    {
	      pipe[0][0].vf[0] = FAdd (VF[fs][0], Q);
	      statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      pipe[0][0].vf[1] = FAdd (VF[fs][1], Q);
	      statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      pipe[0][0].vf[2] = FAdd (VF[fs][2], Q);
	      statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      pipe[0][0].vf[3] = FAdd (VF[fs][3], Q);
	      statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 1:		/* MADDq */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG;
	  pipe[0][0].code = I_MADDq;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    {
	      pipe[0][0].vf[0] = FMAdd (ACC[0], ACC_OFLW[0], VF[fs][0], Q);
              apipe.acc_oflw[0] = fmac_acc_oflw;
	      statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      pipe[0][0].vf[1] = FMAdd (ACC[1], ACC_OFLW[1], VF[fs][1], Q);
              apipe.acc_oflw[1] = fmac_acc_oflw;
	      statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      pipe[0][0].vf[2] = FMAdd (ACC[2], ACC_OFLW[2], VF[fs][2], Q);
              apipe.acc_oflw[2] = fmac_acc_oflw;
	      statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      pipe[0][0].vf[3] = FMAdd (ACC[3], ACC_OFLW[3], VF[fs][3], Q);
              apipe.acc_oflw[3] = fmac_acc_oflw;
	      statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 2:		/* ADDi */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;

	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG;
	  pipe[0][0].code = I_ADDi;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    {
	      pipe[0][0].vf[0] = FAdd (VF[fs][0], I);
	      statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      pipe[0][0].vf[1] = FAdd (VF[fs][1], I);
	      statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      pipe[0][0].vf[2] = FAdd (VF[fs][2], I);
	      statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      pipe[0][0].vf[3] = FAdd (VF[fs][3], I);
	      statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 3:		/* MADDi */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;

	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG;
	  pipe[0][0].code = I_MADDi;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    {
	      pipe[0][0].vf[0] = FMAdd (ACC[0], ACC_OFLW[0], VF[fs][0], I);
	      statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      pipe[0][0].vf[1] = FMAdd (ACC[1], ACC_OFLW[1], VF[fs][1], I);
	      statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      pipe[0][0].vf[2] = FMAdd (ACC[2], ACC_OFLW[2], VF[fs][2], I);
	      statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      pipe[0][0].vf[3] = FMAdd (ACC[3], ACC_OFLW[3], VF[fs][3], I);
	      statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	    }
	  break;
	}
      break;
    case 9:			/* SUBq, MSUBq, SUBi, MSUBi */
      BURST_UPPER0 (iH, dest, ft, fs, fd, bc);

      switch (bc)
	{
	case 0:		/* SUBq */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG;
	  pipe[0][0].code = I_SUBq;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    {
	      pipe[0][0].vf[0] = FSub (VF[fs][0], Q);
	      statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      pipe[0][0].vf[1] = FSub (VF[fs][1], Q);
	      statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      pipe[0][0].vf[2] = FSub (VF[fs][2], Q);
	      statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      pipe[0][0].vf[3] = FSub (VF[fs][3], Q);
	      statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 1:		/* MSUBq */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;
	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG;
	  pipe[0][0].code = I_MSUBq;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    {
	      pipe[0][0].vf[0] = FMSub (ACC[0], ACC_OFLW[0], VF[fs][0], Q);
	      statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      pipe[0][0].vf[1] = FMSub (ACC[1], ACC_OFLW[1], VF[fs][1], Q);
	      statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      pipe[0][0].vf[2] = FMSub (ACC[2], ACC_OFLW[2], VF[fs][2], Q);
	      statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      pipe[0][0].vf[3] = FMSub (ACC[3], ACC_OFLW[3], VF[fs][3], Q);
	      statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 2:		/* SUBi */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;

	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG;
	  pipe[0][0].code = I_SUBi;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    {
	      pipe[0][0].vf[0] = FSub (VF[fs][0], I);
	      statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      pipe[0][0].vf[1] = FSub (VF[fs][1], I);
	      statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      pipe[0][0].vf[2] = FSub (VF[fs][2], I);
	      statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      pipe[0][0].vf[3] = FSub (VF[fs][3], I);
	      statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 3:		/* MSUBi */
	  if (hazardcheck (me, fs, 0, dest, 2))
	    return 1;

	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG;
	  pipe[0][0].code = I_MSUBi;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    {
	      pipe[0][0].vf[0] = FMSub (ACC[0], ACC_OFLW[0], VF[fs][0], I);
	      statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      pipe[0][0].vf[1] = FMSub (ACC[1], ACC_OFLW[1], VF[fs][1], I);
	      statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      pipe[0][0].vf[2] = FMSub (ACC[2], ACC_OFLW[2], VF[fs][2], I);
	      statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      pipe[0][0].vf[3] = FMSub (ACC[3], ACC_OFLW[3], VF[fs][3], I);
	      statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	    }
	  break;
	}
      break;
    case 10:			/* ADD, MADD, MUL, MAX */
      BURST_UPPER0 (iH, dest, ft, fs, fd, bc);

      switch (bc)
	{
	case 0:		/* ADD */
	  if (hazardcheck (me, fs, ft, dest, 3))
	    return 1;
	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG;
	  pipe[0][0].code = I_ADD;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    {
	      pipe[0][0].vf[0] = FAdd (VF[fs][0], VF[ft][0]);
	      statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      pipe[0][0].vf[1] = FAdd (VF[fs][1], VF[ft][1]);
	      statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      pipe[0][0].vf[2] = FAdd (VF[fs][2], VF[ft][2]);
	      statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      pipe[0][0].vf[3] = FAdd (VF[fs][3], VF[ft][3]);
	      statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 1:		/* MADD */
	  if (hazardcheck (me, fs, ft, dest, 3))
	    return 1;
	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG;
	  pipe[0][0].code = I_MADD;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    {
	      pipe[0][0].vf[0] = FMAdd (ACC[0], ACC_OFLW[0], VF[fs][0], VF[ft][0]);
	      statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      pipe[0][0].vf[1] = FMAdd (ACC[1], ACC_OFLW[1], VF[fs][1], VF[ft][1]);
	      statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      pipe[0][0].vf[2] = FMAdd (ACC[2], ACC_OFLW[2], VF[fs][2], VF[ft][2]);
	      statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      pipe[0][0].vf[3] = FMAdd (ACC[3], ACC_OFLW[3], VF[fs][3], VF[ft][3]);
	      statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 2:		/* MUL */
	  if (hazardcheck (me, fs, ft, dest, 3))
	    return 1;

	  if (indebug ("inst_trace2"))
	    {
#if 0
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\tMUL VF%02d = <%08x,%08x,%08x,%08x>==<%f,%f,%f,%f>\n",
		       fd,
		       f_2_i (FMul (VF[fs][0], VF[ft][0])),
		       f_2_i (FMul (VF[fs][1], VF[ft][1])),
		       f_2_i (FMul (VF[fs][2], VF[ft][2])),
		       f_2_i (FMul (VF[fs][3], VF[ft][3])),
		       FMul (VF[fs][0], VF[ft][0]),
		       FMul (VF[fs][1], VF[ft][1]),
		       FMul (VF[fs][2], VF[ft][2]),
		       FMul (VF[fs][3], VF[ft][3])
		);
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\t <%f,%f,%f,%f>*<%f,%f,%f,%f>\n",
		       VF[fs][0],
		       VF[fs][1],
		       VF[fs][2],
		       VF[fs][3],
		       VF[ft][0],
		       VF[ft][1],
		       VF[ft][2],
		       VF[ft][3]);
#endif
	    }
	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG;
	  pipe[0][0].code = I_MUL;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    {
	      pipe[0][0].vf[0] =
		FMul (VF[fs][0], VF[ft][0]);
	      statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      pipe[0][0].vf[1] =
		FMul (VF[fs][1], VF[ft][1]);
	      statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      pipe[0][0].vf[2] =
		FMul (VF[fs][2], VF[ft][2]);
	      statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      pipe[0][0].vf[3] =
		FMul (VF[fs][3], VF[ft][3]);
	      statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 3:		/* MAX */
	  if (hazardcheck (me, fs, ft, dest, 3))
	    return 1;
	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG_NO_STATUS;
	  pipe[0][0].code = I_MAX;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    {
	      if (FCmp(VF[fs][0], '>', VF[ft][0]))
		{
		  pipe[0][0].vf[0] = VF[fs][0];
		}
	      else
		{
		  pipe[0][0].vf[0] = VF[ft][0];
		}
	    }
	  if (dest & DEST_Y)
	    {
	      if (FCmp(VF[fs][1], '>', VF[ft][1]))
		{
		  pipe[0][0].vf[1] = VF[fs][1];
		}
	      else
		{
		  pipe[0][0].vf[1] = VF[ft][1];
		}
	    }
	  if (dest & DEST_Z)
	    {
	      if (FCmp(VF[fs][2], '>', VF[ft][2]))
		{
		  pipe[0][0].vf[2] = VF[fs][2];
		}
	      else
		{
		  pipe[0][0].vf[2] = VF[ft][2];
		}
	    }
	  if (dest & DEST_W)
	    {
	      if (FCmp(VF[fs][3], '>', VF[ft][3]))
		{
		  pipe[0][0].vf[3] = VF[fs][3];
		}
	      else
		{
		  pipe[0][0].vf[3] = VF[ft][3];
		}
	    }
	  break;
	}
      break;
    case 11:			/* SUB, MSUB, OPMSUB, MINI */
      BURST_UPPER0 (iH, dest, ft, fs, fd, bc);

      switch (bc)
	{
	case 0:		/* SUB */
	  if (hazardcheck (me, fs, ft, dest, 3))
	    return 1;
	  if (indebug ("inst_trace2"))
	    {
#if 0
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\tSUB VF%02d.%x = <%08x,%08x,%08x,%08x>==<%f,%f,%f,%f>\n",
		       fd, dest,
		       f_2_i (FSub (VF[fs][0], VF[ft][0])),
		       f_2_i (FSub (VF[fs][1], VF[ft][1])),
		       f_2_i (FSub (VF[fs][2], VF[ft][2])),
		       f_2_i (FSub (VF[fs][3], VF[ft][3])),
		       FSub (VF[fs][0], VF[ft][0]),
		       FSub (VF[fs][1], VF[ft][1]),
		       FSub (VF[fs][2], VF[ft][2]),
		       FSub (VF[fs][3], VF[ft][3])
		);
#endif
	    }
	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG;
	  pipe[0][0].code = I_SUB;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    {
	      pipe[0][0].vf[0] =
		FSub (VF[fs][0], VF[ft][0]);
	      statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      pipe[0][0].vf[1] =
		FSub (VF[fs][1], VF[ft][1]);
	      statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      pipe[0][0].vf[2] =
		FSub (VF[fs][2], VF[ft][2]);
	      statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      pipe[0][0].vf[3] =
		FSub (VF[fs][3], VF[ft][3]);
	      statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 1:		/* MSUB */
	  if (hazardcheck (me, fs, ft, dest, 3))
	    return 1;
	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG;
	  pipe[0][0].code = I_MSUB;
	  pipe[0][0].code_addr = opc;
	  if (dest & DEST_X)
	    {
	      pipe[0][0].vf[0] = FMSub (ACC[0], ACC_OFLW[0], VF[fs][0], VF[ft][0]);
	      statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	    }
	  if (dest & DEST_Y)
	    {
	      pipe[0][0].vf[1] = FMSub (ACC[1], ACC_OFLW[1], VF[fs][1], VF[ft][1]);
	      statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	    }
	  if (dest & DEST_Z)
	    {
	      pipe[0][0].vf[2] = FMSub (ACC[2], ACC_OFLW[2], VF[fs][2], VF[ft][2]);
	      statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	    }
	  if (dest & DEST_W)
	    {
	      pipe[0][0].vf[3] = FMSub (ACC[3], ACC_OFLW[3], VF[fs][3], VF[ft][3]);
	      statuscheck_fmac (pipe[0][0].vf[3], &pipe[0][0].status, 3);
	    }
	  break;
	case 2:		/* OPMSUB */
	  if (hazardcheck (me, fs, ft, dest, 3))
	    return 1;
	  ASSERT(dest == 0xe); /* enforced by assembler */
	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG;
	  pipe[0][0].code = I_OPMSUB;
	  pipe[0][0].code_addr = opc;
	  if(dest & DEST_X)
	    {
	      pipe[0][0].vf[0] = FMSub (ACC[0], ACC_OFLW[0], VF[fs][1], VF[ft][2]);
              apipe.acc_oflw[0] = fmac_acc_oflw;
	      statuscheck_fmac (pipe[0][0].vf[0], &pipe[0][0].status, 0);
	      if (indebug ("inst_trace2"))
		{
		  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
			   "\tOPMSUB.x a=%08x o=%d fs=%08x ft=%08x => %08x\n",
			   ACC[0].i, ACC_OFLW[0], VF[fs][1].i, VF[ft][2].i, pipe[0][0].vf[0].i);
		}
	    }
	  if(dest & DEST_Y)
	    {
	      pipe[0][0].vf[1] = FMSub (ACC[1], ACC_OFLW[1], VF[fs][2], VF[ft][0]);
              apipe.acc_oflw[1] = fmac_acc_oflw;
	      statuscheck_fmac (pipe[0][0].vf[1], &pipe[0][0].status, 1);
	      if (indebug ("inst_trace2"))
		{
		  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
			   "\tOPMSUB.y a=%08x o=%d fs=%08x ft=%08x => %08x\n",
			   ACC[1].i, ACC_OFLW[1], VF[fs][2].i, VF[ft][0].i, pipe[0][0].vf[1].i);
		}
	    }
	  if(dest & DEST_Z)
	    {
	      pipe[0][0].vf[2] = FMSub (ACC[2], ACC_OFLW[2], VF[fs][0], VF[ft][1]);
              apipe.acc_oflw[2] = fmac_acc_oflw;
	      statuscheck_fmac (pipe[0][0].vf[2], &pipe[0][0].status, 2);
	      if (indebug ("inst_trace2"))
		{
		  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
			   "\tOPMSUB.z a=%08x o=%d fs=%08x ft=%08x => %08x\n",
			   ACC[2].i, ACC_OFLW[2], VF[fs][0].i, VF[ft][1].i, pipe[0][0].vf[2].i);
		}
	    }
	  break;
	case 3:		/* MINI */
	  if (hazardcheck (me, fs, ft, dest, 3))
	    return 1;

	  if (indebug ("inst_trace2"))
	    {
#if 0
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\tMINI VF%02d=min(<%f,%f,%f,%f>, <%f,%f,%f,%f>)=<%08x,%08x,%08x,%08x>=<%f,%f,%f,%f>\n",
		       (int) fd,
		       VF[fs][0],
		       VF[fs][1],
		       VF[fs][2],
		       VF[fs][3],
		       VF[ft][0],
		       VF[ft][1],
		       VF[ft][2],
		       VF[ft][3],
		       f_2_i (min (VF[fs][0], VF[ft][0])),
		       f_2_i (min (VF[fs][1], VF[ft][1])),
		       f_2_i (min (VF[fs][2], VF[ft][2])),
		       f_2_i (min (VF[fs][3], VF[ft][3])),
		       min (VF[fs][0], VF[ft][0]),
		       min (VF[fs][1], VF[ft][1]),
		       min (VF[fs][2], VF[ft][2]),
		       min (VF[fs][3], VF[ft][3])
		);
#endif
	    }
	  pipe[0][0].no = fd;
	  pipe[0][0].mask = dest;
	  pipe[0][0].flag = P_VF_REG_NO_STATUS;
	  if (dest & DEST_X)
	    {
	      if (FCmp(VF[fs][0], '<', VF[ft][0]))
		{
		  pipe[0][0].vf[0] = VF[fs][0];
		}
	      else
		{
		  pipe[0][0].vf[0] = VF[ft][0];
		}
	    }
	  if (dest & DEST_Y)
	    {
	      if (FCmp(VF[fs][1], '<', VF[ft][1]))
		{
		  pipe[0][0].vf[1] = VF[fs][1];
		}
	      else
		{
		  pipe[0][0].vf[1] = VF[ft][1];
		}
	    }
	  if (dest & DEST_Z)
	    {
	      if (FCmp(VF[fs][2], '<', VF[ft][2]))
		{
		  pipe[0][0].vf[2] = VF[fs][2];
		}
	      else
		{
		  pipe[0][0].vf[2] = VF[ft][2];
		}
	    }
	  if (dest & DEST_W)
	    {
	      if (FCmp(VF[fs][3], '<', VF[ft][3]))
		{
		  pipe[0][0].vf[3] = VF[fs][3];
		}
	      else
		{
		  pipe[0][0].vf[3] = VF[ft][3];
		}
	    }
	  break;
	}
      break;
    case 15:
      if (Upper_special (me, iH, iL))
	return 1;
      break;
    default:
      fprintf (stderr, "Undefined opcode(%08lx)\n", iH);
      exit (1);
    }

  if (iH & 0x80000000)
    return 1;
  if (iH & 0x04000000)
    {
      /* DEBUG PRINT */
      if (dpr_mode)
	printf ("\"%08lx\"\n", iL);
      return 1;
    }

  /* LowerOp execute */
  switch ((iL >> 27) & 0x1f)
    {
    case 0:			/* LQ, SQ, ---, --- */
      if (iL & 0x00000400)
	imm11 = (iL & 0x07ff) | 0xfffff800;
      else
	imm11 = iL & 0x07ff;
      switch (get_ic (iL))
	{
	case 0:		/* LQ */
	  {
	    int addr = imm11 + VI[get_fs (iL)];
	    int dest = get_dest (iL);

	    if (indebug ("inst_trace2"))
	      {
#if 0
		fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
			 "\tLQ VF%02d = (%d) = <%08x, %08x, %08x, %08x> = <%f, %f, %f, %f>\n",
			 get_ft (iL),
			 (int) (addr),
			 f_2_i (T2H_F4 (MEM (addr, 0))),
			 f_2_i (T2H_F4 (MEM (addr, 1))),
			 f_2_i (T2H_F4 (MEM (addr, 2))),
			 f_2_i (T2H_F4 (MEM (addr, 3))),
			 T2H_F4 (MEM (addr, 0)),
			 T2H_F4 (MEM (addr, 1)),
			 T2H_F4 (MEM (addr, 2)),
			 T2H_F4 (MEM (addr, 3)));
#endif
	      }
	    if (hazardcheckVI (me, get_fs (iL), 0, 2) ||
		hazardcheckMEM (me, addr))
	      {
		pipe[0][0].flag = P_EMPTY;
		apipe.flag = 0;
		return 1;
	      }
	    pipe[0][1].no = get_ft (iL);
	    pipe[0][1].mask = dest;
	    pipe[0][1].flag = P_VF_REG;
	    pipe[0][1].code = I_LQ;
	    pipe[0][1].code_addr = opc;
	    /* For vu0 we need to handle memory accesses used to fetch
	       vu1 regs.  We handle the common case of memory first though.  */
	    if (me->config.vu_number == 1
		|| addr < VU0_VU1_REG_START)
	      {
		range_check (me, addr);
		if (dest & DEST_X)
		  pipe[0][1].vf[0] = T2H_F4 (MEM (addr, 0));
		if (dest & DEST_Y)
		  pipe[0][1].vf[1] = T2H_F4 (MEM (addr, 1));
		if (dest & DEST_Z)
		  pipe[0][1].vf[2] = T2H_F4 (MEM (addr, 2));
		if (dest & DEST_W)
		  pipe[0][1].vf[3] = T2H_F4 (MEM (addr, 3));
	      }
	    /* Testing vu_number again isn't technically necessary, but
	       it makes the code clearer, and gcc should throw the test
	       away anyway.  */
	    else if (me->config.vu_number == 0
		     && addr >= VU0_VU1_FPREG_START
		     && addr < (VU0_VU1_FPREG_START + VU0_VU1_FPREG_SIZE))
	      {
		int vu1_regno = addr - VU0_VU1_FPREG_START;
		floatie *vf = get_VF (&vu1_device, vu1_regno);

		if (dest & DEST_X)
		  pipe[0][1].vf[0] = vf[0];
		if (dest & DEST_Y)
		  pipe[0][1].vf[1] = vf[1];
		if (dest & DEST_Z)
		  pipe[0][1].vf[2] = vf[2];
		if (dest & DEST_W)
		  pipe[0][1].vf[3] = vf[3];
	      }
	    else if (me->config.vu_number == 0
		     && addr >= VU0_VU1_CTRLREG_START
		     && addr < (VU0_VU1_CTRLREG_START + VU0_VU1_CTRLREG_SIZE))
	      {
		unsigned val = get_ctrlreg (&vu1_device, addr);

		if (dest & DEST_X)
		  pipe[0][1].vf[0].i = val;
		if (dest & DEST_Y)
		  pipe[0][1].vf[1].i = 0;
		if (dest & DEST_Z)
		  pipe[0][1].vf[2].i = 0;
		if (dest & DEST_W)
		  pipe[0][1].vf[3].i = 0;
	      }
	    else
	      mem_range_error (me, addr);
	    break;
	  }
	case 1:		/* SQ */
	  if (indebug ("inst_trace2"))
	    {
#if 0
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\tSQ (%d) = <%08x, %08x, %08x, %08x> = <%f, %f, %f, %f>\n",
		       (int) (imm11 + VI[get_ft (iL)]),
		       f_2_i (VF[get_fs (iL)][0]),
		       f_2_i (VF[get_fs (iL)][1]),
		       f_2_i (VF[get_fs (iL)][2]),
		       f_2_i (VF[get_fs (iL)][3]),
		       VF[get_fs (iL)][0],
		       VF[get_fs (iL)][1],
		       VF[get_fs (iL)][2],
		       VF[get_fs (iL)][3]);
#endif
	    }
	  if (hazardcheck (me, 0, get_fs (iL), get_dest (iL), 1) || /* !ihc: ft->fs */
	      hazardcheckVI (me, get_ft (iL), 0, 2)) /* !ihc: fs->ft */
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
	  pipe[0][1].mask = get_dest (iL);
	  pipe[0][1].flag = P_MEMORY;
	  pipe[0][1].addr = imm11 + VI[get_ft (iL)];
	  pipe[0][1].code = I_SQ;
	  pipe[0][1].code_addr = opc;
	  if (get_dest (iL) & DEST_X)
	    pipe[0][1].vf[0] = VF[get_fs (iL)][0];
	  if (get_dest (iL) & DEST_Y)
	    pipe[0][1].vf[1] = VF[get_fs (iL)][1];
	  if (get_dest (iL) & DEST_Z)
	    pipe[0][1].vf[2] = VF[get_fs (iL)][2];
	  if (get_dest (iL) & DEST_W)
	    pipe[0][1].vf[3] = VF[get_fs (iL)][3];
	  break;
	default:
	  fprintf (stderr, "Undefined opcode(%08lx)\n", iL);
	  exit (1);
	}
      break;
    case 1:			/* ILW, ISW, ---, --- */
      if (iL & 0x00000400)
	imm11 = (iL & 0x07ff) | 0xfffff800;
      else
	imm11 = iL & 0x07ff;
      switch (get_ic (iL))
	{
	case 0:		/* ILW */
	  {
	    int addr = imm11 + VI[get_fs (iL)];
	    int dest = get_dest (iL);

	    if (indebug ("inst_trace2"))
	      {
		printf ("	VI%02d <- MEM(%d).%x = (%x,%x,%x,%x)\n",
			get_ft (iL), addr, dest,
			MEM (addr, 0),
			MEM (addr, 1),
			MEM (addr, 2),
			MEM (addr, 3));
	      }
	    if (hazardcheckVI (me, get_fs (iL), 0, 2) ||
		hazardcheckMEM (me, addr))
	      {
		pipe[0][0].flag = P_EMPTY;
		apipe.flag = 0;
		return 1;
	      }
	    /* ??? The range check should probably go before stuffing the
	       pipeline with contents, which is what we do here (I realize it
	       currently only prints an error message, but maybe later it'll
	       do something else).  However, the code for LQ/LQI/LQD
	       doesn't.  */
	    /* For vu0 we need to handle memory accesses used to fetch
	       vu1 regs.  We handle the common case of memory first though.  */
	    if (me->config.vu_number == 1
		|| addr < VU0_VU1_REG_START)
	      {
		range_check (me, addr);
		if (dest & DEST_X)
		  pipe[0][1].vf[0].i = T2H_I4 (MEM (addr, 0));
		if (dest & DEST_Y)
		  pipe[0][1].vf[1].i = T2H_I4 (MEM (addr, 1));
		if (dest & DEST_Z)
		  pipe[0][1].vf[2].i = T2H_I4 (MEM (addr, 2));
		if (dest & DEST_W)
		  pipe[0][1].vf[3].i = T2H_I4 (MEM (addr, 3));
	      }
	    /* Testing vu_number again isn't technically necessary. 
	       Sue me.  */
	    else if (me->config.vu_number == 0
		     && addr >= VU0_VU1_INTREG_START
		     && addr < (VU0_VU1_INTREG_START + VU0_VU1_INTREG_SIZE))
	      {
		int vu1_regno = addr - VU0_VU1_INTREG_START;
		short *vi = get_VI (&vu1_device, vu1_regno);

		if (dest & DEST_X)
		  pipe[0][1].vf[0].i = *(unsigned short *) vi;
		if (dest & DEST_Y)
		  pipe[0][1].vf[1].i = 0;
		if (dest & DEST_Z)
		  pipe[0][1].vf[2].i = 0;
		if (dest & DEST_W)
		  pipe[0][1].vf[3].i = 0;
	      }
	    else if (me->config.vu_number == 0
		     && addr >= VU0_VU1_CTRLREG_START
		     && addr < (VU0_VU1_CTRLREG_START + VU0_VU1_CTRLREG_SIZE))
	      {
		unsigned short val = get_ctrlreg (&vu1_device, addr);

		if (dest & DEST_X)
		  pipe[0][1].vf[0].i = val;
		if (dest & DEST_Y)
		  pipe[0][1].vf[1].i = 0;
		if (dest & DEST_Z)
		  pipe[0][1].vf[2].i = 0;
		if (dest & DEST_W)
		  pipe[0][1].vf[3].i = 0;
	      }
	    else
	      mem_range_error (me, addr);
	    pipe[0][1].no = get_ft (iL) + 32;
	    pipe[0][1].mask = dest;
	    pipe[0][1].flag = P_VF_REG;
	    pipe[0][1].code = I_ILW;
	    pipe[0][1].code_addr = opc;
	    break;
	  }
	case 1:		/* ISW */
	  if (indebug ("inst_trace2"))
	    {
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\tISW (%ld) = <%x,%x,%x,%x>\n",
		       imm11 + VI[get_fs (iL)],
		       (VI[get_ft (iL)]) & 0xffff,
		       (VI[get_ft (iL)]) & 0xffff,
		       (VI[get_ft (iL)]) & 0xffff,
		       (VI[get_ft (iL)]) & 0xffff);
	    }
	  if (hazardcheckVI (me, get_fs (iL), get_ft (iL), 3))
	    {
	      pipe[0][0].flag = P_EMPTY;
	      apipe.flag = 0;
	      return 1;
	    }
	  pipe[0][1].mask = get_dest (iL);
	  pipe[0][1].flag = P_MEMORY;
	  pipe[0][1].addr = imm11 + VI[get_fs (iL)];
	  pipe[0][1].code = I_ISW;
	  pipe[0][1].code_addr = opc;
	  if (get_dest (iL) & DEST_X)
	    pipe[0][1].vf[0].i = (VI[get_ft (iL)]) & 0xffff;
	  if (get_dest (iL) & DEST_Y)
	    pipe[0][1].vf[1].i = (VI[get_ft (iL)]) & 0xffff;
	  if (get_dest (iL) & DEST_Z)
	    pipe[0][1].vf[2].i = (VI[get_ft (iL)]) & 0xffff;
	  if (get_dest (iL) & DEST_W)
	    pipe[0][1].vf[3].i = (VI[get_ft (iL)]) & 0xffff;
	  break;
	default:
	  fprintf (stderr, "Undefined opcode(LowerOp:%08lx)\n", iL);
	  exit (1);
	}
      break;
    case 2:			/* IADDIU, ISUBIU, ---, --- */
      if (hazardcheckVI (me, get_fs (iL), 0, 2))
	{
	  pipe[0][0].flag = P_EMPTY;
	  apipe.flag = 0;
	  return 1;
	}
      imm15 = ((iL >> 10) & 0x7800) | (iL & 0x7ff);
      switch (get_ic (iL))
	{
	case 0:		/* IADDIU */
	  if (indebug ("inst_trace2"))
	    {
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\tIADDIU VI%02d = %d + %d = %d\n",
		       get_ft (iL),
		       VI[get_fs (iL)],
		       imm15,
		       VI[get_fs (iL)] + imm15
		);
	    }
	  ipipe.no = get_ft (iL);
	  ipipe.flag = 2;
	  ipipe.vi = VI[get_fs (iL)] + imm15;
	  ipipe.code = I_IADDIU;
	  ipipe.code_addr = opc;
	  break;
	case 1:		/* ISUBIU */
	  ipipe.no = get_ft (iL);
	  ipipe.flag = 2;
	  ipipe.vi = VI[get_fs (iL)] - imm15;
	  ipipe.code = I_ISUBIU;
	  ipipe.code_addr = opc;
	  break;
	default:
	  fprintf (stderr, "Undefined opcode(LowerOp:%08lx)\n", iL);
	  exit (1);
	}
      break;
    case 3:
      fprintf (stderr, "Undefined opcode(LowerOp:%08lx)\n", iL);
      exit (1);
      break;
    case 4:			/* FCEQ, FCSET, FCAND, FCOR */
      imm24 = iL & 0x00ffffff;
      switch (get_ic (iL))
	{
	case 0:		/* FCEQ */
	  ipipe.no = 1;
	  ipipe.flag = 1;
	  if (clipflag == imm24)
	    ipipe.vi = 1;
	  else
	    ipipe.vi = 0;
	  ipipe.code = I_FCEQ;
	  ipipe.code_addr = opc;
	  break;
	case 1:		/* FCSET */
	  pipe[0][1].flag = P_CLIP_REG;
	  pipe[0][1].status = imm24;
	  pipe[0][1].code = I_FCSET;
	  pipe[0][1].code_addr = opc;
	  break;
	case 2:		/* FCAND */
	  ipipe.no = 1;
	  ipipe.flag = 1;
	  if (clipflag & imm24)
	    ipipe.vi = 1;
	  else
	    ipipe.vi = 0;
	  ipipe.code = I_FCAND;
	  ipipe.code_addr = opc;
	  break;
	case 3:		/* FCOR */
	  ipipe.no = 1;
	  ipipe.flag = 1;
	  if ((clipflag | imm24) == 0x00ffffff)
	    ipipe.vi = 1;
	  else
	    ipipe.vi = 0;
	  ipipe.code = I_FCOR;
	  ipipe.code_addr = opc;
	  break;
	}
      break;
    case 5:			/* FSEQ, FSSET, FSAND, FSOR */
      imm12 = ((iL >> 10) & 0x0800) | (iL & 0x7ff);
      switch (get_ic (iL))
	{
	case 0:		/* FSEQ */
	  trace_vi_x(me, "FSEQ", get_ft(iL), (statusflag == imm12) ? 1 : 0);
	  ipipe.no = get_ft (iL);
	  ipipe.flag = 1;
	  if (statusflag == imm12)
	    ipipe.vi = 1;
	  else
	    ipipe.vi = 0;
	  ipipe.code = I_FSEQ;
	  ipipe.code_addr = opc;
	  break;
	case 1:		/* FSSET */
	  trace_x(me, "FSSET", "status", imm12);
	  pipe[0][1].flag = P_STATUS_REG;
	  pipe[0][1].status = imm12;
	  pipe[0][1].code = I_FSSET;
	  pipe[0][1].code_addr = opc;
	  break;
	case 2:		/* FSAND */
	  trace_vi_x(me, "FSAND", get_ft(iL), statusflag & imm12);
	  ipipe.no = get_ft (iL);
	  ipipe.flag = 1;
	  ipipe.vi = statusflag & imm12;
	  ipipe.code = I_FSAND;
	  ipipe.code_addr = opc;
	  break;
	case 3:		/* FSOR */
	  trace_vi_x(me, "FSOR", get_ft(iL), statusflag | imm12);
	  ipipe.no = get_ft (iL);
	  ipipe.flag = 1;
	  ipipe.vi = statusflag | imm12;
	  ipipe.code = I_FSOR;
	  ipipe.code_addr = opc;
	  break;
	}
      break;
    case 6:			/* FMEQ, NULL, FMAND, FMOR */
      switch (get_ic (iL))
	{
        case 0:		/* FMEQ */
	  if (indebug ("inst_trace2"))
	    {
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\tFMEQ VI%02d = %08x == %08x\n",
		       get_ft (iL),
		       VI[get_fs (iL)] & 0xffff,
		       MACflag & 0xffff
		);
	    }
	  if (hazardcheckVI (me, get_fs (iL), 0, 2))
	    {
	      ipipe.flag = 0;
	      return 1;
	    }
	  ipipe.no = get_ft (iL);
	  ipipe.flag = 1;
	  if ((short) MACflag == VI[get_fs (iL)])
	    ipipe.vi = 1;
	  else
	    ipipe.vi = 0;
	  ipipe.code = I_FMEQ;
	  ipipe.code_addr = opc;
	  break;
	case 2:		/* FMAND */
	  if (indebug ("inst_trace2"))
	    {
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "\tFMAND VI%02d = %08x & %08x = %08x\n",
		       get_ft (iL),
		       VI[get_fs (iL)],
		       MACflag & 0xffff,
		       (MACflag & 0xffff) & VI[get_fs (iL)]
		);
	    }
	  if (hazardcheckVI (me, get_fs (iL), 0, 2))
	    {
	      ipipe.flag = 0;
	      return 1;
	    }
	  ipipe.no = get_ft (iL);
	  ipipe.flag = 1;
	  ipipe.vi = (MACflag & 0xffff) & VI[get_fs (iL)];
	  ipipe.code = I_FMAND;
	  ipipe.code_addr = opc;
	  break;
	case 3:		/* FMOR */
	  if (hazardcheckVI (me, get_fs (iL), 0, 2))
	    {
	      ipipe.flag = 0;
	      return 1;
	    }
	  ipipe.no = get_ft (iL);
	  ipipe.flag = 1;
	  ipipe.vi = (MACflag & 0xffff) | VI[get_fs (iL)];
	  ipipe.code = I_FMOR;
	  ipipe.code_addr = opc;
	  break;
	}
      break;
    case 7:			/* FCGET, ---, ---, --- */
      switch (get_ic (iL))
	{
	case 0:		/* FCGET */
	  ipipe.no = get_ft (iL);
	  ipipe.flag = 1;
	  ipipe.vi = clipflag & 0x0fff;
	  ipipe.code = I_FCGET;
	  ipipe.code_addr = opc;
	  break;
	default:
	  fprintf (stderr, "Undefined opcode(LowerOp:%08lx)\n", iL);
	  exit (1);
	}
      break;
    case 8:			/* B, BAL, ---, --- */
      if (iL & 0x00000400)
	imm11 = (iL & 0x07ff) | 0xfffff800;
      else
	imm11 = iL & 0x07ff;
      switch (get_ic (iL))
	{
	case 0:		/* B */
	  jaddr = pc + imm11;
	  jflag = 1;
	  break;
	case 1:		/* BAL */
	  jaddr = pc + imm11;
	  jflag = 1;
	  ipipe.no = get_ft (iL);
	  ipipe.flag = 1;
	  ipipe.vi = pc + 1;
	  ipipe.code = I_BAL;
	  ipipe.code_addr = opc;
	  break;
	default:
	  fprintf (stderr, "Undefined opcode(LowerOp:%08lx)\n", iL);
	  exit (1);
	}
      break;
    case 9:			/* JR, JALR, ---, --- */
      if (hazardcheckVI (me, get_fs (iL), 0, 2))
	{
	  pipe[0][0].flag = P_EMPTY;
	  apipe.flag = 0;
	  return 1;
	}
      switch (get_ic (iL))
	{
	case 0:		/* JR */
	  jaddr = VI[get_fs (iL)];
	  jflag = 1;
	  break;
	case 1:		/* JALR */
	  jaddr = VI[get_fs (iL)];
	  jflag = 1;
	  ipipe.no = get_ft (iL);
	  ipipe.flag = 1;
	  ipipe.vi = pc + 1;
	  ipipe.code = I_JAL;
	  ipipe.code_addr = opc;
	  break;
	default:
	  fprintf (stderr, "Undefined opcode(LowerOp:%08lx)\n", iL);
	  exit (1);
	}
      break;
    case 10:			/* IBEQ, IBNE, NULL, NULL */
      if (iL & 0x00000400)
	imm11 = (iL & 0x07ff) | 0xfffff800;
      else
	imm11 = iL & 0x07ff;
      if (hazardcheckVI (me, get_fs (iL), get_ft (iL), 3))
	{
	  pipe[0][0].flag = P_EMPTY;
	  apipe.flag = 0;
	  return 1;
	}
      {
	int ft = get_ft (iL);
	int fs = get_fs (iL);
	short VI_ft, VI_fs;
	if (ipipe.old_reg == ft)
	  VI_ft = ipipe.old_value;
	else
	  VI_ft = VI[ft];

	if (ipipe.old_reg == fs)
	  VI_fs = ipipe.old_value;
	else
	  VI_fs = VI[fs];

        if (indebug ("inst_trace2"))
          {
            fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
                     ">>>\tIBxx VI[%d], VI[%d] : 0x%04x 0x%04x\n", ft, fs, VI_ft, VI_fs);
          }

	switch (get_ic (iL))
	  {
	  case 0:		/* IBEQ */
	    if (indebug ("inst_trace2"))
	      {
		fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
			 ">>> IBEQ VI[%d], VI[%d] : %d %d\n", get_ft (iL), get_fs (iL), VI[get_ft (iL)], VI[get_fs (iL)]);
	      }
	    if (VI_ft == VI_fs)
	      {
		jaddr = pc + imm11;
		jflag = 1;
	      }
	    break;
	  case 1:		/* IBNE */
	    if (VI_ft != VI_fs)
	      {
		jaddr = pc + imm11;
		jflag = 1;
	      }
	    break;
	  default:
	    fprintf (stderr, "Undefined opcode(LowerOp:%08lx)\n", iL);
	    exit (1);
	  }
      }
      break;
    case 11:			/* IBLTZ, IBGTZ, IBLEZ, IBGEZ */
      if (iL & 0x00000400)
	imm11 = (iL & 0x07ff) | 0xfffff800;
      else
	imm11 = iL & 0x07ff;
      if (hazardcheckVI (me, get_fs (iL), 0, 2))
	{
	  pipe[0][0].flag = P_EMPTY;
	  apipe.flag = 0;
	  return 1;
	}
      {
	short VI_fs;
	if (ipipe.old_reg == get_fs (iL))
	  VI_fs = ipipe.old_value;
	else
	  VI_fs = VI[get_fs (iL)];

	switch (get_ic (iL))
	  {
	  case 0:		/* IBLTZ */
	    if (VI_fs < 0)
	      {
		jaddr = pc + imm11;
		jflag = 1;
	      }
	    break;
	  case 1:		/* IBGTZ */
	    if (VI_fs > 0)
	      {
		jaddr = pc + imm11;
		jflag = 1;
	      }
	    break;
	  case 2:		/* IBLEZ */
	    if (VI_fs <= 0)
	      {
		jaddr = pc + imm11;
		jflag = 1;
	      }
	    break;
	  case 3:		/* IBGEZ */
	    if (VI_fs >= 0)
	      {
		jaddr = pc + imm11;
		jflag = 1;
	      }
	    break;
	  }
      }
      break;
    case 16:
      switch ((iL >> 2) & 0xf)
	{
	case 12:		/* IADD, ISUB, IADDI, --- */
	  switch (get_bc (iL))
	    {
	    case 0:		/* IADD */
	      if (indebug ("inst_trace2"))
		{
		  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
			   "\tIADD VI%02d = %d + %d = %d\n",
			   get_fd (iL),
			   VI[get_fs (iL)],
			   VI[get_ft (iL)],
			   VI[get_fs (iL)] + VI[get_ft (iL)]
		    );
		}
	      if (hazardcheckVI (me, get_fs (iL), get_ft (iL), 3))
		{
		  pipe[0][0].flag = P_EMPTY;
		  apipe.flag = 0;
		  return 1;
		}
	      ipipe.no = get_fd (iL);
	      ipipe.flag = 2;
	      ipipe.vi = VI[get_fs (iL)] + VI[get_ft (iL)];
	      ipipe.code = I_IADD;
	      ipipe.code_addr = opc;
	      break;
	    case 1:		/* ISUB */
	      if (hazardcheckVI (me, get_fs (iL), get_ft (iL), 3))
		{
		  pipe[0][0].flag = P_EMPTY;
		  apipe.flag = 0;
		  return 1;
		}
	      ipipe.no = get_fd (iL);
	      ipipe.flag = 2;
	      ipipe.vi = VI[get_fs (iL)] - VI[get_ft (iL)];
	      ipipe.code = I_ISUB;
	      ipipe.code_addr = opc;
	      break;
	    case 2:		/* IADDI */
	      if (hazardcheckVI (me, get_fs (iL), 0, 2))
		{
		  pipe[0][0].flag = P_EMPTY;
		  apipe.flag = 0;
		  return 1;
		}
	      if (iL & 0x00000400)
		imm5 = ((iL >> 6) & 0x1f) | 0xffffffe0;
	      else
		imm5 = (iL >> 6) & 0x1f;
	      if (indebug ("inst_trace2"))
		{
		  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
			   "\tIADDI VI%02d = %d + %d = %d\n",
			   get_ft (iL),
			   VI[get_fs (iL)],
			   imm5,
			   VI[get_fs (iL)] + imm5
		    );
		}
	      ipipe.no = get_ft (iL);
	      ipipe.flag = 2;
	      ipipe.vi = VI[get_fs (iL)] + imm5;
	      ipipe.code = I_IADDI;
	      ipipe.code_addr = opc;
	      break;
	    default:
	      fprintf (stderr,
		       "Undefined opcode(LowerOp:%08lx)\n", iL);
	      exit (1);
	    }
	  break;
	case 13:		/* IAND, IOR */
	  switch (get_bc (iL))
	    {
	    case 0:		/* IAND */
	      if (indebug ("inst_trace2"))
		{
		  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
			   "\tIAND VI%02d = %08x & %08x = %08x\n",
			   get_fd (iL),
			   VI[get_fs (iL)],
			   VI[get_ft (iL)],
			   VI[get_fs (iL)] & VI[get_ft (iL)]
		    );
		}
	      if (hazardcheckVI (me, get_fs (iL), get_ft (iL), 3))
		{
		  pipe[0][0].flag = P_EMPTY;
		  apipe.flag = 0;
		  return 1;
		}
	      ipipe.no = get_fd (iL);
	      ipipe.flag = 1;
	      ipipe.vi = VI[get_fs (iL)] & VI[get_ft (iL)];
	      ipipe.code = I_IAND;
	      ipipe.code_addr = opc;
	      break;
	    case 1:		/* IOR */
	      if (hazardcheckVI (me, get_fs (iL), get_ft (iL), 3))
		{
		  pipe[0][0].flag = P_EMPTY;
		  apipe.flag = 0;
		  return 1;
		}
	      ipipe.no = get_fd (iL);
	      ipipe.flag = 1;
	      ipipe.vi = VI[get_fs (iL)] | VI[get_ft (iL)];
	      ipipe.code = I_IOR;
	      ipipe.code_addr = opc;
	      break;
	    }
	  break;
	case 15:
	  if (Lower_special (me, iL))
	    return 1;
	  break;
	default:
	  fprintf (stderr, "Undefined opcode(%08lx)\n", iL);
	  exit (1);
	}
      break;
    default:
      fprintf (stderr, "Undefined opcode(%08lx)\n", iL);
      exit (1);
    }

  /* Now that we know if the current instruction is a branch
     or a jump (jflag), we can adjust whether we stop after this
     insn or next. */

  /* If this insn is a jump and had [D] or [T], add a delay slot. */
  if (jflag == 1 && (dflag || tflag))
    peflag = 1;
  /* If previous insn had [E] and this insn is a jump, add a delay slot. */
  if (eflag == -1 && jflag == 1)
    peflag = 1;
#if 0
  if (indebug ("inst_trace"))
    {
      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
	       " j=%d pe=%d\n", jflag, peflag);
    }
#endif

  if (eflag == 1)
    {
      eflag = -1;
      peflag = 1;
    }

  return 2;
}

static int
fetch_inst (vu_device * me)
{
/*
   [name]               fetch_inst
   [desc.]              instruction fetchs on Micro memory
   [args]               void
   [return]     0: finished 
   1: fetched
 */
  int rc = 0;

  /* macro-instruction injected? */
  if (mflag == 1)
    {
      mflag = 0;
      return 1;
    }

  if (peflag != -1)
    {
      if (sflag != 1)
	{
	  opc = pc;
	  instbuf[0] = T2H_I4 (uMEM (pc, MEM_UPPER));
	  instbuf[1] = T2H_I4 (uMEM (pc, MEM_LOWER));

	  if (jflag == 1)
	    {
	      pc = jaddr;
	      jflag = 0;
	    }
	  else
	    {
	      pc++;
	    }
	}
      rc = 1;
#if 0
      {
	printf ("fetch_inst: returns %d, opc=0x%08x pc=0x%08x peflag=%d\n",
		rc, opc, pc, peflag);
      }
#endif
    }
  return rc;
}



/* move pipeline stage */
static int
move_pipe (vu_device * me)
{
  /*
   [name]               move_pipe
   [desc.]              move value to next pipeline stage.
   [args]               void
   [return]     0: no value exist in all pipeline stage
   1: some values exist in pipeline stage
 */

  int i, j, move = 0;
  int mask;

  /* Advance pipeline for ACC register.  ACC pipeline is a bit of a fake.
     It's modelled as having a length of 1, which is not "accurate", but
     it produces the right results. */

  if (apipe.flag != 0)
    {
      if (apipe.mask & DEST_X)
	{
	  ACC[0] = apipe.acc[0];
          ACC_OFLW[0] = apipe.acc_oflw[0];
	}
      if (apipe.mask & DEST_Y)
	{
	  ACC[1] = apipe.acc[1];
          ACC_OFLW[1] = apipe.acc_oflw[1];
	}
      if (apipe.mask & DEST_Z)
	{
	  ACC[2] = apipe.acc[2];
          ACC_OFLW[2] = apipe.acc_oflw[2];
	}
      if (apipe.mask & DEST_W)
	{
	  ACC[3] = apipe.acc[3];
          ACC_OFLW[3] = apipe.acc_oflw[3];
	}
      if (indebug ("pipe"))
	{
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		   "    apipe: ACC.%x=<%08x,%08x,%08x,%08x>=(%f,%f,%f,%f): o(%d,%d,%d,%d)\n",
		   apipe.mask,
		   ACC[0].i, ACC[1].i, ACC[2].i, ACC[3].i,
		   ACC[0].f, ACC[1].f, ACC[2].f, ACC[3].f,
		   ACC_OFLW[0], ACC_OFLW[1], ACC_OFLW[2], ACC_OFLW[3]);
	}
      apipe.flag = 0;
    }

  /* VI pipeline has length of 1. */
  if (ipipe.flag == 0)
    {
      ipipe.old_reg = -1;
    }
  else
    {
      if (ipipe.flag == 1)
	ipipe.old_reg = -1;
      else
	{
	  ipipe.old_reg = ipipe.no;
	  ipipe.old_value = VI[ipipe.no];
	}
      ipipe.flag = 0;
      VI[ipipe.no] = ipipe.vi;
      
      if (indebug ("pipe"))
	{
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		   "     ipipe: VI[%d] = %d %s\n",
		   ipipe.no, ipipe.vi,
		   (ipipe.no == 0 ? "(suppressed)" : ""));
	}
    }

  if (indebug ("pipe"))
    VU_CHECK_OPEN_DEBUG (me);

  /* Lower Instruction Pipe */
  switch (pipe[3][1].flag)
    {
    case P_EMPTY:
      break;
    case P_VF_REG:
      mask = pipe[3][1].mask;
      if (pipe[3][1].no > 31)
        {
          if (mask & DEST_X)
            {
	      VI[(pipe[3][1].no) & 0x1f] = pipe[3][1].vf[0].i & 0xffff;
       	    }
          if (mask & DEST_Y)
       	    {
    	      VI[(pipe[3][1].no) & 0x1f] = pipe[3][1].vf[1].i & 0xffff;
       	    }
          if (mask & DEST_Z)
       	    {
    	      VI[(pipe[3][1].no) & 0x1f] = pipe[3][1].vf[2].i & 0xffff;
       	    }
          if (mask & DEST_W)
       	    {
    	      VI[(pipe[3][1].no) & 0x1f] = pipe[3][1].vf[3].i & 0xffff;
       	    }
          if (indebug ("pipe"))
       	    {
    	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
    	               "    pipe[3][1] case P_VF_REG: VI%02d.%x=<%ld,%ld,%ld,%ld>%s\n"
    	               "    pipe[3][1] MAC/status flags=%04x/%04x\n",
    		       pipe[3][1].no & 0x1f,
    		       pipe[3][1].mask,
    		       pipe[3][1].vf[0].i & 0xffff,
    		       pipe[3][1].vf[1].i & 0xffff,
    		       pipe[3][1].vf[2].i & 0xffff,
    		       pipe[3][1].vf[3].i & 0xffff,
    		       (pipe[3][1].no == 0 ? " suppressed" : ""),
                       MACflag, statusflag);
       	    }
        }
      else
        {
          if (indebug ("pipe"))
       	    {
    	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
    	               "    pipe[3][1] case P_VF_REG: VF%02d.%x = <%08x,%08x,%08x,%08x> = <%f,%f,%f,%f> %s\n"
    	               "    pipe[3][1] MAC/status flags=%04x/%04x\n",
    		       pipe[3][1].no,
    		       pipe[3][1].mask,
    		       pipe[3][1].vf[0].i, pipe[3][1].vf[1].i,
		       pipe[3][1].vf[2].i, pipe[3][1].vf[3].i,
    		       pipe[3][1].vf[0].f, pipe[3][1].vf[1].f,
		       pipe[3][1].vf[2].f, pipe[3][1].vf[3].f,
    		       (pipe[3][1].no == 0 ? " suppressed" : ""),
                       MACflag, statusflag);
       	    }

          if (mask & DEST_X)
       	    {
    	      VF[pipe[3][1].no][0] = pipe[3][1].vf[0];
       	    }
          if (mask & DEST_Y)
       	    {
    	      VF[pipe[3][1].no][1] = pipe[3][1].vf[1];
       	    }
          if (mask & DEST_Z)
       	    {
    	      VF[pipe[3][1].no][2] = pipe[3][1].vf[2];
       	    }
          if (mask & DEST_W)
       	    {
    	      VF[pipe[3][1].no][3] = pipe[3][1].vf[3];
       	    }
        }
      pipe[3][1].flag = P_EMPTY;
      break;
    case P_MEMORY:
      {
	/*
	  printf("Lower Pipe -> Mem: %04d.%x = %08x %08x %08x %08x\n", 
	  pipe[3][1].addr, 
	  pipe[3][1].mask, 
	  f_2_i(pipe[3][1].vf[0]),
	  f_2_i(pipe[3][1].vf[1]), 
	  f_2_i(pipe[3][1].vf[2]), 
	  f_2_i(pipe[3][1].vf[3])); */

        int addr = pipe[3][1].addr;
        floatie *pipe_vf = &pipe[3][1].vf[0];

        mask = pipe[3][1].mask;
        if (pipe[3][1].code == I_ISW
	    || pipe[3][1].code == I_ISWR)
          {
    	    /* Handle stores to vu1 regs from vu0.
    	       The test is organized to handle the common case first.  */
    	    if (me->config.vu_number == 1
    	        || addr < VU0_VU1_REG_START)
     	      {
    	        if (mask & DEST_X)
    	          MEM (addr, 0) = H2T_I4 (pipe_vf[0].i & 0xffff);
    	        if (mask & DEST_Y)
    	          MEM (addr, 1) = H2T_I4 (pipe_vf[1].i & 0xffff);
    	        if (mask & DEST_Z)
    	          MEM (addr, 2) = H2T_I4 (pipe_vf[2].i & 0xffff);
    	        if (mask & DEST_W)
    	          MEM (addr, 3) = H2T_I4 (pipe_vf[3].i & 0xffff);
     	      }
    	    else if (me->config.vu_number == 0
		     && addr >= VU0_VU1_INTREG_START
		     && addr < (VU0_VU1_INTREG_START + VU0_VU1_INTREG_SIZE))
     	      {
    	        int vu1_regno = addr - VU0_VU1_INTREG_START;
    	        short *vi = get_VI (&vu1_device, vu1_regno);

    	        /* Only stores to the X section of the register are useful.
    	           See VU Spec 2.10, page 7-8.  */
    	        if (mask & DEST_X)
    	          *vi = (pipe_vf[0].i);
     	      }
            else if (me->config.vu_number == 0
                     && addr >= VU0_VU1_CTRLREG_START
                     && addr < (VU0_VU1_CTRLREG_START + VU0_VU1_CTRLREG_SIZE))
              {
                /* Only stores to the X section of the register are useful.
                   See VU Spec 2.10, page 7-9.  */
                if (mask & DEST_X)
                  set_ctrlreg (&vu1_device, addr, pipe_vf[0].i);
              }
          }
        else
          {
    	    /* Handle stores to vu1 regs from vu0.
    	       The test is organized to handle the common case first.  */
    	    if (me->config.vu_number == 1
    	        || addr < VU0_VU1_FPREG_START)
     	      {
    	        if (mask & DEST_X)
    	          MEM (addr, 0) = H2T_I4 (pipe_vf[0].i);
    	        if (mask & DEST_Y)
    	          MEM (addr, 1) = H2T_I4 (pipe_vf[1].i);
    	        if (mask & DEST_Z)
    	          MEM (addr, 2) = H2T_I4 (pipe_vf[2].i);
    	        if (mask & DEST_W)
    	          MEM (addr, 3) = H2T_I4 (pipe_vf[3].i);
     	      }
    	    /* Testing vu_number again isn't technically necessary.
    	       Sue me.  */
    	    else if (me->config.vu_number == 0
    		     && addr >= VU0_VU1_FPREG_START
    	             && addr < (VU0_VU1_FPREG_START + VU0_VU1_FPREG_SIZE))
     	      {
    	        int vu1_regno = addr - VU0_VU1_FPREG_START;
    	        floatie *vf = get_VF (&vu1_device, vu1_regno);

    	        if (mask & DEST_X)
    	          vf[0] = pipe_vf[0];
    	        if (mask & DEST_Y)
    	          vf[1] = pipe_vf[1];
    	        if (mask & DEST_Z)
    	          vf[2] = pipe_vf[2];
    	        if (mask & DEST_W)
    	          vf[3] = pipe_vf[3];
     	      }
            else if (me->config.vu_number == 0
                     && addr >= VU0_VU1_CTRLREG_START
                     && addr < (VU0_VU1_CTRLREG_START + VU0_VU1_CTRLREG_SIZE))
              {
                /* Only stores to the X section of the register are useful.
                   See VU Spec 2.10, page 7-9.  */
                if (mask & DEST_X)
                  set_ctrlreg (&vu1_device, addr, pipe_vf[0].i);
              }
          }
        pipe[3][1].flag = P_EMPTY;
        break;
      }
    case P_I_REG:		/* move to I-reg */
      ASSERT (0);		/* Never happens */
      /*    I = pipe[3][1].vf[0]; */
      break;
    case P_MFP:
      if (indebug ("pipe"))
	{
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		   "    pipe[3][1] case P_MFP: VF%02d.%x = <%08x,%08x,%08x,%08x> = <%f,%f,%f,%f> %s\n"
		   "    pipe[3][1] MAC/status flags=%04x/%04x\n",
		   pipe[3][1].no,
		   pipe[3][1].mask,
		   pipe[3][1].vf[0].i, pipe[3][1].vf[1].i,
		   pipe[3][1].vf[2].i, pipe[3][1].vf[3].i,
		   pipe[3][1].vf[0].f, pipe[3][1].vf[1].f,
		   pipe[3][1].vf[2].f, pipe[3][1].vf[3].f,
		   (pipe[3][1].no == 0 ? " suppressed" : ""),
		   MACflag, statusflag);
	}

      mask = pipe[3][1].mask;
      if (mask & DEST_X)
	{
	  VF[pipe[3][1].no][0] = pipe[3][1].vf[0];
	}
      if (mask & DEST_Y)
	{
	  VF[pipe[3][1].no][1] = pipe[3][1].vf[1];
	}
      if (mask & DEST_Z)
	{
	  VF[pipe[3][1].no][2] = pipe[3][1].vf[2];
	}
      if (mask & DEST_W)
	{
	  VF[pipe[3][1].no][3] = pipe[3][1].vf[3];
	}
      pipe[3][1].flag = P_EMPTY;
      break;
    default:
      break;
    }
  
  /* Upper Instruction Pipe */
  switch (pipe[3][0].flag)
    {
    case P_EMPTY:
      break;
    case P_VF_REG:		/* move to VF-reg */
      mask = pipe[3][0].mask;
      if (mask & DEST_X)
	{
	  VF[pipe[3][0].no][0] = pipe[3][0].vf[0];
	}
      if (mask & DEST_Y)
	{
	  VF[pipe[3][0].no][1] = pipe[3][0].vf[1];
	}
      if (mask & DEST_Z)
	{
	  VF[pipe[3][0].no][2] = pipe[3][0].vf[2];
	}
      if (mask & DEST_W)
	{
	  VF[pipe[3][0].no][3] = pipe[3][0].vf[3];
	}
      statusflag = (statusflag & 0x00000fe0) |
	(pipe[3][0].status & 0xfff) |
	((pipe[3][0].status & 0x3f) << 6);
      MACflag = (pipe[3][0].status >> 16) & 0xffff;
      
      if (indebug ("pipe"))
	{
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		   "    pipe[3][0] case P_VF_REG: VF%02d.%x = <%08x,%08x,%08x,%08x> = <%f,%f,%f,%f> %s\n"
		   "    pipe[3][0] MAC/status flags=%04x/%04x\n",
		   pipe[3][0].no,
		   pipe[3][0].mask,
		   pipe[3][0].vf[0].i, pipe[3][0].vf[1].i,
		   pipe[3][0].vf[2].i, pipe[3][0].vf[3].i,
		   pipe[3][0].vf[0].f, pipe[3][0].vf[1].f,
		   pipe[3][0].vf[2].f, pipe[3][0].vf[3].f,
		   (pipe[3][0].no == 0 ? " suppressed" : ""),
		   MACflag, statusflag);
	}
      break;
    case P_MEMORY:		/* store from VF-reg */
      /*
	printf("TO MEMORY: %08x.%x = %08x %08x %08x %08x\n", 
	pipe[3][0].addr, 
	pipe[3][0].mask, 
	pipe[3][0].vf[0],
	pipe[3][0].vf[1], 
	pipe[3][0].vf[2], 
	pipe[3][0].vf[3]); */
      mask = pipe[3][0].mask;
      if (mask & DEST_X)
	{
	  MEM (pipe[3][0].addr, 0) = H2T_4 (pipe[3][0].vf[0].i);
	}
      if (mask & DEST_Y)
	{
	  MEM (pipe[3][0].addr, 1) = H2T_4 (pipe[3][0].vf[1].i);
	}
      if (mask & DEST_Z)
	{
	  MEM (pipe[3][0].addr, 2) = H2T_4 (pipe[3][0].vf[2].i);
	}
      if (mask & DEST_W)
	{
	  MEM (pipe[3][0].addr, 3) = H2T_4 (pipe[3][0].vf[3].i);
	}
      break;
    case P_I_REG:
      ASSERT (0);		/* Never happens, replaced by Ipipe */
      break;
    case P_STATUS_REG:	/* status only */
      statusflag = (statusflag & 0x00000fe0) |
	(pipe[3][0].status & 0xfff) |
	((pipe[3][0].status & 0x3f) << 6);
      MACflag = (pipe[3][0].status >> 16) & 0xffff;
      if (indebug ("pipe"))
	{
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		   "    pipe[3][0] opcd=0x%x MAC/status flags=%04x/%04x\n",
		   pipe[3][0].code, MACflag, statusflag);
	}
      break;
    case P_CLIP_REG:
      clipflag = ((clipflag << 6) & 0x00ffffc0) |
	(pipe[3][0].status & 0x3f);
      break;
      
    case P_VF_REG_NO_STATUS:	/* move to VF-reg, n/c on status */
      if (indebug ("pipe"))
	{
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		   "    pipe[3][0] case P_VF_REG_NO_STATUS: VF%02d.%x = <%08x,%08x,%08x,%08x> = <%f,%f,%f,%f> %s\n",
		   pipe[3][0].no,
		   pipe[3][0].mask,
		   pipe[3][0].vf[0].i, pipe[3][0].vf[1].i,
		   pipe[3][0].vf[2].i, pipe[3][0].vf[3].i,
		   pipe[3][0].vf[0].f, pipe[3][0].vf[1].f,
		   pipe[3][0].vf[2].f, pipe[3][0].vf[3].f,
		   (pipe[3][0].no == 0 ? " suppressed" : ""));
	}
      mask = pipe[3][0].mask;
      if (mask & DEST_X)
	{
	  VF[pipe[3][0].no][0] = pipe[3][0].vf[0];
	}
      if (mask & DEST_Y)
	{
	  VF[pipe[3][0].no][1] = pipe[3][0].vf[1];
	}
      if (mask & DEST_Z)
	{
	  VF[pipe[3][0].no][2] = pipe[3][0].vf[2];
	}
      if (mask & DEST_W)
	{
	  VF[pipe[3][0].no][3] = pipe[3][0].vf[3];
	}
      break;
      
    default:
      ASSERT (0);		/* Should never get here. */
      break;
    }
  pipe[3][0].flag = P_EMPTY;
  
  
  /* Handle update for Divide Unit and Q register */
  if (qpipe.no != 0)
    {
      if (qpipe.no == 1)
	{
	  qpipe.no = 0;
	  Q = qpipe.vf;
	  statusflag = (statusflag & 0xfcf) |
	    (qpipe.status << 6) | qpipe.status;
	  if (indebug ("pipe"))
	    {
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "    qpipe: Q = %f, status=%x\n", qpipe.vf.f, statusflag);
	    }
	}
      else
	{
	  qpipe.no--;
	  move = 1;
	}
    }
  
  /* Lower Instruction Pipe -- some insns must be done after the Upper Pipe */
  switch (pipe[3][1].flag)
    {
    case P_EMPTY:
      break;
    case P_STATUS_REG:
      ASSERT (pipe[3][1].code == I_FSSET);
      statusflag &= ~0x00000fc0;
      statusflag |= pipe[3][1].status & 0x00000fc0;
      
      if (indebug ("pipe"))
	{
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		   "    pipe[3][1] FSSET: MAC/status flags=%04x/%04x\n",
		   MACflag, statusflag);
	}
      break;
    case P_CLIP_REG:
      ASSERT (pipe[3][1].code == I_FCSET);
      clipflag = pipe[3][1].status & 0x00ffFfff;
      break;
    default:
      break;
    }
  pipe[3][1].flag = P_EMPTY;
  
  
  /* propagate pipe stage data */
  for (i = 2; i >= 0; i--)
    {
      if (pipe[i][0].flag != 0)
	{
	  move = 1;
	  pipe[i + 1][0] = pipe[i][0];
	  pipe[i][0].status = 0;
	  pipe[i][0].flag = P_EMPTY;
	  
	}
      if (pipe[i][1].flag != 0)
	{
	  move = 1;
	  pipe[i + 1][1] = pipe[i][1];
	  pipe[i][1].status = 0;
	  pipe[i][1].flag = P_EMPTY;
	}
    }
  
  /* S pipeline has length of 1 */
  if (spipe.no != 0)
    {
      if (spipe.no == 1)
	{
	  spipe.no = 0;
	  VN[4] = spipe.vn;
	  if (indebug ("pipe"))
	    {
	      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		       "    spipe: P = %08x = %f\n", spipe.vn.i, spipe.vn.f);
	    }
	}
      else
	{
	  spipe.no--;
	}
    }
  
  /* I pipeline has length of 1 */
  if (Ipipe.flag)
    {
      I = Ipipe.val;
      Ipipe.flag = 0;
      if (indebug ("pipe"))
	{
	  fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
		   "    Ipipe: I = %08x = %f\n", Ipipe.val.i, Ipipe.val.f);
	}
    }
  
  VF[0][0].f = 0.0;
  VF[0][1].f = 0.0; /* was: 0.5 */
  VF[0][2].f = 0.0; /* was: -1.0 */
  VF[0][3].f = 1.0;
  VI[0] = 0;
  
  return move;
}


void
vpecallms_init (vu_device * me)
{
/*
   [name]               vpecallms_init
   [desc.]              Initialize internal status of VU simulator 
   when start to execute a Micro program.
   This routine doesn't use with simvpe().
   This routine use with vpecallms_cycle() and initvpe().
   [args]               void
   [return]     void
 */

  eflag = 0;
  jflag = 0;
  peflag = 0;
  sflag = 0;
  intr_mode = _is_dbg;
  verb_mode = indebug ("verbose");	/* _is_verb; */
  dpr_mode = _is_dump;
}

void
vpecallms_cycle (SIM_DESC sd, vu_device * me)
{
/*
   [name]               vpecallms_cycle
   [desc.]              VU simulator execute 1cycle.
   [args]               void
   [return]     !0: VU is busy
   0: VU is idle
 */
  int stat = 0;

  if (((intr_mode == 1) && (pc == bp)) ||
      ((intr_mode == 3) && (pc == bp)) ||
      ((intr_mode == 4) && (pc == bp)) ||
      (intr_mode == 2))
    {
      interactive (sd, me);
    }

  /* XXX: note that in interactive mode, exec comes before fetch.  Why? */
  if (fetch_inst (me))
    stat += 1;

  if (instbuf[0] & (TXVU_VU_BRK_MASK << 24))
    {
      sim_cpu *cpu = STATE_CPU (sd, 0);

      pc = opc;			/* Back-up the pc */

      sim_engine_halt (sd, cpu, NULL, pc, sim_stopped, SIM_SIGTRAP);
    }

  if (stat && exec_inst (me))
    stat += 2;
  if (move_pipe (me))
    stat += 4;

  if (stat)
    {
      me->run_state = VU_RUN;	/* BUSY */
    }
  else
    {
      me->run_state = VU_READY;	/* NOT BUSY */
    }
}

/*------------------------ GPUIF Interface -----------------------------*/

/****************************************************************************/
/*                                                                          */
/*             Sony Computer Entertainment CONFIDENTIAL                     */
/*      (C) 1997 Sony Computer Entertainment Inc. All Rights Reserved       */
/*                                                                          */
/*      Psuedo GPUIF simulator (handle only PATH1)                          */
/*                                                                          */
/****************************************************************************/

#define PRIM    0x00
#define RGBAQ   0x01
#define ST              0x02
#define UV              0x03
#define XYZF2   0x04
#define XYZ2    0x05
#define TEX0_1  0x06
#define TEX0_2  0x07
#define CLAMP_1 0x08
#define CLAMP_2 0x09
#define XYZF    0x0a
#define XYZF3   0x0c
#define XYZ3    0x0d
#define A_D             0x0e
#define GPUNOP  0x0f

#define NOOUT   0
#define PRTGPUIF        1
#define PRTGPU  2
#define PRTGPI  3
#define MEMGPUIF        4

static u_long stQ;

static void
GPU2_Put_gpu (vu_device * me, int addr, u_long reg1, u_long reg0)
{
  /* float        xx, yy; -=UNUSED=- */

  fprintf (_fp_gpu, "%02x %08lx %08lx\n", addr, reg1, reg0);
}

static void
GPU2_Put_gpi (vu_device * me, int addr, u_long reg1, u_long reg0)
{
  floatie xx, yy;
  /* int ctxt, fst, aa1, abe, fge, tme, iip, prim; -=UNUSED */
  floatie sixteen, r0h, r0l;

  sixteen.f = 16.0;
  r0l.f = (reg0 & 0xffff);
  r0h.f = (reg0 >> 16) & 0xffff;

  switch (addr)
    {
    case PRIM:
      fprintf (_fp_gpu, "PRIM PRIM=%ld ", (reg0 & 0x7));
      if (reg0 & 0x200)
	fprintf (_fp_gpu, "CTXT=1 ");
      if (reg0 & 0x100)
	fprintf (_fp_gpu, "FST=1 ");
      if (reg0 & 0x80)
	fprintf (_fp_gpu, "AA1=1 ");
      if (reg0 & 0x40)
	fprintf (_fp_gpu, "ABE=1 ");
      if (reg0 & 0x20)
	fprintf (_fp_gpu, "FGE=1 ");
      if (reg0 & 0x10)
	fprintf (_fp_gpu, "TME=1 ");
      if (reg0 & 0x8)
	fprintf (_fp_gpu, "IIP=1 ");
      fprintf (_fp_gpu, "\n");
      break;
    case RGBAQ:
      xx.i = reg1;
      fprintf (_fp_gpu, "RGBAQ %ld %ld %ld %ld %e\n", reg0 & 0xff, (reg0 >> 8) & 0xff,
	       (reg0 >> 16) & 0xff, (reg0 >> 24) & 0xff, xx.f);
      break;
    case ST:
      xx.i = reg0;
      yy.i = reg1;
      fprintf (_fp_gpu, "ST %e %e\n", xx.f, yy.f);
      break;
    case UV:
      xx = FDiv (r0l, sixteen);
      yy = FDiv (r0h, sixteen);
      fprintf (_fp_gpu, "UV %f %f\n", xx.f, yy.f);
      break;
    case XYZF2:
      xx = FDiv (r0l, sixteen);
      yy = FDiv (r0h, sixteen);
      fprintf (_fp_gpu, "XYZF2 %f %f %ld %ld\n", xx.f, yy.f,
	       reg1 & 0xffffff, (reg1 >> 24) & 0xff);
      break;
    case XYZ2:
      xx = FDiv (r0l, sixteen);
      yy = FDiv (r0h, sixteen);
      fprintf (_fp_gpu, "XYZ2 %f %f %ld \n", xx.f, yy.f, reg1);
      break;
    case XYZF:
      xx = FDiv (r0l, sixteen);
      yy = FDiv (r0h, sixteen);
      fprintf (_fp_gpu, "XYZF %f %f %ld %ld\n", xx.f, yy.f,
	       reg1 & 0xffffff, (reg1 >> 24) & 0xff);
      break;
    case XYZF3:
      xx = FDiv (r0l, sixteen);
      yy = FDiv (r0h, sixteen);
      fprintf (_fp_gpu, "XYZF3 %f %f %ld %ld\n", xx.f, yy.f,
	       reg1 & 0xffffff, (reg1 >> 24) & 0xff);
      break;
    case XYZ3:
      xx = FDiv (r0l, sixteen);
      yy = FDiv (r0h, sixteen);
      fprintf (_fp_gpu, "XYZ3 %f %f %ld\n", xx.f, yy.f, reg1);
      break;
    case A_D:
      fprintf (_fp_gpu, "!%02x %08lx %08lx\n", addr, reg1, reg0);
      break;
    case GPUNOP:
      break;
    default:
      fprintf (_fp_gpu, "!%02x %08lx %08lx\n", addr, reg1, reg0);
      break;
    }
}

static void
to_gpu (vu_device * me, int regno, int addr)
{
  u_long reg0, reg1;
  u_long mem0, mem1, mem2, mem3;

  mem0 = T2H_4 (MEM (addr, 0));
  mem1 = T2H_4 (MEM (addr, 1));
  mem2 = T2H_4 (MEM (addr, 2));
  mem3 = T2H_4 (MEM (addr, 3));

  switch (regno)
    {
    case RGBAQ:
      reg0 = (mem0 & 0xff) | ((mem1 & 0xff) << 8) |
	((mem2 & 0xff) << 16) | ((mem3 & 0xff) << 24);
      switch (_pgpuif)
	{
	case NOOUT:
	  break;
	case PRTGPUIF:
	case MEMGPUIF:
	  break;
	case PRTGPU:
	  GPU2_Put_gpu (me, RGBAQ, stQ, reg0);
	  break;
	case PRTGPI:
	  GPU2_Put_gpi (me, RGBAQ, stQ, reg0);
	  break;
	}
      break;
    case UV:
      reg0 = (mem0 & 0xffff) | ((mem1 & 0xffff) << 16);
      switch (_pgpuif)
	{
	case NOOUT:
	  break;
	case PRTGPUIF:
	case MEMGPUIF:
	  break;
	case PRTGPU:
	  GPU2_Put_gpu (me, UV, 0, reg0);
	  break;
	case PRTGPI:
	  GPU2_Put_gpi (me, UV, 0, reg0);
	  break;
	}
      break;
    case XYZF2:
    case XYZF3:
      reg0 = (mem0 & 0xffff) | ((mem1 & 0xffff) << 16);
      reg1 = ((mem2 >> 4) & 0xffffff) |
	(((mem3 >> 4) & 0xff) << 24);
      switch (_pgpuif)
	{
	case NOOUT:
	  break;
	case PRTGPUIF:
	case MEMGPUIF:
	  break;
	case PRTGPU:
	  if (mem3 & 0x8000)
	    GPU2_Put_gpu (me, XYZF3, reg1, reg0);
	  else
	    GPU2_Put_gpu (me, XYZF2, reg1, reg0);
	  break;
	case PRTGPI:
	  if (mem3 & 0x8000)
	    GPU2_Put_gpi (me, XYZF3, reg1, reg0);
	  else
	    GPU2_Put_gpi (me, XYZF2, reg1, reg0);
	  break;
	}
      break;
    case ST:
      stQ = mem2;
    case PRIM:
    case TEX0_1:
    case TEX0_2:
    case CLAMP_1:
    case CLAMP_2:
      switch (_pgpuif)
	{
	case NOOUT:
	  break;
	case PRTGPUIF:
	case MEMGPUIF:
	  break;
	case PRTGPU:
	  GPU2_Put_gpu (me, regno, mem1, mem0);
	  break;
	case PRTGPI:
	  GPU2_Put_gpi (me, regno, mem1, mem0);
	  break;
	}
      break;
    case A_D:
      switch (_pgpuif)
	{
	case NOOUT:
	  break;
	case PRTGPUIF:
	case MEMGPUIF:
	  break;
	case PRTGPU:
	  GPU2_Put_gpu (me, mem2 & 0xff, mem1, mem0);
	  break;
	case PRTGPI:
	  GPU2_Put_gpi (me, mem2 & 0xff, mem1, mem0);
	  break;
	}
      break;
    }
}

static void
gif_write (vu_device * me, u_long mem0, u_long mem1, u_long mem2, u_long mem3)
{
  unsigned_16 data;
  sim_cpu *cpu;

  if (indebug ("gpuif"))
    {
      VU_CHECK_OPEN_DEBUG (me);
      fprintf ((me->debug_file != NULL) ? me->debug_file : stdout,
	    "To GIF --> %08lx %08lx %08lx %08lx\n", mem0, mem1, mem2, mem3);
    }
  *A4_16 (&data, 3) = mem0;
  *A4_16 (&data, 2) = mem1;
  *A4_16 (&data, 1) = mem2;
  *A4_16 (&data, 0) = mem3;

  cpu = STATE_CPU (CURRENT_STATE, 0);
  sim_core_write_aligned_16 (
			      cpu,
			      CIA_GET (cpu), write_map, GIF_PATH1_FIFO_ADDR,
			      data);
}


static void
GpuIfKick (vu_device * me, int addr)
{
  int nreg, flg, prim, pre, nloop;
  int regs[16], i, j;
  u_long tag0;
  u_long mem0, mem1, mem2, mem3;

  if (_GIF_SIM_OFF)
    {
      if (_GIF_BUSY)
	{
	  sflag = 1;
	  hc_count++;
	  pipe[0][0].flag = P_EMPTY;
	  apipe.flag = 0;
	}
      else
	{
	  _GIF_VUCALL = 1;
	  _GIF_VUADDR = addr;
	  _GIF_BUSY = 1;
	  sflag = 0;
	}
    }
  else
    {
      do
	{

	  mem0 = T2H_4 (MEM (addr, 0));
	  mem1 = T2H_4 (MEM (addr, 1));
	  mem2 = T2H_4 (MEM (addr, 2));
	  mem3 = T2H_4 (MEM (addr, 3));

	  tag0 = mem0;
	  nreg = (mem1 >> 28) & 0xf;
	  flg = (mem1 >> 26) & 0x3;
	  prim = (mem1 >> 15) & 0x7ff;
	  pre = (mem1 >> 14) & 0x1;
	  nloop = mem0 & 0x7fff;

	  if (nreg == 0)
	    nreg = 16;

	  if (flg != 0)
	    {
	      fprintf (stderr, "GIF: not support FLG(%d)<%d>\n", flg, addr);
	      return;
	    }
	  for (i = 0; i < 8; i++)
	    regs[i] = (mem2 >> (i * 4)) & 0xf;

	  for (i = 8; i < 16; i++)
	    regs[i] = (mem3 >> ((i - 8) * 4)) & 0xf;

	  if (pre)
	    {
	      switch (_pgpuif)
		{
		case NOOUT:
		case PRTGPUIF:
		case MEMGPUIF:
		  break;
		case PRTGPU:
		  GPU2_Put_gpu (me, PRIM, 0, (u_long) prim);
		  break;
		case PRTGPI:
		  GPU2_Put_gpi (me, PRIM, 0, (u_long) prim);
		  break;
		}
	    }
	  if (_pgpuif == PRTGPUIF)
	    fprintf (_fp_gpu, "%08lx %08lx %08lx %08lx\n", mem3, mem2, mem1, mem0);

	  else if (_pgpuif == MEMGPUIF)
	    gif_write (me, mem0, mem1, mem2, mem3);

	  addr++;

	  for (i = 0; i < nloop; i++)
	    {
	      for (j = 0; j < nreg; j++)
		{

		  if (_pgpuif == PRTGPUIF)
		    {
		      mem0 = T2H_4 (MEM (addr, 0));
		      mem1 = T2H_4 (MEM (addr, 1));
		      mem2 = T2H_4 (MEM (addr, 2));
		      mem3 = T2H_4 (MEM (addr, 3));

		      fprintf (_fp_gpu, "%08lx %08lx %08lx %08lx\n", mem3, mem2, mem1, mem0);
		    }
		  else if (_pgpuif == MEMGPUIF)
		    {
		      mem0 = T2H_4 (MEM (addr, 0));
		      mem1 = T2H_4 (MEM (addr, 1));
		      mem2 = T2H_4 (MEM (addr, 2));
		      mem3 = T2H_4 (MEM (addr, 3));

		      gif_write (me, mem0, mem1, mem2, mem3);
		    }
		  else
		    {
		      to_gpu (me, regs[j], addr);
		    }
		  addr++;
		}
	    }
	}
      while ((tag0 & 0x8000) == 0);
    }

}
