#ifndef SKY_INTERACT_H_
#define SKY_INTERACT_H_

#include <sky-vu.h>

void interactive (SIM_DESC sd, vu_device * me);

void print_pipe (SIM_DESC sd, FILE* fp, vu_device *me);
#endif
