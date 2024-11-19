/*  Copyright (C) 1998, Cygnus Solutions                  
*/
#include "sim-main.h"
#include "sim-assert.h"
#include "sim-options.h"
#include "sim-io.h"
#include "sky-hardware.h"
#include "sky-gdb.h"
#include "sky-vu.h"
#include "sky-vif.h"
#include "sky-gpuif.h"
#include "sky-dma.h"
#include "sky-gs.h"
#include "sky-interact.h"
#include "sky-indebug.h"
#include "dis-asm.h"

#ifdef HAVE_STRING_H
#include <string.h>
#else
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#endif

int gdb_vu_pipeorder = ORDER_WZYX;

/* This cache of breakpoint address/original value pairs is used to set/clear
   values that might be in transit in the fifo.  */
struct bp
{
  int type;		/* Either VIF or VU */
  unsigned_4 addr;	/* The source addr of the instruction */
  int val;		/* Original value (for bp removal). */
};

static int fifo_bp_cnt = 0;

struct bp *fifo_bp_cache;
static int bp_cache_size = 0;

enum    /* for set_fifo_breakpoints */
{
  CLR_BP,
  SET_BP
};

/* Read the bp address/value pairs from the GDB comm area into the bp cache */

static void 
init_fifo_bp_cache (SIM_DESC sd, int count)
{
  int i, len;
  struct bp *bp;

  if (bp_cache_size == 0)
    {
      bp_cache_size = 16;
      fifo_bp_cache = (struct bp *) xmalloc (bp_cache_size*sizeof (struct bp));
    }
  else if (bp_cache_size < (count + 1))
    {
      bp_cache_size = count * 2;
      fifo_bp_cache = (struct bp*) xrealloc (fifo_bp_cache, 
                                        bp_cache_size * sizeof (struct bp));
    }

  for (i=0, bp=fifo_bp_cache; i<count; i++, bp++)
    {
      len = sim_read (sd, VIO_BASE + FIFO_BPT_TBL + i*BPT_ELEM_SZ, 
		      (char *) &(bp->type), 4);
      SIM_ASSERT (len == 4);
      
      len = sim_read (sd, VIO_BASE + FIFO_BPT_TBL + i*BPT_ELEM_SZ + 4, 
		      (char *) &(bp->addr), 4);
      SIM_ASSERT (len == 4);
      
      len = sim_read (sd, VIO_BASE + FIFO_BPT_TBL + i*BPT_ELEM_SZ + 8, 
		      (char *) &(bp->val), 4);
      SIM_ASSERT (len == 4);
    }
}
  
/* Search backwards through the FIFO looking for addresses that match
   our bp cache. Either set the breakpoint or restore the orignal code,
   as specified by the SET_BP flag.  */

static void 
set_fifo_breakpoints (struct vif_device *vif, int count, int set_bp)
{
  int i, pc, qw;
  int num_found = 0;
  struct fifo_quadword *fq;
  struct bp *bp;

  /* We start in the future (ie, stuff in the FIFO that hasn't passed yet)
     and work toward the past. That maximizes our chances of finding things
     early.  */
  pc = vif->fifo.origin + vif->fifo.next - 1;

  while (pc >= vif->fifo.origin)
    {
      fq = vif_fifo_access (&(vif->fifo), pc);
      if (fq == NULL)   /* Hit the beginning */
        return;

      for (i=0, bp=fifo_bp_cache; i<count; i++, bp++)
        {
          if ((bp->addr & ~0x0f) == (fq->source_address & ~0x0f))
            {
              /* Get the index into the quadword array. */
              qw = (bp->addr & 15) >> 2;

              if (bp->type == TXVU_CPU_VIF1)
                {
                  if (set_bp == SET_BP)
                    fq->data[qw] = fq->data[qw] | (TXVU_VIF_BRK_MASK << 24);
                  else
                    fq->data[qw] = H2T_4(bp->val);
                }
	      else /* Must be a VU breakpoint */
		{
                  if (set_bp == SET_BP)
		    fq->data[qw+1] = fq->data[qw+1] | (TXVU_VU_BRK_MASK << 24);
                  else
                    fq->data[qw+1] = H2T_4(bp->val);
		}

	      ++num_found;
	      if (num_found == count)
		return;
            }
        }

      --pc;
    }
}

/* Check the given ADDR against the breakpoints in the FIFO bp cache.
   Return TRUE if found.  */

