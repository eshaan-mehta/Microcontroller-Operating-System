// #include "common.h"
// #include "k_task.h"
// #include "main.h"
// #include <stdio.h>

// /* ---------------- demo globals ---------------- */
// static int  i_test = 0;
// static int  count  = 0;

// /* ---------------- task bodies  ---------------- */



// void GenericTask(void *)      /* simple round-robin task   */
// {
//     for (;;)
//     {
//         printf("Generic task tid-%d running\r\n", osGetTID());
//         osYield();
//     }
// }

// void Task1(void *)            /* self-recreates then exits */
// {
//     i_test++;
// //    osYield();

//     TCB t;
//     t.ptask      = GenericTask;
//     t.stack_size = 0x400;
//     osCreateTask(&t);         /* will consume a fresh slot */

//     printf("going to end task 1");
//     osTaskExit();             /* free our current slot     */
//     printf("ended task 1");

// }

// void Task2(void *)            /* prints once, then exits   */
// {
//     printf("Back to you %d\r\n", i_test);

//     TCB t;
//     t.ptask = GenericTask;
//     t.stack_size = 0x200;
//     if (osCreateTask(&t) == RTX_OK)
//         printf("Spawned worker tid-%d\r\n", t.tid);
//     else
//         printf("create failed\r\n");

//     for (;;)
//         osYield();
// }
// /* --------------------- main -------------------- */
// int main(void)
// {
//     HAL_Init();
//     SystemClock_Config();
//     MX_GPIO_Init();
//     MX_USART2_UART_Init();

//     osKernelInit();

//     /* create Task1 (tid-1) */
//     TCB t;
//     t.stack_size = 0x800;
//     t.ptask      = Task1;
//     osCreateTask(&t);

//     /* create Task2 (tid-2) */
//     t.ptask = Task2;
//     osCreateTask(&t);

//     /* create 13 GenericTask instances (tid-3 … tid-15) */
//     for (int i = 0; i < 6; ++i) {
//         t.ptask = GenericTask;
//         osCreateTask(&t);
//     }

//     osKernelStart();          /* scheduler never returns */

//     while (1);
// }