#include "utils.h"

#define I2C_RECOVERY_CLOCKS 9

void i2cBusRecovery(uint sda_pin, uint scl_pin) {
    gpio_init(sda_pin);
    gpio_init(scl_pin);

    gpio_set_dir(sda_pin, GPIO_IN);
    gpio_set_dir(scl_pin, GPIO_OUT);

    // Attempt to clock out any stuck slave devices
    for (int i = 0; i < I2C_RECOVERY_CLOCKS; ++i) {
        gpio_put(scl_pin, 0);
        busy_wait_us(5); // Clock low period
        gpio_put(scl_pin, 1);
        busy_wait_us(5); // Clock high period

        // Check if SDA line has been released by slave
        if (gpio_get(sda_pin)) {
            break; // SDA line is high; assume bus is free
        }
    }

    // Generate a stop condition in case a slave is still in the middle of a transaction
    gpio_set_dir(sda_pin, GPIO_OUT);
    gpio_put(scl_pin, 0);
    busy_wait_us(5);
    gpio_put(sda_pin, 0);
    busy_wait_us(5);
    gpio_put(scl_pin, 1);
    busy_wait_us(5);
    gpio_put(sda_pin, 1);
    busy_wait_us(5);
}

void initI2C(bool force_recovery) {
    // Attempt I2C bus recovery before de-initializing pins
    if (force_recovery) {
        i2cBusRecovery(CONFIG::I2C_SDA_PIN, CONFIG::I2C_SCL_PIN);
    }
    // de-initialise I2C pins in case
    gpio_disable_pulls(CONFIG::I2C_SDA_PIN);
    gpio_set_function(CONFIG::I2C_SDA_PIN, GPIO_FUNC_NULL);
    gpio_disable_pulls(CONFIG::I2C_SCL_PIN);
    gpio_set_function(CONFIG::I2C_SCL_PIN, GPIO_FUNC_NULL);

    // initialise I2C with baudrate
    i2c_init(CONFIG::i2c_port, CONFIG::I2C_BAUD_RATE);

    gpio_set_function(CONFIG::I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(CONFIG::I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(CONFIG::I2C_SDA_PIN);
    gpio_pull_up(CONFIG::I2C_SCL_PIN);
}

int ping() {
    if (!CONFIG::i2c_port) {
        return kSTkErrBusNotInit;
    }

    uint8_t data = 0;
    int result = i2c_write_blocking(CONFIG::i2c_port, kDefaultAddress, &data, 1, false);

    return (result >= 0) ? kSTkErrOk : kSTkErrFail;
}

int readRegisterByte(uint8_t devReg, uint8_t &dataToRead) {
    if (!CONFIG::i2c_port) {
        return kSTkErrBusNotInit;
    }

    // Envoyer l'adresse du registre
    int result = i2c_write_blocking(CONFIG::i2c_port, kDefaultAddress, &devReg, 1, true);
    if (result < 0) {
        return kSTkErrFail;
    }

    // Lire le byte à partir du registre
    result = i2c_read_blocking(CONFIG::i2c_port, kDefaultAddress, &dataToRead, 1, false);
    return (result == 1) ? kSTkErrOk : kSTkErrFail;
}

int readRegisterRegionAnyAddress(uint8_t *devReg, size_t regLength, uint8_t *data, size_t numBytes, size_t &readBytes) {
    if (!CONFIG::i2c_port)
        return kSTkErrBusNotInit;

    if (!data)
        return kSTkErrBusNullBuffer;

    readBytes = 0;
    size_t nOrig = numBytes;
    uint8_t nChunk;
    int nReturned;
    bool firstIteration = true;

    while (numBytes > 0) {
        if (firstIteration) {
            int result = i2c_write_blocking(CONFIG::i2c_port, kDefaultAddress, devReg, regLength, true);
            if (result < 0)
                return kSTkErrFail;
            firstIteration = false;
        }

        nChunk = numBytes > kDefaultBufferChunk ? kDefaultBufferChunk : numBytes;
        nReturned = i2c_read_blocking(CONFIG::i2c_port, kDefaultAddress, data, nChunk, numBytes > nChunk);

        if (nReturned < 0)
            return kSTkErrFail;

        numBytes -= nReturned;
        data += nReturned;
    }

    readBytes = nOrig - numBytes;
    return (readBytes == nOrig) ? kSTkErrOk : kSTkErrBusUnderRead;
}

int readRegisterRegion(uint8_t devReg, uint8_t *data, size_t numBytes, size_t &readBytes)
{
    return readRegisterRegionAnyAddress(&devReg, 1, data, numBytes, readBytes);
}

int writeRegisterByte(uint8_t devReg, uint8_t dataToWrite) {
    if (!CONFIG::i2c_port)
        return kSTkErrBusNotInit;

    uint8_t buffer[2];
    buffer[0] = devReg;       
    buffer[1] = dataToWrite;  

    int result = i2c_write_blocking(CONFIG::i2c_port, kDefaultAddress, buffer, 2, false);

    return result == 2 ? kSTkErrOk : kSTkErrFail;
}

int writeRegisterRegionAddress(uint8_t *devReg, size_t regLength, const uint8_t *data, size_t length) {
    if (!CONFIG::i2c_port)
        return kSTkErrBusNotInit;

    uint8_t buffer[regLength + length];

    if (devReg != nullptr && regLength > 0) {
        memcpy(buffer, devReg, regLength);
    }

    memcpy(buffer + regLength, data, length);

    int result = i2c_write_blocking(CONFIG::i2c_port, kDefaultAddress, buffer, regLength + length, false);

    return result == (int)(regLength + length) ? kSTkErrOk : kSTkErrFail;
}

int writeRegisterRegion(uint8_t devReg, const uint8_t *data, size_t length)
{
    return writeRegisterRegionAddress(&devReg, 1, data, length);
}
