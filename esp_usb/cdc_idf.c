/* TinyUSB CDC-ACM device glue for Klin (ESP-IDF v5.x + espressif/esp_tinyusb).
 * Requires CONFIG_TINYUSB_CDC_ENABLED. Host mode later.
 */
#include "cdc_idf.h"

#include <string.h>

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "tinyusb.h"
#include "tusb_cdc_acm.h"

static int s_inited;
static int s_connected; /* DTR from host */
static uint8_t s_rx[KLIN_USB_CDC_RX_MAX];
static int s_rx_len;
static SemaphoreHandle_t s_rx_mu;

static void klin_usb_cdc_line_state_cb(int itf, cdcacm_event_t *event)
{
    (void)itf;
    if (event == NULL) {
        return;
    }
    s_connected = event->line_state_changed_data.dtr ? 1 : 0;
}

static void klin_usb_cdc_rx_cb(int itf, cdcacm_event_t *event)
{
    uint8_t tmp[KLIN_USB_CDC_RX_MAX];
    size_t n = 0;
    esp_err_t err;

    (void)event;
    err = tinyusb_cdcacm_read(itf, tmp, sizeof(tmp), &n);
    if (err != ESP_OK || n == 0) {
        return;
    }
    if (s_rx_mu == NULL) {
        return;
    }
    xSemaphoreTake(s_rx_mu, portMAX_DELAY);
    if ((int)n > KLIN_USB_CDC_RX_MAX) {
        n = KLIN_USB_CDC_RX_MAX;
    }
    /* Replace staging (simple; document drop-on-overflow). */
    memcpy(s_rx, tmp, n);
    s_rx_len = (int)n;
    xSemaphoreGive(s_rx_mu);
}

int klin_usb_cdc_rx_max(void)
{
    return KLIN_USB_CDC_RX_MAX;
}

int klin_usb_cdc_init(void)
{
    esp_err_t err;
    tinyusb_config_t tusb_cfg;
    tinyusb_config_cdcacm_t acm_cfg;

    if (s_inited) {
        return (int)ESP_OK;
    }

    s_rx_mu = xSemaphoreCreateMutex();
    if (s_rx_mu == NULL) {
        return (int)ESP_ERR_NO_MEM;
    }
    s_rx_len = 0;
    s_connected = 0;

    memset(&tusb_cfg, 0, sizeof(tusb_cfg));
    tusb_cfg.device_descriptor = NULL;
    tusb_cfg.string_descriptor = NULL;
    tusb_cfg.external_phy = false;
#if defined(TUD_OPT_HIGH_SPEED) && TUD_OPT_HIGH_SPEED
    tusb_cfg.fs_configuration_descriptor = NULL;
    tusb_cfg.hs_configuration_descriptor = NULL;
    tusb_cfg.qualifier_descriptor = NULL;
#else
    tusb_cfg.configuration_descriptor = NULL;
#endif

    err = tinyusb_driver_install(&tusb_cfg);
    if (err != ESP_OK) {
        return (int)err;
    }

    memset(&acm_cfg, 0, sizeof(acm_cfg));
    acm_cfg.usb_dev = TINYUSB_USBDEV_0;
    acm_cfg.cdc_port = TINYUSB_CDC_ACM_0;
    acm_cfg.rx_unread_buf_sz = 64;
    acm_cfg.callback_rx = &klin_usb_cdc_rx_cb;
    acm_cfg.callback_rx_wanted_char = NULL;
    acm_cfg.callback_line_state_changed = &klin_usb_cdc_line_state_cb;
    acm_cfg.callback_line_coding_changed = NULL;

    err = tusb_cdc_acm_init(&acm_cfg);
    if (err != ESP_OK) {
        return (int)err;
    }

    s_inited = 1;
    return (int)ESP_OK;
}

int klin_usb_cdc_connected(void)
{
    return (s_inited && s_connected) ? 1 : 0;
}

int klin_usb_cdc_read(uint8_t *out, int max_len)
{
    int n;

    if (out == NULL || max_len < 0) {
        return (int)ESP_ERR_INVALID_ARG;
    }
    if (!s_inited || s_rx_mu == NULL) {
        return (int)ESP_ERR_INVALID_STATE;
    }
    xSemaphoreTake(s_rx_mu, portMAX_DELAY);
    n = s_rx_len;
    if (n > max_len) {
        n = max_len;
    }
    if (n > 0) {
        memcpy(out, s_rx, (size_t)n);
        /* Consume prefix; keep rest. */
        if (n < s_rx_len) {
            memmove(s_rx, s_rx + n, (size_t)(s_rx_len - n));
            s_rx_len -= n;
        } else {
            s_rx_len = 0;
        }
    }
    xSemaphoreGive(s_rx_mu);
    return n;
}

int klin_usb_cdc_write(const uint8_t *data, int len)
{
    size_t queued;

    if (data == NULL || len < 0) {
        return (int)ESP_ERR_INVALID_ARG;
    }
    if (!s_inited) {
        return (int)ESP_ERR_INVALID_STATE;
    }
    if (len == 0) {
        return 0;
    }
    queued = tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0, (uint8_t *)data,
                                        (size_t)len);
    (void)tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_0, pdMS_TO_TICKS(100));
    return (int)queued;
}

int klin_usb_cdc_stop(void)
{
    /* TinyUSB has no full uninstall in all IDF versions — mark idle. */
    s_connected = 0;
    s_inited = 0;
    s_rx_len = 0;
    return (int)ESP_OK;
}
