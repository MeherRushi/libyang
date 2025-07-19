/**
 * @file cbor.c
 * @author 
 * @author 
 * @brief Generic CBOR format parser for libyang
 *
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "in_internal.h"
#include "cborr.h"
#include "ly_common.h"
#include "tree_schema_internal.h"

enum LYCBOR_PARSER_STATUS
lycbor_ctx_status(struct lycbor_ctx *cborctx)
{
    assert(cborctx);

    if (!cborctx->status.count) {
        return LYCBOR_END;
    }

    return (enum LYCBOR_PARSER_STATUS)(uintptr_t)cborctx->status.objs[cborctx->status.count - 1];
}

LY_ERR
lycbor_ctx_new(const struct ly_ctx *ctx, struct ly_in *in, struct lycbor_ctx **cborctx_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lycbor_ctx *cborctx;

    assert(ctx && in && cborctx_p);

    /* Create new CBOR parser context */
    cborctx = calloc(1, sizeof *cborctx);
    LY_CHECK_ERR_RET(!cborctx, LOGMEM(ctx), LY_EMEM);
    cborctx->ctx = ctx;
    cborctx->in = in;

    /* input line logging */
    ly_log_location(NULL, NULL, NULL, in);

    /* Check for empty input */
    if (cborctx->in->current[0] == '\0') {
        LOGVAL(cborctx->ctx, LYVE_SYNTAX, "Empty CBOR file.");
        ret = LY_EVALID;
        goto cleanup;
    }

    /* start parsing */
    LY_CHECK_GOTO(ret = lycbor_ctx_next(cborctx, NULL), cleanup);

cleanup:
    if (ret) {
        free(cborctx);
        cborctx = NULL;
    } else {
        *cborctx_p = cborctx;
    }
    return ret;
}

static LY_ERR
lycbor_next_object_name(struct lycbor_ctx *cborctx)
{
    // TO be written

    return LY_SUCCESS;
}

LY_ERR
lycbor_ctx_next(struct lycbor_ctx *cborctx, enum LYCBOR_PARSER_STATUS *status)
{
    LY_ERR ret = LY_SUCCESS;
    enum LYCBOR_PARSER_STATUS cur;

    assert(cborctx);

    cur = lycbor_ctx_status(cborctx);
    switch (cur) {
    case LYCBOR_OBJECT:
        LY_CHECK_GOTO(ret = lycbor_next_object_name(cborctx), cleanup);
        break;
    case LYCBOR_ARRAY:
        LY_CHECK_GOTO(ret = lycbor_next_value(cborctx, 1), cleanup);
        break;
    case LYCBOR_OBJECT_NEXT:
        LYCBOR_STATUS_POP(cborctx);
        LY_CHECK_GOTO(ret = lycbor_next_object_name(cborctx), cleanup);
        break;
    case LYCBOR_ARRAY_NEXT:
        LYCBOR_STATUS_POP(cborctx);
        LY_CHECK_GOTO(ret = lycbor_next_value(cborctx, 0), cleanup);
        break;
    case LYCBOR_OBJECT_NAME:
        lycbor_ctx_set_value(cborctx, NULL, 0, 0);
        LYCBOR_STATUS_POP(cborctx);
        LY_CHECK_GOTO(ret = lycbor_next_value(cborctx, 0), cleanup);
        break;
    case LYCBOR_OBJECT_CLOSED:
    case LYJSON_ARRAY_CLOSED:
        LYJSON_STATUS_POP(cborctx);
    case LYCBOR_NUMBER:
    case LYCBOR_STRING:
    case LYCBOR_TRUE:
    case LYCBOR_FALSE:
    case LYCBOR_NULL:
        lycbor_ctx_set_value(cborctx, NULL, 0, 0);
        LYCBOR_STATUS_POP(cborctx);
        cur = lycbor_ctx_status(cborctx);
        if (cur == LYCBOR_OBJECT) {
            LY_CHECK_GOTO(ret = lycbor_next_object_item(cborctx), cleanup);
            break;
        } else if (cur == LYCBOR_ARRAY) {
            LY_CHECK_GOTO(ret = lycbor_next_array_item(cborctx), cleanup);
            break;
        }
        assert(cur == LYCBOR_END);
        goto cleanup;
        // goto cleanup;
    case LYCBOR_END:
    LY_CHECK_GOTO(ret = lycbor_next_value(cborctx, 0), cleanup);
        break;
        /* end of input */
        goto cleanup;
    case LYCBOR_ERROR:
        LOGINT(cborctx->ctx);
        ret = LY_EINT;
        goto cleanup;
    }

    // /* skip WS */
    // lycbor_skip_ws(cborctx);
    // if (status) {
    //     *status = cur;
    // }
    // return ret;
cleanup:
    if (!ret && status) {
        *status = lycbor_ctx_status(cborctx);
    }
    return ret;
}