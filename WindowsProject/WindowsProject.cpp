// Projeto 1 Microcontroladores II - Simulador de Sorveteria
// Integrantes: Giulia Colombo, Matheus Fontenele e Natã de Souza

/* Includes */

#include "atualiza.h" // Para atualizar os valores de temperatura, pH, volumes, etc.
#include "Timer.h" // Para o tratamento de timers
#include "framework.h" // Inclui o arquivo de cabeçalho principal do projeto
#include "WindowsProject.h" // Inclui o arquivo de cabeçalho do projeto
#include "SimulatorProtocol.h" // Inclui o protocolo de simulação para comunicação com o Arduino
#include "SerialPort.h" // Inclui a classe SerialPort para comunicação serial
#include <CommCtrl.h> // Para usar controles comuns, como barras de progresso, botões, etc.
#include <thread> // Para usar threads
#include <chrono> // Para usar funções de tempo, como sleep_for
#include <atomic> // Para usar variáveis atômicas, como executando

/* Defines */
#define MAX_LOADSTRING 100


/* Global Variables */

// Volumes tanques e pote
float volumeCreme = 30;
float volumeChocolate = 30;
float volumeMorango = 30;
float volumeMixer = 0;
float volumeCongelamento = 0;
float volumePote = 0;
// PHs tanques 
float phMixer = 0.0f;
int phCreme = 7; // pH do tanque creme
int phMorango = 13; // pH do tanque morango
int phChocolate = 5; // pH do tanque chocolate
// Temperaturas tanques
int temperaturaCreme = 30; // Temperatura do tanque creme
int temperaturaMorango = 60; // Temperatura do tanque morango
int temperaturaChocolate = 80; // Temperatura do tanque chocolate
int temperaturaMixer = 0; // Temperatura inicial do mixer
// Variáveis selecionadas
int tempTargetMixer = -1; // ALVO PARA PRIMEIRA TEMPERATURA DO MIXER
float phTargetMixer = -1.0f;
int tempTargetCongelamento = 0; // ALVO PARA PRIMEIRA TEMPERATURA DO CONGELAMENTO
int tempcongelamento = 0; // Temperatura do congelamento 
int tanqueSelecionado = 1; // Variável para armazenar o tanque selecionado (1, 2 ou 3)
// Timers
int tempoCreme = 0; // Tempos para calcular a COR do mixer
int tempoMorango = 0;
int tempoChocolate = 0;
// Cores do mixer
float mixerR = 0.0f, mixerB = 0.0f, mixerG = 0.0f;
// Cores congelado
float congeladoR = 0, congeladoG = 0, congeladoB = 0;
// Cores alvo
float targetR = 0.0f, targetG = 0.0f, targetB = 0.0f;
bool primeiraCorMixer = true;

// Cores padrão tanques base
const int R_CREME = 255, G_CREME = 255, B_CREME = 100;
const int R_MORANGO = 255, G_MORANGO = 80, B_MORANGO = 80;
const int R_CHOCOLATE = 123, G_CHOCOLATE = 63, B_CHOCOLATE = 0;

bool mostrarMix1 = true;

// Variável para indicar se o pote está abaixo do mixer
bool poteAbaixoDoMixer = false;

// Porta serial
char serialPort[100] = "COM1"; // Porta serial padrão (pode ser alterada pelo usuário)

// Thread para leitura contínua do protocolo
std::thread serialThread;

// Estrutura de controle para variáveis de entrada e saída
Entradas entradas;

// Variáveis globais para a instância do aplicativo e strings de título
HINSTANCE hInst;                                // instância atual
WCHAR szTitle[MAX_LOADSTRING];                  // O texto da barra de título
WCHAR szWindowClass[MAX_LOADSTRING];            // o nome da classe da janela principal

// Variáveis globais para a porta serial e protocolo
SerialPort* g_serial = nullptr;
SimulatorProtocol* g_protocolo = nullptr;

// Declarações de funções
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
INT_PTR CALLBACK    MainDialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    WelcomeDialogProc(HWND, UINT, WPARAM, LPARAM);

// Função para leitura continua do protocolo
std::atomic<bool> executando(true);

/* Functions */
void TestaComandoPR() {
    if (!g_protocolo) {
        OutputDebugStringA("g_protocolo não está inicializado!\n");
        return;
    }

    uint8_t digital;
    std::vector<uint16_t> analog;

    if (g_protocolo->readData(digital, analog, 0)) {
        char msg[64];
        sprintf_s(msg, "[TESTE PR] digitalIn = 0x%02X\n", digital);
        OutputDebugStringA(msg);
    }
    else {
        OutputDebugStringA("[TESTE PR] Falha na leitura da serial (readData)\n");
    }
}

