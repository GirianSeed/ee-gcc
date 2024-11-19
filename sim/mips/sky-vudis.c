/****************************************************************************/
/*                                                                          */
/*             Sony Computer Entertainment CONFIDENTIAL                     */
/*      (C) 1997 Sony Computer Entertainment Inc. All Rights Reserved       */
/*                                                                          */
/*      Experimental disassembler                                           */
/*                                                                          */
/****************************************************************************/

#include <sys/types.h>
#include <stdio.h>

static void
p_bc (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fputc('x', fp);
      break;
    case 1:
      fputc('y', fp);
      break;
    case 2:
      fputc('z', fp);
      break;
    case 3:
      fputc('w', fp);
      break;
    }
}

static void
p_dest (FILE *fp, u_long opcode)
{
  switch ((opcode >> 21) & 0xf)
    {
    case 1:
      fprintf (fp, "w");
      break;
    case 2:
      fprintf (fp, "z");
      break;
    case 3:
      fprintf (fp, "wz");
      break;
    case 4:
      fprintf (fp, "y");
      break;
    case 5:
      fprintf (fp, "yw");
      break;
    case 6:
      fprintf (fp, "yz");
      break;
    case 7:
      fprintf (fp, "yzw");
      break;
    case 8:
      fprintf (fp, "x");
      break;
    case 9:
      fprintf (fp, "xw");
      break;
    case 10:
      fprintf (fp, "xz");
      break;
    case 11:
      fprintf (fp, "xzw");
      break;
    case 12:
      fprintf (fp, "xy");
      break;
    case 13:
      fprintf (fp, "xyw");
      break;
    case 14:
      fprintf (fp, "xyz");
      break;
    case 15:
      fprintf (fp, "xyzw");
      break;
    }
}

static void
bc_reg (FILE *fp, u_long opcode)
{
  fputc('/', fp);
  p_dest (fp, opcode);

  fprintf (fp, " VF%02ld", (opcode >> 6) & 0x1f);
  p_dest (fp, opcode);
  fprintf (fp, ", VF%02ld", (opcode >> 11) & 0x1f);
  p_dest (fp, opcode);
  fprintf (fp, ", VF%02ld", (opcode >> 16) & 0x1f);
  p_bc (fp, opcode);
}

static void
par_reg3 (FILE *fp, u_long opcode)
{
  fprintf (fp, " VF%02ld", (opcode >> 6) & 0x1f);
  p_dest (fp, opcode);
  fprintf (fp, ", VF%02ld", (opcode >> 11) & 0x1f);
  p_dest (fp, opcode);
  fprintf (fp, ", VF%02ld", (opcode >> 16) & 0x1f);
  p_dest (fp, opcode);
}

static void
par_reg_ds (FILE *fp, u_long opcode)
{
  fprintf (fp, " VF%02ld", (opcode >> 6) & 0x1f);
  p_dest (fp, opcode);
  fprintf (fp, ", VF%02ld", (opcode >> 11) & 0x1f);
  p_dest (fp, opcode);
}

static void
mul_q (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "MULq/");
      p_dest (fp, opcode);
      par_reg_ds (fp, opcode);
      fprintf (fp, ", Q");
      break;
    case 1:
      fprintf (fp, "MAXi/");
      p_dest (fp, opcode);
      par_reg_ds (fp, opcode);
      fprintf (fp, ", I");
      break;
    case 2:
      fprintf (fp, "MULi/");
      p_dest (fp, opcode);
      par_reg_ds (fp, opcode);
      fprintf (fp, ", I");
      break;
    case 3:
      fprintf (fp, "MINIi/");
      p_dest (fp, opcode);
      par_reg_ds (fp, opcode);
      fprintf (fp, ", I");
      break;
    }
}

static void
add_q (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "ADDq/");
      p_dest (fp, opcode);
      par_reg_ds (fp, opcode);
      fprintf (fp, ", Q");
      break;
    case 1:
      fprintf (fp, "MADDq/");
      p_dest (fp, opcode);
      par_reg_ds (fp, opcode);
      fprintf (fp, ", Q");
      break;
    case 2:
      fprintf (fp, "ADDi/");
      p_dest (fp, opcode);
      par_reg_ds (fp, opcode);
      fprintf (fp, ", I");
      break;
    case 3:
      fprintf (fp, "MADDi/");
      p_dest (fp, opcode);
      par_reg_ds (fp, opcode);
      fprintf (fp, ", I");
      break;
    }
}

