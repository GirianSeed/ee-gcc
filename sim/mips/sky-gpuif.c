/*  Copyright (C) 1998, Cygnus Solutions                              */

#include "sim-main.h"
#include "sky-device.h"
#include "sky-gpuif.h"
#include "sky-gs.h"
#include "sky-vif.h"
#include "sky-dma.h"
#include "sky-gdb.h"
#include "sim-assert.h"
#include "sky-indebug.h"

#ifdef SKY_GPU2
#include "libgpu2.h"
#endif

/* Internal function declarations.                                   */
static int  gif_io_read_buffer (device *, void *, int, address_word,
                                unsigned, sim_cpu *, sim_cia);
static int  gif_io_write_buffer (device *, const void *, int, address_word,
                                 unsigned, sim_cpu *,  sim_cia);
static int  gif_add_data_to_path_queue (struct GIF_devicefull *, GIF_path *,
                                        const void *, int);
static void gif_consume_queued_data (struct GIF_devicefull *, GIF_path *);
static int  gif_arbiter (struct GIF_devicefull *, GIF_path *);
static void gif_reset_stat_path_bits (struct GIF_devicefull *, unsigned_4);
static void gif_process_giftag (struct GIF_devicefull *, GIF_path *);
static void gif_process_new_active_path ( struct GIF_devicefull *, int);
static void gif_packed_mode (struct GIF_devicefull *, GIF_path *);
static void gif_reglist_mode (struct GIF_devicefull *, GIF_path *);
static void gif_image_mode (struct GIF_devicefull  *, GIF_path *);
static void gif_send_data_to_gs (struct GIF_devicefull *, GIF_path *);
static void gif_write_gs_display_buffer (struct GIF_devicefull *);
static void gif_trace_file_preprocessing (struct GIF_devicefull *, GIF_path *);
static void gif_trace_file_postprocessing (struct GIF_devicefull *, GIF_path *);
static void gif_advance_current_path_queue (struct GIF_devicefull *, GIF_path *);
static void gif_output_giftag (struct GIF_devicefull *, GIF_path *, char * );

const char *trace_default = "gif.dvpasm";
const char *output_file_default = "gsinput.dat";
const char *trace_mode[3] = {"GIFpacked","GIFreglist","GIFimage"};
const char *trace_regs[16] = {"PRIM","RGBAQ","ST","UV","XYZF2","XYZ2",
                              "TEX0_1","TEX0_2","CLAMP_1","CLAMP_2",
                              "XYZF","RES","XYZF3","XYZ3","A_D","NOP"};

/* GIF device definition and initialization                          */
struct GIF_devicefull gif_device =
{
  { "gif",                               /* Device name              */
     &gif_io_read_buffer,                /* Call location for read   */
     &gif_io_write_buffer,               /* Call location for write  */
  },
  {GIF_PATH1,0,0,0,{0},0,0,0,0,0,0,0,1.0}, /* PATH1 tag and queue    */
  {GIF_PATH2,0,0,0,{0},0,0,0,0,0,0,0,1.0}, /* PATH2 tag and queue    */
  {GIF_PATH3,0,0,0,{0},0,0,0,0,0,0,0,1.0}, /* PATH3 tag and queue    */
#ifdef SKY_GPU2
  GIF_FLAG_GS_ENABLED,                   /* General usage flags      */
#else
  0,                                     /* General usage flags      */
#endif
  {0,0,0,0,0,0,0,0,0,0,0,0},             /* GIF register bank        */
  0,                                     /* Local GIF_P3CNT register */
  0,                                     /* Mode of current transfer */
  0,                                     /* Queue quadword pc        */
  {0,{0,0,0,0},{0,0,0,0}},               /* GS refresh buffers       */
  NULL,                                  /* GIF trace file           */
  NULL,                                  /* GIF trace file name      */
  NULL,                                  /* GIF debug file           */
  NULL,                                  /* GIF debug file name      */
  NULL,                                  /* GIF output file          */
  NULL,                                  /* GIF output file name     */
  NULL                                   /* VIF disassembly path     */
}; 


/* Describe and register the GIF to the simulation environment.      */
void 
gif_attach ( SIM_DESC sd )    
{
  /* GIF registers.                                                  */
  sim_core_attach (sd, NULL, 0, access_read_write, 0, 
                   GIF_REGISTER_WINDOW_START, GIF_REGISTER_WINDOW_SIZE,
                   0, &gif_device.dev, NULL);
  
  /* GIF PATH 1 queue (from VU).                                     */
  sim_core_attach (sd, NULL, 0, access_write, 0, 
                   GIF_PATH1_FIFO_ADDR, GIF_QUEUE_BYTES,
                   0, &gif_device.dev, NULL);

  /* GIF PATH 2 queue (from VIF).                                    */
  sim_core_attach (sd, NULL, 0, access_write, 0, 
                   GIF_PATH2_FIFO_ADDR, GIF_QUEUE_BYTES,
                   0, &gif_device.dev, NULL);
  
  /* GIF PATH 3 queue (from main)                                    */
  sim_core_attach (sd, NULL, 0, access_write, 0,
                   GIF_PATH3_FIFO_ADDR, GIF_QUEUE_BYTES,
                   0, &gif_device.dev, NULL);
}


/* Reset the GIF component to initial values.                        */ 
void
gif_reset ()
{
  struct GIF_devicefull *gif = & gif_device;
  const char *gif_reset_string = "\n\n\t******** GIF RESET ********\n\n\n";
  
  /* Reset all registers and control variables to their  */
  /* original state.                                     */
  memset (&gif->regs[0],0,GIF_REGISTER_BYTES * GIF_NUM_REGS);
  gif->image_p3cnt = 0;
  gif->active_mode = 0;
  gif->pc = 0;
  
  /* Reset the queues.                                   */
  GIF_RESET_QUEUE (gif->path1);             
  GIF_RESET_QUEUE (gif->path2);             
  GIF_RESET_QUEUE (gif->path3); 
  if ( gif->vif_path != NULL )
    gif->vif_path->tag_processed = 0; 
  
  /* Send a RESET message to any open files.             */
  if ( gif->trace_file != NULL )
    fprintf (gif->trace_file,gif_reset_string);
  if ( gif->debug_file != NULL )
    fprintf (gif->debug_file,gif_reset_string);
  if ( gif->output_file != NULL )
    fprintf (gif->output_file,gif_reset_string);

  /* Only the simulator window and user options live     */
  /* through a reset.                                    */                                        
  gif->flags &= ( GIF_FLAG_GS_OPEN | GIF_FLAG_OPTION_MASK ); 
}


