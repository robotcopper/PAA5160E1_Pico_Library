#ifndef UTILS_H
#define UTILS_H

#include <cstring>
#include "hardware/i2c.h"
#include "hardware/gpio.h"

namespace CONFIG {

    constexpr uint I2C_BAUD_RATE = 350 * 1000; // 350kHz
    constexpr uint I2C_SDA_PIN = PICO_DEFAULT_I2C_SDA_PIN; // GP4 pin 6;
    constexpr uint I2C_SCL_PIN = PICO_DEFAULT_I2C_SCL_PIN; // GP5 pin 7;

    constexpr i2c_inst_t *i2c_port = i2c0;

}

/// @brief Default I2C addresses of the Qwiic OTOS
static constexpr uint8_t kDefaultAddress = 0x17;

static constexpr int kSTkErrOk = 0;
static constexpr int kSTkErrFail = -1;

const int kSTkErrBaseBus = 0x1000;
const int kSTkErrBusNotInit = kSTkErrFail * (kSTkErrBaseBus + 1);
const int kSTkErrBusNullBuffer = kSTkErrFail * (kSTkErrBaseBus + 6);
const int kSTkErrBusUnderRead = kSTkErrBaseBus + 7;
static constexpr size_t kDefaultBufferChunk = 32;

void i2cBusRecovery(uint sda_pin, uint scl_pin);
void initI2C(bool force_recovery);


int ping();
int readRegisterByte(uint8_t devReg, uint8_t &dataToRead);
int readRegisterRegionAnyAddress(uint8_t *devReg, size_t regLength, uint8_t *data, size_t numBytes, size_t &readBytes);
int readRegisterRegion(uint8_t devReg, uint8_t *data, size_t numBytes, size_t &readBytes);
int writeRegisterByte(uint8_t devReg, uint8_t dataToWrite);
int writeRegisterRegionAddress(uint8_t *devReg, size_t regLength, const uint8_t *data, size_t length);
int writeRegisterRegion(uint8_t devReg, const uint8_t *data, size_t length);

#endif // UTILS_H