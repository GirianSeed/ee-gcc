/*****************************************
 * This file will eventually disappear.  It's
 * temporarily around to handle vu debugging
 * only until gcc support arrives.
 *
 * It will go away once gcc is here.
 */

#include "stdio.h"
#include "sky-interact.h"
#include "sky-vudis.h"
#include "sky-gdb.h"

static float
i_2_f (int x)
{
  return *(float *) &x;
}

static int
f_2_i (float x)
{
  return *(int *) &x;
}

#define H2T_I4(x)       ((long)(H2T_4((x))))
#define T2H_I4(x)       ((long)(T2H_4((x))))
#define T2H_F4(x)       (i_2_f(T2H_4((x))))

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

#define	_pgpuif 	me->junk._pgpuif

#define	_is_dbg 	me->junk._is_dbg
#define	_is_verb 	me->junk._is_verb
#define	_is_dump 	me->junk._is_dump
#define _fp_gpu		me->junk._fp_gpu

#define _GIF_SIM_OFF	me->junk._GIF_SIM_OFF
#define _GIF_BUSY	me->junk._GIF_BUSY
#define _GIF_VUCALL	me->junk._GIF_VUCALL
#define _GIF_VUADDR	me->junk._GIF_VUADDR

#define instbuf		me->junk.instbuf
#define pc		me->junk.pc
#define opc		me->junk.opc
#define jaddr		me->junk.jaddr

#ifdef ACC
#undef ACC
#endif

#define ACC		me->regs.acc
#define Q		me->regs.Q
#define I		me->regs.I
#define R		me->regs.R
#define VN		me->regs.VN
#define MACflag		me->regs.MACflag
#define statusflag	me->regs.statusflag
#define clipflag	me->regs.clipflag

#define eflag		me->junk.eflag
#define jflag		me->junk.jflag
#define peflag		me->junk.peflag
#define sflag		me->junk.sflag
#define lflag           me->junk.lflag
#define mflag           me->junk.mflag

#define bp		me->junk.bp
#define ecount		me->junk.ecount
#define intr_mode	me->junk.intr_mode
#define verb_mode	me->junk.verb_mode
#define dpr_mode	me->junk.dpr_mode
#define all_count	me->junk.all_count
#define hc_count	me->junk.hc_count

#define MEM_UPPER 1
#define MEM_LOWER 0

/* Must be kept in sync with sky-vpe.h */
static char*
vu_code_name[] = {
    "MOVE", "MR32", "LQI", "SQI", "LQD", "SQD", "MTIR", "MFIR", "ILWR", "ISWR",
    "RNEXT", "RGET", "MFP", "XTOP", "XITOP", "ADDA", "SUBA", "MADDA", "MSUBA",
    "ITOF0", "ITOF4", "ITOF12", "ITOF15", "FTOI0", "FTOI4", "FTOI12", "FTOI15",
    "MULA", "MULAq", "ABS", "MULAi", "CLIP", "ADDAq", "MADDAq", "ADDAi", 
    "MADDAi", "SUBAq", "MSUBAq", "SUBi", "MSUBAi", "OPMULA", "ADD", "SUB", 
    "MADD", "MSUB", "MAX", "MINI", "MUL", "MULq", "MAXi", "MULi", "MINIi", 
    "ADDq", "MADDq", "ADDi", "MADDi", "SUBq", "MSUBq", "SUBi", "MSUBi", 
    "OPMSUB", "LQ", "SQ", "ILW", "ISW", "IADDIU", "ISUBIU", "FCEQ",
    "FCSET", "FCAND", "FCOR", "FSEQ", "FSSET", "FSAND", "FSOR", "FMEQ", 
    "FMAND", "FMOR", "FCGET", "BAL", "JAL", "IADD", "ISUB", "IADDI", "IAND", 
    "IOR",

    "DIV", "SQRT", "RSQRT",

    "ESADD", "ERSADD", "ELENG", "ERLENG", "EATANxy", "EATANxz", "ESUM",
    "ESQRT", "ERSQRT", "ERCPR", "ESIN", "EATAN", "EEXP"
};

#define CODE_NAME(x) vu_code_name[(x)]

