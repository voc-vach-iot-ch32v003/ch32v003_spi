# ⚡ ch32v003_spi

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![PlatformIO](https://img.shields.io/badge/PlatformIO-Supported-orange.svg)
![Framework](https://img.shields.io/badge/Framework-ch32v003fun-green.svg)
![MCU](https://img.shields.io/badge/MCU-CH32V003-red.svg)

Thư viện SPI Master (Mode 0, MSB First) cho vi điều khiển CH32V003, dùng chung một API cho **SPI1 phần cứng** và **SPI phần mềm** (bit-banging trên chân GPIO bất kỳ). Mỗi bus được quản lý qua một Handle `SPI_DriverHandle_t`, nên một chương trình có thể dùng nhiều bus cùng lúc.

---

## 🚀 Tính năng nổi bật

- **Hai chế độ, một API:** đổi `driverMode` giữa `SPI_MODE_HARDWARE` và `SPI_MODE_SOFTWARE` mà không phải sửa code truyền nhận.
- **SPI1 phần cứng tới 24MHz** (chỉ F4P6, vì SCK = PC5 không có trên A4M6/J4M6). `spiWrite()` nạp byte kế tiếp ngay khi bộ đệm trống nên SCK chạy liền mạch: 32 byte ở 3MHz mất 84µs, đúng bằng lý thuyết.
- **SPI phần mềm ~1MHz trên mọi gói vỏ:** `spiInit()` giải mã chân một lần, vòng truyền bit chỉ ghi thẳng thanh ghi `BSHR`/`BCR`.
- **Chỉ ghi (write-only):** gán `misoPin = MCU_PIN_NONE` để tiết kiệm một chân (LED ma trận MAX7219, 74HC595...).
- **Quản lý CS:** `spiInitCS()`, `spiSelect()`, `spiDeselect()` cho chân CS bất kỳ.

---

## 📦 Cài đặt (Installation)

```ini
lib_deps =
    https://github.com/voc-vach-iot-ch32v003/ch32v003_spi.git
```

Thư viện tự kéo theo `ch32v003_gpio`.

---

## ⚙️ Cấu hình môi trường

Khai báo gói vỏ chip trong `platformio.ini`:

```ini
build_flags = -DCH32V003_F4P6 ; hoặc -DCH32V003_A4M6 / -DCH32V003_J4M6
```

| Chế độ | Gói vỏ | Chân |
| :-- | :-- | :-- |
| `SPI_MODE_HARDWARE` | F4P6 | SCK = PC5, MOSI = PC6, MISO = PC7. CS là chân GPIO bất kỳ. |
| `SPI_MODE_SOFTWARE` | J4M6, A4M6, F4P6 | `sckPin`, `mosiPin`, `misoPin` tùy chọn. |

Trên A4M6/J4M6, `spiInit()` trả `0` nếu chọn `SPI_MODE_HARDWARE`.

---

## 📝 Code mẫu sử dụng (Quick Start)

Đếm nhị phân trên 8 LED nối vào IC dịch 74HC595, dùng SPI phần mềm nên chạy được trên cả 3 gói vỏ:

```c
#include "ch32fun.h"
#include "ch32v003_delay.h"
#include "ch32v003_gpio.h"
#include "ch32v003_spi.h"

// Chân vật lý (port tương ứng khác nhau theo gói vỏ, xem ch32v003_gpio_pins.h)
#define LATCH_PIN MCU_PIN3 // 74HC595 ST_CP

static SPI_DriverHandle_t spiBus = {
    .driverMode = SPI_MODE_SOFTWARE,
    .config.software = {
        .sckPin = MCU_PIN5,       // 74HC595 SH_CP
        .mosiPin = MCU_PIN6,      // 74HC595 DS
        .misoPin = MCU_PIN_NONE,  // Chỉ ghi
    },
};

static uint8_t counter = 0;

static void setup(void)
{
    spiInit(&spiBus);
    spiInitCS(LATCH_PIN);
}

static void loop(void)
{
    spiSelect(LATCH_PIN);
    spiTransfer(&spiBus, counter++);
    spiWaitIdle(&spiBus);
    spiDeselect(LATCH_PIN); // Sườn lên chốt dữ liệu ra 8 ngõ của 74HC595

    delayMs(200);
}

int main(void)
{
    SystemInit();
    delayMs(1000); // Khoảng chờ an toàn tránh brick chip

    setup();
    while (1)
    {
        loop();
    }
}
```

Dùng SPI1 phần cứng trên F4P6 (SCK = PC5, MOSI = PC6) thì chỉ cần đổi Handle:

```c
static SPI_DriverHandle_t spiBus = {
    .driverMode = SPI_MODE_HARDWARE,
    .config.hardware = {.clockDivider = SPI_CLOCK_DIVIDER_8}, // 48MHz / 8 = 6MHz
};
```

---

## 📁 Danh sách Ví dụ mẫu (Examples)

Thư viện giao tiếp nền: chưa có thư mục `examples/`. Các thư viện thiết bị (VD: `ch32v003_max7219`, `ch32v003_led_matrix`) là ví dụ sử dụng thực tế.

---

## 📑 Tra cứu API (API Reference)

### Khởi tạo

| Hàm xử lý | Tham số | Giá trị trả về | Mô tả |
| :-- | :-- | :-- | :-- |
| `spiInit(handle)` | `SPI_DriverHandle_t* handle` | `uint8_t` | Cấu hình chân và tốc độ, ghi sẵn Port/mặt nạ bit vào các trường nội bộ của Handle. Trả `0` nếu `handle` rỗng hoặc chọn phần cứng trên gói vỏ không có SPI1. |
| `spiInitCS(csPin)` | `uint8_t csPin` | `void` | Đặt chân CS làm Output, mức HIGH (nhả thiết bị). |

### Chip Select

| Hàm xử lý | Tham số | Giá trị trả về | Mô tả |
| :-- | :-- | :-- | :-- |
| `spiSelect(csPin)` | `uint8_t csPin` | `void` | Kéo CS xuống LOW để chọn thiết bị. |
| `spiDeselect(csPin)` | `uint8_t csPin` | `void` | Kéo CS lên HIGH để nhả thiết bị (chốt dữ liệu với IC thanh ghi dịch). |

### Truyền nhận

| Hàm xử lý | Tham số | Giá trị trả về | Mô tả |
| :-- | :-- | :-- | :-- |
| `spiTransfer(handle, data)` | `const SPI_DriverHandle_t* handle`, `uint8_t data` | `uint8_t` | Gửi 1 byte, đồng thời trả về byte nhận trên MISO (`0` nếu `misoPin = MCU_PIN_NONE`). |
| `spiWrite(handle, data, length)` | `const uint8_t* data`, `uint16_t length` | `void` | Ghi liên tiếp một mảng byte, bỏ dữ liệu nhận. Phần cứng: SCK liền mạch, trả về khi byte cuối đã truyền xong. |
| `spiWaitIdle(handle)` | `const SPI_DriverHandle_t* handle` | `void` | Chờ bus rảnh (cờ BSY) trước khi nhả CS. SPI phần mềm trả về ngay. |

### Kiểu dữ liệu

| Kiểu | Giá trị | Mô tả |
| :-- | :-- | :-- |
| `SPI_DriverMode_t` | `SPI_MODE_SOFTWARE`, `SPI_MODE_HARDWARE` | Chế độ của Handle. |
| `SPI_ClockDivider_t` | `SPI_CLOCK_DIVIDER_2` (24MHz) … `SPI_CLOCK_DIVIDER_256` (187.5kHz) | Tần số SCK của SPI1 = 48MHz / hệ số chia. |
| `SPI_DriverHandle_t` | `driverMode` + `config.hardware { clockDivider }` hoặc `config.software { sckPin, mosiPin, misoPin }` | Handle của một bus. Các trường `[NỘI BỘ]` do `spiInit()` ghi. |

---

## 📄 Giấy phép (License) & Tác giả

- **Tác giả:** [Vọc Vạch IoT](https://github.com/voc-vach-iot)
- **Giấy phép:** Phát hành theo giấy phép [MIT License](LICENSE).
