/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright 2018 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Modifications are copyright Amazon. Any included source code is
 * copyrighted by the OpenSSL project and its contributors.
 */

/* File based on openssl/test/rdrand_sanitytest.c and has been modified. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arm_arch.h"

void OPENSSL_cpuid_setup(void);

/* Test helper functions */
int TEST_int_eq(int a, int b)
{
    int result;

    result = a == b;
    if (!result)
        printf("The following comparison failed: %d == %d\n", a, b);

    return result;
}

int TEST_false(int a)
{
    int result;

    result = !a;
    if (!result)
        printf("The following comparison failed: %d is false\n", a);

    return result;
}

int TEST_int_le(int a, int b)
{
    int result;

    result = a <= b;
    if (!result)
        printf("The following comparison failed: %d <= %d\n", a, b);

    return result;
}

int TEST_int_ge(int a, int b)
{
    int result;

    result = a >= b;
    if (!result)
        printf("The following comparison failed: %d >= %d\n", a, b);

    return result;
}

void TEST_run(int test_result)
{
    if (test_result == 1)
        printf("Test succeeded\n");
    else {
        printf("Test failed withs status: %d\n", test_result);
        exit(test_result);
    }
}


#ifdef __aarch64__
size_t OPENSSL_rndrrs_bytes(unsigned char *buf, size_t len);
size_t OPENSSL_rndr_bytes(unsigned char *buf, size_t len);

static int sanity_check_bytes(size_t (*rng)(unsigned char *, size_t),
    int rounds, int min_failures, int max_retries, int max_zero_words)
{
    int testresult = 0;
    unsigned char prior[31] = {0}, buf[31] = {0}, check[7];
    int failures = 0, zero_words = 0;

    int i;
    for (i = 0; i < rounds; i++) {
        size_t generated = 0;

        int retry;
        for (retry = 0; retry < max_retries; retry++) {
            generated = rng(buf, sizeof(buf));
            if (generated == sizeof(buf))
                break;
            failures++;
        }

        /*-
         * Verify that we don't have too many unexpected runs of zeroes
         */
        size_t j;
        for (j = 0; j < sizeof(buf) - 1; j++) {
            if (buf[j] == 0 && buf[j+1] == 0) {
                zero_words++;
            }
        }

        if (!TEST_int_eq(generated, sizeof(buf)))
            goto end;
        if (!TEST_false(!memcmp(prior, buf, sizeof(buf))))
            goto end;

        /* Verify that the last 7 bytes of buf aren't all the same value */
        unsigned char *tail = &buf[sizeof(buf) - sizeof(check)];
        memset(check, tail[0], 7);
        if (!TEST_false(!memcmp(check, tail, sizeof(check))))
            goto end;

        /* Save the result and make sure it's different next time */
        memcpy(prior, buf, sizeof(buf));
    }

    if (!TEST_int_le(zero_words, max_zero_words))
        goto end;

    if (!TEST_int_ge(failures, min_failures))
        goto end;

    testresult = 1;
end:
    return testresult;
}

static int sanity_check_rndr_bytes(void)
{
    printf("Running 'sanity_check_rndr_bytes'...\n");
    return sanity_check_bytes(OPENSSL_rndr_bytes, 1000, 0, 10, 10);
}

static int sanity_check_rndrrs_bytes(void)
{
    printf("Running 'sanity_check_rndrrs_bytes'...\n");
    return sanity_check_bytes(OPENSSL_rndrrs_bytes, 1000, 0, 10000, 10);
}

int setup_tests(void)
{
    printf("Loading test...\n");
    OPENSSL_cpuid_setup();

    int have_rndr_rndrrs = (OPENSSL_armcap_P & (1 << 8)) != 0;

    if (have_rndr_rndrrs) {
        TEST_run(sanity_check_rndr_bytes());
        TEST_run(sanity_check_rndrrs_bytes());
    } else {
        printf("Unable to access rndr and/or rndrrs\n");
        return 1;
    }

    return 0;
}
#else

int setup_tests(void)
{
    printf("Unable to load test.\n");
    return 1;
}

#endif

int main(int argc, char *argv[]) {
    exit(setup_tests());
}
