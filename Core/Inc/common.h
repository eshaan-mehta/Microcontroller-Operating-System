/*
 * common.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: If you feel that there are common
 *      C functions corresponding to this
 *      header, then any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#define TID_NULL 0 //predefined Task ID for the NULL task
#define MAX_TASKS 16 //maximum number of tasks in the system
#define STACK_SIZE 0x200 //min. size of each task’s stack
#define DORMANT 0 //state of terminated task
#define READY 1 //state of task that can be scheduled but is not running
#define RUNNING 2 //state of running task
#define SLEEPING 3
#define RTX_OK 0
#define RTX_ERR -1
#define MSP_INIT_VAL *(uint32_t**)0x0
#define MAIN_STACK_SIZE 0x400

typedef unsigned int U32;
typedef unsigned short U16;
typedef char U8;
typedef unsigned int task_t;
typedef unsigned int size_t;
#define UINT32_MAX 0xFFFFFFFF

// Linker variables
extern U32 _Min_Stack_Size;
extern U32 _img_end;
extern U8 _estack;

// SVC number definitions
#define SVC_KERNEL_START    1
#define SVC_YIELD           2
#define SVC_KERNEL_INIT     3
#define SVC_CREATE_TASK     4
#define SVC_TASK_INFO       5
#define SVC_GET_TID         6
#define SVC_TASK_EXIT       7
#define SVC_MEMORY_INIT     8
#define SVC_MEMORY_ALLOC    9
#define SVC_MEMORY_DEALLOC  10
#define SVC_MEMORY_UTILITY  11
#define SVC_SLEEP           12
#define SVC_PERIOD_YIELD    13
#define SVC_SET_DEADLINE    14
#define SVC_CREATE_DEADLINE 15

#define DEADLINE_BASE 5

// #ifndef NULL
// #define NULL ((void*)0) // Define NULL
// #endif

#endif /* INC_COMMON_H_ */
