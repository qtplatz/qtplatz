//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "JUtils.h"

time_t JUtils::filetimeToUnixTime( int *ft )
{
	typedef unsigned int UINT32;

	UINT32 a0;			// 16 bit, low    bits
	UINT32 a1;			// 16 bit, medium bits
	UINT32 a2;			// 32 bit, high   bits
	unsigned int carry;	// carry bit for subtraction
	int negative;		// whether a represents a negative value

	// Copy the time values to a2/a1/a0
	a2 =  ( UINT32 ) ft[ 1 ];
	a1 = ( ( UINT32 ) ft[ 0 ] ) >> 16;
	a0 = ( ( UINT32 ) ft[ 0 ] ) & 0xffff;

	// Subtract the time difference
	if ( a0 >= 32768 ) a0 -= 32768, carry = 0;
	else a0 += ( 1 << 16 ) - 32768, carry = 1;

	if ( a1 >= 54590    + carry ) a1 -= 54590 + carry, carry = 0;
	else a1 += (1 << 16) - 54590 - carry, carry = 1;

	a2 -= 27111902 + carry;

	// If a is negative, replace a by (-1-a)
	negative = (a2 >= ((UINT32)1) << 31);
	if (negative)
	{
		// Set a to -a - 1 (a is a2/a1/a0)
		a0 = 0xffff - a0;
		a1 = 0xffff - a1;
		a2 = ~a2;
	}

	// Divide a by 10000000 (a = a2/a1/a0), put the rest into r.
	// Split the divisor into 10000 * 1000 which are both less than 0xffff.

	a1 += ( a2 % 10000 ) << 16;
	a2 /= 10000;
	a0 += ( a1 % 10000 ) << 16;
	a1 /= 10000;
	a0 /= 10000;

	a1 += ( a2 % 1000 ) << 16;
	a2 /= 1000;
	a0 += ( a1 % 1000 ) << 16;
	a1 /= 1000;
	a0 /= 1000;

	if (negative)
	{
		// Set a to -a - 1 (a is a2/a1/a0)
		a0 = 0xffff - a0;
		a1 = 0xffff - a1;
		a2 = ~a2;
	}

	// Do not replace this by << 32, it gives a compiler warning and it does
	// not work.
	
	return ( ( ( ( time_t ) a2 ) << 16 ) << 16 ) + ( a1 << 16 ) + a0;
}