/* Handle a read request of GIF specific memory.  Only a register    */
/* read is valid.                                                    */
int
gif_io_read_buffer ( device *me,
                     void *dest,
                     int space,
                     address_word addr,
                     unsigned nr_bytes,
                     sim_cpu *processor,
                     sim_cia cia )
{
  int reg_index = 0;
  struct GIF_devicefull *gif = (struct GIF_devicefull *) me;

  /* Validate a register read request.                               */ 
  if (( addr >= GIF_REGISTER_WINDOW_START ) &&
      ( addr <= GIF_REGISTER_WINDOW_END ))
    {
      unsigned_4 value = 0;

      /* The register read must be 32 bits, aligned.                 */
      if (( nr_bytes != GIF_REGISTER_BYTES ) &&
          ( indebug ("gif")))
        GIF_DEBUG ("GIF warning: Read requested not size of register. Bytes: %d.\n",nr_bytes);

      if ( addr & 0x0f )
        { 
          if ( indebug ("gif"))              
            GIF_DEBUG ("GIF warning: Register read not aligned. Address: %08lx.\n",addr);

          addr &= ~0x0f;                   
        }

      /* Determine the index of the register in the local array.     */
      reg_index = GIF_REG_INDEX (addr);

      switch ( reg_index ) 
        {
        /* READ ONLY registers.                                      */
        case GIF_STAT:
        case GIF_P3CNT:   
        case GIF_P3TAG:  
          if (( reg_index == GIF_STAT ) ||
              (( gif->active_mode == GIF_MODE_IMAGE ) &&
               ((( gif->regs[GIF_STAT] & GIF_REG_STAT_APATH ) >> 10 ) == GIF_PATH3 )))
            value = H2T_4 (gif->regs[reg_index]);
          break;
      
        /* WRITE ONLY registers, non supported and fall throughs.    */
        case GIF_CTRL:
        case GIF_MODE:
        case GIF_RESRV:
        case GIF_TAG0:
        case GIF_TAG1:
        case GIF_TAG2:
        case GIF_TAG3:
        case GIF_CNT:
        case GIF_VIF_M3P:
        default:
          break;
        }

      memcpy (dest, &value, (nr_bytes > GIF_REGISTER_BYTES) ? GIF_REGISTER_BYTES : nr_bytes);
    }
  else
    /* Not even close to the valid range.  Should never occur as     */
    /* the queues were defined in gif_attach() to be write only.     */
    ASSERT (0);
  
  return (nr_bytes);
}


/* Handle a write request on GIF specific memory.                    */
int
gif_io_write_buffer ( device *me,
                      const void *source,
                      int space,
                      address_word addr,
                      unsigned nr_bytes,
                      sim_cpu *processor,
                      sim_cia cia )
{
  struct GIF_devicefull *gif = (struct GIF_devicefull *) me;

  /* Validate a register write request.                              */ 
  if (( addr >= GIF_REGISTER_WINDOW_START ) &&
      ( addr <= GIF_REGISTER_WINDOW_END ))
    {
      unsigned_4 input;
      unsigned_4 active_path; 
      unsigned_4 new_active_path; 
      unsigned_4 reg_index;
      unsigned_4 checkP3Q = 0;
      
      /* The register write must be 32 bits, aligned.                */
      if (( nr_bytes != GIF_REGISTER_BYTES ) &&
          ( indebug ("gif")))
        GIF_DEBUG ("GIF warning: Write not size of register. Bytes: %d.\n", nr_bytes);

      if ( addr & 0x0f )
        {
          if ( indebug ("gif")) 
            GIF_DEBUG ("GIF warning: Register write not aligned. Address: %08lx.\n", addr);
          
          addr &= ~0x0f;                   
        }    

      active_path = ( gif->regs[GIF_STAT] & GIF_REG_STAT_APATH ) >> 10; 

      input = T2H_4 ((( unsigned_4 *) source)[0]);

      /* Determine the index of the register in the local array.     */ 
      reg_index = GIF_REG_INDEX (addr);

      switch ( reg_index ) 
        {
        /* WRITE ONLY registers.                                     */
        /* The control register.  The only support is to reset the   */
        /* GIF.                                                      */
        case GIF_CTRL:
          if ( input & GIF_REG_CTRL_RST ) 
	         gif_reset ();
          break;

        /* Set transfer masks and interrupts.  These setting are     */
        /* mirrored in the GIF_STAT read only register.              */
        case GIF_MODE:
          if ( input & GIF_REG_MODE_M3R_MASK )
            {
              if (!( gif->regs[GIF_STAT] & GIF_REG_STAT_M3R ))
                {
                  /* If PATH3 transfer is under way, the mask must   */
                  /* must for transfer completion.                   */
                  if ( active_path == GIF_PATH3 ) 
                    gif->flags |= GIF_FLAG_M3R_PENDING;
              
                  gif->regs[GIF_STAT] |= GIF_REG_STAT_M3R;
                }  
            }
          else
            {
              gif->regs[GIF_STAT] &= ~GIF_REG_STAT_M3R;   
              
              /* This is the case where the path is currently idle   */
              /* and there maybe data waiting in PATH3.              */
              if (( active_path == GIF_IDLE ) &&
                  ( gif->path3.candidate ))
                {
                  gif_arbiter (gif, &gif->path3);
                  new_active_path = ( gif->regs[GIF_STAT] & GIF_REG_STAT_APATH ) >> 10; 
                  
                  if ( new_active_path == GIF_PATH3 ) 
                    gif_consume_queued_data (gif, &gif->path3);
                }
            }
            
          if ( input & GIF_REG_MODE_IMT_TERM )
            {
              if (!( gif->regs[GIF_STAT] & GIF_REG_STAT_IMT ))
                {
                  /* The IMT can be set if we are IDLE.              */
                  if ( active_path == GIF_IDLE )  
                    {
                      gif->regs[GIF_STAT] |= GIF_REG_STAT_IMT;
                      gif->flags &= ~(GIF_FLAG_IMT_RESET_PEND | GIF_FLAG_IMT_SET_PEND);
                    }
                  else
                    gif->flags |= GIF_FLAG_IMT_SET_PEND;
                }  
            }  
          else
            {
              gif->flags |= GIF_FLAG_IMT_RESET_PEND;
              
              /* Reset the active path.                              */  
              if ( active_path == GIF_IDLE )
                {
                  gif->regs[GIF_STAT] &= ~(GIF_REG_STAT_IMT | GIF_REG_STAT_IP3);
                  gif->flags &= ~(GIF_FLAG_IMT_RESET_PEND | GIF_FLAG_IMT_SET_PEND);
                  
                  if ( gif->path2.candidate ) 
                    {
                      gif_arbiter (gif, &gif->path2);
                      new_active_path = ( gif->regs[GIF_STAT] & GIF_REG_STAT_APATH ) >> 10; 
                  
                      /* This happens when PATH2 direct_hl is held   */
                      /* up on an IMT.                               */
                      if ( new_active_path == GIF_PATH2 ) 
                        gif_consume_queued_data (gif, &gif->path2);
                    }
                }
            }
            
          /* GIF_STAT.P3Q may have toggled.                          */
          checkP3Q = 1;
          
          break;
       
        /* The GIF_STAT register is READ ONLY.  However, the M3P bit */
        /* of the register is set by the VIF through this register.  */                                       
        case GIF_VIF_M3P:
          if ( input & GIF_REG_STAT_M3P )
            gif->regs[GIF_STAT] |=  GIF_REG_STAT_M3P;
          else
            gif->regs[GIF_STAT] &= ~GIF_REG_STAT_M3P; 
           
          /* GIF_STAT.P3Q may have toggled.                          */
          checkP3Q = 1;

          break;

        /* READ ONLY registers and fall through.                     */
        case GIF_STAT:
        case GIF_TAG0:
        case GIF_TAG1:
        case GIF_TAG2:
        case GIF_TAG3:
        case GIF_CNT:
        case GIF_P3CNT:
        case GIF_P3TAG:
        default:
          break;
        } 
              
      /* GIF_STAT.P3Q toggles based on other bits.                   */
      if ( checkP3Q ) 
        {
          /* Set the GIF_STAT.P3Q bit if waiting in PATH3            */
          if ( gif->regs[GIF_STAT] & ( GIF_REG_STAT_M3R | GIF_REG_STAT_M3P ))
            gif->regs[GIF_STAT] |= GIF_REG_STAT_P3Q;
          else
            {
              /* You don't wait if the active path is PATH3 / idle.  */
              if (( active_path == GIF_IDLE ) ||
                  ( active_path == GIF_PATH3 ))
                gif->regs[GIF_STAT] &= ~GIF_REG_STAT_P3Q;
              else
                {
                  GIF_path *active ;
                   
                  if ( active_path == GIF_PATH1 )
                    active = &gif->path1;
                  else
                    active = &gif->path2;
                     
                  /* No waiting if end of packet encountered.        */  
                  if ( active->eop )
                    gif->regs[GIF_STAT] &= ~GIF_REG_STAT_P3Q;
                  else
                    gif->regs[GIF_STAT] |= GIF_REG_STAT_P3Q;
                }
            }    
        }
    }
  else 
    {
      /* Validate and consume a queue write request.                 */
      if (( addr >= GIF_PATH3_FIFO_ADDR ) &&
          ( addr < ( GIF_PATH1_FIFO_ADDR + GIF_QUEUE_BYTES )))
        {
          GIF_path *path = 0;

          /* The queue write must be 128 bits, aligned.              */
          if (( nr_bytes != GIF_QUEUE_BYTES ) &&
              ( indebug ("gif")))
            GIF_DEBUG ("GIF warning: Write not size of queue. Bytes: %d.\n", nr_bytes);

          if ( addr & 0x0f )
            { 
              if ( indebug ("gif")) 
                GIF_DEBUG ("GIF warning: Queue write not aligned. Address: %08lx.\n", addr);
         
              addr &= ~0x0f;                   
            }

          /* Determine the correct queue to place the data.          */
          if ( addr == GIF_PATH1_FIFO_ADDR )
            path = &gif->path1;
          else if ( addr == GIF_PATH2_FIFO_ADDR )
            path = &gif->path2;
          else if ( addr == GIF_PATH3_FIFO_ADDR ) 
            path = &gif->path3;
          else
            ASSERT (0);
         
          if ( path != NULL )
            {
              int consume_data = 0;

              /* Add this data to the PATH queue.  If the current    */
              /* queue row becomes full, we can process it.          */
              consume_data = gif_add_data_to_path_queue (gif, path, source, nr_bytes);
              
              if ( consume_data ) 
                gif_consume_queued_data (gif, path);
            }
        }
      else
        /* Not even close to the valid range.  Should never occur.   */
        ASSERT (0);
    }
  
  return (nr_bytes);
}


