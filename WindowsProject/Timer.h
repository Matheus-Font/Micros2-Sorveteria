#ifndef TIMER_H
#define TIMER_H
#include "resource.h"
#include <windows.h>
#include "SerialPort.h"
#include "SimulatorProtocol.h" // Inclui o protocolo de simulação para comunicação com o Arduino

typedef struct Saidas {
    bool nivel1, nivel2, nivel3, nivelSaida, poteEsteira, encherpote;
    uint16_t temperatura, ph;
} Saidas;

void HandleTimer(HWND hDlg, WPARAM wParam, LPARAM lParam);

#endif