void loopAtualizacao(SerialPort& serial, Entradas& entradas) {
    SimulatorProtocol protocolo(serial);

    while (executando.load()) {
        // 1. Monta byte saidas digitais
		uint8_t digitalOut = 0;
        // Indicadores tanque vazio
        digitalOut |= volumeCreme <  5.0f ? (1 << 0) : 0;
        digitalOut |= volumeMorango < 5.0f ? (1 << 1) : 0;
        digitalOut |= volumeChocolate < 5.0f ? (1 << 2) : 0;
        digitalOut |= volumeCongelamento < 5.0f ? (1 << 3) : 0;
		// Indicador de pote abaixo do mixer
        digitalOut |= poteAbaixoDoMixer ? (1 << 4) : 0;

        // 2. Montar bits analogicos
        std::vector<uint16_t> analogOut = { };
        // 3. Enviar dados
        protocolo.sendData(digitalOut, analogOut);

        // 3. Ler resposta
        uint8_t digitalIn;
        std::vector<uint16_t> analogIn;
        if (protocolo.readData(digitalIn, analogIn, 1)) {
			entradas.valvulaCreme = digitalIn & (1 << 0);
			entradas.valvulaMorango = digitalIn & (1 << 1);
			entradas.valvulaChocolate = digitalIn & (1 << 2);
            entradas.valvulaMixer = digitalIn & (1 << 3);
            entradas.valvulaCongelamento = digitalIn & (1 << 4);
			entradas.esteira = digitalIn & (1 << 5);

            // TO TEST:
            // Lê temperatura alvo para congelamento
            tempTargetCongelamento = analogIn[0];

            // DEBUG: imprime todos os bits recebidos
            char debug[100];
            sprintf_s(debug, sizeof(debug), "digitalIn: %02X (bin: %d%d%d%d%d%d)", digitalIn,
                (digitalIn & 0x20) ? 1 : 0,
                (digitalIn & 0x10) ? 1 : 0,
                (digitalIn & 0x08) ? 1 : 0,
                (digitalIn & 0x04) ? 1 : 0,
                (digitalIn & 0x02) ? 1 : 0,
                (digitalIn & 0x01) ? 1 : 0);

            OutputDebugStringA(debug);
            OutputDebugStringA("\n");
        }

		// Delay 5 ms (intervalo entre as mensagens)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
 }

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    hInst = hInstance;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT));
    MSG msg;

    while (true)
    {
        INT_PTR welcomeResult = DialogBox(hInstance, MAKEINTRESOURCE(IDD_WELCOME), NULL, WelcomeDialogProc);
        if (welcomeResult == -1) {
            MessageBox(NULL, L"Erro ao abrir a janela de boas-vindas", L"ERRO", MB_OK | MB_ICONERROR);
            break;
        }
        if (welcomeResult == IDCANCEL)
            break; // cancela = sair

        while (true)
        {
            INT_PTR mainResult = DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDialogProc);

            if (mainResult == IDCANCEL || mainResult == -1)
                goto fim;
            else if (mainResult == 42)
                break; // volta à tela de boas-vindas
        }
    }
    
fim:
    return 0;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICONE));
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICONE));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDialogProc);

    if (!hDlg)
        return FALSE;

    ShowWindow(hDlg, nCmdShow);
    UpdateWindow(hDlg);

    return TRUE;
}

