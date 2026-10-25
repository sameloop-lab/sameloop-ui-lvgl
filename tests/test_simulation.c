/* SPDX-License-Identifier: MIT */
#include "simulation.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); return EXIT_FAILURE; } } while (0)
int main(void) {
    simulation_t s;
    simulation_init(&s);
    CHECK(s.mode == LOOP_OFF && s.setpoint == 25 && !s.has_sample);
    CHECK(simulation_unavailable(&s));
    simulation_advance(&s, 100000);
    CHECK(s.has_sample && s.measurement == 32.4 && !simulation_unavailable(&s));
    CHECK(!simulation_setpoint(&s, NAN));
    CHECK(!simulation_setpoint(&s, INFINITY));
    CHECK(!simulation_setpoint(&s, 14));
    CHECK(!simulation_setpoint(&s, 41));
    CHECK(!simulation_mode(&s, (loop_mode_t)42));
    CHECK(s.setpoint == 25);
    CHECK(simulation_mode(&s, LOOP_AUTO));
    simulation_advance(&s, 100000);
    CHECK(s.requested == 100 && s.applied > 0 && s.applied < 100);
    s.paused = true;
    simulation_advance(&s, 999000);
    CHECK(!simulation_unavailable(&s));
    simulation_advance(&s, 1000);
    CHECK(simulation_unavailable(&s) && s.requested == 0);
    s.paused = false;
    simulation_advance(&s, 100000);
    CHECK(!simulation_unavailable(&s) && s.requested > 0);
    s.temperature = 55;
    simulation_advance(&s, 100000);
    CHECK(s.overtemperature);
    s.sensor_valid = false;
    simulation_advance(&s, 100000);
    CHECK(simulation_unavailable(&s) && s.requested == 0 && s.overtemperature);
    s.connected = false;
    CHECK(!simulation_setpoint(&s, 28));
    CHECK(!simulation_mode(&s, LOOP_OFF));
    CHECK(s.setpoint == 25 && s.mode == LOOP_AUTO);
    s.connected = true;
    simulation_restart(&s);
    CHECK(s.mode == LOOP_OFF && s.setpoint == 25 && s.requested == 0);
    simulation_init(&s);
    CHECK(simulation_mode(&s, LOOP_AUTO));
    simulation_advance(&s, 30000000);
    CHECK(s.measurement < 30);
    CHECK(simulation_mode(&s, LOOP_OFF) && s.requested == 0);
    simulation_t a, b;
    simulation_init(&a); simulation_init(&b);
    simulation_mode(&a, LOOP_AUTO); simulation_mode(&b, LOOP_AUTO);
    simulation_advance(&a, 10000000);
    for (int i = 0; i < 100; ++i) simulation_advance(&b, 100000);
    CHECK(fabs(a.temperature - b.temperature) < 1e-12);
    puts("PASS: startup, validation, actuation, timeout, recovery, alarms, disconnect, restart, cooling, repeatability");
    return EXIT_SUCCESS;
}