/* Add the data to the appropriate PATH queue.                       */
int 
gif_add_data_to_path_queue ( struct GIF_devicefull *gif,
                             GIF_path *path,
                             const void *source,
                             int nr_bytes )
{
  unsigned_4 full_quadword = 0;
  unsigned_4 *current_row;

  /* Reallocate the queue to a larger size if the received data will */
  /* overflow the queue.                                             */
  if ( path->queue_length == path->queue_row )
    {
      GIF_values *new_GIF_input ;

      /* Grow the queue.                                             */
      unsigned_4 new_length = path->queue_length + GIF_QUEUE_INCREMENT;

      new_GIF_input = zalloc (new_length * GIF_PATH_QUEUE_SIZE);
       
      ASSERT (new_GIF_input != NULL);

      /* Copy the contents of the current queue to the new, larger   */
      /* queue and free the current queue.                           */
      if ( path->queue_length != 0 ) 
        {
          memcpy (new_GIF_input,path->queue,GIF_PATH_QUEUE_SIZE * path->queue_length);
          zfree (path->queue);
        }
       
      /* Reset the queue to point to the new, larger queue.          */
      path->queue_length = new_length;
      path->queue = new_GIF_input;
    }

  current_row = ( unsigned_4 * ) &(path->queue[path->queue_row].data);

  /* Handle partial/full writes ( 4/8/16 bytes ) to the GIF queues.  */  
  switch ( nr_bytes )
    {
    case 4:
      if ( path->partial_index < 4 )
      {
        current_row[path->partial_index] = T2H_4 (((unsigned_4 *) source)[0]);
        path->partial_index++;  
      }
      else
        ASSERT (0);
      break;

    case 8:
      if ( path->partial_index < 3 )
      {
        current_row[path->partial_index] = T2H_4 (((unsigned_4 *) source)[0]);
        path->partial_index++;
        current_row[path->partial_index] = T2H_4 (((unsigned_4 *) source)[1]);
        path->partial_index++;
      }
      else
        ASSERT (0);
      break;
  
    case 16:
      current_row[0] = T2H_4 (((unsigned_4 *) source)[0]);
      current_row[1] = T2H_4 (((unsigned_4 *) source)[1]);
      current_row[2] = T2H_4 (((unsigned_4 *) source)[2]);
      current_row[3] = T2H_4 (((unsigned_4 *) source)[3]);
      path->partial_index = 4;
      break;

    default:    
      break;  
    }

  /* An full partial index represents a full quadword of data.       */ 
  if ( path->partial_index == 4 )
    {
      unsigned_4 throw_away_data = 0;

      /* Check to see if the DMAC has sent over a DMA tag... ignore  */
      /* it.                                                         */
      if ( path->name == GIF_PATH3 ) 
        GIF_MEM_READ (DMA_D2_PKTFLAG, &throw_away_data, 4) ;

      if (!( throw_away_data ))
        {
          path->queue[path->queue_row].flags = 0;
          path->queue[path->queue_row].pc = gif->pc;
          gif->pc++;
      
          /* Check to see if this is a PATH2 direct_hl transfer      */
          /* ( which can not be interrupted by GIF_MODE.IMT )        */
          if ( path->name == GIF_PATH2 ) 
            {
              unsigned_4 vif_code = 0;

              GIF_MEM_READ (VIF1_REGISTER_WINDOW_START+(VIF_REG_CODE * 16),
                            &vif_code,4);
        
              if (( vif_code & GIF_VIF_CODE_DIRECT_HL ) == GIF_VIF_CODE_DIRECT_HL ) 
                path->queue[path->queue_row].flags |= GIF_PATH_DIRECT_HL;
            }
 
          path->queue_row++;
          full_quadword = 1;
        }

      path->partial_index = 0;
    }

  return (full_quadword);
}


