#pragma once

// Memfault platform configuration for STM32F401 + Renode demo

// Project key for uploads (also present in plan XML)
#define MEMFAULT_PROJECT_KEY "F6Wm0RMUXOI79msG9DVbcuh2OIqZepCW"

// Event storage sizes (tune later)
#define MEMFAULT_EVENT_STORAGE_READ_BATCHING_ENABLED 1
#define MEMFAULT_EVENT_STORAGE_READ_BATCHING_MAX_BYTES (1024)

// Reboot tracking config (use default sizes from SDK)
// Metrics heartbeat sampling interval (seconds)
#define MEMFAULT_METRICS_HEARTBEAT_INTERVAL_SECS 60

// Enable minimal logging via platform hooks
#define MEMFAULT_PLATFORM_LOG_CONFIG_DEFAULTS 1

// Use GNU build-id to identify builds
#define MEMFAULT_USE_GNU_BUILD_ID 1
