.syntax unified
.cpu cortex-m4
.thumb

.global Kernel_Start_Handler
.type Kernel_Start_Handler, %function

.section .text.Kernel_Start_Handler, "ax", %progbits

Kernel_Start_Handler:
	LDMIA R0!, {R4-R11}		// Load memory and increment R0 each time
    MSR PSP, R0             // Set PSP to stackptr (parameter to this function)
    LDR LR, =#0xFFFFFFFD     // Switch back to unprivileged mode using PSP
    BX LR                   // Exit
