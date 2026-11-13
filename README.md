
[torabo-tsuki LP](https://github.com/sekigon-gonnoc/torabo-tsuki-lp)用のZMKファームウェア

* _centralがついているuf2をトラックボールがついている方に、_peripheralを反対側に書き込んでください
* キーマップはkeymap-editorおよびzmk-studioで編集できます
## Clearing pairings (settings wipe)

Use this when the halves won't connect to each other, or a half still connects to an old computer. On these boards the stock `settings_reset` firmware can leave old pairings behind; the wipe firmware erases them for sure and reports the result.

`torabo_tsuki_lp_left_wipe.uf2` and `torabo_tsuki_lp_right_wipe.uf2` erase a half's settings (Bluetooth pairings, the other half, ZMK Studio keymap edits) when you hold the two outermost keys of its top row for 5 seconds: top-left on the left half, top-right on the right half. No other key does anything in this firmware.

Leaving bootloader mode: unplug the USB cable, turn the power switch on, then plug the cable back in.

| # | Half | Mode | Do this | Result |
|---|---|---|---|---|
| 1 | Left | Bootloader | Copy `torabo_tsuki_lp_left_wipe.uf2` onto the drive | Drive closes |
| 2 | Left | Running (leave bootloader mode) | Hold the two top-left keys for 5 s | Log shows `WIPE DONE (left)` |
| 3 | Left | Bootloader | Copy the left half's normal firmware (see below) | Drive closes |
| 4 | Right | Bootloader | Copy `torabo_tsuki_lp_right_wipe.uf2` onto the drive | Drive closes |
| 5 | Right | Running (leave bootloader mode) | Hold the two top-right keys for 5 s | Log shows `WIPE DONE (right)` |
| 6 | Right | Bootloader | Copy the right half's normal firmware (see below) | Drive closes |
| 7 | Both | Running (leave bootloader mode) | Turn both on next to each other and type on the half without the trackball | Its keys type |
| 8 | - | - | On each computer remove the keyboard from the Bluetooth list and pair again | |

Normal firmware for steps 3 and 6 (`_central` goes on the half with the trackball):

| Setup | Left half | Right half |
|---|---|---|
| Trackball on the left | `torabo_tsuki_lp_left_central.uf2` | `torabo_tsuki_lp_right_peripheral.uf2` |
| Trackball on the right | `torabo_tsuki_lp_left_peripheral.uf2` | `torabo_tsuki_lp_right_central.uf2` |

Reading the log (optional; the wipe works without it): with the half running the wipe firmware and plugged in by USB, open its serial port (shown as "USB Serial Device (COMx)" on Windows) in a terminal such as Tera Term, any speed. Every 5 s it prints `WIPE FIRMWARE (...)` before the wipe and `WIPE DONE (...)` after; `WIPE FAILED` means the erase didn't work. Only one program can open the port at a time.

Wipe one half at a time, and avoid doing it while another unpaired torabo-tsuki half is on nearby: at first connection a half pairs with whichever half it finds first.
