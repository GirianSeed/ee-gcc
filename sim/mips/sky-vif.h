/* Copyright (C) 1998, Cygnus Solutions */

#ifndef H_VIF_H
#define H_VIF_H

#include "sim-main.h"
#include "sky-device.h"


/* External functions */

struct vif_fifo;
struct fifo_quadword;
struct vif_device;

void vif0_attach(SIM_DESC sd);
void vif0_issue(SIM_DESC sd);
void vif1_attach(SIM_DESC sd);
void vif1_issue(SIM_DESC sd);
void vif0_reset();
void vif1_reset();

void vif_options(struct vif_device *device, unsigned_4 option, char *option_string);
int read_vif_reg (struct vif_device *device, int regno, void *buf);
int write_vif_reg (struct vif_device *device, int regno, const void *buf);
int read_vif_pc (struct vif_device *device, void *buf);
int read_vif_pcx (struct vif_device *device, void *buf);
struct fifo_quadword* vif_fifo_access(struct vif_fifo*, unsigned_4 qwnum);


/* Quadword data type */

typedef unsigned_4 quadword[4];

/* truncate address to quadword */
#define ADDR_TRUNC_QW(addr) ((addr) & ~0x0f)
/* extract offset in quadword */
#define ADDR_OFFSET_QW(addr) ((addr) & 0x0f)


/* SCEI memory mapping information */

enum 
{
  VIF0_REGISTER_WINDOW_START = 0x10003800, /* VIF registers */
  VIF1_REGISTER_WINDOW_START = 0x10003C00,
  VIF0_FIFO_ADDR =             0x10004000, /* VIF input FIFOs */
  VIF1_FIFO_ADDR =             0x10005000,
};


#define VU0_CIA (VU0_REGISTER_WINDOW_START + VU_REG_CIA)
#define VU1_CIA (VU1_REGISTER_WINDOW_START + VU_REG_CIA)


enum /* other devices' register bit masks */
{
/* GPUIF STAT register */
  GPUIF_REG_STAT_APATH_E = 11,
  GPUIF_REG_STAT_APATH_B = 10,
};

/* COP2 STAT register */
#define COP2_REG_STAT_ADDR VPU_STAT_ADDR
enum
{
  COP2_REG_STAT_VBS1_E = 8,
  COP2_REG_STAT_VBS1_B = 8,
  COP2_REG_STAT_VBS0_E = 0,
  COP2_REG_STAT_VBS0_B = 0,
};

/* Quadword indices of VIF registers.  Actual registers sit at bottom
   32 bits of each quadword. */
enum
{
  VIF_REG_STAT =    0x00,
  VIF_REG_FBRST =   0x01,
  VIF_REG_ERR =     0x02,
  VIF_REG_MARK =    0x03,
  VIF_REG_CYCLE =   0x04,
  VIF_REG_MODE =    0x05,
  VIF_REG_NUM =     0x06,
  VIF_REG_MASK =    0x07,
  VIF_REG_CODE =    0x08,
  VIF_REG_ITOPS =   0x09,
  VIF_REG_BASE =    0x0a /* vif1 only */,
  VIF_REG_OFST =    0x0b /* vif1 only */,
  VIF_REG_TOPS =    0x0c /* vif1 only */,
  VIF_REG_ITOP =    0x0d,
  VIF_REG_TOP =     0x0e /* vif1 only */,
  VIF_REG_DBF =     0x0f /* vif1 only */,
  VIF_REG_R0 =      0x10 /* R0 .. R3 must be contiguous */,
  VIF_REG_R1 =      0x11,
  VIF_REG_R2 =      0x12,
  VIF_REG_R3 =      0x13,
  VIF_REG_C0 =      0x14 /* C0 .. C3 must be contiguous */,
  VIF_REG_C1 =      0x15,
  VIF_REG_C2 =      0x16,
  VIF_REG_C3 =      0x17,
/* one plus last index */
  VIF_NUM_REGS =    0x18,
};

#define VIF_REGISTER_WINDOW_SIZE  (sizeof(quadword) * VIF_NUM_REGS)



