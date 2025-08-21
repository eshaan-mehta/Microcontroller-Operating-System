.syntax unified
.cpu cortex-m4
.thumb

.global SVC_Handler
.type SVC_Handler, %function

.section .text.SVC_Handler, "ax", %progbits
.weak SVC_Handler

SVC_Handler:
    TST lr, #4
    ITE EQ
    MRSEQ r0, MSP
    MRSNE r0, PSP
    B SVC_Handler_Main
