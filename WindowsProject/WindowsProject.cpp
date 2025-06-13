// WindowsProject.cpp : Define o ponto de entrada para o aplicativo.
//
#include "atualiza.h" // Para atualizar os valores de temperatura, pH, volumes, etc.
#include "Timer.h" // Para o tratamento de timers
#include "framework.h" // Inclui o arquivo de cabeçalho principal do projeto
#include "WindowsProject.h" // Inclui o arquivo de cabeçalho do projeto
#include <CommCtrl.h> // Para usar controles comuns, como barras de progresso, botões, etc.
#include "SimulatorProtocol.h" // Inclui o protocolo de simulação para comunicação com o Arduino

//===============================================================================================
SerialPort* g_serial = nullptr;
SimulatorProtocol* g_protocolo = nullptr;
//===============================================================================================

#define MAX_LOADSTRING 100

#include "SerialPort.h" // Inclui a classe SerialPort para comunicação serial
#include <thread> // Para usar threads
#include <chrono> // Para usar funções de tempo, como sleep_for
#include <atomic> // Para usar variáveis atômicas, como executando

// Estrutura de controle que pode ser atualizada externamente (ex: por botões)
typedef struct Entradas {
    bool valvula1 = false;
    bool valvula2 = false;
    bool valvula3 = false;
    bool valvulaSaida = false;
    bool misturador = false;
    bool esteira = false;
    bool selecao = false;
    uint16_t temperaturaControle = 512; // valor analógico (0–1023)
};

std::atomic<bool> executando(true);

// Variáveis Globais:
HINSTANCE hInst;                                // instância atual
WCHAR szTitle[MAX_LOADSTRING];                  // O texto da barra de título
WCHAR szWindowClass[MAX_LOADSTRING];            // o nome da classe da janela principal

//VARIÁVEIS GLOBAIS PARA O PROJETO:
int desperdicio = 15;
float volumemixer = 0;
float volumecreme = 30;
float volumechocolate = 30;
float volumemorango = 30;
float volumecongelamento = 0;
float volumepote = 0;
int phmixer = 0; // pH do mixer (inicialmente indefinido, pode ser ajustado depois)
int ph1 = 7; // pH do tanque creme
int ph2 = 13; // pH do tanque morango
int ph3 = 5; // pH do tanque chocolate
int temp1 = 30; // Temperatura do tanque creme
int temp2 = 60; // Temperatura do tanque morango
int temp3 = 80; // Temperatura do tanque chocolate
int tempmixer = 0; // Temperatura inicial do mixer
int tempTargetMixer = -1; // ALVO PARA PRIMEIRA TEMPERATURA DO MIXER
int tempTargetCongelamento = 0; // ALVO PARA PRIMEIRA TEMPERATURA DO CONGELAMENTO
int tempcongelamento = 0; // Temperatura do congelamento 
int tanqueSelecionado = 1; // Variável para armazenar o tanque selecionado (1, 2 ou 3)
char serialPort[100] = "COM1"; // Porta serial padrão (pode ser alterada pelo usuário)

int tempoCreme = 0; // Tempos para calcular a COR do mixer
int tempoMorango = 0;
int tempoChocolate = 0;
float mixerR = 0.0f;
float mixerG = 0.0f;
float mixerB = 0.0f;
float targetR = 0.0f, targetG = 0.0f, targetB = 0.0f;
bool primeiraCorMixer = true;
const int R_CREME = 255, G_CREME = 255, B_CREME = 100;
const int R_MORANGO = 255, G_MORANGO = 80, B_MORANGO = 80;
const int R_CHOCOLATE = 123, G_CHOCOLATE = 63, B_CHOCOLATE = 0;

bool mostrarMix1 = true;
bool poteAbaixoDoMixer = false;

std::thread serialThread;
Entradas entradas;
Saidas saidas;


// Declarações de encaminhamento de funções incluídas nesse módulo de código:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);

INT_PTR CALLBACK    MainDialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    WelcomeDialogProc(HWND, UINT, WPARAM, LPARAM);


// Objetos de classes internas ao projeto
// SimulatorProtocol* simProto = nullptr;

