/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/arch/cpu.h>
#include <bmetal/binding/intc/riscv/rv_priv.h>
#include <bmetal/syscall.h>

.globl k_trap_start
.type k_trap_start, @function
.globl k_trap_end
.type k_trap_end, @function

.section ".text", "ax"
.align 2
k_trap_start:
	csrrw sp, CSR_XSCRATCH, sp
	beq   sp, x0, k_trap_in_kernel

k_trap_in_user:
	addi  sp, sp, -REGOFF_ALL
	store_caller_regs
	store_callee_regs
	store_special_regs
	store_status_regs t0
	csrr  t0, CSR_XSCRATCH
	OP_ST t0, REGOFF_SP(sp)
	addi  t0, sp, REGOFF_ALL
	OP_ST t0, REGOFF_MSCRATCH(sp)
	csrw  CSR_XSCRATCH, x0

	# Save pointer of user regs
	# This may break caller-saved regs
	mv    a0, sp
	call  k_cpu_set_current_user_regs
	OP_LD a0, REGOFF_A0(sp)
	OP_LD a1, REGOFF_A1(sp)
	OP_LD a2, REGOFF_A2(sp)
	OP_LD a3, REGOFF_A3(sp)
	OP_LD a4, REGOFF_A4(sp)
	OP_LD a5, REGOFF_A5(sp)
	OP_LD a6, REGOFF_A6(sp)
	OP_LD a7, REGOFF_A7(sp)

	j     k_trap_common

k_trap_in_kernel:
	csrrw sp, CSR_XSCRATCH, sp
	addi  sp, sp, -REGOFF_ALL
	store_caller_regs
	store_callee_regs
	store_special_regs
	store_status_regs t0
	addi  t0, sp, REGOFF_ALL
	OP_ST t0, REGOFF_SP(sp)
	j     k_trap_common

k_trap_common:
	# Check syscall
	csrr t0, CSR_XCAUSE
#ifdef CONFIG_RISCV_MACHINE_MODE
	li   t1, RV_CAUSE_EXC_ECALL_M
#else /* CONFIG_RISCV_MACHINE_MODE */
	li   t1, RV_CAUSE_EXC_ECALL_S
#endif /* CONFIG_RISCV_MACHINE_MODE */
	beq  t0, t1, k_trap_syscall

	# Check interrupt (mcause MSB = 1) or exception (MSB = 0)
	not  t1, x0
	srli t1, t1, 1
	not  t1, t1
	and  t0, t0, t1
	bne  x0, t0, k_trap_interrupt
	j    k_trap_exception

k_trap_syscall:
	# Return to next instruction of ecall
	csrr  t0, CSR_XEPC
	addi  t0, t0, 4
	OP_ST t0, REGOFF_MEPC(sp)

	# Check syscall number
	blt   a7, zero, k_trap_syscall_unknown
	li    t0, MAX_SYSCALLS
	bge   a7, t0, k_trap_syscall_unknown

	# Find syscall handler
	la    t0, k_table_syscalls
	slli  t1, a7, REGOFF
	add   t0, t0, t1
	OP_LD t0, (t0)
	beqz  t0, k_trap_syscall_unknown
	jalr  t0

k_trap_syscall_end:
	# Store return value of syscall
	OP_ST a0, REGOFF_A0(sp)

	j     k_trap_end

k_trap_syscall_unknown:
	mv    a6, a5
	mv    a5, a4
	mv    a4, a3
	mv    a3, a2
	mv    a2, a1
	mv    a1, a0
	mv    a0, a7
	call  k_sys_unknown
	j     k_trap_syscall_end

k_trap_interrupt:
	csrr a0, CSR_XCAUSE
	call k_arch_riscv_interrupt
	j    k_trap_end

k_trap_exception:
	csrr a0, CSR_XCAUSE
	call k_arch_riscv_exception
	j    k_trap_end

k_trap_end:
	# Disable interrupt
	li   t0, BIT(RV_STATUS_IE)
	csrc CSR_XSTATUS, t0

	load_status_regs t0
	load_caller_regs
	load_callee_regs
	load_special_regs
	OP_LD sp, REGOFF_SP(sp)

	mret
