/**
 * @file
 * @brief Function definitions for global VxWorks specific settings
 *
 * @copyright Copyright (C) 2018 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied."
 */

#if !defined(VXWORKS_OS_H)
#define VXWORKS_OS_H 1

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * @brief symbol to use for dividing floating point numbers
 */
#define remainder fmod

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* if !defined(VXWORKS_OS_H) */
