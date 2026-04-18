#include "i2c_controller.h"
#include "app_logger.h"
#include "hardware_config.h"

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <chrono>
#include <cstdint>
#include <sstream>
#include <string>
#include <thread>

I2cController::I2cController()
    : fd_(-1), door_open_(false), active_pos_(Position::N0)
{}

I2cController::~I2cController() {
    Deactivate();
    if (fd_ >= 0)
        close(fd_);
}

// Escribe un único registro de 8 bits al XL9535.
static bool WriteReg(int fd, uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    return write(fd, buf, 2) == 2;
}

// Reintenta la escritura hasta kI2cRetries veces con un delay entre intentos.
static bool WriteRegRetry(int fd, uint8_t reg, uint8_t value) {
    for (int i = 0; i < kI2cRetries; ++i) {
        if (WriteReg(fd, reg, value))
            return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(kI2cRetryDelayMs));
    }
    std::ostringstream oss;
    oss << "WriteReg failed: reg=0x" << std::hex << (int)reg
        << " tras " << std::dec << kI2cRetries << " intentos";
    AppLogger::Error("I2C", oss.str());
    return false;
}

bool I2cController::Init() {
    std::string dev = "/dev/i2c-" + std::to_string(kI2cBus);
    fd_ = open(dev.c_str(), O_RDWR);
    if (fd_ < 0)
        return false;

    if (ioctl(fd_, I2C_SLAVE, kI2cAddress) < 0) {
        close(fd_);
        fd_ = -1;
        return false;
    }

    // Warmup: el primer write tras ioctl falla en algunos drivers, se ignora.
    WriteReg(fd_, kRegOutputPort0, 0x00);

    // Configurar todos los pines como output (ignoramos error — algunos
    // clones del XL9535 no permiten escribir estos registros pero arrancan
    // en modo output por defecto).
    WriteRegRetry(fd_, kRegConfigPort0, 0x00);
    WriteRegRetry(fd_, kRegConfigPort1, 0x00);

    // Pre-set outputs a LOW.
    WriteRegRetry(fd_, kRegOutputPort0, 0x00);
    WriteRegRetry(fd_, kRegOutputPort1, 0x00);

    return true;
}

void I2cController::WriteState(uint16_t state) {
    WriteRegRetry(fd_, kRegOutputPort0, static_cast<uint8_t>(state & 0xFF));
    WriteRegRetry(fd_, kRegOutputPort1, static_cast<uint8_t>(state >> 8));
}

uint16_t I2cController::DoorBit() const {
    return door_open_ ? (1u << kDoorPin) : 0u;
}

void I2cController::Activate(Position pos) {
    if (active_pos_ != Position::N0)
        return;

    int idx     = static_cast<int>(pos) - 1;
    int row_pin = kRowPins[idx / 8];
    int col_pin = kColPins[idx % 8];

    WriteState(DoorBit() | (1u << row_pin) | (1u << col_pin));
    active_pos_ = pos;
}

void I2cController::Deactivate() {
    if (active_pos_ == Position::N0)
        return;

    int idx     = static_cast<int>(active_pos_) - 1;
    int col_pin = kColPins[idx % 8];

    WriteState(DoorBit() | (1u << col_pin));
    std::this_thread::sleep_for(std::chrono::milliseconds(kRelayOffDelayMs));
    WriteState(DoorBit());
    active_pos_ = Position::N0;
}

void I2cController::OpenDoor() {
    door_open_ = true;

    uint16_t state = DoorBit();
    if (active_pos_ != Position::N0) {
        int idx = static_cast<int>(active_pos_) - 1;
        state |= (1u << kRowPins[idx / 8]);
        state |= (1u << kColPins[idx % 8]);
    }
    WriteState(state);
}

void I2cController::CloseDoor() {
    door_open_ = false;

    uint16_t state = 0u;
    if (active_pos_ != Position::N0) {
        int idx = static_cast<int>(active_pos_) - 1;
        state |= (1u << kRowPins[idx / 8]);
        state |= (1u << kColPins[idx % 8]);
    }
    WriteState(state);
}

void I2cController::RunRelayTest() {
    for (int pin = 0; pin < 16; ++pin) {
        WriteState(1u << pin);
        std::this_thread::sleep_for(std::chrono::milliseconds(kRelayTestDwellMs));
        WriteState(0u);
        std::this_thread::sleep_for(std::chrono::milliseconds(kRelayTestDwellMs));
    }
}
