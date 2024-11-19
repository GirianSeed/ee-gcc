#ifndef SKY_BITS_H_
#define SKY_BITS_H_


#define SINGLE_BIT(n) (((u_long)1) << (n))
#define SINGLE_BIT(n) (((u_long)1) << (n))

#define SET_BIT(a, n)	(a) |= SINGLE_BIT(n)
#define RESET_BIT(a, n) (a) &= ~(SINGLE_BIT(n))

#endif
