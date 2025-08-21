#include "common.h"
#include "k_task.h"
#include "k_mem.h"
#include <stdint.h>
#include "stm32f4xx_it.h"
#include "main.h"

TCB buffer[MAX_TASKS];
int kernel_initialized = 0;
int kernel_started = 0;
int current_tid = 0;
int task_count = 0;
static uint8_t idle_stack[STACK_SIZE];

static inline void build_initial_frame(int index)
{
    U32 *stack_pointer = (U32*)buffer[index].stack_high;

    /* Hardware frame */
    *(--stack_pointer) = 0x01000000U;              /* xPSR */
    *(--stack_pointer) = (U32)buffer[index].ptask; /* PC */
    *(--stack_pointer) = 0xFFFFFFFDU;              /* LR (EXC_RETURN) */
    *(--stack_pointer) = 0;                        /* R12 */
    *(--stack_pointer) = 0;                        /* R3 */
    *(--stack_pointer) = 0;                        /* R2 */
    *(--stack_pointer) = 0;                        /* R1 */
    *(--stack_pointer) = 0;                        /* R0 */

    /* Callee-saved (dummy) */
    for (int j = 0; j < 8; j++) {
        *(--stack_pointer) = 0xA;                  /* R11..R4 */
    }
    buffer[index].stack_ptr = stack_pointer;
}

void Kernel_Init(void)
{
    // intiialize the TCB with all default values
    // NOTE: keep in mind for lab 3, we will need a function pointer for the null task
    for (int i = 0; i < MAX_TASKS; i++)
    {
        buffer[i].state = DORMANT;
        buffer[i].tid = TID_NULL;
        buffer[i].ptask = NULL;
        buffer[i].stack_size = 0;
        buffer[i].stack_high = 0;
        buffer[i].stack_ptr = 0;
        buffer[i].deadline = 0;
        buffer[i].time_remaining = 0;
    }

    kernel_initialized = 1;
    kernel_started = 0;
    current_tid = TID_NULL;
    task_count = 1;

    Memory_Init();

    // configure null task
    void *mem = Malloc(STACK_SIZE);
    if (mem != NULL) {
        buffer[0].ptask = &idleTask;
        U32 top0 = (U32)((char*)mem + STACK_SIZE);
        top0 &= ~0x7U;
        buffer[0].stack_high = top0;
        buffer[0].stack_size = STACK_SIZE;
        buffer[0].deadline = UINT32_MAX;
        buffer[0].time_remaining = UINT32_MAX;
        buffer[0].state = READY;
        buffer[0].tid = TID_NULL;
        build_initial_frame(0);
    }

    //configure interrupts
    SHPR3 = (SHPR3 & ~(0xFFU << 24)) | (0xF0U << 24); //SysTick is lowest priority (highest number)
    SHPR3 = (SHPR3 & ~(0xFFU << 16)) | (0xE0U << 16); //PendSV is in the middle
    SHPR2 = (SHPR2 & ~(0xFFU << 24)) | (0xD0U << 24); //SVC is highest priority (lowest number)
}

int Kernel_CreateTask(TCB *task)
{
    if (!kernel_initialized || task == NULL || task->stack_size < STACK_SIZE || task_count >= MAX_TASKS || task->ptask == NULL || task->stack_size % 8 != 0)
    {
        printf("[osCreateTask] Invalid parameters for task creation.\r\n");
        return RTX_ERR;
    }

    __disable_irq();
    for (int i = 1; i < MAX_TASKS; i++)
    {
        if (buffer[i].state != DORMANT)
        {
            continue;
        }

        // IMPORTANT: temporarily assign current_tid to the new tid because malloc assigns memory to the current tid
        int temp = current_tid;
        current_tid = i;
        uint8_t *mem = Malloc(task->stack_size); // use the kernel malloc api
        current_tid = temp; // need to revert before continuing

        if (!mem) {
            break;
        }

        buffer[i].ptask = task->ptask;
        buffer[i].tid = i;

        buffer[i].stack_size = task->stack_size;
        U32 raw_high = (U32)((char*)mem + task->stack_size);
        raw_high &= ~0x7U; // force 8-byte alignment
        buffer[i].stack_high = raw_high;
        buffer[i].stack_ptr = (U32*)buffer[i].stack_high;
        buffer[i].state = READY;
        buffer[i].deadline = DEADLINE_BASE;
        buffer[i].time_remaining = buffer[i].deadline;

        build_initial_frame(i);

        task->tid = i;
        task_count++;

        // ask scheduler to pick next task (preemption if needed)
        if (kernel_started && buffer[i].time_remaining < buffer[current_tid].time_remaining) {
            SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
        }
        
        __enable_irq();
        return RTX_OK;
    }

    __enable_irq();
    return RTX_ERR;
}

