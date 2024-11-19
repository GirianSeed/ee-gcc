/* Copyright (C) 1998 Cygnus Solutions */
/* PKE/DMA interrupt function test */

/* globals */

int num_ok = 0;
int num_errors = 0;
int int_count = 0;
int int_cause = 0;


/* macros */

#define TEST(x) do_test(x, #x, __LINE__)



/* prototypes */


void enable_interrupts();
void do_test(int ok, const char* test, int line);



/* Utility functions */

void
enable_interrupts()
{
  /* set IE [0] bit in SR */
  asm volatile ("mfc0 $3,$12; li $2,0x00000001; or $3,$3,$2; mtc0 $3,$12"
		: /* no outputs */
		: /* no inputs */
		: "$3", "$2" /* clobbered */);
}


void
disable_interrupts()
{
  asm volatile ("mfc0 $3,$12; li $2,0xfffffffe; and $3,$3,$2; mtc0 $3,$12"
		: /* no outputs */
		: /* no inputs */
		: "$3", "$2" /* clobbered */);
}


void
do_test(int ok, const char* test, int line)
{
  static int test_num = 0;

  printf("[%3d @ %3d] (%s): ", ++test_num, line, test);
  if(ok)
    {
      num_ok ++;
      printf("ok\n");
    }
  else
    {
      num_errors ++;
      printf("ko\n");
    }
}



/* Tests */


/* test00: test interrupt masking & basic logic */
void test00()
{
  TEST(int_count == 0);
  disable_interrupts();
  TEST(int_count == 0);
  enable_interrupts();
  TEST(int_count == 0);
}


/* test01: test PKE interrupt flagged instructions */
void test01()
{
#define SET(v,d3,d2,d1,d0) do { (v)[0]=(d0); (v)[1]=(d1); (v)[2]=(d2); (v)[3]=(d3); } while (0)
#define SEND(v,a) do { int i; for(i=0; i<4; i++) (a)[i] = (v)[i]; } while (0)

  volatile int* pke0_fifo  = (int*) 0xb0004000;
  volatile int* pke0_fbrst = (int*) 0xb0003810;
  unsigned int fbrst_cont = 0x00000008;

  unsigned int data[4];

  int_count = 0;
  enable_interrupts();
  TEST(int_count == 0);

  SET(data, 0,0,0,0); /* four PKENOPs */
  SEND(data, pke0_fifo);
  TEST(int_count == 0);

  SET(data, 0,0,0,0x80000000); /* PKENOP[i] as first operation */
  SEND(data, pke0_fifo);
  TEST(int_count == 1);
  TEST(int_cause & (1<<11)); /* IP3 */
  * pke0_fbrst = fbrst_cont;

  /* send it again, masking interrupts temporarily */
  disable_interrupts();
  SEND(data, pke0_fifo);
  TEST(int_count == 1);
  TEST(int_cause & (1<<11)); /* IP3 */
  enable_interrupts();
  TEST(int_count == 2);
  TEST(int_cause & (1<<11)); /* IP3 */
  * pke0_fbrst = fbrst_cont;

#undef SET
#undef SEND
}


/* test02: test DMA interrupt flagged instructions */
void test02()
{
#define START_DMA(d) do { *dma_d_ctrl = 0x01; *dma_d0_qwc = 0x999; *dma_d0_tadr = (unsigned int) & d[0]; *dma_d0_chcr = 0x000001c5; } while (0)
#define WAIT_DMA() do { ; } while (*dma_d0_qwc == 0x999)

  volatile int* dma_d_ctrl = (int*) 0xb000e000;
  volatile int* dma_d0_chcr = (int*) 0xb0008000;
  volatile int* dma_d0_qwc = (int*) 0xb0008020;
  volatile int* dma_d0_tadr = (int*) 0xb0008030;

  static unsigned int data[] __attribute__ ((aligned (16))) =
    {
      0x10000001, 0x00000000, 0x00000000, 0x00000000,  /* DMACNT 1 */
      0x00000000, 0x00000000, 0x00000000, 0x00000000,  /* 4 * PKENOP */
      0xf0000000, 0x00000000, 0x00000000, 0x00000000,  /* DMAEND[i] */
    };

  int_count = 0;
  enable_interrupts();
  TEST(int_count == 0);

  disable_interrupts();
  START_DMA(data);
  WAIT_DMA();
  TEST(int_count == 0);
  enable_interrupts();
  TEST(int_count == 1);

  START_DMA(data);
  WAIT_DMA();
  TEST(int_count == 2);
}





/* Mainline */


int main()
{
  /* tests */
  test00();
  test01();
  test02();

  /* summarize */
  printf("%d ok, %d bad\n", num_ok, num_errors);
  if(num_errors > 0)
    exit(47);
  else
    exit(0);
}