/* Unpack and send the queued data to GS for consumption.            */
void
gif_consume_queued_data ( struct GIF_devicefull *gif,
                          GIF_path *path )
{
  unsigned_4 active_path = 0;
  unsigned_4 queue_data = GIF_QUEUE_DATA;

  /* Read in the GIF tag if not yet encountered for this path.       */
  if (!( path->tag_processed )) 
    {
      gif_process_giftag (gif, path);
      queue_data = GIF_QUEUE_TAG;
    }
  else
    path->candidate = 1;
  
  active_path = ( gif->regs[GIF_STAT] & GIF_REG_STAT_APATH ) >> 10;
 
  switch ( active_path ) 
    {  
    /* If no transfers are currently in progress, this may be the    */
    /* start of an active transfer.                                  */
    case GIF_IDLE:
      path->data_flags |= GIF_PATH_FORCE_PATH;
      active_path = gif_arbiter (gif,path);
      break;

    case GIF_PATH1:
    case GIF_PATH2:
      break;

    /* If PATH3 is the active path and the mode is image, reset the  */
    /* GIF_P3CNT register.                                           */
    case GIF_PATH3:
      if (( path->name != GIF_PATH3 ) &&
          ( gif->active_mode == GIF_MODE_IMAGE ))
        gif->regs[GIF_P3CNT] = gif->image_p3cnt;
      break;

    default:
      ASSERT (0);
      break;
    }
 
  /* If the data passed to the arbiter is queue data.                */
  if (( queue_data  == GIF_QUEUE_DATA ) &&
      ( active_path == path->name ))
    {
      unsigned_4 call_arbiter = 0;

      /* Update PATH3 counters ( Shadow of GIF_P3CNT and the         */
      /* quadword counter when GIF_STAT.IMT is set ).                */
      if (( path->name == GIF_PATH3 ) &&
          ( gif->active_mode == GIF_MODE_IMAGE ) &&
          ( gif->image_p3cnt != 0 ))
        gif->image_p3cnt--;

      /* Data is transferred to GS if either the end of primitive,   */
      /* end of packet OR IMT burst condition occurs.                */
      path->data_flags = 0;
      
      if (( path->name == GIF_PATH3 )                 &&
          ( gif->regs[GIF_STAT] & GIF_REG_STAT_IMT )  &&
          (( path->queue_row >= GIF_IMAGE_IMT_BURST ) ||
           ( path->queue_row >= path->iterations - path->cur_iteration )))
        {
          call_arbiter++;
          path->cur_iteration += GIF_IMAGE_IMT_BURST;
          path->data_flags |= GIF_PATH_END_OF_IMT_BURST;

          if ( path->cur_iteration >= path->iterations )
            {
              path->data_flags |= GIF_PATH_END_OF_PRIMITIVE;
              if ( path->eop )
                path->data_flags |= GIF_PATH_END_OF_PACKET;
            }
        }
      else
        {
          if ( path->queue_row >= path->iterations ) 
            {
              path->data_flags |= GIF_PATH_END_OF_PRIMITIVE;
          
              if ( path->eop ) 
                {
                  path->data_flags |= GIF_PATH_END_OF_PACKET;
                  call_arbiter++;
                }
            }
        }
          
      if ( path->data_flags ) 
        {
          gif_trace_file_preprocessing (gif, path);

          /* Unpack the data according to the mode requested.        */
          switch ( path->flag ) 
            {
            case GIF_MODE_PACKED: 
              gif_packed_mode (gif, path);
              break;
                                   
            case GIF_MODE_REGLIST:
              gif_reglist_mode (gif, path);
              break;
                   
            case GIF_MODE_IMAGE:
              gif_image_mode (gif, path);
              break;
         
            default:
              ASSERT (0) ;
              break;
            }
 
          gif_trace_file_postprocessing (gif, path);

          /* We have sent data out to GS.  Advance the queue on this */
          /* path and determine the new active path (if changed).    */
          gif_advance_current_path_queue (gif, path);

          if ( call_arbiter ) 
            {
              int new_active_path;

              new_active_path = gif_arbiter (gif, path);
              
              /* Check to see if the new active path has data that   */
              /* can be sent to GS during this cycle as well.        */
              if ( new_active_path != GIF_IDLE ) 
                gif_process_new_active_path (gif, new_active_path );
            }
          else
            {
              path->data_flags = 0; 
              
              /* This path is still the active transfer path...      */
              /* There may be some consumable data in the queue.     */
              if ( path->candidate ) 
                gif_consume_queued_data (gif, path);
            }
        }
    }
}


/* Arbiter - Determine correct active path.                          */  
int 
gif_arbiter ( struct GIF_devicefull *gif,
              GIF_path *path )
{  
  GIF_path *new_path = 0;

  /* Certain flags are set/reset at end of packet.                   */
  if ( path->data_flags & ( GIF_PATH_END_OF_PACKET | GIF_PATH_FORCE_PATH))
    {
      if ( gif->flags & GIF_FLAG_IMT_SET_PEND ) 
        {
          gif->regs[GIF_STAT] |= GIF_REG_STAT_IMT;
          gif->flags &= ~GIF_FLAG_IMT_SET_PEND;
        }

      if ( gif->flags & GIF_FLAG_IMT_RESET_PEND )  
        {
          gif->regs[GIF_STAT] &= ~( GIF_REG_STAT_IMT | GIF_REG_STAT_IP3 );
          gif->flags &= ~GIF_FLAG_IMT_RESET_PEND;
        }
     
      if ( gif->flags & GIF_FLAG_M3R_PENDING ) 
        {
          gif->regs[GIF_STAT] |= ( GIF_REG_STAT_M3R | GIF_REG_STAT_P3Q );
          gif->flags &= ~GIF_FLAG_M3R_PENDING;
        }
    }
   
  if ( path->data_flags & GIF_PATH_FORCE_PATH ) 
    {
      /* There are two instance when a path can not be chosen.  If   */
      /* it is a PATH2 direct_hl and GIF_STAT.IMT is set OR a paused */
      /* PATH3.                                                      */
      if ((( path->name == GIF_PATH2 )                &&
           ( gif->regs[GIF_STAT] & GIF_REG_STAT_IMT ) &&
           ( path->direct_hl ))                       ||
          (( path->name == GIF_PATH3 )                &&
           ( gif->regs[GIF_STAT] & ( GIF_REG_STAT_M3R | GIF_REG_STAT_M3P )))) 
        new_path = 0;
      else
        new_path = path ;
    }
  else
    {
      /* The arbiter priority checks PATH1 and PATH2 first.          */
      if (( gif->path1.candidate ) ||
          ( gif->path2.candidate ))
        {
          /* The pc represents the time the quadword at the top of   */
          /* path queue was received.                                */
          if ( gif->path1.pc < gif->path2.pc ) 
            {
              if ( gif->path1.candidate ) 
                new_path = &gif->path1;
              else
                {
                  if ( gif->path2.candidate ) 
                    {
                      if (( gif->regs[GIF_STAT] & GIF_REG_STAT_IMT ) &&
                          ( gif->path2.direct_hl ))
                        new_path = 0;
                      else
                        new_path = &gif->path2;
                    }
                }
            }
          else
            {
              if ( gif->path2.candidate )
                {
                  if (( gif->regs[GIF_STAT] & GIF_REG_STAT_IMT ) &&
                      ( gif->path2.direct_hl ))
                     new_path = 0;
                   else
                     new_path = &gif->path2;
                }
       
              if (( new_path == 0 ) &&
                  ( gif->path1.candidate )) 
                new_path = &gif->path1;
            }
        }

      /* PATH3 has the lowest priority.                              */
      if (( new_path == 0 ) && 
          ( gif->path3.candidate ))
        {
          /* A paused PATH3 can not be chosen.                       */
          if (!( gif->regs[GIF_STAT] & ( GIF_REG_STAT_M3R | GIF_REG_STAT_M3P )))
            new_path = &gif->path3;
        }
    }

  path->data_flags = 0;

  /* Reset the path status bits to reflect the arbiter's decision.   */
  gif_reset_stat_path_bits (gif, ( new_path == 0 ) ? GIF_IDLE : new_path->name);
  
  return (( new_path == 0 ) ? GIF_IDLE : new_path->name );
}


