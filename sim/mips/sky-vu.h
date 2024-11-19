


/* Copyright (C) 1998, Cygnus Solutions */

#ifndef SKY_VU_H
#define SKY_VU_H

#include <sys/types.h>
#include "sim-main.h"
#include "sky-vpe.h"
#include "sky-device.h"

/* External functions */

void vu0_issue (SIM_DESC sd);
void vu0_reset ();
void vu0_attach (SIM_DESC sd);

void vu1_attach (SIM_DESC sd);
void vu1_issue (SIM_DESC sd);
void vu1_reset ();


#ifndef TARGET_SKY_B

/* VU0 coprocessor interface: for use in mips/interp.c only */

int  vu0_busy (void);
int  vu0_q_busy (void);
void vu0_macro_issue (unsigned_4 upper, unsigned_4 lower);
int  vu0_micro_interlock_released (void);
void vu0_micro_interlock_clear (void);

int  vu0_read_cop2_register(int reg_num);
void vu0_write_cop2_register(int reg_num, int value);

#endif

/* invariant relationships at any instant in time:

 * NOT (vu0_busy) => vu0_micro_interlock_released 
 * vu0_q_busy => vu0_busy
 */

/* Address space definition */

#define VU0_MEM0_WINDOW_START   0x11000000
#define VU0_MEM0_SIZE           0x1000  /* 4K = 4096 */
#define VU0_MEM1_WINDOW_START   0x11004000
#define VU0_MEM1_SIZE           0x1000  /* 4K = 4096 */
#define VU0_REGISTER_WINDOW_START 0x10000c00
#define VU1_MEM0_WINDOW_START   0x11008000
#define VU1_MEM0_SIZE           0x4000  /* 16K = 16384 */
#define VU1_MEM1_WINDOW_START   0x1100c000
#define VU1_MEM1_SIZE           0x4000  /* 16K = 16384 */
#define VU1_REGISTER_WINDOW_START 0x11007000

enum  /* VU source tracking tables */
{
  VU0_MEM0_SRCADDR_START =     0x19800000,
  VU0_MEM1_SRCADDR_START =     0x19804000,
  VU1_MEM0_SRCADDR_START =     0x19808000,
  VU1_MEM1_SRCADDR_START =     0x1980C000
};
  
/* Address in vu0 space of vu1 regs, in quadwords.  */
#define VU0_VU1_REG_START     0x400
#define VU0_VU1_FPREG_START   0x400
#define VU0_VU1_FPREG_SIZE    0x20
#define VU0_VU1_INTREG_START  0x420
#define VU0_VU1_INTREG_SIZE   0x10
#define VU0_VU1_CTRLREG_START 0x430
#define VU0_VU1_CTRLREG_SIZE  0x10

/* types & structs */

typedef u_long MEM_entry_t[4];
typedef u_long uMEM_entry_t[2];

typedef enum
  {
    VU_READY,
    VU_RUN,
    VU_BREAK
  }
vu_run_state;

/* Bits for VPU-FBRST */
/* See VU Specifications (Ver. 2.10), p7-5 */
#define VPU_FBRST_TE1_MASK 0x00000800
#define VPU_FBRST_DE1_MASK 0x00000400
#define VPU_FBRST_RS1_MASK 0x00000200
#define VPU_FBRST_FB1_MASK 0x00000100
#define VPU_FBRST_TE0_MASK 0x00000008
#define VPU_FBRST_DE0_MASK 0x00000004
#define VPU_FBRST_RS0_MASK 0x00000002
#define VPU_FBRST_FB0_MASK 0x00000001

/* Bits for VPU-STAT */
/* See VU Specifications (Ver. 2.10), p7-6 & 7-11 */
#define VPU_STAT_VBS0_BIT 0     /* VU0 busy               0: Idle, 1: Busy */
#define VPU_STAT_VDS0_BIT 1     /* VU0 D bit stop 0: No stop, 1: Stop */
#define VPU_STAT_VTS0_BIT 2     /* VU0 T bit stop 0: No stop, 1: Stop */
#define VPU_STAT_VFS0_BIT 3     /* VU0 ForceBreak stop    0: No stop, 1: Stop */
#define VPU_STAT_DIV0_BIT 5     /* VU0 DIV busy           0: Idle, 1: Busy */
#define VPU_STAT_PBS0_BIT 7     /* VIF0 busy              0: Idle, 1: Busy */