/* VIF commands */
enum 
{
  VIF_CMD_VIFNOP = 0x00,
  VIF_CMD_STCYCL = 0x01,
  VIF_CMD_OFFSET = 0x02,
  VIF_CMD_BASE = 0x03,
  VIF_CMD_ITOP = 0x04,
  VIF_CMD_STMOD = 0x05,
  VIF_CMD_MSKPATH3 = 0x06,
  VIF_CMD_VIFMARK = 0x07,
  VIF_CMD_FLUSHE = 0x10,
  VIF_CMD_FLUSH = 0x11,
  VIF_CMD_FLUSHA = 0x13,
  VIF_CMD_VIFMSCAL = 0x14, /* CAL == "call" */
  VIF_CMD_VIFMSCNT = 0x17, /* CNT == "continue" */
  VIF_CMD_VIFMSCALF = 0x15, /* CALF == "call after flush" */
  VIF_CMD_STMASK = 0x20,
  VIF_CMD_STROW = 0x30,
  VIF_CMD_STCOL = 0x31,
  VIF_CMD_MPG = 0x4A,
  VIF_CMD_DIRECT = 0x50,
  VIF_CMD_DIRECTHL = 0x51,

  VIF_CMD_MASK        = 0x7F,

  VIF_CMD_UNPACK = 0x60,
  VIF_CMD_UNPACK_MASK = 0x60,
};



/* register bitmasks: bit numbers for end and beginning of fields */

/* VIF opcode */
enum
{
  VIF_OPCODE_I_E = 31,
  VIF_OPCODE_I_B = 31,
  VIF_OPCODE_CMD_E = 30,
  VIF_OPCODE_CMD_B = 24,
  VIF_OPCODE_NUM_E = 23,
  VIF_OPCODE_NUM_B = 16,
  VIF_OPCODE_IMM_E = 15,
  VIF_OPCODE_IMM_B = 0,

/* STAT register */
  VIF_REG_STAT_FQC_E = 28,
  VIF_REG_STAT_FQC_B = 24,
  VIF_REG_STAT_FDR_E = 23,
  VIF_REG_STAT_FDR_B = 23,
  VIF_REG_STAT_ER1_E = 13,
  VIF_REG_STAT_ER1_B = 13,
  VIF_REG_STAT_ER0_E = 12,
  VIF_REG_STAT_ER0_B = 12,
  VIF_REG_STAT_INT_E = 11,
  VIF_REG_STAT_INT_B = 11,
  VIF_REG_STAT_PIS_E = 10,
  VIF_REG_STAT_PIS_B = 10,
  VIF_REG_STAT_PFS_E = 9,
  VIF_REG_STAT_PFS_B = 9,
  VIF_REG_STAT_PSS_E = 8,
  VIF_REG_STAT_PSS_B = 8,
  VIF_REG_STAT_DBF_E = 7,
  VIF_REG_STAT_DBF_B = 7,
  VIF_REG_STAT_MRK_E = 6,
  VIF_REG_STAT_MRK_B = 6,
  VIF_REG_STAT_PGW_E = 3,
  VIF_REG_STAT_PGW_B = 3,
  VIF_REG_STAT_PEW_E = 2,
  VIF_REG_STAT_PEW_B = 2,
  VIF_REG_STAT_PPS_E = 1,
  VIF_REG_STAT_PPS_B = 0,

