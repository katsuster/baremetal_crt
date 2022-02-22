/* SPDX-License-Identifier: Apache-2.0 */

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <bmetal/init.h>
#include <bmetal/arch.h>
#include <bmetal/comm.h>
#include <bmetal/libc_support.h>
#include <bmetal/printk.h>
#include <bmetal/thread.h>
#include <bmetal/drivers/cpu.h>

#if (CONFIG_INTR_STACK_SIZE % CONFIG_STACK_ALIGN) != 0
#  error Invalid interrupt stack size. \
         Please check configs about INTR_STACK_SIZE and STACK_ALIGN.
#endif
#if (CONFIG_MAIN_CORE >= CONFIG_NUM_CORES) != 0
#  error Invalid main core. \
         Please check configs about MAIN_CORE and NUM_CORES.
#endif

extern char __bss_start[], __bss_end[];
extern char __sbss_start[], __sbss_end[];
extern char __data_start[], __data_end[], __data_load[];
extern char __sdata_start[], __sdata_end[], __sdata_load[];

extern __init_func_t __initcall_start[];
extern __init_func_t __initcall_end[];

define_stack(__stack_intr, CONFIG_NUM_CORES * CONFIG_INTR_STACK_SIZE);

int main(int argc, char *argv[], char *envp[]);

/* +1 is for argv[0] */
static char *argv[CONFIG_COMM_MAX_ARGS + 1];
/* always NULL */
static char *envp[2];

static const struct __comm_section __section(BAREMETAL_CRT_COMM_SECTION) __used __comm_s = {
	.magic     = BAREMETAL_CRT_COMM_MAGIC,
};

static void clear_bss(void)
{
	memset(__bss_start, 0, __bss_end - __bss_start);
	memset(__sbss_start, 0, __sbss_end - __sbss_start);
}

static void copy_data(void)
{
#ifdef CONFIG_XIP
	memcpy(__data_start, __data_load, __data_end - __data_start);
	memcpy(__sdata_start, __sdata_load, __sdata_end - __sdata_start);
#endif /* CONFIG_XIP */
}

static int init_proc(void)
{
	struct __process_info *pi = __get_current_process();

	pi->pid = CONFIG_MAIN_PID;

	/* Init stdin/out/err */
	__file_stdio_init(pi);

	/* Init thread info */
	for (int i = 0; i < CONFIG_NUM_CORES; i++) {
		struct __thread_info *ti = __get_raw_thread(i);

		ti->tid = i + CONFIG_MAIN_PID;
	}

	/*
	 * Set thread pointer. Need for main thread only.
	 * CPU drivers set thread pointer for other threads.
	 */
	__set_current_thread(__get_raw_thread(0));

	return 0;
}

static int init_drivers(void)
{
	int cnt = __initcall_end - __initcall_start;
	int res = 0;

	for (int i = 0; i < cnt; i++) {
		int r;

		r = __initcall_start[i]();
		if (r) {
			printk("Initcall failed.\n");
			res = r;
		}
	}

	return res;
}

static int load_argv(const struct __comm_area_header *h, const char *buf_args)
{
	const struct __comm_arg_header *ha;
	uintptr_t buf = (uintptr_t)buf_args;
	int p = 0;

	for (int i = 0; i < h->num_args; i++) {
		ha = (const struct __comm_arg_header *)buf;
		buf += sizeof(struct __comm_arg_header);

		argv[p] = (char *)buf;
		p++;
		buf += ha->size;

		buf = ALIGN_OF(buf, 8);
	}

	return 0;
}

static int init_args(int *argc)
{
	struct __comm_area_header *h_area = (struct __comm_area_header *)__comm_area;

	*argc = 1;

	if (h_area->magic == BAREMETAL_CRT_COMM_MAGIC) {
		size_t sz = ALIGN_OF(sizeof(struct __comm_area_header), 8);

		if (CONFIG_COMM_MAX_ARGS < h_area->num_args) {
			printk("Exceed number of args (req:%" PRId32 ", max:%d)\n",
				h_area->num_args, CONFIG_COMM_MAX_ARGS);
		}

		load_argv(h_area, __comm_area + sz);
		*argc = h_area->num_args + 1;
	}
	if (argv[0] == NULL) {
		printk("Missing kernel name. Use default.\n");
		argv[0] = "main";
	}

	return 0;
}

void __prep_main(void)
{
	clear_bss();
	copy_data();

	__libc_init();
	init_proc();
	init_drivers();

	/* Boot other cores */
	__cpu_wakeup_all();

	int argc;
	init_args(&argc);

	int r = main(argc, argv, envp);

	exit(r);
}

void __prep_sub(void)
{
}
