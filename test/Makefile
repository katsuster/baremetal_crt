
CC  = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
ADD_AUXDATA ?= $(USE_SYSROOT)/bin/add_auxdata

# Config for bmetal.mk
USE_GLIBC   ?= n
USE_MUSL    ?= n
USE_NEWLIB  ?= n
USE_SYSROOT ?= ./sysroot

# Test with additional libraries (please read doc/test.adoc)
ifeq ($(USE_GLIBC),y)
WITH_POSIX ?= y
WITH_MT    ?= y
endif
ifeq ($(USE_MUSL),y)
WITH_POSIX ?= y
WITH_MT    ?= y
endif
ifeq ($(USE_NEWLIB),y)
WITH_POSIX ?= n
WITH_MT    ?= n
endif

WITH_CXX ?= n
WITH_EIGEN ?= n
WITH_OPENBLAS ?= n
WITH_OPENSSL ?= n

# 32bit newlib
#APP_CFLAGS_RV  = -march=rv32imafc -mabi=ilp32f -mcmodel=medany
#APP_LDFLAGS_RV = -march=rv32imafc -mabi=ilp32f -mcmodel=medany

# 32bit libc
#APP_CFLAGS_RV  = -march=rv32gc -mabi=ilp32d -mcmodel=medany
#APP_LDFLAGS_RV = -march=rv32gc -mabi=ilp32d -mcmodel=medany
#APP_CFLAGS_RV  = -march=rv32imac -mabi=ilp32 -mcmodel=medany
#APP_LDFLAGS_RV = -march=rv32imac -mabi=ilp32 -mcmodel=medany

# 64bit libc
APP_CFLAGS_RV  = -march=rv64gc -mabi=lp64d -mcmodel=medany
APP_LDFLAGS_RV = -march=rv64gc -mabi=lp64d -mcmodel=medany
#APP_CFLAGS_RV  = -march=rv64imac -mabi=lp64 -mcmodel=medany
#APP_LDFLAGS_RV = -march=rv64imac -mabi=lp64 -mcmodel=medany

APP_CPPFLAGS = -Wall
APP_CFLAGS   = -O2 -gdwarf-4 $(APP_CFLAGS_RV)
APP_CXXFLAGS = $(APP_CFLAGS)
APP_LDFLAGS  = $(APP_LDFLAGS_RV)

CPPFLAGS = $(APP_CPPFLAGS) $(EXT_CPPFLAGS)
CFLAGS   = $(APP_CFLAGS)   $(EXT_CFLAGS)
CXXFLAGS = $(APP_CXXFLAGS) $(EXT_CXXFLAGS)
LDFLAGS  = $(APP_LDFLAGS)  $(EXT_LDFLAGS)

BINS = \
	args clock \
	crt_string \
	ctype \
	daxpy \
	echo flush \
	hello \
	malloc \
	mathf \
	md5 memcpy \
	saxpy sgemm \
	vecadd vecaddf \
	donut/donut \
	coremark/coremark \
	dhrystone/dhrystone

EXT_LDADD += -lm

ifeq ($(WITH_MT),y)
BINS += \
	mt_clock \
	mt_vecadd mt_vecaddf \
	pthread pthread_barrier pthread_cond \
	reboot \
	donut/mt_donut
endif

ifeq ($(WITH_CXX),y)
BINS += \
	hello_cpp
endif

ifeq ($(WITH_POSIX),y)
BINS += \
	gpio sleep
endif

ifeq ($(WITH_EIGEN),y)
BINS += \
	hello_eigen
EXT_CPPFLAGS += \
	-I $(USE_SYSROOT)/include/eigen3
endif
ifeq ($(WITH_OPENBLAS),y)
EXT_CFLAGS += \
	-DUSE_CBLAS
EXT_CXXFLAGS += \
	-DUSE_CBLAS
EXT_LDADD += \
	-lopenblas
endif
ifeq ($(WITH_OPENSSL),y)
BINS += \
	hello_openssl
EXT_LDADD += \
	-lcrypto
endif