static void 
_fprintf(SIM_DESC sd, FILE* fp, const char *fmt, ...)
{
  va_list ap;
  char buf[256];

  va_start (ap, fmt);
  vsprintf (buf, fmt, ap);
  va_end(ap);

  if (fp)
    sim_io_write (sd, fileno(fp), buf, strlen (buf));
  else
    sim_io_write_stdout (sd, buf, strlen (buf));
}      	

static void 
print_mask(SIM_DESC sd, FILE *fp, int mask) {
  if (gdb_vu_pipeorder == ORDER_WZYX)
    {
      if (mask & DEST_W)
	_fprintf (sd, fp, "w");
      if (mask & DEST_Z)
	_fprintf (sd, fp, "z");
      if (mask & DEST_Y)
	_fprintf (sd, fp, "y");
      if (mask & DEST_X)
	_fprintf (sd, fp, "x");
      if (!(mask & DEST_W))
	_fprintf (sd, fp, " ");
      if (!(mask & DEST_Z))
	_fprintf (sd, fp, " ");
      if (!(mask & DEST_Y))
	_fprintf (sd, fp, " ");
      if (!(mask & DEST_X))
	_fprintf (sd, fp, " ");
    }
  else
    {
      if (mask & DEST_X)
	_fprintf (sd, fp, "x");
      if (mask & DEST_Y)
	_fprintf (sd, fp, "y");
      if (mask & DEST_Z)
	_fprintf (sd, fp, "z");
      if (mask & DEST_W)
	_fprintf (sd, fp, "w");
      if (!(mask & DEST_X))
	_fprintf (sd, fp, " ");
      if (!(mask & DEST_Y))
	_fprintf (sd, fp, " ");
      if (!(mask & DEST_Z))
	_fprintf (sd, fp, " ");
      if (!(mask & DEST_W))
	_fprintf (sd, fp, " ");
    }
}

static void
print_vector (SIM_DESC sd, FILE * fp, int mask, float f0, float f1, float f2, float f3)
{
  _fprintf (sd, fp, "<");
  if (gdb_vu_pipeorder == ORDER_WZYX)
    {
      _fprintf (sd, fp, "%f,", f3);
      _fprintf (sd, fp, "%f,", f2);
      _fprintf (sd, fp, "%f,", f1);
      _fprintf (sd, fp, "%f", f0);
    }
  else
    {
      _fprintf (sd, fp, "%f,", f0);
      _fprintf (sd, fp, "%f,", f1);
      _fprintf (sd, fp, "%f,", f2);
      _fprintf (sd, fp, "%f", f3);
    }
  _fprintf (sd, fp, ">\n");
}

