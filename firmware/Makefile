# Simple Makefile for STM32F401 + Memfault demo
# Usage:
#   make -j
#   make clean
#   make USE_RCC_RESET=1    # enable RCC-based reset reason (requires CubeF4 HAL/LL headers)

# Toolchain
CC              := arm-none-eabi-gcc
AS              := arm-none-eabi-gcc
LD              := arm-none-eabi-gcc
OBJCOPY         := arm-none-eabi-objcopy
SIZE            := arm-none-eabi-size

# Paths
FW_DIR          := $(CURDIR)
APP_DIR         := $(FW_DIR)/app
STARTUP_DIR     := $(FW_DIR)/startup
LINKER_DIR      := $(FW_DIR)/linker
BUILD_DIR       := $(FW_DIR)/build_make
OBJ_DIR         := $(BUILD_DIR)/obj

# Third-party
SDK_ROOT        ?= $(FW_DIR)/../third_party/memfault/memfault-firmware-sdk
CUBE_DRIVERS    ?= $(FW_DIR)/../third_party/STM32CubeF4/Drivers

# Options
USE_RCC_RESET   ?= 0

# CPU/Compiler flags
CPU_FLAGS       := -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
CFLAGS_BASE     := $(CPU_FLAGS) -ffunction-sections -fdata-sections -Wall -Wextra -O0 \
                   -DSTM32F401xE
ASFLAGS         := $(CPU_FLAGS)
LDFLAGS         := $(CPU_FLAGS) -Wl,--gc-sections -Wl,--build-id -T$(LINKER_DIR)/STM32F401.ld

# Libraries
LDLIBS          := -lc -lm -lgcc -lnosys

# Includes
INCLUDES        := \
  -I$(APP_DIR) \
  -I$(SDK_ROOT)/components/include \
  -I$(SDK_ROOT)/ports/include

# Optional CubeF4 headers (needed for RCC LL access when USE_RCC_RESET=1)
ifeq ($(USE_RCC_RESET),1)
INCLUDES        += \
  -I$(CUBE_DRIVERS)/STM32F4xx_HAL_Driver/Inc \
  -I$(CUBE_DRIVERS)/CMSIS/Device/ST/STM32F4xx/Include \
  -I$(CUBE_DRIVERS)/CMSIS/Include
endif

CFLAGS          := $(CFLAGS_BASE) $(INCLUDES)

# Sources
APP_SRCS        := \
  $(APP_DIR)/main.c \
  $(APP_DIR)/memfault_platform_port.c

STARTUP_SRCS    := \
  $(STARTUP_DIR)/startup_stm32f401.s

# Memfault components (all .c under components)
MEMFAULT_SRCS   := $(shell find $(SDK_ROOT)/components -type f -name '*.c')

# Optional RCC reboot tracking source
ifeq ($(USE_RCC_RESET),1)
RCC_SRC         := $(SDK_ROOT)/ports/stm32cube/f4/rcc_reboot_tracking.c
else
RCC_SRC         :=
endif

SRCS            := $(APP_SRCS) $(STARTUP_SRCS) $(MEMFAULT_SRCS) $(RCC_SRC)

# Objects
OBJS            := $(patsubst $(FW_DIR)/%.c,$(OBJ_DIR)/%.o,$(filter %.c,$(SRCS))) \
                   $(patsubst $(FW_DIR)/%.s,$(OBJ_DIR)/%.o,$(filter %.s,$(SRCS))) \
                   $(patsubst $(FW_DIR)/%.S,$(OBJ_DIR)/%.o,$(filter %.S,$(SRCS)))

# Target
ELF             := $(BUILD_DIR)/app.elf

.PHONY: all clean dirs size

all: $(ELF) size

$(ELF): dirs $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# C sources
$(OBJ_DIR)/%.o: $(FW_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Assembly sources
$(OBJ_DIR)/%.o: $(FW_DIR)/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(FW_DIR)/%.S
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

size: $(ELF)
	$(SIZE) $(ELF)

dirs:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