/* Reset the GIF_STAT register wait bits according to the path       */
/* currently transferring.                                           */
void
gif_reset_stat_path_bits ( struct GIF_devicefull *gif,
                           unsigned_4 path_name )
{ 
  /* Reset ALL queue wait and transfers bits of the GIF_STAT register.*/
  gif->regs[GIF_STAT] &= ~( GIF_REG_STAT_P1Q | GIF_REG_STAT_P2Q | GIF_REG_STAT_P3Q ); 
  gif->regs[GIF_STAT] &= ~GIF_REG_STAT_APATH;

  /* Update the queue wait bits according to the new path.           */
  gif->regs[GIF_STAT] |= ((( path_name << 10 ) & GIF_REG_STAT_APATH )
                           | GIF_REG_STAT_OPH );

  /* You only wait in the other queues if the eop bit is zero in the */
  /* active path's current gif tag.                                  */
  switch ( path_name ) 
    {
    case GIF_IDLE:
      /* Mark no transfer currently in progress.                     */
      gif->regs[GIF_STAT] &= ~GIF_REG_STAT_OPH;
      if ( gif->regs[GIF_STAT] & ( GIF_REG_STAT_M3R | GIF_REG_STAT_P3Q ))
        gif->regs[GIF_STAT] |= GIF_REG_STAT_P3Q;
      gif->active_mode = 0;   
      break;
      
    case GIF_PATH1:
      if ( gif->path1.eop == 0 )
        gif->regs[GIF_STAT] |= ( GIF_REG_STAT_P2Q | GIF_REG_STAT_P3Q );
      gif->active_mode = gif->path1.flag;
      break;

    case GIF_PATH2: 
      if ( gif->path2.eop == 0 )
        gif->regs[GIF_STAT] |= ( GIF_REG_STAT_P1Q | GIF_REG_STAT_P3Q );
      gif->active_mode = gif->path2.flag;
      break;

    case GIF_PATH3:
      if ( gif->path3.eop == 0 )
        gif->regs[GIF_STAT] |= ( GIF_REG_STAT_P1Q | GIF_REG_STAT_P2Q );
      gif->active_mode = gif->path3.flag;
      break;
        
    default:
      ASSERT (0);
      break;
    }

  /* If GIF_STAT.IMT is on and we are not on PATH3, set the          */
  /* PATH3 interrupt bit.                                            */
  if (( gif->regs[GIF_STAT] & GIF_REG_STAT_IMT ) &&
      (( path_name == GIF_PATH1 ) ||
       ( path_name == GIF_PATH2 )))
     gif->regs[GIF_STAT] |=  GIF_REG_STAT_IP3;
  else
     gif->regs[GIF_STAT] &= ~GIF_REG_STAT_IP3;
}

 
/* Read a GIF tag from the indicated queue and update the path       */
/* variables with the contents.                                      */                              
void
gif_process_giftag ( struct GIF_devicefull *gif,
                     GIF_path *path )
{
  unsigned_4 current_row = path->queue_row - 1;
  GIF_values *current = &(path->queue[current_row]);  
      
  /* Update tag variables with the contents of the GIFtag sent.      */
  path->tag_processed = 1;
  path->pc    = current->pc ;
  path->nloop = current->data[0] & GIF_BIT_MASK(15); 
  path->flag  = ( current->data[1] >> 26 ) & GIF_BIT_MASK(2);
  path->eop   = ( current->data[0] >> 15 ) & GIF_BIT_MASK(1);   
  path->cur_iteration = 0;
  
  /* When processing an IMT burst, a PATH2 direct_hl request will    */
  /* be honoured.                                                    */
  if ( current->flags & GIF_PATH_DIRECT_HL ) 
    path->direct_hl = 1;
  else
    path->direct_hl = 0;

  path->Q_register = 1.0;
      
  /* Update counters based on the transfer mode.                     */  
  if ( path->flag == GIF_MODE_IMAGE )
    {
      path->iterations = path->nloop;

      /* Update PATH 3 specific registers with tag information.      */
      if ( path->name == GIF_PATH3 ) 
        {
          gif->regs[GIF_P3TAG] = path->nloop | ( path->eop << 15 );
          gif ->image_p3cnt = path->nloop;
        }
    }
  else
    {
      int i;
      path->pre  = ( current->data[1] >> 14 ) & GIF_BIT_MASK(1);
      path->prim = ( current->data[1] >> 15 ) & GIF_BIT_MASK(11);
      path->nreg = ( current->data[1] >> 28 ) & GIF_BIT_MASK(4);
                    
      /* A zero number of registers gets reset to the max.           */
      if ( path->nreg == 0 ) 
        path->nreg = GS_HI_REGS;
            
      path->max_reg_index = path->nreg - 1;
      
      /* Load the local register bank (first eight, last eight).     */
      for ( i = 0 ; i < GS_LO_REGS; i++ )                          
        path->regs[i] = ( current->data[2] >> ( i * 4 )) & GIF_BIT_MASK(4);
            
      for ( i = GS_LO_REGS ; i < GS_HI_REGS; i++ ) 
        path->regs[i] = ( current->data[3] >> (( i - GS_LO_REGS ) * 4 )) & GIF_BIT_MASK(4);
            
      /* Iterations depend on size of data expected (128 bits vs     */
      /* 64 bits).                                                   */
      if ( path->flag == GIF_MODE_PACKED )
        path->iterations = path->nreg * path->nloop;
      else
        {
          /* In REGLIST mode, an odd iteration count is rounded.     */ 
          path->iterations = (( path->nreg * path->nloop ) + 1 ) / 2;
          path->reglist_odd = ( path->nreg * path->nloop ) % 2 ;
        }
    }

  /* A nloop of zero is acceptable but the next write to the path    */
  /* should be a new gif tag.                                        */
  if ( path->nloop == 0 ) 
    path->tag_processed = 0; 
  else
    path->candidate = 1;

  /* This tag was encountered on an empty queue.  Reset it.          */
  if ( current_row == 0 ) 
    path->queue_row = current_row;
}


/* Move the queue forward to reflect consumed data.                  */
void
gif_advance_current_path_queue ( struct GIF_devicefull *gif,
                                 GIF_path *path )
{
  /* Nothing left in the queue, just reset it...                     */
  if ( path->advance == path->queue_row ) 
    {  
      path->queue_row = 0;
      path->candidate = 0;
     
      /* Tag needs to be read at end of primitive or end of packet.  */
      if ( path->data_flags & (  GIF_PATH_END_OF_PRIMITIVE 
                               | GIF_PATH_END_OF_PACKET ))
        path->tag_processed = 0;
    }
  else
    {
      /* Queue contents must be moved up...                          */
      unsigned_4 current_row = path->queue_row;
      unsigned_4 entries_to_move = path->queue_row - path->advance;
      
      path->queue_row = path->advance ;
      
      /* If there is at least the start of a new primitive, read in  */
      /* the gif tag.                                                */
      if ( path->data_flags & GIF_PATH_END_OF_PRIMITIVE ) 
        {
          path->data_flags = 0;
          path->queue_row++;
          entries_to_move--;

          gif_process_giftag (gif, path);
        }

      /* The tag for the next primitive on this path was processed.  */
      if ( path->tag_processed ) 
        {
          unsigned_4 i;
          
          for ( i = 0; i < entries_to_move; i++ )
            path->queue[i] = path->queue[path->queue_row + i];
          
          path->queue_row = entries_to_move;
        }
      else
        {
          /* The tag was found to contain an invalid NLOOP (0), see  */
          /* if the next quadword is ok...                           */
          path->queue_row = current_row;
          path->advance++;
          gif_advance_current_path_queue (gif, path);
        }
    }

  path->advance   = 0;
  path->direct_hl = 0;
}

/* After the current path has completed the transfer, other paths    */
/* are checked to see if they are able to transfer.                  */
void
gif_process_new_active_path ( struct GIF_devicefull *gif,
                              int    path_name )
{
  GIF_path *path;                                                                       

  /* Grab a pointer to the current path.                             */
  if ( path_name == GIF_PATH1 ) 
    path = &gif->path1;
  else if ( path_name == GIF_PATH2 ) 
    path = &gif->path2;
  else
    path = &gif->path3;
          
  gif_consume_queued_data (gif, path);
}


