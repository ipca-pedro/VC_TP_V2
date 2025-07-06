#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Header.h" 

// NOTA: 'Header.h' também deverá ser atualizado com os novos nomes das funções e parâmetros.

/**
 * Função: vc_imagem_nova
 * Descrição: Aloca memória para uma nova imagem IVC com as dimensões e parâmetros especificados.
 * Parâmetros:
 *   - largura: largura da imagem em píxeis
 *   - altura: altura da imagem em píxeis
 *   - canais: número de canais (ex: 1 para cinzento, 3 para BGR)
 *   - niveis: número de níveis de intensidade (normalmente 256)
 * Retorna: Ponteiro para a estrutura IVC criada, ou NULL em caso de erro.
 */
IVC* vc_imagem_nova(int largura, int altura, int canais, int niveis) {
    IVC* imagem = (IVC*)malloc(sizeof(IVC));
    if (imagem == NULL) return NULL;

    imagem->width = largura;
    imagem->height = altura;
    imagem->channels = canais;
    imagem->levels = niveis;
    imagem->bytesperline = largura * canais;
    imagem->data = (unsigned char*)malloc(largura * altura * canais * sizeof(unsigned char));

    if (imagem->data == NULL) {
        free(imagem);
        return NULL;
    }
    return imagem;
}

/**
 * Função: vc_imagem_free
 * Descrição: Liberta a memória alocada para uma imagem IVC.
 * Parâmetros:
 *   - imagem: ponteiro para a estrutura IVC a libertar
 * Retorna: NULL para permitir atribuição (ex: imagem = vc_imagem_free(imagem);).
 */
IVC* vc_imagem_free(IVC* imagem) {
    if (imagem != NULL) {
        if (imagem->data != NULL) {
            free(imagem->data);
            imagem->data = NULL;
        }
        free(imagem);
        imagem = NULL;
    }
    return imagem;
}

/**
 * Função: vc_bgr_para_cinzento
 * Descrição: Converte uma imagem a cores (formato BGR) para tons de cinzento.
 * Parâmetros:
 *   - origem: ponteiro para a imagem de origem (a cores)
 *   - destino: ponteiro para a imagem de destino (tons de cinzento)
 * Retorna: 1 em caso de sucesso, 0 em caso de erro.
 */
int vc_bgr_para_cinzento(IVC* origem, IVC* destino) {
    if (!origem || !destino || !origem->data || !destino->data) return 0;
    if ((origem->width != destino->width) || (origem->height != destino->height)) return 0;
    if ((origem->channels != 3) || (destino->channels != 1)) return 0;

    unsigned char* dados_origem = (unsigned char*)origem->data;
    unsigned char* dados_destino = (unsigned char*)destino->data;
    int largura = origem->width;
    int altura = origem->height;

    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            long int indice_origem = y * origem->bytesperline + x * origem->channels;
            long int indice_destino = y * destino->bytesperline + x * destino->channels;

            // Ordem BGR usada pelo OpenCV
            float valor_azul = (float)dados_origem[indice_origem];
            float valor_verde = (float)dados_origem[indice_origem + 1];
            float valor_vermelho = (float)dados_origem[indice_origem + 2];

            // Fórmula de luminância padrão
            unsigned char valor_cinzento = (unsigned char)((valor_vermelho * 0.299) + (valor_verde * 0.587) + (valor_azul * 0.114));

            dados_destino[indice_destino] = valor_cinzento;
        }
    }
    return 1;
}

/**
 * Função: vc_cinzento_para_binario
 * Descrição: Binariza uma imagem em tons de cinzento com base num limiar.
 * Parâmetros:
 *   - origem: ponteiro para a imagem de origem (tons de cinzento)
 *   - destino: ponteiro para a imagem de destino (binária)
 *   - limiar: valor de limiarização (0-255)
 * Retorna: 1 em caso de sucesso, 0 em caso de erro.
 */
int vc_cinzento_para_binario(IVC* origem, IVC* destino, int limiar) {
    if (!origem || !destino || !origem->data || !destino->data) return 0;
    if ((origem->width != destino->width) || (origem->height != destino->height) || (origem->channels != destino->channels)) return 0;
    if (origem->channels != 1) return 0;

    unsigned char* dados_origem = (unsigned char*)origem->data;
    unsigned char* dados_destino = (unsigned char*)destino->data;
    int tamanho_total_pixeis = origem->width * origem->height;

    for (int i = 0; i < tamanho_total_pixeis; i++) {
        if (dados_origem[i] > limiar) {
            dados_destino[i] = 255; // Branco
        }
        else {
            dados_destino[i] = 0;   // Preto
        }
    }
    return 1;
}

