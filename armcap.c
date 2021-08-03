/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright 2011-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Modifications are copyright Amazon. Any included source code is
 * copyrighted by the OpenSSL project and its contributors.
 */

/*
 * File is sourced from openssl/crypto/armcap.c and has been modified to
 * support RNDR and RNDRRS.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <openssl/crypto.h>
#include <unistd.h>

#include "arm_arch.h"

unsigned int OPENSSL_armcap_P = 0;

static sigset_t all_masked;

static sigjmp_buf ill_jmp;
static void ill_handler(int sig)
{
    siglongjmp(ill_jmp, sig);
}

/*
 * Following subroutines could have been inlined, but it's not all
 * ARM compilers support inline assembler...
 */
# ifdef __aarch64__
void _armv8_rng_probe(void);

size_t OPENSSL_rndr_asm(unsigned char *buf, size_t len);
size_t OPENSSL_rndrrs_asm(unsigned char *buf, size_t len);

size_t OPENSSL_rndr_bytes(unsigned char *buf, size_t len);
size_t OPENSSL_rndrrs_bytes(unsigned char *buf, size_t len);

static size_t OPENSSL_rndr_wrapper(size_t (*func)(unsigned char *, size_t), unsigned char *buf, size_t len)
{
    size_t buffer_size;
    int i;

    for (i = 0; i < 8; i++) {
        buffer_size = func(buf, len);
        if (buffer_size == len)
            break;
        usleep(5000);  /* 5000 microseconds (5 milliseconds) */
    }
    return buffer_size;
}

size_t OPENSSL_rndr_bytes(unsigned char *buf, size_t len)
{
    return OPENSSL_rndr_wrapper(OPENSSL_rndr_asm, buf, len);
}

size_t OPENSSL_rndrrs_bytes(unsigned char *buf, size_t len)
{
    return OPENSSL_rndr_wrapper(OPENSSL_rndrrs_asm, buf, len);
}
# endif

# if defined(__GNUC__) && __GNUC__>=2
void OPENSSL_cpuid_setup(void) __attribute__ ((constructor));
# endif

# if defined(__GLIBC__) && defined(__GLIBC_PREREQ)
#  if __GLIBC_PREREQ(2, 16)
#   include <sys/auxv.h>
#   define OSSL_IMPLEMENT_GETAUXVAL
#  endif
# endif
# if defined(__FreeBSD__)
#  include <sys/param.h>
#  if __FreeBSD_version >= 1200000
#   include <sys/auxv.h>
#   define OSSL_IMPLEMENT_GETAUXVAL

static unsigned long getauxval(unsigned long key)
{
  unsigned long val = 0ul;

  if (elf_aux_info((int)key, &val, sizeof(val)) != 0)
    return 0ul;

  return val;
}
#  endif
# endif

/*
 * ARM puts the feature bits for Crypto Extensions in AT_HWCAP2, whereas
 * AArch64 used AT_HWCAP.
 */
# if defined(__aarch64__)
                                  /* AT_HWCAP2 */
#  define HWCAP2                 26
#  define HWCAP2_RNG             (1 << 16)
# endif

void OPENSSL_cpuid_setup(void)
{
    const char *e;
    struct sigaction ill_oact, ill_act;
    sigset_t oset;
    static int trigger = 0;

    if (trigger)
        return;
    trigger = 1;

    OPENSSL_armcap_P = 0;

    if ((e = getenv("OPENSSL_armcap"))) {
        OPENSSL_armcap_P = (unsigned int)strtoul(e, NULL, 0);
        return;
    }


#ifdef __aarch64__
# ifdef OSSL_IMPLEMENT_GETAUXVAL
    if (getauxval(HWCAP2) & HWCAP2_RNG)
        OPENSSL_armcap_P |= ARMV8_RNG;
# else
    sigfillset(&all_masked);
    sigdelset(&all_masked, SIGILL);
    sigdelset(&all_masked, SIGTRAP);
    sigdelset(&all_masked, SIGFPE);
    sigdelset(&all_masked, SIGBUS);
    sigdelset(&all_masked, SIGSEGV);

    memset(&ill_act, 0, sizeof(ill_act));
    ill_act.sa_handler = ill_handler;
    ill_act.sa_mask = all_masked;

    sigprocmask(SIG_SETMASK, &ill_act.sa_mask, &oset);
    sigaction(SIGILL, &ill_act, &ill_oact);

    /* If we used getauxval, we already have all the values */
    if (sigsetjmp(ill_jmp, 1) == 0) {
        _armv8_rng_probe();
        OPENSSL_armcap_P |= ARMV8_RNG;
    }
# endif
#endif
}
