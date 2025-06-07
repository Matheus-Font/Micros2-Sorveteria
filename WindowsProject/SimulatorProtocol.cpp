#include "SimulatorProtocol.h"
#include <iostream>

SimulatorProtocol::SimulatorProtocol(SerialPort& serial) : serial(serial) {}

bool SimulatorProtocol::sendBytes(const std::vector<uint8_t>& data) {
    std::string bytes(data.begin(), data.end());
    return serial.write(bytes);
}

bool SimulatorProtocol::receiveBytes(std::vector<uint8_t>& buffer, size_t length) {
    std::string raw = serial.read(length);
    if (raw.size() < length) return false;
    buffer.assign(raw.begin(), raw.begin() + length);
    return true;
}

bool SimulatorProtocol::sendData(uint8_t digital, const std::vector<uint16_t>& analog) {
    uint8_t n = analog.size();
    std::vector<uint8_t> packet;

    packet.push_back('P');
    packet.push_back('W');
    packet.push_back(n);

    // Digital
    packet.push_back(digital);

    // Analógicos
    for (uint16_t val : analog) {
        packet.push_back(val & 0xFF);
        packet.push_back((val >> 8) & 0xFF);
    }

    if (!sendBytes(packet)) return false;

    // Espera resposta "A", "W"
    std::vector<uint8_t> response;
    if (!receiveBytes(response, 2)) return false;
    return (response[0] == 'A' && response[1] == 'W');
}

bool SimulatorProtocol::readData(uint8_t& digitalOut, std::vector<uint16_t>& analogOut, uint8_t nAnalog) {
    std::vector<uint8_t> packet = { 'P', 'R', nAnalog };
    if (!sendBytes(packet)) return false;

    // Espera n*2 + 4 bytes de resposta
    size_t totalBytes = nAnalog * 2 + 4;
    std::vector<uint8_t> response;
    if (!receiveBytes(response, totalBytes)) return false;

    if (response[0] != 'A' || response[1] != 'R' || response[2] != nAnalog) return false;

    digitalOut = response[3];

    analogOut.clear();
    for (size_t i = 0; i < nAnalog; i++) {
        size_t idx = 4 + i * 2;
        uint16_t val = response[idx] | (response[idx + 1] << 8);
        analogOut.push_back(val);
    }

    return true;
}
