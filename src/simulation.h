/* SPDX-License-Identifier: MIT */
#ifndef SAMELOOP_SIMULATION_H
#define SAMELOOP_SIMULATION_H
#include <stdbool.h>
#include <stdint.h>

typedef enum { LOOP_OFF, LOOP_AUTO } loop_mode_t;
typedef struct {
    double temperature, setpoint, requested, applied, rpm, measurement;
    loop_mode_t mode;
    uint64_t now_us, last_valid_us;
    bool has_sample, has_valid, overtemperature;
    bool sensor_valid, paused, connected, heat_boost;
} simulation_t;

void simulation_init(simulation_t *s);
void simulation_advance(simulation_t *s, uint64_t elapsed_us);
bool simulation_unavailable(const simulation_t *s);
bool simulation_setpoint(simulation_t *s, double value);
bool simulation_mode(simulation_t *s, loop_mode_t mode);
void simulation_restart(simulation_t *s);
#endif
