/*****************************************************************************
* Project: FreeAct Example for Raspberry Pi Pico board
* Board: RP2040
*****************************************************************************/
#ifndef BSP_H
#define BSP_H

#include "FreeAct.h"

void BSP_init(void);
void BSP_start(void);
void BSP_led0_on(void);
void BSP_led0_off(void);
void BSP_led0_toggle(void);

enum Signals {
    TIMEOUT_SIG = USER_SIG
};

extern Active *AO_blinkyButton;

#endif /* BSP_H */