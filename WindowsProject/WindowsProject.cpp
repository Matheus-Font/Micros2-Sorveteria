// WindowsProject.cpp : Define o ponto de entrada para o aplicativo.
//

#include "framework.h"
#include "WindowsProject.h"
#include <CommCtrl.h> // Para usar controles comuns, como barras de progresso, botões, etc.
#include "SimulatorProtocol.h"

#define MAX_LOADSTRING 100

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
float vazao = 0.02f; // vazão em litros 0.2L/s // 0.002f= 2mL/s (2mL por segundo)
int phmixer = 0; // pH do mixer (inicialmente indefinido, pode ser ajustado depois)
int ph1 = 7; // pH do tanque creme
int ph2 = 13; // pH do tanque morango
int ph3 = 5; // pH do tanque chocolate
int temp1 = 30; // Temperatura do tanque creme
int temp2 = 60; // Temperatura do tanque morango
int temp3 = 80; // Temperatura do tanque chocolate
int tempmixer = 70; // Temperatura inicial do mixer
int tempcongelamento = 5; // Temperatura do congelamento 
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
UINT tempoAnimacaoMix = 0;
bool poteAbaixoDoMixer = false;

static int pos_x = 340; // posição inicial X 
static int pos_y = 592; // posição Y fixa 
const int larguraPote = 46; // Largura do controle IDC_POTE em pixels
const int larguraBarra = 30; // Largura da barra IDC_PROGRESSPOTE 
int offsetX = (larguraPote - larguraBarra) / 2;

// Declarações de encaminhamento de funções incluídas nesse módulo de código:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    MainDialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    WelcomeDialogProc(HWND, UINT, WPARAM, LPARAM);


// Objetos de classes internas ao projeto
SimulatorProtocol simProto();

/**
 * @brief Função principal de entrada do aplicativo Windows.
 * @param hInstance Instância atual do aplicativo.
 * @param hPrevInstance Instância anterior do aplicativo (não utilizada).
 * @param lpCmdLine Linha de comando.
 * @param nCmdShow Parâmetro para exibição da janela.
 * @return Código de saída do aplicativo.
 */
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





/**
 * @brief Registra a classe de janela principal.
 * @param hInstance Instância do aplicativo.
 * @return ATOM identificador da classe registrada.
 */
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
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

/**
 * @brief Inicializa a instância do aplicativo e cria a janela principal.
 * @param hInstance Instância do aplicativo.
 * @param nCmdShow Parâmetro para exibição da janela.
 * @return TRUE se a inicialização foi bem-sucedida, FALSE caso contrário.
 */
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


/**
 * @brief Processa as mensagens da janela principal.
 * @param hWnd Handle da janela.
 * @param message Mensagem recebida.
 * @param wParam Parâmetro adicional da mensagem.
 * @param lParam Parâmetro adicional da mensagem.
 * @return Resultado do processamento da mensagem.
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Analise as seleções do menu:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Adicione qualquer código de desenho que use hdc aqui...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

/**
 * @brief Atualiza os valores de temperatura e pH na interface.
 * @param hDlg Handle do diálogo principal.
 */
