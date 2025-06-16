
#include "Atualiza.h"
#include "Timer.h"
#include <windows.h>
#include <commctrl.h>  // Para PBM_SETPOS, etc.
#include "framework.h"
#include "WindowsProject.h"
#include <cwchar>  // Para usar swprintf com wchar_t

extern Entradas entradas;

// Declare externs se as variáveis estão no main.cpp
extern float volumeMixer, volumeCongelamento, volumeCreme, volumeMorango, volumeChocolate, volumePote;
extern int tempoCreme, tempoMorango, tempoChocolate;
extern float mixerR, mixerG, mixerB, targetR, targetG, targetB, congeladoR, congeladoG, congeladoB;
extern bool primeiraCorMixer, poteAbaixoDoMixer, mostrarMix1;
extern int temperaturaMixer, tempTargetMixer, tempcongelamento, tempTargetCongelamento;
extern float phMixer, phTargetMixer;
const int R_CREME = 255, G_CREME = 255, B_CREME = 100;
const int R_MORANGO = 255, G_MORANGO = 80, B_MORANGO = 80;
const int R_CHOCOLATE = 123, G_CHOCOLATE = 63, B_CHOCOLATE = 0;
extern int temperaturaCreme, temperaturaMorango, temperaturaChocolate, phCreme, phMorango, phChocolate; // Temperaturas e ph's para creme, morango e chocolate

static int pos_x = 340; // posição inicial X 
static int pos_y = 592; // posição Y fixa 
static float pos_barra = 340.0f; // posição da barra de progresso do pote
UINT tempoAnimacaoMix = 0;
float vazao = 0.02f; // vazão em litros 0.2L/s // 0.002f= 2mL/s (2mL por segundo)



