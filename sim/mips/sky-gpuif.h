/*  Copyright (C) 1998, Cygnus Solutions                               */

#ifndef SKY_GIF_H_
#define SKY_GIF_H_

#include "sim-main.h"
#include "sky-vif.h"

/* GIF register address space / boundaries.                            */ 
#define GIF_REGISTER_WINDOW_START 0x10003000
#define GIF_REGISTER_WINDOW_END   0x100037ff
#define GIF_REGISTER_WINDOW_SIZE (GIF_REGISTER_WINDOW_END - GIF_REGISTER_WINDOW_START)

/* Register addresses for use when reading / writing.                  */
#define GIF_REG_CTRL    0x10003000 /* Control                          */
#define GIF_REG_MODE    0x10003010 /* Mode setting                     */
#define GIF_REG_STAT    0x10003020 /* Status                           */
#define GIF_REG_RESRV   0x10003030 /* Reserved                         */
#define GIF_REG_TAG0    0x10003040 /* Previous GIF tag bits 31-0       */
#define GIF_REG_TAG1    0x10003050 /* Previous GIF tag bits 63-32      */
#define GIF_REG_TAG2    0x10003060 /* Previous GIF tag bits 95-64      */
#define GIF_REG_TAG3    0x10003070 /* Previous GIF tag bits 127-96     */
#define GIF_REG_CNT     0x10003080 /* Status part way through transfer */
#define GIF_REG_P3CNT   0x10003090 /* Counter of PATH3 image transfer  */
#define GIF_REG_P3TAG   0x100030a0 /* GIFtag of PATH3 image transfer   */
#define GIF_REG_VIF_M3P 0x100030b0 /* VIF update of GIF_STAT.M3P       */

/* Local index into the GIF device register array.                     */
#define GIF_NUM_REGS            12 /* Number of GIF registers          */
#define GIF_CTRL              0x00 /* Control                          */
#define GIF_MODE              0x01 /* Mode setting                     */
#define GIF_STAT              0x02 /* Status                           */
#define GIF_RESRV             0x03 /* Reserved                         */
#define GIF_TAG0              0x04 /* Previous tag bits 31-0     (NOT SUPPORTED) */
#define GIF_TAG1              0x05 /* Previous tag bits 63-32    (NOT SUPPORTED) */
#define GIF_TAG2              0x06 /* Previous tag bits 95-64    (NOT SUPPORTED) */
#define GIF_TAG3              0x07 /* Previous tag bits 127-96   (NOT SUPPORTED) */
#define GIF_CNT               0x08 /* Transfer status            (NOT SUPPORTED) */
#define GIF_P3CNT             0x09 /* Counter of PATH3 image transfer  */
#define GIF_P3TAG             0x0a /* GIFtag of PATH3 image transfer   */
#define GIF_VIF_M3P           0x0b /* VIF update of GIF_STAT.M3P       */
                                                                       
/* GIF_REG_CRTL register constants (writeable).                        */
#define GIF_REG_CTRL_RST      0x01 /* GIF reset                        */
#define GIF_REG_CTRL_ABT      0x04 /* GIF command reset          (NOT SUPPORTED) */
#define GIF_REG_CTRL_PSE      0x08 /* Transfer pause             (NOT SUPPORTED) */

/* GIF_REG_MODE register bits and constants (writeable).               */
#define GIF_REG_MODE_M3R_MASK   0x00000001 /* PATH3 mask status        */
#define GIF_REG_MODE_IMT_TERM   0x00000004 /* PATH3 image termination  */
#define GIF_REG_MODE_ERO_ENABLE 0x00000010 /* Detect ill DMA-tags(NOT SUPPORTED) */