void AtualizaValorTempPh(HWND hDlg) {
    int valor = 0;
    HWND hProg = GetDlgItem(hDlg, IDC_PROGRESSTEMP);

    // A barra de progresso sempre mostra a temperatura do CONGELAMENTO
    SendMessage(hProg, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessage(hProg, PBM_SETPOS, tempcongelamento, 0);

    // Atualiza textos da interface dependendo se está em modo temperatura ou pH
    if (IsDlgButtonChecked(hDlg, IDC_RADIOTEMP) == BST_CHECKED) {
        SetDlgItemText(hDlg, IDC_STATICTPH, L"Temperatura (\x00B0\C):");
        SetDlgItemText(hDlg, IDC_STATIC, L"Temperatura (\x00B0\C):");
        valor = tempmixer;
    }
    else {
        SetDlgItemText(hDlg, IDC_STATICTPH, L"Valor de pH:");
        SetDlgItemText(hDlg, IDC_STATIC, L"Valor de pH:");
        valor = phmixer;
    }

    // Atualiza os campos de exibição ao lado de cada tanque
    WCHAR buffer[16];
    swprintf(buffer, 16, L"%d(\x00B0\C)", temp1);
    SetDlgItemText(hDlg, IDC_TPHCREME, buffer);
    swprintf(buffer, 16, L"%d(\x00B0\C)", temp2);
    SetDlgItemText(hDlg, IDC_TPHMORANGO, buffer);
    swprintf(buffer, 16, L"%d(\x00B0\C)", temp3);
    SetDlgItemText(hDlg, IDC_TPHCHOCOLATE, buffer);
    swprintf(buffer, 16, L"%d(\x00B0\C)", tempmixer);
    SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);
    swprintf(buffer, 16, L"%d(\x00B0\C)", tempcongelamento);
    SetDlgItemText(hDlg, IDC_TPHCONGELAMENTO, buffer);

    if (IsDlgButtonChecked(hDlg, IDC_RADIOPH) == BST_CHECKED) {
        swprintf(buffer, 16, L"pH %d", ph1);
        SetDlgItemText(hDlg, IDC_TPHCREME, buffer);
        swprintf(buffer, 16, L"pH %d", ph2);
        SetDlgItemText(hDlg, IDC_TPHMORANGO, buffer);
        swprintf(buffer, 16, L"pH %d", ph3);
        SetDlgItemText(hDlg, IDC_TPHCHOCOLATE, buffer);
        swprintf(buffer, 16, L"pH %d", phmixer);
        SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);
    }

    // Atualiza o valor exibido ao lado da barra
    SetDlgItemInt(hDlg, IDC_TEMP, valor, FALSE);
}

/**
 * @brief Atualiza os volumes dos tanques na interface.
 * @param hDlg Handle do diálogo principal.
 */
void AtualizaVolumes(HWND hDlg) {
    WCHAR buffer[16];

    swprintf(buffer, 16, L"%.1f", volumecreme);
    SetDlgItemText(hDlg, IDC_VOLUMECREME, buffer);

    swprintf(buffer, 16, L"%.1f", volumemorango);
    SetDlgItemText(hDlg, IDC_VOLUMEMORANGO, buffer);

    swprintf(buffer, 16, L"%.1f", volumechocolate);
    SetDlgItemText(hDlg, IDC_VOLUMECHOCOLATE, buffer);

    swprintf(buffer, 16, L"%.1f", volumemixer);
    SetDlgItemText(hDlg, IDC_VOLUMEMIXER, buffer);

    swprintf(buffer, 16, L"%.1f", volumecongelamento);
    SetDlgItemText(hDlg, IDC_VOLUMECONGELAMENTO, buffer);

    swprintf(buffer, 16, L"%.1f", volumepote);
    SetDlgItemText(hDlg, IDC_VOLUMEPOTE, buffer);
}

