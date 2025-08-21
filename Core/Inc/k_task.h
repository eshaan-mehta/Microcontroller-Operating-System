/*
* k_task.h
*
*  Created on: Jan 5, 2024
*      Author: nexususer
*
*      NOTE: any C functions you write must go into a corresponding c file that you create in the Core->Src folder
*/
#include "common.h"
#ifndef INC_K_TASK_H_
#define INC_K_TASK_H_
#define SHPR2 (*((volatile uint32_t*)0xE000ED1C))//SVC is bits 31-28
#define SHPR3 (*((volatile uint32_t*)0xE000ED20))//SysTick is bits 31-28, and PendSV is bits 23-20

typedef struct task_control_block {
    void (*ptask)(void* args);  // 0x00 - 4 bytes
    U32 stack_high;             // 0x04 - 4 bytes
    task_t tid;                 // 0x08 - 4 bytes
    U8 state;                   // 0x0C - 1 byte
    U16 stack_size;             // 0x0D - 2 bytes
    U32* stack_ptr;             // 0x10 - 4 bytes
    U32 deadline;               // 0x14 - 4 bytes
    U32 time_remaining;         // 0x18 - 4 bytes
} TCB;

// API calls exposed to user
void osKernelInit(void);
int  osCreateTask(TCB* task);
int  osKernelStart(void);
void osYield(void);
int  osTaskInfo(task_t tid, TCB* task_copy);
task_t osGetTID(void);
int  osTaskExit(void);
int schedule_next(void);
void idleTask(void);
void osSleep(int timeInMs);
void osPeriodYield();
int osSetDeadline(int deadline, task_t TID);
int osCreateDeadlineTask(int deadline, TCB* task);

// Internal global kernel state
extern TCB buffer[MAX_TASKS];
extern int current_tid;
extern int kernel_initialized;
extern int kernel_started;
extern int task_count;

void Kernel_Init(void);
int Kernel_CreateTask(TCB *task);
int Kernel_Start(void);
int Kernel_TaskInfo(task_t tid, TCB *task_copy);
int Kernel_GetTID(void);
int Kernel_TaskExit(void);
void Kernel_Sleep(int timeInMs);
void Kernel_PeriodYield();
int Kernel_SetDeadline(int deadline, task_t TID);
int Kernel_CreateDeadlineTask(int deadline, TCB* task);

// Assembly Handlers
extern void Kernel_Start_Handler(U32* stackptr);

#endif /* INC_K_TASK_H_ */
