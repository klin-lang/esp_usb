/* Thin TinyUSB CDC-ACM helpers for Klin — ESP-IDF v5.x + esp_tinyusb.
 * Device mode first (S2/S3/P4 OTG). USB host later.
 * Buffers are caller-owned; RX staging is a fixed ring (documented max).
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Fixed RX staging capacity (bytes). Overflow drops oldest. */
#define KLIN_USB_CDC_RX_MAX 256

int klin_usb_cdc_init(void);
int klin_usb_cdc_connected(void);
/** Copy up to `max_len` RX bytes into `out`. Returns length, or negative on error. */
int klin_usb_cdc_read(uint8_t *out, int max_len);
/** Queue + flush `len` bytes from `data`. Returns bytes accepted, or negative. */
int klin_usb_cdc_write(const uint8_t *data, int len);
int klin_usb_cdc_stop(void);
int klin_usb_cdc_rx_max(void);

#ifdef __cplusplus
}
#endif
