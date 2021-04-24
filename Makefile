# serprog Makefile

include scripts/Makefile.common

# Target
target := serprog

# Device
device := lpc54114

# Source files
srcs += \
	$(wildcard app/*.c) \
	$(wildcard board/*.c) \
	device/drivers/fsl_reset.c \
	device/drivers/fsl_common.c \
	device/drivers/fsl_clock.c \
	device/drivers/fsl_power.c \
	device/drivers/fsl_flexcomm.c \
	device/drivers/fsl_gpio.c \
	device/drivers/fsl_usart.c \
	device/drivers/fsl_spi.c \
	device/gcc/startup_LPC54114_cm4.S \
	device/system_LPC54114_cm4.c \
	components/FreeRTOS-Kernel/tasks.c \
	components/FreeRTOS-Kernel/list.c \
	components/FreeRTOS-Kernel/timers.c \
	components/FreeRTOS-Kernel/queue.c \
	components/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c \
	components/tinyusb/src/tusb.c \
	components/tinyusb/src/common/tusb_fifo.c \
	components/tinyusb/src/device/usbd.c \
	components/tinyusb/src/device/usbd_control.c \
	components/tinyusb/src/class/cdc/cdc_device.c \
	components/tinyusb/src/class/vendor/vendor_device.c \
	components/tinyusb/src/portable/nxp/lpc_ip3511/dcd_lpc_ip3511.c

# Project specific ASMFLAGS
ASMFLAGS += -DNDEBUG -D__STARTUP_CLEAR_BSS
ASMFLAGS += -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb

# Preject specific CFLAGS
CFLAGS += -DNDEBUG -DCPU_LPC54114J256BD64_cm4
CFLAGS += -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb
## tinyusb
CFLAGS += \
	-DCFG_TUSB_MCU=OPT_MCU_LPC54XXX \
	-DCFG_TUSB_OS=OPT_OS_FREERTOS \
	-DCFG_TUSB_MEM_SECTION='__attribute__((section(".data")))' \
	-DCFG_TUSB_MEM_ALIGN='__attribute__((aligned(64)))'
## Include file search directories
CFLAGS += -Iapp/
CFLAGS += -Iboard/
CFLAGS += -ICMSIS/Include/
CFLAGS += -Idevice/
CFLAGS += -Idevice/drivers
CFLAGS += -Icomponents/tinyusb/src
CFLAGS += -Icomponents/FreeRTOS-Kernel/include
CFLAGS += -Icomponents/FreeRTOS-Kernel/portable/GCC/ARM_CM4F
## Optimization
CFLAGS += -O2

# Project specific LDFLAGS
LDFLAGS += -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb
## Libraries
LDFLAGS += -L device/gcc -l power_cm4_hardabi
## Link script
LDFLAGS += -Tdevice/gcc/LPC54114J256_cm4_flash.ld
## Map file
LDFLAGS += -Xlinker -Map=$(O)/output.map

include scripts/Makefile.check
include scripts/Makefile.rules
