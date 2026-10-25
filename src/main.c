/* SPDX-License-Identifier: MIT */
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "lvgl.h"
#include "simulation.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static simulation_t simulation;
static uint64_t previous;
static uint64_t clock_us(void) {
    const uint64_t counter = SDL_GetPerformanceCounter();
    const uint64_t hz = SDL_GetPerformanceFrequency();
    return (counter / hz) * UINT64_C(1000000) + (counter % hz) * UINT64_C(1000000) / hz;
}
static void update(lv_timer_t *timer) {
    (void)timer;
    const uint64_t now = clock_us();
    simulation_advance(&simulation, now - previous);
    previous = now;
    ui_refresh();
}
static int screenshot(lv_display_t *display, const char *path) {
    lv_refr_now(display);
    SDL_Renderer *renderer = lv_sdl_window_get_renderer(display);
    int w, h;
    if (SDL_GetRendererOutputSize(renderer, &w, &h) != 0) return 1;
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
    if (!surface) return 1;
    const int read_result = SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888,
                                               surface->pixels, surface->pitch);
    const int result = read_result == 0 ? SDL_SaveBMP(surface, path) : -1;
    SDL_FreeSurface(surface);
    return result != 0;
}
#define REQUIRE(x) do { if (!(x)) { fprintf(stderr, "UI smoke failed: %s\n", #x); return 1; } } while (0)
static int smoke_test(void) {
    lv_obj_send_event(ui_control(UI_PLUS), LV_EVENT_CLICKED, NULL);
    REQUIRE(simulation.setpoint == 25.5);
    lv_obj_send_event(ui_control(UI_AUTO), LV_EVENT_CLICKED, NULL);
    REQUIRE(simulation.mode == LOOP_AUTO);
    simulation_advance(&simulation, 100000);
    ui_refresh();
    REQUIRE(simulation.applied > 0 && simulation.applied < simulation.requested);
    lv_obj_send_event(ui_control(UI_PAUSE), LV_EVENT_CLICKED, NULL);
    simulation_advance(&simulation, 1000000);
    ui_refresh();
    REQUIRE(simulation_unavailable(&simulation) && simulation.requested == 0);
    lv_obj_send_event(ui_control(UI_PAUSE), LV_EVENT_CLICKED, NULL);
    simulation_advance(&simulation, 100000);
    ui_refresh();
    REQUIRE(!simulation_unavailable(&simulation));
    lv_obj_send_event(ui_control(UI_DISCONNECT), LV_EVENT_CLICKED, NULL);
    REQUIRE(lv_obj_has_state(ui_control(UI_PLUS), LV_STATE_DISABLED));
    REQUIRE(!simulation_setpoint(&simulation, 30));
    lv_obj_send_event(ui_control(UI_DISCONNECT), LV_EVENT_CLICKED, NULL);
    REQUIRE(!lv_obj_has_state(ui_control(UI_PLUS), LV_STATE_DISABLED));
    lv_obj_send_event(ui_control(UI_RESTART), LV_EVENT_CLICKED, NULL);
    REQUIRE(simulation.mode == LOOP_OFF && simulation.setpoint == 25);
    puts("PASS: LVGL event wiring, mode, setpoint, pause/recovery, disconnect, restart");
    return 0;
}
int main(int argc, char **argv) {
    bool smoke = false;
    const char *image = NULL;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--smoke-test") == 0) smoke = true;
        else if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc) image = argv[++i];
        else {
            fprintf(stderr, "Usage: %s [--smoke-test] [--screenshot output.bmp]\n", argv[0]);
            return strcmp(argv[i], "--help") == 0 ? 0 : 2;
        }
    }
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }
    lv_init();
    lv_display_t *display = lv_sdl_window_create(1100, 900);
    if (!display || !lv_sdl_window_get_renderer(display)) {
        fprintf(stderr, "SDL window/renderer failed: %s\n", SDL_GetError());
        return 1;
    }
    lv_sdl_window_set_title(display, "SameLoop Lab - Bind & Seek / LVGL");
    /* Fixed desktop layout: scroll vertically for smaller displays. */
    lv_sdl_window_set_resizeable(display, false);
    lv_group_t *group = lv_group_create();
    lv_group_set_default(group);
    lv_sdl_mouse_create();
    lv_indev_t *keyboard = lv_sdl_keyboard_create();
    lv_indev_set_group(keyboard, group);
    /* Crown mode routes the wheel through the mouse input device. */
    simulation_init(&simulation);
    ui_create(&simulation);
    if (smoke || image) {
        const int result = smoke ? smoke_test() : 0;
        if (result != 0) return result;
        /* A consistent illustrative frame, distinct from normal OFF startup. */
        simulation_setpoint(&simulation, 28);
        simulation_mode(&simulation, LOOP_AUTO);
        simulation_advance(&simulation, 1000000);
        ui_refresh();
        lv_obj_update_layout(lv_screen_active());
        lv_refr_now(display);
        if (image && screenshot(display, image)) {
            fprintf(stderr, "Screenshot failed: %s\n", SDL_GetError());
            return 1;
        }
        lv_deinit();
        SDL_Quit();
        return 0;
    }
    previous = clock_us();
    lv_timer_create(update, 100, NULL);
    while (lv_display_get_default() != NULL) {
        uint32_t wait = lv_timer_handler();
        if (wait > 10) wait = 10;
        SDL_Delay(wait ? wait : 1);
    }
    lv_deinit();
    SDL_Quit();
    return 0;
}
