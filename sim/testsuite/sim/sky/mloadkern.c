/* Copyright 1998 Cygnus Solutions */


unsigned int
load_user_code()
{
  unsigned int entry;
  __asm__ __volatile__ ("break 0xffff1; add %0,$4,$0"
			: "=r" (entry) /* outputs */
			: /* no inputs */
			: "$4", "memory");
  return entry;
}


void
branch_user_code(unsigned int pc)
{
  void (*entry) () = (void (*)()) pc;
  (*entry)(); /* will probably not return */
}



int
main()
{
  unsigned int pc;
  int count = 0;

  while(1)
    {
      count ++;
      printf("Preparing load user code, iteration %d\n", count);
      pc = load_user_code();
      printf("Loaded, PC=0x%08lx.\n", (long) pc);
      if(pc != 0)
	{
	  printf("Branching to user code.\n");
	  branch_user_code(pc);
	  printf("Returned from user code.\n");
	}
      else
	{
	  printf("Cannot branch, exiting.\n");
	  exit(1);
	}
    }
}
