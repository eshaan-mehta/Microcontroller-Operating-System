#include "common.h"
#include "k_task.h"
#include "k_mem.h"
#include <stdint.h>
#include "stm32f4xx_it.h"
#include "main.h"

U32 HEAP_START;
U32 HEAP_END;

// Globals from k_mem.h
META* free_head = NULL;
META* alloc_head = NULL;
int memory_initialized = 0;

int Memory_Init() {
    if (!kernel_initialized || memory_initialized) {
        return RTX_ERR;
    }

    HEAP_START = (U32)&_img_end;
    HEAP_END = (U32)&_estack - (U32)&_Min_Stack_Size;
    
    free_head = (META*)HEAP_START;
    *free_head = (META){
        .block_size = (U32)((char*)HEAP_END - (char*)HEAP_START - sizeof(META)),
        .next = NULL,
        .prev = NULL,
        .tid = TID_NULL
    };
    
    alloc_head = NULL;
    memory_initialized = 1;
    
    return RTX_OK;
}

void* Malloc(size_t size) {
    if (!kernel_initialized || !memory_initialized || size == 0) {
        return NULL;
    }

    // 4 byte align the requeted size
    if (size%4 != 0) {
        size += 4-(size%4);
    }

    // find first block large enough
    META* cur = free_head;
    while (cur && cur->block_size < size) {
        cur = cur->next;
    }
    if (!cur) return NULL;

    U16 leftover = cur->block_size - size;

    // split cur into new free block
    if (leftover > sizeof(META) + 4) {
        META* leftover_free = (META*)((char*)cur + sizeof(META) + size); // starting address of new block
        leftover_free->block_size = leftover - sizeof(META);
        leftover_free->tid = TID_NULL;
        leftover_free->next = cur->next;
        leftover_free->prev = cur->prev;
        
        // update pointers of the adjacent blocks
        if (leftover_free->next) leftover_free->next->prev = leftover_free;
        if (leftover_free->prev) leftover_free->prev->next = leftover_free;
        else free_head = leftover_free;

        // update cur block size to match
        cur->block_size = size;
    } else {
        // use whole block: remove cur from free list
        if (cur->next) cur->next->prev = cur->prev;
        if (cur->prev) cur->prev->next = cur->next;
        else free_head = cur->next;
    }

    // add cur front of alloc list (don't need to sort)
    cur->next = alloc_head;
    if (alloc_head) alloc_head->prev = cur;
    cur->prev = NULL;
    alloc_head = cur;
    cur->tid = current_tid;

    // return pointer to just after metadata
    return (void*)((char*)cur + sizeof(META));
}

int Dealloc(void* ptr) {
    if (!kernel_initialized || !memory_initialized) {
        return RTX_ERR;
    }

    if (ptr == NULL) {
        return RTX_OK;
    }
    
    // ensure pointer in the alloc list and at the start of block
    META* alloc = alloc_head;
    int found = 0;
    while (alloc) {
        if ((void*)((char*)alloc + sizeof(META)) == ptr) {
            found = 1;
            break;
        }
        alloc = alloc->next;
    }
    if (!found) return RTX_ERR;
    
    // ensure block belongs to owner
    META* meta = (META*)((char*)ptr - sizeof(META));
    if (meta->tid != current_tid) {
        return RTX_ERR;
    } 
    
    // remove from alloc list
    if (meta->next) meta->next->prev = meta->prev;
    if (meta->prev) meta->prev->next = meta->next;
    else alloc_head = meta->next;

    // insert into sorted free list
    META* next = free_head;
    META* prev = NULL;
    while (next && next < meta) {
        prev = next;
        next = next->next;
    }
    // insert between prev and next
    meta->next = next;
    meta->prev = prev;
    if (next) next->prev = meta;
    if (prev) prev->next = meta;
    else free_head = meta;

    // coalesce with next
    int adj_next = meta->next && 
                ((char*)meta + sizeof(META) + meta->block_size == (char*)(meta->next));
    if (adj_next) {
        META* nxt = meta->next;
        meta->block_size += sizeof(META) + nxt->block_size;
        meta->next = nxt->next;
        if (nxt->next) nxt->next->prev = meta;
    }

    // coalesce with prev
    int adj_prev = meta->prev && 
                ((char*)meta->prev + sizeof(META) + meta->prev->block_size == (char*)meta);
    if (adj_prev) {
        META* prv = meta->prev;
        prv->block_size += sizeof(META) + meta->block_size;
        prv->next = meta->next;
        if (meta->next) meta->next->prev = prv;
    }

    meta->tid = TID_NULL;
    return RTX_OK;
}

int Memory_Utility(size_t size) {
    if (!kernel_initialized || !memory_initialized || size==0) {
        return 0;
    }
    
    unsigned int count = 0;
    
    META *cur = free_head;
    while (cur) {
        size_t region_bytes = sizeof(META) + cur->block_size;

        if (region_bytes < size) {
            ++count;
        }
        cur = cur->next;
    }

    return count;
}
