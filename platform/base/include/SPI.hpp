#pragma once

#include <cstdint>
#include <cstdio>
#include <vector>

struct SPIBusConfig
{
    int8_t mosi;
    int8_t sclk;
    int8_t miso;
    int8_t wrpt;
    int8_t hold;
    uint16_t maxSize;

    bool operator== (const SPIBusConfig& config) const
    {
        return config.hold == hold
            && config.maxSize == maxSize
            && config.miso == miso
            && config.mosi == mosi
            && config.sclk == sclk
            && config.wrpt == wrpt;
    }
};

struct SPITuple
{
    uint8_t cmd;
    std::vector<uint8_t> data;

    bool operator== (const SPITuple& cmdSeq) const
    {
        return cmd == cmdSeq.cmd && data.size() == cmdSeq.data.size();
    }
};

class SPI
{
    public:
        SPI() = default;
        ~SPI() = default;
        virtual void Send(uint8_t *payload, size_t len, bool isCmd = false) = 0;
};
