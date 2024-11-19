/****************************************************************************/
/*                                                                          */
/*             Sony Computer Entertainment CONFIDENTIAL                     */
/*      (C) 1997 Sony Computer Entertainment Inc. All Rights Reserved       */
/*                                                                          */
/*       VU simulator pipeline definitions                                  */
/*                                                                          */
/****************************************************************************/
#ifndef SKY_VPE_H_
#define SKY_VPE_H_

#include <stdio.h>
#include <sys/types.h>
#include <strings.h>
#include <math.h>

#include "sim-main.h"


typedef union _floatie
{
  float f;
  unsigned i;
} floatie;

/* should assert sizeof(float) == sizeof(unsigned) == sizeof(floatie) */


typedef enum
  {
    I_MOVE, I_MR32, I_LQI, I_SQI, I_LQD, I_SQD, I_MTIR, I_MFIR,
    I_ILWR, I_ISWR, I_RNEXT, I_RGET, I_MFP, I_XTOP, I_XITOP, I_ADDA,
    I_SUBA, I_MADDA, I_MSUBA, I_ITOF0, I_ITOF4, I_ITOF12, I_ITOF15, I_FTOI0,
    I_FTOI4, I_FTOI12, I_FTOI15, I_MULA, I_MULAq, I_ABS, I_MULAi, I_CLIP,
    I_ADDAq, I_MADDAq, I_ADDAi, I_MADDAi, I_SUBAq, I_MSUBAq, I_SUBAi, I_MSUBAi,
    I_OPMULA, I_ADD, I_SUB, I_MADD, I_MSUB, I_MAX, I_MINI, I_MUL,
    I_MULq, I_MAXi, I_MULi, I_MINIi, I_ADDq, I_MADDq, I_ADDi, I_MADDi,
    I_SUBq, I_MSUBq, I_SUBi, I_MSUBi, I_OPMSUB, I_LQ, I_SQ, I_ILW,
    I_ISW, I_IADDIU, I_ISUBIU, I_FCEQ, I_FCSET, I_FCAND, I_FCOR, I_FSEQ,
    I_FSSET, I_FSAND, I_FSOR, I_FMEQ, I_FMAND, I_FMOR, I_FCGET, I_BAL,
    I_JAL, I_IADD, I_ISUB, I_IADDI, I_IAND, I_IOR,

    I_DIV, I_SQRT, I_RSQRT,

    I_ESADD, I_ERSADD, I_ELENG, I_ERLENG, I_EATANxy,
    I_EATANxz, I_ESUM, I_ESQRT, I_ERSQRT, I_ERCPR,
    I_ESIN, I_EATAN, I_EEXP
  }
vu_code_t;

typedef enum {
    P_EMPTY = 0, 
    P_VF_REG = 1, 
    P_MEMORY = 2,
    P_I_REG = 3, 	/* Not Used */
    P_STATUS_REG = 4,
    P_CLIP_REG = 5,
    P_EFU = 6,		/* Not Used */
    P_MFP = 7,
    P_VF_REG_NO_STATUS = 8
} pipe_flag_t;

typedef struct
  {
    int no;			/* destination register number VF:0-31 VI:32-47 */
    int mask;			/* specify which units calculate */
    floatie vf[4];		/* calculated value */
    int flag;			/*0: empty in this pipeline stage,
				   1: write value to reg,
				   2: store value from reg,
				   3: write value to I-reg ( not used ),
				   4: write only status to statusflag,
				   5: write only clip value to clipflag,
				   6: move value from EFU reg */
    u_long status;		/* calculation unit status */
    u_long addr;		/* store address ( store operation ) */
    vu_code_t code;		/* instruction */
    u_long code_addr;
  }
pipeline;			/* pipeline stage specification for FMAC, Ld/St, RANDU, FDIV */

typedef struct
  {
    int old_reg;                /* previous value of a reg is needed for */
    short old_value;            /* some add/sub-ibxxx sequences */
    int no;			/* destination register number VI:0-15 */
    short vi;			/* calculated value */
    int flag;			/* 0: empty in this pipeline stage,
				   1: write value to reg */
    int code;			/* instruction */
    u_long code_addr;
  }
i_pipeline;			/* pipeline stage specification for IALU */

typedef struct
  {
    int flag;			/* 0: empty in this pipeline stage,
				   1: write value to I-reg */
    floatie val;		/* calculated value */
  }
loi_pipeline;			/* pipeline stage specification for I bit */

typedef struct
  {
    int mask;			/* specify which units calculate */
    floatie acc[4];		/* calculated value */
    int acc_oflw[4];            /* accumulator overflow flags */
    int flag;			/* 0: empty in this pipeline stage,
				   1: write value to ACC */
  }
a_pipeline;			/* accumulator pipeline stage specification */

typedef struct
  {
    int no;			/* cycle delay before write to P register */
    floatie vn;			/* calculated value */
    int code;			/* instruction */
    u_long code_addr;
  }
su_pipeline;

typedef struct
  {
    int no;			/* cycle delay before write to Q register */
    floatie vf;			/* calculated value */
    u_long status;
    int code;			/* instruction */
    u_long code_addr;
  }
q_pipeline;

#define DEST_X	0x8
#define DEST_Y	0x4
#define DEST_Z	0x2
#define DEST_W	0x1

#endif
