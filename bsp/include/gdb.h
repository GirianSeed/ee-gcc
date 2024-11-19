/*
 * gdb.h -- Definitions for GDB stub.
 *
 * Copyright (c) 1998, 1999 Cygnus Support
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */
#ifndef __GDB_H__
#define __GDB_H__ 1

#include "gdb-cpu.h"

#ifndef DEBUG_STUB
#define DEBUG_STUB 0
#endif

/*
 *  These need to match the same in devo/gdb/target.h
 */
#define  TARGET_SIGNAL_INT  2
#define  TARGET_SIGNAL_ILL  4
#define  TARGET_SIGNAL_TRAP 5
#define  TARGET_SIGNAL_ABRT 6
#define  TARGET_SIGNAL_FPE  8
#define  TARGET_SIGNAL_BUS  10
#define  TARGET_SIGNAL_SEGV 11


/*
 *  Socket to use for tcp/ip debug channel.
 */
#define GDB_TCP_SOCKET  1000


#ifndef __ASSEMBLER__

/* generic gdb protocol handler */
extern void _bsp_gdb_handler(int exc_nr, void *saved_regs);


/* start forming an outgoing gdb packet */
/* if ack is true, prepend an ack character */
extern void _gdb_pkt_start(int ack);

/* Append data to packet using formatted string. */
extern void _gdb_pkt_append(char *fmt, ...);

/* Calculate checksum and append to end of packet. */
extern void _gdb_pkt_end(void);

/* Send the packet. Blocks waiting for ACK */
extern void _gdb_pkt_send(void);

/*
 * Convert the hex data in 'buf' into 'count' bytes to be placed in 'mem'.
 * Returns a pointer to the character in mem AFTER the last byte written.
 */
extern char *__unpack_bytes_to_mem(char *buf, char *mem, int count);

/*
 * While finding valid hex chars, build an unsigned long int.
 * Return number of hex chars processed.
 */
extern int __unpack_ulong(char **ptr, unsigned long *val);

/*
 * Unpack 'count' hex characters, forming them into a binary value.
 * Return that value as an int. Adjust the source pointer accordingly.
 */
extern int __unpack_nibbles(char **ptr, int count);


#endif

#endif /* __GDB_H__ */
