Demo using 
1. STM32F4 Makefile that builds a firmware with Memfault SDK, dump_chunks function
2. Renode, flashing that STM32F4
  a. Hooking it up to port 3333 for GDB server
2. Spinning up GDP server on 3333
  a. loading up memfault functions
3.  Pressing buttons, via renode emulation, and see the gdb server posting chunks to memfault

Result
![Demo](demo.gif)

For longer step by step (2 mins):
https://www.loom.com/share/ac729cf37054411994923d6ffa2deb37?sid=1a0f9f72-2666-4609-a3df-dc4ea6a09043



## Overview
This repo demonstrates:
- Building an STM32F4 firmware with the Memfault SDK and a "dump chunks" path.
- Running it in Renode with a GDB server on :3333.
- Pressing a virtual button to trigger `memfault_data_export_dump_chunks()` and sending chunks to the Memfault cloud.

Key files:
- `renode-demo/Makefile_renode_example.mk` — builds firmware, includes Memfault SDK components and RAM-backed coredump storage.
- `renode-demo/renode-example.c` — minimal app; on button release it calls `memfault_data_export_dump_chunks()`.
- `renode-demo/memfault_platform_port.c` — Memfault platform glue; logs wired to `printf` so chunks appear on USART2.
- `renode-demo/renode-config.resc` — Renode script to load the board, firmware, analyzer, and start GDB server 3333.

## Prerequisites
- A Memfault project-key
- Renode
- Arm GNU toolchain (arm-none-eabi-*)
- GDB that supports python


## Build
Run from the repo root:

```sh
make -C renode-demo -j4
```

Artifacts: `renode-demo/renode-example.elf` and `.bin`.

## Run in Renode
From the repo root:

```sh
renode -e "i @renode-demo/renode-config.resc"
```

This will:
- Create an STM32F4 Discovery machine
- Load the ELF
- Open the USART2 analyzer window
- Start a GDB server on port 3333

## Simulate a button press
In the Renode monitor:

```
gpioPortA.UserButton Press
emulation RunFor "100 ms"
gpioPortA.UserButton Release
```

On release, the app prints "button pressed" and calls `memfault_data_export_dump_chunks()`. Base64 chunk lines appear in the USART2 analyzer because `memfault_platform_log()` is wired to `printf`.

## Upload chunks to Memfault (via GDB script)
1) Connect GDB to Renode:
```sh
arm-none-eabi-gdb renode-demo/renode-example.elf \
  -ex "target remote :3333"
```

2) Load Memfault’s GDB helper (see docs: https://mflt.io/send-chunks-via-gdb):
- Source the script that hooks `memfault_data_export_chunk()` and posts chunks to your project.
- Configure your API key and org/project per the script’s instructions.

3) Press the button in Renode. The script auto-detects chunk prints and uploads them to Memfault.

## Alternative: manual export
- Copy the base64 chunk lines from the USART2 analyzer and upload with the Memfault CLI or API.
- Or save binary chunks and upload (example: `chunk_v2_single_chunk_msg.bin`).

## Notes on coredumps (panics)
- Build includes `panics` by default: `MEMFAULT_COMPONENTS := core util panics metrics` in `Makefile_renode_example.mk`.
- Coredump storage is satisfied by the SDK’s RAM-backed port:
  - `$(MEMFAULT_SDK_ROOT)/ports/panics/src/memfault_platform_ram_backed_coredump.c`
- If you don’t need coredumps yet, you can simplify:
  - Change to `MEMFAULT_COMPONENTS := core util metrics`
  - Remove the RAM-backed coredump `CFILES` line.

## Troubleshooting
- No logs in analyzer: ensure `showAnalyzer sysbus.usart2` is open and that `memfault_platform_log()` uses `vprintf` (see `memfault_platform_port.c`).
- GDB cannot connect: confirm Renode started GDB server (`machine StartGdbServer 3333`).
- Linker errors about coredump storage: either include the RAM-backed coredump source or drop `panics`.
- Button path: try fully qualified monitor path `sysbus.gpioPortA.UserButton Press` if needed.

## Directory structure (relevant bits)
```
renode-demo/
  Makefile_renode_example.mk
  renode-example.c
  memfault_platform_port.c
  renode-config.resc
third_party/memfault/memfault-firmware-sdk/
  components/
  ports/
