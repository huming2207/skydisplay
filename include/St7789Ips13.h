//
// Created by hu on 28/02/19.
//

#ifndef GOLDEN_LOCK_ST7789IPS13_H
#define GOLDEN_LOCK_ST7789IPS13_H


#include "Display.hpp"

class St7789Ips13 : public Display
{
public:
    void SelectArea(uint16_t xStart, uint16_t xEnd, uint16_t yStart, uint16_t yEnd);
    void WritePixel(uint16_t color);
    void ClearFrameBuf();


private:
    void
};


#endif //GOLDEN_LOCK_ST7789IPS13_H
