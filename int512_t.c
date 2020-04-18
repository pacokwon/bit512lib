#include "int512_t.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t get_bit(int512_t num, int n) {
    /* n: 0 ~ 511 */
    int array_idx = 15 - n / 32;
    int nth = n % 32;
    int gauss_shift = nth / 8 * 8;

    return (num.data[array_idx] >> gauss_shift) >> (7 - (nth - gauss_shift)) & 0x1;
}

void set_bit(pint512_t num, int n, uint8_t bit) {
    int array_idx = 15 - n / 32;
    int nth = n % 32;
    int gauss_shift = nth / 8 * 8;
    num->data[array_idx] |= bit << (gauss_shift + 7 - (nth - gauss_shift));
}

bool get_sign(int512_t num) {
    return get_bit(num, 0);
}

int512_t negative(int512_t num) {
    for (int i = 0; i < 16; i++)
        num.data[i] = ~num.data[i];

    int512_t one;
    for (int i = 0; i < 16; i++)
        one.data[i] = 0;
    one.data[0] = (0x1 << 24);

    return int512_add(num, one);
}

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
        for (int j = 0; j < 4; j++) {
            result.data[i] *= 0x100;
            uint32_t tmp = num % 0x10;
            num /= 0x10;
            tmp += (num % 0x10) * 0x10;
            num /= 0x10;
            result.data[i] += tmp;
        }
    }

    if (isNegative) {
        // complement
        for (int i = 0; i < 16; i++) {
            result.data[i] = ~result.data[i];
        }

        int512_t one;
        for (int i = 0; i < 16; i++)
            one.data[i] = 0;
        one.data[0] = (0x1 << 24);

        result = int512_add(result, one);
    }

	return result;
}

bool int512_equal(const int512_t lhs, const int512_t rhs)
{
    for (int i = 0; i < 16; i++) {
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

            mask <<= 8;
        }
    }

	return false;
}

int512_t int512_add(int512_t lhs, int512_t rhs)
{
	int512_t result;

    uint8_t ltmp, rtmp;
    uint8_t carry = 0;
    uint8_t mask = 0xFF;
    for (int i = 0; i < 16; i++) {
        result.data[i] = 0;
        for (int j = 0; j < 4; j++) {
            result.data[i] <<= 8;
            ltmp = (lhs.data[i] >> 8 * (3 - j)) & mask;
            rtmp = (rhs.data[i] >> 8 * (3 - j)) & mask;
            result.data[i] += (ltmp + rtmp + carry) & mask;
            carry = (ltmp + rtmp + carry) >> 8;
        }
    }

	return result;
}

int512_t int512_mul(int512_t lhs, int512_t rhs)
{
	int512_t result;
    for (int i = 0; i < 16; i++)
        result.data[i] = 0;

    int carry = 0;
    for (int i = 0; i < 512; i++) {
        int sum = carry;
        for (int k = 0; k <= i; k++) {
            sum += get_bit(lhs, 511 - k) & get_bit(rhs, 511 - (i - k));
        }
        set_bit(&result, 511 - i, sum % 2);
        carry = sum / 2;
    }

	return result;
}

int512_t int512_div(int512_t lhs, int512_t rhs)
{
	int512_t result;

    int512_t zero = int_to_int512(0);
    int512_t one = int_to_int512(1);

    bool lsign = get_sign(lhs);
    bool rsign = get_sign(rhs);

    // positive on lhs, negative on rhs
    if (lsign) {
        if (rsign) {    // l is -, r is -
            lhs = negative(lhs);
        } else {        // l is -, r is +
            // using result as a temporary variable.
            // might want to make a dedicated variable if desired
            result = lhs;
            lhs = rhs;
            rhs = result;
        }
    } else {
        if (rsign) {}   // l is +, r is -, do nothing
        else {          // l is +, r is +
            rhs = negative(rhs);
        }
    }

    // set result to -1
    for (int i = 0; i < 16; i++)
        result.data[i] = 0xFFFFFFFF;

    // while (lhs >= zero)
    while (!int512_greater(zero, lhs)) {
        lhs = int512_add(lhs, rhs);
        result = int512_add(result, one);
    }


	return lsign ^ rsign ? negative(result) : result;
}

int512_t int512_mod(int512_t lhs, int512_t rhs)
{
    bool lsign = get_sign(lhs);
    bool rsign = get_sign(rhs);

    if (lsign)
        lhs = negative(lhs);

    if (rsign)
        rhs = negative(rhs);

    // lhs = q * rhs + r
    // r = lhs - q * rhs

    int512_t quotient = int512_div(lhs, rhs);
    int512_t rem = int512_add(lhs, negative(int512_mul(quotient, rhs)));

    return lsign ^ rsign ? negative(rem) : rem;
}
