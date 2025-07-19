/**
 * @file parser_cbor.c
 * @author
 * @brief CBOR data parser for libyang
 *
 * Copyright (c) 2020 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#ifdef ENABLE_CBOR_SUPPORT

#include "parser_cbor.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <cbor.h>
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "in_internal.h"
#include "log.h"
#include "ly_common.h"
#include "parser_data.h"
#include "parser_internal.h"
#include "plugins_exts.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "validation.h"

#include "cborr.h"

/**
 * @brief Free the CBOR parser context.
 *
 * @param[in] lydctx Data parser context to free.
 */
static void
lyd_cbor_ctx_free(struct lyd_ctx *lydctx)
{
    struct lyd_cbor_ctx *ctx = (struct lyd_cbor_ctx *)lydctx;
    
    if (!ctx) {
        return;
    }
    
    ly_set_erase(&ctx->node_when, NULL);
    ly_set_erase(&ctx->node_types, NULL);
    ly_set_erase(&ctx->meta_types, NULL);
    ly_set_erase(&ctx->ext_node, NULL);
    ly_set_erase(&ctx->ext_val, NULL);
    free(ctx);
}

/**
 * @brief Create new CBOR parser context.
 *
 * This follows the same pattern as lydxml_ctx_new() and lydjson_ctx_new().
 */
LY_ERR
lydcbor_ctx_new(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext,
                uint32_t parse_opts, uint32_t val_opts, enum lyd_cbor_format format,
                struct lyd_cbor_ctx **lydctx_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_cbor_ctx *lydctx;

    assert(ctx && lydctx_p);

    /* Initialize context */
    lydctx = calloc(1, sizeof *lydctx);
    LY_CHECK_ERR_RET(!lydctx, LOGMEM(ctx), LY_EMEM);

    lydctx->ctx = ctx;
    lydctx->ext = ext;
    lydctx->parse_opts = parse_opts;
    lydctx->val_opts = val_opts;
    lydctx->format = format;
    lydctx->free = lyd_cbor_ctx_free;

    /* Initialize sets */
    LY_CHECK_ERR_GOTO(ly_set_new(&lydctx->node_when), ret = LY_EMEM, cleanup);
    LY_CHECK_ERR_GOTO(ly_set_new(&lydctx->node_types), ret = LY_EMEM, cleanup);
    LY_CHECK_ERR_GOTO(ly_set_new(&lydctx->meta_types), ret = LY_EMEM, cleanup);
    LY_CHECK_ERR_GOTO(ly_set_new(&lydctx->ext_node), ret = LY_EMEM, cleanup);
    LY_CHECK_ERR_GOTO(ly_set_new(&lydctx->ext_val), ret = LY_EMEM, cleanup);

    *lydctx_p = lydctx;
    return ret;

cleanup:
    if (lydctx) {
        lyd_cbor_ctx_free((struct lyd_ctx *)lydctx);
    }
    return ret;
}

/**
 * @brief Convert a CBOR item to a string representation.
 *
 * This function handles the low-level CBOR to string conversion,
 * similar to how JSON parser converts JSON values to strings.
 *
 * @param[in] item CBOR item to convert.
 * @param[out] str_val String value (allocated, caller must free).
 * @param[out] str_len String length.
 * @return LY_ERR value.
 */
