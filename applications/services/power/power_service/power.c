#include "power_i.h"

#include <furi.h>
#include <furi_hal.h>
#include <xtreme.h>

#define POWER_OFF_TIMEOUT 90
#define TAG "Power"

void power_draw_battery_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    Power* power = context;
    BatteryIcon battery_icon = XTREME_SETTINGS()->battery_icon;
    if(battery_icon == BatteryIconOff) return;

    canvas_draw_icon(canvas, 0, 0, &I_Battery_25x8);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, -1, 0, 1, 8);
    canvas_draw_box(canvas, 0, -1, 24, 1);
    canvas_draw_box(canvas, 0, 8, 24, 1);
    canvas_draw_box(canvas, 25, 1, 2, 6);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 25, 2, 1, 4);

    if(power->info.gauge_is_ok) {
        char batteryPercentile[4];
        snprintf(batteryPercentile, sizeof(batteryPercentile), "%d", power->info.charge);

        if((battery_icon == BatteryIconPercent) &&
           (power->state !=
            PowerStateCharging)) { //if display battery percentage, black background white text
            canvas_set_font(canvas, FontBatteryPercent);
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, 1, 1, 22, 6);
            canvas_set_color(canvas, ColorWhite);
            canvas_draw_str_aligned(canvas, 11, 4, AlignCenter, AlignCenter, batteryPercentile);
        } else if(
            (battery_icon == BatteryIconInvertedPercent) &&
            (power->state !=
             PowerStateCharging)) { //if display inverted percentage, white background black text
            canvas_set_font(canvas, FontBatteryPercent);
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_str_aligned(canvas, 11, 4, AlignCenter, AlignCenter, batteryPercentile);
        } else if(
            (battery_icon == BatteryIconRetro3) &&
            (power->state != PowerStateCharging)) { //Retro style segmented display, 3 parts
            if(power->info.charge > 25) {
                canvas_draw_box(canvas, 2, 2, 6, 4);
            }
            if(power->info.charge > 50) {
                canvas_draw_box(canvas, 9, 2, 6, 4);
            }
            if(power->info.charge > 75) {
                canvas_draw_box(canvas, 16, 2, 6, 4);
            }
        } else if(
            (battery_icon == BatteryIconRetro5) &&
            (power->state != PowerStateCharging)) { //Retro style segmented display, 5 parts
            if(power->info.charge > 10) {
                canvas_draw_box(canvas, 2, 2, 3, 4);
            }
            if(power->info.charge > 30) {
                canvas_draw_box(canvas, 6, 2, 3, 4);
            }
            if(power->info.charge > 50) {
                canvas_draw_box(canvas, 10, 2, 3, 4);
            }
            if(power->info.charge > 70) {
                canvas_draw_box(canvas, 14, 2, 3, 4);
            }
            if(power->info.charge > 90) {
                canvas_draw_box(canvas, 18, 2, 3, 4);
            }
        } else if(
            (battery_icon == BatteryIconBarPercent) &&
            (power->state != PowerStateCharging) && // Default bar display with percentage
            (power->info.voltage_battery_charge_limit >=
             4.2)) { // not looking nice with low voltage indicator
            canvas_set_font(canvas, FontBatteryPercent);

            // align charge display value with digits to draw
            uint8_t bar_charge = power->info.charge;
            if(bar_charge > 23 && bar_charge < 38) {
                bar_charge = 23;
            } else if(bar_charge >= 38 && bar_charge < 62) {
                bar_charge = 50;
            } else if(bar_charge >= 62 && bar_charge < 74) {
                bar_charge = 74;
            }

            // drawing digits
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, 1, 1, (bar_charge * 22) / 100, 6);
            if(bar_charge < 38) { // both digits are black
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_str_aligned(
                    canvas, 11, 4, AlignCenter, AlignCenter, batteryPercentile);
            } else if(bar_charge >= 38 && bar_charge < 74) { // first digit is white
                canvas_set_color(canvas, ColorWhite);

                // first
                char batteryPercentileFirstDigit[2];
                snprintf(
                    batteryPercentileFirstDigit,
                    sizeof(batteryPercentileFirstDigit),
                    "%c",
                    batteryPercentile[0]);
                canvas_draw_str_aligned(
                    canvas, 9, 4, AlignCenter, AlignCenter, batteryPercentileFirstDigit);

                // second
                char batteryPercentileSecondDigit[2];
                snprintf(
                    batteryPercentileSecondDigit,
                    sizeof(batteryPercentileSecondDigit),
                    "%c",
                    batteryPercentile[1]);
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_str_aligned(
                    canvas, 15, 4, AlignCenter, AlignCenter, batteryPercentileSecondDigit);
            } else { // charge >= 74, both digits are white
                canvas_set_color(canvas, ColorWhite);
                canvas_draw_str_aligned(
                    canvas, 11, 4, AlignCenter, AlignCenter, batteryPercentile);
            }

        } else { //default bar display, added here to serve as fallback/default behaviour.
            canvas_draw_box(canvas, 2, 2, (power->info.charge + 4) / 5, 4);
        }

        // TODO: Verify if it displays correctly with custom battery skins !!!
        if(power->info.voltage_battery_charge_limit < 4.2) {
            // Battery charging voltage is modified, indicate with cross pattern
            canvas_invert_color(canvas);
            uint8_t battery_bar_width = (power->info.charge + 4) / 5;
            bool cross_odd = false;
            // Start 1 further in from the battery bar's x position
            for(uint8_t x = 3; x <= battery_bar_width; x++) {
                // Cross pattern is from the center of the battery bar
                // y = 2 + 1 (inset) + 1 (for every other)
                canvas_draw_dot(canvas, x, 3 + (uint8_t)cross_odd);
                cross_odd = !cross_odd;
            }
            canvas_invert_color(canvas);
        }

        if(power->state == PowerStateCharging) {
            canvas_set_bitmap_mode(canvas, 1);
            // TODO: replace -1 magic for uint8_t with re-framing
            if(battery_icon == BatteryIconPercent) {
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_box(canvas, 1, 1, 22, 6);
                canvas_draw_icon(canvas, 2, -1, &I_Charging_lightning_9x10);
                canvas_set_color(canvas, ColorWhite);
                canvas_draw_icon(canvas, 2, -1, &I_Charging_lightning_mask_9x10);
                canvas_set_font(canvas, FontBatteryPercent);
                canvas_draw_str_aligned(
                    canvas, 16, 4, AlignCenter, AlignCenter, batteryPercentile);
            } else if(battery_icon == BatteryIconInvertedPercent) {
                canvas_set_color(canvas, ColorWhite);
                canvas_draw_box(canvas, 1, 1, 22, 6);
                canvas_draw_icon(canvas, 2, -1, &I_Charging_lightning_9x10);
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_icon(canvas, 2, -1, &I_Charging_lightning_mask_9x10);
                canvas_set_font(canvas, FontBatteryPercent);
                canvas_draw_str_aligned(
                    canvas, 16, 4, AlignCenter, AlignCenter, batteryPercentile);
            } else if(battery_icon == BatteryIconBarPercent) {
                // clean-up default charging bar display
                canvas_set_color(canvas, ColorWhite);
                canvas_draw_box(canvas, 1, 1, 22, 6);

                // align charge display value with digits to draw
                uint8_t bar_charge = power->info.charge;

                if(bar_charge > 48 && bar_charge < 63) {
                    bar_charge = 48;
                } else if(bar_charge >= 63 && bar_charge < 84) {
                    bar_charge = 75;
                } else if(bar_charge >= 84 && bar_charge < 96) {
                    bar_charge = 96;
                }
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_box(canvas, 1, 1, (bar_charge * 22) / 100, 6);

                // drawing charge icon
                canvas_draw_icon(canvas, 2, -1, &I_Charging_lightning_9x10);
                canvas_set_color(canvas, ColorWhite);
                canvas_draw_icon(canvas, 2, -1, &I_Charging_lightning_mask_9x10);

                // drawing digits
                canvas_set_font(canvas, FontBatteryPercent);
                if(bar_charge < 64) { // both digits are black
                    canvas_set_color(canvas, ColorBlack);
                    canvas_draw_str_aligned(
                        canvas, 16, 4, AlignCenter, AlignCenter, batteryPercentile);
                } else if(bar_charge >= 64 && bar_charge < 84) { // first digit is white
                    canvas_set_color(canvas, ColorWhite);

                    // first
                    char batteryPercentileFirstDigit[2];
                    snprintf(
                        batteryPercentileFirstDigit,
                        sizeof(batteryPercentileFirstDigit),
                        "%c",
                        batteryPercentile[0]);
                    canvas_draw_str_aligned(
                        canvas, 14, 4, AlignCenter, AlignCenter, batteryPercentileFirstDigit);

                    // second
                    char batteryPercentileSecondDigit[2];
                    snprintf(
                        batteryPercentileSecondDigit,
                        sizeof(batteryPercentileSecondDigit),
                        "%c",
                        batteryPercentile[1]);
                    canvas_set_color(canvas, ColorBlack);
                    canvas_draw_str_aligned(
                        canvas, 20, 4, AlignCenter, AlignCenter, batteryPercentileSecondDigit);
                } else { // charge >= 84, both digits are white
                    canvas_set_color(canvas, ColorWhite);
                    canvas_draw_str_aligned(
                        canvas, 16, 4, AlignCenter, AlignCenter, batteryPercentile);
                }
            } else {
                canvas_set_color(canvas, ColorWhite);
                canvas_draw_icon(canvas, 8, -1, &I_Charging_lightning_mask_9x10);
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_icon(canvas, 8, -1, &I_Charging_lightning_9x10);
            }
            canvas_set_bitmap_mode(canvas, 0);
        }
    } else {
        canvas_draw_box(canvas, 8, 3, 8, 2);
    }
}