  VIF_REG_STAT_PPS_IDLE = 0x00 /* ready to execute next instruction */,
  VIF_REG_STAT_PPS_WAIT = 0x01 /* not enough words in FIFO */,
  VIF_REG_STAT_PPS_DECODE = 0x02 /* decoding instruction */,
  VIF_REG_STAT_PPS_STALL = 0x02 /* alias state for stall (e.g., FLUSHE) */,
  VIF_REG_STAT_PPS_XFER = 0x03 /* transferring instruction operands */,

/* DBF register */
  VIF_REG_DBF_DF_E = 0,
  VIF_REG_DBF_DF_B = 0,

/* OFST register */
  VIF_REG_OFST_OFFSET_E = 9,
  VIF_REG_OFST_OFFSET_B = 0,

/* OFST register */
  VIF_REG_TOPS_TOPS_E = 9,
  VIF_REG_TOPS_TOPS_B = 0,

/* BASE register */
  VIF_REG_BASE_BASE_E = 9,
  VIF_REG_BASE_BASE_B = 0,

/* ITOPS register */
  VIF_REG_ITOPS_ITOPS_E = 9,
  VIF_REG_ITOPS_ITOPS_B = 0,

/* MODE register */
  VIF_REG_MODE_MDE_E = 1,
  VIF_REG_MODE_MDE_B = 0,

/* NUM register */
  VIF_REG_NUM_NUM_E = 9,
  VIF_REG_NUM_NUM_B = 0,

/* MARK register */
  VIF_REG_MARK_MARK_E = 15,
  VIF_REG_MARK_MARK_B = 0,

/* ITOP register */
  VIF_REG_ITOP_ITOP_E = 9,
  VIF_REG_ITOP_ITOP_B = 0,

/* TOP register */
  VIF_REG_TOP_TOP_E = 9,
  VIF_REG_TOP_TOP_B = 0,

/* MASK register */
  VIF_REG_MASK_MASK_E = 31,
  VIF_REG_MASK_MASK_B = 0,

/* CYCLE register */
  VIF_REG_CYCLE_WL_E = 15,
  VIF_REG_CYCLE_WL_B = 8,
  VIF_REG_CYCLE_CL_E = 7,
  VIF_REG_CYCLE_CL_B = 0,

/* ERR register */
  VIF_REG_ERR_ME1_E = 2,
  VIF_REG_ERR_ME1_B = 2,
  VIF_REG_ERR_ME0_E = 1,
  VIF_REG_ERR_ME0_B = 1,
  VIF_REG_ERR_MII_E = 0,
  VIF_REG_ERR_MII_B = 0,

/* FBRST command bitfields */
  VIF_REG_FBRST_STC_E = 3,
  VIF_REG_FBRST_STC_B = 3,
  VIF_REG_FBRST_STP_E = 2,
  VIF_REG_FBRST_STP_B = 2,
  VIF_REG_FBRST_FBK_E = 1,
  VIF_REG_FBRST_FBK_B = 1,
  VIF_REG_FBRST_RST_E = 0,
  VIF_REG_FBRST_RST_B = 0,

/* MSKPATH3 command bitfields */
  VIF_REG_MSKPATH3_E = 15,
  VIF_REG_MSKPATH3_B = 15,
};


/* UNPACK opcodes */
#define VIF_UNPACK(vn,vl) ((vn) << 2 | (vl))
#define VIF_UNPACK_S_32  VIF_UNPACK(0, 0)
#define VIF_UNPACK_S_16  VIF_UNPACK(0, 1)
#define VIF_UNPACK_S_8   VIF_UNPACK(0, 2)
#define VIF_UNPACK_V2_32 VIF_UNPACK(1, 0)
#define VIF_UNPACK_V2_16 VIF_UNPACK(1, 1)
#define VIF_UNPACK_V2_8  VIF_UNPACK(1, 2)
#define VIF_UNPACK_V3_32 VIF_UNPACK(2, 0)
#define VIF_UNPACK_V3_16 VIF_UNPACK(2, 1)
#define VIF_UNPACK_V3_8  VIF_UNPACK(2, 2)
#define VIF_UNPACK_V4_32 VIF_UNPACK(3, 0)
#define VIF_UNPACK_V4_16 VIF_UNPACK(3, 1)
#define VIF_UNPACK_V4_8  VIF_UNPACK(3, 2)
#define VIF_UNPACK_V4_5  VIF_UNPACK(3, 3)


/* MASK register sub-field definitions */
enum
{
  VIF_MASKREG_INPUT = 0,
  VIF_MASKREG_ROW = 1,
  VIF_MASKREG_COLUMN = 2,
  VIF_MASKREG_NOTHING = 3,
};

/* STMOD register field definitions */
enum
{
  VIF_MODE_INPUT = 0,
  VIF_MODE_ADDROW = 1,
  VIF_MODE_ACCROW = 2,
};


