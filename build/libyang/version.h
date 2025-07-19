/**
 * @file version.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang version definitions
 *
 * Copyright (c) 2020 - 2024 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_VERSION_H_
#define LY_VERSION_H_

#include "ly_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LY_VERSION_MAJOR 3 /**< libyang major version number */
#define LY_VERSION_MINOR 9 /**< libyang minor version number */
#define LY_VERSION_MICRO 1 /**< libyang micro version number */
#define LY_VERSION "3.9.1" /**< libyang version string */

#define LY_PROJ_VERSION_MAJOR 3 /**< project major version number */
#define LY_PROJ_VERSION_MINOR 12 /**< project minor version number */
#define LY_PROJ_VERSION_MICRO 2 /**< project micro version number */
#define LY_PROJ_VERSION "3.12.2" /**< project version string */

/**
 * @brief Get libyang major SO version.
 *
 * @return LY version.
 */
LIBYANG_API_DECL uint32_t ly_version_so_major(void);

/**
 * @brief Get libyang minor SO version.
 *
 * @return LY version.
 */
LIBYANG_API_DECL uint32_t ly_version_so_minor(void);

/**
 * @brief Get libyang micro SO version.
 *
 * @return LY version.
 */
LIBYANG_API_DECL uint32_t ly_version_so_micro(void);

/**
 * @brief Get libyang string SO version.
 *
 * @return LY version.
 */
LIBYANG_API_DECL const char *ly_version_so_str(void);

/**
 * @brief Get libyang major project version.
 *
 * @return LY version.
 */
LIBYANG_API_DECL uint32_t ly_version_proj_major(void);

/**
 * @brief Get libyang minor project version.
 *
 * @return LY version.
 */
LIBYANG_API_DECL uint32_t ly_version_proj_minor(void);

/**
 * @brief Get libyang micro project version.
 *
 * @return LY version.
 */
LIBYANG_API_DECL uint32_t ly_version_proj_micro(void);

/**
 * @brief Get libyang string project version.
 *
 * @return LY version.
 */
LIBYANG_API_DECL const char *ly_version_proj_str(void);

#ifdef __cplusplus
}
#endif

#endif /* LY_VERSION_H_ */