static ViewPort* power_battery_view_port_alloc(Power* power) {
    ViewPort* battery_view_port = view_port_alloc();
    view_port_set_width(battery_view_port, icon_get_width(&I_Battery_25x8));
    view_port_draw_callback_set(battery_view_port, power_draw_battery_callback, power);
    gui_add_view_port(power->gui, battery_view_port, GuiLayerStatusBarRight);
    return battery_view_port;
}

static void power_start_auto_shutdown_timer(Power* power) {
    furi_timer_start(power->auto_shutdown_timer, furi_ms_to_ticks(power->shutdown_idle_delay_ms));
}

static void power_stop_auto_shutdown_timer(Power* power) {
    furi_timer_stop(power->auto_shutdown_timer);
}

static uint32_t power_is_running_auto_shutdown_timer(Power* power) {
    return furi_timer_is_running(power->auto_shutdown_timer);
}

static void power_input_event_callback(const void* value, void* context) {
    furi_assert(value);
    furi_assert(context);
    const InputEvent* event = value;
    Power* power = context;
    if(event->type == InputTypePress) {
        power_start_auto_shutdown_timer(power);
    }
}

static void power_auto_shutdown_arm(Power* power) {
    if(power->shutdown_idle_delay_ms) {
        if(power->input_events_subscription == NULL) {
            power->input_events_subscription = furi_pubsub_subscribe(
                power->input_events_pubsub, power_input_event_callback, power);
        }
        power_start_auto_shutdown_timer(power);
    }
}