static void
sub_q (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "SUBq/");
      p_dest (fp, opcode);
      par_reg_ds (fp, opcode);
      fprintf (fp, ", Q");
      break;
    case 1:
      fprintf (fp, "MSUBq/");
      p_dest (fp, opcode);
      par_reg_ds (fp, opcode);
      fprintf (fp, ", Q");
      break;
    case 2:
      fprintf (fp, "SUBi/");
      p_dest (fp, opcode);
      par_reg_ds (fp, opcode);
      fprintf (fp, ", I");
      break;
    case 3:
      fprintf (fp, "MSUBi/");
      p_dest (fp, opcode);
      par_reg_ds (fp, opcode);
      fprintf (fp, ", I");
      break;
    }
}

static void
sky_add (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "ADD/");
      p_dest (fp, opcode);
      par_reg3 (fp, opcode);
      break;
    case 1:
      fprintf (fp, "MADD/");
      p_dest (fp, opcode);
      par_reg3 (fp, opcode);
      break;
    case 2:
      fprintf (fp, "MUL/");
      p_dest (fp, opcode);
      par_reg3 (fp, opcode);
      break;
    case 3:
      fprintf (fp, "MAX/");
      p_dest (fp, opcode);
      par_reg3 (fp, opcode);
      break;
    }
}

static void
sub (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "SUB/");
      p_dest (fp, opcode);
      par_reg3 (fp, opcode);
      break;
    case 1:
      fprintf (fp, "MSUB/");
      p_dest (fp, opcode);
      par_reg3 (fp, opcode);
      break;
    case 2:
      fprintf (fp, "OPMSUB.xyz VF%02ld", (opcode >> 6) & 0x1f);
      fprintf (fp, ", VF%02ld, VF%02ld", (opcode >> 11) & 0x1f,
	      (opcode >> 16) & 0x1f);
      break;
    case 3:
      fprintf (fp, "MINI/");
      p_dest (fp, opcode);
      par_reg3 (fp, opcode);
      break;
    }
}

static void
bc_acc (FILE *fp, u_long opcode)
{
  fputc('/', fp);
  p_dest (fp, opcode);

  fprintf (fp, " ACC");
  p_dest (fp, opcode);
  fprintf (fp, ", VF%02ld", (opcode >> 11) & 0x1f);
  p_dest (fp, opcode);
  fprintf (fp, ", VF%02ld", (opcode >> 16) & 0x1f);
  p_bc (fp, opcode);
}

static void
itof (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "0/");
      break;
    case 1:
      fprintf (fp, "4/");
      break;
    case 2:
      fprintf (fp, "12/");
      break;
    case 3:
      fprintf (fp, "15/");
      break;
    }
  p_dest (fp, opcode);
  fprintf (fp, " VF%02ld", (opcode >> 16) & 0x1f);
  p_dest (fp, opcode);
  fprintf (fp, ", VF%02ld", (opcode >> 11) & 0x1f);
  p_dest (fp, opcode);
}

static void
par_reg_s (FILE *fp, u_long opcode)
{
  fprintf (fp, " ACC");
  p_dest (fp, opcode);
  fprintf (fp, ", VF%02ld", (opcode >> 11) & 0x1f);
  p_dest (fp, opcode);
}

static void
mula_q (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "MULAq/");
      p_dest (fp, opcode);
      par_reg_s (fp, opcode);
      fprintf (fp, ", Q");
      break;
    case 1:
      fprintf (fp, "ABS/");
      p_dest (fp, opcode);
      fprintf (fp, " VF%02ld", (opcode >> 11) & 0x1f);
      p_dest (fp, opcode);
      fprintf (fp, ", VF%02ld", (opcode >> 16) & 0x1f);
      p_dest (fp, opcode);
      break;
    case 2:
      fprintf (fp, "MULAi/");
      p_dest (fp, opcode);
      par_reg_s (fp, opcode);
      fprintf (fp, ", I");
      break;
    case 3:
      fprintf (fp, "CLIPw/xyz");
      fprintf (fp, " VF%02ldxyz", (opcode >> 11) & 0x1f);
      fprintf (fp, ", VF%02ldw", (opcode >> 16) & 0x1f);
      break;
    }
}

static void
adda_q (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "ADDAq/");
      p_dest (fp, opcode);
      par_reg_s (fp, opcode);
      fprintf (fp, ", Q");
      break;
    case 1:
      fprintf (fp, "MADDAq/");
      p_dest (fp, opcode);
      par_reg_s (fp, opcode);
      fprintf (fp, ", Q");
      break;
    case 2:
      fprintf (fp, "ADDAi/");
      p_dest (fp, opcode);
      par_reg_s (fp, opcode);
      fprintf (fp, ", I");
      break;
    case 3:
      fprintf (fp, "MADDAi/");
      p_dest (fp, opcode);
      par_reg_s (fp, opcode);
      fprintf (fp, ", I");
      break;
    }
}