/* GIF_REG_STAT register bits (read only).                             */
#define GIF_REG_STAT_M3R        0x00000001 /* PATH3 mask by register   */
#define GIF_REG_STAT_M3P        0x00000002 /* PATH3 mask by VIF        */
#define GIF_REG_STAT_IMT        0x00000004 /* PATH3 image mode term    */
#define GIF_REG_STAT_PSE        0x00000008 /* Pause transfer     (NOT SUPPORTED) */
#define GIF_REG_STAT_ERO        0x00000010 /* Detect ill DMA-tags(NOT SUPPORTED) */
#define GIF_REG_STAT_IP3        0x00000020 /* PATH3 interrupted        */
#define GIF_REG_STAT_P3Q        0x00000040 /* PATH3 waiting in queue   */
#define GIF_REG_STAT_P2Q        0x00000080 /* PATH2 waiting in queue   */
#define GIF_REG_STAT_P1Q        0x00000100 /* PATH1 waiting in queue   */
#define GIF_REG_STAT_OPH        0x00000200 /* Output path (current)    */
#define GIF_REG_STAT_APATH      0x00000c00 /* Transferring data path   */
#define GIF_REG_STAT_DIR        0x00001000 /* Direction          (NOT SUPPORTED) */   
#define GIF_REG_STAT_FQC        0x1f000000 /* GIF fifo valid     (NOT SUPPORTED) */

/* GIF mask of VIF_REG_CODE register bits.                             */
#define GIF_VIF_CODE_DIRECT_HL  0x51000000 /* DIRECT_HL mask on PATH2  */

/* GIF queue addresses.                                                */                                            
#define GIF_PATH1_FIFO_ADDR     0x10006020  /* Data from VU            */
#define GIF_PATH2_FIFO_ADDR     0x10006010  /* Data from VIF           */ 
#define GIF_PATH3_FIFO_ADDR     0x10006000  /* Data from CORE          */

/* GIF queue definition.                                               */
typedef struct {
  unsigned_4 flags;      /* Status flags associated with the row       */
  unsigned_4 pc;         /* Pseudo program counter for queue arrivals  */
  quadword   data;       /* Row values placed on queue                 */
} GIF_values;

/* GS input definition.                                                */
typedef struct {
  unsigned_4 reg_value;  /* GS register value                          */
  unsigned_4 data_high;  /* High order 32 bits of data                 */
  unsigned_4 data_low;   /* Low order 32 bits of data                  */
} GS_values;

/* GS data buffer defines determined by user.                          */
typedef struct {
  unsigned_4 options;        /* GS register option content flag        */
  unsigned_4 reg_address[4]; /* GS register addresses                  */
  long long  reg_value[4];   /* GS register values                     */
} GS_refresh;

#define GIF_PATH_REFRESH           0x00000001 /* Send refresh to GS    */
#define GIF_PATH_END_OF_PRIMITIVE  0x00000010 /* End of primitive      */
#define GIF_PATH_END_OF_PACKET     0x00000020 /* End of packet         */
#define GIF_PATH_END_OF_IMT_BURST  0x00000040 /* End of IMT burst      */
#define GIF_PATH_DIRECT_HL         0x00000100 /* Direct_hl transfer    */
#define GIF_PATH_FORCE_PATH        0x00001000 /* Force arbiter choice  */

#define GIF_QUEUE_DATA                1     /* Queue data encountered  */
#define GIF_QUEUE_TAG                 2     /* Tag encountered         */
#define GIF_QUEUE_INCREMENT        4096     /* GIF queue size increment*/

/* GIF path queue and tag structures, and associated constants.         */
typedef struct {

  unsigned_4 name;          /* Path name         0x01: PATH 1               
                                                 0x10: PATH 2 
                                                 0x11: PATH 3          */
  /* Queue structures.                                                 */
  GIF_values *queue;        /* Dynamic queue of GIF value structs      */
  unsigned_4 queue_length;  /* Length of the dynamic GIF queue         */
  unsigned_4 queue_row;     /* Current row within the GIF queue        */
  GS_values  current_GS;    /* Values to send to GS                    */
  unsigned_4 partial_index; /* Partial quadword queue write index      */
  unsigned_4 pc;            /* Arbiter counter for PATH data arrival   */
  unsigned_4 advance;       /* Number of rows to advance the queue     */
  unsigned_4 candidate;     /* Queue contains data                     */
  unsigned_4 data_flags;    /* Data transfer indicator                 */
  unsigned_4 trace_on;      /* Trace this path?                        */
  
  /* GIF tag variables.                                                */
  unsigned_4 tag_processed; /* Has the GIF tag been processed?         */
  float      Q_register;    /* Internal register in PACKED mode        */
  unsigned_4 nloop;         /* Number of loops through the data        */
  unsigned_4 eop;           /* End of packet     0: not end 1: end     */
  unsigned_4 pre;           /* PRIM field enable 0: ignore  1: enable  */
  unsigned_4 prim;          /* Data for PRIM                           */
  unsigned_4 flag;          /* Data format       0x00: PACKED       
                                                 0x01: REGLIST               
                                                 0x10: IMAGE
                                                 0x11: Reserved        */
  unsigned_4 nreg;          /* Register descriptor count               */
  unsigned_4 regs[16];      /* Bank of 16 registers                    */
  
  /* Miscellaneous indicators and flags.                               */
  unsigned_4 max_reg_index; /* Maximum index into the register array   */
  unsigned_4 iterations;    /* Number of iterations against the data   */
  unsigned_4 cur_iteration; /* Number of iterations currently performed*/
  unsigned_4 reglist_odd;   /* Possible dangling data on REGLIST       */
  unsigned_4 direct_hl;     /* PATH2 direct_hl transfer                */
} GIF_path;