static void power_auto_shutdown_inhibit(Power* power) {
    power_stop_auto_shutdown_timer(power);
    if(power->input_events_subscription) {
        furi_pubsub_unsubscribe(power->input_events_pubsub, power->input_events_subscription);
        power->input_events_subscription = NULL;
    }
}

static void power_loader_callback(const void* message, void* context) {
    furi_assert(context);
    Power* power = context;
    const LoaderEvent* event = message;

    if(event->type == LoaderEventTypeApplicationStarted) {
        power_auto_shutdown_inhibit(power);
    } else if(event->type == LoaderEventTypeApplicationStopped) {
        power_auto_shutdown_arm(power);
    }
}

static void power_auto_shutdown_timer_callback(void* context) {
    furi_assert(context);
    Power* power = context;
    power_auto_shutdown_inhibit(power);
    power_off(power);
}

static void power_shutdown_time_changed_callback(const void* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Power* power = context;
    power->shutdown_idle_delay_ms = *(uint32_t*)event;
    if(power->shutdown_idle_delay_ms) {
        power_auto_shutdown_arm(power);
    } else if(power_is_running_auto_shutdown_timer(power)) {
        power_auto_shutdown_inhibit(power);
    }
}

Power* power_alloc() {
    Power* power = malloc(sizeof(Power));

    // Records
    power->notification = furi_record_open(RECORD_NOTIFICATION);
    power->gui = furi_record_open(RECORD_GUI);

    // Pubsub
    power->event_pubsub = furi_pubsub_alloc();
    power->settings_events = furi_pubsub_alloc();
    power->loader = furi_record_open(RECORD_LOADER);
    power->input_events_pubsub = furi_record_open(RECORD_INPUT_EVENTS);
    power->input_events_subscription = NULL;
    power->app_start_stop_subscription =
        furi_pubsub_subscribe(loader_get_pubsub(power->loader), power_loader_callback, power);
    power->settings_events_subscription =
        furi_pubsub_subscribe(power->settings_events, power_shutdown_time_changed_callback, power);

    power->input_events_pubsub = furi_record_open(RECORD_INPUT_EVENTS);
    power->input_events_subscription = NULL;

    // State initialization
    power->state = PowerStateNotCharging;
    power->battery_low = false;
    power->power_off_timeout = POWER_OFF_TIMEOUT;
    power->api_mtx = furi_mutex_alloc(FuriMutexTypeNormal);

    // Gui
    power->view_dispatcher = view_dispatcher_alloc();
    power->power_off = power_off_alloc();
    view_dispatcher_add_view(
        power->view_dispatcher, PowerViewOff, power_off_get_view(power->power_off));
    power->power_unplug_usb = power_unplug_usb_alloc();
    view_dispatcher_add_view(
        power->view_dispatcher,
        PowerViewUnplugUsb,
        power_unplug_usb_get_view(power->power_unplug_usb));
    view_dispatcher_attach_to_gui(
        power->view_dispatcher, power->gui, ViewDispatcherTypeFullscreen);

    // Battery view port
    power->battery_view_port = power_battery_view_port_alloc(power);
    power->show_low_bat_level_message = true;

    //Auto shutdown timer
    power->auto_shutdown_timer =
        furi_timer_alloc(power_auto_shutdown_timer_callback, FuriTimerTypeOnce, power);

    return power;
}