void
print_pipe (SIM_DESC sd, FILE *fp, vu_device * me)
{
/*
   [name]               printpipe
   [desc.]              print out pipeline status to stdout. (for debug mode)
   [args]               void
   [return]     void
 */

  int i;
  int empty = 1;
  _fprintf (sd, fp, "Addr  Dly Insn  Tgt       Value\n");
  for (i = 3; i >= 0; i--)
    {
      int delay = 4 - i;
      switch (pipe[i][0].flag)
	{
	case P_EMPTY:
	  break;
	case P_VF_REG:
	case P_VF_REG_NO_STATUS:
	  _fprintf (sd, fp, "%5d ", pipe[i][0].code_addr);
	  _fprintf (sd, fp, "%2d ", delay);
	  _fprintf (sd, fp, "%-6s ", CODE_NAME (pipe[i][0].code));
	  _fprintf (sd, fp, "VF%02d.", pipe[i][0].no);
	  print_mask (sd, fp, pipe[i][0].mask);
	  _fprintf (sd, fp, " ");
	  print_vector (sd, fp, pipe[i][0].mask,
			pipe[i][0].vf[0].f, pipe[i][0].vf[1].f,
			pipe[i][0].vf[2].f, pipe[i][0].vf[3].f);
	  empty = 0;
	  break;
	case P_MEMORY:		/* store from reg */
	  _fprintf (sd, fp, "%5d ", pipe[i][0].code_addr);
	  _fprintf (sd, fp, "%2d ", delay);
	  _fprintf (sd, fp, "%-6s ", CODE_NAME (pipe[i][0].code));
	  _fprintf (sd, fp, "(%04d).", pipe[i][0].addr);
	  print_mask (sd, fp, pipe[i][0].mask);
	  _fprintf (sd, fp, " ");
	  print_vector (sd, fp, pipe[i][0].mask,
			pipe[i][0].vf[0].f, pipe[i][0].vf[1].f,
			pipe[i][0].vf[2].f, pipe[i][0].vf[3].f);
	  empty = 0;
	  break;
	case P_STATUS_REG:	/* only status */
	  _fprintf (sd, fp, "%5d ", pipe[i][0].code_addr);
	  _fprintf (sd, fp, "%2d ", delay);
	  _fprintf (sd, fp, "%-6s ", CODE_NAME (pipe[i][0].code));
	  _fprintf (sd, fp, "stat      ", pipe[i][0].addr);
	  _fprintf (sd, fp, "%08x\n", pipe[i][0].status);
	  empty = 0;
	  break;
	case P_CLIP_REG:	/* only clip */
	  _fprintf (sd, fp, "%5d ", pipe[i][0].code_addr);
	  _fprintf (sd, fp, "%2d ", delay);
	  _fprintf (sd, fp, "%-6s ", CODE_NAME (pipe[i][0].code));
	  _fprintf (sd, fp, "clip      ", pipe[i][0].addr);
	  _fprintf (sd, fp, "%08x\n", pipe[i][0].status);
	  empty = 0;
	  break;
	}
    }

  if (qpipe.no != 0)
    {
      _fprintf (sd, fp, "%5d ", qpipe.code_addr);
      _fprintf (sd, fp, "%2d ", qpipe.no);
      _fprintf (sd, fp, "%-6s ", CODE_NAME (qpipe.code));
      _fprintf (sd, fp, "Q         ");
      _fprintf (sd, fp, "%f\n", qpipe.vf);
      empty = 0;
    }

  for (i = 3; i >= 0; i--)
    {
      int delay = 4 - i;
      switch (pipe[i][1].flag)
	{
	case P_VF_REG:		/* move to reg */
	case P_MFP:		/* move from EFU */
	  if (pipe[i][1].no > 31)
	    {
	      _fprintf (sd, fp, "%5d ", pipe[i][1].code_addr);
	      _fprintf (sd, fp, "%2d ", delay);
	      _fprintf (sd, fp, "%-6s ", CODE_NAME (pipe[i][1].code));
	      _fprintf (sd, fp, "VI%02d     ", pipe[i][1].no - 32);
	      if (pipe[i][1].mask & DEST_X)
		_fprintf (sd, fp, " %d\n", *(long *) &pipe[i][1].vf[0] & 0xffff);
	      if (pipe[i][1].mask & DEST_Y)
		_fprintf (sd, fp, " %d\n", *(long *) &pipe[i][1].vf[1] & 0xffff);
	      if (pipe[i][1].mask & DEST_Z)
		_fprintf (sd, fp, " %d\n", *(long *) &pipe[i][1].vf[2] & 0xffff);
	      if (pipe[i][1].mask & DEST_W)
		_fprintf (sd, fp, " %d\n", *(long *) &pipe[i][1].vf[3] & 0xffff);
	    }
	  else
	    {
	      _fprintf (sd, fp, "%5d ", pipe[i][1].code_addr);
	      _fprintf (sd, fp, "%2d ", delay);
	      _fprintf (sd, fp, "%-6s ", CODE_NAME (pipe[i][1].code));
	      _fprintf (sd, fp, "VF%02d.", pipe[i][1].no);
	      print_mask (sd, fp, pipe[i][1].mask);
	      _fprintf (sd, fp, " ");
	      print_vector (sd, fp, pipe[i][1].mask,
			    pipe[i][1].vf[0].f, pipe[i][1].vf[1].f,
			    pipe[i][1].vf[2].f, pipe[i][1].vf[3].f);
	    }
	  empty = 0;
	  break;
	case P_MEMORY:		/* store from reg */
	  _fprintf (sd, fp, "%5d ", pipe[i][1].code_addr);
	  _fprintf (sd, fp, "%2d ", delay);
	  _fprintf (sd, fp, "%-6s ", CODE_NAME (pipe[i][1].code));
	  _fprintf (sd, fp, "(%04d).", pipe[i][1].addr);
	  print_mask (sd, fp, pipe[i][1].mask);
	  _fprintf (sd, fp, " ");
	  print_vector (sd, fp, pipe[i][1].mask,
			pipe[i][1].vf[0].f, pipe[i][1].vf[1].f,
			pipe[i][1].vf[2].f, pipe[i][1].vf[3].f);
	  empty = 0;
	  break;
	case 3:		/* move to I-reg */
	  _fprintf (sd, fp, "Lo:%d(LOI) -> I <%f>\n", i, pipe[i][1].vf[0]);
	  empty = 0;
	  break;
	case P_STATUS_REG:	/* only status */
	  _fprintf (sd, fp, "%5d ", pipe[i][1].code_addr);
	  _fprintf (sd, fp, "%2d ", delay);
	  _fprintf (sd, fp, "%-6s ", CODE_NAME (pipe[i][1].code));
	  _fprintf (sd, fp, "stat      ", pipe[i][1].addr);
	  _fprintf (sd, fp, "%08x\n", pipe[i][1].status);
	  empty = 0;
	  break;
	case P_CLIP_REG:	/* only clip */
	  _fprintf (sd, fp, "%5d ", pipe[i][1].code_addr);
	  _fprintf (sd, fp, "%2d ", delay);
	  _fprintf (sd, fp, "%-6s ", CODE_NAME (pipe[i][1].code));
	  _fprintf (sd, fp, "clip      ", pipe[i][1].addr);
	  _fprintf (sd, fp, "%08x\n", pipe[i][1].status);
	  empty = 0;
	  break;
	default:
	  ;
	}
    }
  if (ipipe.flag)
    {

      _fprintf (sd, fp, "Lo:%d(%s) -> VI%02d <%d>\n",
		i, CODE_NAME (ipipe.code), ipipe.no, ipipe.vi);
      empty = 0;
    }
  if (spipe.no != 0)
    {
      _fprintf (sd, fp, "%5d ", spipe.code_addr);
      _fprintf (sd, fp, "%2d ", spipe.no);
      _fprintf (sd, fp, "%-6s ", CODE_NAME (spipe.code));
      _fprintf (sd, fp, "P         ");
      _fprintf (sd, fp, "%f\n", spipe.vn);
      empty = 0;
    }
  if (empty)
    {
      _fprintf (sd, fp, "\n");
    }
}

