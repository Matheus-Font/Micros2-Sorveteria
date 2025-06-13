#include "SimulatorProtocol.h"
#include <iostream>

SimulatorProtocol::SimulatorProtocol(SerialPort& serial) : serial(serial) {}

bool SimulatorProtocol::sendBytes(const std::vector<uint8_t>& data) {
    std::string bytes(data.begin(), data.end());
    return serial.write(bytes);
}

//===============================================================================================
bool SimulatorProtocol::receiveBytes(std::vector<uint8_t>& buffer, size_t length) {
    std::string raw = serial.read(length);
    char dbg[100];
    sprintf_s(dbg, "Tentando ler %zu bytes. Recebido: %zu\n", length, raw.size());
    OutputDebugStringA(dbg);

    if (raw.size() < length) return false;
    buffer.assign(raw.begin(), raw.begin() + length);
    return true;
}
//===============================================================================================

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

//===============================================================================================
bool SimulatorProtocol::readData(uint8_t& digitalOut, std::vector<uint16_t>& analogOut, uint8_t nAnalog) {
    // 1. Monta o pacote 'P' 'R' n
    std::vector<uint8_t> packet = { 'P', 'R', nAnalog };
    if (!sendBytes(packet)) {
        OutputDebugStringA("[DEBUG] Falha ao enviar comando PR\n");
        return false;
    }

    // 2. Calcula número total de bytes esperados na resposta
    size_t totalBytes = nAnalog * 2 + 4;
    std::vector<uint8_t> response;

    if (!receiveBytes(response, totalBytes)) {
        char dbg[64];
        sprintf_s(dbg, "[DEBUG] Falha ao ler %zu bytes da resposta\n", totalBytes);
        OutputDebugStringA(dbg);
        return false;
    }

    // 3. Mostra a resposta completa recebida
    char dbgResp[128];
    sprintf_s(dbgResp, "[DEBUG] Recebido: %02X %02X %02X %02X\n",
        response[0], response[1], response[2], response[3]);
    OutputDebugStringA(dbgResp);

    // 4. Validação de cabeçalho da resposta
    if (response[0] != 'A' || response[1] != 'R' || response[2] != nAnalog) {
        OutputDebugStringA("[DEBUG] Erro no cabeçalho da resposta.\n");

        if (response[0] != 'A') OutputDebugStringA("? response[0] != 'A'\n");
        if (response[1] != 'R') OutputDebugStringA("? response[1] != 'R'\n");

        char dbg[64];
        sprintf_s(dbg, "? response[2] = %02X, esperado: %02X\n", response[2], nAnalog);
        OutputDebugStringA(dbg);

        return false;
    }

    // 5. Interpreta os dados
    digitalOut = response[3];
    analogOut.clear();

    for (size_t i = 0; i < nAnalog; ++i) {
        size_t idx = 4 + i * 2;
        if (idx + 1 >= response.size()) {
            OutputDebugStringA("[DEBUG] Resposta analógica incompleta\n");
            return false;
        }

        uint16_t val = (response[idx] << 8) | response[idx + 1]; // MSB + LSB
        analogOut.push_back(val);
    }

    return true;
}
//===============================================================================================