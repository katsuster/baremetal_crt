/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_GLIBC_SYSCALL_H_
#define BAREMETAL_CRT_GLIBC_SYSCALL_H_

#include <bmetal/generated/autoconf.h>

#ifndef CONFIG_USE_GLIBC
#  error Include wrong syscall header.
#endif

/* TODO: to be implemented */
#define MAX_SYSCALLS    0

#endif /* BAREMETAL_CRT_GLIBC_SYSCALL_H_ */