int 
is_vif_breakpoint (unsigned_4 addr)
{
  int i;

  for (i=0; i<fifo_bp_cnt; i++)
    {
      if (fifo_bp_cache[i].addr == addr)
	return 1;
    }

  return 0;
}

/* Set up sim controlled breakpoints when execution is suspended/halted */

static SIM_RC
resume_handler (SIM_DESC sd)
{
  if (sim_read (sd, VIO_BASE + FIFO_BPT_CNT, (char *) &fifo_bp_cnt, 4) != 4)
    return SIM_RC_FAIL;

  if (fifo_bp_cnt)
    {
      init_fifo_bp_cache (sd, fifo_bp_cnt);
#ifndef TARGET_SKY_B
      set_fifo_breakpoints (&vif0_device, fifo_bp_cnt, SET_BP);
#endif
      set_fifo_breakpoints (&vif1_device, fifo_bp_cnt, SET_BP);
    }

  return SIM_RC_OK;
}

/* Clean-up sim controlled breakpoints when execution is suspended/halted */

static SIM_RC
suspend_handler (SIM_DESC sd)
{
  sim_cpu *cpu = STATE_CPU (sd, 0);

  /* Set LAST_DEVICE so GDB knows who interrupted */
  sim_core_write_aligned_1 (cpu, NULL_CIA, write_map, 
                            LAST_DEVICE, cpu->cur_device);

  if (sim_read (sd, VIO_BASE + FIFO_BPT_CNT, (char *) &fifo_bp_cnt, 4) != 4)
    return SIM_RC_FAIL;

  if (fifo_bp_cnt) 
    {
      init_fifo_bp_cache (sd, fifo_bp_cnt);
#ifndef TARGET_SKY_B
      set_fifo_breakpoints (&vif0_device, fifo_bp_cnt, CLR_BP);
#endif
      set_fifo_breakpoints (&vif1_device, fifo_bp_cnt, CLR_BP);
    }

  return SIM_RC_OK;
}

void
sky_sim_engine_halt (SIM_DESC sd, sim_cpu *last, sim_cia cia)
{
  sim_cpu *cpu = STATE_CPU (sd, 0);

  if (cpu->cur_device == TXVU_CPU_MASTER)
    {
      if (last != NULL)
        CIA_SET (last, cia);
    }

  suspend_handler (sd);
}

void
sky_sim_engine_restart (SIM_DESC sd, sim_cpu *last, sim_cia cia)
{
  if (last != NULL)
    CIA_SET (last, cia);

  resume_handler (sd);
}

/* Called from simulator module initialization.  */

SIM_RC
sky_sim_module_install (SIM_DESC sd)
{
  sim_module_add_resume_fn (sd, resume_handler);
  sim_module_add_suspend_fn (sd, suspend_handler);

  return SIM_RC_OK;
}

enum {
  OPTION_VU_PIPE = OPTION_START,
  OPTION_VU_PIPE_ORDER,
#ifndef TARGET_SKY_B
  OPTION_VIF0_LIST,
#endif
  OPTION_VIF1_LIST,
  OPTION_LOG,
  OPTION_LOG_FILE,
#ifdef SKY_FUNIT
  OPTION_FLOAT_TYPE,
#endif
  OPTION_RESET,
  OPTION_GS_ENABLE,
  OPTION_GS_REFRESH1,
  OPTION_GS_REFRESH2,
  OPTION_GIF_REFRESH,
  OPTION_SKY_DEBUG,
  OPTION_SKY_DEBUG_FILE,
  OPTION_SKY_LOAD_NEXT
};


static DECLARE_OPTION_HANDLER( vu_option_handler );

static const OPTION vu_com_options[] =
{
#ifdef TARGET_SKY_B
  { {"pipe", optional_argument, NULL, OPTION_VU_PIPE},
      '\0', "[vu]1", "Show VU pipeline (interactive mode only)",
      vu_option_handler },
#else
  { {"pipe", optional_argument, NULL, OPTION_VU_PIPE},
      '\0', "[vu]0|1", "Show VU pipeline (interactive mode only)",
      vu_option_handler },
#endif
  { {"pipe-order", required_argument, NULL, OPTION_VU_PIPE_ORDER},
     '\0', "wzyx|xyzw", "Vector order in VU pipeline display",
     vu_option_handler},
  { {NULL, no_argument, NULL, 0}, '\0', NULL, NULL, NULL }
};

