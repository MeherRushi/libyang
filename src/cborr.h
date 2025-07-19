/**
 * @file cbor.h
 * @author 
 * @author 
 * @brief Generic CBOR format parser routines.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_CBOR_H_
#define LY_CBOR_H_

#include <stddef.h>
#include <stdint.h>
#include "log.h"
#include "set.h"

enum    LYCBOR_PARSER_STATUS {
    LYCBOR_ERROR = 0,       /* CBOR parser error - value is used as an error return code */
    LYCBOR_OBJECT,          /* CBOR object */
    LYCBOR_OBJECT_NEXT,     /* CBOR object next item */
    LYCBOR_OBJECT_CLOSED,   /* CBOR object closed */
    LYCBOR_ARRAY,           /* CBOR array */
    LYCBOR_ARRAY_NEXT,      /* CBOR array next item */
    LYCBOR_ARRAY_CLOSED,    /* CBOR array closed */
    LYCBOR_OBJECT_NAME,     /* CBOR object name */
    LYCBOR_NUMBER,          /* CBOR number value */
    LYCBOR_STRING,          /* CBOR string value */
    LYCBOR_TRUE,            /* CBOR true value */
    LYCBOR_FALSE,           /* CBOR false value */
    LYCBOR_NULL,            /* CBOR null value */
    LYCBOR_END              /* end of input data */
};

struct lycbor_ctx {
    const struct ly_ctx *ctx;
    struct ly_in *in;       /* input structure */

    struct ly_set status;   /* stack of ::LYJSON_PARSER_STATUS values corresponding to the JSON items being processed */

    const char *value;      /* ::LYJSON_STRING, ::LYJSON_NUMBER, ::LYJSON_OBJECT_NAME */
    size_t value_len;       /* ::LYJSON_STRING, ::LYJSON_NUMBER, ::LYJSON_OBJECT_NAME */
    ly_bool dynamic;        /* ::LYJSON_STRING, ::LYJSON_NUMBER, ::LYJSON_OBJECT_NAME */

    struct {
        enum LYCBOR_PARSER_STATUS status;
        uint32_t status_count;
        const char *value;
        size_t value_len;
        ly_bool dynamic;
        const char *input;
    } backup;

};

/**
 * @brief Get current status of the parser.
 *
 * @param[in] cborctx CBOR parser context to check.
 * @return ::LYCBOR_PARSER_STATUS according to the last parsed token.
 */
enum LYCBOR_PARSER_STATUS lycbor_ctx_status(struct lycbor_ctx *cborctx);

/**
 * @brief Create a new CBOR parser context and start parsing.
 *
 * @param[in] ctx libyang context.
 * @param[in] in CBOR data to parse.
 * @param[out] cborctx New CBOR parser context with status referring the parsed value.
 *
 * @return LY_ERR value.
 */
LY_ERR
lycbor_ctx_new(const struct ly_ctx *ctx, struct ly_in *in, struct lycbor_ctx **cborctx_p)

LY_ERR
lycbor_ctx_next(struct lycbor_ctx *cborctx, enum LYCBOR_PARSER_STATUS *status)


#endif /* LY_CBOR_H_ */
