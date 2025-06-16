#ifndef TIMER_H
#define TIMER_H
#include "resource.h"
#include <windows.h>
#include "SerialPort.h"
#include "SimulatorProtocol.h" // Inclui o protocolo de simulação para comunicação com o Arduino

// Estrutura de controle para variáveis de saída
typedef struct Entradas {
    bool valvulaCreme;
    bool valvulaMorango;
    bool valvulaChocolate;
    bool valvulaCongelamento;
    bool valvulaMixer;
	bool esteira;
    uint16_t temperatura;
    uint16_t ph;
} Saidas;

void HandleTimer(HWND hDlg, WPARAM wParam, LPARAM lParam);

#endif
