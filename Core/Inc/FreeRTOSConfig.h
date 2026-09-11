#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_PREEMPTION            1
#define configUSE_IDLE_HOOK             0
#define configUSE_TICK_HOOK             0
#if defined(STM32F103xB)
#define configCPU_CLOCK_HZ              ((uint32_t)72000000)
#else
#define configCPU_CLOCK_HZ              ((uint32_t)84000000)
#endif
#define configTICK_RATE_HZ              ((TickType_t)1000)
#define configMAX_PRIORITIES            (5)
#define configMINIMAL_STACK_SIZE        ((uint16_t)128)
#if defined(STM32F103xB)
#define configTOTAL_HEAP_SIZE           ((size_t)8192)
#else
#define configTOTAL_HEAP_SIZE           ((size_t)40960)
#endif
#define configMAX_TASK_NAME_LEN         (12)
#define configUSE_TRACE_FACILITY        1
#define configUSE_16_BIT_TICKS          0
#define configIDLE_SHOULD_YIELD         1
#define configUSE_MUTEXES               1
#define configQUEUE_REGISTRY_SIZE       8
#define configCHECK_FOR_STACK_OVERFLOW  0
#define configUSE_RECURSIVE_MUTEXES     1
#define configUSE_COUNTING_SEMAPHORES   1
#define configUSE_QUEUE_SETS            0
#define configUSE_TASK_NOTIFICATIONS     1
#define configSUPPORT_DYNAMIC_ALLOCATION 1

/* FreeRTOS GCC/Cortex-M4 端口的裸异常处理器必须直接占用这两个向量。 */
#define vPortSVCHandler                 SVC_Handler
#define xPortPendSVHandler               PendSV_Handler

#define configPRIO_BITS                       4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define INCLUDE_vTaskPrioritySet        1
#define INCLUDE_uxTaskPriorityGet       1
#define INCLUDE_vTaskDelete             1
#define INCLUDE_vTaskSuspend            1
#define INCLUDE_xTaskGetSchedulerState  1
#define INCLUDE_vTaskDelayUntil         1
#define INCLUDE_vTaskDelay              1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_xTaskGetIdleTaskHandle   0
#define INCLUDE_xSemaphoreGetMutexClient 0

#define configASSERT(x) if ((x) == 0) { taskDISABLE_INTERRUPTS(); for(;;); }

#endif
