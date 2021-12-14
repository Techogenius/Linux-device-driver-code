obj-m := chr_drv_ex1.o
# this will be the name of the object file

KDIR := /lib/modules/$(shell uname -r)/build
# this will automatically take the kernel version

PWD := $(shell pwd)
# we will get the full-path of present working directory
# this variable did not work, so I used the RHS directly

default:
	$(MAKE) -C $(KDIR) M=$(shell pwd) modules
install:
	$(MAKE) -C $(KDIR) M=$(shell pwd) module_install
clean:
	$(MAKE) -C $(KDIR) M=$(shell pwd) clean