static void
printlist (vu_device * me, char *buf)
{
/*
   [name]               printlist
   [desc.]              print out instruction list to stdout (for debug mode)
   [args]               buf: input data from debug mode command line 
   [return]     void
 */

  char dmy[32];
  int no, i;
  u_long inst[2];

  if (index (buf, ' ') != (char *) NULL)
    {
      sscanf (buf, "%s %d", dmy, &no);
    }
  else
    {
      no = opc;
    }
  for (i = 0; i < 10; i++)
    {
      inst[0] = T2H_I4 (uMEM (no + i, MEM_UPPER));
      inst[1] = T2H_I4 (uMEM (no + i, MEM_LOWER));
      fprintf (me->console_out, "%04d: %08lx %08lx:", no + i, inst[0], inst[1]);
      opcode_analyze (me->console_out, inst, "\t");
      fputc('\n', me->console_out);
    }
}

static void
printreg (vu_device * me, char *buf)
{
/* 
   [name]               printreg
   [desc.]              print out register values(float or int) to stdout
   [args]               buf: input data from debug mode command line 
   [return]     void
 */

  int i;
  /*char dmy[32]; -=UNUSED=- */

  if (buf[1] == 'F')
    {
      if (buf[2] == 0)
	{
	  for (i = 0; i < 32; i++)
	    {
	      fprintf (me->console_out, "VF%02d <%f> <%f> <%f> <%f>\n",
		      i, VF[i][3], VF[i][2], VF[i][1], VF[i][0]);
	    }
	}
      else
	{
	  sscanf (buf, "VF%d", &i);
	  fprintf (me->console_out, "VF%02d <%f> <%f> <%f> <%f>\n",
		  i, VF[i][3], VF[i][2], VF[i][1], VF[i][0]);
	}
    }
  else
    {
      if (buf[1] == 'I')
	{
	  if (buf[2] == 0)
	    {
	      for (i = 0; i < 16; i++)
		{
		  fprintf (me->console_out, "VI%02d <%d>\n", i, VI[i]);
		}
	    }
	  else
	    {
	      sscanf (buf, "VI%d", &i);
	      fprintf (me->console_out, "VI%02d <%d>\n", i, VI[i]);
	    }
	}
    }
}

