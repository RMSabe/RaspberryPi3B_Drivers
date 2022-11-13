obj-m += ARMTIMER_CtrlMod.o
obj-m += DMA_CtrlMod.o
obj-m += GPIO_CtrlMod.o
obj-m += GPCLK_CtrlMod.o
obj-m += I2C_CtrlMod.o
obj-m += INTR_CtrlMod.o
obj-m += MMU32_mod.o
obj-m += SYSTIMER_CtrlMod.o

KDIR = /lib/modules/$(shell uname -r)/build/

all:
        make -C $(KDIR) M=$(shell pwd) modules
        
clean:
        make -C $(KDIR) M=$(shell pwd) clean