static void
suba_q (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "SUBAq/");
      p_dest (fp, opcode);
      par_reg_s (fp, opcode);
      fprintf (fp, ", Q");
      break;
    case 1:
      fprintf (fp, "MSUBAq/");
      p_dest (fp, opcode);
      par_reg_s (fp, opcode);
      fprintf (fp, ", Q");
      break;
    case 2:
      fprintf (fp, "SUBAi/");
      p_dest (fp, opcode);
      par_reg_s (fp, opcode);
      fprintf (fp, ", I");
      break;
    case 3:
      fprintf (fp, "MSUBAi/");
      p_dest (fp, opcode);
      par_reg_s (fp, opcode);
      fprintf (fp, ", I");
      break;
    }
}

static void
adda (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "ADDA/");
      p_dest (fp, opcode);
      par_reg_s (fp, opcode);
      fprintf (fp, ", VF%02ld", (opcode >> 16) & 0x1f);
      p_dest (fp, opcode);
      break;
    case 1:
      fprintf (fp, "MADDA/");
      p_dest (fp, opcode);
      par_reg_s (fp, opcode);
      fprintf (fp, ", VF%02ld", (opcode >> 16) & 0x1f);
      p_dest (fp, opcode);
      break;
    case 2:
      fprintf (fp, "MULA/");
      p_dest (fp, opcode);
      par_reg_s (fp, opcode);
      fprintf (fp, ", VF%02ld", (opcode >> 16) & 0x1f);
      p_dest (fp, opcode);
      break;
    case 3:
      fprintf (fp, "Undefined opcode(%08lx)", opcode);
      break;
    }
}

static void
suba (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "SUBA/");
      p_dest (fp, opcode);
      par_reg_s (fp, opcode);
      fprintf (fp, ", VF%02ld", (opcode >> 16) & 0x1f);
      p_dest (fp, opcode);
      break;
    case 1:
      fprintf (fp, "MSUBA/");
      p_dest (fp, opcode);
      par_reg_s (fp, opcode);
      fprintf (fp, ", VF%02ld", (opcode >> 16) & 0x1f);
      p_dest (fp, opcode);
      break;
    case 2:
      fprintf (fp, "OPMULA/xyz ACCxyz, VF%02ld", (opcode >> 11) & 0x1f);
      p_dest (fp, opcode);
      fprintf (fp, ", VF%02ld", (opcode >> 16) & 0x1f);
      p_dest (fp, opcode);
      break;
    case 3:
      fprintf (fp, "NOP");
      break;
    }
}

static void
special (FILE *fp, u_long opcode)
{
  switch ((opcode >> 6) & 0xf)
    {				/* bit41-38 */
    case 0:
      fprintf (fp, "ADDA");
      p_bc (fp, opcode);
      bc_acc (fp, opcode);
      break;
    case 1:
      fprintf (fp, "SUBA");
      p_bc (fp, opcode);
      bc_acc (fp, opcode);
      break;
    case 2:
      fprintf (fp, "MADDA");
      p_bc (fp, opcode);
      bc_acc (fp, opcode);
      break;
    case 3:
      fprintf (fp, "MSUBA");
      p_bc (fp, opcode);
      bc_acc (fp, opcode);
      break;
    case 4:
      fprintf (fp, "ITOF");
      itof (fp, opcode);
      break;
    case 5:
      fprintf (fp, "FTOI");
      itof (fp, opcode);
      break;
    case 6:
      fprintf (fp, "MULA");
      p_bc (fp, opcode);
      bc_acc (fp, opcode);
      break;
    case 7:
      mula_q (fp, opcode);
      break;
    case 8:
      adda_q (fp, opcode);
      break;
    case 9:
      suba_q (fp, opcode);
      break;
    case 10:
      adda (fp, opcode);
      break;
    case 11:
      suba (fp, opcode);
      break;
    default:
      fprintf (fp, "Undefined opcode(%08lx)", opcode);
      break;
    }
}

