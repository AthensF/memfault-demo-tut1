cmake_minimum_required(VERSION 3.16)

# Force ARM GCC toolchain before declaring the project to prevent AppleClang fallback
if(NOT DEFINED CMAKE_C_COMPILER)
  set(CMAKE_C_COMPILER arm-none-eabi-gcc)
endif()
if(NOT DEFINED CMAKE_ASM_COMPILER)
  set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
endif()

# Ensure CMake doesn't inject macOS host flags like -arch/-isysroot for cross-compiles
set(CMAKE_OSX_ARCHITECTURES "" CACHE STRING "" FORCE)
set(CMAKE_OSX_DEPLOYMENT_TARGET "" CACHE STRING "" FORCE)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(stm32f401_memfault_demo C ASM)

# Toolchain assumptions: arm-none-eabi-* in PATH

# CPU flags for STM32F401 (Cortex-M4, no FPU)
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=soft")

set(CMAKE_C_FLAGS "${CPU_FLAGS} -ffunction-sections -fdata-sections -Wall -Wextra -O0")
set(CMAKE_ASM_FLAGS "${CPU_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CPU_FLAGS} -Wl,--gc-sections -Wl,--build-id -T${CMAKE_CURRENT_LIST_DIR}/linker/STM32F401.ld")

# Include paths
include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/app
  ${CMAKE_CURRENT_LIST_DIR}/../third_party/memfault/memfault-firmware-sdk/components/include
  ${CMAKE_CURRENT_LIST_DIR}/../third_party/memfault/memfault-firmware-sdk/ports/include
  ${CMAKE_CURRENT_LIST_DIR}/../third_party/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../third_party/STM32CubeF4/Drivers/CMSIS/Device/ST/STM32F4xx/Include
  ${CMAKE_CURRENT_LIST_DIR}/../third_party/STM32CubeF4/Drivers/CMSIS/Include
)

# Define MCU variant for STM32Cube headers
add_compile_definitions(STM32F401xE)

# Memfault sources (components only; platform hooks provided in app/)
file(GLOB_RECURSE MEMFAULT_SRCS
  ${CMAKE_CURRENT_LIST_DIR}/../third_party/memfault/memfault-firmware-sdk/components/*.c
)

# App sources
set(APP_SRCS
  ${CMAKE_CURRENT_LIST_DIR}/app/main.c
  ${CMAKE_CURRENT_LIST_DIR}/app/memfault_platform_port.c
  ${CMAKE_CURRENT_LIST_DIR}/startup/startup_stm32f401.s
  ${CMAKE_CURRENT_LIST_DIR}/../third_party/memfault/memfault-firmware-sdk/ports/stm32cube/f4/rcc_reboot_tracking.c
)

add_executable(app.elf ${APP_SRCS} ${MEMFAULT_SRCS})

# Link against minimal libs
target_link_libraries(app.elf m c gcc nosys)

# Output directory
set_target_properties(app.elf PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/build)

# Print size after build
add_custom_command(TARGET app.elf POST_BUILD
  COMMAND arm-none-eabi-size app.elf
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/build
)
