# SPDX-License-Identifier: Apache-2.0

if(CONFIG_64BIT)
  set(RV_ARCH_BASE rv64)
  set(RV_MABI_BASE lp64)
  set(RV_ARCH_EXT)
  set(RV_MABI_EXT)
  prj_compile_options(-mcmodel=medany)
else()
  set(RV_ARCH_BASE rv32)
  set(RV_MABI_BASE ilp32)
  set(RV_ARCH_EXT)
  set(RV_MABI_EXT)
endif()

if(CONFIG_RISCV_EXT_I_OLD)
  set(RV_ARCH_BASE ${RV_ARCH_BASE}i)
endif()

# From binutils 2.38
if(CONFIG_RISCV_EXT_I)
  set(RV_ARCH_BASE ${RV_ARCH_BASE}i)
  set(RV_ARCH_EXT ${RV_ARCH_EXT}_zicsr_zifencei)
endif()

if(CONFIG_RISCV_EXT_M)
  set(RV_ARCH_BASE ${RV_ARCH_BASE}m)
endif()
if(CONFIG_RISCV_EXT_A)
  set(RV_ARCH_BASE ${RV_ARCH_BASE}a)
endif()
if(CONFIG_RISCV_EXT_F)
  set(RV_ARCH_BASE ${RV_ARCH_BASE}f)
  set(RV_MABI_EXT f)
endif()
if(CONFIG_RISCV_EXT_D)
  set(RV_ARCH_BASE ${RV_ARCH_BASE}d)
  set(RV_MABI_EXT d)
endif()
if(CONFIG_RISCV_EXT_C)
  set(RV_ARCH_BASE ${RV_ARCH_BASE}c)
endif()
if(CONFIG_RISCV_EXT_V)
  set(RV_ARCH_BASE ${RV_ARCH_BASE}v)
endif()

if(CONFIG_RISCV_EXT_ZBA)
  set(RV_ARCH_EXT ${RV_ARCH_EXT}_zba)
endif()
if(CONFIG_RISCV_EXT_ZBB)
  set(RV_ARCH_EXT ${RV_ARCH_EXT}_zbb)
endif()
if(CONFIG_RISCV_EXT_ZBC)
  set(RV_ARCH_EXT ${RV_ARCH_EXT}_zbc)
endif()
if(CONFIG_RISCV_EXT_ZBS)
  set(RV_ARCH_EXT ${RV_ARCH_EXT}_zbs)
endif()

if(CONFIG_RISCV_EXT_ZICBOM)
  set(RV_ARCH_EXT ${RV_ARCH_EXT}_zicbom)
endif()
if(CONFIG_RISCV_EXT_ZICBOZ)
  set(RV_ARCH_EXT ${RV_ARCH_EXT}_zicboz)
endif()
if(CONFIG_RISCV_EXT_ZICBOP)
  set(RV_ARCH_EXT ${RV_ARCH_EXT}_zicbop)
endif()

set(RV_ARCH ${RV_ARCH_BASE}${RV_ARCH_EXT})
set(RV_MABI ${RV_MABI_BASE}${RV_MABI_EXT})

prj_compile_options(-march=${RV_ARCH})
prj_compile_options(-mabi=${RV_MABI})

message("${SPACER} march is '${RV_ARCH}'")
message("${SPACER} mabi  is '${RV_MABI}'")
