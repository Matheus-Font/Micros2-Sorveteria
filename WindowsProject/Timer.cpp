
#include "Atualiza.h"
#include "Timer.h"
#include <windows.h>
#include <commctrl.h>  // Para PBM_SETPOS, etc.
#include "framework.h"
#include "WindowsProject.h"
#include <cwchar>  // Para usar swprintf com wchar_t

extern Saidas saidas;

// Declare externs se as variáveis estão no main.cpp
extern float volumemixer, volumecongelamento, volumecreme, volumemorango, volumechocolate, volumepote;
extern int tempoCreme, tempoMorango, tempoChocolate;
extern float mixerR, mixerG, mixerB, targetR, targetG, targetB;
extern bool primeiraCorMixer, poteAbaixoDoMixer, mostrarMix1;
extern int tempmixer, tempTargetMixer, tempcongelamento, tempTargetCongelamento, phmixer;
const int R_CREME = 255, G_CREME = 255, B_CREME = 100;
const int R_MORANGO = 255, G_MORANGO = 80, B_MORANGO = 80;
const int R_CHOCOLATE = 123, G_CHOCOLATE = 63, B_CHOCOLATE = 0;

static int pos_x = 340; // posição inicial X 
static int pos_y = 592; // posição Y fixa 
static float pos_barra = 340.0f; // posição da barra de progresso do pote
UINT tempoAnimacaoMix = 0;
float vazao = 0.02f; // vazão em litros 0.2L/s // 0.002f= 2mL/s (2mL por segundo)



