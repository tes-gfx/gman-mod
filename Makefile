obj-m := gman.o
gman-y := \
	gman_drv.o

#ccflags-y := -DDISABLE_ASSERTIONS
ccflags-y += -DDEBUG=1

KERNEL_SRC := $(SDKTARGETSYSROOT)/usr/src/kernel

SRC := $(shell pwd)

.PHONY:
all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)

.PHONY:
modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install

.PHONY:
clean:
	rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.symvers

.PHONY:
deploy: all
	scp *.ko root@$(BOARD_IP):/home/root/
	#scp *.ko root@$(BOARD_IP):/lib/modules/5.4.124-lts-altera/extra