static void power_check_charging_state(Power* power) {
    if(furi_hal_power_is_charging()) {
        if((power->info.charge == 100) || (furi_hal_power_is_charging_done())) {
            if(power->state != PowerStateCharged) {
                notification_internal_message(power->notification, &sequence_charged);
                power->state = PowerStateCharged;
                power->event.type = PowerEventTypeFullyCharged;
                furi_pubsub_publish(power->event_pubsub, &power->event);
            }
        } else {
            if(power->state != PowerStateCharging) {
                notification_internal_message(power->notification, &sequence_charging);
                power->state = PowerStateCharging;
                power->event.type = PowerEventTypeStartCharging;
                furi_pubsub_publish(power->event_pubsub, &power->event);
            }
        }
    } else {
        if(power->state != PowerStateNotCharging) {
            notification_internal_message(power->notification, &sequence_not_charging);
            power->state = PowerStateNotCharging;
            power->event.type = PowerEventTypeStopCharging;
            furi_pubsub_publish(power->event_pubsub, &power->event);
        }
    }
}

static bool power_update_info(Power* power) {
    PowerInfo info;

    info.is_charging = furi_hal_power_is_charging();
    info.gauge_is_ok = furi_hal_power_gauge_is_ok();
    info.charge = furi_hal_power_get_pct();
    info.health = furi_hal_power_get_bat_health_pct();
    info.capacity_remaining = furi_hal_power_get_battery_remaining_capacity();
    info.capacity_full = furi_hal_power_get_battery_full_capacity();
    info.current_charger = furi_hal_power_get_battery_current(FuriHalPowerICCharger);
    info.current_gauge = furi_hal_power_get_battery_current(FuriHalPowerICFuelGauge);
    info.voltage_battery_charge_limit = furi_hal_power_get_battery_charge_voltage_limit();
    info.voltage_charger = furi_hal_power_get_battery_voltage(FuriHalPowerICCharger);
    info.voltage_gauge = furi_hal_power_get_battery_voltage(FuriHalPowerICFuelGauge);
    info.voltage_vbus = furi_hal_power_get_usb_voltage();
    info.temperature_charger = furi_hal_power_get_battery_temperature(FuriHalPowerICCharger);
    info.temperature_gauge = furi_hal_power_get_battery_temperature(FuriHalPowerICFuelGauge);

    furi_mutex_acquire(power->api_mtx, FuriWaitForever);
    bool need_refresh = power->info.charge != info.charge;
    need_refresh |= power->info.is_charging != info.is_charging;
    power->info = info;
    furi_mutex_release(power->api_mtx);

    return need_refresh;
}

