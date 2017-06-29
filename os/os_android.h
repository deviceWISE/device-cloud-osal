/**
 * @file
 * @brief
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */
#ifndef OS_ANDROID_H
#define OS_ANDROID_H

/*
 * This file provides the pieces necessary for the OS abstraction layer to
 * function on Android. Most of the functionality is provided by POSIX, but
 * some things are slightly different on Android.
 */

#ifndef OS_H
#error "This file must be included only by os.h"
#endif /* ifndef OS_H */

#define COMMAND_PREFIX           ""
#define SERVICE_SHUTDOWN_CMD     "svc power shutdown"
#define SERVICE_START_CMD        "start %s"
#define SERVICE_STATUS_CMD       "ps | grep %s"
#define SERVICE_STOP_CMD         "stop %s"
#define SERVICE_REBOOT_CMD       "svc power reboot \"rebooting\""
#define OTA_DUP_PATH             "/data/local/tmp"

#define COMMAND_OUTPUT_MAX_LEN   128u

/* 
 * This must come at the end to pickup the definitions and #include values
 * that are specified above.
 */
#include <osal/os/os_posix.h>

#endif /* ifndef OS_ANDROID_H */