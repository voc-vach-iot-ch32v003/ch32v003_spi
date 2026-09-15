# ⚡ ch32v003_spi

Thư viện giao tiếp **SPI Master** (hỗ trợ cả **Hardware SPI** và **Software Bit-banging**) chuẩn hóa theo kiểu chân vật lý `MCU_Pin_t` cho vi điều khiển **CH32V003** trên nền framework **ch32v003fun**.
Thư viện tự động linh hoạt giữa việc tối ưu tốc độ phần cứng trên các dòng chip nhiều chân (như TSSOP20 / `CH32V003_F4P6`) hoặc tùy chọn bất kỳ chân GPIO nào để làm Software SPI cho các gói chip nhỏ gọn (SOP8, SOP16).

---

## 🚀 Tính năng nổi bật

- **Hỗ trợ chế độ Kép (Dual Mode):** Tích hợp sẵn cả **Hardware SPI1** (tốc độ cao) lẫn **Software SPI** (tùy biến chân linh hoạt).
- **Tự động thích ứng gói vỏ (Package Adaptability):** Cho phép các gói chip ít chân (như SOP8) định nghĩa chân `sckPin`, `mosiPin`, `misoPin` hoàn toàn tự do.
- **Hỗ trợ Hardware Remap:** Tích hợp tùy chọn Remap chân NSS phần cứng (`NONE` hoặc `NSS_PC0`) cho dòng `CH32V003_F4P6`.
- **Quản lý CS an toàn:** Cung cấp sẵn các hàm `spiInitCS`, `spiSelect`, `spiDeselect` chốt dữ liệu tức thì.

---

## 📑 Cấu trúc hàm (API Reference)

### 1. Khởi tạo Bus SPI

| Hàm xử lý                     | Tham số                       | Giá trị trả về | Mô tả                                                                                                  |
| ----------------------------- | ----------------------------- | -------------- | ------------------------------------------------------------------------------------------------------ |
| **`spiInitHardware(remap)`**  | `SPI_HardwareRemap_t remap`   | `void`         | Khởi tạo SPI1 phần cứng (chỉ áp dụng khi `CH32V003_F4P6` được define). Tham số `NONE` hoặc `NSS_PC0`.  |
| **`spiInitSoftware(config)`** | `SPI_SoftwareConfig_t config` | `void`         | Khởi tạo Software Bit-banging SPI (SPI Mode 0, MSB First) với các chân vật lý tùy chọn trong `config`. |
| **`spiInitCS(csPin)`**        | `MCU_Pin_t csPin`             | `void`         | Khởi tạo chân GPIO làm CS ở chế độ `OUTPUT` và mặc định đưa lên mức `HIGH`.                            |

### 2. Quản lý Chip Select (CS) & Truyền dữ liệu

| Hàm xử lý                | Tham số           | Giá trị trả về | Mô tả                                                                |
| ------------------------ | ----------------- | -------------- | -------------------------------------------------------------------- |
| **`spiSelect(csPin)`**   | `MCU_Pin_t csPin` | `void`         | Kéo chân CS xuống mức `LOW` để bắt đầu phiên truyền SPI.             |
| **`spiDeselect(csPin)`** | `MCU_Pin_t csPin` | `void`         | Kéo chân CS lên mức `HIGH` để kết thúc phiên truyền và chốt dữ liệu. |
| **`spiTransfer(data)`**  | `uint8_t data`    | `uint8_t`      | Truyền 1 byte dữ liệu và nhận về 1 byte từ thiết bị Tớ (Slave).      |
| **`spiWaitIdle()`**      | `void`            | `void`         | Chờ cho bus SPI phần cứng hoàn tất trạng thái bận (`BSY`).           |

---

## ⚙️ Cấu hình Software SPI

Dành cho các thiết kế dùng dòng chip gói nhỏ (SOP8) hoặc không sử dụng chân Hardware SPI cố định:

```c
SPI_SoftwareConfig_t spiConfig = {
    .sckPin  = MCU_PIN5,       // Chân Clock
    .mosiPin = MCU_PIN6,       // Chân Data Out
    .misoPin = MCU_PIN_NONE    // Gán MCU_PIN_NONE nếu không sử dụng MISO (Master Out Only)
};

```

---

## 📝 Code mẫu sử dụng

### 1. Sử dụng Hardware SPI (Gói chip TSSOP20 / F4P6)

```c
#include "ch32fun.h"
#include <ch32v003_gpio.h>
#include <ch32v003_spi.h>

#define CS_PIN MCU_PIN3

void setup() {
    // Khởi tạo SPI phần cứng không Remap chân NSS
    spiInitHardware(NONE);

    // Khởi tạo chân Chip Select
    spiInitCS(CS_PIN);
}

void loop() {
    spiSelect(CS_PIN);
    spiTransfer(0xA5); // Gửi 1 byte qua Hardware SPI
    spiWaitIdle();
    spiDeselect(CS_PIN);
}

int main() {
    SystemInit();
    setup();
    while (1) {
        loop();
    }
}

```

### 2. Sử dụng Software SPI (Gói chip SOP8 / Tùy chọn chân GPIO)

```c
#include "ch32fun.h"
#include <ch32v003_gpio.h>
#include <ch32v003_spi.h>

#define CS_PIN MCU_PIN3

void setup() {
    // 1. Khai báo cấu hình chân Software SPI
    SPI_SoftwareConfig_t softSpi = {
        .sckPin  = MCU_PIN5,
        .mosiPin = MCU_PIN6,
        .misoPin = MCU_PIN_NONE // Chỉ truyền, không nhận
    };

    // 2. Khởi tạo Software SPI và chân CS
    spiInitSoftware(softSpi);
    spiInitCS(CS_PIN);
}

void loop() {
    spiSelect(CS_PIN);
    spiTransfer(0x55); // Gửi 1 byte qua Software Bit-banging
    spiDeselect(CS_PIN);
}

int main() {
    SystemInit();
    setup();
    while (1) {
        loop();
    }
}

```

---