int Kernel_TaskInfo(task_t tid, TCB *task_copy)
{
    if (!kernel_initialized || task_copy == NULL)
    {
        return RTX_ERR;
    }
    if (tid >= MAX_TASKS || tid == TID_NULL || buffer[tid].state == DORMANT)
    {
        return RTX_ERR;
    }
    *task_copy = buffer[tid]; // copy the task info with a different space in memory

    return RTX_OK;
}

int Kernel_GetTID(void)
{
    return (kernel_initialized && kernel_started) ? current_tid : TID_NULL;
}

int Kernel_TaskExit(void)
{
    if (current_tid == TID_NULL || current_tid >= MAX_TASKS)
        return RTX_ERR;

    buffer[current_tid].state = DORMANT;

    void *stack_block_start = (void*)((char*)buffer[current_tid].stack_high - buffer[current_tid].stack_size);
    
    int result = Dealloc(stack_block_start);
    if (result != RTX_OK) {
        return RTX_ERR;
    }
    buffer[current_tid].stack_ptr = (U32 *)buffer[current_tid].stack_high;
    task_count--;

    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // yield
    return RTX_OK;
}

void Kernel_Sleep(int timeInMs)
{
    if (timeInMs <= 0) {
        buffer[current_tid].state = READY;
    } else {
        buffer[current_tid].state = SLEEPING;
        buffer[current_tid].time_remaining = (U32)timeInMs;
    }

    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; /* trigger context switch */
}

void Kernel_PeriodYield()
{
    buffer[current_tid].state          = SLEEPING;
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; /* trigger context switch */
}

int Kernel_SetDeadline(int deadline, task_t TID){
    if (deadline <= 0 || TID > MAX_TASKS || TID == current_tid || buffer[TID].state != READY) {
        return RTX_ERR;
    }

    __disable_irq();

    buffer[TID].deadline = deadline;
    buffer[TID].time_remaining = deadline;
    if(kernel_started && buffer[current_tid].time_remaining > buffer[TID].time_remaining) {
        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    }

    __enable_irq();
    return RTX_OK;
}

int Kernel_CreateDeadlineTask(int deadline, TCB* task){
    if (deadline <= 0 || !kernel_initialized || task == NULL || task->stack_size < STACK_SIZE || task_count >= MAX_TASKS || task->ptask == NULL || task->stack_size % 8 != 0)
    {
        printf("[osCreateTask] Invalid parameters for task creation.\r\n");
        return RTX_ERR;
    }

    for (int i = 1; i < MAX_TASKS; i++)
    {
        if (buffer[i].state != DORMANT)
        {
            continue;
        }

        // IMPORTANT: temporarily assign current_tid to the new tid because malloc assigns memory to the current tid
        int temp = current_tid;
        current_tid = i;
        uint8_t *mem = Malloc(task->stack_size); // use the kernel malloc api
        current_tid = temp; // need to revert before continuing

        if (!mem) {
            break;
        }

        buffer[i].ptask = task->ptask;
        buffer[i].tid = i;
        buffer[i].stack_size = task->stack_size;

        U32 raw_high = (U32)((char*)mem + task->stack_size);
        raw_high &= ~0x7U;
        buffer[i].stack_high = raw_high;
        buffer[i].stack_ptr  = (U32*)raw_high;
        buffer[i].state = READY;
        buffer[i].deadline = deadline;
        buffer[i].time_remaining = deadline;

        build_initial_frame(i);

        task->tid = i;
        task_count++;

        // ask scheduler to pick next task (preemption if needed)
        if (kernel_started && buffer[i].time_remaining < buffer[current_tid].time_remaining) {
            SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
        }
        
        return RTX_OK;
    }

    return RTX_ERR;
}
