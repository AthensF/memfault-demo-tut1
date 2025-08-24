#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "memfault/components.h"
#include "memfault/ports/reboot_reason.h"
#include "memfault_platform_config.h"

// NOTE: This is a minimal scaffold tailored for STM32F401 + Renode demo.
// Fill in HAL-specific pieces (System reset, timekeeping, UART logging) when wiring STM32CubeF4.

void memfault_platform_get_device_info(sMemfaultDeviceInfo *info) {
  *info = (sMemfaultDeviceInfo){
      .device_serial = "STM32F401-EMU-0001",
      .software_type = "app-fw",
      .software_version = "1.0.0",
      .hardware_version = "stm32f401-nucleo",
  };
}

void memfault_platform_reboot(void) {
  // TODO: Replace with NVIC_SystemReset() once HAL/CMSIS is wired
  while (1) {}
}

bool memfault_platform_time_get_current(sMemfaultCurrentTime *time) {
  *time = (sMemfaultCurrentTime){
      .type = kMemfaultCurrentTimeType_UnixEpochTimeSec,
      .info = {.unix_timestamp_secs = 0},
  };
  return false; // no RTC in demo
}

size_t memfault_platform_sanitize_address_range(void *start_addr, size_t desired_size) {
  static const struct {
    uint32_t start_addr;
    size_t length;
  } s_mcu_mem_regions[] = {
      // SRAM on STM32F401 (up to 96KB at 0x20000000). Use a broad range for demo.
      { .start_addr = 0x20000000, .length = 0x00018000 },
  };

  for (size_t i = 0; i < MEMFAULT_ARRAY_SIZE(s_mcu_mem_regions); i++) {
    const uint32_t lower = s_mcu_mem_regions[i].start_addr;
    const uint32_t upper = lower + s_mcu_mem_regions[i].length;
    if ((uint32_t)start_addr >= lower && ((uint32_t)start_addr < upper)) {
      return MEMFAULT_MIN(desired_size, upper - (uint32_t)start_addr);
    }
  }
  return 0;
}

MEMFAULT_PUT_IN_SECTION(".noinit.mflt_reboot_info") static uint8_t s_reboot_tracking[MEMFAULT_REBOOT_TRACKING_REGION_SIZE];

MEMFAULT_WEAK void memfault_reboot_reason_get(sResetBootupInfo *info) {
  // TODO: Read RCC_CSR flags to map to Memfault reasons
  *info = (sResetBootupInfo){
      .reset_reason_reg = 0x0,
      .reset_reason = kMfltRebootReason_Unknown,
  };
}

void memfault_platform_reboot_tracking_boot(void) {
  sResetBootupInfo reset_info = {0};
  memfault_reboot_reason_get(&reset_info);
  memfault_reboot_tracking_boot(s_reboot_tracking, &reset_info);
}

MEMFAULT_WEAK bool memfault_platform_metrics_timer_boot(uint32_t period_sec, MemfaultPlatformTimerCallback *callback) {
  (void)period_sec; (void)callback;
  return false;
}

MEMFAULT_WEAK uint64_t memfault_platform_get_time_since_boot_ms(void) {
  return 0;
}

// Minimal USART2 (PA2 TX) setup for logging under Renode
// Bare-metal register definitions to avoid HAL dependencies
#define PERIPH_BASE        0x40000000UL
#define AHB1PERIPH_BASE    (PERIPH_BASE + 0x00020000UL)
#define APB1PERIPH_BASE    (PERIPH_BASE + 0x00000000UL)
#define RCC_BASE           (AHB1PERIPH_BASE + 0x00003800UL)  // 0x40023800
#define GPIOA_BASE         (AHB1PERIPH_BASE + 0x00000000UL)  // 0x40020000
#define USART2_BASE        (APB1PERIPH_BASE + 0x00004400UL)  // 0x40004400

