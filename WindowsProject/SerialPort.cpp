#include "SerialPort.h"
#include <iostream>

SerialPort::SerialPort(const std::string& portName)
    : hSerial(INVALID_HANDLE_VALUE), portName("\\\\.\\" + portName), connected(false) {
}

SerialPort::~SerialPort() {
    close();
}

bool SerialPort::open() {
    hSerial = CreateFileA(
        portName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Erro ao abrir porta " << portName << "\n";
        return false;
    }

    if (!configurePort()) return false;
    configureTimeouts();

    connected = true;
    return true;
}

void SerialPort::close() {
    if (connected) {
        CloseHandle(hSerial);
        connected = false;
    }
}

bool SerialPort::configurePort() {
    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(dcb);

    if (!GetCommState(hSerial, &dcb)) {
        std::cerr << "Erro ao obter estado da porta.\n";
        return false;
    }

    dcb.BaudRate = CBR_19200;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcb)) {
        std::cerr << "Erro ao configurar DCB.\n";
        return false;
    }

    return true;
}

void SerialPort::configureTimeouts() {
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hSerial, &timeouts);
}

bool SerialPort::write(const std::string& data) {
    DWORD bytesSent;
    return WriteFile(hSerial, data.c_str(), data.length(), &bytesSent, NULL);
}

std::string SerialPort::read(size_t numBytes) {
    char* buffer = new char[numBytes + 1];
    DWORD bytesRead;
    ReadFile(hSerial, buffer, numBytes, &bytesRead, NULL);
    buffer[bytesRead] = '\0';

    std::string result(buffer);
    delete[] buffer;
    return result;
}

bool SerialPort::isConnected() const {
    return connected;
}

std::vector<std::string> SerialPort::listAvailablePorts() {
    std::vector<std::string> availablePorts;

    for (int i = 1; i <= 255; ++i) {
        std::string port = "\\\\.\\COM" + std::to_string(i);

        HANDLE h = CreateFileA(
            port.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
        );

        if (h != INVALID_HANDLE_VALUE) {
            // A porta está disponível
            availablePorts.push_back("COM" + std::to_string(i));
            CloseHandle(h);
        }
    }

    return availablePorts;
}
