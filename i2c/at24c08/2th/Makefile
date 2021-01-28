KERNELDIR := /home/hlk/mtkopenwrt/linux-3.10.14

TOPDIR  := /home/hlk/mtkopenwrt/staging_dir
ARCH = mips
CROSS_COMPILE = $(TOPDIR)/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-
DEBUG = y 
export CC      := $(TOPDIR)/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-gcc
export LD      := $(TOPDIR)/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-ld
###################kernel moudel###################################

ccflags-y += $(MODULE_FLAGS) -Inet/nat

#MODULE_NAME=app_memdev

#OBJS:= k_proc.o
#OBJS+= k_uilt.o
#obj-m += $(MODULE_NAME).o
#$(MODULE_NAME)-objs += $(OBJS)
obj-m += i2c_mt7621_platdev.o
obj-m += i2c-ralink.o

##################user moudel #################################

#compile and lib parameter
export TARGET  := i2c-test
export LIBS    := -L$(TOPDIR)/target-mipsel_24kec+dsp_uClibc-0.9.33.2
export LDFLAGS :=
#export DEFINES := -lubox -luci
#export INCLUDE := -I$(TOPDIR)/target-mipsel_24kc_musl-1.1.16/usr/include
#export CFLAGS  := -g -Wall -O3 $(DEFINES) $(INCLUDE)
export CFLAGS  := -g -Wall -O3 
export CXXFLAGS:= $(CFLAGS) 

ifeq ($(DEBUG),y)
	DEBFLAGS = -O -g -DSCULL_DEBUG 
else
	DEBFLAGS = -O2
endif 
###################### build ##########################

all: modules app

modules:
	$(MAKE) -C $(KERNELDIR)  M=$(shell pwd) modules ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
	rm -f *.o *.mod.o *.mod.c *.symvers  modul* 

$(MODULE_NAME).o: $(OBJS)
	$(LD) $(LD_RFLAG) -r -o $@ $(OBJS)
	
app: 
	$(MAKE) -C ./src/

clean:
	rm -f *.o *.mod.o *.mod.c *.symvers  modul* *.ko .*.ko.cmd .*.mod.o.cmd .*.o.cmd
	#$(MAKE) -C $(KERNELDIR) M=$(shell pwd) modules clean ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
