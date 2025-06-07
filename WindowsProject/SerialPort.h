#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <windows.h>
#include <string>
#include <vector>

class SerialPort {
public:
    SerialPort(const std::string& portName);
    ~SerialPort();

    bool open();
    void close();

    bool write(const std::string& data);
    std::string read(size_t numBytes);

    bool isConnected() const;

    // Novo método estático
    static std::vector<std::string> listAvailablePorts();

private:
    HANDLE hSerial;
    std::string portName;
    bool connected;

    bool configurePort();
    void configureTimeouts();
};

#endif // SERIALPORT_H
