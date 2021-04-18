# serprog project Makefile
SHELL:= bash -e

# Cross compile prefix
CROSS_COMPILE := ~/opt/toolchain/gcc-arm-none-eabi-10-2020-q4-major/bin/arm-none-eabi-

# Toolchain define
CC := $(CROSS_COMPILE)gcc
OBJCOPY := $(CROSS_COMPILE)objcopy
SIZE := $(CROSS_COMPILE)size

# Target
target := serprog

# Build output directory
OUTPUT := build

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

# Default ASMFLAGS
ASMFLAGS := -Wall -fno-common -ffunction-sections -fdata-sections
ASMFLAGS += -ffreestanding -fno-builtin -mapcs -std=gnu99

# Default CFLAGS
CFLAGS := -Wall -fno-common -ffunction-sections -fdata-sections
CFLAGS += -ffreestanding -fno-builtin -mapcs -std=gnu99
CFLAGS += -MMD -MP

# Default LDFLAGS
LDFLAGS := -Wall -fno-common -ffunction-sections -fdata-sections
LDFLAGS += --specs=nano.specs --specs=nosys.specs
LDFLAGS += -ffreestanding -fno-builtin -mapcs
LDFLAGS += -Xlinker --gc-sections -Xlinker -static

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
LDFLAGS += -Xlinker -Map=$(OUTPUT)/output.map

# Common rules
objs := $(patsubst %.c,$(OUTPUT)/%.o,$(srcs))
objs := $(patsubst %.S,$(OUTPUT)/%.o,$(objs))
deps := $(objs:%.o=%.d)

$(OUTPUT)/$(target): $(objs)
	@echo -e "  LD\t$@"
	@$(CC) -o $@ $^ $(LDFLAGS)
	@echo -e "  HEX\t$@"
	@$(OBJCOPY) -O ihex $@ $@.hex
	@$(SIZE) --format=berkeley $@

show:
	@echo $(objs) | tr " " "\n"

flash: $(OUTPUT)/$(target)
	@pyocd flash -t lpc54114 $(OUTPUT)/$(target).hex

-include $(deps)
$(OUTPUT)/%.o: %.c
	@echo -e "  CC\t$<"
	@if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
	@$(CC) -c -o $@ $< $(CFLAGS)

$(OUTPUT)/%.o: %.S
	@echo -e "  AS\t$<"
	@if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
	@$(CC) -c -o $@ $< $(ASMFLAGS)

.PHONY: clean
clean:
	@echo "  CLEAN"
	@rm -rf $(OUTPUT)
