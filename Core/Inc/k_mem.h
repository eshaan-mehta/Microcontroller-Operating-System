/*
 * k_mem.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_K_MEM_H_
#define INC_K_MEM_H_

typedef struct mehtadata {
    U16 block_size;           // 2 bytes
    // U16 padding;              // 2 bytes
    struct mehtadata* next;   // 4 bytes
    struct mehtadata* prev;   // 4 bytes
    task_t tid;               // 4 bytes
} META;

// public APIs exposed to user
int k_mem_init(void);
void* k_mem_alloc(size_t size);
int k_mem_dealloc(void* ptr);
int k_mem_count_extfrag(size_t size);

// global variables
extern META* free_head;
extern META* alloc_head;
extern int memory_initialized;

// internal SVC functions
int Memory_Init();
void* Malloc(size_t size);
int Dealloc(void* ptr);
int Memory_Utility(size_t size);

#endif /* INC_K_MEM_H_ */
