/* Minimal Cortex-M4 startup for STM32F401 */

  .syntax unified
  .cpu cortex-m4
  .thumb

  .extern main

  .section .isr_vector,"a",%progbits
  .type g_pfnVectors, %object
  .globl g_pfnVectors
g_pfnVectors:

  .word _estack            /* Initial Stack Pointer */
  .word Reset_Handler      /* Reset Handler */
  .word Default_Handler    /* NMI Handler */
  .word Default_Handler    /* Hard Fault Handler */
  .word Default_Handler    /* MPU Fault Handler */
  .word Default_Handler    /* Bus Fault Handler */
  .word Default_Handler    /* Usage Fault Handler */
  .word 0                  /* Reserved */
  .word 0                  /* Reserved */
  .word 0                  /* Reserved */
  .word 0                  /* Reserved */
  .word Default_Handler    /* SVCall Handler */
  .word Default_Handler    /* Debug Monitor Handler */
  .word 0                  /* Reserved */
  .word Default_Handler    /* PendSV Handler */
  .word Default_Handler    /* SysTick Handler */

/* Minimal vector table entries for peripherals (all default) */
  .rept 80
  .word Default_Handler
  .endr

  .size g_pfnVectors, .-g_pfnVectors

  .text
  .thumb
  .thumb_func
  .globl Reset_Handler
Reset_Handler:
  /* Normally data/bss init happens here; for demo keep minimal */
  bl main

LoopForever:
  b LoopForever

  .thumb_func
Default_Handler:
  b .
