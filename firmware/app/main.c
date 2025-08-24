#include <stdint.h>
#include <stdbool.h>

// TODO: Wire up STM32CubeF4 HAL init for STM32F401 and UART2 for logging.
// For now, provide a bare loop so the ELF can be loaded in Renode during scaffolding.

int memfault_platform_boot(void); // provided by memfault_platform_port.c (stub for now)

int main(void) {
  // TODO: HAL_Init(); SystemClock_Config(); MX_USART2_UART_Init(); etc.

  // Initialize Memfault platform (will no-op until fully wired)
  (void)memfault_platform_boot();

  // Simple idle loop
  while (1) {
    // TODO: toggle LED, feed heartbeat metrics, etc.
  }

  // Unreachable
  // return 0;
}
