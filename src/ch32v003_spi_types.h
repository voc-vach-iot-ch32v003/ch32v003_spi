/**
 * @file ch32v003_spi_types.h
 * @author Vọc Vạch IoT
 * @brief Định nghĩa enum và cấu trúc Handle cấu hình SPI (Hardware / Software) cho CH32V003.
 */

#ifndef CH32V003_SPI_TYPES_H
#define CH32V003_SPI_TYPES_H

#include "ch32v003_gpio.h"

/**
 * @brief Danh mục phân loại chế độ hoạt động của Driver SPI.
 */
typedef enum
{
    SPI_MODE_SOFTWARE = 0, /**< Bit-banging trên chân bất kỳ (Mọi gói vỏ J4M6 / A4M6 / F4P6) */
    SPI_MODE_HARDWARE = 1 /**< Khối SPI1 phần cứng: SCK=PC5, MOSI=PC6, MISO=PC7 (Chỉ TSSOP20 / F4P6) */
} SPI_DriverMode_t;

/**
 * @brief Hệ số chia xung nhịp SPI1 phần cứng (Tần số SCK = HCLK 48MHz / hệ số chia).
 */
typedef enum
{
    SPI_CLOCK_DIVIDER_2 = 0, /**< 24 MHz */
    SPI_CLOCK_DIVIDER_4 = 1, /**< 12 MHz */
    SPI_CLOCK_DIVIDER_8 = 2, /**< 6 MHz */
    SPI_CLOCK_DIVIDER_16 = 3, /**< 3 MHz */
    SPI_CLOCK_DIVIDER_32 = 4, /**< 1.5 MHz */
    SPI_CLOCK_DIVIDER_64 = 5, /**< 750 kHz */
    SPI_CLOCK_DIVIDER_128 = 6, /**< 375 kHz */
    SPI_CLOCK_DIVIDER_256 = 7 /**< 187.5 kHz */
} SPI_ClockDivider_t;

/**
 * @brief Cấu hình chi tiết cho tầng SPI Hardware (SPI1 Master, Mode 0, MSB First, NSS mềm).
 */
typedef struct
{
    SPI_ClockDivider_t clockDivider; /**< Hệ số chia xung nhịp SCK (Mặc định 0 = 24MHz) */
} SPI_HardwareConfiguration_t;

/**
 * @brief Cấu hình chân vật lý cho tầng SPI Software (Mode 0, MSB First).
 */
typedef struct
{
    uint8_t sckPin; /**< Chân Clock vật lý (Lấy từ sơ đồ MCU_Pin_t) */
    uint8_t mosiPin; /**< Chân MOSI vật lý (Lấy từ sơ đồ MCU_Pin_t) */
    uint8_t misoPin; /**< Chân MISO vật lý, gán MCU_PIN_NONE nếu chỉ ghi */
} SPI_SoftwareConfiguration_t;

/**
 * @brief Cấu trúc quản lý ngữ cảnh Driver SPI tổng thể.
 * @details Người dùng chỉ điền driverMode và config; các trường [NỘI BỘ] do spiInit() tự tính sẵn
 *          (Port + mặt nạ bit) để vòng bit-banging ghi thẳng thanh ghi BSHR/BCR, không giải mã chân mỗi bit.
 */
typedef struct
{
    SPI_DriverMode_t driverMode; /**< Chế độ Driver hoạt động (Hardware/Software) */
    union
    {
        SPI_HardwareConfiguration_t hardware; /**< Cấu hình phần cứng chuyên dụng */
        SPI_SoftwareConfiguration_t software; /**< Cấu hình phần mềm Bit-banging */
    } config;

    GPIO_TypeDef* sckPort; /**< [NỘI BỘ] Port của chân SCK */
    GPIO_TypeDef* mosiPort; /**< [NỘI BỘ] Port của chân MOSI */
    GPIO_TypeDef* misoPort; /**< [NỘI BỘ] Port của chân MISO (NULL nếu không dùng) */
    uint8_t sckMask; /**< [NỘI BỘ] Mặt nạ bit chân SCK */
    uint8_t mosiMask; /**< [NỘI BỘ] Mặt nạ bit chân MOSI */
    uint8_t misoMask; /**< [NỘI BỘ] Mặt nạ bit chân MISO */
} SPI_DriverHandle_t;

#endif // CH32V003_SPI_TYPES_H
