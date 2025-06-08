// WindowsProject.cpp : Define o ponto de entrada para o aplicativo.
//

#include "framework.h"
#include "WindowsProject.h"
#include <CommCtrl.h> // Para usar controles comuns, como barras de progresso, botões, etc.

#define MAX_LOADSTRING 100

// Variáveis Globais:
HINSTANCE hInst;                                // instância atual
WCHAR szTitle[MAX_LOADSTRING];                  // O texto da barra de título
WCHAR szWindowClass[MAX_LOADSTRING];            // o nome da classe da janela principal

//VARIÁVEIS GLOBAIS PARA O PROJETO:
int desperdicio = 15;
float volumemixer = 50;
float volumecreme = 30;
float volumechocolate = 30;
float volumemorango = 30;
float volumecongelamento = 40; // (40L)
float volumepote = 2.5; // Volume do pote em litros (2.5L)
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

static int pos_x = 340; // posição inicial X
static int pos_y = 592; // posição Y fixa (ajuste conforme necessário)

// Declarações de encaminhamento de funções incluídas nesse módulo de código:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    MainDialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    WelcomeDialogProc(HWND, UINT, WPARAM, LPARAM);


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





//
//  FUNÇÃO: MyRegisterClass()
//
//  FINALIDADE: Registra a classe de janela.
//
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

//
//   FUNÇÃO: InitInstance(HINSTANCE, int)
//
//   FINALIDADE: Salva o identificador de instância e cria a janela principal
//
//   COMENTÁRIOS:
//
//        Nesta função, o identificador de instâncias é salvo em uma variável global e
//        crie e exiba a janela do programa principal.
//
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


//
//  FUNÇÃO: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  FINALIDADE: Processa as mensagens para a janela principal.
//
//  WM_COMMAND  - processar o menu do aplicativo
//  WM_PAINT    - Pintar a janela principal
//  WM_DESTROY  - postar uma mensagem de saída e retornar
//
//
// recursos.rc
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
void AtualizaValorTempPh(HWND hDlg) {
    int valor = 0;
    HWND hProg = GetDlgItem(hDlg, IDC_PROGRESSTEMP);

    // A barra de progresso sempre mostra a temperatura do MIXER
    SendMessage(hProg, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessage(hProg, PBM_SETPOS, tempmixer, 0);

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
        SendDlgItemMessage(hDlg, IDC_PROGRESSTEMP, PBM_SETPOS, tempmixer, 0); // Valor da barra de progresso
        HWND hProg = GetDlgItem(hDlg, IDC_PROGRESSTEMP);
        SendMessage(hProg, PBM_SETBARCOLOR, 0, (LPARAM)RGB(255, 0, 0)); // CORES RGB (VALOR RED, VALOR GREEN, VALOR BLUE)
        //SendMessage(hProg, PBM_SETBKCOLOR, 0, (LPARAM)RGB(230, 230, 230)); // SE QUISER MUDAR A COR DE FUNDO DA BARRA DE PROGRESSO.

        // CREME
		SendDlgItemMessage(hDlg, IDC_CREME, PBM_SETRANGE, 0, MAKELPARAM(0, 30)); //Range de 0 a 30
		SendDlgItemMessage(hDlg, IDC_CREME, PBM_SETPOS, volumecreme, 0); // Posição inicial
        HWND hProgCreme = GetDlgItem(hDlg, IDC_CREME);
        SendMessage(hProgCreme, PBM_SETBARCOLOR, 0, (LPARAM)RGB(255, 255, 100)); // Cor
		ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCREME), SW_SHOW); // Mostra a válvula de creme desligada
		ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCREME), SW_HIDE); // Esconde a válvula de creme ligada


        // MORANGO
        SendDlgItemMessage(hDlg, IDC_MORANGO, PBM_SETRANGE, 0, MAKELPARAM(0, 30));
        SendDlgItemMessage(hDlg, IDC_MORANGO, PBM_SETPOS, volumemorango, 0);
        HWND hProgMorango = GetDlgItem(hDlg, IDC_MORANGO);
        SendMessage(hProgMorango, PBM_SETBARCOLOR, 0, (LPARAM)RGB(255, 80, 80));
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFMORANGO), SW_SHOW); // Mostra a válvula de creme desligada
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONMORANGO), SW_HIDE); // Esconde a válvula de creme ligada

        // CHOCOLATE
        SendDlgItemMessage(hDlg, IDC_CHOCOLATE, PBM_SETRANGE, 0, MAKELPARAM(0, 30));
        SendDlgItemMessage(hDlg, IDC_CHOCOLATE, PBM_SETPOS, volumechocolate, 0);
        HWND hProgChocolate = GetDlgItem(hDlg, IDC_CHOCOLATE);
        SendMessage(hProgChocolate, PBM_SETBARCOLOR, 0, (LPARAM)RGB(123, 63, 0));
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCHOCOLATE), SW_SHOW); // Mostra a válvula de creme desligada
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCHOCOLATE), SW_HIDE); // Esconde a válvula de creme ligada

        // MIXER (já está assim, apenas certifique-se que seja volume)
        SendDlgItemMessage(hDlg, IDC_PROGRESSTEMP, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
        SendDlgItemMessage(hDlg, IDC_PROGRESSTEMP, PBM_SETPOS, volumemixer, 0); // se quiser mostrar o volume do mixer

        // CONGELAMENTO 
        SendDlgItemMessage(hDlg, IDC_PROGRESSCONGELAMENTO, PBM_SETRANGE, 0, MAKELPARAM(0, 40));
        SendDlgItemMessage(hDlg, IDC_PROGRESSCONGELAMENTO, PBM_SETPOS, volumecongelamento, 0); // se quiser mostrar o volume do mixer

        HWND hPote = GetDlgItem(hDlg, IDC_POTE);
        SetWindowPos(GetDlgItem(hDlg, IDC_POTE), NULL, pos_x, pos_y, 0, 0, //Posicão inicial do pote
            SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);

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
		if (wParam == 1) // Verifica se é o timer com ID 1
        {
            // Só move se o checkbox estiver marcado
            if (IsDlgButtonChecked(hDlg, IDC_ESTEIRAON) == BST_CHECKED)
            {
				pos_x += 1; // Velocidade de movimento do pote (1 pixel por tick)
                SetWindowPos(GetDlgItem(hDlg, IDC_POTE), 0, pos_x, pos_y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

                if (pos_x > 1050) {
                    pos_x = 340; // Reseta a posição

                }
            }
        }
        break;

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
//Dialog de About
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

//Janela de boas-vindas
INT_PTR CALLBACK WelcomeDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG: {
        //FORÇA O ICONE 
        SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONE)));
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONE)));
        HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICONE));
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            TCHAR serialW[100];
            GetDlgItemText(hDlg, IDC_EDIT_SERIAL, serialW, 100);

            if (wcslen(serialW) == 0) {
                MessageBox(hDlg, L"Por favor, insira o número serial. Exemplo: COM1", L"Aviso", MB_OK | MB_ICONWARNING);
                return (INT_PTR)TRUE;
            }

            // Converte TCHAR (Unicode) para char (ANSI)
            WideCharToMultiByte(CP_ACP, 0, serialW, -1, serialPort, sizeof(serialPort), NULL, NULL);

            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}