/* extract a MASK register sub-field for row [0..3] and column [0..3] */
/* MASK register is laid out of 2-bit values in this r-c order */
/* m33 m32 m31 m30 m23 m22 m21 m20 m13 m12 m11 m10 m03 m02 m01 m00 */
#define VIF_MASKREG_GET(me,row,col) \
((((me)->regs[VIF_REG_MASK][0]) >> (8*(row) + 2*(col))) & 0x03)


/* operations - replace with those in sim-bits.h when convenient */

/* unsigned 32-bit mask of given width */
#define BIT_MASK(width) ((width) == 31 ? 0xffffffff : (((unsigned_4)1) << (width+1)) - 1)
/* e.g.: BIT_MASK(4) = 00011111 */

/* mask between given given bits numbers (MSB) */
#define BIT_MASK_BTW(begin,end) ((BIT_MASK(end) & ~((begin) == 0 ? 0 : BIT_MASK((begin)-1))))
/* e.g.: BIT_MASK_BTW(4,11) = 0000111111110000 */

/* set bitfield value */
#define BIT_MASK_SET(lvalue,begin,end,value) \
do { \
  ASSERT((begin) <= (end)); \
  (lvalue) &= ~BIT_MASK_BTW((begin),(end)); \
  (lvalue) |= ((value) << (begin)) & BIT_MASK_BTW((begin),(end)); \
} while(0)

/* get bitfield value */
#define BIT_MASK_GET(rvalue,begin,end) \
  (((rvalue) & BIT_MASK_BTW(begin,end)) >> (begin))
/* e.g., BIT_MASK_GET(0000111100001111, 2, 8) = 0000000100001100 */

