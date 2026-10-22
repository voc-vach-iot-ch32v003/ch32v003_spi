#include "ch32v003_spi.h"

uint8_t spiInit(SPI_DriverHandle_t* handle)
{
    if (handle == NULL) return 0;

    // --- TRƯỜNG HỢP 1: SỬ DỤNG SPI1 PHẦN CỨNG (Chỉ gói TSSOP20 có chân SCK = PC5) ---
    if (handle->driverMode == SPI_MODE_HARDWARE)
    {
#if defined(CH32V003_F4P6)
        RCC->APB2PCENR |= RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO;
        AFIO->PCFR1 &= ~(1UL << 0); // SPI1 về sơ đồ chân mặc định (SCK=PC5, MOSI=PC6, MISO=PC7)

        pinMode(MCU_SPI_SCK, AF_PP_50M);
        pinMode(MCU_SPI_MOSI, AF_PP_50M);
        pinMode(MCU_SPI_MISO, INPUT);

        // Master, NSS quản lý bằng phần mềm (SSM + SSI), Mode 0, MSB First
        SPI1->CTLR1 = SPI_CTLR1_MSTR | SPI_CTLR1_SSM | SPI_CTLR1_SSI |
            ((handle->config.hardware.clockDivider & 0x07) << 3);
        SPI1->CTLR1 |= SPI_CTLR1_SPE;
        return 1;
#else
        return 0;
#endif
    }

    // --- TRƯỜNG HỢP 2: SỬ DỤNG SPI PHẦN MỀM (BIT-BANGING) ---
    const SPI_SoftwareConfiguration_t* pins = &handle->config.software;
    uint8_t pinNumber;

    pinMode(pins->sckPin, OUTPUT_50M);
    pinMode(pins->mosiPin, OUTPUT_50M);
    digitalWrite(pins->sckPin, LOW); // Mode 0: SCK nghỉ ở mức THẤP
    digitalWrite(pins->mosiPin, LOW);

    // Giải mã chân một lần duy nhất, vòng truyền bit chỉ còn ghi thẳng BSHR/BCR
    decodeHardwarePin(pins->sckPin, &handle->sckPort, &pinNumber);
    handle->sckMask = 1 << pinNumber;
    decodeHardwarePin(pins->mosiPin, &handle->mosiPort, &pinNumber);
    handle->mosiMask = 1 << pinNumber;

    handle->misoPort = NULL;
    handle->misoMask = 0;
    if (MCU_PIN_IS_GPIO(pins->misoPin))
    {
        pinMode(pins->misoPin, INPUT);
        decodeHardwarePin(pins->misoPin, &handle->misoPort, &pinNumber);
        handle->misoMask = 1 << pinNumber;
    }
    return 1;
}

void spiInitCS(const uint8_t csPin)
{
    pinMode(csPin, OUTPUT);
    spiDeselect(csPin);
}

void spiSelect(const uint8_t csPin)
{
    digitalWrite(csPin, LOW);
}

void spiDeselect(const uint8_t csPin)
{
    digitalWrite(csPin, HIGH);
}

uint8_t spiTransfer(const SPI_DriverHandle_t* handle, uint8_t data)
{
#if defined(CH32V003_F4P6)
    if (handle->driverMode == SPI_MODE_HARDWARE)
    {
        while (!(SPI1->STATR & SPI_STATR_TXE));
        SPI1->DATAR = data;
        while (!(SPI1->STATR & SPI_STATR_RXNE));
        return SPI1->DATAR;
    }
#endif

    // Software SPI Bit-banging (MSB First, SPI Mode 0): đặt MOSI -> SCK lên (chốt) -> đọc MISO -> SCK xuống
    GPIO_TypeDef* const sckPort = handle->sckPort;
    GPIO_TypeDef* const mosiPort = handle->mosiPort;
    const uint8_t sckMask = handle->sckMask;
    const uint8_t mosiMask = handle->mosiMask;
    uint8_t rxData = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
        if (data & 0x80)
        {
            mosiPort->BSHR = mosiMask;
        }
        else
        {
            mosiPort->BCR = mosiMask;
        }
        data <<= 1;

        sckPort->BSHR = sckMask;

        rxData <<= 1;
        if (handle->misoPort != NULL && (handle->misoPort->INDR & handle->misoMask))
        {
            rxData |= 0x01;
        }

        sckPort->BCR = sckMask;
    }
    return rxData;
}

void spiWrite(const SPI_DriverHandle_t* handle, const uint8_t* data, const uint16_t length)
{
#if defined(CH32V003_F4P6)
    if (handle->driverMode == SPI_MODE_HARDWARE)
    {
        // Nạp byte kế tiếp ngay khi bộ đệm phát trống (không chờ nhận xong từng byte) -> SCK liền mạch
        for (uint16_t i = 0; i < length; i++)
        {
            while (!(SPI1->STATR & SPI_STATR_TXE));
            SPI1->DATAR = data[i];
        }
        while (!(SPI1->STATR & SPI_STATR_TXE)); // Byte cuối đã vào thanh ghi dịch -> BSY chắc chắn phản ánh đúng
        while (SPI1->STATR & SPI_STATR_BSY);

        // Bỏ byte nhận cuối và xóa cờ tràn OVR (đọc DATAR rồi STATR) để spiTransfer sau không đọc nhầm
        (void)SPI1->DATAR;
        (void)SPI1->STATR;
        return;
    }
#endif
    for (uint16_t i = 0; i < length; i++)
    {
        spiTransfer(handle, data[i]);
    }
}

void spiWaitIdle(const SPI_DriverHandle_t* handle)
{
#if defined(CH32V003_F4P6)
    if (handle->driverMode == SPI_MODE_HARDWARE)
    {
        while (SPI1->STATR & SPI_STATR_BSY);
    }
#else
    (void)handle;
#endif
}