//===============================================================================================
void TestaComandoPR() {
    if (!g_protocolo) {
        OutputDebugStringA("❌ g_protocolo não está inicializado!\n");
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
//===============================================================================================


void loopAtualizacao(SerialPort& serial, Entradas& entradas, Saidas& saidas) {
    SimulatorProtocol protocolo(serial);

    // TESTE MANUAL APÓS INICIAR A SERIAL
    
    //===============================================================================================
    while (executando.load()) {
        // 1. Montar bits digitais
        uint16_t bits = 0;
        bits |= entradas.valvula1 << 0;
        bits |= entradas.valvula2 << 1;
        bits |= entradas.valvula3 << 2;
        bits |= entradas.valvulaSaida << 3;
        bits |= entradas.misturador << 4;
        bits |= entradas.esteira << 5;
        bits |= entradas.selecao << 6;

        // 2. Enviar dados
        std::vector<uint16_t> analogOut = { entradas.temperaturaControle };
        protocolo.sendData(bits, analogOut);

        // 3. Ler resposta
        uint8_t digitalIn;
        std::vector<uint16_t> analogIn;
        if (protocolo.readData(digitalIn, analogIn, 2)) {
            saidas.nivel1 = digitalIn & (1 << 0);
            saidas.nivel2 = digitalIn & (1 << 1);
            saidas.nivel3 = digitalIn & (1 << 2);
            saidas.nivelSaida = digitalIn & (1 << 3);
            saidas.poteEsteira = digitalIn & (1 << 5); // Bit 5 = pino 13

            if (analogIn.size() >= 2) {
                saidas.temperatura = analogIn[0];
                saidas.ph = analogIn[1];
            }

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

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
 } //===============================================================================================


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

        //FORÇA O ICONE NA JANELA E NA BARRA DE TAREFAS.
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONE))); // Icone pequeno
		HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICONE)); // Carrega o ícone
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONE))); // Icone grande


        //Botão de Temperatura / Ph  
        CheckDlgButton(hDlg, IDC_RADIOTEMP, BST_CHECKED); // Estado inicial do botão de temperatura
        AtualizaValorTempPh(hDlg); // Atualiza o valor de temperatura ou pH com base no botão selecionado
       
        // Inicializa os valores de temperatura nos tanques
        WCHAR buffer[16];
        swprintf(buffer, 16, L"%d(C)", temp2);
        SetDlgItemText(hDlg, IDC_TPHMORANGO, buffer);
        swprintf(buffer, 16, L"%d(C)", temp1);
        SetDlgItemText(hDlg, IDC_TPHCREME, buffer);
        swprintf(buffer, 16, L"%d(C)", temp3);
        SetDlgItemText(hDlg, IDC_TPHCHOCOLATE, buffer);
        swprintf(buffer, 16, L"%d(C)", tempmixer);
        SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);
        swprintf(buffer, 16, L"%d(C)", tempcongelamento);
        SetDlgItemText(hDlg, IDC_TPHCONGELAMENTO, buffer);
                  
        // Inicializa a barra de progresso EXTERNA (VISIVEL)
        SendDlgItemMessage(hDlg, IDC_PROGRESSTEMP, PBM_SETRANGE, 0, MAKELPARAM(0, 100)); // Limites de escala da barra de progresso
        SendDlgItemMessage(hDlg, IDC_PROGRESSTEMP, PBM_SETPOS, tempcongelamento, 0); // Valor da barra de progresso
        HWND hProg = GetDlgItem(hDlg, IDC_PROGRESSTEMP);
        SendMessage(hProg, PBM_SETBARCOLOR, 0, (LPARAM)RGB(255, 0, 0)); // COR VERMELHA (VALOR RED, VALOR GREEN, VALOR BLUE)
      
        // CREME
        SendDlgItemMessage(hDlg, IDC_CREME, PBM_SETRANGE, 0, MAKELPARAM(0, 30)); //Range de 0 a 30
        SendDlgItemMessage(hDlg, IDC_CREME, PBM_SETPOS, volumecreme, 0); // Posição inicial
        HWND hProgCreme = GetDlgItem(hDlg, IDC_CREME);
        SendMessage(hProgCreme, PBM_SETBARCOLOR, 0, (LPARAM)RGB(R_CREME, G_CREME, B_CREME)); // Cor
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCREME), SW_SHOW); // Mostra a válvula de creme desligada
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCREME), SW_HIDE); // Esconde a válvula de creme ligada


        // MORANGO
        SendDlgItemMessage(hDlg, IDC_MORANGO, PBM_SETRANGE, 0, MAKELPARAM(0, 30)); //Range de 0 a 30
		SendDlgItemMessage(hDlg, IDC_MORANGO, PBM_SETPOS, volumemorango, 0); // Posição inicial
        HWND hProgMorango = GetDlgItem(hDlg, IDC_MORANGO);
        SendMessage(hProgMorango, PBM_SETBARCOLOR, 0, (LPARAM)RGB(R_MORANGO, G_MORANGO, B_MORANGO));
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFMORANGO), SW_SHOW); // Mostra a válvula de creme desligada
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONMORANGO), SW_HIDE); // Esconde a válvula de creme ligada

        // CHOCOLATE
		SendDlgItemMessage(hDlg, IDC_CHOCOLATE, PBM_SETRANGE, 0, MAKELPARAM(0, 30)); //Range de 0 a 30
		SendDlgItemMessage(hDlg, IDC_CHOCOLATE, PBM_SETPOS, volumechocolate, 0); // Posição inicial
        HWND hProgChocolate = GetDlgItem(hDlg, IDC_CHOCOLATE);
        SendMessage(hProgChocolate, PBM_SETBARCOLOR, 0, (LPARAM)RGB(R_CHOCOLATE, G_CHOCOLATE, B_CHOCOLATE));
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCHOCOLATE), SW_SHOW); // Mostra a válvula de creme desligada
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCHOCOLATE), SW_HIDE); // Esconde a válvula de creme ligada

        // MIXER
		SendDlgItemMessage(hDlg, IDC_PROGRESSTEMP, PBM_SETRANGE, 0, MAKELPARAM(0, 70)); // Range de 0 a 70
		SendDlgItemMessage(hDlg, IDC_PROGRESSTEMP, PBM_SETPOS, volumecongelamento, 0); // Posição inicial

        // CONGELAMENTO 
		SendDlgItemMessage(hDlg, IDC_PROGRESSCONGELAMENTO, PBM_SETRANGE, 0, MAKELPARAM(0, 40)); // Range de 0 a 40
		SendDlgItemMessage(hDlg, IDC_PROGRESSCONGELAMENTO, PBM_SETPOS, volumecongelamento, 0); // Posição inicial

        //POTE    
        // Inicializa o valor e o range da barra
		SendDlgItemMessage(hDlg, IDC_PROGRESSPOTE, PBM_SETRANGE, 0, MAKELPARAM(0, 2.5)); // Range de 0 a 2.5 
		SendDlgItemMessage(hDlg, IDC_PROGRESSPOTE, PBM_SETPOS, (int)volumepote, 0); // Posição inicial


		HWND hPote = GetDlgItem(hDlg, IDC_POTE); // Obtém o handle do pote
		HWND hBarra = GetDlgItem(hDlg, IDC_PROGRESSPOTE); // Obtém o handle da barra de progresso do pote

		ShowWindow(GetDlgItem(hDlg, IDC_PROGRESSPOTE), SWP_SHOWWINDOW); // Mostra a barra de progresso do pote

     
        //=================================

        g_serial = new SerialPort(serialPort);
        if (g_serial->open()) {
            OutputDebugStringA("✅ Porta aberta com sucesso!\n");

            g_protocolo = new SimulatorProtocol(*g_serial);
            executando = true;
            serialThread = std::thread(loopAtualizacao, std::ref(*g_serial), std::ref(entradas), std::ref(saidas));
        }
        else {
            OutputDebugStringA("❌ Erro ao abrir porta serial.\n");
        }

        //=================================



      

        SetTimer(hDlg, 1, 10, NULL); // Timer com ID 1 e intervalo de 10ms
		AtualizaVolumes(hDlg);  // Atualiza os volumes iniciais na interface
		AtualizaValorTempPh(hDlg); // Atualiza os valores de temperatura e pH na interface

        return (INT_PTR)TRUE;
    }

    case WM_TIMER:
		HandleTimer(hDlg, wParam, lParam); // Chama a função de tratamento do timer
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            // Botões de controle
        case IDC_RADIOTEMP:
        case IDC_RADIOPH:
			AtualizaValorTempPh(hDlg); // Atualiza o valor de temperatura ou pH com base no botão selecionado
            break;

        case IDC_CHECKCREME:
        {
			BOOL checked = IsDlgButtonChecked(hDlg, IDC_CHECKCREME) == BST_CHECKED; // Verifica se o checkbox está marcado
			ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCREME), checked ? SW_SHOW : SW_HIDE); // Mostra ou esconde a válvula de creme ligada
			ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCREME), checked ? SW_HIDE : SW_SHOW); // Mostra ou esconde a válvula de creme desligada
            if (tempTargetMixer == -1 && volumemixer == 0.0f) { //Muda a temperatura do mixer inicial
				tempmixer = temp1; // aplica direto a primeira temperatura sendo creme
            }
			tempTargetMixer = temp1; // Define a temperatura alvo do mixer para creme
        }
        break;
        case IDC_CHECKMORANGO:
        {
			BOOL checked = IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO) == BST_CHECKED; // Verifica se o checkbox está marcado
			ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONMORANGO), checked ? SW_SHOW : SW_HIDE); // Mostra ou esconde a válvula de morango ligada
			ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFMORANGO), checked ? SW_HIDE : SW_SHOW); // Mostra ou esconde a válvula de morango desligada
            if (tempTargetMixer == -1 && volumemixer == 0.0f) { //Muda a temperatura do mixer inicial
                tempmixer = temp2; // aplica direto a primeira temperatura sendo creme
            }
			tempTargetMixer = temp2; // Define a temperatura alvo do mixer para morango
        }
        break;
        case IDC_CHECKCHOCOLATE:
        {
			BOOL checked = IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE) == BST_CHECKED; // Verifica se o checkbox está marcado
			ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCHOCOLATE), checked ? SW_SHOW : SW_HIDE); // Mostra ou esconde a válvula de chocolate ligada
			ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCHOCOLATE), checked ? SW_HIDE : SW_SHOW); // Mostra ou esconde a válvula de chocolate desligada
            if (tempTargetMixer == -1 && volumemixer == 0.0f) { //Muda a temperatura do mixer inicial
                tempmixer = temp3; // aplica direto a primeira temperatura sendo creme
            }
			tempTargetMixer = temp3; // Define a temperatura alvo do mixer para chocolate
        }
        break;
        case IDC_CHECKMIXER:
        {
			BOOL checked = IsDlgButtonChecked(hDlg, IDC_CHECKMIXER) == BST_CHECKED; // Verifica se o checkbox está marcado
			ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONMIXER), checked ? SW_SHOW : SW_HIDE); // Mostra ou esconde a válvula do mixer ligada
			ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFMIXER), checked ? SW_HIDE : SW_SHOW); // Mostra ou esconde a válvula do mixer desligada

            if (checked) {
                // Atualiza imediatamente a temperatura e a interface
                tempcongelamento = tempmixer;
                tempTargetCongelamento = tempmixer;

                WCHAR buffer[16];
                swprintf(buffer, 16, L"%d(\x00B0\C)", tempcongelamento);
                SetDlgItemText(hDlg, IDC_TPHCONGELAMENTO, buffer);
            }
            else {
                tempTargetCongelamento = -25; // mínimo de -25 graus
            }
        }
        break;
        case IDC_CHECKCONGELADO:
        {
			BOOL checked = IsDlgButtonChecked(hDlg, IDC_CHECKCONGELADO) == BST_CHECKED; // Verifica se o checkbox está marcado
			ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCONGELADO), checked ? SW_SHOW : SW_HIDE); // Mostra ou esconde a válvula de congelamento ligada
			ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCONGELADO), checked ? SW_HIDE : SW_SHOW); // Mostra ou esconde a válvula de congelamento desligada
        }
        break;
        case IDC_CHECKRESETVOLUMES:
        {
			if (IsDlgButtonChecked(hDlg, IDC_CHECKRESETVOLUMES) == BST_CHECKED) // Se o checkbox estiver marcado
            {
				// Reseta os volumes para os valores iniciais
                volumecreme = 30.0f;
                volumemorango = 30.0f;
                volumechocolate = 30.0f;

                // Atualiza as barras de progresso
                SendDlgItemMessage(hDlg, IDC_CREME, PBM_SETPOS, (int)volumecreme, 0);
                SendDlgItemMessage(hDlg, IDC_MORANGO, PBM_SETPOS, (int)volumemorango, 0);
                SendDlgItemMessage(hDlg, IDC_CHOCOLATE, PBM_SETPOS, (int)volumechocolate, 0);
                               
                AtualizaVolumes(hDlg); // Atualiza os valores numéricos

                // Desmarca automaticamente o checkbox após o reset
                CheckDlgButton(hDlg, IDC_CHECKRESETVOLUMES, BST_UNCHECKED);
            }
        }
        break;

        case IDC_TROCARSERIAL:
            strcpy_s(serialPort, sizeof(serialPort), "COM1");
            MessageBoxA(hDlg, serialPort, "Serial resetada para", MB_OK | MB_ICONINFORMATION);
            return TRUE;

            // Se clicar "Fechar"...
        case IDCANCEL:
            DestroyWindow(hDlg);
            break;
        }
        break;
        // Se clicar no "X" da janela principal...
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