/**
 * Função: vc_cinzento_negativo
 * Descrição: Gera o negativo de uma imagem em tons de cinzento.
 * Parâmetros:
 *   - imagem: ponteiro para a imagem a ser invertida (operação in-place).
 * Retorna: 1 em caso de sucesso, 0 em caso de erro.
 */
int vc_cinzento_negativo(IVC* imagem) {
    if (!imagem || !imagem->data || imagem->channels != 1) return 0;

    unsigned char* dados_imagem = (unsigned char*)imagem->data;
    int tamanho_total_pixeis = imagem->width * imagem->height;

    for (int i = 0; i < tamanho_total_pixeis; i++) {
        dados_imagem[i] = 255 - dados_imagem[i];
    }
    return 1;
}

/**
 * Função: vc_desenha_caixa_delimitadora
 * Descrição: Desenha a caixa delimitadora (bounding box) de um blob numa imagem a cores.
 * Parâmetros:
 *   - imagem: ponteiro para a imagem de destino (a cores)
 *   - blob: ponteiro para a estrutura OVC do blob
 * Retorna: 1 em caso de sucesso, 0 em caso de erro.
 */
int vc_desenha_caixa_delimitadora(IVC* imagem, OVC* blob) {
    if (!imagem || !blob || !imagem->data || imagem->channels != 3) return 0;

    int x_inicial = blob->x;
    int y_inicial = blob->y;
    int largura_blob = blob->width;
    int altura_blob = blob->height;
    unsigned char azul = 0, verde = 255, vermelho = 0; // Cor Verde

    // Linhas verticais
    for (int y = y_inicial; y < y_inicial + altura_blob; y++) {
        if (y < 0 || y >= imagem->height) continue;

        // Linha esquerda
        if (x_inicial >= 0 && x_inicial < imagem->width) {
            long int indice = y * imagem->bytesperline + x_inicial * imagem->channels;
            imagem->data[indice] = azul;
            imagem->data[indice + 1] = verde;
            imagem->data[indice + 2] = vermelho;
        }
        // Linha direita
        int x_final = x_inicial + largura_blob - 1;
        if (x_final >= 0 && x_final < imagem->width) {
            long int indice = y * imagem->bytesperline + x_final * imagem->channels;
            imagem->data[indice] = azul;
            imagem->data[indice + 1] = verde;
            imagem->data[indice + 2] = vermelho;
        }
    }

    // Linhas horizontais
    for (int x = x_inicial; x < x_inicial + largura_blob; x++) {
        if (x < 0 || x >= imagem->width) continue;

        // Linha superior
        if (y_inicial >= 0 && y_inicial < imagem->height) {
            long int indice = y_inicial * imagem->bytesperline + x * imagem->channels;
            imagem->data[indice] = azul;
            imagem->data[indice + 1] = verde;
            imagem->data[indice + 2] = vermelho;
        }
        // Linha inferior
        int y_final = y_inicial + altura_blob - 1;
        if (y_final >= 0 && y_final < imagem->height) {
            long int indice = y_final * imagem->bytesperline + x * imagem->channels;
            imagem->data[indice] = azul;
            imagem->data[indice + 1] = verde;
            imagem->data[indice + 2] = vermelho;
        }
    }
    return 1;
}

/**
 * Função: vc_desenha_centro_massa
 * Descrição: Desenha o centro de massa de um blob (como uma cruz) numa imagem a cores.
 * Parâmetros:
 *   - imagem: ponteiro para a imagem de destino (a cores)
 *   - blob: ponteiro para a estrutura OVC do blob
 *   - tamanho_cruz: tamanho (em píxeis) de cada braço da cruz
 * Retorna: 1 em caso de sucesso, 0 em caso de erro.
 */