static SIM_RC
vu_option_handler (SIM_DESC sd, sim_cpu * cpu, int opt, char *arg, int is_cmd)
{
  switch (opt)
    {
    case OPTION_VU_PIPE:

      if (arg == NULL)
	{	
#ifndef TARGET_SKY_B
	  /* print both pipes */
	  sim_io_printf (sd, "VU0 Pipeline:\n");
	  print_pipe (sd, NULL, &vu0_device);
#endif
	  sim_io_printf (sd, "VU1 Pipeline:\n");
	  print_pipe (sd, NULL, &vu1_device);
	}
      else
	{
#ifndef TARGET_SKY_B
	  if (strcmp (arg, "vu0") == 0 || ((*arg == '0') && (*(arg + 1) == 0)))
	    {
	      sim_io_printf (sd, "VU0 Pipeline:\n");
	      print_pipe (sd, NULL, &vu0_device);
	    }
	  else 
#endif
	  if (!strcmp (arg, "vu1") || ((*arg == '1') && (*(arg + 1) == 0)))
	    {
	      sim_io_printf (sd, "VU1 Pipeline:\n");
	      print_pipe (sd, NULL, &vu1_device);
	    }
	  else
	    sim_io_eprintf (sd, "invalid argument %s, pipe command ignored\n", arg);
	}
      break;
    case OPTION_VU_PIPE_ORDER:
      if (strcmp (arg, "xyzw") == 0)
	{
	  gdb_vu_pipeorder = ORDER_XYZW;
	}
      else if (strcmp (arg, "wzyx") == 0)
	{
	  gdb_vu_pipeorder = ORDER_WZYX;
	}
      else
	{
	  sim_io_error (sd, "Invalid argument %s, pipe-order command ignored.\n\
(Valid arguments are 'xyzw' or 'wzyx')\n", arg);
	}
      break;
    default:
      sim_io_error (sd, "Illegal VU command\n");
      break;
  }

  return SIM_RC_OK;
}

static DECLARE_OPTION_HANDLER( vif_option_handler );

static const OPTION vif_com_options[] =
{
#ifndef TARGET_SKY_B
  { {"list-vif0", optional_argument, NULL, OPTION_VIF0_LIST},
      '\0', "range", "Show VIF0 FIFO   (interactive mode only)",
      vif_option_handler },
#endif
  { {"list-vif1", optional_argument, NULL, OPTION_VIF1_LIST},
      '\0', "range", "Show VIF1 FIFO   (interactive mode only)",
      vif_option_handler },
  { {NULL, no_argument, NULL, 0}, '\0', NULL, NULL, NULL }
};

static disassemble_info info;

static int
dis_asm_read_memory (bfd_vma addr, bfd_byte *myaddr, int len, 
             struct disassemble_info *info)
{
  int count = sim_read ((SIM_DESC) info->stream, addr, myaddr, len);

  return !(count == len);
}

static void
dis_asm_memory_error (int rc, bfd_vma addr, struct disassemble_info *info)
{
  sim_io_error ((SIM_DESC) info->stream, 
                "Error accessing memory at address 0x%08llx\n", addr);
}

static void
dis_asm_print_address (bfd_vma addr, struct disassemble_info *info)
{
  sim_io_printf ((SIM_DESC) info->stream, "0x%08llx", addr); 
}
  
static void 
init_disassemble_info( SIM_DESC sd )
{
  info.fprintf_func = (fprintf_ftype) sim_io_printf;
  info.stream = (FILE *) sd;
  info.endian = BFD_ENDIAN_LITTLE;
  info.read_memory_func = dis_asm_read_memory;
  info.memory_error_func = dis_asm_memory_error;
  info.print_address_func = dis_asm_print_address;
}

static int
find_insn_in_fifo (struct vif_fifo* fifo, int *pcptr)
{
  int i, pc = *pcptr;
  struct fifo_quadword *fq;
  int insns_found = 0;

  while (pc >= 0)
    {
      fq = vif_fifo_access (fifo, pc);
      if (fq == NULL)
        return 0;

      for (i=0; i<4; i++)
        {
	  /* Both words of a DMA instruction are marked. Only count it once */
          if (fq->word_class[i]==wc_vifcode 
	      || (fq->word_class[i]==wc_dma && i%2==0))
            insns_found++;
        }

      if (insns_found > 0)
        {
          *pcptr = pc;
          return insns_found;
        }

      --pc;
    }

  return 0;
}

#define DEFAULT_LIST_COUNT      10

