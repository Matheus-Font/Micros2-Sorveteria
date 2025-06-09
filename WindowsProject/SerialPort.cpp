#pragma once 

#include "SerialPort.h"
#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <initguid.h>
#include <devguid.h>   // GUID_DEVCLASS_PORTS
#include <setupapi.h>
#include <vector>
#include <string>

#pragma comment(lib, "setupapi.lib")

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
    std::vector<std::string> ports;

    HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, NULL, NULL, DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE) return ports;

    SP_DEVINFO_DATA devInfoData;
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); ++i) {
        char buffer[256];
        if (SetupDiGetDeviceRegistryPropertyA(
            hDevInfo, &devInfoData, SPDRP_FRIENDLYNAME,
            NULL, (PBYTE)buffer, sizeof(buffer), NULL)) {

            std::string name(buffer);

            size_t start = name.find("COM");
            if (start != std::string::npos) {
                size_t end = name.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", start);
                std::string comPort = name.substr(start, end - start);
                ports.push_back(comPort);
            }
        }
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    return ports;
}