/* These ugly macro hacks allow succinct bitfield accesses */
/* set a bitfield in a register by "name" */
#define VIF_REG_MASK_SET(me,reg,flag,value) \
     do { \
       unsigned_4 old = BIT_MASK_GET(((me)->regs[VIF_REG_##reg][0]), \
                    VIF_REG_##reg##_##flag##_B, VIF_REG_##reg##_##flag##_E); \
       BIT_MASK_SET(((me)->regs[VIF_REG_##reg][0]), \
                    VIF_REG_##reg##_##flag##_B, VIF_REG_##reg##_##flag##_E, \
                    (value)); \
       if( indebug ((me)->dev.name)) \
         { \
           if (old != (value)) \
           { \
             if (((me)->fifo_trace_file == NULL ) &&  \
                 ((me)->fifo_trace_file_name != NULL )) \
               sky_open_file (&((me)->fifo_trace_file), \
                              (me)->fifo_trace_file_name, \
                              (char *) NULL, _IOLBF ); \
             fprintf (((me)->fifo_trace_file != NULL) ? \
                      (me)->fifo_trace_file : stdout, \
                      "# Reg %s:%s = 0x%x\n", #reg, #flag, (unsigned)(value)); \
           } \
         } \
     } while(0)

/* get a bitfield from a register by "name" */
#define VIF_REG_MASK_GET(me,reg,flag) \
     BIT_MASK_GET(((me)->regs[VIF_REG_##reg][0]), \
                  VIF_REG_##reg##_##flag##_B, VIF_REG_##reg##_##flag##_E)


#define VIF_LIMIT(value,max) ((value) > (max) ? (max) : (value))


/* Classify words in a FIFO quadword */
enum wordclass
{
  wc_dma = 'D',
  wc_vifcode = 'P',
  wc_unknown = '?',
  wc_vifdata = '.',
  wc_gpuiftag = 'g'
};


/* One row in the FIFO */
struct fifo_quadword
{
  /* 128 bits of data */
  quadword data;
  /* source main memory address (or 0: unknown) */
  unsigned_4 source_address;
  /* classification of words in quadword; wc_dma set on DMA tags at FIFO write */
  enum wordclass word_class[4];
};


/* quadword FIFO structure for VIF */ 
typedef struct vif_fifo
{
  struct fifo_quadword* quadwords; /* pointer to fifo quadwords */
  unsigned_4 origin; /* quadword serial number of quadwords[0] */
  unsigned_4 length; /* length of quadword pointer array: 0..N */
  unsigned_4 next;   /* relative index of first unfilled quadword: 0..length-1 */
} vif_fifo;

#define VIF_FIFO_GROW_SIZE 1024 /* number of quadword pointers to allocate */
#define VIF_FIFO_ARCHEOLOGY 1024 /* number of old quadwords to keep as history */



/* VIF internal state: FIFOs, registers, handle to VU friend */
struct vif_device
{
  /* common device info */
  device dev;

  /* identity: 0=VIF0, 1=VIF1 */
  int vif_number;
  int flags;

  /* quadword registers: data in [0] word only */
  quadword regs[VIF_NUM_REGS];

  /* write buffer for FIFO address */
  quadword fifo_qw_in_progress;
  int fifo_qw_done; /* bitfield */

  /* FIFO - private: use only vif_fifo_* routines to access */
  struct vif_fifo fifo;  /* array of FIFO quadword pointers */
  FILE* fifo_trace_file; /* stdio stream open in append mode, or 0 for no trace */
  char* fifo_trace_file_name; /* user defined debug trace file name  */

  /* FIFO cache -- curry last search vif_pcrel_fifo results */
  unsigned_4 last_fifo_pc;
  unsigned_4 last_qw_pc;
  unsigned_4 last_num;
  unsigned_4 last_new_fifo_pc;
  unsigned_4 last_new_qw_pc;

  /* PC */
  int fifo_pc;  /* 0 .. (fifo_num_elements-1): quadword index of next instruction */
  int qw_pc;    /* 0 .. 3:                     word index of next instruction */

  /* Disassembly state */
  FILE *trace_file;
  char *trace_file_name;
};


#ifndef TARGET_SKY_B
extern struct vif_device vif0_device;
#endif
extern struct vif_device vif1_device;



/* Flags for VIF.flags */

#define VIF_FLAG_NONE        0x00
#define VIF_FLAG_PENDING_PSS 0x01 /* PSS bit written-to; set STAT:PSS after current instruction */
#define VIF_FLAG_INT_NOLOOP  0x02 /* INT VIFcode received; INT/PIS set; suppress loop after resumption */
#define VIF_FLAG_TRACE_ON    0x04 /* Trace file request from command line  */                                      

/* Kludge alert */

#define VIF_MEM_READ(me,addr,data,size) \
    do { \
      sim_cpu* cpu = STATE_CPU(CURRENT_STATE, 0); \
      unsigned_##size value = \
        sim_core_read_aligned_##size(cpu, CIA_GET(cpu), read_map, \
                                     (SIM_ADDR)(addr)); \
      memcpy((unsigned_##size*) (data), (void*) & value, size); \
     } while(0)

#define VIF_MEM_WRITE(me,addr,data,size) \
    do { sim_cpu* cpu = STATE_CPU(CURRENT_STATE, 0); \
         unsigned_##size value; \
         memcpy((void*) & value, (unsigned_##size*)(data), size); \
         sim_core_write_aligned_##size(cpu, CIA_GET(cpu), write_map, \
                                       (SIM_ADDR)(addr), value); \
         if (indebug ((me)->dev.name)) \
           { \
             int i; \
             unsigned_##size value_te; \
             value_te = H2T_##size(value); \
             if (((me)->fifo_trace_file == NULL ) && \
                 ((me)->fifo_trace_file_name != NULL )) \
               sky_open_file (&((me)->fifo_trace_file), \
                              (me)->fifo_trace_file_name, \
                              (char *) NULL, _IOLBF ); \
             fprintf (((me)->fifo_trace_file != NULL) ? \
                      (me)->fifo_trace_file : stdout, \
                      "# Write %2d bytes  to  ", size); \
             fprintf (((me)->fifo_trace_file != NULL) ? \
                      (me)->fifo_trace_file : stdout, \
                     "0x%08lx: ", (unsigned long)(addr)); \
             for(i=0; i<size; i++) \
               fprintf (((me)->fifo_trace_file != NULL) ? \
                        (me)->fifo_trace_file : stdout, \
                        " %02x", ((unsigned_1*)(& value_te))[i]); \
             fprintf (((me)->fifo_trace_file != NULL) ? \
                      (me)->fifo_trace_file : stdout, \
                      "\n"); \
           } \
        } while(0)      


#endif /* H_VIF_H */
