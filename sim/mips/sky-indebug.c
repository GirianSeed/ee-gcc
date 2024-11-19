#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "sim-main.h"
#include "sky-indebug.h"

int
indebug (char *p)
{
  if ( sky_debug_string == NULL )
    return (0);
  else
    return (strstr ( sky_debug_string, p) != 0);
}
