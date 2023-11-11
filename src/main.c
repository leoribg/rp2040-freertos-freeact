#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include <assert.h>
#include <stdio.h>
#include "FreeAct.h"
#include "bsp.h"

typedef struct {
  Active super; /* Inherit Active base class */
  TimeEvent te;
} blinkLed;

static void blinkLed_dispatch(blinkLed *const me, Event const *const e) {
    switch (e->sig) {
    case INIT_SIG: {/* intentionally fall through... */
        TimeEvent_arm(&me->te, (200 / portTICK_RATE_MS));
        break;
    }
    case TIMEOUT_SIG: {
        BSP_led0_toggle();
        TimeEvent_arm(&me->te, (200 / portTICK_RATE_MS));
        break;
    }
    }
}

void blinkLed_ctor(blinkLed * const me) {
    Active_ctor(&me->super, (DispatchHandler)&blinkLed_dispatch);
    me->te.type = TYPE_ONE_SHOT;
    TimeEvent_ctor(&me->te, TIMEOUT_SIG, &me->super);
}

static StackType_t blinkLedStack[configMINIMAL_STACK_SIZE];
static Event *blinkLed_queue[10];
static blinkLed blinkled;
Active *AO_blinkLed = &blinkled.super;

int main()
{
    BSP_init();

    blinkLed_ctor(&blinkled);

    Active_start(AO_blinkLed, 1, blinkLed_queue,
                 sizeof(blinkLed_queue) / sizeof(blinkLed_queue[0]),
                 blinkLedStack, sizeof(blinkLedStack), 0);
    
    vTaskStartScheduler();

    while(1){};
}