static SIM_RC
vif_option_handler( sd, cpu, opt, arg, is_command )
     SIM_DESC sd;
     sim_cpu *cpu;
     int opt;
     char *arg;
     int is_command;
{
  struct vif_device *vif;
  struct fifo_quadword *fq;
  int start_pc, start_qw;
  int end_pc, end_qw;
  int i, count;
  int insns_found = 0;
  unsigned32 pc, qw;
  
  if (!info.read_memory_func)
    init_disassemble_info( sd );

  switch( opt ) {
#ifndef TARGET_SKY_B
  case OPTION_VIF0_LIST:
    vif = &vif0_device;
    break;
#endif

  case OPTION_VIF1_LIST:
    vif = &vif1_device;
    break;

  default:
    sim_io_error ( sd, "Illegal list-VIF command\n" );
    return SIM_RC_OK;
  }

  if (!arg)
    {
      /* List the last DEFAULT_LIST_COUNT vif instructions. 
         Unfortunately, the pseudo-pc is incremented for every word that 
         passes through the fifo, which means we have to search back to find 
         the most recent VIF instruction, and then continue searching until 
         we find DEFAULT_LIST_COUNT more. */
      end_pc = vif->last_fifo_pc;
      insns_found = find_insn_in_fifo (&(vif->fifo), &end_pc);

      if (insns_found == 0)
        return SIM_RC_OK;

      count = DEFAULT_LIST_COUNT;
      start_pc = end_pc;
    }
  else /* user specified a range of some sort */
    {
      char *comma;

      /* legal ranges are CNT or FIRST,LAST where either first or last may
         be omitted. 

         Check to see if there are one or two args. */
      comma = index (arg, ',');
      if (comma == NULL)        /* no comma, increase count */
        {
          end_pc = vif->last_fifo_pc;
          insns_found = find_insn_in_fifo (&(vif->fifo), &end_pc);
          if( insns_found == 0 )
            return SIM_RC_OK;

          if (sscanf (arg, "%d", &count) != 1)
            {
              sim_io_error ( sd, "Argument %s not understood\n", arg);
              count = DEFAULT_LIST_COUNT;
            }

	  if (count < 0) /* you never know :-( */
	    return SIM_RC_OK;

          start_pc = end_pc;
        }
      else /* there was a comma */
        {
          /* We allow open-ended ranges of <start,> and <,end>.
             Start by breaking arg into two separate strings. */
          comma++;
          *(comma - 1) = 0;

          if (sscanf (arg, "%d", &start_pc) != 1)
            start_pc = 0;
          else
            start_pc >>= 2; /* make index to fifo */

	  if (start_pc < 0)
	    start_pc = 0;

          if (sscanf (comma, "%d", &end_pc) != 1)
            end_pc = vif->last_fifo_pc;
          else
            end_pc >>= 2; /* make index to fifo */

	  if (end_pc < start_pc)
	    return SIM_RC_OK;

	  if (end_pc > vif->last_fifo_pc)
	    end_pc = vif->last_fifo_pc;

	  count = 0;
          insns_found = -1; 
        }
    }

  if (insns_found >= 0 && insns_found < count)
    {
      /* Found the most recent VIF insn, now find count more. */
      start_pc = end_pc - 1;
      while (start_pc>=0 && insns_found<count)
        {
          insns_found += find_insn_in_fifo (&(vif->fifo), &start_pc);
          --start_pc;
        }

      /* This can happen if end_pc is 0 (but vif->last_qw_pc > 0).  */
      if (start_pc < 0)
	start_pc = 0;
    }

  count = 0;
  for (pc=start_pc; pc<=end_pc; pc++)
    {
      fq = vif_fifo_access (&(vif->fifo), pc);
      for (qw=0; qw<4; qw++)
        {
          bfd_vma addr;

          if (fq->word_class[qw]==wc_dma && qw%2==0)
            info.mach = bfd_mach_dvp_dma;
          else if (fq->word_class[qw] == wc_vifcode)
            info.mach = bfd_mach_dvp_vif;
          else
            {
	      if (pc != end_pc) /* don't start anything in last quad-word */
		{
		  if (count == 0)
		    {
		      addr = (fq->source_address & ~15) | (qw << 2);
		      sim_io_printf (sd, "       (0x%08llx)", addr);
		    }

		  ++count; /* count is in words */
		}
	      continue;
	    }

	  if (count > 0)
	    {
	      sim_io_printf (sd, "\tdata - %d bytes\n", count << 2);
	      count = 0;
	    }

          addr = (fq->source_address & ~15) | (qw << 2);
          sim_io_printf (sd, "%5d: (0x%08llx)\t", (pc << 2) | qw, addr);

	  /* DMA instructions are 8-bytes long, so adjust qw accordingly.  */
	  /* This assumes that the opcodes library was configured with
	     --enable-targets=dvp-elf !! */
          if (print_insn_dvp (addr, &info) == 8)
	    qw++;

          sim_io_printf ( sd, "\n");
        }
    }

  if (count > 0) /* clear off any remaining count */
    sim_io_printf (sd, "\tdata - %d bytes\n", count << 2);

  return SIM_RC_OK;
}