static LY_ERR
lydcbor_item_to_string(const cbor_item_t *item, char **str_val, size_t *str_len)
{
    LY_ERR ret = LY_SUCCESS;

    assert(item && str_val && str_len);
    *str_val = NULL;
    *str_len = 0;

    switch (cbor_typeof(item))
    {
    case CBOR_TYPE_UINT:
    {
        uint64_t val = cbor_get_int(item);
        int len = snprintf(NULL, 0, "%" PRIu64, val);
        if (len < 0) {
            return LY_ESYS;
        }
        *str_val = malloc(len + 1);
        LY_CHECK_ERR_RET(!*str_val, LOGMEM(NULL), LY_EMEM);
        sprintf(*str_val, "%" PRIu64, val);
        *str_len = len;
        break;
    }
    case CBOR_TYPE_NEGINT:
    {
        int64_t val = -1 - (int64_t)cbor_get_int(item);
        int len = snprintf(NULL, 0, "%" PRId64, val);
        if (len < 0) {
            return LY_ESYS;
        }
        *str_val = malloc(len + 1);
        LY_CHECK_ERR_RET(!*str_val, LOGMEM(NULL), LY_EMEM);
        sprintf(*str_val, "%" PRId64, val);
        *str_len = len;
        break;
    }
    case CBOR_TYPE_BYTESTRING:
        *str_len = cbor_bytestring_length(item);
        *str_val = malloc(*str_len + 1);
        LY_CHECK_ERR_RET(!*str_val, LOGMEM(NULL), LY_EMEM);
        memcpy(*str_val, cbor_bytestring_handle(item), *str_len);
        (*str_val)[*str_len] = '\0';
        break;
    case CBOR_TYPE_STRING:
        *str_len = cbor_string_length(item);
        *str_val = malloc(*str_len + 1);
        LY_CHECK_ERR_RET(!*str_val, LOGMEM(NULL), LY_EMEM);
        memcpy(*str_val, cbor_string_handle(item), *str_len);
        (*str_val)[*str_len] = '\0';
        break;
    case CBOR_TYPE_FLOAT_CTRL:
        if (cbor_float_ctrl_is_ctrl(item))
        {
            switch (cbor_ctrl_value(item))
            {
            case CBOR_CTRL_TRUE:
                *str_val = strdup("true");
                *str_len = 4;
                break;
            case CBOR_CTRL_FALSE:
                *str_val = strdup("false");
                *str_len = 5;
                break;
            case CBOR_CTRL_NULL:
                *str_val = strdup("");
                *str_len = 0;
                break;
            default:
                LOGVAL(NULL, LYVE_SYNTAX, "Unsupported CBOR control value");
                ret = LY_EVALID;
                break;
            }
            LY_CHECK_ERR_RET(!*str_val, LOGMEM(NULL), LY_EMEM);
        }
        else
        {
            /* Float value */
            double val = cbor_float_get_float(item);
            int len = snprintf(NULL, 0, "%g", val);
            if (len < 0) {
                return LY_ESYS;
            }
            *str_val = malloc(len + 1);
            LY_CHECK_ERR_RET(!*str_val, LOGMEM(NULL), LY_EMEM);
            sprintf(*str_val, "%g", val);
            *str_len = len;
        }
        break;
    default:
        LOGVAL(NULL, LYVE_SYNTAX, "Unsupported CBOR data type %d", cbor_typeof(item));
        ret = LY_EVALID;
        break;
    }

    return ret;
}

/**
 * @brief Get string key from CBOR map item.
 *
 * For named identifier format, keys should be strings.
 * For SID format, keys would be integers (future implementation).
 */
static LY_ERR
lydcbor_get_key_string(struct lyd_cbor_ctx *lydctx, const cbor_item_t *key_item,
                       char **key_str, size_t *key_len)
{
    LY_ERR ret = LY_SUCCESS;

    assert(lydctx && key_item && key_str && key_len);

    switch (lydctx->format)
    {
    case LYD_CBOR_NAMED:
        /* Keys must be strings for named format */
        if (!cbor_isa_string(key_item))
        {
            LOGVAL(lydctx->ctx, LYVE_SYNTAX, "CBOR map key must be string for named identifier format");
            return LY_EVALID;
        }
        ret = lydcbor_item_to_string(key_item, key_str, key_len);
        break;
    case LYD_CBOR_SID:
        /* Future: Handle SID integer keys */
        LOGVAL(lydctx->ctx, LYVE_SYNTAX, "CBOR SID format not yet implemented");
        ret = LY_ENOT;
        break;
    default:
        LOGVAL(lydctx->ctx, LYVE_SYNTAX, "Unknown CBOR format");
        ret = LY_EINVAL;
        break;
    }

    return ret;
}

LY_ERR
lydcbor_parse_value(struct lyd_cbor_ctx *lydctx, const struct lysc_node *snode,
                    const void *cbor_item, struct lyd_node **node)
{
    LY_ERR ret = LY_SUCCESS;
    const cbor_item_t *item = (const cbor_item_t *)cbor_item;
    char *str_val = NULL;
    size_t str_len = 0;

    assert(lydctx && snode && item && node);

    /* Convert CBOR value to string */
    LY_CHECK_GOTO(ret = lydcbor_item_to_string(item, &str_val, &str_len), cleanup);

    /* Create data node based on schema node type */
    switch (snode->nodetype)
    {
    case LYS_LEAF:
    case LYS_LEAFLIST:
        ret = lyd_create_term(snode, str_val, str_len, 0, 0, NULL, LY_VALUE_JSON, NULL, LYD_HINT_DATA, NULL, node);
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        /* For anydata/anyxml, we store the CBOR directly */
        ret = lyd_create_any(snode, cbor_item, LYD_ANYDATA_CBOR, 0, node);
        break;
    default:
        LOGVAL(lydctx->ctx, LYVE_SYNTAX, "Invalid schema node type for CBOR value");
        ret = LY_EVALID;
        break;
    }

cleanup:
    free(str_val);
    return ret;
}