#define VPU_STAT_VBS1_BIT 8     /* VU0 busy               0: Idle, 1: Busy */
#define VPU_STAT_VDS1_BIT 9     /* VU0 D bit stop 0: No stop, 1: Stop */
#define VPU_STAT_VTS1_BIT 10    /* VU0 T bit stop 0: No stop, 1: Stop */
#define VPU_STAT_VFS1_BIT 11    /* VU0 ForceBreak stop    0: No stop, 1: Stop */
#define VPU_STAT_VGS1_BIT 12
#define VPU_STAT_DIV1_BIT 13    /* VU0 DIV busy           0: Idle, 1: Busy */
#define VPU_STAT_EFU1_BIT 14

typedef struct
  {
    floatie VF[32][4];
    short VI[16];
    u_long MST;
    u_long MMC;
    u_long MCP;
    u_long MTPC;
    u_long CMSAR0;
    u_long FBRST;               /* VU0 only; ctl reg */
    u_long CMSAR1;
    u_long VPU_STAT;            /* VU0 only; status reg */

    floatie acc[4];               /* accumulator registers */
    int acc_oflw[4];            /* accumulator overflow flags */
    floatie Q;                    /* FDIV register */
    floatie I;                    /* I register */
    u_long R;                   /* RANDU register */
    floatie VN[32];               /* EFU registers *//* P register == VN[4] */
    u_long MACflag;
    u_long statusflag;
    u_long clipflag;
  }
vu_regs;


/* Register offsets within memory-mapped window */
enum
  {
    COP2_REG_STATUSFLAG = 16,           /* Start of misc registers */
    COP2_REG_MACFLAG = 17,
    COP2_REG_CLIPFLAG = 18,
    COP2_REG_R       =  20,
    COP2_REG_I       =  21,
    COP2_REG_Q       =  22,
    COP2_REG_TPC     = 26,
                                /* Start of special registers */
    COP2_REG_CMSAR0  = 27,      /* VU0-only */
    COP2_REG_FBRST   = 28,
    COP2_REG_VPUSTAT = 29,
    COP2_REG_CIA     = 30,
    COP2_REG_CMSAR1  = 31       /* VU0-only */
  };

/* Register offsets within memory-mapped window */
enum
  {
    VU_REG_VF = 0,              /* Start of VF registers */
    VU_REG_VI = 0x200,          /* Start of VI registers */
    VU_REG_MST = 0x300,         /* Start of misc registers */
    VU_REG_MMC = 0x310,
    VU_REG_MCP = 0x320,
    VU_REG_MR = 0x330,
    VU_REG_MI = 0x340,
    VU_REG_MQ = 0x350,
    VU_REG_MP = 0x360,
    VU_REG_MTPC = 0x3a0,
                                /* Start of special registers */
    VU_REG_CMSAR0 = 0x3b0,      /* VU0-only */
    VU_REG_FBRST = 0x3c0,
    VU_REG_STAT = 0x3d0,
    VU_REG_CIA = 0x3e0,
    VU_REG_CMSAR1 = 0x3f0,      /* VU0-only */
    VU_REG_END = 0x3f0
  };


#define VPU_STAT_ADDR 0x110073d0

#define VPU_STAT            0x110073d0

typedef struct
  {
    pipeline     pipe[4][2];    /* Upper/Lower FMAC, Ld/St, RANDU piepline */
    q_pipeline   qpipe;         /* FDIV(DIV,SQRT,RSQRT) pipelinei state */
    i_pipeline   ipipe;         /* IALU pipeline */
    a_pipeline   apipe;         /* ACC pipeline stage */
    su_pipeline  spipe;         /* EFU pipeline stage */
    loi_pipeline Ipipe;         /* I-bit pipeline stage */
  }
vu_pipeline;

