/**
 * @file
 * @brief
 *
 * @copyright Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */
#ifndef OS_VXWORKS_H
#define OS_VXWORKS_H

/*
 * This file provides the pieces necessary for the OS abstraction layer to
 * function on VxWorks. Most of the functionality is provided by POSIX, but
 * some things are provided by the VxWorks kernel.
 */

#ifndef OS_H
#error "This file must be included only by os.h"
#endif /* ifndef OS_H */

#include <vxWorks.h>

#include <uuid.h>        /* for libuuid functions + uuid_t */
#include <semLib.h>      /* for SEM_ID */
#include <sockLib.h>     /* for socket() */
#include <selectLib.h>   /* for select() */

/* Maximum host name length string */
#define OS_HOST_MAX_LEN 256

/*
 * NOTE: For a POSIX RW semaphore there is no limit on the number of
 *       readers. Use a value of 255 as it should be larger than we
 *       need.
 */
#define VX_RW_SEM_MAX_READERS 255

typedef SEM_ID os_thread_rwlock_t;

/* 
 * This must come at the end to pickup the definitions and #include values
 * that are specified above. 
 */
#include <osal/os/os_posix.h>

#endif /* ifndef OS_VXWORKS_H */