INT_PTR CALLBACK MainDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        // Ícones da janela
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONE)));
        SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONE)));

        // Botão de Temperatura / pH
        CheckDlgButton(hDlg, IDC_RADIOTEMP, BST_CHECKED);
        AtualizaValorTempPh(hDlg);

        // Inicializa os valores de temperatura nos tanques
        WCHAR buffer[16];
        swprintf(buffer, 16, L"%d(C)", temperaturaMorango);
        SetDlgItemText(hDlg, IDC_TPHMORANGO, buffer);
        swprintf(buffer, 16, L"%d(C)", temperaturaCreme);
        SetDlgItemText(hDlg, IDC_TPHCREME, buffer);
        swprintf(buffer, 16, L"%d(C)", temperaturaChocolate);
        SetDlgItemText(hDlg, IDC_TPHCHOCOLATE, buffer);
        swprintf(buffer, 16, L"%d(C)", temperaturaMixer);
        SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);
        swprintf(buffer, 16, L"%d(C)", tempcongelamento);
        SetDlgItemText(hDlg, IDC_TPHCONGELAMENTO, buffer);

        // Barra de progresso externa (congelamento)
        SendDlgItemMessage(hDlg, IDC_PROGRESSTEMP, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
        SendDlgItemMessage(hDlg, IDC_PROGRESSTEMP, PBM_SETPOS, tempcongelamento, 0);
        HWND hProg = GetDlgItem(hDlg, IDC_PROGRESSTEMP);
        SendMessage(hProg, PBM_SETBARCOLOR, 0, (LPARAM)RGB(255, 0, 0));

        // Creme
        SendDlgItemMessage(hDlg, IDC_CREME, PBM_SETRANGE, 0, MAKELPARAM(0, 30));
        SendDlgItemMessage(hDlg, IDC_CREME, PBM_SETPOS, volumeCreme, 0);
        HWND hProgCreme = GetDlgItem(hDlg, IDC_CREME);
        SendMessage(hProgCreme, PBM_SETBARCOLOR, 0, (LPARAM)RGB(R_CREME, G_CREME, B_CREME));
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCREME), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCREME), SW_HIDE);

        // Morango
        SendDlgItemMessage(hDlg, IDC_MORANGO, PBM_SETRANGE, 0, MAKELPARAM(0, 30));
        SendDlgItemMessage(hDlg, IDC_MORANGO, PBM_SETPOS, volumeMorango, 0);
        HWND hProgMorango = GetDlgItem(hDlg, IDC_MORANGO);
        SendMessage(hProgMorango, PBM_SETBARCOLOR, 0, (LPARAM)RGB(R_MORANGO, G_MORANGO, B_MORANGO));
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFMORANGO), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONMORANGO), SW_HIDE);

        // Chocolate
        SendDlgItemMessage(hDlg, IDC_CHOCOLATE, PBM_SETRANGE, 0, MAKELPARAM(0, 30));
        SendDlgItemMessage(hDlg, IDC_CHOCOLATE, PBM_SETPOS, volumeChocolate, 0);
        HWND hProgChocolate = GetDlgItem(hDlg, IDC_CHOCOLATE);
        SendMessage(hProgChocolate, PBM_SETBARCOLOR, 0, (LPARAM)RGB(R_CHOCOLATE, G_CHOCOLATE, B_CHOCOLATE));
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCHOCOLATE), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCHOCOLATE), SW_HIDE);

        // Mixer
        SendDlgItemMessage(hDlg, IDC_PROGRESSMIXER, PBM_SETRANGE, 0, MAKELPARAM(0, 70));
        SendDlgItemMessage(hDlg, IDC_PROGRESSMIXER, PBM_SETPOS, volumeMixer, 0);

        // Congelamento
        SendDlgItemMessage(hDlg, IDC_PROGRESSCONGELAMENTO, PBM_SETRANGE, 0, MAKELPARAM(0, 40));
        SendDlgItemMessage(hDlg, IDC_PROGRESSCONGELAMENTO, PBM_SETPOS, volumeCongelamento, 0);

        // Pote
        SendDlgItemMessage(hDlg, IDC_PROGRESSPOTE, PBM_SETRANGE, 0, MAKELPARAM(0, 2.5));
        SendDlgItemMessage(hDlg, IDC_PROGRESSPOTE, PBM_SETPOS, (int)volumePote, 0);
        HWND hPote = GetDlgItem(hDlg, IDC_POTE);
        HWND hBarra = GetDlgItem(hDlg, IDC_PROGRESSPOTE);
        ShowWindow(GetDlgItem(hDlg, IDC_PROGRESSPOTE), SWP_SHOWWINDOW);

        // Comunicação Serial
        g_serial = new SerialPort(serialPort);
        if (g_serial->open()) {
            OutputDebugStringA("Porta aberta com sucesso!\n");
            g_protocolo = new SimulatorProtocol(*g_serial);
            executando = true;
            serialThread = std::thread(loopAtualizacao, std::ref(*g_serial), std::ref(entradas));
        }
        else {
            OutputDebugStringA("Erro ao abrir porta serial.\n");
        }

        SetTimer(hDlg, 1, 10, NULL);
        AtualizaVolumes(hDlg);
        AtualizaValorTempPh(hDlg);

        return (INT_PTR)TRUE;
    }

    case WM_TIMER:
        HandleTimer(hDlg, wParam, lParam);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_RADIOTEMP:
        case IDC_RADIOPH:
            AtualizaValorTempPh(hDlg);
            break;

        case IDC_CHECKCREME:
        {
            BOOL checked = IsDlgButtonChecked(hDlg, IDC_CHECKCREME) == BST_CHECKED || entradas.valvulaCreme;
            if (tempTargetMixer == -1 && volumeMixer == 0.0f)
                temperaturaMixer = temperaturaCreme;
            tempTargetMixer = temperaturaCreme;
        }
        break;

        case IDC_CHECKMORANGO:
        {
            BOOL checked = IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO) == BST_CHECKED || entradas.valvulaMorango;
            if (tempTargetMixer == -1 && volumeMixer == 0.0f)
                temperaturaMixer = temperaturaMorango;
            tempTargetMixer = temperaturaMorango;
        }
        break;

        case IDC_CHECKCHOCOLATE:
        {
            BOOL checked = IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE) == BST_CHECKED || entradas.valvulaChocolate;
            if (tempTargetMixer == -1 && volumeMixer == 0.0f)
                temperaturaMixer = temperaturaChocolate;
            tempTargetMixer = temperaturaChocolate;
        }
        break;

        case IDC_CHECKMIXER:
        {
            BOOL checked = IsDlgButtonChecked(hDlg, IDC_CHECKMIXER) == BST_CHECKED || entradas.valvulaMixer;
            if (checked) {
                tempcongelamento = temperaturaMixer;
                tempTargetCongelamento = temperaturaMixer;

                WCHAR buffer[16];
                swprintf(buffer, 16, L"%d(\x00B0\C)", tempcongelamento);
                SetDlgItemText(hDlg, IDC_TPHCONGELAMENTO, buffer);
            }
            else {
                tempTargetCongelamento = -25;
            }
        }
        break;

        case IDC_CHECKCONGELADO:
        {
            BOOL checked = (IsDlgButtonChecked(hDlg, IDC_CHECKCONGELADO) == BST_CHECKED || entradas.esteira);
        }
        break;

        case IDC_CHECKRESETVOLUMES:
        {
            if (IsDlgButtonChecked(hDlg, IDC_CHECKRESETVOLUMES) == BST_CHECKED)
            {
                volumeCreme = 30.0f;
                volumeMorango = 30.0f;
                volumeChocolate = 30.0f;

                SendDlgItemMessage(hDlg, IDC_CREME, PBM_SETPOS, (int)volumeCreme, 0);
                SendDlgItemMessage(hDlg, IDC_MORANGO, PBM_SETPOS, (int)volumeMorango, 0);
                SendDlgItemMessage(hDlg, IDC_CHOCOLATE, PBM_SETPOS, (int)volumeChocolate, 0);

                AtualizaVolumes(hDlg);
                CheckDlgButton(hDlg, IDC_CHECKRESETVOLUMES, BST_UNCHECKED);
            }
        }
        break;

        case IDC_TROCARSERIAL:
            strcpy_s(serialPort, sizeof(serialPort), "COM1");
            MessageBoxA(hDlg, serialPort, "Serial resetada para", MB_OK | MB_ICONINFORMATION);
            return TRUE;

        case IDCANCEL:
            DestroyWindow(hDlg);
            break;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hDlg);
        break;

    case WM_DESTROY:
        executando = false;
        if (serialThread.joinable()) serialThread.join();

        delete g_protocolo;
        delete g_serial;

        PostQuitMessage(0);
        break;
    }
    return FALSE;
}


