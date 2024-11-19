/*  Copyright (C) 1998, Cygnus Solutions
*/

#include <stdlib.h>
#include "config.h"

#include "sim-main.h"
#include "sim-assert.h"
#include "sky-device.h"
#include "sky-psio.h"
#include "sky-indebug.h"

#ifdef HAVE_STRING_H
#include <string.h>
#else
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#endif

static int
psio_io_read_buffer (device *me,
                    void *dest,
                    int space,
                    address_word addr1,
                    unsigned nr_bytes,
                    sim_cpu *processor,
                    sim_cia cia);
static int
psio_io_write_buffer (device *me,
                     const void *source,
                     int space,
                     address_word addr,
                     unsigned nr_bytes,
                     sim_cpu *processor,
                     sim_cia cia);

struct psio_device psio_device = 
{ 
  { "PSIO Device",      /* Device                 */
    &psio_io_read_buffer,
    &psio_io_write_buffer
  }
};

void 
psio_attach (SIM_DESC sd) 
{
    sim_core_attach (sd,
                    NULL,
                    0,
                    access_read_write,
                    0,
                    PSIO_REGISTER_WINDOW_START,
                    PSIO_REGISTER_WINDOW_SIZE,
                    0,
                    &psio_device.dev,
                    NULL);
}

static int
psio_io_read_buffer (device *me,
                   void *dest,
                   int space,
                   address_word addr1,
                   unsigned nr_bytes,
                   sim_cpu *processor,
                   sim_cia cia)
{
    unsigned addr = (unsigned) addr1;
    unsigned *const pmem = (unsigned*) dest;
    unsigned temp;
    struct psio_device *psio = (struct psio_device *) me;

    ASSERT (addr >= PSIO_REGISTER_WINDOW_START && addr < PSIO_REGISTER_WINDOW_END);

    if (nr_bytes != 4)
        fprintf(stderr, "PSIO read i/o length != 4\n");

    switch (addr)
    {
    case PSIO_SI_CSR:
	/* TODO: Should return SIO_DONE only when character is ready. */
	*pmem = H2T_4(SIO_DONE); 
        break;
    case PSIO_SI_CDR:
	/* TODO: Should be non-blocking read. */
	*pmem = H2T_4(getchar());
	break;

    case PSIO_SO_CSR:
	*pmem = H2T_4(SIO_DONE);
	break;
    case PSIO_SO_CDR:
	*pmem = H2T_4(0);	/* Read undefined? */
	break;
    default:
        fprintf (stderr, "PSIO read on undefined register\n");
        *pmem = H2T_4(0);
	break;
    }

    return nr_bytes;
}

static int
psio_io_write_buffer (device *me,
                    const void *source,
                    int space,
                    address_word addr,
                    unsigned nr_bytes,
                    sim_cpu *cpu,
                    sim_cia cia)
{
    const unsigned *pmem = (unsigned *) source;
    unsigned requested, actual;
    struct psio_device *psio = (struct psio_device *) me;

    ASSERT (addr >= PSIO_REGISTER_WINDOW_START && addr < PSIO_REGISTER_WINDOW_END);
    if (nr_bytes != 4)
        fprintf(stderr, "PSIO write i/o length != 4\n");

    switch (addr) {
    case PSIO_SI_CSR:
	/* TODO: Should return SIO_DONE only when character is ready. */
        break;
    case PSIO_SI_CDR:
	/* Write undefined? */
	break;

    case PSIO_SO_CSR:
	/* TODO: Has START/DONE protocol. */
	break;
    case PSIO_SO_CDR:
	putchar(T2H_4(*pmem));
	break;
    default:
        fprintf (stderr, "PSIO write on undefined register\n");
	break;
    }

    return nr_bytes;
}
