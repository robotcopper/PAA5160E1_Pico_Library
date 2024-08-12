#pragma once

#include "sfeQwiicOtos.h"
#include "utils.h"
#include "pico/stdlib.h"

class QwiicOTOS : public sfeQwiicOtos
{
  protected:
    void delayMs(uint32_t ms) {
        sleep_ms(ms); 
    }
};