/* Unpack a quadwords according to the PACKED mode specifications    */
/* and place data on the output to GS queue                          */
void
gif_packed_mode ( struct GIF_devicefull *gif,
                  GIF_path *path )
{
  unsigned_4 reg_index = 0;
  unsigned_4 i;
  GS_values *input = &(path->current_GS);

  /* Place the PRIM value on the queue if requested.                 */
  if ( path->pre == GIF_PRIM_ENABLE )
    {
      path->pre = GIF_PRIM_DISABLE;
       
      input->reg_value = GS_PRIM;
      input->data_high = 0;
      input->data_low  = path->prim;
      
      gif_send_data_to_gs (gif, path);
    }

  path->advance = path->iterations;
 
  for ( i = 0; i < path->advance; i++ ) 
    {
      GIF_TRACE_INPUT (i);
      /* Get the register value from the tag array.                  */
      input->reg_value = path->regs[reg_index];
     
      /* Unpack the data based on GS register value.                 */
      switch ( input->reg_value ) 
        { 
        case GS_PRIM:
          input->data_low  = path->queue[i].data[0] & GIF_BIT_MASK(11);
          input->data_high = 0;
          break;

        case GS_RGBAQ:
          input->data_low  = ( ( path->queue[i].data[0] & GIF_BIT_MASK(8) )         |
                              (( path->queue[i].data[1] & GIF_BIT_MASK(8) ) << 8  ) |
                              (( path->queue[i].data[2] & GIF_BIT_MASK(8) ) << 16 ) |
                              (( path->queue[i].data[3] & GIF_BIT_MASK(8) ) << 24 ) );
          /* The Q internal register is a VPU compliant float.       */
          input->data_high =  *((unsigned_4 *) &path->Q_register ) ;
          break;

        case GS_ST:
          input->data_low  = path->queue[i].data[0];
          input->data_high = path->queue[i].data[1];
          /* Store the Q internal register as a VPU compliant float. */
          path->Q_register = *((float *) &(path->queue[i].data[2]));
          break;

        case GS_UV: 
          input->data_low  = ( ( path->queue[i].data[0] & GIF_BIT_MASK(16) ) |
                              (( path->queue[i].data[1] & GIF_BIT_MASK(16) ) << 16 ));
          input->data_high = 0;
          break;

        case GS_XYZF2:
          input->data_low  = ( ( path->queue[i].data[0] & GIF_BIT_MASK(16) ) |
                              (( path->queue[i].data[1] & GIF_BIT_MASK(16) ) << 16 ));
          input->data_high = ( ( path->queue[i].data[2] >> 4 ) & GIF_BIT_MASK(24)) |
                             ((( path->queue[i].data[3] >> 4 ) & GIF_BIT_MASK(8) ) << 24 );
        
          /* There may be a request to send this as XYZF3 instead.   */
          if (( path->queue[i].data[3] >> 15 ) & GIF_BIT_MASK(1) )
            input->reg_value = GS_XYZF3;
          break;
            
        case GS_XYZ2:
           input->data_low  = ( ( path->queue[i].data[0] & GIF_BIT_MASK(16) ) |
                               (( path->queue[i].data[1] & GIF_BIT_MASK(16) ) << 16 ));
           input->data_high =  path->queue[i].data[2];

          /* There may be a request to send this as XYZ3 instead.    */
          if (( path->queue[i].data[3] >> 15 ) & GIF_BIT_MASK(1) )
            input->reg_value = GS_XYZ3;
          break;
  
        case GS_XYZF:
          input->data_low  = ( ( path->queue[i].data[0] & GIF_BIT_MASK(16) ) |
                              (( path->queue[i].data[1] & GIF_BIT_MASK(16) ) << 16 ));
          input->data_high = ( ( path->queue[i].data[2] >> 4 ) & GIF_BIT_MASK(24)) |
                             ((( path->queue[i].data[3] >> 4 ) & GIF_BIT_MASK(8) ) << 24 );
          break;
     
        case GS_A_D:
          input->data_low  = path->queue[i].data[0];
          input->data_high = path->queue[i].data[1];
          input->reg_value = path->queue[i].data[2];
          break;

        case GS_NOP:   /* Do nothing - it's a noop!                  */
          break;
          
        case GS_TEX0_1:
        case GS_TEX0_2:
        case GS_CLAMP_1:
        case GS_CLAMP_2:                  
        case GS_XYZF3:
        case GS_XYZ3:
          input->data_low  = path->queue[i].data[0];
          input->data_high = path->queue[i].data[1];
          break;
            
        case GS_RESERVED:
        default:
          input->data_low  = 0;
          input->data_high = 0;  
          break;
        }

      if ( input->reg_value != GS_NOP ) 
        gif_send_data_to_gs (gif, path);
  
      /* Update the register index ( wrap if required ).             */
      if ( reg_index == path->max_reg_index ) 
        reg_index = 0;
      else
        reg_index++;
    }
}


/* Unpack a double words according to the REGLIST mode specs and     */
/* place the data on the output to GS queue.                         */
void 
gif_reglist_mode ( struct GIF_devicefull *gif,
                   GIF_path *path )
{
  int i,x;
  unsigned_4 reg_index = 0;

  path->advance = path->iterations;

  /* Two loops are required as the outer loop is in 128 bit words    */
  /* and the inner loop is in 64 bit words.                          */
  for ( x = 0; x < path->advance; x++ ) 
    {
      GIF_TRACE_INPUT (x);
                     
      /* The data is provided in 2x64 bit chunks in REGLIST mode.    */
      for ( i = 0; i < 4; i+= GIF_REGLIST_CHUNKS ) 
        {
          path->current_GS.reg_value = path->regs[reg_index];
      
          /* Certain register values cause GIF not to send the data. */
          if (( path->current_GS.reg_value != GS_A_D ) &&
              ( path->current_GS.reg_value != GS_NOP ))
            {
              path->current_GS.data_high = path->queue[x].data[i+1];
              path->current_GS.data_low  = path->queue[x].data[i];
  
              gif_send_data_to_gs (gif, path);
            }

          /* On a REGLIST, it is possible to receive only 64 usable  */
          /* bits in the last quadword.                              */
          if (( x == ( path->iterations - 1 ) ) &&
              ( path->reglist_odd ))
            break;
       
          if ( reg_index == path->max_reg_index ) 
            reg_index = 0;
          else
            reg_index++;
        }
    }
}


/* Unpack a quadwords according to the IMAGE mode specifications     */
/* and send the data off to GS.                                      */
void
gif_image_mode ( struct GIF_devicefull *gif, 
                 GIF_path *path )
{
  unsigned_4 i;

  if ( path->data_flags & GIF_PATH_END_OF_IMT_BURST ) 
    {
      /* Look for the case where the end of packet / primitive has   */
      /* been reached and we have less than 8 quadwords to send.     */
      if ( path->data_flags & GIF_PATH_END_OF_PRIMITIVE ) 
        path->advance = GIF_IMAGE_IMT_BURST - ( path->cur_iteration - path->iterations );
      else
        path->advance = GIF_IMAGE_IMT_BURST;
    }
  else
    path->advance = path->iterations;
  
  for ( i = 0; i < path->advance; i++ ) 
    {
      GIF_TRACE_INPUT (i);
            
      /* Image mode goes to the Hardware register only.              */
      path->current_GS.reg_value = GS_HWREG ;                 

      /* Image mode comes in 128 bit runs, divide into two units.    */
      path->current_GS.data_high = path->queue[i].data[1];
      path->current_GS.data_low  = path->queue[i].data[0];

      gif_send_data_to_gs (gif, path);

      path->current_GS.data_high = path->queue[i].data[3];
      path->current_GS.data_low  = path->queue[i].data[2];

      gif_send_data_to_gs (gif, path);
    }
}


/* Check to see if the user has requested a trace file.              */
void
gif_trace_file_preprocessing ( struct GIF_devicefull *gif,
                               GIF_path *path )  
{

#ifdef SKY_GPU2
  /* Open the simulator only once.                                   */
  if ((!( gif->flags & GIF_FLAG_GS_OPEN )) &&
      ( gif->flags & GIF_FLAG_GS_ENABLED ))
    {
      GS_InitSim ();
      GS_OpenSim ("Sky Simulator", 640, 480, 1, 0);
      gif->flags |= GIF_FLAG_GS_OPEN;
    }
#endif

  /* Trace file requested.  Open it and write out the GIF tag.       */
  if ( path->trace_on )
    {
      /* Initial set up of the trace file.                           */
      if ( gif->trace_file == NULL )
        {
          sky_open_file (&gif->trace_file,gif->trace_file_name,
                        (char *) trace_default, _IOFBF);

          if ( gif->trace_file != NULL ) 
            fprintf (gif->trace_file,"\t.global dma_gif\n\ndma_gif:\n\n");
        }
        
      /* Reassemble the GIF tag and send it to the trace file.       */
      if ( gif->trace_file != NULL ) 
        {
          fprintf (gif->trace_file,
                   "DmaCnt *                                             ; Path%1d transfer \n",
                   path->name);

          gif_output_giftag (gif, path, NULL);
        }
    }
}
     

