#as:
#objdump: -dr
#name: gifreglist-1

.*: +file format .*

Disassembly of section .vutext:

0+0000 <foo>:
   0:	00 00 00 00 	gifreglist regs={rgbaq},nloop=0
   4:	00 00 00 14 
   8:	01 00 00 00 
   c:	00 00 00 00 

0+0010 .*:
  10:	00 80 00 00 	gifreglist regs={rgbaq},nloop=0,eop
  14:	00 00 00 14 
  18:	01 00 00 00 
  1c:	00 00 00 00 

0+0020 .*:
  20:	00 80 00 00 	gifreglist regs={rgbaq},nloop=0,eop
  24:	00 00 00 14 
  28:	01 00 00 00 
  2c:	00 00 00 00 

0+0030 .*:
  30:	00 00 00 00 	gifreglist regs={tex0_1,tex0_2,clamp_1,clamp_2},nloop=0
  34:	00 00 00 44 
  38:	76 98 00 00 
  3c:	00 00 00 00 

0+0040 .*:
  40:	03 00 00 00 	gifreglist regs={xyzf,xyzf3,xyz3},nloop=3
  44:	00 00 00 34 
  48:	ca 0d 00 00 
  4c:	00 00 00 00 
  50:	01 00 00 00 
  54:	01 00 00 00 
  58:	02 00 00 00 
  5c:	02 00 00 00 
  60:	03 00 00 00 
  64:	03 00 00 00 
  68:	04 00 00 00 
  6c:	04 00 00 00 
  70:	05 00 00 00 
  74:	05 00 00 00 
  78:	06 00 00 00 
  7c:	05 00 00 00 
  80:	07 00 00 00 
  84:	07 00 00 00 
  88:	08 00 00 00 
  8c:	08 00 00 00 
  90:	09 00 00 00 
  94:	09 00 00 00 
  98:	00 00 00 00 
  9c:	00 00 00 00 

0+00a0 .*:
  a0:	01 00 00 00 	gifreglist regs={xyzf,xyzf3},nloop=1
  a4:	00 00 00 24 
  a8:	ca 00 00 00 
  ac:	00 00 00 00 
  b0:	01 00 00 00 
  b4:	02 00 00 00 
  b8:	03 00 00 00 
  bc:	04 00 00 00 