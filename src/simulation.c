/* SPDX-License-Identifier: MIT */
#include "simulation.h"
#include <math.h>
#include <string.h>

static double clamp(double v, double low, double high) {
    return fmax(low, fmin(high, v));
}
void simulation_init(simulation_t *s) {
    memset(s, 0, sizeof(*s));
    s->temperature = 32.4;
    s->setpoint = 25;
    s->sensor_valid = true;
    s->connected = true;
}
bool simulation_unavailable(const simulation_t *s) {
    return !s->sensor_valid || !s->has_valid ||
           s->now_us - s->last_valid_us >= UINT64_C(1000000);
}
bool simulation_setpoint(simulation_t *s, double v) {
    if (!s->connected || !isfinite(v) || v < 15 || v > 40) return false;
    s->setpoint = v;
    return true;
}
bool simulation_mode(simulation_t *s, loop_mode_t mode) {
    if (!s->connected || (mode != LOOP_OFF && mode != LOOP_AUTO)) return false;
    s->mode = mode;
    if (mode == LOOP_OFF) s->requested = 0;
    return true;
}
void simulation_restart(simulation_t *s) {
    s->mode = LOOP_OFF;
    s->setpoint = 25;
    s->requested = 0;
}
void simulation_advance(simulation_t *s, uint64_t elapsed_us) {
    while (elapsed_us) {
        const uint64_t step = elapsed_us > 100000 ? 100000 : elapsed_us;
        elapsed_us -= step;
        s->now_us += step;
        const double dt = (double)step / 1000000.0;
        if (!s->paused) {
            s->measurement = s->temperature;
            s->has_sample = true;
            if (s->sensor_valid) {
                s->last_valid_us = s->now_us;
                s->has_valid = true;
                s->overtemperature = s->measurement > 50;
            }
        }
        s->requested = s->mode == LOOP_AUTO && !simulation_unavailable(s)
                       ? clamp(20 * (s->measurement - s->setpoint), 0, 100) : 0;
        s->applied += (s->requested - s->applied) * (1 - exp(-dt / 0.6));
        s->rpm += (40 * s->applied - s->rpm) * (1 - exp(-dt / 0.8));
        const double heat = s->heat_boost ? 1.2 : 0.18;
        const double loss = (0.015 + 0.12 * s->applied / 100) * (s->temperature - 20);
        s->temperature = clamp(s->temperature + dt * (heat - loss), -40, 150);
    }
}