/* Check to see if the user has requested a refresh                  */
void
gif_trace_file_postprocessing ( struct GIF_devicefull *gif,
                                GIF_path *path )
{
  /* Check to see if the user has requested the refresh register     */
  /* to be hit at the end of packet.                                 */
  if (( path->data_flags & GIF_PATH_END_OF_PACKET ) &&
      ( gif->flags & GIF_FLAG_GIF_REFRESH  ))
    {
      /* Write to the GS display buffers before sending the refresh. */
      if ( gif->flags & GIF_FLAG_GS_USER_DEF ) 
           gif_write_gs_display_buffer (gif);
        
#ifdef SKY_GPU2
      if ( gif->flags & GIF_FLAG_GS_ENABLED )
        GS_PutPort (GS_REFRESH,0);
      else
        sim_io_printf (current_state,"%02lx %08lx %08lx \n",GS_REFRESH,0,0);
#else
      sim_io_printf (current_state,"%02lx %08lx %08lx \n",GS_REFRESH,0,0);
#endif
      /* Send the refresh to the GIF output file.                    */
      if (( gif->flags & GIF_FLAG_TRACE_OUTPUT ) &&
          ( gif->output_file != NULL ))
        fprintf (gif->output_file,"%02lx %08lx %08lx \n",GS_REFRESH,0,0);
    }

  /* Send the end of the GIF tag to the trace facility.              */
  if (( path->trace_on ) &&
      ( gif->trace_file != NULL ))
    fprintf (gif->trace_file,".endGif \n.EndDmaData \n\n");
}


/* Send the queued data to the GS library for consumption.           */
void 
gif_send_data_to_gs ( struct GIF_devicefull *gif,
                      GIF_path *path )
{    
  GS_values *input;
  long long sim_data;
  
  input = &(path->current_GS );

  /* Toggle the display buffer if the user has defined the refresh   */
  /* buffers and write to the GS registers.                          */
  if (( gif->flags & GIF_FLAG_GS_USER_DEF ) &&
      ( input->reg_value == GS_REFRESH ))
    {
      if ( gif->flags & GIF_FLAG_GS_DISPLAY1 ) 
        {
          gif->flags &= ~GIF_FLAG_GS_DISPLAY1;
          gif->flags |=  GIF_FLAG_GS_DISPLAY2;
        }
      else
        {
          gif->flags &= ~GIF_FLAG_GS_DISPLAY2;
          gif->flags |=  GIF_FLAG_GS_DISPLAY1;
        }

       gif_write_gs_display_buffer (gif);
    }

#ifdef SKY_GPU2
  if ( gif->flags & GIF_FLAG_GS_ENABLED )
    {
      sim_data = (long long) input->data_high << 32 | 
                 (long long) input->data_low;

      GS_PutPort (input->reg_value, sim_data);
    }
  else
    sim_io_printf (current_state,"%02lx %08lx %08lx \n",input->reg_value,input->data_high,
                   input->data_low);
#else
  sim_io_printf (current_state,"%02lx %08lx %08lx \n",input->reg_value,input->data_high,
                 input->data_low);
#endif         

  /* Send the GS data to the GIF output file if requested.           */
  if ( gif->flags & GIF_FLAG_TRACE_OUTPUT ) 
    {
      if ( gif->output_file == NULL ) 
        sky_open_file (&gif->output_file, gif->output_file_name,
                       (char *) output_file_default, _IOFBF );

      if ( gif->output_file != NULL ) 
        fprintf (gif->output_file, "%02lx %08lx %08lx \n",
                 input->reg_value,input->data_high,input->data_low);
    }
}

                                                          
/* Update the GS refresh buffers with data provided by the user.     */
void
gif_write_gs_display_buffer ( struct GIF_devicefull *gif )
{
  unsigned_4 index;
  GS_refresh *refresh;

  refresh = &(gif->refresh_values);

  /* Determined the correct set of buffers to use.                   */
  if ( gif->flags & GIF_FLAG_GS_DISPLAY1 )
    index = 0;
  else
    index = 2;
    
  /* Write to the GS registers requested by the user.                */
  GIF_MEM_WRITE (refresh->reg_address[index],&refresh->reg_value[index],8);
  GIF_MEM_WRITE (refresh->reg_address[index+1],&refresh->reg_value[index+1],8);
}