static void
quad_load (FILE *fp, u_long opcode)
{
  switch ((opcode >> 25) & 0x3)
    {
    case 0:
      fprintf (fp, "LQ/");
      p_dest (fp, opcode);
      fprintf (fp, " VF%02ld", (opcode >> 16) & 0x1f);
      p_dest (fp, opcode);
      fprintf (fp, ", %ld(VI%02ld)", opcode & 0x7ff, (opcode >> 11) & 0x1f);
      break;
    case 1:
      fprintf (fp, "SQ/");
      p_dest (fp, opcode);
      fprintf (fp, " VF%02ld", (opcode >> 11) & 0x1f);
      p_dest (fp, opcode);
      fprintf (fp, ", %ld(VI%02ld)", opcode & 0x7ff, (opcode >> 16) & 0x1f);
      break;
    default:
      fprintf (fp, "Undefined opcode(%08lx)", opcode);
      break;
    }
}

static void
int_load (FILE *fp, u_long opcode)
{
  switch ((opcode >> 25) & 0x3)
    {
    case 0:
      fprintf (fp, "ILW/");
      p_dest (fp, opcode);
      fprintf (fp, " VI%02ld", (opcode >> 16) & 0x1f);
      fprintf (fp, ", %ld(VI%02ld)", opcode & 0x7ff, (opcode >> 11) & 0x1f);
      break;
    case 1:
      fprintf (fp, "ISW/");
      p_dest (fp, opcode);
      fprintf (fp, " VI%02ld", (opcode >> 16) & 0x1f);
      fprintf (fp, ", %ld(VI%02ld)", opcode & 0x7ff, (opcode >> 11) & 0x1f);
      break;
    default:
      fprintf (fp, "Undefined opcode(%08lx)", opcode);
      break;
    }
}

static void
int_br (FILE *fp, u_long opcode)
{
  switch ((opcode >> 25) & 0x3)
    {
    case 0:
      fprintf (fp, "IBEQ");
      fprintf (fp, " VI%02ld, VI%02ld, 0x%lx",
	      (opcode >> 16) & 0x1f, (opcode >> 11) & 0x1f, opcode & 0x7ff);
      break;
    case 1:
      fprintf (fp, "IBNE");
      fprintf (fp, " VI%02ld, VI%02ld, 0x%lx",
	      (opcode >> 16) & 0x1f, (opcode >> 11) & 0x1f, opcode & 0x7ff);
      break;
    default:
      fprintf (fp, "Undefined opcode(%08lx)", opcode);
      break;
    }
}

static void
cond_int_br (FILE *fp, u_long opcode)
{
  switch ((opcode >> 25) & 0x3)
    {
    case 0:
      fprintf (fp, "IBLTZ");
      fprintf (fp, " VI%02ld, 0x%lx", (opcode >> 11) & 0x1f, opcode & 0x7ff);
      break;
    case 1:
      fprintf (fp, "IBGTZ");
      fprintf (fp, " VI%02ld, 0x%lx", (opcode >> 11) & 0x1f, opcode & 0x7ff);
      break;
    case 2:
      fprintf (fp, "IBLEZ");
      fprintf (fp, " VI%02ld, 0x%lx", (opcode >> 11) & 0x1f, opcode & 0x7ff);
      break;
    case 3:
      fprintf (fp, "IBGEZ");
      fprintf (fp, " VI%02ld, 0x%lx", (opcode >> 11) & 0x1f, opcode & 0x7ff);
      break;
    }
}

static void
br (FILE *fp, u_long opcode)
{
  switch ((opcode >> 25) & 0x3)
    {
    case 0:
      fprintf (fp, "B 0x%lx", opcode & 0x7ff);
      break;
    case 1:
      fprintf (fp, "BAL VI%02ld, 0x%lx", (opcode >> 16) & 0x1f, opcode & 0x7ff);
      break;
    default:
      fprintf (fp, "Undefined opcode(%08lx)", opcode);
      break;
    }
}

static void
jr (FILE *fp, u_long opcode)
{
  switch ((opcode >> 25) & 0x3)
    {
    case 0:
      fprintf (fp, "JR VI%02ld", (opcode >> 11) & 0x1f);
      break;
    case 1:
      fprintf (fp, "JALR VI%02ld, VI%02ld",
	      (opcode >> 16) & 0x1f, (opcode >> 11) & 0x1f);
      break;
    default:
      fprintf (fp, "Undefined opcode(%08lx)", opcode);
      break;
    }
}

static void
uint_add (FILE *fp, u_long opcode)
{
  u_int imm;

  switch ((opcode >> 25) & 0x3)
    {
    case 0:
      fprintf (fp, "IADDIU");
      imm = ((opcode >> 10) & 0x7800) | (opcode & 0x7ff);
      fprintf (fp, " VI%02ld, VI%02ld, %d",
	      (opcode >> 16) & 0x1f, (opcode >> 11) & 0x1f, imm);
      break;
    case 1:
      fprintf (fp, "ISUBIU");
      imm = ((opcode >> 10) & 0x7800) | (opcode & 0x7ff);
      fprintf (fp, " VI%02ld, VI%02ld, %d",
	      (opcode >> 16) & 0x1f, (opcode >> 11) & 0x1f, imm);
      break;
    default:
      fprintf (fp, "Undefined opcode(%08lx)", opcode);
      break;
    }
}

