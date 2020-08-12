

obj-m := ccs811.o
ccs811-objs := ccs811-core.o ccs811-i2c.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
KERNELDIR = /home/sguarin/Facu/IMD/linux

all default: modules
install: modules_install

modules modules_install help clean:
	$(MAKE) -C $(KERNELDIR) M=$(shell pwd) $@
