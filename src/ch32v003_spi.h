/**
 * @file ch32v003_spi.h
 * @author Vọc Vạch IoT
 * @brief Thư viện SPI Master hợp nhất (Hardware SPI1 / Software Bit-banging) theo Handle cho CH32V003.
 */

#ifndef CH32V003_SPI_H
#define CH32V003_SPI_H

#include "ch32fun.h"
#include "ch32v003_gpio.h"
#include "ch32v003_spi_types.h"

// ============================================================================
// HÀM KHỞI TẠO BUS SPI
// ============================================================================

/**
 * @brief Khởi tạo Bus SPI Master theo cấu hình trong Handle (Mode 0, MSB First).
 * @param handle Con trỏ Handle đã điền driverMode và config. Các trường [NỘI BỘ] sẽ được ghi.
 * @return uint8_t 1 nếu thành công, 0 nếu cấu hình lỗi (VD: chọn Hardware trên gói vỏ không có PC5).
 */
uint8_t spiInit(SPI_DriverHandle_t* handle);

// ============================================================================
// HÀM QUẢN LÝ CHÂN CHIP SELECT (CS / NSS SOFTWARE)
// ============================================================================

/**
 * @brief Khởi tạo chân GPIO làm CS (Output, mặc định HIGH = nhả thiết bị).
 * @param csPin Chân vật lý (MCU_Pin_t / MCU_SpecialPin_t).
 */
void spiInitCS(uint8_t csPin);

/**
 * @brief Kéo CS xuống LOW để chọn thiết bị.
 * @param csPin Chân vật lý (MCU_Pin_t / MCU_SpecialPin_t).
 */
void spiSelect(uint8_t csPin);

/**
 * @brief Kéo CS lên HIGH để nhả thiết bị (chốt dữ liệu với IC dạng thanh ghi dịch như MAX7219).
 * @param csPin Chân vật lý (MCU_Pin_t / MCU_SpecialPin_t).
 */
void spiDeselect(uint8_t csPin);

// ============================================================================
// HÀM TRUYỀN NHẬN DỮ LIỆU
// ============================================================================

/**
 * @brief Truyền đồng thời nhận 1 Byte qua SPI.
 * @param handle Con trỏ Handle đã khởi tạo bằng spiInit().
 * @param data Byte cần truyền.
 * @return uint8_t Byte nhận về trên MISO (0 nếu không cấu hình MISO ở chế độ Software).
 */
uint8_t spiTransfer(const SPI_DriverHandle_t* handle, uint8_t data);

/**
 * @brief Ghi liên tiếp một mảng byte (bỏ qua dữ liệu nhận về).
 * @param handle Con trỏ Handle đã khởi tạo bằng spiInit().
 * @param data Mảng byte cần ghi.
 * @param length Số byte cần ghi.
 */
void spiWrite(const SPI_DriverHandle_t* handle, const uint8_t* data, uint16_t length);

/**
 * @brief Chờ bus SPI truyền xong byte cuối (cờ BSY) trước khi nhả CS. Chế độ Software trả về ngay.
 * @param handle Con trỏ Handle đã khởi tạo bằng spiInit().
 */
void spiWaitIdle(const SPI_DriverHandle_t* handle);

#endif // CH32V003_SPI_H