static void
fceq (FILE *fp, u_long opcode)
{
  switch ((opcode >> 25) & 0x3)
    {
    case 0:
      fprintf (fp, "FCEQ 0x%lx", opcode & 0x00ffffff);
      break;
    case 1:
      fprintf (fp, "FCSET 0x%lx", opcode & 0x00ffffff);
      break;
    case 2:
      fprintf (fp, "FCAND 0x%lx", opcode & 0x00ffffff);
      break;
    case 3:
      fprintf (fp, "FCOR 0x%lx", opcode & 0x00ffffff);
      break;
    }
}

static void
fseq (FILE *fp, u_long opcode)
{
  u_long imm12;

  imm12 = ((opcode & 0x00200000) >> 10) |
    (fp, opcode & 0x7ff);
  switch ((opcode >> 25) & 0x3)
    {
    case 0:
      fprintf (fp, "FSEQ VI%02ld, 0x%lx", (opcode >> 16) & 0x1f, imm12);
      break;
    case 1:
      fprintf (fp, "FSSET 0x%lx", imm12);
      break;
    case 2:
      fprintf (fp, "FSAND VI%02ld, 0x%lx", (opcode >> 16) & 0x1f, imm12);
      break;
    case 3:
      fprintf (fp, "FSOR VI%02ld, 0x%lx", (opcode >> 16) & 0x1f, imm12);
      break;
    }
}

static void
fcget (FILE *fp, u_long opcode)
{
  switch ((opcode >> 25) & 0x3)
    {
    case 0:
      fprintf (fp, "FCGET VI%02ld", (opcode >> 16) & 0x1f);
      break;
    default:
      fprintf (fp, "Undefined opcode(%08lx)", opcode);
      break;
    }
}

static void
fmand (FILE *fp, u_long opcode)
{
  switch ((opcode >> 25) & 0x3)
    {
    case 0:
      fprintf (fp, "FMEQ VI%02ld, VI%02ld",
	      (opcode >> 16) & 0x1f, (opcode >> 11) & 0x1f);
      break;
    case 2:
      fprintf (fp, "FMAND VI%02ld, VI%02ld",
	      (opcode >> 16) & 0x1f, (opcode >> 11) & 0x1f);
      break;
    case 3:
      fprintf (fp, "FMOR VI%02ld, VI%02ld",
	      (opcode >> 16) & 0x1f, (opcode >> 11) & 0x1f);
      break;
    default:
      fprintf (fp, "Undefined opcode(%08lx)", opcode);
      break;
    }
}

static void
iand (FILE *fp, u_long opcode)
{
  /* int imm5; -=UNUSED=- */
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "IAND VI%02ld, VI%02ld, VI%02ld",
	      (opcode >> 6) & 0x1f, (opcode >> 11) & 0x1f,
	      (opcode >> 16) & 0x1f);
      break;
    case 1:
      fprintf (fp, "IOR VI%02ld, VI%02ld, VI%02ld",
	      (opcode >> 6) & 0x1f, (opcode >> 11) & 0x1f,
	      (opcode >> 16) & 0x1f);
      break;
    default:
      fprintf (fp, "Undefined opcode(%08lx)", opcode);
      break;
    }
}
static void
iadd (FILE *fp, u_long opcode)
{
  int imm5;
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "IADD VI%02ld, VI%02ld, VI%02ld",
	      (opcode >> 6) & 0x1f, (opcode >> 11) & 0x1f,
	      (opcode >> 16) & 0x1f);
      break;
    case 1:
      fprintf (fp, "ISUB VI%02ld, VI%02ld, VI%02ld",
	      (opcode >> 6) & 0x1f, (opcode >> 11) & 0x1f,
	      (opcode >> 16) & 0x1f);
      break;
    case 2:
      if (opcode & 0x0400)
	{
	  imm5 = ((opcode >> 6) & 0x1f) | 0xffffffe0;
	}
      else
	imm5 = ((opcode >> 6) & 0x1f);
      fprintf (fp, "IADDI VI%02ld, VI%02ld, %d",
	      (opcode >> 16) & 0x1f, (opcode >> 11) & 0x1f, imm5);
      break;
    default:
      fprintf (fp, "Undefined opcode(%08lx)", opcode);
      break;
    }
}