/* Complete GIF device ( includes a tag for each valid PATH ).         */  
struct GIF_devicefull {
   
  device     dev;                   /* Standard device definition      */
  GIF_path   path1;                 /* Current PATH1 structure         */       
  GIF_path   path2;                 /* Current PATH2 structure         */
  GIF_path   path3;                 /* Current PATH3 structure         */
  int        flags;                 /* General flag area for GIF device*/
  unsigned_4 regs[GIF_NUM_REGS];    /* GIF register bank               */
  unsigned_4 image_p3cnt;           /* Local version of GIF_P3CNT      */
  unsigned_4 active_mode;           /* Current mode of transfer        */
  unsigned_4 pc;                    /* Data arrival counter            */
  GS_refresh refresh_values;        /* User defined GS refresh values  */
  FILE       *trace_file;           /* GIF trace output file           */
  char       *trace_file_name;      /* GIF trace file name             */
  FILE       *debug_file;           /* GIF debug output file           */
  char       *debug_file_name;      /* GIF debug file name             */
  FILE       *output_file;          /* GIF output file ( to GS )       */
  char       *output_file_name;     /* GIF output file name            */
  GIF_path   *vif_path;             /* VIF disassembly path structure  */
};

extern struct GIF_devicefull gif_device;

/* GIF general flags.                                                  */
#define GIF_FLAG_GS_OPEN        0x00000001 /* GS initialized           */
#define GIF_FLAG_M3R_PENDING    0x00000010 /* PATH3 mask pending       */
#define GIF_FLAG_IMT_SET_PEND   0x00000020 /* PATH3 IMT set pending    */
#define GIF_FLAG_IMT_RESET_PEND 0x00000040 /* PATH3 IMT reset pending  */

/* GIF user run-time option flags.                                     */
#define GIF_FLAG_GS_ENABLED     0x00100000 /* Runtime disable of GS    */
#define GIF_FLAG_GIF_REFRESH    0x00200000 /* GIF to send 0x7f         */
#define GIF_FLAG_TRACE_OUTPUT   0x00800000 /* Trace GIF output to file */
#define GIF_FLAG_GS_USER_DEF    0x01000000 /* User defined GS refresh  */
#define GIF_FLAG_GS_DISPLAY1    0x02000000 /* GS display buffer 1      */
#define GIF_FLAG_GS_DISPLAY2    0x04000000 /* GS display buffer 2      */
#define GIF_FLAG_OPTION_MASK    0x0ff00000 /* Option mask for resets   */

/* GIF Tag constants.                                                  */
#define GIF_MODE_PACKED     0x00   /* Packed data format               */
#define GIF_MODE_REGLIST    0x01   /* Reglist data format              */
#define GIF_MODE_IMAGE      0x02   /* Image data format                */
#define GIF_MODE_RESERVED   0x03   /* Reserved data format             */
#define GIF_PRIM_DISABLE    0x00   /* Prim disabled                    */
#define GIF_PRIM_ENABLE     0x01   /* Prim enabled                     */
#define GIF_IMAGE_IMT_BURST 0x08   /* 8 quadword burst (GIF_STAT.IMT)  */

/* PATH names.                                                         */
#define GIF_IDLE            0x00   /* No active paths                  */
#define GIF_PATH1           0x01   /* PATH 1 - from VPU                */
#define GIF_PATH2           0x02   /* PATH 2 - from VIF                */
#define GIF_PATH3           0x03   /* PATH 3 - from R5900              */

