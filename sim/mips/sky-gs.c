/*  Copyright (C) 1998, Cygnus Solutions                             */

#include "sim-main.h"
#include "sky-device.h"
#include "sky-gs.h"
#include "sky-gpuif.h"
#include "sky-gdb.h"
#include "sky-indebug.h"
#include "sim-assert.h"

#ifdef SKY_GPU2
#include "libgpu2.h"
#endif

/* Internal function declarations.                                   */
static int gs_io_read_buffer (device *, void *, int, address_word,
                              unsigned, sim_cpu *, sim_cia);
static int gs_io_write_buffer (device *, const void *, int, address_word,
                               unsigned, sim_cpu *, sim_cia);

/* GS device definition and initialization.                          */
struct GS_devicefull gs_device = 
{ 
  { "gs",                            /* Device name                  */
    &gs_io_read_buffer,              /* Call location for read       */
    &gs_io_write_buffer,             /* Call location for write      */
  },
  NULL,                              /* GS debug file                */
  NULL                               /* GS debug file name           */
};


/* Handle a read request of GS specific memory.  No reads are valid. */
int
gs_io_read_buffer ( device *me,
                    void *dest,
                    int space,
                    address_word addr,
                    unsigned nr_bytes,
                    sim_cpu *processor,
                    sim_cia cia )
{
  struct GS_devicefull *gs = (struct GS_devicefull *) me;

  /* Validate a register read request.                               */
  if (( addr >= GS_REGISTER_WINDOW_START ) &&
      ( addr <= GS_REGISTER_WINDOW_END ))
    {
      long long value = 0;

      /* No valid read at the moment.                                */
      memcpy (dest, &value, (nr_bytes > GS_REGISTER_BYTES) ? GS_REGISTER_BYTES : nr_bytes);

      if ( indebug ("gs"))
        GS_DEBUG ("GS Error: Registers are write only. Address: %08lx.\n", addr);
    }
  else
    /* Not even close to the valid range. Should never occur.        */
    ASSERT (0);
    
  return (nr_bytes);
}


/* Handle a write request on GS specific memory.                     */
int
gs_io_write_buffer ( device *me,
                     const void *source,
                     int space,
                     address_word addr,
                     unsigned nr_bytes,
                     sim_cpu *processor,
                     sim_cia cia )
{
  struct GS_devicefull *gs = (struct GS_devicefull *) me;
  
  /* Validate a register write request.                              */ 
  if (( addr >= GS_REGISTER_WINDOW_START ) &&
      ( addr <= GS_REGISTER_WINDOW_END ))
    {
      int return_code = 0;
      unsigned_4 input[2];
      char *trace_file_name;
      long long gs_data;

      /* The register write must be 64 bits, aligned.                */
      if (( nr_bytes != GS_REGISTER_BYTES ) &&
          ( indebug("gs")))
        GS_DEBUG ("GS warning: Write requested not size of register. Bytes: %d.\n", nr_bytes);  

       if ( addr & 0x0f )
        {
          if ( indebug("gs"))
            GS_DEBUG ("GS warning: Register write not aligned. Address: %08lx.\n", addr); 
          addr &= ~0x0f ;
        }      
    
      /* Regardless of the size of input, only take 64 bits.         */ 
      input[0] = T2H_4 ((( unsigned_4 *) source)[0]);
      input[1] = T2H_4 ((( unsigned_4 *) source)[1]);

      /* The GS registers are memory mapped to simulation addresses. */
      /* The data passed is sent to the GS through a SCEI provided   */
      /* call or pumped out to a stdout.                             */
      switch ( addr )  
        {
        case GS_REG_PMODE: 
        case GS_REG_DISPFB1:   
        case GS_REG_DISPLAY1: 
        case GS_REG_DISPFB2:  
        case GS_REG_DISPLAY2:
          
#ifdef SKY_GPU2
          if ( gif_device.flags & GIF_FLAG_GS_ENABLED ) 
            {
              gs_data = (long long) input[1] << 32 | (long long) input[0];

              if (!( gif_device.flags & GIF_FLAG_GS_OPEN ))
                {
                  GS_InitSim ();
                  GS_OpenSim ("Sky Simulator",640,480,1,0);
                  gif_device.flags |= GIF_FLAG_GS_OPEN;
                }

              /* Write the data to the requested register.           */
              return_code = GS_PutCtlPort (addr, gs_data);

              /* Anything else is considered a failure.              */      
              if ( return_code != 1 )
                sim_io_eprintf (current_state,"GS Error: GS_PutCtlPort(%08lx,%16Lx) returned %d.\n", 
                                addr,gs_data,return_code);
            }
          else
            sim_io_printf (current_state,"%08lx %08lx%08lx\n",addr,input[1],input[0]);
#else
	    sim_io_printf (current_state,"%08lx %08lx%08lx\n",addr,input[1],input[0]);
#endif
          break;

        /* All other register values are ignored.                    */
        case GS_REG_SMODE1:  
        case GS_REG_SMODE2:  
        case GS_REG_SRFSH:  
        case GS_REG_SYNCH1: 
        case GS_REG_SYNCH2:  
        case GS_REG_SYNCV:      
        case GS_REG_EXTBUF:   
        case GS_REG_EXTDATA:   
        case GS_REG_EXTWRITE:   
        case GS_REG_BGCOLOR:   
        case GS_REG_CSR:   
        case GS_REG_IMR:   
        case GS_REG_BUDIR:   
        case GS_REG_SIGID:   
        case GS_REG_LABELID:   
        case GS_REG_SYSCNT:   
        default:
          if ( indebug ("gs"))
            GS_DEBUG ("GS warning: Invalid register write. Address: %08lx.\n",addr);  
         
          break;
        } 
    }
  else 
    /* Not even close to the valid range. Should never occur.        */
    ASSERT (0);        
    
  return (nr_bytes);
}


/* Describe and register GS to the simulation environment.           */
void 
gs_attach ( SIM_DESC sd )    
{
  /* GS registers.                                                   */
  sim_core_attach (sd,
                   NULL, 
                   0, 
                   access_read_write, 
                   0,
                   GS_REGISTER_WINDOW_START, 
                   GS_REGISTER_WINDOW_SIZE,
                   0, 
                   &gs_device.dev, 
                   NULL);
}  

/* Runtime command option handling.                                  */
void
gs_options( struct GS_devicefull *gs,
            unsigned_4  option,
            char       *input_string )
{            
  switch ( option ) 
    {
    /* --sky-debug-file gs=file                                     */
    case SKY_OPT_DEBUG_NAME:
      if ( gs->debug_file != NULL ) 
        {
          fclose (gs->debug_file);
          gs->debug_file = NULL;
        }
      sky_store_file_name (&gs->debug_file_name,input_string);
      break;
  
    /* Simulator is closing.                                         */
    case SKY_OPT_CLOSE:
      if ( gs->debug_file != NULL )
        {  
          fclose (gs->debug_file);
          gs->debug_file = NULL;
        }
      break;                                     
      
    default:
      ASSERT (0);
      break;
    }
}                                                            


void 
gs_reset ()    
{
  /* XXX: anything to do here? */
}
