# esp_usb

Thin **ESP-IDF TinyUSB** bindings for [Klin](https://github.com/klin-lang/klin).

USB OTG is in the **silicon** (S2 / S3 / P4); this package does **not** belong in
[`machine_esp`](https://github.com/klin-lang/machine_esp) (MMIO Pin…Adc+Rmt).
Sibling of [`esp_wifi`](https://github.com/klin-lang/esp_wifi) /
[`esp_eth`](https://github.com/klin-lang/esp_eth) /
[`esp_ble`](https://github.com/klin-lang/esp_ble).
Distinct from RP [`UsbCdc`](https://github.com/klin-lang/machine_rp) poll ACM.

C engine = **TinyUSB** via `espressif/esp_tinyusb`. Klin is a thin FFI client.

## Status (`@v0.1.0`)

| API | Notes |
|---|---|
| `cdc_init` | TinyUSB driver + CDC-ACM0 |
| `cdc_connected` | Host DTR |
| `cdc_read` / `cdc_write` | Caller buffers; RX staging max `cdc_rx_max()` (256) |
| `cdc_stop` | Mark idle |
| USB **host** / HID / MSC / MIDI | **Out of scope** (later tags) |

`version()` → `1`.

## Requirements

- Klin compiler
- ESP-IDF **v5.x** + component `espressif/esp_tinyusb` (`^1`)
- SoC with USB-OTG (ESP32-S2 / S3 / P4)
- `CONFIG_TINYUSB_CDC_ENABLED=y`

## Usage

```klin
import "github/klin-lang/esp_usb" usb

@[cexport, codename("klin_app_main")]
fn app() {
  let mut e = usb.cdc_init()
  if e != usb.err_ok() {
    return
  }
  while !usb.cdc_connected() {
  }
  let mut hi: [5]u8
  hi[0] = 104
  hi[1] = 101
  hi[2] = 108
  hi[3] = 108
  hi[4] = 111
  let _w = usb.cdc_write(cast(*u8, &hi[0]), 5)
}
```

```sh
klin get github/klin-lang/esp_usb@v0.1.0
```

## Contract

- No Klin GC / hidden heap — TX/RX payloads are buffers you pass.
- RX staging is a **fixed 256-byte** ring replace (overflow drops prior unread).
- TinyUSB task / endpoint buffers are **IDF contracts**.
- Errors are `i32` (0 = OK).
