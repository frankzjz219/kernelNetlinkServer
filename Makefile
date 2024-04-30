obj-m += kServer.o
# 指定编译器
CC=gcc-12
# Specify the path for the kernel build system.
KDIR := /lib/modules/$(shell uname -r)/build

# Specify the path for the current directory.
PWD := $(shell pwd)

# Default target to compile the kernel module.
all:
		$(MAKE) -C $(KDIR) M=$(PWD) modules

# Target to clean the build.
clean:
		$(MAKE) -C $(KDIR) M=$(PWD) clean

# Optional: install the module
install:
		$(MAKE) -C $(KDIR) M=$(PWD) modules_install

# Optional: remove the module (useful for cleaning up installation)
uninstall:
		$(MAKE) -C $(KDIR) M=$(PWD) modules_uninstall