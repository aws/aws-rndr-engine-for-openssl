/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright 2011-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Modifications are copyright Amazon. Any included source code is
 * copyrighted by the OpenSSL project and its contributors.
 */

/*
 * File based on  openssl/crypto/engine/eng_rdrand.c and has been modified to
 * create an engine for RNDR.
 */

/* Some Engine APIs are deprecated with OpenSSL 3.0 and are required */
#define OPENSSL_SUPPRESS_DEPRECATED

#include <openssl/opensslconf.h>

#include <stdio.h>
#include <string.h>
#include <openssl/engine.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/crypto.h>

#ifdef __aarch64__
#include "arm_arch.h"

size_t OPENSSL_rndr_bytes(unsigned char *buf, size_t len);

static int get_random_bytes(unsigned char *buf, int num)
{
    if (num < 0) {
        return 0;
    }

    return (size_t)num == OPENSSL_rndr_bytes(buf, (size_t)num);
}

static int random_status(void)
{
    return 1;
}

static RAND_METHOD rndr_meth = {
    NULL,                       /* seed */
    get_random_bytes,
    NULL,                       /* cleanup */
    NULL,                       /* add */
    get_random_bytes,
    random_status,
};

static int rndr_init(ENGINE *e)
{
    return 1;
}

static const char *engine_e_rndr_id = "rndr";
static const char *engine_e_rndr_name = "Arm RNDR engine";

static int bind_helper(ENGINE *e)
{
    if (!ENGINE_set_id(e, engine_e_rndr_id) ||
        !ENGINE_set_name(e, engine_e_rndr_name) ||
        !ENGINE_set_flags(e, ENGINE_FLAGS_NO_REGISTER_ALL) ||
        !ENGINE_set_init_function(e, rndr_init) ||
        !ENGINE_set_RAND(e, &rndr_meth))
        return 0;

    return 1;
}

static int bind(ENGINE *e, const char *id)
{
    return bind_helper(e);
}


static ENGINE *ENGINE_rndr(void)
{
    ENGINE *ret = ENGINE_new();
    if (ret == NULL)
        return NULL;
    if (!bind_helper(ret)) {
        ENGINE_free(ret);
        return NULL;
    }
    return ret;
}

void engine_load_rndr_int(void)
{
    if (OPENSSL_armcap_P & ARMV8_RNG) {
        ENGINE *toadd = ENGINE_rndr();
        if (!toadd)
            return;
        ERR_set_mark();
        ENGINE_add(toadd);
        /*
        * If the "add" worked, it gets a structural reference. So either way, we
        * release our just-created reference.
        */
        ENGINE_free(toadd);
        /*
        * If the "add" didn't work, it was probably a conflict because it was
        * already added (eg. someone calling ENGINE_load_blah then calling
        * ENGINE_load_builtin_engines() perhaps).
        */
        ERR_pop_to_mark();
    }
}


IMPLEMENT_DYNAMIC_BIND_FN(bind)
IMPLEMENT_DYNAMIC_CHECK_FN()
#else
void engine_load_rndr_int(void)
{
}
#endif
