#pragma once

#include "SerialPort.h"
#include <vector>
#include <cstdint>

class SimulatorProtocol {
public:
    SimulatorProtocol(SerialPort& serial);

    bool sendData(uint8_t digital, const std::vector<uint16_t>& analog);
    bool readData(uint8_t& digitalOut, std::vector<uint16_t>& analogOut, uint8_t nAnalog);

private:
    SerialPort& serial;

    bool sendBytes(const std::vector<uint8_t>& data);
    bool receiveBytes(std::vector<uint8_t>& buffer, size_t length);
};