#pragma once

#include "Hal.hpp"

class ESP32 : public Hal
{
    public:
        explicit ESP32(HalIO& _io);
        ~ESP32();
        void WriteSPI(uint8_t *payload, size_t len, uint8_t dcLvl) override;
        void ReadSPI(uint8_t *recvBuf, size_t len, uint8_t dcLvl) override;
        void WaitSPI(uint32_t waitMS) override;
        void ToggleIO(int8_t pin, bool on) override;
        void PrepareIO(int8_t pin, bool isOutput) override;
        bool ReadIO(int8_t pin) override;

    private:
        HalIO io{};
        spi_device_handle_t spiDeviceHandle{};

};