static DECLARE_OPTION_HANDLER( log_option_handler );

static const OPTION log_com_options[] =
{
  { {"log", required_argument, NULL, OPTION_LOG},
      'l',
#ifndef TARGET_SKY_B
      "all|gif|gif1|gif2|gif3|gs|vif0|vif1=on|off", "Unit disassembly/input logging",
#else
      "gs=on|off", "Unit disassembly/input logging",
#endif
      log_option_handler },
  { {"log-file", required_argument, NULL, OPTION_LOG_FILE},
      '\0',
#ifndef TARGET_SKY_B
      "gif|gs|vif0|vif1=FILENAME", "Specify unit and file name for disassembly/input logging",
#else
      "gs=FILENAME", "Specify unit and file name for disassembly/input logging",
#endif
      log_option_handler },
  { {NULL, no_argument, NULL, 0}, '\0', NULL, NULL, NULL }
};

/* Check conformance and find sub-option in arg.
 * arg is supposed to be of the form "unit=something", 
 * where unit is one of vu0 or vu1 or vif0 or vif1 or gif. 
 *
 * Returns ptr to "something" if found and unit checks out.
 */  
static char*
find_option_after_equal_sign( sd, arg )
     SIM_DESC sd;
     char *arg;
{
  char *optptr = arg;

  while( *optptr && (*optptr != '=') ) /* Find the equal sign */
    optptr++;

  if( !(*optptr) ) {    /* Equal sign not found */
    sim_io_eprintf( sd, "Argument must be of the form `unit=opt'\n" );
    return 0;
  }

  *(optptr++) = '\0';   /* Null the equal sign out to split arg */

  if( !(*optptr) ) {    /* Would anyone do "unit=" except to be mean? */
    sim_io_eprintf( sd, "Missing option after `='\n" );
    return 0;
  }

  /* check validity of first sub-option */
  if (   strcmp (arg,"all")  == 0 
      || strcmp (arg,"dmac") == 0  
      || strcmp (arg,"vif0") == 0
      || strcmp (arg,"vif1") == 0
      || strcmp (arg,"vu0")  == 0
      || strcmp (arg,"vu1")  == 0
      || strcmp (arg,"gif")  == 0 
      || strcmp (arg,"gif1") == 0 
      || strcmp (arg,"gif2") == 0 
      || strcmp (arg,"gif3") == 0
      || strcmp (arg,"gs")   == 0 )
    return optptr;

  sim_io_eprintf (sd, "Unit %s unknown \n", arg);
  return 0;
}