/**
 * @brief Parse a CBOR container (map or array).
 *
 * This is the core function that handles CBOR maps and arrays,
 * similar to lydjson_parse_container() in the JSON parser.
 */
static LY_ERR
lydcbor_parse_container(struct lyd_cbor_ctx *lydctx, struct lyd_node *parent,
                        struct lyd_node **first_p, struct ly_set *parsed, const cbor_item_t *cbor_container)
{
    LY_ERR ret = LY_SUCCESS;
    const struct lysc_node *snode = NULL;
    struct lyd_node *node = NULL;
    char *key_str = NULL;
    size_t key_len = 0;

    assert(lydctx && parsed && cbor_container);

    if (cbor_isa_map(cbor_container))
    {
        /* Handle CBOR map (object) */
        size_t map_size = cbor_map_size(cbor_container);
        struct cbor_pair *pairs = cbor_map_handle(cbor_container);

        for (size_t i = 0; i < map_size; ++i)
        {
            const cbor_item_t *key_item = pairs[i].key;
            const cbor_item_t *value_item = pairs[i].value;

            /* Get key string */
            LY_CHECK_GOTO(ret = lydcbor_get_key_string(lydctx, key_item, &key_str, &key_len), cleanup);

            /* Find schema node */
            if (parent)
            {
                snode = lys_find_child(parent->schema, NULL, key_str, key_len, 0, 0);
            }
            else if (lydctx->ext)
            {
                snode = lysc_ext_find_node(lydctx->ext, NULL, key_str, key_len, 0, 0);
            }
            else
            {
                /* Find top-level node */
                const struct lys_module *mod;
                uint32_t idx;
                for (idx = 0; idx < lydctx->ctx->list.count; ++idx)
                {
                    mod = lydctx->ctx->list.objs[idx];
                    if (mod->compiled && mod->compiled->data)
                    {
                        snode = lys_find_child(NULL, mod->compiled->data, key_str, key_len, 0, 0);
                        if (snode)
                        {
                            break;
                        }
                    }
                }
            }

            if (!snode)
            {
                if (lydctx->parse_opts & LYD_PARSE_STRICT)
                {
                    LOGVAL(lydctx->ctx, LYVE_REFERENCE, "Unknown element \"%.*s\".", (int)key_len, key_str);
                    ret = LY_EVALID;
                    goto cleanup;
                }
                else
                {
                    LOGWRN(lydctx->ctx, "Unknown element \"%.*s\".", (int)key_len, key_str);
                    free(key_str);
                    key_str = NULL;
                    continue;
                }
            }

            /* Parse the value based on schema node type */
            if (snode->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA | LYS_ANYXML))
            {
                /* Terminal node */
                LY_CHECK_GOTO(ret = lydcbor_parse_value(lydctx, snode, value_item, &node), cleanup);
            }
            else if (snode->nodetype & LYD_NODE_INNER)
            {
                /* Inner node */
                ret = lyd_create_inner(snode, &node);
                LY_CHECK_GOTO(ret, cleanup);

                /* Recursively parse children */
                if (cbor_isa_map(value_item) || cbor_isa_array(value_item))
                {
                    ret = lydcbor_parse_container(lydctx, node, lyd_node_child_p(node), parsed, value_item);
                    LY_CHECK_GOTO(ret, cleanup);
                }
            }
            else
            {
                LOGVAL(lydctx->ctx, LYVE_SYNTAX, "Invalid schema node type");
                ret = LY_EVALID;
                goto cleanup;
            }

            /* Insert the node */
            if (parent)
            {
                lyd_insert_node(parent, NULL, node, 0);
            }
            else
            {
                lyd_insert_node(NULL, first_p, node, 0);
            }

            /* Add to parsed set */
            LY_CHECK_GOTO(ret = ly_set_add(parsed, node, 1, NULL), cleanup);

            free(key_str);
            key_str = NULL;
            node = NULL;
        }
    }
    else if (cbor_isa_array(cbor_container))
    {
        /* Handle CBOR array - typically for leaf-lists or lists without keys */
        size_t array_size = cbor_array_size(cbor_container);
        cbor_item_t **array_handle = cbor_array_handle(cbor_container);

        for (size_t i = 0; i < array_size; ++i)
        {
            const cbor_item_t *item = array_handle[i];

            /* For arrays, we need to determine the schema node from parent context */
            /* This is a simplified approach - more complex logic needed for real implementation */
            if (parent && parent->schema)
            {
                /* Use parent's schema to determine child type */
                snode = lysc_node_child(parent->schema);
                if (snode && (snode->nodetype & (LYS_LEAFLIST | LYS_LIST)))
                {
                    LY_CHECK_GOTO(ret = lydcbor_parse_value(lydctx, snode, item, &node), cleanup);
                    lyd_insert_node(parent, NULL, node, 0);
                    LY_CHECK_GOTO(ret = ly_set_add(parsed, node, 1, NULL), cleanup);
                    node = NULL;
                }
            }
        }
    }

