/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_MMAN_H_
#define BAREMETAL_CRT_MMAN_H_

#include <bmetal/bmetal.h>
#include <bmetal/arch.h>

#define PROT_NONE        0x00
#define PROT_READ        0x04
#define PROT_WRITE       0x02
#define PROT_EXEC        0x01

#define MAP_FILE         0x0001
#define MAP_ANON         0x0002
#define MAP_TYPE         0x000f
#define MAP_ANONYMOUS    MAP_ANON
#define MAP_COPY         0x0020
#define MAP_SHARED       0x0010
#define MAP_PRIVATE      0x0000
#define MAP_FIXED        0x0100
#define MAP_NOEXTEND     0x0200
#define MAP_HASSEMPHORE  0x0400
#define MAP_INHERIT      0x0800

#endif /* BAREMETAL_CRT_MMAN_H_ */