INT_PTR CALLBACK MainDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        
        //FORÇA O ICONE NA JANELA E NA BARRA DE TAREFAS.
        //HICON hIcon = LoadIcon(NULL, IDI_EXCLAMATION); // Teste com ícone nativo
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONE)));
        HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICONE));
        SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONE)));


        //Botão de Temperatura / Ph  
        CheckDlgButton(hDlg, IDC_RADIOTEMP, BST_CHECKED); // Estado inicial do botão de temperatura
        // Atualiza todos os campos (temperatura, pH, labels, barra)
        AtualizaValorTempPh(hDlg);
        SetDlgItemInt(hDlg, IDC_TEMP, tempmixer, FALSE);; // Inicializa o valor de temperatura com a variável global tempmixer

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

        // Inicializa os valores dos controles com as variáveis globais
        SetDlgItemInt(hDlg, IDC_DESPERDICIO, desperdicio, FALSE);
        //SetDlgItemInt(hDlg, IDC_VOLUME, volume, FALSE);

        // Inicializa a barra de progresso
        SendDlgItemMessage(hDlg, IDC_PROGRESSTEMP, PBM_SETRANGE, 0, MAKELPARAM(0, 100)); // Limites de escala da barra de progresso
        SendDlgItemMessage(hDlg, IDC_PROGRESSTEMP, PBM_SETPOS, tempcongelamento, 0); // Valor da barra de progresso
        HWND hProg = GetDlgItem(hDlg, IDC_PROGRESSTEMP);
        SendMessage(hProg, PBM_SETBARCOLOR, 0, (LPARAM)RGB(255, 0, 0)); // CORES RGB (VALOR RED, VALOR GREEN, VALOR BLUE)
        //SendMessage(hProg, PBM_SETBKCOLOR, 0, (LPARAM)RGB(230, 230, 230)); // SE QUISER MUDAR A COR DE FUNDO DA BARRA DE PROGRESSO.

        // CREME
		SendDlgItemMessage(hDlg, IDC_CREME, PBM_SETRANGE, 0, MAKELPARAM(0, 30)); //Range de 0 a 30
		SendDlgItemMessage(hDlg, IDC_CREME, PBM_SETPOS, volumecreme, 0); // Posição inicial
        HWND hProgCreme = GetDlgItem(hDlg, IDC_CREME);
        SendMessage(hProgCreme, PBM_SETBARCOLOR, 0, (LPARAM)RGB(R_CREME, G_CREME, B_CREME)); // Cor
		ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCREME), SW_SHOW); // Mostra a válvula de creme desligada
		ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCREME), SW_HIDE); // Esconde a válvula de creme ligada


        // MORANGO
        SendDlgItemMessage(hDlg, IDC_MORANGO, PBM_SETRANGE, 0, MAKELPARAM(0, 30));
        SendDlgItemMessage(hDlg, IDC_MORANGO, PBM_SETPOS, volumemorango, 0);
        HWND hProgMorango = GetDlgItem(hDlg, IDC_MORANGO);
        SendMessage(hProgMorango, PBM_SETBARCOLOR, 0, (LPARAM)RGB(R_MORANGO, G_MORANGO, B_MORANGO));
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFMORANGO), SW_SHOW); // Mostra a válvula de creme desligada
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONMORANGO), SW_HIDE); // Esconde a válvula de creme ligada

        // CHOCOLATE
        SendDlgItemMessage(hDlg, IDC_CHOCOLATE, PBM_SETRANGE, 0, MAKELPARAM(0, 30));
        SendDlgItemMessage(hDlg, IDC_CHOCOLATE, PBM_SETPOS, volumechocolate, 0);
        HWND hProgChocolate = GetDlgItem(hDlg, IDC_CHOCOLATE);
        SendMessage(hProgChocolate, PBM_SETBARCOLOR, 0, (LPARAM)RGB(R_CHOCOLATE, G_CHOCOLATE, B_CHOCOLATE));
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCHOCOLATE), SW_SHOW); // Mostra a válvula de creme desligada
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCHOCOLATE), SW_HIDE); // Esconde a válvula de creme ligada

        // MIXER (já está assim, apenas certifique-se que seja volume)
        SendDlgItemMessage(hDlg, IDC_PROGRESSTEMP, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
        SendDlgItemMessage(hDlg, IDC_PROGRESSTEMP, PBM_SETPOS, volumecongelamento, 0); // se quiser mostrar o volume do mixer

        // CONGELAMENTO 
        SendDlgItemMessage(hDlg, IDC_PROGRESSCONGELAMENTO, PBM_SETRANGE, 0, MAKELPARAM(0, 40));
        SendDlgItemMessage(hDlg, IDC_PROGRESSCONGELAMENTO, PBM_SETPOS, volumecongelamento, 0); // se quiser mostrar o volume do mixer
        
        //POTE    
        SetWindowPos(GetDlgItem(hDlg, IDC_PROGRESSPOTE), NULL, pos_x, pos_y, 0, 0,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
        // Inicializa o valor e o range da barra
        SendDlgItemMessage(hDlg, IDC_PROGRESSPOTE, PBM_SETRANGE, 0, MAKELPARAM(0, 2.5)); // Volume de 0 a 5L
        SendDlgItemMessage(hDlg, IDC_PROGRESSPOTE, PBM_SETPOS, (int)volumepote, 0);


        HWND hPote = GetDlgItem(hDlg, IDC_POTE);
        HWND hBarra = GetDlgItem(hDlg, IDC_PROGRESSPOTE);

        SetWindowPos(hPote, HWND_BOTTOM, pos_x, pos_y, 0, 0,
            SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);  // pote no fundo

        ShowWindow(GetDlgItem(hDlg, IDC_PROGRESSPOTE), SWP_SHOWWINDOW);

        HANDLE hSerial = CreateFileA(
            serialPort,                // usa o valor digitado
            GENERIC_READ | GENERIC_WRITE,
            0, NULL, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, NULL
        );

        SetTimer(hDlg, 1, 10, NULL); // Timer com ID 1 e intervalo de 10ms
        AtualizaVolumes(hDlg);
        AtualizaValorTempPh(hDlg);

        return (INT_PTR)TRUE;
    }
    case WM_TIMER:  // Caso o timer seja ativado
    {
        if (wParam == 1) // Verifica se é o timer com ID 1
        {
            // Transfere conteúdo do mixer para congelamento
            if (IsDlgButtonChecked(hDlg, IDC_CHECKMIXER) == BST_CHECKED && volumemixer >= vazao) {
                volumemixer -= vazao;
                volumecongelamento += vazao;

                HWND hProgCong = GetDlgItem(hDlg, IDC_PROGRESSCONGELAMENTO);
                SendMessage(hProgCong, PBM_SETBARCOLOR, 0, (LPARAM)RGB((int)mixerR, (int)mixerG, (int)mixerB));

                // Atualiza visualmente
                SendDlgItemMessage(hDlg, IDC_PROGRESSMIXER, PBM_SETPOS, (int)volumemixer, 0);
                SendDlgItemMessage(hDlg, IDC_PROGRESSCONGELAMENTO, PBM_SETPOS, (int)volumecongelamento, 0);
                AtualizaVolumes(hDlg);

                if (volumecongelamento > 40.0f) volumecongelamento = 40.0f;
                if (volumemixer < 0.0f) volumemixer = 0.0f;
                
                if (volumemixer <= 0.0f) {
                    mixerR = mixerG = mixerB = 0;
                    targetR = targetG = targetB = 0;
                    primeiraCorMixer = true;
                    tempoCreme = tempoMorango = tempoChocolate = 0;
                }

            }

            // Só move se o checkbox estiver marcado
            if (IsDlgButtonChecked(hDlg, IDC_ESTEIRAON) == BST_CHECKED)
            {
                pos_x += 1; // Velocidade de movimento do pote (1 pixel por tick)
				pos_y = 592; 
              
                // Verifica se pote está abaixo do mixer
                if (pos_x >= 635 && pos_x <= 705)
                {
                    poteAbaixoDoMixer = true; 
                    EnableWindow(GetDlgItem(hDlg, IDC_ESTEIRAON), TRUE);
                }
                else
                {
                    poteAbaixoDoMixer = false;
                }

				// Verifica se o pote passou do limite direito
                if (pos_x > 1150) {
                    pos_x = 340; // Reseta a posição do pote se passar do limite
                    pos_y = 592; // Mantém a posição Y fixa
                    ShowWindow(GetDlgItem(hDlg, IDC_PROGRESSPOTE), SW_HIDE); // Esconde a barra de progresso do pote
					EnableWindow(GetDlgItem(hDlg, IDC_ESTEIRAON), TRUE); // Desabilita o botão de esteira quando o pote volta ao início

					// Reseta o volume do pote
                }

                // Atualiza posições do pote e da barra
                SetWindowPos(GetDlgItem(hDlg, IDC_POTE), NULL, pos_x, pos_y, 0, 0,
                    SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

                SetWindowPos(GetDlgItem(hDlg, IDC_PROGRESSPOTE), HWND_TOP, pos_x + 5, pos_y + 10, 0, 0,
                    SWP_NOSIZE | SWP_SHOWWINDOW);
            }
            
        }
        auto setCorAlvo = [&](int r, int g, int b) {
            if (primeiraCorMixer) {
                mixerR = r;
                mixerG = g;
                mixerB = b;
                primeiraCorMixer = false;
            }
            targetR = r;
            targetG = g;
            targetB = b;
            };

        // Controle de enchimento do mixer
        bool atualizou = false;
        

        if (IsDlgButtonChecked(hDlg, IDC_CHECKCREME) == BST_CHECKED && volumecreme >= vazao) {
            volumecreme -= vazao;
            volumemixer += vazao;
            tempoCreme++;
            setCorAlvo(R_CREME, G_CREME, B_CREME);
            atualizou = true;
        }
        else if (IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO) == BST_CHECKED && volumemorango >= vazao) {
            volumemorango -= vazao;
            volumemixer += vazao;
            tempoMorango++;
            setCorAlvo(R_MORANGO, G_MORANGO, B_MORANGO);
            atualizou = true;
        }
        else if (IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE) == BST_CHECKED && volumechocolate >= vazao) {
            volumechocolate -= vazao;
            volumemixer += vazao;
            tempoChocolate++;
            setCorAlvo(R_CHOCOLATE, G_CHOCOLATE, B_CHOCOLATE);
            atualizou = true;
        }



        // Limita valores para não ultrapassar os máximos
        if (volumemixer > 500.0f) volumemixer = 500.0f;
        if (volumecreme < 0.0f) volumecreme = 0.0f;
        if (volumemorango < 0.0f) volumemorango = 0.0f;
        if (volumechocolate < 0.0f) volumechocolate = 0.0f;

        if (atualizou) {
            AtualizaVolumes(hDlg);

            // Atualiza barras de progresso correspondentes
            SendDlgItemMessage(hDlg, IDC_CREME, PBM_SETPOS, (int)volumecreme, 0);
            SendDlgItemMessage(hDlg, IDC_MORANGO, PBM_SETPOS, (int)volumemorango, 0);
            SendDlgItemMessage(hDlg, IDC_CHOCOLATE, PBM_SETPOS, (int)volumechocolate, 0);
            SendDlgItemMessage(hDlg, IDC_PROGRESSMIXER, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
            SendDlgItemMessage(hDlg, IDC_PROGRESSMIXER, PBM_SETPOS, (int)volumemixer, 0);

            

			float passoRGB = 0.1f; // velocidade de mudanca de cor

            auto transicao = [&](float& atual, float alvo) {
                if (atual < alvo)
                    atual = min(alvo, atual + passoRGB);
                else if (atual > alvo)
                    atual = max(alvo, atual - passoRGB);
                };

            transicao(mixerR, targetR);
            transicao(mixerG, targetG);
            transicao(mixerB, targetB);

            COLORREF corMixer = RGB(mixerR, mixerG, mixerB);
            HWND hProgMixer = GetDlgItem(hDlg, IDC_PROGRESSMIXER);
            SendMessage(hProgMixer, PBM_SETBARCOLOR, 0, (LPARAM)corMixer);

            if (volumemixer <= 0.0f) {
                mixerR = mixerG = mixerB = 0;
                targetR = targetG = targetB = 0;
                primeiraCorMixer = true;
                tempoCreme = tempoMorango = tempoChocolate = 0;
            }



        }
        //PASSA O SORVETE PRO POTE
        if (IsDlgButtonChecked(hDlg, IDC_CHECKCONGELADO) == BST_CHECKED && poteAbaixoDoMixer && volumecongelamento >= vazao)
        {
            volumecongelamento -= vazao;
            volumepote += vazao;

            
            // Limita os volumes
            if (volumepote > 2.5f) volumepote = 2.5f;
            if (volumecongelamento < 0.0f) volumecongelamento = 0.0f;

            // Atualiza visualmente a barra do congelamento e o volume do pote
            SendDlgItemMessage(hDlg, IDC_PROGRESSCONGELAMENTO, PBM_SETPOS, (int)volumecongelamento, 0);
            AtualizaVolumes(hDlg);

        }
		//ANIMAÇÃO DO MIXER
        tempoAnimacaoMix += 10;
        if (tempoAnimacaoMix >= 100 &&
            (IsDlgButtonChecked(hDlg, IDC_CHECKCREME) == BST_CHECKED ||
                IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO) == BST_CHECKED ||
                IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE) == BST_CHECKED))
        {
            tempoAnimacaoMix = 0;
            mostrarMix1 = !mostrarMix1;

            ShowWindow(GetDlgItem(hDlg, IDC_MIX1), mostrarMix1 ? SW_SHOW : SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_MIX2), mostrarMix1 ? SW_HIDE : SW_SHOW);
        }
        else if (
            IsDlgButtonChecked(hDlg, IDC_CHECKCREME) != BST_CHECKED &&
            IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO) != BST_CHECKED &&
            IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE) != BST_CHECKED)
        {
            // Nenhum botão pressionado: desliga as imagens
            ShowWindow(GetDlgItem(hDlg, IDC_MIX1), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_MIX2), SW_HIDE);
            tempoAnimacaoMix = 0;
        }
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            // Botões de controle
        case IDC_RADIOTEMP:
        case IDC_RADIOPH:
            AtualizaValorTempPh(hDlg);
            break;

        case IDC_CHECKCREME:
        {
            BOOL checked = IsDlgButtonChecked(hDlg, IDC_CHECKCREME) == BST_CHECKED;

            ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCREME), checked ? SW_SHOW : SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCREME), checked ? SW_HIDE : SW_SHOW);
        }
        break;
        case IDC_CHECKMORANGO:
        {
            BOOL checked = IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO) == BST_CHECKED;

            ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONMORANGO), checked ? SW_SHOW : SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFMORANGO), checked ? SW_HIDE : SW_SHOW);
        }
        break;
        case IDC_CHECKCHOCOLATE:
        {
            BOOL checked = IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE) == BST_CHECKED;
            ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCHOCOLATE), checked ? SW_SHOW : SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCHOCOLATE), checked ? SW_HIDE : SW_SHOW);
		}
		break;
        case IDC_CHECKMIXER:
        {
            BOOL checked = IsDlgButtonChecked(hDlg, IDC_CHECKMIXER) == BST_CHECKED;
            ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONMIXER), checked ? SW_SHOW : SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFMIXER), checked ? SW_HIDE : SW_SHOW);
        }
        break;
        case IDC_CHECKCONGELADO:
        {
            BOOL checked = IsDlgButtonChecked(hDlg, IDC_CHECKCONGELADO) == BST_CHECKED;
            ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCONGELADO), checked ? SW_SHOW : SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCONGELADO), checked ? SW_HIDE : SW_SHOW);
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
        PostQuitMessage(0);
        break;
    }


    return FALSE;
}

/**
 * @brief Procedimento do diálogo "Sobre".
 * @param hDlg Handle do diálogo.
 * @param message Mensagem recebida.
 * @param wParam Parâmetro adicional da mensagem.
 * @param lParam Parâmetro adicional da mensagem.
 * @return TRUE se a mensagem foi processada, FALSE caso contrário.
 */
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

std::wstring to_wstring(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}

/**
 * @brief Procedimento do diálogo de boas-vindas.
 * @param hDlg Handle do diálogo.
 * @param message Mensagem recebida.
 * @param wParam Parâmetro adicional da mensagem.
 * @param lParam Parâmetro adicional da mensagem.
 * @return TRUE se a mensagem foi processada, FALSE caso contrário.
 */
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

            for (std::string port: ports) {
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

