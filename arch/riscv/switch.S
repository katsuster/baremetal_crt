/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/arch/cpu.h>
#include <bmetal/syscall.h>

.globl k_arch_riscv_copy_current_context
.type k_arch_riscv_copy_current_context, @function
.globl k_arch_riscv_context_switch
.type k_arch_riscv_context_switch, @function

.section ".text", "ax"
.align 2
k_arch_riscv_copy_current_context:
	OP_ST sp, REGOFF_SP(a0)

	OP_CALLER_REGS(OP_ST, a0)
	OP_CALLEE_REGS(OP_ST, a0)
	OP_SPECIAL_REGS(OP_ST, a0)

	csrr  t0, CSR_XEPC
	OP_ST t0, REGOFF_MEPC(a0)
	csrr  t0, CSR_XSTATUS
	OP_ST t0, REGOFF_MSTATUS(a0)
	csrr  t0, CSR_XSCRATCH
	OP_ST t0, REGOFF_MSCRATCH(a0)
	OP_LD t0, REGOFF_T0(a0)

	ret
k_arch_riscv_context_switch:
	li    a7, SYS_context_switch
	ecall
	ret