int vc_desenha_centro_massa(IVC* imagem, OVC* blob, int tamanho_cruz) {
    if (!imagem || !blob || !imagem->data || imagem->channels != 3) return 0;

    int centro_x = blob->xc;
    int centro_y = blob->yc;
    unsigned char azul = 0, verde = 0, vermelho = 255; // Cor Vermelha

    // Linha horizontal da cruz
    for (int x = centro_x - tamanho_cruz; x <= centro_x + tamanho_cruz; x++) {
        if (x >= 0 && x < imagem->width && centro_y >= 0 && centro_y < imagem->height) {
            long int indice = centro_y * imagem->bytesperline + x * imagem->channels;
            imagem->data[indice] = azul;
            imagem->data[indice + 1] = verde;
            imagem->data[indice + 2] = vermelho;
        }
    }

    // Linha vertical da cruz
    for (int y = centro_y - tamanho_cruz; y <= centro_y + tamanho_cruz; y++) {
        if (y >= 0 && y < imagem->height && centro_x >= 0 && centro_x < imagem->width) {
            long int indice = y * imagem->bytesperline + centro_x * imagem->channels;
            imagem->data[indice] = azul;
            imagem->data[indice + 1] = verde;
            imagem->data[indice + 2] = vermelho;
        }
    }
    return 1;
}

/**
 * Função: vc_blob_cor_a_descartar
 * Descrição: Determina se um blob deve ser descartado com base na sua cor média (convertida para HSV).
 * Parâmetros:
 *   - imagem_cor: ponteiro para a imagem a cores de onde se extrai a cor
 *   - info_blob: ponteiro para a estrutura OVC do blob
 *   - nome_ficheiro: nome do ficheiro de vídeo para lógicas específicas
 * Retorna: 1 se a cor for para descartar, 0 caso contrário.
 */
int vc_blob_cor_a_descartar(IVC* imagem_cor, OVC* info_blob, const char* nome_ficheiro) {
    if (!imagem_cor || !info_blob || !imagem_cor->data || imagem_cor->channels != 3) return 0;

    // Amostragem de cor numa pequena região de interesse (ROI) à volta do centro de massa
    int tamanho_roi = 5;
    int metade_roi = tamanho_roi / 2;
    long long soma_r = 0, soma_g = 0, soma_b = 0;
    int contagem_pixeis = 0;

    for (int y = info_blob->yc - metade_roi; y <= info_blob->yc + metade_roi; y++) {
        for (int x = info_blob->xc - metade_roi; x <= info_blob->xc + metade_roi; x++) {
            if (x >= 0 && x < imagem_cor->width && y >= 0 && y < imagem_cor->height) {
                long int indice_pixel = y * imagem_cor->bytesperline + x * imagem_cor->channels;
                soma_b += imagem_cor->data[indice_pixel];
                soma_g += imagem_cor->data[indice_pixel + 1];
                soma_r += imagem_cor->data[indice_pixel + 2];
                contagem_pixeis++;
            }
        }
    }

    if (contagem_pixeis == 0) return 0; // Evita divisão por zero

    // Calcula a cor média e normaliza para o intervalo [0, 1]
    float media_r_norm = ((float)soma_r / contagem_pixeis) / 255.0f;
    float media_g_norm = ((float)soma_g / contagem_pixeis) / 255.0f;
    float media_b_norm = ((float)soma_b / contagem_pixeis) / 255.0f;

    // Conversão manual de RGB para HSV
    float h, s, v; // Hue, Saturation, Value (Matiz, Saturação, Valor)
    float max_cor = fmaxf(fmaxf(media_r_norm, media_g_norm), media_b_norm);
    float min_cor = fminf(fminf(media_r_norm, media_g_norm), media_b_norm);
    float delta = max_cor - min_cor;

    v = max_cor;
    s = (max_cor == 0.0f) ? 0.0f : delta / max_cor;

    if (s == 0.0f) { h = 0.0f; } // Cinzento, matiz indefinido
    else {
        if (max_cor == media_r_norm) { h = 60.0f * fmodf(((media_g_norm - media_b_norm) / delta), 6.0f); }
        else if (max_cor == media_g_norm) { h = 60.0f * (((media_b_norm - media_r_norm) / delta) + 2.0f); }
        else { h = 60.0f * (((media_r_norm - media_g_norm) / delta) + 4.0f); }
        if (h < 0.0f) { h += 360.0f; } // Garante que o matiz está entre 0 e 360
    }

    // Lógica para definir se uma cor deve ser descartada (objetos coloridos)
    int cor_e_vermelha = (h >= 340 || h <= 20) && (s > 0.5f) && (v > 0.3f);
    int cor_e_verde = (h >= 75 && h <= 175) && (s > 0.40f) && (v > 0.20f);
    int cor_e_azul = (h >= 180 && h <= 280) && (s > 0.4f) && (v > 0.3f);
    int cor_e_amarela = (h >= 45 && h <= 75) && (s > 0.7f) && (v > 0.6f);

    // Filtro dinâmico para objetos pretos, ajustado por vídeo
    int cor_e_preta = 0;
    if (strcmp(nome_ficheiro, "video1.mp4") == 0) {
        cor_e_preta = v < 0.12f; // Limiar de brilho baixo para video1
    }
    else if (strcmp(nome_ficheiro, "video2.mp4") == 0) {
        cor_e_preta = v < 0.18f; // Limiar de brilho baixo para video2
    }

    // Se a cor corresponder a qualquer um dos critérios, o blob é descartado
    return (cor_e_vermelha || cor_e_verde || cor_e_azul || cor_e_amarela || cor_e_preta);
}