std::wstring to_wstring(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}

INT_PTR CALLBACK WelcomeDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {

    switch (message) {
    case WM_INITDIALOG: {

        // Ícones
        SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONE)));
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONE)));

        // Preenche a ComboBox com portas COM fictícias (ajuste conforme necessário)
        HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_SERIAL);

        if (hCombo) {
            std::vector<std::string> ports = SerialPort::listAvailablePorts();

            for (std::string port : ports) {
                std::wstring wport = to_wstring(port);
                SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)wport.c_str());
            }
            SendMessage(hCombo, CB_SETCURSEL, 0, 0); // Seleciona o primeiro item por padrão
        }

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: {
            HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_SERIAL);
            int selIndex = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);

            if (selIndex == CB_ERR) {
                // Nenhuma seleção feita
                MessageBox(hDlg, L"Por favor, selecione uma porta serial.", L"Aviso", MB_OK | MB_ICONWARNING);
                return (INT_PTR)TRUE;
            }

            // Obtém o texto selecionado
            TCHAR serialW[100];
            SendMessage(hCombo, CB_GETLBTEXT, selIndex, (LPARAM)serialW);

            // Converte TCHAR (Unicode) para char (ANSI)
            WideCharToMultiByte(CP_ACP, 0, serialW, -1, serialPort, sizeof(serialPort), NULL, NULL);

            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        }

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }

    return (INT_PTR)FALSE;
}