/* Runtime command option handling.                                  */
void
gif_options( struct GIF_devicefull *gif,
             unsigned_4 option,
             char       *input_string,
             unsigned_4 *address,
             long long  *values )
{
  unsigned_4 index;
  GIF_path   *path;
  GS_refresh *refresh;

  /* --enable-gs on|off - turn on|off GS library routines.           */
  /* --gs-refresh1 opts - defines the GS refresh 1 set of registers. */
  /* --gs-refresh2 opts - defines the GS refresh 2 set of registers. */
  /* --screen-refresh on|off - turn on|off GS screen refresh (0x7f). */
  /* --sky-debug-file gif=file - name of file to store debug info.   */
  /* --log gifx=on|off  - turn on|off GIF disassembly logging.       */ 
  /* --log-file gif=name - name of file to store GIF disassembly.    */
  /* --log gs=on|off    - turn on|off GIF output logging.            */
  /* --log-file gs=name - name of file to store GIF output.          */
  switch ( option ) 
    {
    /* --enable-gs on                                                */
    case SKY_OPT_GS_ENABLE:
      gif->flags |=  GIF_FLAG_GS_ENABLED;
      break;

    /* --enable-gs off                                               */
    case SKY_OPT_GS_DISABLE:
      gif->flags &= ~GIF_FLAG_GS_ENABLED;
      break;

    /* gs-refresh                                                    */
    case SKY_OPT_GS_REFRESH1:
    case SKY_OPT_GS_REFRESH2:
      if ( option == SKY_OPT_GS_REFRESH1 ) 
         index = 0;
      else
         index = 2;
       
      refresh = &gif->refresh_values;
      refresh->reg_address[index]   = address[0];
      refresh->reg_address[index+1] = address[1];
      refresh->reg_value[index]   = values[0];
      refresh->reg_value[index+1] = values[1];
      
      refresh->options += option;

      /* The option is only set when both refresh buffers are        */
      /* defined.                                                    */
      if ( refresh->options == ( SKY_OPT_GS_REFRESH1 + SKY_OPT_GS_REFRESH2 ))
        gif->flags |= GIF_FLAG_GS_USER_DEF;
      break;

    /* --screen-refresh on                                           */
    case SKY_OPT_GIF_REFRESH_ON:
      gif->flags |=  GIF_FLAG_GIF_REFRESH;
      break; 

    /* --screen-refresh off                                          */
    case SKY_OPT_GIF_REFRESH_OFF:
      gif->flags &= ~GIF_FLAG_GIF_REFRESH;
      break;
  
    /* --sky-debug-file gif=file                                     */
    case SKY_OPT_DEBUG_NAME:
      if ( gif->debug_file != NULL ) 
        {
          fclose (gif->debug_file);
          gif->debug_file = NULL;
        }
      sky_store_file_name (&gif->debug_file_name,input_string);
      break;
   
    /* --log gifx=on                                                 */
    /* --log gifx=off                                                */
    case SKY_OPT_TRACE_ON:
    case SKY_OPT_TRACE_OFF:
      /* Determine the paths to trace/untrace.                       */
      if ( strcmp (input_string,"gif") == 0 ) 
        path = NULL;
      else if ( strcmp (input_string,"gif1") == 0) 
        path = &gif->path1;
      else if ( strcmp (input_string,"gif2") == 0) 
        path = &gif->path2;
      else if ( strcmp (input_string,"gif3") == 0) 
        path = &gif->path3;
      
      if ( option == SKY_OPT_TRACE_ON )
        {
          /* A NULL path represents all paths.                       */
          if ( path == NULL ) 
            {
              gif->path1.trace_on = 1; 
              gif->path2.trace_on = 1; 
              gif->path3.trace_on = 1; 
            } 
          else
            path->trace_on = 1;
        }
      else
        {
          /* Option == SKY_OPT_TRACE_OFF.                            */
          if ( path == NULL ) 
            { 
              gif->path1.trace_on = 0; 
              gif->path2.trace_on = 0; 
              gif->path3.trace_on = 0; 
            }
          else
            path->trace_on = 0;

          /* Close up the file if all GIF tracing is turned off.     */
          if (( gif->path1.trace_on == 0 ) &&
              ( gif->path2.trace_on == 0 ) &&
              ( gif->path3.trace_on == 0 ))
            {
              fclose (gif->trace_file);
              gif->trace_file = NULL;
            }
        }
      break;

    /* --log-file gif-file                                           */  
    case SKY_OPT_TRACE_NAME:
      if ( gif->trace_file != NULL ) 
        {
          fclose (gif->trace_file);
          gif->trace_file = NULL;
        }
      sky_store_file_name (&gif->trace_file_name,input_string);
      break;

    /* --log gs=on                                                   */
    case SKY_OPT_GIF_OUTPUT_ON:
      gif->flags |= GIF_FLAG_TRACE_OUTPUT;
      break;
    
    /* --log gs=off                                                  */
    /* --log-file gs=file                                            */
    case SKY_OPT_GIF_OUTPUT_OFF:
    case SKY_OPT_GIF_OUTPUT_NAME:

      /* Close up the file when output is cancelled or name changed. */
      if ( gif->output_file != NULL ) 
        {
          fclose (gif->output_file);
          gif->output_file = NULL ;
        }
      
      if ( option == SKY_OPT_GIF_OUTPUT_OFF ) 
        gif->flags &= ~GIF_FLAG_TRACE_OUTPUT;
      else
        sky_store_file_name (&gif->output_file_name,input_string);
      
      break;

    /* Simulator is closing.                                         */
    case SKY_OPT_CLOSE:
      /* If the user has requested a log file, place the required    */
      /* ending syntax into the file before closing.  The values are */
      /* not set to NULL as a sim_open() must be called.             */
      if ( gif->trace_file != NULL )
        {
          fprintf (gif->trace_file,"DmaEnd 0 \n.EndDmaData \n");
          fclose (gif->trace_file);
          gif->trace_file = NULL;
        }
      if ( gif->debug_file != NULL ) 
        {
          fclose (gif->debug_file);
          gif->debug_file = NULL;
        }
      if ( gif->output_file != NULL ) 
        {
          fclose (gif->output_file);
          gif->output_file = NULL;
        }
      break;                                     
      
    default:
      ASSERT (0);  
      break;
    }
}


/* Format the GIFtag for the trace facility. --log gifx=on           */
void
gif_output_giftag ( struct GIF_devicefull *gif, 
                    GIF_path *path,
                    char *output_buffer ) 
{
  char prim_string[20];
  char reg_string[100];
  int  nloop;

  if ( path->prim ) 
    sprintf (prim_string,"PRIM=0x%08lx,",path->prim);
  else
    prim_string[0] = '\0';

  /* Dump out the registers.                                         */
  if ( path->nreg )
    {
      int i;
      int reg_size;
      int reg_size_total = 0;
      char temp_string[100];
      char *temp_string_ptr = temp_string;
      
      for ( i = 0; i < path->nreg ; i++ ) 
        {
          sprintf (temp_string_ptr,"%s,",trace_regs[path->regs[i]]);
          reg_size = strlen (trace_regs[path->regs[i]]) + 1;
          reg_size_total  += reg_size;
          temp_string_ptr += reg_size;
        }
      temp_string[reg_size_total - 1] = '\0';
      sprintf (reg_string,"REGS={%s},",temp_string);
    }
    else
      reg_string[0] = '\0';
  
  /* In the case of an IMT PATH3 image transfer, the NLOOP may not   */
  /* be the same as the GIFtag states.                               */
  if (( path->name == GIF_PATH3 ) &&
      ( gif->regs[GIF_STAT] & GIF_REG_STAT_IMT ))
    {
      /* Look for the case where the end of packet / primitive has   */
      /* been reached and we have less than 8 quadwords to send.     */
      if ( path->data_flags & GIF_PATH_END_OF_PRIMITIVE ) 
        nloop = GIF_IMAGE_IMT_BURST - ( path->cur_iteration - path->iterations );
      else
        nloop = GIF_IMAGE_IMT_BURST;
    }
  else
    nloop = path->nloop;
        
  /* Out goes the GIFtag.  The output to a file is for the --log gif */
  /* option, the output to a buffer is for --log vif.                */
  if ( output_buffer == NULL) 
    fprintf (gif->trace_file,"%s %s %s NLOOP=%d%s \n",
             trace_mode[path->flag],
             prim_string,
             reg_string,
             nloop,
             ( path->eop ) ? ", EOP" : "");
  else
    sprintf (output_buffer,"%s %s %s NLOOP=%d%s",
             trace_mode[path->flag],
             prim_string,
             reg_string,
             nloop,
             ( path->eop ) ? ", EOP" : "");
}


/* Format GIF data for the VIF disassembler.                         */
void
gif_disassemble_vif_data ( char *output_string,
                           quadword *data )
{
  GIF_path *path;
  
  /* Allocate and initialize a VIF specific path for common routine  */
  /* use.                                                            */
  if ( gif_device.vif_path == NULL )
    {
      gif_device.vif_path = zalloc (sizeof (GIF_path)); 
      ASSERT (gif_device.vif_path != NULL);

      gif_device.vif_path->queue =  zalloc ( sizeof(GIF_values));
      ASSERT (gif_device.vif_path->queue != NULL);

      gif_device.vif_path->name = GIF_PATH2;
    }
  
  path = gif_device.vif_path;

  path->queue[0].data[0] = ((unsigned_4 *) data)[0];
  path->queue[0].data[1] = ((unsigned_4 *) data)[1];
  path->queue[0].data[2] = ((unsigned_4 *) data)[2];
  path->queue[0].data[3] = ((unsigned_4 *) data)[3];
               
  path->queue_row = 1;

  /* If the GIFtag has been processed, VIF data follows,             */
  if ( path->tag_processed )
    {
      path->cur_iteration++;
      
      sprintf (output_string,".word  0x%08lx, 0x%08lx, 0x%08lx, 0x%08lx",
               path->queue[0].data[0],path->queue[0].data[1],
               path->queue[0].data[2],path->queue[0].data[3]);
               
      /* Reached the end of the primitive.                           */
      if (path->iterations == path->cur_iteration)
        {
          strcat (output_string,"\n\t.endGif");
          path->tag_processed = 0;
        }
    }
  else
    {
      /* This is the start of a new primitive, send out the GIFtag.  */
      gif_process_giftag (NULL,path);
      gif_output_giftag (&gif_device,path,output_string);
    }
}