void HandleTimer(HWND hDlg, WPARAM wParam, LPARAM lParam) { // Função chamada pelo timer
    if (wParam == 1) {
        {
            // Transfere conteúdo do mixer para congelamento
			if (IsDlgButtonChecked(hDlg, IDC_CHECKMIXER) == BST_CHECKED && volumemixer >= vazao) { // Verifica se o mixer está ativo e se há volume suficiente
				
                // Atualiza numericamente os volumes
                volumemixer -= vazao; // Reduz o volume do mixer
				volumecongelamento += vazao; // Aumenta o volume de congelamento

                // Barra de progresso de congelamento ganha a cor do mixer
                HWND hProgCong = GetDlgItem(hDlg, IDC_PROGRESSCONGELAMENTO); 
                SendMessage(hProgCong, PBM_SETBARCOLOR, 0, (LPARAM)RGB((int)mixerR, (int)mixerG, (int)mixerB)); 

                // Atualiza visualmente os volumes
                SendDlgItemMessage(hDlg, IDC_PROGRESSMIXER, PBM_SETPOS, (int)volumemixer, 0);
                SendDlgItemMessage(hDlg, IDC_PROGRESSCONGELAMENTO, PBM_SETPOS, (int)volumecongelamento, 0);
                AtualizaVolumes(hDlg);

				// Limita os volumes para não ultrapassar os máximos
                if (volumecongelamento > 40.0f) volumecongelamento = 40.0f;
                if (volumemixer < 0.0f) volumemixer = 0.0f;


				if (volumemixer <= 0.01f) { // Se o volume do mixer for muito baixo, reseta as cores e tempos
                    mixerR = mixerG = mixerB = 0;
                    targetR = targetG = targetB = 0;
                    primeiraCorMixer = true;
                    tempoCreme = tempoMorango = tempoChocolate = 0;
                    tempmixer = 0;
					phmixer = 0;    
                    tempTargetMixer = -1;

                    // Atualiza imediatamente o Valor numérico após zerar o mixer
                    WCHAR buffer[16];
                    if (IsDlgButtonChecked(hDlg, IDC_RADIOTEMP) == BST_CHECKED) {
                        swprintf(buffer, 16, L"%d(\x00B0\C)", tempmixer); // 0 °C
                    }
                    else {
                        swprintf(buffer, 16, L"pH %d", phmixer); // phmixer 0
                    }
                    SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);

                }

            }
            //===============================================================================================
            if (saidas.poteEsteira) // Se D13 está em nível alto, move...
            {
                OutputDebugStringA("? ESTEIRA ATIVADA VIA PINO D13\n");
                //===============================================================================================
                pos_x += 1;
                pos_barra += 1.0f;

                if (pos_x >= 635 && pos_x <= 705)
                    poteAbaixoDoMixer = true;
                else
                    poteAbaixoDoMixer = false;

                if (pos_x > 1000) {
                    pos_x = 340;
                    pos_barra = 340.0f;
                    volumepote = 0;
                    SendDlgItemMessage(hDlg, IDC_PROGRESSPOTE, PBM_SETPOS, (int)volumepote, 0);
                    AtualizaVolumes(hDlg);
                    ShowWindow(GetDlgItem(hDlg, IDC_SPLASH), SW_HIDE);
                }

                SetWindowPos(GetDlgItem(hDlg, IDC_POTE), NULL, pos_x, pos_y, 0, 0,
                    SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

                SetWindowPos(GetDlgItem(hDlg, IDC_PROGRESSPOTE), HWND_TOP, (int)(pos_barra + 5), pos_y + 10, 0, 0,
                    SWP_NOSIZE | SWP_SHOWWINDOW);
            }
            else {
                // ESTEIRA PARADA
                poteAbaixoDoMixer = false;
            }



        }
        // Verifica nível baixo de qualquer tanque
        bool alertaBaixo = (volumecreme <= 5.0f || volumemorango <= 5.0f || volumechocolate <= 5.0f);

        // Mostra ou esconde o aviso
        ShowWindow(GetDlgItem(hDlg, IDC_WARNING), alertaBaixo ? SW_SHOW : SW_HIDE);

        // Função para definir a cor alvo do mixer
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
        if (volumemixer > 70.0f) volumemixer = 70.0f;
        if (volumecreme < 0.0f) volumecreme = 0.0f;
        if (volumemorango < 0.0f) volumemorango = 0.0f;
        if (volumechocolate < 0.0f) volumechocolate = 0.0f;

        if (atualizou) {
            AtualizaVolumes(hDlg);
            // Atualiza barras de progresso correspondentes
            SendDlgItemMessage(hDlg, IDC_CREME, PBM_SETPOS, (int)volumecreme, 0);
            SendDlgItemMessage(hDlg, IDC_MORANGO, PBM_SETPOS, (int)volumemorango, 0);
            SendDlgItemMessage(hDlg, IDC_CHOCOLATE, PBM_SETPOS, (int)volumechocolate, 0);
            SendDlgItemMessage(hDlg, IDC_PROGRESSMIXER, PBM_SETPOS, (int)volumemixer, 0);

			// MUDANÇA DE COR DO MIXER
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

			// Reseta a cor do mixer se o volume for zero
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

            // Limita os volumes numericos
            if (volumepote > 2.5f) volumepote = 2.5f;
            if (volumecongelamento < 0.0f) volumecongelamento = 0.0f;

            // Atualiza visualmente as barras
            SendDlgItemMessage(hDlg, IDC_PROGRESSCONGELAMENTO, PBM_SETPOS, (int)volumecongelamento, 0); // Atualiza a barra de congelamento visualmente
            SendDlgItemMessage(hDlg, IDC_PROGRESSPOTE, PBM_SETPOS, (int)volumepote, 0); // Atualiza a barra do pote visualmente
            AtualizaVolumes(hDlg); // Atualiza numericamente os volumes do pote na interface

            // Barra de progresso do pote ganha a cor do mixer = congelamento
            HWND hProgPote = GetDlgItem(hDlg, IDC_PROGRESSPOTE); 
            SendMessage(hProgPote, PBM_SETBARCOLOR, 0, (LPARAM)RGB((int)mixerR, (int)mixerG, (int)mixerB));

            if (volumepote >= 2.5f) {
                // Esconde a barra de progresso do pote
                ShowWindow(GetDlgItem(hDlg, IDC_PROGRESSPOTE), SW_HIDE);
                // Mostra o splash
                ShowWindow(GetDlgItem(hDlg, IDC_SPLASH), SW_SHOW);
            }
            else {
                // Mostra a barra de progresso se o volume estiver abaixo do limite
                ShowWindow(GetDlgItem(hDlg, IDC_PROGRESSPOTE), SW_SHOW);
                // Esconde o splash
                ShowWindow(GetDlgItem(hDlg, IDC_SPLASH), SW_HIDE);
            }

        }

        //ANIMAÇÃO DO MIXER
        tempoAnimacaoMix += 10;

        // Controle gradual da temperatura do mixer
        static int delayTemp = 0;
        delayTemp++;
        if (delayTemp >= 10) {
            delayTemp = 0;

            // Só atualiza temperatura se algum botão de creme, morango ou chocolate estiver marcado
            if (
                IsDlgButtonChecked(hDlg, IDC_CHECKCREME) == BST_CHECKED ||
                IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO) == BST_CHECKED ||
                IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE) == BST_CHECKED
                ) {
                if (tempTargetMixer != -1 && tempmixer != tempTargetMixer) {
                    if (tempmixer < tempTargetMixer)
                        tempmixer += 1;
                    else if (tempmixer > tempTargetMixer)
                        tempmixer -= 1;
                }
            }

            HWND hProgTemp = GetDlgItem(hDlg, IDC_PROGRESSTEMP);
            SendMessage(hProgTemp, PBM_SETPOS, tempcongelamento, 0);
        }


        // Controle gradual da temperatura do congelamento
        static int delayTempCong = 0;
        delayTempCong++;
        if (delayTempCong >= 10) {
            delayTempCong = 0;

            if (tempcongelamento != tempTargetCongelamento) {
                if (tempcongelamento < tempTargetCongelamento)
                    tempcongelamento += 1;
                else if (tempcongelamento > tempTargetCongelamento)
                    tempcongelamento -= 1;
                if (tempTargetCongelamento < 5)
                    tempTargetCongelamento = 5;

                // Atualiza label
                WCHAR buffer[16];
                swprintf(buffer, 16, L"%d(\x00B0\C)", tempcongelamento);
                SetDlgItemText(hDlg, IDC_TPHCONGELAMENTO, buffer);
            }
        }


        // Atualiza texto do TPHMIXER independentemente do modo
        WCHAR buffer[16];
        if (IsDlgButtonChecked(hDlg, IDC_RADIOTEMP) == BST_CHECKED)
            swprintf(buffer, 16, L"%d(\x00B0\C)", tempmixer);
        else
            swprintf(buffer, 16, L"pH %d", phmixer);
        SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);

        // Atualiza campo lateral da barra (se for temperatura)
        if (IsDlgButtonChecked(hDlg, IDC_RADIOTEMP) == BST_CHECKED)
            SetDlgItemInt(hDlg, IDC_TEMP, tempcongelamento, FALSE);


        // Controle de animação do mixer (HÉLICE)
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
    }
}



