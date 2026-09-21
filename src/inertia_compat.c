// SPDX-License-Identifier: MIT

// zmk-input-inertia targets a newer ZMK that names this function
// zmk_endpoint_send_mouse_report(). ZMK v0.3 (pinned in west.yml) names it
// zmk_endpoints_send_mouse_report(), so forward the call to keep linking.

#include <zephyr/sys/util_macro.h>

#if IS_ENABLED(CONFIG_ZMK_INPUT_PROCESSOR_INERTIA)

#include <zmk/endpoints.h>

int zmk_endpoint_send_mouse_report(void) { return zmk_endpoints_send_mouse_report(); }

#endif