#define RCC_AHB1ENR        (*(volatile uint32_t *)(RCC_BASE   + 0x30))
#define RCC_APB1ENR        (*(volatile uint32_t *)(RCC_BASE   + 0x40))
#define GPIOA_MODER        (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_AFRL         (*(volatile uint32_t *)(GPIOA_BASE + 0x20))
#define USART2_SR          (*(volatile uint32_t *)(USART2_BASE + 0x00))
#define USART2_DR          (*(volatile uint32_t *)(USART2_BASE + 0x04))
#define USART2_BRR         (*(volatile uint32_t *)(USART2_BASE + 0x08))
#define USART2_CR1         (*(volatile uint32_t *)(USART2_BASE + 0x0C))

static void prv_uart2_init(void) {
  // Enable clocks: GPIOA (AHB1ENR bit0), USART2 (APB1ENR bit17)
  RCC_AHB1ENR |= (1u << 0);
  RCC_APB1ENR |= (1u << 17);

  // Configure PA2 as AF7 (USART2_TX)
  // MODER: set pin2 to alternate function (10b)
  GPIOA_MODER &= ~(0x3u << (2 * 2));
  GPIOA_MODER |=  (0x2u << (2 * 2));
  // AFRL: set pin2 AF7
  GPIOA_AFRL &= ~(0xFu << (4 * 2));
  GPIOA_AFRL |=  (0x7u << (4 * 2));

  // Assuming APB1 at 16MHz (HSI). Set 115200 baud.
  // BRR = Fck / baud ≈ 16000000 / 115200 ≈ 139 (0x8B)
  USART2_BRR = 0x008B;

  // Enable USART, transmitter
  const uint32_t UE = (1u << 13);
  const uint32_t TE = (1u << 3);
  USART2_CR1 = UE | TE;
}

static void prv_uart2_write(const char *buf, size_t len) {
  const uint32_t SR_TXE = (1u << 7);
  const uint32_t SR_TC  = (1u << 6);
  for (size_t i = 0; i < len; i++) {
    while ((USART2_SR & SR_TXE) == 0) {}
    USART2_DR = (uint8_t)buf[i];
  }
  while ((USART2_SR & SR_TC) == 0) {}
}

static void prv_log_write(const char *prefix, const char *msg) {
  char line[256];
  int n = 0;
  if (prefix && prefix[0]) {
    n = snprintf(line, sizeof(line), "%s%s\n", prefix, msg);
  } else {
    n = snprintf(line, sizeof(line), "%s\n", msg);
  }
  if (n < 0) return;
  size_t len = (size_t)((n < (int)sizeof(line)) ? n : (int)sizeof(line));
  prv_uart2_write(line, len);
}

MEMFAULT_PRINTF_LIKE_FUNC(2, 3) void memfault_platform_log(eMemfaultPlatformLogLevel level, const char *fmt, ...) {
  char buf[192];
  va_list args; va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  const char *prefix = "[I] ";
  if (level == kMemfaultPlatformLogLevel_Warning) prefix = "[W] ";
  else if (level == kMemfaultPlatformLogLevel_Error) prefix = "[E] ";
  prv_log_write(prefix, buf);
}

MEMFAULT_PRINTF_LIKE_FUNC(1, 2) void memfault_platform_log_raw(const char *fmt, ...) {
  char buf[192];
  va_list args; va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  prv_log_write("", buf);
}

int memfault_platform_boot(void) {
  // Bring up UART2 early so logs are visible via Renode telnet (port 3456)
  prv_uart2_init();
  memfault_build_info_dump();
  memfault_device_info_dump();
  memfault_platform_reboot_tracking_boot();

  static uint8_t s_event_storage[1024];
  const sMemfaultEventStorageImpl *evt_storage = memfault_events_storage_boot(s_event_storage, sizeof(s_event_storage));
  memfault_trace_event_boot(evt_storage);
  memfault_reboot_tracking_collect_reset_info(evt_storage);

  sMemfaultMetricBootInfo boot_info = {
      .unexpected_reboot_count = memfault_reboot_tracking_get_crash_count(),
  };
  memfault_metrics_boot(evt_storage, &boot_info);

  MEMFAULT_LOG_INFO("Memfault Initialized!");
  return 0;
}
