/* SPDX-License-Identifier: MIT */
#include "ui.h"
#include <math.h>
#include <stdint.h>

static simulation_t *sim;
static lv_obj_t *controls[UI_COUNT];
static lv_obj_t *temperature, *quality, *target, *setpoint, *requested, *applied, *rpm;
static lv_obj_t *sensor_status, *age_status, *alarm_status, *link_status, *progress, *banner;
static const uint32_t BLUE = 0x1762f5, INK = 0x10244a, MUTED = 0x65758e;
static const uint32_t GREEN = 0x14835c, ORANGE = 0xb75c00;

static lv_obj_t *label(lv_obj_t *parent, const char *text, const lv_font_t *font, uint32_t color) {
    lv_obj_t *o = lv_label_create(parent);
    lv_label_set_text(o, text);
    lv_obj_set_style_text_font(o, font, 0);
    lv_obj_set_style_text_color(o, lv_color_hex(color), 0);
    return o;
}
static lv_obj_t *box(lv_obj_t *parent, int width) {
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_set_width(o, width);
    lv_obj_set_height(o, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(o, lv_color_white(), 0);
    lv_obj_set_style_border_color(o, lv_color_hex(0xe4eaf4), 0);
    lv_obj_set_style_border_width(o, 1, 0);
    lv_obj_set_style_radius(o, 16, 0);
    lv_obj_set_style_pad_all(o, 20, 0);
    lv_obj_set_style_pad_row(o, 14, 0);
    lv_obj_set_flex_flow(o, LV_FLEX_FLOW_COLUMN);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    return o;
}
static lv_obj_t *row(lv_obj_t *parent) {
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_width(o, lv_pct(100));
    lv_obj_set_height(o, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(o, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_column(o, 16, 0);
    lv_obj_set_style_pad_row(o, 12, 0);
    lv_obj_set_flex_align(o, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    return o;
}
static void click(lv_event_t *e) {
    const ui_action_t id = (ui_action_t)(intptr_t)lv_event_get_user_data(e);
    switch (id) {
    case UI_MINUS: simulation_setpoint(sim, sim->setpoint - 0.5); break;
    case UI_PLUS: simulation_setpoint(sim, sim->setpoint + 0.5); break;
    case UI_OFF: simulation_mode(sim, LOOP_OFF); break;
    case UI_AUTO: simulation_mode(sim, LOOP_AUTO); break;
    case UI_HEAT: sim->heat_boost = !sim->heat_boost; break;
    case UI_INVALID: sim->sensor_valid = !sim->sensor_valid; break;
    case UI_PAUSE: sim->paused = !sim->paused; break;
    case UI_DISCONNECT: sim->connected = !sim->connected; break;
    case UI_INJECT: sim->temperature = 55; break;
    case UI_RESTART: simulation_restart(sim); break;
    default: break;
    }
    ui_refresh();
}
static lv_obj_t *button(lv_obj_t *parent, const char *text, ui_action_t id, int width) {
    lv_obj_t *o = lv_button_create(parent);
    controls[id] = o;
    lv_obj_remove_style_all(o);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    lv_obj_set_style_outline_width(o, 2, LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_color(o, lv_color_hex(BLUE), LV_STATE_FOCUS_KEY);
    lv_obj_set_size(o, width, 48);
    lv_obj_set_style_bg_color(o, lv_color_hex(0xe9f0ff), 0);
    lv_obj_set_style_bg_color(o, lv_color_hex(BLUE), LV_STATE_CHECKED);
    lv_obj_set_style_text_color(o, lv_color_hex(INK), 0);
    lv_obj_set_style_text_color(o, lv_color_white(), LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(o, 0, 0);
    lv_obj_set_style_radius(o, 10, 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_40, LV_STATE_DISABLED);
    lv_obj_t *l = lv_label_create(o);
    lv_label_set_text(l, text);
    lv_obj_center(l);
    lv_obj_add_event_cb(o, click, LV_EVENT_CLICKED, (void *)(intptr_t)id);
    return o;
}
static void selected(ui_action_t id, bool value) {
    if (value) lv_obj_add_state(controls[id], LV_STATE_CHECKED);
    else lv_obj_remove_state(controls[id], LV_STATE_CHECKED);
}
static void disabled(ui_action_t id, bool value) {
    if (value) lv_obj_add_state(controls[id], LV_STATE_DISABLED);
    else lv_obj_remove_state(controls[id], LV_STATE_DISABLED);
}
static lv_obj_t *metric(lv_obj_t *parent, const char *title) {
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_size(o, 240, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(o, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(o, 8, 0);
    label(o, title, &lv_font_montserrat_14, MUTED);
    return label(o, "--", &lv_font_montserrat_28, INK);
}
static lv_obj_t *status(lv_obj_t *parent, const char *title) {
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_size(o, 220, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(o, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(o, 8, 0);
    label(o, title, &lv_font_montserrat_14, MUTED);
    return label(o, "Unknown", &lv_font_montserrat_16, MUTED);
}
void ui_create(simulation_t *simulation) {
    sim = simulation;
    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xf3f6fb), 0);
    lv_obj_set_style_pad_all(screen, 24, 0);
    lv_obj_set_style_pad_row(screen, 18, 0);
    lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(screen, LV_DIR_VER);
    lv_obj_t *header = row(screen);
    label(header, "SameLoop Lab", &lv_font_montserrat_28, INK);
    label(header, "Bind & Seek", &lv_font_montserrat_20, MUTED);
    label(header, "SIMULATION / LVGL", &lv_font_montserrat_16, BLUE);
    label(screen, "Local backend  |  Planned: redpesk / AGL / OpenHarmony / webOS",
          &lv_font_montserrat_14, MUTED);
    banner = label(screen, "", &lv_font_montserrat_16, ORANGE);
    lv_obj_add_flag(banner, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *top = row(screen);
    lv_obj_t *temp_card = box(top, lv_pct(48));
    label(temp_card, "Chamber temperature", &lv_font_montserrat_20, INK);
    temperature = label(temp_card, "-- °C", &lv_font_montserrat_48, INK);
    quality = label(temp_card, "Waiting for first sample", &lv_font_montserrat_14, MUTED);
    label(temp_card, "Target / accepted setpoint", &lv_font_montserrat_14, MUTED);
    target = label(temp_card, "25.0 °C", &lv_font_montserrat_28, BLUE);

    lv_obj_t *settings = box(top, lv_pct(48));
    label(settings, "Temperature setpoint", &lv_font_montserrat_20, INK);
    lv_obj_t *steps = row(settings);
    button(steps, "-", UI_MINUS, 52);
    setpoint = label(steps, "25.0 °C", &lv_font_montserrat_28, INK);
    button(steps, "+", UI_PLUS, 52);
    label(settings, "15-40 °C / 0.5 °C steps", &lv_font_montserrat_14, MUTED);
    lv_obj_t *modes = row(settings);
    button(modes, "OFF", UI_OFF, 150);
    button(modes, "AUTO", UI_AUTO, 150);

    lv_obj_t *fan = box(screen, lv_pct(100));
    label(fan, "Cooling fan / feedback", &lv_font_montserrat_20, INK);
    lv_obj_t *metrics = row(fan);
    requested = metric(metrics, "Requested duty");
    applied = metric(metrics, "Applied duty");
    rpm = metric(metrics, "Measured speed");
    progress = lv_bar_create(fan);
    lv_obj_set_size(progress, lv_pct(100), 8);
    lv_obj_set_style_bg_color(progress, lv_color_hex(BLUE), LV_PART_INDICATOR);

    lv_obj_t *health = box(screen, lv_pct(100));
    label(health, "System status", &lv_font_montserrat_20, INK);
    lv_obj_t *statuses = row(health);
    sensor_status = status(statuses, "Sensor / availability");
    age_status = status(statuses, "Data freshness");
    alarm_status = status(statuses, "Overtemperature");
    link_status = status(statuses, "Simulator link");

    lv_obj_t *tools = box(screen, lv_pct(100));
    label(tools, "Simulation controls", &lv_font_montserrat_20, INK);
    label(tools, "Selected blue buttons indicate active faults. No physical device is connected.",
          &lv_font_montserrat_14, MUTED);
    lv_obj_t *faults = row(tools);
    button(faults, "Extra heat", UI_HEAT, 175);
    button(faults, "Invalid sensor", UI_INVALID, 175);
    button(faults, "Pause samples", UI_PAUSE, 175);
    button(faults, "Disconnect", UI_DISCONNECT, 175);
    lv_obj_t *actions = row(tools);
    button(actions, "Inject 55 °C", UI_INJECT, 175);
    button(actions, "Restart controller", UI_RESTART, 220);
    label(screen, "Same Control Loop, Different Embedded Stacks", &lv_font_montserrat_14, MUTED);
    ui_refresh();
}
void ui_refresh(void) {
    const bool fresh = sim->connected && !simulation_unavailable(sim);
    if (sim->connected && sim->has_sample) lv_label_set_text_fmt(temperature, "%.1f °C", sim->measurement);
    else lv_label_set_text(temperature, "-- °C");
    lv_obj_set_style_text_color(temperature, lv_color_hex(fresh ? INK : MUTED), 0);
    lv_label_set_text(quality, fresh ? "Live measurement" : "Unavailable / last known sample");
    lv_label_set_text_fmt(target, "%.1f °C", sim->setpoint);
    lv_label_set_text_fmt(setpoint, "%.1f °C", sim->setpoint);
    if (sim->connected) lv_obj_add_flag(banner, LV_OBJ_FLAG_HIDDEN);
    else {
        lv_obj_remove_flag(banner, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(banner, "Disconnected: readings unavailable; mode and setpoint are last known.");
    }
    if (sim->connected) lv_label_set_text_fmt(requested, "%.0f %%", sim->requested);
    else lv_label_set_text(requested, "--");
    if (sim->connected && sim->has_sample) {
        lv_label_set_text_fmt(applied, "%.0f %%", sim->applied);
        lv_label_set_text_fmt(rpm, "%ld rpm", lround(sim->rpm));
        lv_bar_set_value(progress, (int32_t)lround(sim->applied), LV_ANIM_OFF);
        lv_obj_remove_flag(progress, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_label_set_text(applied, "--"); lv_label_set_text(rpm, "--");
        lv_obj_add_flag(progress, LV_OBJ_FLAG_HIDDEN);
    }
    const char *sensor = !sim->connected || !sim->has_sample ? "Unknown / unavailable" :
                         !sim->sensor_valid ? "Invalid / unavailable" :
                         fresh ? "Valid / available" : "Valid / stale";
    lv_label_set_text(sensor_status, sensor);
    lv_obj_set_style_text_color(sensor_status, lv_color_hex(fresh ? GREEN : ORANGE), 0);
    if (sim->connected && sim->has_valid)
        lv_label_set_text_fmt(age_status, "%llu ms%s", (unsigned long long)((sim->now_us - sim->last_valid_us) / 1000), fresh ? "" : " / stale");
    else lv_label_set_text(age_status, "Unavailable");
    lv_label_set_text(alarm_status, !fresh ? "Unknown" : sim->overtemperature ? "ACTIVE" : "Inactive");
    lv_obj_set_style_text_color(alarm_status, lv_color_hex(!fresh ? MUTED : sim->overtemperature ? ORANGE : GREEN), 0);
    lv_label_set_text(link_status, sim->connected ? "Connected" : "Disconnected");
    selected(UI_OFF, sim->mode == LOOP_OFF); selected(UI_AUTO, sim->mode == LOOP_AUTO);
    selected(UI_HEAT, sim->heat_boost); selected(UI_INVALID, !sim->sensor_valid);
    selected(UI_PAUSE, sim->paused); selected(UI_DISCONNECT, !sim->connected);
    disabled(UI_MINUS, !sim->connected || sim->setpoint <= 15);
    disabled(UI_PLUS, !sim->connected || sim->setpoint >= 40);
    disabled(UI_OFF, !sim->connected); disabled(UI_AUTO, !sim->connected);
}
lv_obj_t *ui_control(ui_action_t action) {
    return action >= 0 && action < UI_COUNT ? controls[action] : NULL;
}
