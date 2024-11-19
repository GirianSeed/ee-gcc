
/* data for .bss section */ 
char buffer[1000];

void
_start()
{
  /* occupy the processor for a little while */
  int i, j;
  for(j=0; j<100; j++)
    for(i=0; i<sizeof(buffer); i++)
      buffer[i] = (char) (i ^ j);

  /* return */
  return;
}
