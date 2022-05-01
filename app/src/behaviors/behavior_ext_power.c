/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define STRIP DT_CHOSEN(zmk_underglow)
#define STRIP_LABEL DT_LABEL(DT_CHOSEN(zmk_underglow))
#define ZMK_DISPLAY_NAME CONFIG_LVGL_DISPLAY_DEV_NAME

#define DT_DRV_COMPAT zmk_behavior_ext_power
#define DEFAULT_POWER_DOMAIN DT_CHOSEN(zmk_default_power_domain)

#include <device.h>
#include <devicetree.h>
#include <drivers/behavior.h>
#include <drivers/ext_power.h>
#include <pm/device.h>

#include <dt-bindings/zmk/ext_power.h>

#include <logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int
on_keymap_binding_convert_central_state_dependent_params(struct zmk_behavior_binding *binding,
                                                         struct zmk_behavior_binding_event event) {
    LOG_ERR("In on_keymap_binding_convert_central_state_dependent_params with param: %d", binding->param1);

    if (binding->param1 == EXT_POWER_TOGGLE_CMD) {

        const struct device *ext_power = DEVICE_DT_GET(DEFAULT_POWER_DOMAIN);
        if (ext_power == NULL) {
            LOG_ERR("Unable to retrieve ext_power device");
            return -EIO;
        }

        enum pm_device_state pm_state;
        if(pm_device_state_get(ext_power, &pm_state) != 0) {
            LOG_ERR("Could not get pm device state for ext power");
            return -EIO;
        }

        binding->param1 = pm_state > 0 ? EXT_POWER_ON_CMD : EXT_POWER_OFF_CMD;
        LOG_ERR("New binding param %d", binding->param1);
    }

    return 0;
}

int print_debug_info_underglow() {
    LOG_ERR("In print_debug_info");

    static const struct device *led_strip;
    led_strip = device_get_binding(STRIP_LABEL);
    if (led_strip) {
        LOG_ERR("Found LED strip device %s", STRIP_LABEL);
    } else {
        LOG_ERR("LED strip device %s not found", STRIP_LABEL);
        return -EINVAL;
    }

    bool on_pd = pm_device_on_power_domain(led_strip);
    LOG_ERR("Underglow on power domain: %d", on_pd);

    enum pm_device_state pm_state;
    if(pm_device_state_get(led_strip, &pm_state) != 0) {
        LOG_ERR("Could not get pm device state for underglow");
        return -EIO;
    } else {
        LOG_ERR("Underglow pm state: %d", pm_state);
    }
}

int print_debug_info_display() {
    LOG_ERR("In print_debug_info");

    static const struct device *display;
    display = device_get_binding(ZMK_DISPLAY_NAME);
    if (display == NULL) {
        LOG_ERR("Failed to find display device: %s", ZMK_DISPLAY_NAME);
        return -EINVAL;
    } else {
        LOG_ERR("Found display device %s", ZMK_DISPLAY_NAME);
    }

    bool on_pd = pm_device_on_power_domain(display);
    LOG_ERR("Display on power domain: %d", on_pd);

    enum pm_device_state pm_state;
    if(pm_device_state_get(display, &pm_state) != 0) {
        LOG_ERR("Could not get pm device state for display");
        return -EIO;
    } else {
        LOG_ERR("Display pm state: %d", pm_state);
    }
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    LOG_ERR("In on_keymap_binding_pressed with param: %d", binding->param1);

    print_debug_info_underglow();
    print_debug_info_display();

    const struct device *ext_power = DEVICE_DT_GET(DEFAULT_POWER_DOMAIN);

    if (ext_power == NULL) {
        LOG_ERR("Unable to retrieve ext_power device");
        return -EIO;
    }

    LOG_ERR("Got ext_power: %p", ext_power);
    enum pm_device_state pm_state;
    if(pm_device_state_get(ext_power, &pm_state) != 0) {
        LOG_ERR("Could not get pm device state for ext_power");
        return -EIO;
    } else {
        LOG_ERR("Ext power pm state: %s", pm_device_state_str(pm_state));
    }

    switch (binding->param1) {
        case EXT_POWER_OFF_CMD:
            LOG_ERR("Running EXT_POWER_OFF_CMD");
            pm_device_action_run(ext_power, PM_DEVICE_ACTION_TURN_OFF);
            break;
        case EXT_POWER_ON_CMD:
            LOG_ERR("Running EXT_POWER_ON_CMD");
            pm_device_action_run(ext_power, PM_DEVICE_ACTION_TURN_ON);
            break;
        default:
            LOG_ERR("Unknown ext_power command: %d", binding->param1);
            return -ENOTSUP;
    }

    if(pm_device_state_get(ext_power, &pm_state) != 0) {
        LOG_ERR("Finished ext power keybind, but could not get pm device state for ext_power");
    } else {
        LOG_ERR("Finished ext power keybind with new state: %s; Lock status: %d; Busy status: %d", pm_device_state_str(pm_state), pm_device_state_is_locked(ext_power), pm_device_is_busy(ext_power));
    }

    return 0;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_ext_power_init(const struct device *dev) {
    LOG_ERR("In behavior_ext_power_init");
    return 0;
};

static const struct behavior_driver_api behavior_ext_power_driver_api = {
    .binding_convert_central_state_dependent_params =
        on_keymap_binding_convert_central_state_dependent_params,
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
};

DEVICE_DT_INST_DEFINE(0, behavior_ext_power_init, NULL, NULL, NULL, APPLICATION,
                      CONFIG_APPLICATION_INIT_PRIORITY, &behavior_ext_power_driver_api);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
