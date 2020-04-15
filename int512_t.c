#include "int512_t.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int512_t int_to_int512(int num)
{
	int512_t result;
    int isNegative = 0;
    if (num < 0) {
        num *= -1;
        isNegative = 1;
    }

    for (int i = 0; i < 16; i++) {
        result.data[i] = 0;
        /* printf("i: %d\n", i); */
        for (int j = 0; j < 4; j++) {
            result.data[i] *= 0x100;
            uint32_t tmp = num % 0x10;
            num /= 0x10;
            tmp += (num % 0x10) * 0x10;
            num /= 0x10;
            result.data[i] += tmp;
            /* printf("%04x ", result.data[i]); */
        }
    }

/*     if (isNegative) { */
/*         // complement */
/*         for (int i = 0; i < 16; i++) { */
/*             result.data[i] = ~result.data[i]; */
/*         } */

/*         for (int j = 0; j < 16; j++) { */
/*             printf("%08x\n", result.data[j]); */
/*         } */
/*         // add 1 */
/*         int carry = 1; */
/*         for (int i = 15; i >= 0; i--) { */

/*             result.data[i] += 1; */
/*         } */
/*     } */

	return result;
}

bool int512_equal(const int512_t lhs, const int512_t rhs)
{
    for (int i = 0; i < 16; i++) {
        /* printf("%08X %08X %08X\n", lhs.data[i], rhs.data[i], lhs.data[i] ^ rhs.data[i]); */
        /* printf("%08X %08X\n", lhs.data[i], rhs.data[i]); */
        if ((lhs.data[i] ^ rhs.data[i]) != 0)
            return false;
    }

	return true;
}

bool int512_greater(const int512_t lhs, const int512_t rhs)
{
    int lsign = (lhs.data[15] & 0x80) >> 7;
    int rsign = (rhs.data[15] & 0x80) >> 7;

    if (lsign != rsign) {
        return lsign < rsign;
    }

    for (int i = 15; i >= 0; i--) {
        uint32_t mask = 0xFF;

        for (int j = 0; j < 4; j++) {
            uint32_t lmask = lhs.data[i] & mask;
            uint32_t rmask = rhs.data[i] & mask;

            if (lmask != rmask)
                return lmask > rmask;

            mask = mask << 8;
        }
    }

	return false;
}

int512_t int512_add(int512_t lhs, int512_t rhs)
{
	int512_t result;

    uint8_t ltmp, rtmp;
    uint8_t carry = 0;
    for (int i = 0; i < 16; i++) {
        uint32_t mask = 0xFF;
        result.data[i] = 0;
        for (int j = 0; j < 4; j++) {
            result.data[i] <<= 8;
            ltmp = (lhs.data[i] >> 8 * (3 - j)) & mask;
            rtmp = (rhs.data[i] >> 8 * (3 - j)) & mask;
            result.data[i] += (ltmp + rtmp + carry) & mask;
            carry = (ltmp + rtmp) >> 8;
        }
    }

	return result;
}

int512_t int512_mul(int512_t lhs, int512_t rhs)
{
	int512_t result;

	// TODO: Put something!

	return result;
}

int512_t int512_div(int512_t lhs, int512_t rhs)
{
	int512_t result;

	// TODO: Put something!

	return result;
}

int512_t int512_mod(int512_t lhs, int512_t rhs)
{
	int512_t result;

	// TODO: Put something!

	return result;
}