/* ============================================================================
 * Operações Morfológicas Binárias
 * ==========================================================================*/

 /**
  * Função: vc_binario_erosao
  * Descrição: Realiza a operação de erosão numa imagem binária.
  * Parâmetros:
  *   - origem: ponteiro para a imagem de origem (binária)
  *   - destino: ponteiro para a imagem de destino (binária)
  *   - tamanho_kernel: tamanho do elemento estruturante (deve ser ímpar)
  * Retorna: 1 em caso de sucesso, 0 em caso de erro.
  */
int vc_binario_erosao(IVC* origem, IVC* destino, int tamanho_kernel) {
    if (!origem || !destino || !origem->data || !destino->data) return 0;
    if (origem->width != destino->width || origem->height != destino->height || origem->channels != 1 || destino->channels != 1) return 0;
    if (tamanho_kernel < 1 || tamanho_kernel % 2 == 0) return 0;

    int largura = origem->width, altura = origem->height, bytes_por_linha = origem->bytesperline;
    unsigned char* dados_origem = origem->data;
    unsigned char* dados_destino = destino->data;
    int metade_kernel = tamanho_kernel / 2;

    // Copia a imagem para o destino para não processar a mesma imagem que está a ser lida
    memcpy(dados_destino, dados_origem, largura * altura);

    for (int y = metade_kernel; y < altura - metade_kernel; y++) {
        for (int x = metade_kernel; x < largura - metade_kernel; x++) {
            unsigned char valor_minimo = 255;
            // Itera sobre a vizinhança definida pelo kernel
            for (int ky = -metade_kernel; ky <= metade_kernel; ky++) {
                for (int kx = -metade_kernel; kx <= metade_kernel; kx++) {
                    // Erosão encontra o valor mínimo na vizinhança
                    if (dados_origem[(y + ky) * bytes_por_linha + (x + kx)] < valor_minimo) {
                        valor_minimo = dados_origem[(y + ky) * bytes_por_linha + (x + kx)];
                    }
                }
            }
            dados_destino[y * bytes_por_linha + x] = valor_minimo;
        }
    }
    return 1;
}

/**
 * Função: vc_binario_dilatacao
 * Descrição: Realiza a operação de dilatação numa imagem binária.
 * Parâmetros:
 *   - origem: ponteiro para a imagem de origem (binária)
 *   - destino: ponteiro para a imagem de destino (binária)
 *   - tamanho_kernel: tamanho do elemento estruturante (deve ser ímpar)
 * Retorna: 1 em caso de sucesso, 0 em caso de erro.
 */
int vc_binario_dilatacao(IVC* origem, IVC* destino, int tamanho_kernel) {
    if (!origem || !destino || !origem->data || !destino->data) return 0;
    if (origem->width != destino->width || origem->height != destino->height || origem->channels != 1 || destino->channels != 1) return 0;
    if (tamanho_kernel < 1 || tamanho_kernel % 2 == 0) return 0;

    int largura = origem->width, altura = origem->height, bytes_por_linha = origem->bytesperline;
    unsigned char* dados_origem = origem->data;
    unsigned char* dados_destino = destino->data;
    int metade_kernel = tamanho_kernel / 2;

    memcpy(dados_destino, dados_origem, largura * altura);

    for (int y = metade_kernel; y < altura - metade_kernel; y++) {
        for (int x = metade_kernel; x < largura - metade_kernel; x++) {
            unsigned char valor_maximo = 0;
            // Itera sobre a vizinhança definida pelo kernel
            for (int ky = -metade_kernel; ky <= metade_kernel; ky++) {
                for (int kx = -metade_kernel; kx <= metade_kernel; kx++) {
                    // Dilatação encontra o valor máximo na vizinhança
                    if (dados_origem[(y + ky) * bytes_por_linha + (x + kx)] > valor_maximo) {
                        valor_maximo = dados_origem[(y + ky) * bytes_por_linha + (x + kx)];
                    }
                }
            }
            dados_destino[y * bytes_por_linha + x] = valor_maximo;
        }
    }
    return 1;
}

