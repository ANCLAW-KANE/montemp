obj-m += montemp.o
montemp-objs := main.o file_utils.o thermal_hwmon.o stats.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean