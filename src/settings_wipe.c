// SPDX-License-Identifier: MIT
//
// Settings wipe firmware (settings-wipe snippet). Hold the two outermost keys
// of this half's top row for 5 s to erase its settings partition: Bluetooth
// pairings, the split partner, and ZMK Studio edits. Use it when the
// settings_reset firmware leaves old pairings behind. Progress goes to the USB
// log; the keymap is empty, so no key types anything. Flash the normal
// firmware afterwards.

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/settings.h>

LOG_MODULE_REGISTER(settings_wipe, LOG_LEVEL_INF);

// must match ZMK's reset_settings_nvs.c
#if DT_HAS_CHOSEN(zephyr_settings_partition)
#define SETTINGS_PARTITION DT_FIXED_PARTITION_ID(DT_CHOSEN(zephyr_settings_partition))
#else
#define SETTINGS_PARTITION FIXED_PARTITION_ID(storage_partition)
#endif

// The two outermost keys of the top row (matrix row 1). Their positions
// depend on the physical layout the half has selected (size_s, size_m,
// size_l), so accept the pair for each.
#if IS_ENABLED(CONFIG_SHIELD_TORABO_TSUKI_LP_LEFT)
#define HALF "left"
#define KEYS "two top-left keys"
static const uint32_t wipe_pairs[][2] = {{0, 1}, {12, 13}};
#else
#define HALF "right"
#define KEYS "two top-right keys"
static const uint32_t wipe_pairs[][2] = {{8, 9}, {10, 11}, {22, 23}};
#endif

#define HOLD_MS 5000
#define REPORT_INTERVAL_MS 5000

static bool held[ARRAY_SIZE(wipe_pairs)][2];
static bool wiped;
static int erase_rc;
static struct k_work_delayable wipe_work;
static struct k_work_delayable report_work;

// Number of bytes in the partition that are not erased (0xFF), or a negative
// error code.
static int count_used_bytes(void) {
    const struct flash_area *fa;
    uint8_t buf[256];
    int used = 0;

    int rc = flash_area_open(SETTINGS_PARTITION, &fa);
    if (rc) {
        return rc;
    }

    for (off_t off = 0; off < fa->fa_size; off += sizeof(buf)) {
        rc = flash_area_read(fa, off, buf, sizeof(buf));
        if (rc) {
            flash_area_close(fa);
            return rc;
        }
        for (size_t i = 0; i < sizeof(buf); i++) {
            if (buf[i] != 0xFF) {
                used++;
            }
        }
    }

    flash_area_close(fa);
    return used;
}

static void report_work_handler(struct k_work *work) {
    int used = count_used_bytes();

    if (!wiped) {
        LOG_INF("WIPE FIRMWARE (" HALF "): hold the " KEYS " for 5 s to erase. "
                "Settings in use: %d bytes",
                used);
    } else if (erase_rc == 0 && used == 0) {
        LOG_INF("WIPE DONE (" HALF "): settings are empty. Flash the normal firmware now.");
    } else {
        LOG_ERR("WIPE FAILED (" HALF "): erase returned %d, %d bytes still in use", erase_rc,
                used);
    }

    k_work_schedule(&report_work, K_MSEC(REPORT_INTERVAL_MS));
}

static void wipe_work_handler(struct k_work *work) {
    // Stop Bluetooth first, so a reconnecting host or half can't write a
    // new pairing back after the erase.
    int rc = bt_disable();
    if (rc) {
        LOG_WRN("Bluetooth did not stop (%d), erasing anyway", rc);
    }

    erase_rc = zmk_settings_erase();
    wiped = true;
    LOG_INF("WIPE: erase returned %d", erase_rc);
    k_work_reschedule(&report_work, K_NO_WAIT);
}

static int settings_wipe_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *ev = as_zmk_position_state_changed(eh);
    bool pair_held = false;

    if (ev->state) {
        LOG_INF("key at position %d pressed", ev->position);
    }

    for (size_t i = 0; i < ARRAY_SIZE(wipe_pairs); i++) {
        for (size_t k = 0; k < 2; k++) {
            if (ev->position == wipe_pairs[i][k]) {
                held[i][k] = ev->state;
            }
        }
        pair_held |= held[i][0] && held[i][1];
    }

    if (wiped) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    if (pair_held && !k_work_delayable_is_pending(&wipe_work)) {
        LOG_INF("WIPE: keys held, erasing in %d s unless released", HOLD_MS / 1000);
        k_work_schedule(&wipe_work, K_MSEC(HOLD_MS));
    } else if (!pair_held && k_work_delayable_is_pending(&wipe_work)) {
        k_work_cancel_delayable(&wipe_work);
        LOG_INF("WIPE: keys released, cancelled");
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(settings_wipe, settings_wipe_listener);
ZMK_SUBSCRIPTION(settings_wipe, zmk_position_state_changed);

static int settings_wipe_init(void) {
    k_work_init_delayable(&wipe_work, wipe_work_handler);
    k_work_init_delayable(&report_work, report_work_handler);
    k_work_schedule(&report_work, K_MSEC(REPORT_INTERVAL_MS));
    return 0;
}

SYS_INIT(settings_wipe_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
