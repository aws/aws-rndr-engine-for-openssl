/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright 2011-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Modifications are copyright Amazon. Any included source code is
 * copyrighted by the OpenSSL project and its contributors.
 */

/* File is based on openssl/crypto/arm_arch.h and has been modified. */

# define ARMV8_RNG       (1<<8)
extern unsigned int OPENSSL_armcap_P;

size_t OPENSSL_rndr_bytes(unsigned char *buf, size_t len);
size_t OPENSSL_rndrrs_bytes(unsigned char *buf, size_t len);
