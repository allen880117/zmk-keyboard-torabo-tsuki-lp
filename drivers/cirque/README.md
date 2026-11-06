# Cirque Pinnacle driver (vendored)

Taken from the `cirque-trackpad` branch of
[KYENS/zmk_torabotsuki_kyens_custom](https://github.com/KYENS/zmk_torabotsuki_kyens_custom),
which is a copy of `drivers/input` from
[billyjacoby/cirque-input-module](https://github.com/billyjacoby/cirque-input-module) at `ba53fe8`. That fork adds absolute mode to
[petejohanson/cirque-input-module](https://github.com/petejohanson/cirque-input-module), the original ZMK contributors' driver (MIT).

That branch's commit "Vendor Cirque Pinnacle driver" holds the unmodified original.

## Changes from upstream

- **Fixed: the trackpad stops working after a while.** In absolute mode, the data-ready (DR) interrupt is masked when it fires and only unmasked after a successful read. If one read failed (an I2C glitch, or DR firing without fresh data), the function returned early. The interrupt then stayed masked and the pad went dead until the next reboot. Now the status is always cleared and the interrupt re-armed.
- **Added: watchdog.** Every 500 ms the driver checks whether DR is stuck active with no read pending. If it is, the driver services it, which re-arms the interrupt. The check is skipped while the device is suspended.
- **Added: `absolute-mode-z-min`.** In absolute mode, touches weaker than this Z value (touch strength, 0–63) are ignored. Without it, a hovering finger or electrical noise produces faint "touches" at random positions, and the cursor floats on its own. The driver also reports Z (as `ABS_Z`) so the gesture processor can log it.
- **Added: configuration check.** Every 2 s the watchdog reads one of the pad's settings back (`FEED_CFG1`). If the pad has reset itself (a brief voltage dip or static zap) it comes back in its default mode and goes quiet. When that happens, the whole configuration is written again.
- **Fixed here: the same dead-pad bug in relative mode.** `pinnacle_report_data_rel` also returned early on a failed read without clearing the status or re-arming the interrupt. It now does both, like absolute mode.
- **Removed here:** the ZMK idle sleeper (`CONFIG_ZMK_INPUT_PINNACLE_IDLE_SLEEPER`). ZMK only counts key presses as activity, so it put the pad to sleep during trackpad-only use.
- **Rewritten here: `abs-rel-divisor` mode.** The pad runs in absolute mode and the driver sends the change in a smoothed finger position as relative motion. The pad's own relative mode was too coarse to use (laggy and jumpy). The old version of this mode cast deltas to `int8_t` (fast swipes wrapped around), had no touch-strength filter or smoothing, and left the interrupt masked after a failed read. Inversion is done in software in this mode. New property: `abs-rel-start-threshold`.