typedef struct
  {
    int _is_dbg;
    int _is_verb;
    int _is_dump;
    int _pgpuif;
    FILE *_fp_gpu;

    int _GIF_SIM_OFF;
    int _GIF_BUSY;
    int _GIF_VUCALL;
    int _GIF_VUADDR;

    u_long instbuf[2];          /* instruction buffer. instbuf[0]:Upper, instbuf[1]:Lower */
    u_long opc, pc;             /* opc is addr of current insn, pc is current PC. see fetch_inst() */
    u_long jaddr;               /* branch address */


    int eflag, jflag, peflag, sflag, lflag, mflag, dflag, tflag;
    /*      eflag: E-bit detect flag
       0: not detect, 1: detect, -1: fetch stage terminate
       jflag: branch detect flag
       0: not detect, 1: detect
       peflag: end delay slot execute flag
       0: not execute, 1: execute, -1: finished ececution
       sflag: data hazard stall flag
       0: not stall, 1: stall 
       lflag: M-bit interlock detect flag 
       mflag: COP2 macro-instruction anti-fetch flag
       dflag: D-bit detect flag (debug)
       tflag: T-bit detect flag (trace) */
    u_long bp;                  /* break point address */
    u_long ecount;              /* end counter */
    int intr_mode;              /* interactive mode enable */
    int verb_mode;              /* verbose mode enable */
    int dpr_mode;               /* PRT (debug print) instruction enable */
    u_long all_count;           /* amount of executed cycles */
    u_long hc_count;            /* amount of hazard stall cycles */
  }
vu_junk;

typedef struct
  {
    int vu_number;
    u_long register_window_start;
    u_long mem0_window_start;
    u_long mem0_size;
    u_long mem0_tracking_start;
    u_long mem1_window_start;
    u_long mem1_size;
    u_long mem1_tracking_start;
  }
vu_config;

typedef struct vu_device
  {
    /* Warning: this must be the first member.
       This struct is treated as a derived class of struct device.  */
    device dev;                 /* device header */

    vu_config config;

    char *umem_buffer_tofree;   /* unaligned buffer pointers */
    char *mem_buffer_tofree;

    MEM_entry_t *MEM_buffer;    /* VU (data) memory */
    unsigned MEM_size;          /* number of quadwords */
    uMEM_entry_t *uMEM_buffer;  /* Micro (instruction) memory */
    unsigned uMEM_size;         /* number of diwords */
    
    FILE *debug_file;           /* VUx debug file       */
    char *debug_file_name;      /* VUx debug file name  */
    
    vu_regs regs;
    vu_pipeline pipeline;
    vu_junk junk;
    vu_run_state run_state;

    FILE *console_out, *console_in;     /* I/O for interactive VU sessions.   To be deleted.  */
  }
vu_device;


/* register handling functions */

int read_vu_int_reg (vu_regs * regs, int regno, void *buf);
int read_vu_vec_reg (vu_regs * rp, int row, int xyzw, void *buf);
int read_vu_acc_reg (vu_regs * regs, int xyzw, void *buf);
/* both misc and special? */
int read_vu_misc_reg (vu_regs * regs, int raddr, void *buf);
int read_vu_special_reg (vu_device * me, int raddr, void *buf);

int write_vu_int_reg (vu_regs * rp, int regno, const void *buf);
int write_vu_vec_reg (vu_regs * rp, int r, int xyzw, const void *b);
int write_vu_acc_reg (vu_regs * regs, int xyzw, const void *buf);
/* both misc and special? */
int write_vu_misc_reg (vu_regs * rp, int ra, const void *buf);
int write_vu_special_reg (vu_device * me, int raddr, const void *buf);
void vu_options (vu_device * me, int option, char *option_string);

#define VU_CHECK_OPEN_DEBUG(vu)                              \
        if (( vu->debug_file == NULL ) &&                    \
            ( vu->debug_file_name != NULL ))                 \
          sky_open_file (&vu->debug_file,vu->debug_file_name,\
                         (char *) NULL, _IOLBF );

/* allow access to two static instances */

#ifndef TARGET_SKY_B
extern vu_device vu0_device;
#endif

extern vu_device vu1_device;


#endif /* SKY_VU_H */
