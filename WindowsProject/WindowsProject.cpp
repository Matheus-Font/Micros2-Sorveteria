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
int volume = 50;
int ph1 = 7; // pH do tanque 1
int ph2 = 13; // pH do tanque 2
int ph3 = 5; // pH do tanque 3
int temperatura = 50;
int temp1 = 30; // Temperatura do tanque 1
int temp2 = 60; // Temperatura do tanque 2
int temp3 = 80; // Temperatura do tanque 3
int tanqueSelecionado = 1; // Variável para armazenar o tanque selecionado (1, 2 ou 3)

// Declarações de encaminhamento de funções incluídas nesse módulo de código:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    MainDialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    MenuDialogProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Coloque o código aqui.

    // Inicializar cadeias de caracteres globais
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Realize a inicialização do aplicativo:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT));

    MSG msg;

    // Loop de mensagem principal:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
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

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));



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
            case IDC_MENU:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_MENU), hWnd, MenuDialogProc);
                break;
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

    // Qual tanque está selecionado?
    if (IsDlgButtonChecked(hDlg, IDC_TANQUE1) == BST_CHECKED) tanqueSelecionado = 1;
    else if (IsDlgButtonChecked(hDlg, IDC_TANQUE2) == BST_CHECKED) tanqueSelecionado = 2;
    else if (IsDlgButtonChecked(hDlg, IDC_TANQUE3) == BST_CHECKED) tanqueSelecionado = 3;

    // Temperatura
    if (IsDlgButtonChecked(hDlg, IDC_RADIOTEMP) == BST_CHECKED) {
        SetDlgItemText(hDlg, IDC_STATICTPH, L"Temperatura (\x00B0\C):"); // Troca o texto do lado do valor numerico
		SetDlgItemText(hDlg, IDC_STATIC, L"Temperatura (\x00B0\C):"); //Troca o texto encima da barra de progresso
        switch (tanqueSelecionado) {
        case 1: valor = temp1; break;
        case 2: valor = temp2; break;
        case 3: valor = temp3; break;
        }

        // Atualiza faixa e valor da barra para temperatura (0–100)
        SendMessage(hProg, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
        SendMessage(hProg, PBM_SETPOS, valor, 0);
    }

    // pH
    else {
        SetDlgItemText(hDlg, IDC_STATICTPH, L"Valor de pH:");
        SetDlgItemText(hDlg, IDC_STATIC, L"Valor de pH:");

        switch (tanqueSelecionado) {
        case 1: valor = ph1; break;
        case 2: valor = ph2; break;
        case 3: valor = ph3; break;
        }

        // Atualiza faixa e valor da barra para pH (0–14)
        SendMessage(hProg, PBM_SETRANGE, 0, MAKELPARAM(0, 14));
        SendMessage(hProg, PBM_SETPOS, valor, 0);
    }

    // Atualiza o valor mostrado na caixa de texto
    SetDlgItemInt(hDlg, IDC_TEMP, valor, FALSE);
}



INT_PTR CALLBACK MainDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG: 
    {
        //FORÇA O ICONE NA JANELA E NA BARRA DE TAREFAS.
        HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
        SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

		//Botão de Temperatura / Ph / Tanque selecionado
		CheckDlgButton(hDlg, IDC_TANQUE1, BST_CHECKED);   // Estado inicial do botão do tanque 1
        CheckDlgButton(hDlg, IDC_RADIOTEMP, BST_CHECKED); // Estado inicial do botão de temperatura
		SetDlgItemInt(hDlg, IDC_TEMP, temp1, FALSE); // Inicializa o valor de temperatura com a variável global temp1


		// Inicializa os valores dos controles com as variáveis globais
        SetDlgItemInt(hDlg, IDC_DESPERDICIO, desperdicio, FALSE);
        SetDlgItemInt(hDlg, IDC_VOLUME, volume, FALSE);

		// Inicializa a barra de progresso
		SendDlgItemMessage(hDlg, IDC_PROGRESSTEMP, PBM_SETRANGE, 0, MAKELPARAM(0, 100)); // Limites de escala da barra de progresso
        SendDlgItemMessage(hDlg, IDC_PROGRESSTEMP, PBM_SETPOS, temperatura, 0); // Valor da barra de progresso
        HWND hProg = GetDlgItem(hDlg, IDC_PROGRESSTEMP);
        SendMessage(hProg, PBM_SETBARCOLOR, 0, (LPARAM)RGB(150, 0, 200)); // CORES RGB (VALOR RED, VALOR GREEN, VALOR BLUE)
		//SendMessage(hProg, PBM_SETBKCOLOR, 0, (LPARAM)RGB(230, 230, 230)); // SE QUISER MUDAR A COR DE FUNDO DA BARRA DE PROGRESSO.
        
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
		// Botões de controle
		case IDC_RADIOTEMP: // Botão de temperatura selecionado
        case IDC_RADIOPH: // Botão de pH selecionado
		case IDC_TANQUE1: // Botão do tanque 1 selecionado
		case IDC_TANQUE2: // Botão do tanque 2 selecionado
		case IDC_TANQUE3: // Botão do tanque 3 selecionado
			AtualizaValorTempPh(hDlg); // Chama a função para atualizar o valor de temperatura ou pH
            break;

        // Coisas da janela extra, caso precise.
        case IDC_MENU:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_MENU), hDlg, MenuDialogProc);
            break;
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

//Dialog de menu
INT_PTR CALLBACK MenuDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        // Você pode inicializar a progress bar aqui, se quiser
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

