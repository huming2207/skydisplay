#pragma once

#include <cstdint>
#include <cstdio>
#include <vector>

struct HalIO
{
    int8_t mosi = -1;
    int8_t sclk = -1;
    int8_t miso = -1;
    int8_t wrpt = -1;
    int8_t hold = -1;
    int8_t dc = -1;
    int8_t rst = -1;
    int8_t spi_mode = -1;
    uint16_t sizeX = 0;
    uint16_t sizeY = 0;
    uint8_t spiSpeedMHz = 0;

    bool operator== (const HalIO& config) const
    {
        return config.hold == hold
            && config.sizeX == sizeX
            && config.sizeY == sizeY
            && config.miso == miso
            && config.mosi == mosi
            && config.sclk == sclk
            && config.wrpt == wrpt;
    }
};

struct PayloadTuple
{
    uint8_t cmd;
    std::vector<uint8_t> data;

    bool operator== (const PayloadTuple& cmdSeq) const
    {
        return cmd == cmdSeq.cmd && data.size() == cmdSeq.data.size();
    }
};

class Hal
{
    public:
        Hal() = default;
        ~Hal() = default;
        virtual void WriteSPI(uint8_t *payload, size_t len, uint8_t dcLvl) = 0;
        virtual void ReadSPI(uint8_t *recvBuf, size_t len, uint8_t dcLvl) = 0;
        virtual void WaitSPI(uint32_t waitMS) = 0;
        virtual void ToggleIO(int8_t pin, bool on) = 0;
        virtual void PrepareIO(int8_t pin, bool isOutput) = 0;
        virtual bool ReadIO(int8_t pin) = 0;
};
