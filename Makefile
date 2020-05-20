obj-m := globalmem.o
KERNEL_DIR :=/home/fniu/projects/self-S/linux/kernel/linux
default:
	make -C $(KERNEL_DIR) M=$(shell pwd) modules
install:
	insmod globalmem.ko
uninstall:
	rmmod globalmem.ko

clean:
	make -C $(KERNEL_DIR) M=$(shell pwd) clean
	