static void
printregI (vu_device * me, char *buf)
{
/* 
   [name]               printreg
   [desc.]              print out register values(hex) to stdout
   [args]               buf: input data from debug mode command line 
   [return]     void
 */

  int i;
  /* char dmy[32]; -=unused=- */

  if (buf[1] == 'f')
    {
      if (buf[2] == 0)
	{
	  for (i = 0; i < 32; i++)
	    {
	      fprintf (me->console_out, "VF%02d <%08lx> <%08lx> <%08lx> <%08lx>\n",
		      i, *(u_long *) & (VF[i][3]), *(u_long *) & (VF[i][2]),
		      *(u_long *) & (VF[i][1]), *(u_long *) & (VF[i][0]));
	    }
	}
      else
	{
	  sscanf (buf, "vf%d", &i);
	  fprintf (me->console_out, "VF%02d <%08lx> <%08lx> <%08lx> <%08lx>\n",
		  i, *(u_long *) & (VF[i][3]), *(u_long *) & (VF[i][2]),
		  *(u_long *) & (VF[i][1]), *(u_long *) & (VF[i][0]));
	}
    }
  else
    {
      if (buf[1] == 'i')
	{
	  if (buf[2] == 0)
	    {
	      for (i = 0; i < 16; i++)
		{
		  fprintf (me->console_out, "VI%02d <%x>\n", i, VI[i]);
		}
	    }
	  else
	    {
	      sscanf (buf, "vi%d", &i);
	      fprintf (me->console_out, "VI%02d <%x>\n", i, VI[i]);
	    }
	}
    }
}

static void
setVFreg (vu_device * me, char *buf)
{
/* 
   [name]               setVFreg
   [desc.]              set value to VF regster (for debug mode)
   [args]               buf: input data from debug mode command line 
   [return]     void
 */

  int no;
  float val0, val1, val2, val3;

  sscanf (buf, "F%d %f %f %f %f\n", &no, &val0, &val1, &val2, &val3);
  if ((no <= 0) || (no > 31))
    {
      fprintf (me->console_out, "cannot set value VF%02d\n", no);
    }
  else
    {
      VF[no][0].f = val3;
      VF[no][1].f = val2;
      VF[no][2].f = val1;
      VF[no][3].f = val0;
      fprintf (me->console_out, "VF%02d <%f> <%f> <%f> <%f>\n",
	      no, VF[no][3].f, VF[no][2].f, VF[no][1].f, VF[no][0].f);
    }
}

static void
setVIreg (vu_device * me, char *buf)
{
/* 
   [name]               setVIreg
   [desc.]              set value to VI regster (for debug mode)
   [args]               buf: input data from debug mode command line 
   [return]     void
 */

  int no, val;

  sscanf (buf, "I%d %d\n", &no, &val);
  if ((no <= 0) || (no > 24))
    {
      fprintf (me->console_out, "cannot set value VI%02d\n", no);
    }
  else
    {
      VI[no] = val;
      fprintf (me->console_out, "VI%02d <%d>\n", no, VI[no]);
    }
}