static void
move (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      if (((opcode >> 11 & 0x1f) == 0) && ((opcode >> 16) & 0x1f) == 0)
	{
	  fprintf (fp, "NOP");
	}
      else
	{
	  fprintf (fp, "MOVE/");
	  p_dest (fp, opcode);
	  fprintf (fp, " VF%02ld", (opcode >> 16) & 0x1f);
	  p_dest (fp, opcode);
	  fprintf (fp, ", VF%02ld", (opcode >> 11) & 0x1f);
	  p_dest (fp, opcode);
	}
      break;
    case 1:
      fprintf (fp, "MR32/");
      p_dest (fp, opcode);
      fprintf (fp, " VF%02ld", (opcode >> 16) & 0x1f);
      p_dest (fp, opcode);
      fprintf (fp, ", VF%02ld", (opcode >> 11) & 0x1f);
      p_dest (fp, opcode);
      break;
    default:
      fprintf (fp, "Undefined opcode(%08lx)", opcode);
      break;
    }
}

static void
pinc_load (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "LQI/");
      p_dest (fp, opcode);
      fprintf (fp, " VF%02ld", (opcode >> 16) & 0x1f);
      p_dest (fp, opcode);
      fprintf (fp, ",(VI%02ld++)", (opcode >> 11) & 0x1f);
      break;
    case 1:
      fprintf (fp, "SQI/");
      p_dest (fp, opcode);
      fprintf (fp, " VF%02ld", (opcode >> 11) & 0x1f);
      p_dest (fp, opcode);
      fprintf (fp, ",(VI%02ld++)", (opcode >> 16) & 0x1f);
      break;
    case 2:
      fprintf (fp, "LQD/");
      p_dest (fp, opcode);
      fprintf (fp, " VF%02ld", (opcode >> 16) & 0x1f);
      p_dest (fp, opcode);
      fprintf (fp, ",(--VI%02ld)", (opcode >> 11) & 0x1f);
      break;
    case 3:
      fprintf (fp, "SQD/");
      p_dest (fp, opcode);
      fprintf (fp, " VF%02ld", (opcode >> 11) & 0x1f);
      p_dest (fp, opcode);
      fprintf (fp, ",(--VI%02ld)", (opcode >> 16) & 0x1f);
      break;
    }
}

static void
p_ftf (FILE *fp, u_long ftf)
{
  switch (ftf)
    {
    case 0:
      fputc('x', fp);
      break;
    case 1:
      fputc('y', fp);
      break;
    case 2:
      fputc('z', fp);
      break;
    case 3:
      fputc('w', fp);
      break;
    }
}

static void
mftir (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "MTIR");
      fprintf (fp, " VI%02ld, VF%02ld",
	      (opcode >> 16) & 0x1f, (opcode >> 11) & 0x1f);
      p_ftf (fp, (opcode >> 21) & 0x3);
      break;
    case 1:
      fprintf (fp, "MFIR/");
      p_dest (fp, opcode);
      fprintf (fp, " VF%02ld", (opcode >> 16) & 0x1f);
      p_dest (fp, opcode);
      fprintf (fp, ", VI%02ld", (opcode >> 11) & 0x1f);
      break;
    case 2:
      fprintf (fp, "ILWR/");
      p_dest (fp, opcode);
      fprintf (fp, " VI%02ld, VI%02ld",
	      (opcode >> 16) & 0x1f, (opcode >> 11) & 0x1f);
      break;
    case 3:
      fprintf (fp, "ISWR/");
      p_dest (fp, opcode);
      fprintf (fp, " VI%02ld, VI%02ld",
	      (opcode >> 16) & 0x1f, (opcode >> 11) & 0x1f);
      break;
    }
}

static void
sky_div (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "DIV Q, VF%02ld", (opcode >> 11) & 0x1f);
      p_ftf (fp, (opcode >> 21) & 0x3);
      fprintf (fp, ", VF%02ld", (opcode >> 16) & 0x1f);
      p_ftf (fp, (opcode >> 23) & 0x3);
      break;
    case 1:
      fprintf (fp, "SQRT Q, VF%02ld", (opcode >> 16) & 0x1f);
      p_ftf (fp, (opcode >> 21) & 0x3);
      break;
    case 2:
      fprintf (fp, "RSQRT Q, VF%02ld", (opcode >> 11) & 0x1f);
      p_ftf (fp, (opcode >> 21) & 0x3);
      fprintf (fp, ", VF%02ld", (opcode >> 16) & 0x1f);
      p_ftf (fp, (opcode >> 23) & 0x3);
      break;
    case 3:
      fprintf (fp, "WAITQ");
      break;
    }
}