/* GS registers                                                        */
#define GS_PRIM             0x00
#define GS_RGBAQ            0x01
#define GS_ST               0x02
#define GS_UV               0x03
#define GS_XYZF2            0x04
#define GS_XYZ2             0x05
#define GS_TEX0_1           0x06
#define GS_TEX0_2           0x07
#define GS_CLAMP_1          0x08
#define GS_CLAMP_2          0x09
#define GS_XYZF             0x0a
#define GS_RESERVED         0x0b
#define GS_XYZF3            0x0c
#define GS_XYZ3             0x0d
#define GS_A_D              0x0e
#define GS_NOP              0x0f
#define GS_HWREG            0x54   /* Hard coded register for IMAGE    */
#define GS_REFRESH          0x7f   /* Refresh the image                */
#define GS_LO_REGS             8   /* First  bank of 8 GS registers    */
#define GS_HI_REGS            16   /* Second bank of 8 GS registers    */

/* GIF read and write memory sizes.                                    */
#define GIF_REGISTER_BYTES  sizeof( unsigned_4 ) /* 32 bit registers   */
#define GIF_QUEUE_BYTES     sizeof( quadword )   /* 128 bit GIF queues */
#define GIF_PATH_QUEUE_SIZE sizeof( GIF_values ) /* GIF input queues   */
#define GIF_REGLIST_CHUNKS     2                 /* 2 x 64bits         */  


/* General bit masks for GIF use.                                      */
#define GIF_BIT_MASK(num_bits) ~((unsigned_4)0xffffffff << num_bits)  
#define GIF_REG_INDEX(_x)       (( _x & 0x000007f0 ) >> 4 ) 

/* GIF reads and writes against simulator memory.                      */
#define GIF_MEM_READ( addr, data, size )                              \
    do {                                                              \
         sim_cpu* cpu = STATE_CPU (CURRENT_STATE, 0);                 \
         unsigned_##size value =                                      \
           sim_core_read_aligned_##size (cpu, CIA_GET(cpu),           \
                                         read_map,(SIM_ADDR)(addr));  \
         memcpy ((unsigned_##size*) (data), (void*) &value, size);    \
    } while (0)                                                      

#define GIF_MEM_WRITE( addr, data, size )                             \
    do {                                                              \
         sim_cpu* cpu = STATE_CPU (CURRENT_STATE, 0);                 \
         unsigned_##size value;                                       \
         memcpy ((void*) &value, (unsigned_##size*)(data), size);     \
         sim_core_write_aligned_##size (cpu, CIA_GET(cpu), write_map, \
                                        (SIM_ADDR)(addr), value);     \
    } while (0)

#define GIF_RESET_QUEUE( _path )                                      \
         _path.queue_row     = 0;                                     \
         _path.tag_processed = 0;                                     \
         _path.pc            = 0;                                     \
         _path.partial_index = 0;                                     \
         _path.advance       = 0;                                     \
         _path.data_flags    = 0;                                     \
         _path.candidate     = 0;                                     \
         _path.direct_hl     = 0;                                   
   
                                                                      
/* External function declarations and associated defines.            */
void gif_attach ( SIM_DESC sd );
void gif_reset ();
void gif_disassemble_vif_data ( char *output_buffer, quadword *data );
void gif_options ( struct GIF_devicefull *, unsigned_4, 
                   char *, unsigned_4 *, long long * );     


/* GIF trace file macro.                                             */
#define GIF_TRACE_INPUT(z)                                               \
        if (( path->trace_on ) &&                                        \
            ( gif->trace_file != NULL ))                                 \
          fprintf (gif->trace_file,                                      \
                   ".word  0x%08lx, 0x%08lx, 0x%08lx, 0x%08lx \n",       \
                   path->queue[z].data[0],path->queue[z].data[1],        \
                   path->queue[z].data[2],path->queue[z].data[3] );      
         

/* GIF debug file macro.                                             */
#define GIF_DEBUG(string,var)                                            \
        {                                                                \
          if (( gif->debug_file == NULL ) &&                             \
              ( gif->debug_file_name != NULL ))                          \
            sky_open_file (&gif->debug_file,gif->debug_file_name,        \
                           (char *) NULL, _IOLBF );                      \
          fprintf ((gif->debug_file != NULL) ? gif->debug_file : stdout, \
                   string,var);                                          \
        }
          
#endif