cleanup:
    free(key_str);
    if (ret && node)
    {
        lyd_free_tree(node);
    }
    return ret;
}

static LY_ERR
lydcbor_subtree_r(struct lyd_cbor_ctx *lydctx, struct lyd_node *parent, struct lyd_node **first_p, struct ly_set *parsed)
{
    // called as lydcbor_subtree_r(lydctx, parent, first_p, parsed);
    LY_ERR r, rc = LY_SUCCESS;
    enum LYCBOR_PARSER_STATUS status = lycbor_ctx_status(lydctx->cborctx);      // will fail since lydctx->cborctxis NULL
    //
    const char *name, *prefix = NULL, *expected = NULL;
    size_t name_len, prefix_len = 0;
    ly_bool is_meta = 0, parse_subtree;
    const struct lysc_node *snode = NULL;
    struct lysc_ext_instance *ext = NULL;
    struct lyd_node *node = NULL, *attr_node = NULL;
    const struct ly_ctx *ctx = lydctx->cborctx->ctx;    //ly_ctx is the context of YANG schema
                                                        //???why is a ly_ctx associated with every lycbor_ctx in lyd_cbor_ctx??
    char *value = NULL;

    assert(parent || first_p);
    assert((status == LYCBOR_OBJECT) || (status == LYCBOR_OBJECT_NEXT));
    
    printf("exiting lydcbor_subtree_r\n");

    return LY_SUCCESS;


}

LY_ERR
lydcbor_detect_format(struct ly_in *in, enum lyd_cbor_format *format)
{
    /* Simple heuristic: try to parse as CBOR and examine structure */
    /* For now, default to named format */
    (void)in;
    *format = LYD_CBOR_NAMED;
    return LY_SUCCESS;
}

LY_ERR
lydcbor_parse_metadata(struct lyd_cbor_ctx *lydctx, const void *cbor_item, struct lyd_node *node)
{
    /* Future implementation for CBOR metadata parsing */
    (void)lydctx;
    (void)cbor_item;
    (void)node;
    return LY_SUCCESS;
}

LY_ERR
lyd_parse_cbor(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
               struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, uint32_t int_opts,
               struct ly_set *parsed, ly_bool *subtree_sibling, struct lyd_ctx **lydctx_p)
{
    printf("Entered lyd_parse_cbor\n");

    LY_ERR ret = LY_SUCCESS;
    LY_ERR r, rc = LY_SUCCESS;
    struct lyd_cbor_ctx *lydctx = NULL;
    enum LYCBOR_PARSER_STATUS status;

    cbor_item_t *cbor_data = NULL;
    size_t cbor_size = strlen(in->current);
    printf("Parsing CBOR data of size: %zu\n", cbor_size);

    const char *cbor_input = NULL;
    struct cbor_load_result result = {0};
    enum lyd_cbor_format format;

    assert(ctx && in && lydctx_p);
    assert(!(parse_opts & ~LYD_PARSE_OPTS_MASK));
    assert(!(val_opts & ~LYD_VALIDATE_OPTS_MASK));
    
    (void)int_opts; /* Currently unused */
    (void)subtree_sibling; /* Currently unused */

    /* Detect CBOR format - Named or SID */ 
    LY_CHECK_GOTO(ret = lydcbor_detect_format(in, &format), cleanup);

    /* Initialize context --- needs to be changed as lydctx->cborctx has to be initialised */
    // lydctx did not have member cborctx, was added later
    LY_CHECK_GOTO(ret = lydcbor_ctx_new(ctx, ext, parse_opts, val_opts, format, &lydctx), cleanup);