void
interactive (SIM_DESC sd, vu_device * me)
{
/*
   [name]               interactive
   [desc.]              debug mode monitor
   [args]               void
   [return]     void
 */

  int stat = 1;
  char buf[128];
  char dmy[32];
  int addr;

  fprintf (me->console_out, "%04ld: ", opc);
  opcode_analyze (me->console_out, instbuf, "\t");
  fputc('\n', me->console_out);
  while (stat)
    {
      fprintf (me->console_out, ">");
      fflush(me->console_out);
      fgets (buf, 128, me->console_in);
      switch (buf[0])
	{
	case 'b':		/* breakpoint */
	  sscanf (buf, "%s %ld", dmy, &bp);
	  break;
	case 'p':		/* pipe */
	  print_pipe (sd, me->console_out, me);
	  break;
	case 'r':		/* run */
	  sscanf (buf, "%s %ld", dmy, &pc);
	  intr_mode = 1;
	  stat = 0;
	  break;
	case 'e':		/* end count */
	  sscanf (buf, "%s %ld", dmy, &ecount);
	  intr_mode = 4;
	  stat = 0;
	  break;
	case 'd':		/* dump MEM by hex */
	  sscanf (buf, "%s %d", dmy, &addr);
	  fprintf (me->console_out, "%04d %08lx %08lx %08lx %08lx\n", addr,
		MEM (addr, 3), MEM (addr, 2), MEM (addr, 1), MEM (addr, 0));
	  break;
	case 'D':		/* dump MEM by float */
	  sscanf (buf, "%s %d", dmy, &addr);
	  fprintf (me->console_out, "%04d %f %f %f %f\n", addr,
		  *(float *) &MEM (addr, 3), *(float *) &MEM (addr, 2),
		  *(float *) &MEM (addr, 1), *(float *) &MEM (addr, 0));
	  /* XXX: what about T2H_F4? */
	  break;
	case 'l':		/* list */
	  printlist (me, buf);
	  break;
	case 's':		/* step */
	  intr_mode = 2;
	  stat = 0;
	  break;
	case 'c':		/* continue */
	  intr_mode = 3;
	  stat = 0;
	  break;
	case 'i':		/* print I */
	  fprintf (me->console_out, "I : %f\n", I);
	  break;
	case 'V':		/* print reg */
	  printreg (me, buf);
	  break;
	case 'v':		/* print reg */
	  printregI (me, buf);
	  break;
	case 'F':		/* set val to VF reg */
	  setVFreg (me, buf);
	  break;
	case 'I':		/* set val to VI reg */
	  setVIreg (me, buf);
	  break;
	case 'Q':		/* print Q */
	  fprintf (me->console_out, "Q : %f\n", Q);
	  break;
	case 'P':		/* print P */
	  fprintf (me->console_out, "P : %f\n", VN[4]);
	  break;
	case 'R':		/* print R */
	  fprintf (me->console_out, "R : 0x%08lx\n", R);
	  break;
	case 'A':		/* print ACC */
	  fprintf (me->console_out, "ACC:<%f> <%f> <%f> <%f>\n",
		  ACC[0], ACC[1], ACC[2], ACC[3]);
	  break;
	case 'M':		/* print MAC flag */
	  fprintf (me->console_out, "MAC:<%04lx>\n", MACflag);
	  break;
	case 'S':		/* print status flag */
	  fprintf (me->console_out, "status:<%03lx>\n", statusflag);
	  break;
	case 'C':		/* print clip flag */
	  fprintf (me->console_out, "clip:<%03lx>\n", clipflag);
	  break;
	case 'h':		/* help */
	  fprintf (me->console_out, "COMMAND LIST\n");
	  fprintf (me->console_out, "r[un]\n");
	  fprintf (me->console_out, "b[reakpoint] <addr>\n");
	  fprintf (me->console_out, "c[ontinue]\n");
	  fprintf (me->console_out, "s[tep]\n");
	  fprintf (me->console_out, "l[ist] <addr>\n");
	  fprintf (me->console_out, "p[ipe]\n");
	  fprintf (me->console_out, "VF<num>(float)\n");
	  fprintf (me->console_out, "vf<num>(hex)\n");
	  fprintf (me->console_out, "VI<num>\n");
	  fprintf (me->console_out, "F<num> <val> <val> <val> <val>\n");
	  fprintf (me->console_out, "I<num> <val>\n");
	  fprintf (me->console_out, "Q\n");
	  fprintf (me->console_out, "P\n");
	  fprintf (me->console_out, "R\n");
	  fprintf (me->console_out, "M[ac flag]\n");
	  fprintf (me->console_out, "S[tatus flag]\n");
	  fprintf (me->console_out, "C[lip flag]\n");
	  fprintf (me->console_out, "d[ump MEM(hex)] <addr>\n");
	  fprintf (me->console_out, "D[ump MEM(float)] <addr>\n");
	  fprintf (me->console_out, "e[nd count] <num>\n");
	  fprintf (me->console_out, "h[elp]\n");
	  break;
	default:
	  break;
	}
    }
}

