.syntax unified
.cpu cortex-m4
.thumb

.global PendSV_Handler
.type PendSV_Handler, %function

.section .text.PendSV_Handler, "ax", %progbits

.thumb_func
PendSV_Handler:
    CPSID i  // disable irq

    //store current PSP and push registers 
    MRS R0, PSP
    STMDB R0!, {R4-R11}

    MOV R5, LR // save link register

    // save the PSP into the buffer[old]stack_ptr
    LDR R1, =current_tid
    LDR R2, [R1]
    LDR R3, =buffer
    MOV R4, #28
    MUL R2, R2, R4
    ADD R3, R3, R2
    STR R0, [R3, #16]

    // figure out which task to run next
    BL schedule_next

    MOV LR, R5 // restore link register

    // load new tasks PSP from buffer[new].stack_ptr into RO
    LDR R1, =current_tid
    LDR R2, [R1]
    LDR R3, =buffer
    MOV R4, #28
    MUL R2, R2, R4
    ADD R3, R3, R2
    LDR R0, [R3, #16]

    // pop registers of new tasks stack
    LDMIA R0!, {R4-R11}
    MSR PSP, R0

    CPSIE i // enable irq
    BX LR
