/* Minimal linker script placeholder for STM32F401 (F401RE-like)
 * Flash: 512K @ 0x08000000
 * SRAM:  96K  @ 0x20000000
 * NOTE: For production, use the exact variant's ST-provided linker script.
 */

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 96K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);
ENTRY(Reset_Handler)

SECTIONS
{
  .isr_vector :
  {
    KEEP(*(.isr_vector))
  } > FLASH

  /* Code and read-only data */
  .text :
  {
    *(.text*)
    *(.rodata*)
    KEEP(*(.init))
    KEEP(*(.fini))
  } > FLASH

  .ARM.exidx :
  {
    *(.ARM.exidx*)
  } > FLASH

  /* GNU build-id note used by Memfault */
  .note.gnu.build-id :
  {
    __start_gnu_build_id_start = .;
    KEEP(*(.note.gnu.build-id))
  } > FLASH

  /* Initialized data in RAM with load image in FLASH */
  .data :
  {
    _sdata = .;
    *(.data*)
    _edata = .;
  } > RAM AT > FLASH
  _sidata = LOADADDR(.data);

  .bss :
  {
    _sbss = .;
    *(.bss*)
    *(COMMON)
    _ebss = .;
  } > RAM

  /* Memfault reboot tracking noinit section */
  .noinit (NOLOAD) :
  {
    *(.noinit*)
    *(.noinit.mflt_reboot_info)
  } > RAM

  /* Provide legacy end symbol expected by nosys sbrk */
  PROVIDE(end = _ebss);
}
