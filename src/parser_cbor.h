/**
 * @file parser_cbor.h
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

#ifndef LY_PARSER_CBOR_H_
#define LY_PARSER_CBOR_H_

#ifdef ENABLE_CBOR_SUPPORT

#include <stddef.h>
#include <stdint.h>

#include "log.h"
#include "tree_data.h"
#include "parser_internal.h"
#include "set.h"

struct ly_ctx;
struct ly_in;
struct ly_out;
struct lyd_ctx;
struct lyd_node;
struct ly_set;
struct lysc_ext_instance;
struct lysc_node;

/**
 * @brief CBOR format variants for different encoding schemes
 */
enum lyd_cbor_format
{
    LYD_CBOR_NAMED, /**< CBOR with named identifiers (JSON-like) */
    LYD_CBOR_SID    /**< CBOR with Schema Item identifiers (future implementation) */
};

/**
 * @brief Internal context for CBOR YANG data parser.
 *
 * This structure extends the basic lyd_ctx pattern used throughout libyang
 * and provides CBOR-specific parsing state and configuration.
 */
struct lyd_cbor_ctx
{
    const struct lysc_ext_instance *ext; /**< extension instance possibly changing document root context, NULL if none */
    uint32_t parse_opts;                 /**< various @ref dataparseroptions. */
    uint32_t val_opts;                   /**< various @ref datavalidationoptions. */
    uint32_t int_opts;                   /**< internal parser options */
    uint32_t path_len;                   /**< used bytes in the path buffer */
    char path[LYD_PARSER_BUFSIZE];       /**< buffer for the generated path */
    struct ly_set node_when;             /**< set of nodes with "when" conditions */
    struct ly_set node_types;            /**< set of nodes with unresolved types */
    struct ly_set meta_types;            /**< set of metadata with unresolved types */
    struct ly_set ext_node;              /**< set of nodes with extension instances to validate */
    struct ly_set ext_val;               /**< set of nested extension data to validate */
    struct lyd_node *op_node;            /**< if an operation is being parsed, its node */
    struct lyd_node *tree;               /**< parsed data tree to validate, may be NULL */

    /* CBOR-specific members */
    enum lyd_cbor_format format; /**< CBOR format being parsed */
    const struct ly_ctx *ctx;    /**< libyang context */

    /* callbacks */
    lyd_ctx_free_clb free; /**< destructor */
};

/**
 * @brief Parse CBOR data into libyang data tree.
 *
 * This function mirrors the signature and behavior of lyd_parse_json() but handles
 * CBOR input instead. It supports both named identifier and SID formats.
 *
 * @param[in] ctx libyang context.
 * @param[in] ext Optional extension instance to parse data following the schema tree specified in the extension instance
 * @param[in] parent Parent to connect the parsed nodes to, if any.
 * @param[in,out] first_p Pointer to the first top-level parsed node, used only if @p parent is NULL.
 * @param[in] in Input structure to read from.
 * @param[in] parse_opts Options for parser, see @ref dataparseroptions.
 * @param[in] val_opts Options for the validation phase, see @ref datavalidationoptions.
 * @param[in] int_opts Internal data parser options.
 * @param[out] parsed Set to add all the parsed siblings into.
 * @param[out] subtree_sibling Set if ::LYD_PARSE_SUBTREE is used and another subtree is following in @p in.
 * @param[out] lydctx_p Data parser context to finish validation.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_cbor(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
                      struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, uint32_t int_opts,
                      struct ly_set *parsed, ly_bool *subtree_sibling, struct lyd_ctx **lydctx_p);

/**
 * @brief Parse CBOR data from memory into libyang data tree.
 *
 * Convenience function for parsing CBOR data directly from memory buffer.
 *
 * @param[in] ctx libyang context.
 * @param[in] data CBOR data to parse.
 * @param[in] data_len Length of @p data.
 * @param[in] format CBOR format variant (named or SID).
 * @param[in] parse_opts Parse options, see @ref dataparseroptions.
 * @param[in] val_opts Validation options, see @ref datavalidationoptions.
 * @param[out] tree Parsed data tree.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyd_parse_cbor_data(const struct ly_ctx *ctx, const char *data, size_t data_len,
                                            enum lyd_cbor_format format, uint32_t parse_opts, uint32_t val_opts, struct lyd_node **tree);

/**
 * @brief Print libyang data tree as CBOR.
 *
 * @param[in] root Root node of the data tree to print.
 * @param[in] format CBOR format variant to use for output.
 * @param[in] out Output structure to write to.
 * @param[in] options Print options.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyd_print_cbor_data(const struct lyd_node *root, enum lyd_cbor_format format,
                                            struct ly_out *out, uint32_t options);

/* Internal functions (used by parser_data.c and other libyang components) */

/**
 * @brief Detect CBOR format variant from input data.
 *
 * @param[in] in Input structure to analyze.
 * @param[out] format Detected format.
 * @return LY_ERR value.
 */
LY_ERR lydcbor_detect_format(struct ly_in *in, enum lyd_cbor_format *format);

/**
 * @brief Parse a single CBOR value according to schema node.
 *
 * @param[in] lydctx CBOR parser context.
 * @param[in] snode Schema node for the value.
 * @param[in] cbor_item CBOR item to parse.
 * @param[out] node Created data node.
 * @return LY_ERR value.
 */
LY_ERR lydcbor_parse_value(struct lyd_cbor_ctx *lydctx, const struct lysc_node *snode,
                           const void *cbor_item, struct lyd_node **node);

/**
 * @brief Parse CBOR metadata/attributes.
 *
 * @param[in] lydctx CBOR parser context.
 * @param[in] cbor_item CBOR item containing metadata.
 * @param[in,out] node Data node to attach metadata to.
 * @return LY_ERR value.
 */
LY_ERR lydcbor_parse_metadata(struct lyd_cbor_ctx *lydctx, const void *cbor_item, struct lyd_node *node);

/**
 * @brief Create a new CBOR parser context.
 *
 * @param[in] ctx libyang context.
 * @param[in] ext Extension instance providing context for the top level element, NULL if none.
 * @param[in] parse_opts Parse options, see @ref dataparseroptions.
 * @param[in] val_opts Validation options, see @ref datavalidationoptions.
 * @param[in] format CBOR format variant (named or SID).
 * @param[out] lydctx_p Pointer to the created CBOR parser context.
 * @return LY_ERR value.
 */
LY_ERR lydcbor_ctx_new(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext,
                       uint32_t parse_opts, uint32_t val_opts, enum lyd_cbor_format format,
                       struct lyd_cbor_ctx **lydctx_p);

#endif /* ENABLE_CBOR_SUPPORT */

#endif /* LY_PARSER_CBOR_H_ */