static SIM_RC
log_option_handler( sd, cpu, opt, arg, is_command )
     SIM_DESC sd;
     sim_cpu *cpu;
     int opt;
     char *arg;
     int is_command;
{
  char *subopt = NULL;

  switch (opt) {
  case OPTION_LOG:
    if (arg == NULL) 
      {
        sim_io_printf( sd, "Show current logging status\n" );
        break;
      }

    subopt = find_option_after_equal_sign( sd, arg );

    if (!subopt) 
      {
        sim_io_eprintf( sd, "--log ignored\n" );
        break;
      }

    if ((strcmp (subopt, "on" ) == 0) ||
        (strcmp (subopt, "off") == 0))
      {
        int on = 0;
        if (strcmp (subopt, "on") == 0) 
          on = 1;
        
        if (strcmp (arg,"all") == 0)
          {
            gif_options (&gif_device,   (on) ? SKY_OPT_GIF_OUTPUT_ON : SKY_OPT_GIF_OUTPUT_OFF,NULL,0,0);
#ifndef TARGET_SKY_B
            gif_options (&gif_device,   (on) ? SKY_OPT_TRACE_ON : SKY_OPT_TRACE_OFF,"gif",0,0);
            vif_options (&vif0_device,(on) ? SKY_OPT_TRACE_ON : SKY_OPT_TRACE_OFF,NULL);
            vif_options (&vif1_device,(on) ? SKY_OPT_TRACE_ON : SKY_OPT_TRACE_OFF,NULL);
#endif
          }
        else if (strcmp (arg,"gs") == 0) 
          gif_options (&gif_device, (on) ? SKY_OPT_GIF_OUTPUT_ON : SKY_OPT_GIF_OUTPUT_OFF,NULL,0,0);
#ifndef TARGET_SKY_B
        else if ((strcmp (arg,"gif" ) == 0) ||
                 (strcmp (arg,"gif1") == 0) ||
                 (strcmp (arg,"gif2") == 0) ||
                 (strcmp (arg,"gif3") == 0)) 
          gif_options (&gif_device,(on) ? SKY_OPT_TRACE_ON : SKY_OPT_TRACE_OFF,arg,0,0);
        else if (strcmp (arg, "vif0") == 0) 
          vif_options (&vif0_device,(on) ? SKY_OPT_TRACE_ON : SKY_OPT_TRACE_OFF,NULL);
        else if (strcmp (arg, "vif1") == 0)
          vif_options (&vif1_device,(on) ? SKY_OPT_TRACE_ON : SKY_OPT_TRACE_OFF,NULL);
#endif        
        else 
          sim_io_eprintf (sd,"Disassembly/input logging not available for %s\n", (arg) ? arg : "");
      }
    else
      sim_io_eprintf (sd, "Option must be either `on' or `off', --log ignored\n");
    break;

  case OPTION_LOG_FILE:
    subopt = find_option_after_equal_sign (sd,arg);

    if (!subopt) 
      {
        sim_io_eprintf (sd, "--log-file ignored\n");
        break;
      } 

    if (strcmp (arg,"gs") == 0 )
      gif_options (&gif_device,SKY_OPT_GIF_OUTPUT_NAME,subopt,0,0);
#ifndef TARGET_SKY_B
    else if (strcmp (arg,"gif") == 0 )
      gif_options (&gif_device,SKY_OPT_TRACE_NAME,subopt,0,0);
    else if (strcmp (arg,"vif0") == 0 )
      vif_options (&vif0_device, SKY_OPT_TRACE_NAME,subopt);
    else if (strcmp (arg,"vif1") == 0 )                        
      vif_options (&vif1_device, SKY_OPT_TRACE_NAME,subopt);
#endif
    else
      sim_io_printf (sd, "Logging not available for %s\n", (arg) ? arg : "");
    break;
  }
    
  return SIM_RC_OK;
}


static DECLARE_OPTION_HANDLER( sky_option_handler );

static const OPTION sky_com_options[] =
{
#ifdef SKY_FUNIT
  { {"float-type", required_argument, NULL, OPTION_FLOAT_TYPE},
      '\0', "fast|accurate", "Use fast (host) or accurate (target) floating point",
      sky_option_handler },
#endif
  { {"reset", no_argument, NULL, OPTION_RESET},
     '\0', NULL, "Reset simulated peripherals (interactive mode only)",
     sky_option_handler },
  { {"enable-gs", required_argument, NULL, OPTION_GS_ENABLE},
     '\0', "on|off", "Enable GS library routines",
     sky_option_handler },
  { {"gs-refresh1", required_argument, NULL, OPTION_GS_REFRESH1},
     '\0', "0xaddress0=0xvalue0:0xaddress1=0xvalue1", "GS refresh buffer 1 addresses and values",
     sky_option_handler },
  { {"gs-refresh2", required_argument, NULL, OPTION_GS_REFRESH2},
     '\0', "0xaddress0=0xvalue0:0xaddress1=0xvalue1", "GS refresh buffer 2 addresses and values",
     sky_option_handler },
  { {"screen-refresh", required_argument, NULL, OPTION_GIF_REFRESH},
     '\0', "on|off", "Refresh simulator screen at end of packet",
     sky_option_handler },
  { {"sky-debug", required_argument, NULL, OPTION_SKY_DEBUG},
     '\0', "DEBUG STRING", 
     DISPLAY_OPTIONAL("Display internal debug information"),
     sky_option_handler, DISPLAY_OPTIONAL(NULL) }, 
  { {"sky-debug-file", required_argument, NULL, OPTION_SKY_DEBUG_FILE},
     '\0', "dmac|gif|gs|vif0|vif1|vu0|vu1=FILENAME",
     DISPLAY_OPTIONAL("Specify unit and file name for debug information"),
     sky_option_handler, DISPLAY_OPTIONAL(NULL) },

#ifndef TARGET_SKY_B
  { {"load-next", required_argument, NULL, OPTION_SKY_LOAD_NEXT},
      '\0', "EXEC", "Specify the next executable to load in multiphase mode",
      sky_option_handler },
#endif

  { {NULL, no_argument, NULL, 0}, '\0', NULL, NULL, NULL }
};