static void power_check_low_battery(Power* power) {
    if(!power->info.gauge_is_ok) {
        return;
    }

    // Check battery charge and vbus voltage
    if((power->info.charge == 0) && (power->info.voltage_vbus < 4.0f) &&
       power->show_low_bat_level_message) {
        if(!power->battery_low) {
            view_dispatcher_send_to_front(power->view_dispatcher);
            view_dispatcher_switch_to_view(power->view_dispatcher, PowerViewOff);
        }
        power->battery_low = true;
    } else {
        if(power->battery_low) {
            view_dispatcher_switch_to_view(power->view_dispatcher, VIEW_NONE);
            power->power_off_timeout = POWER_OFF_TIMEOUT;
        }
        power->battery_low = false;
    }
    // If battery low, update view and switch off power after timeout
    if(power->battery_low) {
        PowerOffResponse response = power_off_get_response(power->power_off);
        if(response == PowerOffResponseDefault) {
            if(power->power_off_timeout) {
                power_off_set_time_left(power->power_off, power->power_off_timeout--);
            } else {
                power_off(power);
            }
        } else if(response == PowerOffResponseOk) {
            power_off(power);
        } else if(response == PowerOffResponseHide) {
            view_dispatcher_switch_to_view(power->view_dispatcher, VIEW_NONE);
            if(power->power_off_timeout) {
                power_off_set_time_left(power->power_off, power->power_off_timeout--);
            } else {
                power_off(power);
            }
        } else if(response == PowerOffResponseCancel) {
            view_dispatcher_switch_to_view(power->view_dispatcher, VIEW_NONE);
        }
    }
}

static void power_check_battery_level_change(Power* power) {
    if(power->battery_level != power->info.charge) {
        power->battery_level = power->info.charge;
        power->event.type = PowerEventTypeBatteryLevelChanged;
        power->event.data.battery_level = power->battery_level;
        furi_pubsub_publish(power->event_pubsub, &power->event);
    }
}

static void power_check_charge_cap(Power* power) {
    if(power->info.charge >= XTREME_SETTINGS()->charge_cap) {
        if(!power->info.is_charge_capped) { // Suppress charging if charge reaches custom cap
            power->info.is_charge_capped = true;
            furi_hal_power_suppress_charge_enter();
        }
    } else {
        if(power->info.is_charge_capped) { // Start charging again if charge below custom cap
            power->info.is_charge_capped = false;
            furi_hal_power_suppress_charge_exit();
        }
    }
}

void power_trigger_ui_update(Power* power) {
    view_port_update(power->battery_view_port);
}

int32_t power_srv(void* p) {
    UNUSED(p);

    if(!furi_hal_is_normal_boot()) {
        FURI_LOG_W(TAG, "Skipping start in special boot mode");
        return 0;
    }

    Power* power = power_alloc();
    if(!LOAD_POWER_SETTINGS(&power->shutdown_idle_delay_ms)) {
        power->shutdown_idle_delay_ms = 0;
        SAVE_POWER_SETTINGS(&power->shutdown_idle_delay_ms);
    }
    power_auto_shutdown_arm(power);
    power_update_info(power);
    power->info.is_charge_capped = false; // default false
    furi_record_create(RECORD_POWER, power);

    while(1) {
        // Update data from gauge and charger
        bool need_refresh = power_update_info(power);

        // Check low battery level
        power_check_low_battery(power);

        // Check and notify about charging state
        power_check_charging_state(power);

        // Check and notify about battery level change
        power_check_battery_level_change(power);

        // Check charge cap, compare with user setting and suppress/unsuppress charging
        power_check_charge_cap(power);

        // Update battery view port
        if(need_refresh) {
            view_port_update(power->battery_view_port);
        }

        // Check OTG status and disable it in case of fault
        if(furi_hal_power_is_otg_enabled()) {
            furi_hal_power_check_otg_status();
        }

        furi_delay_ms(1000);
    }

    furi_crash("That was unexpected");

    return 0;
}
