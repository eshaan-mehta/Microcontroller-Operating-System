// #include "common.h"
// #include "k_task.h"
// #include "stm32f4xx_hal.h"
// #include <stdio.h>

// #define BUSY_COUNT 50000   // Tune as needed for ~1 ms delay

// // //------------------------------------------------------------------------------
// // // Task that never yields — runs full timeslice before being preempted
// // void TaskNoYield(void *arg) {
// //     (void)arg;
// //     int count = 0;
// //     for (;;) {
// //         printf("[TaskNoYield] Running iteration %d\r\n", count++);
// //         for (volatile int i = 0; i < BUSY_COUNT; i++);
// //     }
// // }

// // //------------------------------------------------------------------------------
// // // Task that yields after one print — voluntarily gives up the CPU
// // void TaskWithYield(void *arg) {
// //     (void)arg;
// //     int count = 0;
// //     for (;;) {
// //         printf("[TaskWithYield] Yielding at iteration %d\r\n", count++);
// //         osYield();
// //     }
// // }


// void Task1(void *) {
//    while(1){
//      printf("1\r\n");
//      for (int i_cnt = 0; i_cnt < 5000; i_cnt++);
//      osYield();
//    }
// }


// void Task2(void *) {
//    while(1){
//      printf("2\r\n");
//      for (int i_cnt = 0; i_cnt < 5000; i_cnt++);
//      osYield();
//    }
// }


// void Task3(void *) {
//    while(1){
//      printf("3\r\n");
//      for (int i_cnt = 0; i_cnt < 5000; i_cnt++);
//      osYield();
//    }
// }



// int main(void) {
// 	/* MCU Configuration: Don't change this or the whole chip won't work!*/

//   /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
//   HAL_Init();
//   /* Configure the system clock */
//   SystemClock_Config();

//   /* Initialize all configured peripherals */
//   MX_GPIO_Init();
//   MX_USART2_UART_Init();
//   /* MCU Configuration is now complete. Start writing your code below this line */

//   osKernelInit();

//   TCB st_mytask;
//   st_mytask.stack_size = STACK_SIZE;
//   st_mytask.ptask = &Task1;
//   osCreateTask(&st_mytask);


//   st_mytask.ptask = &Task2;
//   osCreateTask(&st_mytask);


//   st_mytask.ptask = &Task3;
//   osCreateTask(&st_mytask);

//   osKernelStart();

//   while (1);
// }


#include "common.h"
#include "k_task.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>

#define BUSY_COUNT 50000   // ~1 ms spin-count

volatile int Acount = 0, Bcount = 0;

// Task A: no yields, 40 ms deadline
void TaskA(void *arg) {
    (void)arg;
    for (;;) {
        printf("A%d =%d\r\n", Acount, Bcount);
        // spin for ~8 ms
        for (volatile int i = 0; i < BUSY_COUNT * 8; i++);
        Acount++;
    }
}

// Task B: no yields,  8 ms deadline
void TaskB(void *arg) {
    (void)arg;
    for (;;) {
        printf("B%d =%d\r\n", Bcount, Acount);
        // spin for ~1 ms
        for (volatile int i = 0; i < BUSY_COUNT; i++);
        Bcount++;
    }
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    osKernelInit();
    TCB t;
    t.stack_size = STACK_SIZE;

    // Create the two deadline-driven tasks:
    t.ptask = &TaskA;
    osCreateDeadlineTask(40, &t);  // 40 ms slice

    t.ptask = &TaskB;
    osCreateDeadlineTask( 8, &t);  //  8 ms slice

    osKernelStart();
    for (;;);
}