void HandleTimer(HWND hDlg, WPARAM wParam, LPARAM lParam) { // Função chamada pelo timer
    if (wParam == 1) {
        {
            if (pos_x >= 635 && pos_x <= 705)
                poteAbaixoDoMixer = true;
            else
                poteAbaixoDoMixer = false;

            // Transfere conteúdo do mixer para congelamento
			if ((IsDlgButtonChecked(hDlg, IDC_CHECKMIXER) == BST_CHECKED || entradas.valvulaMixer) && volumeMixer >= vazao) { // Verifica se o mixer está ativo e se há volume suficiente
                // Se for via sinal digital (saidas.nivelSaida) e temperatura ainda não foi sincronizada
                static bool sincronizouTemp = false;
                if (!sincronizouTemp) {
                    tempcongelamento = temperaturaMixer;
                    tempTargetCongelamento = temperaturaMixer;

                    WCHAR buffer[16];
                    swprintf(buffer, 16, L"%d(\x00B0\C)", tempcongelamento);
                    SetDlgItemText(hDlg, IDC_TPHCONGELAMENTO, buffer);

                    HWND hProgTemp = GetDlgItem(hDlg, IDC_PROGRESSTEMP);
                    SendMessage(hProgTemp, PBM_SETPOS, tempcongelamento, 0);

                    sincronizouTemp = true;
                }
                else {
                    sincronizouTemp = false; // Reseta para permitir nova sincronização na próxima ativação
                }

                // Atualiza numericamente os volumes
                volumeMixer -= vazao; // Reduz o volume do mixer
				volumeCongelamento += vazao; // Aumenta o volume de congelamento
                congeladoR = mixerR;
                congeladoG = mixerG;
                congeladoB = mixerB;


                // Barra de progresso de congelamento ganha a cor do mixer
                HWND hProgCong = GetDlgItem(hDlg, IDC_PROGRESSCONGELAMENTO); 
                SendMessage(hProgCong, PBM_SETBARCOLOR, 0, (LPARAM)RGB((int)mixerR, (int)mixerG, (int)mixerB)); 

                // Atualiza visualmente os volumes
                SendDlgItemMessage(hDlg, IDC_PROGRESSMIXER, PBM_SETPOS, (int)volumeMixer, 0);
                SendDlgItemMessage(hDlg, IDC_PROGRESSCONGELAMENTO, PBM_SETPOS, (int)volumeCongelamento, 0);
                AtualizaVolumes(hDlg);

				// Limita os volumes para não ultrapassar os máximos
                if (volumeCongelamento > 40.0f) volumeCongelamento = 40.0f;
                if (volumeMixer < 0.0f) volumeMixer = 0.0f;


                if (volumeMixer <= 0.01f) {
                    mixerR = mixerG = mixerB = 0;
                    targetR = targetG = targetB = 0;
                    primeiraCorMixer = true;
                    tempoCreme = tempoMorango = tempoChocolate = 0;
                    temperaturaMixer = 0;
                    phMixer = 0;
                    tempTargetMixer = -1;
                    phTargetMixer = -1;  


                    sincronizouTemp = false; 

                    // Atualiza imediatamente o Valor numérico após zerar o mixer
                    WCHAR buffer[16];
                    if (IsDlgButtonChecked(hDlg, IDC_RADIOTEMP) == BST_CHECKED)
                        swprintf(buffer, 16, L"%d(\x00B0\C)", temperaturaMixer);
                    else
                        swprintf(buffer, 16, L"pH %.1f", phMixer);
                    SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);
                }


            }
            //===============================================================================================
            if (IsDlgButtonChecked(hDlg, IDC_ESTEIRAON) == BST_CHECKED || entradas.esteira) // Se D13 está em nível alto, move...
            {
                OutputDebugStringA("? ESTEIRA ATIVADA VIA PINO D13\n");
                //===============================================================================================
                pos_x += 1;
                pos_barra += 0.99999f;
                volumePote += 0;

                // Mostra a barra de progresso
                ShowWindow(GetDlgItem(hDlg, IDC_PROGRESSPOTE), SW_SHOW);

                if (pos_x > 1000) {
                    pos_x = 340;
                    pos_barra = 340.0f;
                    volumePote = 0;
                    SendDlgItemMessage(hDlg, IDC_PROGRESSPOTE, PBM_SETPOS, (int)volumePote, 0);
                    AtualizaVolumes(hDlg);
                    ShowWindow(GetDlgItem(hDlg, IDC_SPLASH), SW_HIDE);
                }

                SetWindowPos(GetDlgItem(hDlg, IDC_POTE), NULL, pos_x, pos_y, 0, 0,
                    SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

                SetWindowPos(GetDlgItem(hDlg, IDC_PROGRESSPOTE), HWND_TOP, (int)(pos_barra + 5), pos_y + 10, 0, 0,
                    SWP_NOSIZE | SWP_SHOWWINDOW);
                
                if (volumePote >= 0)
                {
                    ShowWindow(GetDlgItem(hDlg, IDC_PROGRESSPOTE), SW_HIDE);
                }
            }
            else {
                // ESTEIRA PARADA
                
                // Mostra a barra de progresso se o volume estiver abaixo do limite
                ShowWindow(GetDlgItem(hDlg, IDC_PROGRESSPOTE), SW_SHOW);
            }



        }
        // Verifica nível baixo de qualquer tanque
        bool alertaBaixo = (volumeCreme <= 5.0f || volumeMorango <= 5.0f || volumeChocolate <= 5.0f);

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
        if ((IsDlgButtonChecked(hDlg, IDC_CHECKCREME) == BST_CHECKED || entradas.valvulaCreme) && volumeCreme >= vazao) {
            volumeCreme -= vazao;
            volumeMixer += vazao;
            tempoCreme++;
            setCorAlvo(R_CREME, G_CREME, B_CREME);
            temperaturaMixer = temperaturaCreme;
            tempTargetMixer = temperaturaCreme;
            if (phTargetMixer < 0.0f && volumeMixer == 0.0f) {
                phMixer = (float)phCreme; // primeira vez: seta direto
            }
            phTargetMixer = (float)phCreme; // sempre atualiza alvo
            WCHAR buffer[16];
            swprintf(buffer, 16, L"%d(\x00B0\C)", temperaturaMixer);
            SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);
            atualizou = true;
        }
        else if ((IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO) == BST_CHECKED || entradas.valvulaMorango) && volumeMorango >= vazao) {
            volumeMorango -= vazao;
            volumeMixer += vazao;
            tempoMorango++;
            setCorAlvo(R_MORANGO, G_MORANGO, B_MORANGO);
            temperaturaMixer = temperaturaMorango;
            tempTargetMixer = temperaturaMorango;
            if (phTargetMixer < 0.0f && volumeMixer == 0.0f) {
                phMixer = (float)phMorango;
            }
            phTargetMixer = (float)phMorango;
            WCHAR buffer[16];
            swprintf(buffer, 16, L"%d(\x00B0\C)", temperaturaMixer);
            SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);
            atualizou = true;
        }
        else if ((IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE) == BST_CHECKED || entradas.valvulaChocolate) && volumeChocolate >= vazao) {
            volumeChocolate -= vazao;
            volumeMixer += vazao;
            tempoChocolate++;
            setCorAlvo(R_CHOCOLATE, G_CHOCOLATE, B_CHOCOLATE);
            temperaturaMixer = temperaturaChocolate;
            tempTargetMixer = temperaturaChocolate;
            if (phTargetMixer < 0.0f && volumeMixer == 0.0f) {
                phMixer = (float)phChocolate;
            }
            phTargetMixer = (float)phChocolate;
            WCHAR buffer[16];
            swprintf(buffer, 16, L"%d(\x00B0\C)", temperaturaMixer);
            SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);
            atualizou = true;
        }



        // Limita valores para não ultrapassar os máximos
        if (volumeMixer > 70.0f) volumeMixer = 70.0f;
        if (volumeCreme < 0.0f) volumeCreme = 0.0f;
        if (volumeMorango < 0.0f) volumeMorango = 0.0f;
        if (volumeChocolate < 0.0f) volumeChocolate = 0.0f;

        if (atualizou) {
            AtualizaVolumes(hDlg);
            // Atualiza barras de progresso correspondentes
            SendDlgItemMessage(hDlg, IDC_CREME, PBM_SETPOS, (int)volumeCreme, 0);
            SendDlgItemMessage(hDlg, IDC_MORANGO, PBM_SETPOS, (int)volumeMorango, 0);
            SendDlgItemMessage(hDlg, IDC_CHOCOLATE, PBM_SETPOS, (int)volumeChocolate, 0);
            SendDlgItemMessage(hDlg, IDC_PROGRESSMIXER, PBM_SETPOS, (int)volumeMixer, 0);

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
            if (volumeMixer <= 0.0f) {
                mixerR = mixerG = mixerB = 0;
                targetR = targetG = targetB = 0;
                primeiraCorMixer = true;
                tempoCreme = tempoMorango = tempoChocolate = 0;
                phMixer = 0;
                phTargetMixer = -1;
            }

        }
        //PASSA O SORVETE PRO POTE
        if ((entradas.valvulaCongelamento || IsDlgButtonChecked(hDlg, IDC_CHECKCONGELADO) == BST_CHECKED) && poteAbaixoDoMixer && volumeCongelamento >= vazao )
        {
            volumeCongelamento -= vazao;
            volumePote += vazao;

            // Limita os volumes numericos
            if (volumePote > 2.5f) volumePote = 2.5f;
            if (volumeCongelamento < 0.0f) volumeCongelamento = 0.0f;

            // Atualiza visualmente as barras
            SendDlgItemMessage(hDlg, IDC_PROGRESSCONGELAMENTO, PBM_SETPOS, (int)volumeCongelamento, 0); // Atualiza a barra de congelamento visualmente
            SendDlgItemMessage(hDlg, IDC_PROGRESSPOTE, PBM_SETPOS, (int)volumePote, 0); // Atualiza a barra do pote visualmente
            AtualizaVolumes(hDlg); // Atualiza numericamente os volumes do pote na interface

            // Barra de progresso do pote ganha a cor do congelamento
            HWND hProgPote = GetDlgItem(hDlg, IDC_PROGRESSPOTE); 
            SendMessage(hProgPote, PBM_SETBARCOLOR, 0, (LPARAM)RGB((int)congeladoR, (int)congeladoG, (int)congeladoB));


            if (volumePote >= 2.5f) {
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
            if (IsDlgButtonChecked(hDlg, IDC_CHECKCREME) == BST_CHECKED ||
                IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO) == BST_CHECKED ||
                IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE) == BST_CHECKED || 
                entradas.valvulaCreme || entradas.valvulaMorango || entradas.valvulaChocolate) 
            {
                if (tempTargetMixer != -1 && temperaturaMixer != tempTargetMixer) {
                    if (temperaturaMixer < tempTargetMixer)
                        temperaturaMixer += 1;
                    else if (temperaturaMixer > tempTargetMixer)
                        temperaturaMixer -= 1;
                }
            }

            HWND hProgTemp = GetDlgItem(hDlg, IDC_PROGRESSTEMP);
            SendMessage(hProgTemp, PBM_SETPOS, tempcongelamento, 0);
        }


        // Controle gradual da temperatura do congelamento
        // Detecta transição da saída do mixer (nivelSaida)
        static bool saidaAnterior = false;

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
                    tempTargetCongelamento = -25;

                // Atualiza label
                WCHAR buffer[16];
                swprintf(buffer, 16, L"%d(\x00B0\C)", tempcongelamento);
                SetDlgItemText(hDlg, IDC_TPHCONGELAMENTO, buffer);
            }
        }

        if (volumeMixer > 0.0f && phTargetMixer >= 0.0f) {
            float passoPH = 0.05f;

            if (phMixer < phTargetMixer)
                phMixer = min(phTargetMixer, phMixer + passoPH);
            else if (phMixer > phTargetMixer)
                phMixer = max(phTargetMixer, phMixer - passoPH);
        }


        // Atualiza texto do TPHMIXER independentemente do modo
        WCHAR buffer[16];
        if (IsDlgButtonChecked(hDlg, IDC_RADIOTEMP) == BST_CHECKED)
            swprintf(buffer, 16, L"%d(\x00B0\C)", temperaturaMixer);
        else
            swprintf(buffer, 16, L"pH %.1f", phMixer);
        SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);

        // Atualiza campo lateral da barra (se for temperatura)
        if (IsDlgButtonChecked(hDlg, IDC_RADIOTEMP) == BST_CHECKED)
            SetDlgItemInt(hDlg, IDC_TEMP, tempcongelamento, FALSE);


        // Controle de animação do mixer (HÉLICE)
        if (tempoAnimacaoMix >= 100 &&
            (IsDlgButtonChecked(hDlg, IDC_CHECKCREME) == BST_CHECKED ||
                IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO) == BST_CHECKED ||
                IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE) == BST_CHECKED ||
                entradas.valvulaCreme || entradas.valvulaMorango || entradas.valvulaChocolate))
        {
            tempoAnimacaoMix = 0;
            mostrarMix1 = !mostrarMix1;

            ShowWindow(GetDlgItem(hDlg, IDC_MIX1), mostrarMix1 ? SW_SHOW : SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_MIX2), mostrarMix1 ? SW_HIDE : SW_SHOW);
        }
        else if (
            IsDlgButtonChecked(hDlg, IDC_CHECKCREME) != BST_CHECKED &&
            IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO) != BST_CHECKED &&
            IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE) != BST_CHECKED &&
            entradas.valvulaCreme && entradas.valvulaMorango && entradas.valvulaChocolate)
        {
            // Nenhum botão pressionado: desliga as imagens
            ShowWindow(GetDlgItem(hDlg, IDC_MIX1), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_MIX2), SW_HIDE);
            tempoAnimacaoMix = 0;
        }

        // Atualiza visualmente as válvulas com base nos sinais recebidos
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCREME), (entradas.valvulaCreme || IsDlgButtonChecked(hDlg, IDC_CHECKCREME)) ? SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCREME), (entradas.valvulaCreme || IsDlgButtonChecked(hDlg, IDC_CHECKCREME)) ? SW_HIDE : SW_SHOW);

        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONMORANGO), (entradas.valvulaMorango || IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO)) ? SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFMORANGO), (entradas.valvulaMorango || IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO)) ? SW_HIDE : SW_SHOW);

        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCHOCOLATE), (entradas.valvulaChocolate || IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE)) ? SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCHOCOLATE), (entradas.valvulaChocolate || IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE)) ? SW_HIDE : SW_SHOW);

        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONMIXER), (entradas.valvulaMixer || IsDlgButtonChecked(hDlg, IDC_CHECKMIXER)) ? SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFMIXER), (entradas.valvulaMixer || IsDlgButtonChecked(hDlg, IDC_CHECKMIXER)) ? SW_HIDE : SW_SHOW);

        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCONGELADO), (entradas.valvulaCongelamento || IsDlgButtonChecked(hDlg, IDC_CHECKCONGELADO)) ? SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCONGELADO), (entradas.valvulaCongelamento || IsDlgButtonChecked(hDlg, IDC_CHECKCONGELADO)) ? SW_HIDE : SW_SHOW);

    }
}



