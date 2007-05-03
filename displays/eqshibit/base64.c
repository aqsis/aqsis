#include <string.h>
#include <stdio.h>

#include "base64.h"

static const unsigned char *b64_tbl =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const unsigned char b64_pad = '=';

/* base64 encode a group of between 1 and 3 input chars into a group of
 * 4 output chars */
static void encode_group(unsigned char output[], const unsigned char
input[], int n)
{
        unsigned char ingrp[3];

        ingrp[0] = n > 0 ? input[0] : 0;
        ingrp[1] = n > 1 ? input[1] : 0;
        ingrp[2] = n > 2 ? input[2] : 0;

        /* upper 6 bits of ingrp[0] */
        output[0] = n > 0 ? b64_tbl[ingrp[0] >> 2] : b64_pad;
        /* lower 2 bits of ingrp[0] | upper 4 bits of ingrp[1] */
        output[1] = n > 0 ? b64_tbl[((ingrp[0] & 0x3) << 4) | (ingrp[1]
>> 4)]
                : b64_pad;
        /* lower 4 bits of ingrp[1] | upper 2 bits of ingrp[2] */
        output[2] = n > 1 ? b64_tbl[((ingrp[1] & 0xf) << 2) | (ingrp[2]
>> 6)]
                : b64_pad;
        /* lower 6 bits of ingrp[2] */
        output[3] = n > 2 ? b64_tbl[ingrp[2] & 0x3f] : b64_pad;

}

int b64_encode(char *dest, const char *src, int len)
{
        int outsz = 0;

        while (len > 0) {
                encode_group(dest + outsz, src, len > 3 ? 3 : len);
                len -= 3;
                src += 3;
                outsz += 4;
        }

        return outsz;
}

/* base64 decode a group of 4 input chars into a group of between 0 and
 * 3
 * output chars */
static void decode_group(unsigned char output[], const unsigned char
input[], int *n)
{
        unsigned char *t1,*t2;
        *n = 0;

        if (input[0] == b64_pad)
                return;

        if (input[1] == b64_pad) {
                fprintf(stderr, "Warning: orphaned bits ignored.\n");
                return;
        }

        t1 = strchr(b64_tbl, input[0]);
        t2 = strchr(b64_tbl, input[1]);

        if ((t1 == NULL) || (t2 == NULL)) {
                fprintf(stderr, "Warning: garbage found, giving up.\n");
                return;
        }

        output[(*n)++] = ((t1 - b64_tbl) << 2) | ((t2 - b64_tbl) >> 4);

        if (input[2] == b64_pad)
                return;

        t1 = strchr(b64_tbl, input[2]);

        if (t1 == NULL) {
                fprintf(stderr, "Warning: garbage found, giving up.\n");
                return;
        }

        output[(*n)++] = ((t2 - b64_tbl) << 4) | ((t1 - b64_tbl) >> 2);

        if (input[3] == b64_pad)
                return;

        t2 = strchr(b64_tbl, input[3]);

        if (t2 == NULL) {
                fprintf(stderr, "Warning: garbage found, giving up.\n");
                return;
        }

        output[(*n)++] = ((t1 - b64_tbl) << 6) | (t2 - b64_tbl);

        return;
}

int b64_decode(char *dest, const char *src)
{
        int len;
        int outsz = 0;

        while (*src) {
                decode_group(dest + outsz, src, &len);
                src += 4;
                outsz += len;
        }

        return outsz;
}