ARGS_OBJS = args.o
CLOCK_OBJS = clock.o
CRT_STRING_OBJS = crt_string.o
CTYPE_OBJS = ctype.o
DAXPY_OBJS = daxpy.o
ECHO_OBJS = echo.o
FLUSH_OBJS = flush.o
GPIO_OBJS = gpio.o
HELLO_OBJS = hello.o
HELLO_CPP_OBJS = hello_cpp.o
HELLO_EIGEN_OBJS = hello_eigen.o
HELLO_OPENSSL_OBJS = hello_openssl.o
MALLOC_OBJS = malloc.o
MATHF_OBJS = mathf.o
MD5_OBJS = md5.o
MEMCPY_OBJS = memcpy.o
MT_CLOCK_OBJS = mt_clock.o
MT_VECADD_OBJS = mt_vecadd.o cmn_vecaddf.o
MT_VECADDF_OBJS = mt_vecaddf.o cmn_vecaddf.o
PTHREAD_OBJS = pthread.o
PTHREAD_BARRIER_OBJS = pthread_barrier.o
PTHREAD_COND_OBJS = pthread_cond.o
REBOOT_OBJS = reboot.o
SAXPY_OBJS = saxpy.o
SGEMM_OBJS = sgemm.o
SLEEP_OBJS = sleep.o
VECADD_OBJS = vecadd.o cmn_vecaddf.o
VECADDF_OBJS = vecaddf.o cmn_vecaddf.o
DONUT_OBJS = donut/donut.o
MT_DONUT_OBJS = donut/mt_donut.o
OBJS = \
	$(ARGS_OBJS) $(CLOCK_OBJS) \
	$(CRT_STRING_OBJS) \
	$(CTYPE_OBJS) \
	$(DAXPY_OBJS) \
	$(ECHO_OBJS) $(FLUSH_OBJS) $(GPIO_OBJS) \
	$(HELLO_OBJS) $(HELLO_CPP_OBJS) $(HELLO_EIGEN_OBJS) \
	$(HELLO_OPENSSL_OBJS) \
	$(MALLOC_OBJS) \
	$(MATHF_OBJS) \
	$(MD5_OBJS) $(MEMCPY_OBJS) \
	$(MT_CLOCK_OBJS) \
	$(MT_VECADD_OBJS) $(MT_VECADDF_OBJS) \
	$(PTHREAD_OBJS) $(PTHREAD_BARRIER_OBJS) $(PTHREAD_COND_OBJS) \
	$(REBOOT_OBJS) \
	$(SAXPY_OBJS) $(SGEMM_OBJS) $(SLEEP_OBJS) \
	$(VECADD_OBJS) $(VECADDF_OBJS) \
	$(DONUT_OBJS) $(MT_DONUT_OBJS)

all: all_auxdata

# Add ELF and program headers into special section 'auxdata' of binary
all_auxdata: $(BINS)
	$(ADD_AUXDATA) $(BINS)

args: $(ARGS_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

clock: $(CLOCK_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

crt_string: $(CRT_STRING_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

ctype: $(CTYPE_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

daxpy: $(DAXPY_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

echo: $(ECHO_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

flush: $(FLUSH_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

gpio: $(GPIO_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

hello: $(HELLO_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

hello_cpp: $(HELLO_CPP_OBJS)
	$(CXX) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

hello_eigen: $(HELLO_EIGEN_OBJS)
	$(CXX) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

hello_openssl: $(HELLO_OPENSSL_OBJS)
	$(CXX) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

malloc: $(MALLOC_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

mathf: $(MATHF_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

md5: $(MD5_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

memcpy: $(MEMCPY_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

mt_clock: $(MT_CLOCK_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

mt_vecadd: $(MT_VECADD_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

mt_vecaddf: $(MT_VECADDF_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

pthread: $(PTHREAD_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

pthread_barrier: $(PTHREAD_BARRIER_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

pthread_cond: $(PTHREAD_COND_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

reboot: $(REBOOT_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

saxpy: $(SAXPY_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

sgemm: $(SGEMM_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

sleep: $(SLEEP_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

vecadd: $(VECADD_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

vecaddf: $(VECADDF_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

donut/donut: $(DONUT_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

donut/mt_donut: $(MT_DONUT_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) $(EXT_LDADD) -o $@

coremark/coremark:
	USE_SYSROOT=../$(USE_SYSROOT) \
	USE_GLIBC=$(USE_GLIBC) \
	USE_NEWLIB=$(USE_NEWLIB) \
	CC="$(CC)" \
	CPPFLAGS="$(APP_CPPFLAGS)" \
	CFLAGS="$(APP_CFLAGS)" \
	LDFLAGS="$(APP_LDFLAGS)" \
	$(MAKE) -C coremark

dhrystone/dhrystone:
	USE_SYSROOT=../$(USE_SYSROOT) \
	USE_GLIBC=$(USE_GLIBC) \
	USE_NEWLIB=$(USE_NEWLIB) \
	CC="$(CC)" \
	CPPFLAGS="$(APP_CPPFLAGS)" \
	CFLAGS="$(APP_CFLAGS)" \
	LDFLAGS="$(APP_LDFLAGS)" \
	$(MAKE) -C dhrystone

clean:
	rm -f $(BINS) $(OBJS)
	$(MAKE) -C coremark $@
	$(MAKE) -C dhrystone $@

include bmetal.mk