static void
esadd_ersadd (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "ESADD");
      fprintf (fp, " P, VF%02ld", (opcode >> 11) & 0x1f);
      break;
    case 1:
      fprintf (fp, "ERSADD");
      fprintf (fp, " P, VF%02ld", (opcode >> 11) & 0x1f);
      break;
    case 2:
      fprintf (fp, "ELENG");
      fprintf (fp, " P, VF%02ld", (opcode >> 11) & 0x1f);
      break;
    case 3:
      fprintf (fp, "ERLENG");
      fprintf (fp, " P, VF%02ld", (opcode >> 11) & 0x1f);
      break;
    }
}

static void
esrsqrt (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "ESQRT");
      fprintf (fp, " P, VF%02ld", (opcode >> 11) & 0x1f);
      p_ftf (fp, (opcode >> 21) & 0x3);
      break;
    case 1:
      fprintf (fp, "ERSQRT");
      fprintf (fp, " P, VF%02ld", (opcode >> 11) & 0x1f);
      p_ftf (fp, (opcode >> 21) & 0x3);
      break;
    case 2:
      fprintf (fp, "ERCPR");
      fprintf (fp, " P, VF%02ld", (opcode >> 11) & 0x1f);
      p_ftf (fp, (opcode >> 21) & 0x3);
      break;
    case 3:
      fprintf (fp, "WAITP");
      break;
    }
}

static void
eatanxyz (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "EATANxy");
      fprintf (fp, " P, VF%02ld", (opcode >> 11) & 0x1f);
      break;
    case 1:
      fprintf (fp, "EATANxz");
      fprintf (fp, " P, VF%02ld", (opcode >> 11) & 0x1f);
      break;
    case 2:
      fprintf (fp, "ESUM");
      fprintf (fp, " P, VF%02ld", (opcode >> 11) & 0x1f);
      break;
    }
}

static void
esinatan (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "ESIN");
      fprintf (fp, " P, VF%02ld", (opcode >> 11) & 0x1f);
      p_ftf (fp, (opcode >> 21) & 0x3);
      break;
    case 1:
      fprintf (fp, "EATAN");
      fprintf (fp, " P, VF%02ld", (opcode >> 11) & 0x1f);
      p_ftf (fp, (opcode >> 21) & 0x3);
      break;
    case 2:
      fprintf (fp, "EEXP");
      fprintf (fp, " P, VF%02ld", (opcode >> 11) & 0x1f);
      p_ftf (fp, (opcode >> 21) & 0x3);
      break;
    }
}

static void
mfp (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "MFP/");
      p_dest (fp, opcode);
      fprintf (fp, " VF%02ld", (opcode >> 16) & 0x1f);
      p_dest (fp, opcode);
      fprintf (fp, ", P");
      break;
    }
}

static void
xgkick (FILE *fp, u_long opcode)
{
  fprintf (fp, "XGKICK VI%02ld", (opcode >> 11) & 0x1f);
}

static void
xitop (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "XTOP VI%02ld", (opcode >> 16) & 0x1f);
      break;
    case 1:
      fprintf (fp, "XITOP VI%02ld", (opcode >> 16) & 0x1f);
      break;
    }
}

static void
sky_rand (FILE *fp, u_long opcode)
{
  switch (opcode & 0x3)
    {
    case 0:
      fprintf (fp, "RNEXT/");
      p_dest (fp, opcode);
      fprintf (fp, " VF%02ld", (opcode >> 16) & 0x1f);
      p_dest (fp, opcode);
      fprintf (fp, ", R");
      break;
    case 1:
      fprintf (fp, "RGET/");
      p_dest (fp, opcode);
      fprintf (fp, " VF%02ld", (opcode >> 16) & 0x1f);
      p_dest (fp, opcode);
      fprintf (fp, ", R");
      break;
    case 2:
      fprintf (fp, "RINIT R, VF%02ld", (opcode >> 11) & 0x1f);
      p_ftf (fp, (opcode >> 21) & 0x3);
      break;
    case 3:
      fprintf (fp, "RXOR R, VF%02ld", (opcode >> 11) & 0x1f);
      p_ftf (fp, (opcode >> 21) & 0x3);
      break;
    }
}

