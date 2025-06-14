﻿
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
extern float mixerR, mixerG, mixerB, targetR, targetG, targetB, congeladoR, congeladoG, congeladoB;
extern bool primeiraCorMixer, poteAbaixoDoMixer, mostrarMix1;
extern int tempmixer, tempTargetMixer, tempcongelamento, tempTargetCongelamento;
extern float phmixer, phTargetMixer;
const int R_CREME = 255, G_CREME = 255, B_CREME = 100;
const int R_MORANGO = 255, G_MORANGO = 80, B_MORANGO = 80;
const int R_CHOCOLATE = 123, G_CHOCOLATE = 63, B_CHOCOLATE = 0;
extern int temp1, temp2, temp3, ph1, ph2, ph3; // Temperaturas e ph's para creme, morango e chocolate

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
			if ((IsDlgButtonChecked(hDlg, IDC_CHECKMIXER) == BST_CHECKED || saidas.nivelSaida) && volumemixer >= vazao) { // Verifica se o mixer está ativo e se há volume suficiente
                // Se for via sinal digital (saidas.nivelSaida) e temperatura ainda não foi sincronizada
                static bool sincronizouTemp = false;
                if (saidas.nivelSaida && !sincronizouTemp) {
                    tempcongelamento = tempmixer;
                    tempTargetCongelamento = tempmixer;

                    WCHAR buffer[16];
                    swprintf(buffer, 16, L"%d(\x00B0\C)", tempcongelamento);
                    SetDlgItemText(hDlg, IDC_TPHCONGELAMENTO, buffer);

                    HWND hProgTemp = GetDlgItem(hDlg, IDC_PROGRESSTEMP);
                    SendMessage(hProgTemp, PBM_SETPOS, tempcongelamento, 0);

                    sincronizouTemp = true;
                }
                else if (!saidas.nivelSaida) {
                    sincronizouTemp = false; // Reseta para permitir nova sincronização na próxima ativação
                }

                // Atualiza numericamente os volumes
                volumemixer -= vazao; // Reduz o volume do mixer
				volumecongelamento += vazao; // Aumenta o volume de congelamento
                congeladoR = mixerR;
                congeladoG = mixerG;
                congeladoB = mixerB;


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


                if (volumemixer <= 0.01f) {
                    mixerR = mixerG = mixerB = 0;
                    targetR = targetG = targetB = 0;
                    primeiraCorMixer = true;
                    tempoCreme = tempoMorango = tempoChocolate = 0;
                    tempmixer = 0;
                    phmixer = 0;
                    tempTargetMixer = -1;
                    phTargetMixer = -1;  


                    sincronizouTemp = false; 

                    // Atualiza imediatamente o Valor numérico após zerar o mixer
                    WCHAR buffer[16];
                    if (IsDlgButtonChecked(hDlg, IDC_RADIOTEMP) == BST_CHECKED)
                        swprintf(buffer, 16, L"%d(\x00B0\C)", tempmixer);
                    else
                        swprintf(buffer, 16, L"pH %.1f", phmixer);
                    SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);
                }


            }
            //===============================================================================================
            if (saidas.poteEsteira || IsDlgButtonChecked(hDlg, IDC_ESTEIRAON) == BST_CHECKED) // Se D13 está em nível alto, move...
            {
                OutputDebugStringA("? ESTEIRA ATIVADA VIA PINO D13\n");
                //===============================================================================================
                pos_x += 1;
                pos_barra += 0.99999f;
                volumepote += 0;

                // Mostra a barra de progresso
                ShowWindow(GetDlgItem(hDlg, IDC_PROGRESSPOTE), SW_SHOW);

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
                
                if (volumepote >= 0)
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
        if ((saidas.nivel1 || IsDlgButtonChecked(hDlg, IDC_CHECKCREME) == BST_CHECKED) && volumecreme >= vazao) {
            volumecreme -= vazao;
            volumemixer += vazao;
            tempoCreme++;
            setCorAlvo(R_CREME, G_CREME, B_CREME);
            tempmixer = temp1;
            tempTargetMixer = temp1;
            if (phTargetMixer < 0.0f && volumemixer == 0.0f) {
                phmixer = (float)ph1; // primeira vez: seta direto
            }
            phTargetMixer = (float)ph1; // sempre atualiza alvo
            WCHAR buffer[16];
            swprintf(buffer, 16, L"%d(\x00B0\C)", tempmixer);
            SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);
            atualizou = true;
        }
        else if ((saidas.nivel2 || IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO) == BST_CHECKED) && volumemorango >= vazao) {
            volumemorango -= vazao;
            volumemixer += vazao;
            tempoMorango++;
            setCorAlvo(R_MORANGO, G_MORANGO, B_MORANGO);
            tempmixer = temp2;
            tempTargetMixer = temp2;
            if (phTargetMixer < 0.0f && volumemixer == 0.0f) {
                phmixer = (float)ph2;
            }
            phTargetMixer = (float)ph2;
            WCHAR buffer[16];
            swprintf(buffer, 16, L"%d(\x00B0\C)", tempmixer);
            SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);
            atualizou = true;
        }
        else if ((saidas.nivel3 || IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE) == BST_CHECKED) && volumechocolate >= vazao) {
            volumechocolate -= vazao;
            volumemixer += vazao;
            tempoChocolate++;
            setCorAlvo(R_CHOCOLATE, G_CHOCOLATE, B_CHOCOLATE);
            tempmixer = temp3;
            tempTargetMixer = temp3;
            if (phTargetMixer < 0.0f && volumemixer == 0.0f) {
                phmixer = (float)ph3;
            }
            phTargetMixer = (float)ph3;
            WCHAR buffer[16];
            swprintf(buffer, 16, L"%d(\x00B0\C)", tempmixer);
            SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);
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
                phmixer = 0;
                phTargetMixer = -1;
            }

        }
        //PASSA O SORVETE PRO POTE
        if ((saidas.encherpote || IsDlgButtonChecked(hDlg, IDC_CHECKCONGELADO) == BST_CHECKED) && poteAbaixoDoMixer && volumecongelamento >= vazao )
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

            // Barra de progresso do pote ganha a cor do congelamento
            HWND hProgPote = GetDlgItem(hDlg, IDC_PROGRESSPOTE); 
            SendMessage(hProgPote, PBM_SETBARCOLOR, 0, (LPARAM)RGB((int)congeladoR, (int)congeladoG, (int)congeladoB));


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
            if (IsDlgButtonChecked(hDlg, IDC_CHECKCREME) == BST_CHECKED ||
                IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO) == BST_CHECKED ||
                IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE) == BST_CHECKED || 
                saidas.nivel1 || saidas.nivel2 || saidas.nivel3) 
            {
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
        // Detecta transição da saída do mixer (nivelSaida)
        static bool saidaAnterior = false;

        if (saidaAnterior && !saidas.nivelSaida) {
            // Quando a válvula de saída foi fechada (transição HIGH → LOW)
            tempTargetCongelamento = -25;
        }

        saidaAnterior = saidas.nivelSaida;

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

        if (volumemixer > 0.0f && phTargetMixer >= 0.0f) {
            float passoPH = 0.05f;

            if (phmixer < phTargetMixer)
                phmixer = min(phTargetMixer, phmixer + passoPH);
            else if (phmixer > phTargetMixer)
                phmixer = max(phTargetMixer, phmixer - passoPH);
        }


        // Atualiza texto do TPHMIXER independentemente do modo
        WCHAR buffer[16];
        if (IsDlgButtonChecked(hDlg, IDC_RADIOTEMP) == BST_CHECKED)
            swprintf(buffer, 16, L"%d(\x00B0\C)", tempmixer);
        else
            swprintf(buffer, 16, L"pH %.1f", phmixer);
        SetDlgItemText(hDlg, IDC_TPHMIXER, buffer);

        // Atualiza campo lateral da barra (se for temperatura)
        if (IsDlgButtonChecked(hDlg, IDC_RADIOTEMP) == BST_CHECKED)
            SetDlgItemInt(hDlg, IDC_TEMP, tempcongelamento, FALSE);


        // Controle de animação do mixer (HÉLICE)
        if (tempoAnimacaoMix >= 100 &&
            (IsDlgButtonChecked(hDlg, IDC_CHECKCREME) == BST_CHECKED ||
                IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO) == BST_CHECKED ||
                IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE) == BST_CHECKED ||
                saidas.nivel1 || saidas.nivel2 || saidas.nivel3))
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
            saidas.nivel1 && saidas.nivel2 && saidas.nivel3)
        {
            // Nenhum botão pressionado: desliga as imagens
            ShowWindow(GetDlgItem(hDlg, IDC_MIX1), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_MIX2), SW_HIDE);
            tempoAnimacaoMix = 0;
        }

        // Atualiza visualmente as válvulas com base nos sinais recebidos
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCREME), (saidas.nivel1 || IsDlgButtonChecked(hDlg, IDC_CHECKCREME)) ? SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCREME), (saidas.nivel1 || IsDlgButtonChecked(hDlg, IDC_CHECKCREME)) ? SW_HIDE : SW_SHOW);

        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONMORANGO), (saidas.nivel2 || IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO)) ? SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFMORANGO), (saidas.nivel2 || IsDlgButtonChecked(hDlg, IDC_CHECKMORANGO)) ? SW_HIDE : SW_SHOW);

        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCHOCOLATE), (saidas.nivel3 || IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE)) ? SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCHOCOLATE), (saidas.nivel3 || IsDlgButtonChecked(hDlg, IDC_CHECKCHOCOLATE)) ? SW_HIDE : SW_SHOW);

        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONMIXER), (saidas.nivelSaida || IsDlgButtonChecked(hDlg, IDC_CHECKMIXER)) ? SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFMIXER), (saidas.nivelSaida || IsDlgButtonChecked(hDlg, IDC_CHECKMIXER)) ? SW_HIDE : SW_SHOW);

        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAONCONGELADO), (saidas.encherpote || IsDlgButtonChecked(hDlg, IDC_CHECKCONGELADO)) ? SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_VALVULAOFFCONGELADO), (saidas.encherpote || IsDlgButtonChecked(hDlg, IDC_CHECKCONGELADO)) ? SW_HIDE : SW_SHOW);

    }
}



