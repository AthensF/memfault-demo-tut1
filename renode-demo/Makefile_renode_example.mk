PROJECT = renode-example
BUILD_DIR = bin

CFILES = renode-example.c

DEVICE=stm32f407vgt6
OOCD_FILE = board/stm32f4discovery.cfg

OPENCM3_DIR=libopencm3

# Add Memfault SDK includes
# Memfault integration (per Cortex-M guide)
MEMFAULT_PORT_ROOT := .
MEMFAULT_SDK_ROOT := ../third_party/memfault/memfault-firmware-sdk
MEMFAULT_COMPONENTS := core util panics metrics #panics
include $(MEMFAULT_SDK_ROOT)/makefiles/MemfaultWorker.mk

# Add Memfault component include paths
INCLUDES += $(patsubst %,-I%,$(MEMFAULT_COMPONENTS_INC_FOLDERS))
INCLUDES += -I$(MEMFAULT_SDK_ROOT)/ports/include
INCLUDES += -I$(MEMFAULT_PORT_ROOT)

# Add Memfault component sources and platform port glue
CFILES += $(MEMFAULT_COMPONENTS_SRCS)
CFILES += memfault_platform_port.c
# Add RAM-backed coredump storage port to satisfy coredump platform symbols
CFILES += $(MEMFAULT_SDK_ROOT)/ports/panics/src/memfault_platform_ram_backed_coredump.c

include $(OPENCM3_DIR)/mk/genlink-config.mk
include rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
