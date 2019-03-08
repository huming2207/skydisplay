#pragma once

#include <cstdint>

class Display
{
    public:
        virtual void SelectArea(uint16_t xStart, uint16_t xEnd, uint16_t yStart, uint16_t yEnd) = 0;
        virtual void WritePixel(uint16_t color) = 0;
        virtual void ClearFrameBuf() = 0;
};