/**
 * Função: vc_binario_abertura
 * Descrição: Realiza a operação de abertura (erosão seguida de dilatação). Útil para remover ruído pequeno.
 * Parâmetros:
 *   - origem: ponteiro para a imagem de origem (binária)
 *   - destino: ponteiro para a imagem de destino (binária)
 *   - tamanho_kernel: tamanho do kernel (ímpar)
 *   - temp: imagem temporária auxiliar
 * Retorna: 1 em caso de sucesso, 0 em caso de erro.
 */
int vc_binario_abertura(IVC* origem, IVC* destino, int tamanho_kernel, IVC* temp) {
    if (!vc_binario_erosao(origem, temp, tamanho_kernel)) return 0;
    if (!vc_binario_dilatacao(temp, destino, tamanho_kernel)) return 0;
    return 1;
}

/**
 * Função: vc_binario_fecho
 * Descrição: Realiza a operação de fecho (dilatação seguida de erosão). Útil para preencher pequenos buracos.
 * Parâmetros:
 *   - origem: ponteiro para a imagem de origem (binária)
 *   - destino: ponteiro para a imagem de destino (binária)
 *   - tamanho_kernel: tamanho do kernel (ímpar)
 *   - temp: imagem temporária auxiliar
 * Retorna: 1 em caso de sucesso, 0 em caso de erro.
 */
int vc_binario_fecho(IVC* origem, IVC* destino, int tamanho_kernel, IVC* temp) {
    if (!vc_binario_dilatacao(origem, temp, tamanho_kernel)) return 0;
    if (!vc_binario_erosao(temp, destino, tamanho_kernel)) return 0;
    return 1;
}

/**
  * Função: vc_cinzento_box_blur
  * Descrição: Aplica um filtro de suavização (box blur) a uma imagem em tons de cinzento.
  * Parâmetros:
  *   - origem: ponteiro para a imagem de origem (tons de cinzento)
  *   - destino: ponteiro para a imagem de destino (tons de cinzento)
  *   - tamanho_kernel: tamanho do kernel (ímpar >= 3)
  * Retorna: 1 em caso de sucesso, 0 em caso de erro.
  */
int vc_cinzento_box_blur(IVC* origem, IVC* destino, int tamanho_kernel) {
    if (!origem || !destino || !origem->data || !destino->data) return 0;
    if (origem->width != destino->width || origem->height != destino->height || origem->channels != 1 || destino->channels != 1) return 0;
    if (tamanho_kernel < 3 || tamanho_kernel % 2 == 0) return 0;

    int largura = origem->width;
    int altura = origem->height;
    unsigned char* dados_origem = origem->data;
    unsigned char* dados_destino = destino->data;
    int metade_kernel = tamanho_kernel / 2;

    memcpy(dados_destino, dados_origem, largura * altura);

    for (int y = metade_kernel; y < altura - metade_kernel; y++) {
        for (int x = metade_kernel; x < largura - metade_kernel; x++) {
            long long int soma = 0;
            for (int ky = -metade_kernel; ky <= metade_kernel; ky++) {
                for (int kx = -metade_kernel; kx <= metade_kernel; kx++) {
                    soma += dados_origem[(y + ky) * largura + (x + kx)];
                }
            }
            dados_destino[y * largura + x] = (unsigned char)(soma / (tamanho_kernel * tamanho_kernel));
        }
    }
    return 1;
}

/**
 * Função: vc_desenha_linha_horizontal
 * Descrição: Desenha uma linha horizontal numa imagem a cores.
 * Parâmetros:
 *   - imagem: ponteiro para a imagem de destino (a cores)
 *   - y: coordenada vertical da linha
 *   - vermelho, verde, azul: componentes da cor da linha
 * Retorna: 1 em caso de sucesso, 0 em caso de erro.
 */
int vc_desenha_linha_horizontal(IVC* imagem, int y, int vermelho, int verde, int azul) {
    if (!imagem || !imagem->data || imagem->channels != 3 || y < 0 || y >= imagem->height) return 0;

    for (int x = 0; x < imagem->width; x++) {
        long int indice_pixel = y * imagem->bytesperline + x * imagem->channels;
        imagem->data[indice_pixel] = azul;
        imagem->data[indice_pixel + 1] = verde;
        imagem->data[indice_pixel + 2] = vermelho;
    }
    return 1;
}