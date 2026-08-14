#include "cdc_idf.h"
#include <string.h>

static int s_inited;
static int s_conn;
static uint8_t s_rx[256];
static int s_rx_len;

int klin_usb_cdc_rx_max(void) { return 256; }
int klin_usb_cdc_init(void)
{
    s_inited = 1;
    s_conn = 1;
    s_rx[0] = 65;
    s_rx_len = 1;
    return 0;
}
int klin_usb_cdc_connected(void) { return s_inited && s_conn; }
int klin_usb_cdc_read(uint8_t *out, int max_len)
{
    int n;
    if (out == NULL || max_len < 0) {
        return -1;
    }
    n = s_rx_len;
    if (n > max_len) {
        n = max_len;
    }
    if (n > 0) {
        memcpy(out, s_rx, (size_t)n);
        s_rx_len = 0;
    }
    return n;
}
int klin_usb_cdc_write(const uint8_t *data, int len)
{
    if (data == NULL || len < 0) {
        return -1;
    }
    return len;
}
int klin_usb_cdc_stop(void)
{
    s_conn = 0;
    s_inited = 0;
    return 0;
}
