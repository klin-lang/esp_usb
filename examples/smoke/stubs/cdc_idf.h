#pragma once
#include <stdint.h>
int klin_usb_cdc_init(void);
int klin_usb_cdc_connected(void);
int klin_usb_cdc_read(uint8_t *out, int max_len);
int klin_usb_cdc_write(const uint8_t *data, int len);
int klin_usb_cdc_stop(void);
int klin_usb_cdc_rx_max(void);
