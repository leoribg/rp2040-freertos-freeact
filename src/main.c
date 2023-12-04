#include "FreeRTOS.h"
#include "task.h"
#include <assert.h>
#include <stdio.h>
#include "FreeAct.h"
#include "bsp.h"

typedef struct {
  Active super; /* Inherit Active base class */
  TimeEvent te;
} blinkLed;

typedef struct {
  Active super; /* Inherit Active base class */
  bool isLedBlinking;
} button;

static StackType_t buttonStack[configMINIMAL_STACK_SIZE];
static Event *button_queue[10];
static button Button;
Active *AO_button = &Button.super;

static StackType_t blinkLedStack[configMINIMAL_STACK_SIZE];
static Event *blinkLed_queue[10];
static blinkLed blinkled;
Active *AO_blinkLed = &blinkled.super;

static void button_dispatch(button *const me, Event const *const e) {
    switch (e->sig) {
    case INIT_SIG: {/* intentionally fall through... */
        break;
    }
    case BUTTON_PRESSED_SIG: {
        me->isLedBlinking = !me->isLedBlinking;
        if(me->isLedBlinking) {
            static Event const resumeLedEvt = {RESUME_LED_SIG};
            Active_post(AO_blinkLed, &resumeLedEvt);
        }
        else { 
            static Event const stopLedEvt = {STOP_LED_SIG};
            Active_post(AO_blinkLed, &stopLedEvt);
        }
        break;
    }
    }
}

static void blinkLed_dispatch(blinkLed *const me, Event const *const e) {
    switch (e->sig) {
    case INIT_SIG: { /* intentionally fall through... */
        TimeEvent_arm(&me->te, (200 / portTICK_RATE_MS));
        break;
    }
    case TIMEOUT_SIG: {
        BSP_led0_toggle();
        TimeEvent_arm(&me->te, (200 / portTICK_RATE_MS));
        break;
    }
    case RESUME_LED_SIG: {
        TimeEvent_arm(&me->te, (200 / portTICK_RATE_MS));
        break;
    }
    case STOP_LED_SIG: {
        BSP_led0_on();
        TimeEvent_disarm(&me->te);
        break;
    }
    }
}

void button_ctor(button * const me) {
    Active_ctor(&me->super, (DispatchHandler)&button_dispatch);
    me->isLedBlinking = true;
}

void blinkLed_ctor(blinkLed * const me) {
    Active_ctor(&me->super, (DispatchHandler)&blinkLed_dispatch);
    me->te.type = TYPE_ONE_SHOT;
    TimeEvent_ctor(&me->te, TIMEOUT_SIG, &me->super);
}

int main()
{
    BSP_init();

    button_ctor(&Button);
    blinkLed_ctor(&blinkled);

    Active_start(AO_blinkLed, 1, blinkLed_queue,
                 sizeof(blinkLed_queue) / sizeof(blinkLed_queue[0]),
                 blinkLedStack, sizeof(blinkLedStack), 0);

    Active_start(AO_button, 2, button_queue,
                 sizeof(button_queue) / sizeof(button_queue[0]),
                 buttonStack, sizeof(buttonStack), 0);

    vTaskStartScheduler();

    while(1){};
}