static SIM_RC
sky_option_handler( sd, cpu, opt, arg, is_command )
     SIM_DESC sd;
     sim_cpu *cpu;
     int opt;
     char *arg;
     int is_command;
{
  char *subopt = NULL;

  switch( opt ) 
    {

#ifdef SKY_FUNIT
    case OPTION_FLOAT_TYPE:
      /* Use host (fast) or target (accurate) floating point implementation. */
      if (arg && (strcmp (arg,"fast") == 0 ||
                  strcmp (arg,"host") == 0))
          STATE_FP_TYPE_OPT (sd) &= ~STATE_FP_TYPE_OPT_ACCURATE;
      else if (arg && (strcmp (arg,"accurate") == 0 ||
                       strcmp (arg,"target") == 0))
          STATE_FP_TYPE_OPT (sd) |= STATE_FP_TYPE_OPT_ACCURATE;
      else
        {
          sim_io_eprintf (sd, "Unrecognized float-type option `%s'\n", (arg) ? arg : "");
          return SIM_RC_FAIL;
        }
      return SIM_RC_OK;
#endif

    case OPTION_RESET:
      /* XXX STATE_MLOAD_INDEX (sd) = 0; */ /* reset multi-phase load */
      vif0_reset();
      vif1_reset();
      vu0_reset();
      vu1_reset();
      gif_reset();
      dma_reset();
      gs_reset();
      break;

    case OPTION_GS_ENABLE:
      /* Enable GS libraries.  */
      if ( arg && strcmp (arg, "on") == 0 )
        gif_options (&gif_device,SKY_OPT_GS_ENABLE,NULL,0,0);
      else if ( arg && strcmp (arg, "off") == 0 )
        gif_options (&gif_device,SKY_OPT_GS_DISABLE,NULL,0,0);
      else
        {
          sim_io_eprintf (sd, "Unrecognized enable-gs option `%s'\n", (arg) ? arg : "");
          return SIM_RC_FAIL;
        }
      return SIM_RC_OK;

    case OPTION_GS_REFRESH1:
    case OPTION_GS_REFRESH2:
      {
        unsigned_4 address[2];
        long long value[2];
        char c[3];
       
        if ( arg && strlen (arg) == 59 && arg[10] == '=' &&
             arg[29] == ':' &&  arg[40] == '=' &&
             ( sscanf (arg,"%lx%c%Lx%c%lx%c%Lx", &address[0],&c[0],&value[0],
                      &c[1],&address[1],&c[2],&value[1]) == 7 ))
          {
            gif_options (&gif_device, ( opt == OPTION_GS_REFRESH1 ) ?
                         SKY_OPT_GS_REFRESH1:SKY_OPT_GS_REFRESH2,
                         NULL,&address[0],&value[0]);
          }
        else
          {
            sim_io_eprintf (sd, "Unrecognized gs-refresh option `%s'\n", (arg) ? arg : "");
            return SIM_RC_FAIL;
          }
      }
      return SIM_RC_OK;
  
    case OPTION_GIF_REFRESH:
      if ( arg && strcmp (arg, "on") == 0 )
        gif_options (&gif_device,SKY_OPT_GIF_REFRESH_ON,NULL,0,0);
      else if ( arg && strcmp (arg, "off") == 0 )
        gif_options (&gif_device,SKY_OPT_GIF_REFRESH_OFF,NULL,0,0);
      else
        {
          sim_io_eprintf (sd, "Unrecognized screen-refresh option `%s'\n", (arg) ? arg : "");
          return SIM_RC_FAIL;
        }
      return SIM_RC_OK;
    
    case OPTION_SKY_DEBUG:
      if (arg)
        {
          if ( sky_debug_string != NULL ) 
            zfree (sky_debug_string);
         
          sky_debug_string = zalloc (strlen (arg) + 1);
          
          if (sky_debug_string != 0)
            strcpy (sky_debug_string, arg);
          else
            return SIM_RC_FAIL;
        }  
      return SIM_RC_OK;

    case OPTION_SKY_DEBUG_FILE:
      subopt = find_option_after_equal_sign (sd,arg);

      if (!subopt) 
        {
          sim_io_eprintf (sd, "--sky-debug-file ignored\n");
          return SIM_RC_FAIL;
        } 
     
      if (strcmp (arg,"dmac") == 0 )
        dma_options (&dma_device,SKY_OPT_DEBUG_NAME,subopt);
      else if (strcmp (arg,"gs") == 0 )
        gs_options (&gs_device,SKY_OPT_DEBUG_NAME,subopt);  
      else if (strcmp (arg,"gif") == 0 )
        gif_options (&gif_device,SKY_OPT_DEBUG_NAME,subopt,0,0);
#ifndef TARGET_SKY_B
      else if (strcmp (arg,"vif0") == 0 )
        vif_options (&vif0_device,SKY_OPT_DEBUG_NAME,subopt);
#endif
      else if (strcmp (arg,"vif1") == 0 )                        
        vif_options (&vif1_device,SKY_OPT_DEBUG_NAME,subopt);
#ifndef TARGET_SKY_B
      else if (strcmp (arg,"vu0") == 0 )
        vu_options (&vu0_device,SKY_OPT_DEBUG_NAME,subopt);
#endif
      else if (strcmp (arg,"vu1") == 0 )                        
        vu_options (&vu1_device,SKY_OPT_DEBUG_NAME,subopt);
      else
        {
          sim_io_printf (sd, "Debug tracing not available for %s", (arg) ? arg : "");
          return SIM_RC_FAIL;
        }
      return SIM_RC_OK;

    case OPTION_SKY_LOAD_NEXT:
      {
	if(STATE_MLOAD_COUNT(sd) >= MAX_MLOAD_COUNT)
	  {
	    sim_io_eprintf (sd, "Too many executables in next-load list.  The limit is %d.\n",
			    MAX_MLOAD_COUNT);
	    return SIM_RC_FAIL;
	  }
	else
	  {
	    STATE_MLOAD_NAME (sd) [STATE_MLOAD_COUNT (sd)] = xstrdup (arg);
	    STATE_MLOAD_COUNT (sd) ++;
	  }
	break;
      }
    }

  return SIM_RC_OK;
}