    LY_CHECK_GOTO(rc = lyd_parse_cbor_init(ctx, in, parse_opts, val_opts, &lydctx));

    // Parse using cbor parser
    //read subtrees
    do {
        printf("Inside do while loop\n");
        r = lydcbor_subtree_r(lydctx, parent, first_p, parsed);
        // LY_DPARSER_ERR_GOTO(r, ret = r, lydctx, cleanup); // Uncomment when lydcbor_subtree_r is implemented

        status = lycbor_ctx_status(lydctx->cborctx);
        if (!(int_opts & LYD_INTOPT_WITH_SIBLINGS)) {   //what is int_opts? what exactly is this checking?
            break;
        }

    } while (status == LYCBOR_OBJECT_NEXT);


    /* Parse CBOR using libcbor */
    cbor_data = cbor_load((const unsigned char *)in->current, cbor_size, &result);
    if (!cbor_data || result.error.code != CBOR_ERR_NONE)
    {
        LOGVAL(ctx, LYVE_SYNTAX, "Failed to parse CBOR data: %s",
               result.error.code == CBOR_ERR_NONE ? "unknown error" : "parsing error");
        ret = LY_EVALID;
        goto cleanup;
    }

    /* Parse the CBOR structure */
    ret = lydcbor_parse_container(lydctx, parent, first_p, parsed, cbor_data);
    LY_CHECK_GOTO(ret, cleanup);

cleanup:
    if (cbor_data)
    {
        cbor_decref(&cbor_data);
    }

    if (ret)
    {
        if (lydctx) {
            lyd_cbor_ctx_free((struct lyd_ctx *)lydctx);
            lydctx = NULL;
        }
    }

    *lydctx_p = (struct lyd_ctx *)lydctx;
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_parse_cbor_data(const struct ly_ctx *ctx, const char *data, size_t data_len, enum lyd_cbor_format format,
                    uint32_t parse_opts, uint32_t val_opts, struct lyd_node **tree)
{
    LY_ERR ret;
    struct ly_in *in;
    struct lyd_ctx *lydctx = NULL;
    struct ly_set parsed = {0};

    LY_CHECK_ARG_RET(ctx, ctx, tree, LY_EINVAL);
    
    (void)format; /* Currently unused */

    *tree = NULL;

    if (!data || !data_len)
    {
        return LY_SUCCESS;
    }

    /* Initialize parsed set */
    ly_set_new(&parsed);

    /* Create input structure */
    LY_CHECK_RET(ly_in_new_memory(data, &in));

    /* Parse CBOR data */
    ret = lyd_parse_cbor(ctx, NULL, NULL, tree, in, parse_opts, val_opts, 0, &parsed, NULL, &lydctx);

    /* Cleanup */
    ly_in_free(in, 0);
    if (lydctx)
    {
        lydctx->free(lydctx);
    }
    ly_set_erase(&parsed, NULL);

    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_print_cbor_data(const struct lyd_node *root, enum lyd_cbor_format format, struct ly_out *out, uint32_t options)
{
    (void)root;
    (void)format;
    (void)out;
    (void)options;

    LOGERR(NULL, LY_ENOT, "CBOR printing not yet implemented");
    return LY_ENOT;
}

static LY_ERR
lyd_parse_cbor_init(const struct ly_ctx *ctx, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts,
                    struct lyd_cbor_ctx **lydctx_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_cbor_ctx *lydctx;

    assert(lydctx_p);

    /* Initialize context */
    lydctx = calloc(1, sizeof *lydctx);
    LY_CHECK_ERR_RET(!lydctx, LOGMEM(ctx), LY_EMEM);
    lydctx->parse_opts = parse_opts;
    lydctx->val_opts = val_opts;
    lydctx->free = lyd_cbor_ctx_free;

    /* Create CBOR parser context */
    LY_CHECK_ERR_RET(ret = lycbor_ctx_new(ctx, in, &lydctx->cborctx), free(lydctx), ret);
    status - lycbor_ctx_status(lydctx->cborctx);

    /* Check for top-level object */
    if (lycbor_ctx_status(lydctx->cborctx) != LYCBOR_OBJECT) {
        LOGVAL(ctx, LYVE_SYNTAX_JSON, "Expected top-level CBOR object.");
        free(lydctx);
        *lydctx_p = NULL;
        return LY_EVALID;
    }

    *lydctx_p = lydctx;
    return LY_SUCCESS;
}


#endif /* ENABLE_CBOR_SUPPORT */