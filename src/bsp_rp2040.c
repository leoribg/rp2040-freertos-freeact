/*****************************************************************************
* Project: FreeAct Example for Raspberry Pi Pico board
* Board: RP2040
*****************************************************************************/

#include "FreeAct.h" /* Free Active Object interface */
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <assert.h>
#include "bsp.h"
/* add other drivers if necessary... */

/* LEDs and Push-buttons on the EMF32-SLSTK3401A board ---------------------*/
const uint LED_PIN = PICO_DEFAULT_LED_PIN;

/* Function Prototype ======================================================*/
void vApplicationTickHook(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize);
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize);

/* Hooks ===================================================================*/
/* Application hooks used in this project ==================================*/
/* NOTE: only the "FromISR" API variants are allowed in vApplicationTickHook*/
void vApplicationTickHook(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* notify FreeRTOS to perform context switch from ISR, if needed */
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
/*..........................................................................*/
void vApplicationIdleHook(void) {
#ifdef NDEBUG
    /* Put the CPU and peripherals to the low-power mode.
    * you might need to customize the clock management for your application,
    * see the datasheet for your particular Cortex-M3 MCU.
    */
#endif
}
/*..........................................................................*/
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    /* ERROR!!! */
}
/*..........................................................................*/
/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must
 * provide an implementation of vApplicationGetIdleTaskMemory() to provide
 * the memory that is used by the Idle task.
 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside
     * this function then they must be declared static - otherwise they will
     * be allocated on the stack and so not exists after this function exits.
     */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t  uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    /* Pass out a pointer to the StaticTask_t structure in which the
     * Idle task's state will be stored.
     */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = &uxIdleTaskStack[0];

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes.
     */
    *pulIdleTaskStackSize = sizeof(uxIdleTaskStack) / sizeof(uxIdleTaskStack[0]);
}

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must
 * provide an implementation of vApplicationGetTimerTaskMemory() to provide
 * the memory that is used by the Timer task.
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
    /* If the buffers to be provided to the Timer task are declared inside
     * this function then they must be declared static - otherwise they will
     * be allocated on the stack and so not exists after this function exits.
     */
    static StaticTask_t xTimerTask_TCB;
    static StackType_t  uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    /* Pass out a pointer to the StaticTask_t structure in which the
     * Timer task's state will be stored.
     */
    *ppxTimerTaskTCBBuffer   = &xTimerTask_TCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = &uxTimerTaskStack[0];

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configTIMER_TASK_STACK_DEPTH is specified in words, not bytes.
     */
    *pulTimerTaskStackSize   = (uint32_t)configTIMER_TASK_STACK_DEPTH;
}

/* BSP functions ===========================================================*/
void BSP_init(void) {
    /* NOTE: SystemInit() already called from the startup code
    *  but SystemCoreClock needs to be updated
    */
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    BSP_led0_on();

    stdio_init_all();
}
/*..........................................................................*/
void BSP_led0_off(void) {
    gpio_put(LED_PIN, 0);
}
/*..........................................................................*/
void BSP_led0_on(void) {
    gpio_put(LED_PIN, 1);
}

void BSP_led0_toggle(void) {
    bool level = gpio_get_out_level(LED_PIN);
    gpio_put(LED_PIN, !level);
}
/*..........................................................................*/
void BSP_start(void) {
    /* set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate */
    /* SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC); done in FreeRTOS */

    /* assign all priority bits for preemption-prio. and none to sub-prio. */
    /* ... */

    /* enable IRQs... */
    /* ... */
}
/*..........................................................................*/
/* error-handling function called by exception handlers in the startup code */
void assert_failed(char const *module, int loc); /* prototype */
void assert_failed(char const *module, int loc) {
    /* NOTE: add here your application-specific error handling */
    (void)module;
    (void)loc;
#ifndef NDEBUG /* debug build? */
    /* light-up both LEDs */
    gpio_put(LED_PIN, 1);

    /* tie the CPU in this endless loop and wait for the debugger... */
    while (1) {
    }
#else /* production build */
    /* TODO: do whatever is necessary to put the system in a fail-safe state */
    /* important!!! */
    NVIC_SystemReset(); /* reset the CPU */
#endif
}

/*****************************************************************************
* NOTE1:
* Only ISRs prioritized at or below the
* configMAX_SYSCALL_INTERRUPT_PRIORITY level (i.e.,
* with the numerical values of priorities equal or higher than
* configMAX_SYSCALL_INTERRUPT_PRIORITY) are allowed to call any
* QP/FreeRTOS services. These ISRs are "kernel-aware".
*
* Only ISRs prioritized at or below the configMAX_SYSCALL_INTERRUPT_PRIORITY
* level (i.e., with the numerical values of priorities equal or higher than
* configMAX_SYSCALL_INTERRUPT_PRIORITY) are allowed to call any QF services.
* These ISRs are "kernel-aware".
*
* Conversely, any ISRs prioritized above the
* configMAX_SYSCALL_INTERRUPT_PRIORITY priority level (i.e., with
* the numerical values of priorities less than
* configMAX_SYSCALL_INTERRUPT_PRIORITY) are never disabled and are
* not aware of the kernel. Such "kernel-unaware" ISRs cannot call any
* QP/FreeRTOS services. The only mechanism by which a "kernel-unaware" ISR
* can communicate with the QF framework is by triggering a "kernel-aware"
* ISR, which can post/publish events.
*
* For more information, see article "Running the RTOS on a ARM Cortex-M Core"
* http://www.freertos.org/RTOS-Cortex-M3-M4.html
*/