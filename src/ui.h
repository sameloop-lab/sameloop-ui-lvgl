/* SPDX-License-Identifier: MIT */
#ifndef SAMELOOP_UI_H
#define SAMELOOP_UI_H
#include "lvgl.h"
#include "simulation.h"
typedef enum {
    UI_MINUS, UI_PLUS, UI_OFF, UI_AUTO, UI_HEAT, UI_INVALID,
    UI_PAUSE, UI_DISCONNECT, UI_INJECT, UI_RESTART, UI_COUNT
} ui_action_t;
void ui_create(simulation_t *simulation);
void ui_refresh(void);
/* Exposed for event-wiring smoke tests. */
lv_obj_t *ui_control(ui_action_t action);
#endif