static void
Special_Low (FILE *fp, u_long opcode)
{
  switch ((opcode >> 6) & 0x1f)
    {
    case 12:
      move (fp, opcode);
      break;
    case 13:
      pinc_load (fp, opcode);
      break;
    case 14:
      sky_div (fp, opcode);
      break;
    case 15:
      mftir (fp, opcode);
      break;
    case 16:
      sky_rand (fp, opcode);
      break;
    case 25:
      mfp (fp, opcode);
      break;
    case 26:
      xitop (fp, opcode);
      break;
    case 27:
      xgkick (fp, opcode);
      break;
    case 28:
      esadd_ersadd (fp, opcode);
      break;
    case 29:
      eatanxyz (fp, opcode);
      break;
    case 30:
      esrsqrt (fp, opcode);
      break;
    case 31:
      esinatan (fp, opcode);
      break;
    default:
      fprintf (fp, "Undefined opcode(%08lx)", opcode);
      break;
    }

}

static void
maj_spcial (FILE *fp, u_long opcode)
{
  if (((opcode >> 2) & 0xf) == 12)
    {
      iadd (fp, opcode);
    }
  else if (((opcode >> 2) & 0xf) == 13)
    {
      iand (fp, opcode);
    }
  else if (((opcode >> 2) & 0xf) == 15)
    {
      Special_Low (fp, opcode);
    }
  else
    {
      fprintf (fp, "Undefined opcode(%08lx)", opcode);
    }
}

void
opcode_analyze (FILE *fp, u_long * opcode, const char *separator)
{
  /* UpperOp */
  if (opcode[0] & 0x40000000)
    fprintf (fp, "NOP[e]");
  else
   switch ((opcode[0] >> 2) & 0xf)
    {				/* bit37-34 */
    case 0:
      fprintf (fp, "ADD");
      p_bc (fp, opcode[0]);
      bc_reg (fp, opcode[0]);
      break;
    case 1:
      fprintf (fp, "SUB");
      p_bc (fp, opcode[0]);
      bc_reg (fp, opcode[0]);
      break;
    case 2:
      fprintf (fp, "MADD");
      p_bc (fp, opcode[0]);
      bc_reg (fp, opcode[0]);
      break;
    case 3:
      fprintf (fp, "MSUB");
      p_bc (fp, opcode[0]);
      bc_reg (fp, opcode[0]);
      break;
    case 4:
      fprintf (fp, "MAX");
      p_bc (fp, opcode[0]);
      bc_reg (fp, opcode[0]);
      break;
    case 5:
      fprintf (fp, "MINI");
      p_bc (fp, opcode[0]);
      bc_reg (fp, opcode[0]);
      break;
    case 6:
      fprintf (fp, "MUL");
      p_bc (fp, opcode[0]);
      bc_reg (fp, opcode[0]);
      break;
    case 7:
      mul_q (fp, opcode[0]);
      break;
    case 8:
      add_q (fp, opcode[0]);
      break;
    case 9:
      sub_q (fp, opcode[0]);
      break;
    case 10:
      sky_add (fp, opcode[0]);
      break;
    case 11:
      sub (fp, opcode[0]);
      break;
    case 15:
      special (fp, opcode[0]);
      break;
    default:
      fprintf (fp, "Undefined opcode(%08lx)", opcode[0]);
      break;
    }

  fputs (separator, fp);

  /* LowerOp */
  if (opcode[0] & 0x80000000)
    {
      fprintf (fp, "LOI %f", *(float *) &opcode[1]);
    }
  else if (opcode[0] & 0x40000000)
    {
      fprintf (fp, "NOP");
    }
  else if (opcode[0] & 0x04000000)
    {
      fprintf (fp, "PRT %08lx", opcode[1]);
    }
  else
    {
      switch ((opcode[1] >> 27) & 0x1f)
	{
	case 0:
	  quad_load (fp, opcode[1]);
	  break;
	case 1:
	  int_load (fp, opcode[1]);
	  break;
	case 2:
	  uint_add (fp, opcode[1]);
	  break;
	case 4:
	  fceq (fp, opcode[1]);
	  break;
	case 5:
	  fseq (fp, opcode[1]);
	  break;
	case 6:
	  fmand (fp, opcode[1]);
	  break;
	case 7:
	  fcget (fp, opcode[1]);
	  break;
	case 8:
	  br (fp, opcode[1]);
	  break;
	case 9:
	  jr (fp, opcode[1]);
	  break;
	case 10:
	  int_br (fp, opcode[1]);
	  break;
	case 11:
	  cond_int_br (fp, opcode[1]);
	  break;
	case 16:
	  maj_spcial (fp, opcode[1]);
	  break;
	default:
	  fprintf (fp, "Undefined opcode(%08lx)", opcode[1]);
	  break;
	}
    }
}
