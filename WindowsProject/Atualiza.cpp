#include "atualiza.h"
#include <windows.h>
#include <commctrl.h> // Para PBM_*
#include "resource.h" // Para os IDC_*
#include <cwchar>  // Para usar swprintf com wchar_t
#include "WindowsProject.h"

// Variáveis globais usadas nas atualizações (vindas de outro .cpp)
extern int temperaturaCreme, temperaturaMorango, temperaturaChocolate;
extern int temperaturaMixer, tempcongelamento;
extern int phCreme, phMorango, phChocolate;
extern float volumeCreme, volumeMorango, volumeChocolate, phMixer;
extern float volumeMixer, volumeCongelamento, volumePote;


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
        valor = temperaturaMixer;
    }
    else {
        SetDlgItemText(hDlg, IDC_STATICTPH, L"Valor de pH:");
        SetDlgItemText(hDlg, IDC_STATIC, L"Valor de pH:");
        valor = phMixer;
    }

    WCHAR buffer[16];
    swprintf(buffer, 16, L"%d(\x00B0\C)", temperaturaCreme);
    SetDlgItemText(hDlg, IDC_TPHCREME, buffer);
    swprintf(buffer, 16, L"%d(\x00B0\C)", temperaturaMorango);
    SetDlgItemText(hDlg, IDC_TPHMORANGO, buffer);
    swprintf(buffer, 16, L"%d(\x00B0\C)", temperaturaChocolate);
    SetDlgItemText(hDlg, IDC_TPHCHOCOLATE, buffer);
    swprintf(buffer, 16, L"%d(\x00B0\C)", temperaturaMixer);
    SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);
    swprintf(buffer, 16, L"%d(\x00B0\C)", tempcongelamento);
    SetDlgItemText(hDlg, IDC_TPHCONGELAMENTO, buffer);

    if (IsDlgButtonChecked(hDlg, IDC_RADIOPH) == BST_CHECKED) {
        swprintf(buffer, 16, L"pH %d", phCreme);
        SetDlgItemText(hDlg, IDC_TPHCREME, buffer);
        swprintf(buffer, 16, L"pH %d", phMorango);
        SetDlgItemText(hDlg, IDC_TPHMORANGO, buffer);
        swprintf(buffer, 16, L"pH %d", phChocolate);
        SetDlgItemText(hDlg, IDC_TPHCHOCOLATE, buffer);
        swprintf(buffer, 16, L"pH %d", phMixer);
        SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);
    }

    SetDlgItemInt(hDlg, IDC_TEMP, valor, FALSE);
}

void AtualizaVolumes(HWND hDlg) {
    WCHAR buffer[16];

    swprintf(buffer, 16, L"%.1f", volumeCreme);
    SetDlgItemText(hDlg, IDC_VOLUMECREME, buffer);

    swprintf(buffer, 16, L"%.1f", volumeMorango);
    SetDlgItemText(hDlg, IDC_VOLUMEMORANGO, buffer);

    swprintf(buffer, 16, L"%.1f", volumeChocolate);
    SetDlgItemText(hDlg, IDC_VOLUMECHOCOLATE, buffer);

    swprintf(buffer, 16, L"%.1f", volumeMixer);
    SetDlgItemText(hDlg, IDC_VOLUMEMIXER, buffer);

    swprintf(buffer, 16, L"%.1f", volumeCongelamento);
    SetDlgItemText(hDlg, IDC_VOLUMECONGELAMENTO, buffer);

    swprintf(buffer, 16, L"%.1f", volumePote);
    SetDlgItemText(hDlg, IDC_VOLUMEPOTE, buffer);
}