void
gdb_attach ( SIM_DESC sd )
{
  sim_core_attach (sd,
                   NULL,
                   0 /*level */ ,
                   access_read_write,
                   0, 
                   GDB_COMM_AREA,
                   GDB_COMM_SIZE /*nr_bytes */ ,
                   0 /*modulo */ ,
                   NULL,
                   NULL /*buffer */ );
  return;
}


void
sky_command_options_open ( SIM_DESC sd )
{

#if defined SKY_FUNIT
  /* Set "--float-type fast" as the default. */
  STATE_FP_TYPE_OPT (sd) &= ~STATE_FP_TYPE_OPT_ACCURATE;
#endif
  sky_debug_string = NULL;
  sim_add_option_table (sd, NULL, sky_com_options);
  sim_add_option_table (sd, NULL, log_com_options);
  sim_add_option_table (sd, NULL, vu_com_options);
  sim_add_option_table (sd, NULL, vif_com_options);

  return;
}


void
sky_command_options_close ( SIM_DESC sd )
{
  /* Close off file handles  */
  dma_options (&dma_device, SKY_OPT_CLOSE,NULL);
  gs_options  (&gs_device,  SKY_OPT_CLOSE,NULL);
  gif_options (&gif_device, SKY_OPT_CLOSE,NULL,0,0);
#ifndef TARGET_SKY_B
  vif_options (&vif0_device,SKY_OPT_CLOSE,NULL);
#endif
  vif_options (&vif1_device,SKY_OPT_CLOSE,NULL);
#ifndef TARGET_SKY_B
  vu_options  (&vu0_device, SKY_OPT_CLOSE,NULL);
#endif
  vu_options  (&vu1_device, SKY_OPT_CLOSE,NULL);
  
  return;
}


void
sky_store_file_name (char **at_location, char *file_name)
{
  /* Allow the user to overwrite the existing file name.  */
  if (*at_location != NULL)
    zfree (at_location);

  *at_location = zalloc (strlen (file_name) + 1);
   
  ASSERT (*at_location != NULL);
   
  if (*at_location != NULL)
    strcpy (*at_location, file_name);
  
  return;
}

void
sky_open_file ( FILE **file_ptr, char *file_name, char *default_name,
                int    buf_mode  )
{
  if ( *file_ptr != NULL ) 
    fclose ( *file_ptr );
  
  *file_ptr = fopen ((file_name != NULL) ? file_name : default_name, "w");
             
  ASSERT (*file_ptr != NULL);
          
  if ( *file_ptr ) 
    setvbuf (*file_ptr, NULL, buf_mode, 2048 );

  return; 
}
