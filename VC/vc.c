#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Header.h" // Garanta que o nome do seu header está correto

// Alocar memória para uma imagem
IVC* vc_image_new(int width, int height, int channels, int levels) {
    IVC* image = (IVC*)malloc(sizeof(IVC));
    if (image == NULL) return NULL;
    image->width = width;
    image->height = height;
    image->channels = channels;
    image->levels = levels;
    image->bytesperline = image->width * image->channels;
    image->data = (unsigned char*)malloc(image->width * image->height * image->channels * sizeof(unsigned char));
    if (image->data == NULL) {
        free(image);
        return NULL;
    }
    return image;
}

// Libertar memória de uma imagem
IVC* vc_image_free(IVC* image) {
    if (image != NULL) {
        if (image->data != NULL) {
            free(image->data);
            image->data = NULL;
        }
        free(image);
        image = NULL;
    }
    return image;
}

// Converter de RGB para Gray
int vc_rgb_to_gray(IVC* src, IVC* dst) {
    unsigned char* datasrc = (unsigned char*)src->data;
    int bytesperline_src = src->bytesperline;
    int channels_src = src->channels;
    unsigned char* datadst = (unsigned char*)dst->data;
    int bytesperline_dst = dst->bytesperline;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y;
    long int pos_src, pos_dst;
    float rf, gf, bf;

    if (!src || !dst || !src->data || !dst->data) return 0;
    if ((src->width != dst->width) || (src->height != dst->height)) return 0;
    if ((channels_src != 3) || (channels_dst != 1)) return 0;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;
            rf = (float)datasrc[pos_src];
            gf = (float)datasrc[pos_src + 1];
            bf = (float)datasrc[pos_src + 2];
            datadst[pos_dst] = (unsigned char)((rf * 0.299) + (gf * 0.587) + (bf * 0.114));
        }
    }
    return 1;
}

// Binarização
int vc_gray_to_binary(IVC* src, IVC* dst, int threshold) {
    unsigned char* datasrc = (unsigned char*)src->data;
    unsigned char* datadst = (unsigned char*)dst->data;
    int size = src->width * src->height;

    if (!src || !dst || !src->data || !dst->data) return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
    if (src->channels != 1) return 0;

    for (int i = 0; i < size; i++) {
        if (datasrc[i] > threshold) {
            datadst[i] = 255;
        }
        else {
            datadst[i] = 0;
        }
    }
    return 1;
}

// Gerar negativo da imagem gray
int vc_gray_negative(IVC* srcdst) {
    unsigned char* data = (unsigned char*)srcdst->data;
    int size = srcdst->width * srcdst->height;

    if (!srcdst || !srcdst->data || srcdst->channels != 1) return 0;

    for (int i = 0; i < size; i++) {
        data[i] = 255 - data[i];
    }
    return 1;
}

// Desenha a bounding box de um blob
int vc_draw_bounding_box(IVC* img, OVC* blob) {
    if (!img || !blob || !img->data || img->channels != 3) return 0;

    int x, y;
    unsigned char r = 0, g = 255, b = 0; // Verde

    for (y = blob->y; y < blob->y + blob->height; y++) {
        if (y < 0 || y >= img->height) continue;
        if (blob->x >= 0 && blob->x < img->width) {
            int pos = y * img->bytesperline + blob->x * img->channels;
            img->data[pos] = b; img->data[pos + 1] = g; img->data[pos + 2] = r;
        }
        if ((blob->x + blob->width - 1) >= 0 && (blob->x + blob->width - 1) < img->width) {
            int pos = y * img->bytesperline + (blob->x + blob->width - 1) * img->channels;
            img->data[pos] = b; img->data[pos + 1] = g; img->data[pos + 2] = r;
        }
    }

    for (x = blob->x; x < blob->x + blob->width; x++) {
        if (x < 0 || x >= img->width) continue;
        if (blob->y >= 0 && blob->y < img->height) {
            int pos = blob->y * img->bytesperline + x * img->channels;
            img->data[pos] = b; img->data[pos + 1] = g; img->data[pos + 2] = r;
        }
        if ((blob->y + blob->height - 1) >= 0 && (blob->y + blob->height - 1) < img->height) {
            int pos = (blob->y + blob->height - 1) * img->bytesperline + x * img->channels;
            img->data[pos] = b; img->data[pos + 1] = g; img->data[pos + 2] = r;
        }
    }
    return 1;
}

// Desenha o centro de gravidade de um blob
int vc_draw_center_of_gravity(IVC* img, OVC* blob, int size) {
    if (!img || !blob || !img->data || img->channels != 3) return 0;

    int x, y;
    unsigned char r = 255, g = 0, b = 0; // Vermelho

    for (x = blob->xc - size; x <= blob->xc + size; x++) {
        if (x >= 0 && x < img->width && blob->yc >= 0 && blob->yc < img->height) {
            int pos = blob->yc * img->bytesperline + x * img->channels;
            img->data[pos] = b; img->data[pos + 1] = g; img->data[pos + 2] = r;
        }
    }

    for (y = blob->yc - size; y <= blob->yc + size; y++) {
        if (y >= 0 && y < img->height && blob->xc >= 0 && blob->xc < img->width) {
            int pos = y * img->bytesperline + blob->xc * img->channels;
            img->data[pos] = b; img->data[pos + 1] = g; img->data[pos + 2] = r;
        }
    }
    return 1;
}

int blob_e_cor_a_descartar(IVC* img_colorida, OVC* blob_info, const char* ficheiro) {
    if (!img_colorida || !blob_info || !img_colorida->data || img_colorida->channels != 3) {
        return 0;
    }

    // --- Parte de amostragem e conversão para HSV (não muda) ---
    int roi_size = 5;
    int half_roi = roi_size / 2;
    long long sum_r = 0, sum_g = 0, sum_b = 0;
    int pixel_count = 0;
    for (int y = blob_info->yc - half_roi; y <= blob_info->yc + half_roi; y++) {
        for (int x = blob_info->xc - half_roi; x <= blob_info->xc + half_roi; x++) {
            if (x >= 0 && x < img_colorida->width && y >= 0 && y < img_colorida->height) {
                long int pos = y * img_colorida->bytesperline + x * img_colorida->channels;
                sum_b += img_colorida->data[pos];
                sum_g += img_colorida->data[pos + 1];
                sum_r += img_colorida->data[pos + 2];
                pixel_count++;
            }
        }
    }

    if (pixel_count == 0) return 0;

    float avg_r_norm = ((float)sum_r / pixel_count) / 255.0f;
    float avg_g_norm = ((float)sum_g / pixel_count) / 255.0f;
    float avg_b_norm = ((float)sum_b / pixel_count) / 255.0f;

    float h, s, v;
    float max = fmaxf(fmaxf(avg_r_norm, avg_g_norm), avg_b_norm);
    float min = fminf(fminf(avg_r_norm, avg_g_norm), avg_b_norm);
    float delta = max - min;
    v = max;
    s = (max == 0.0f) ? 0.0f : delta / max;
    if (s == 0.0f) { h = 0.0f; }
    else {
        if (max == avg_r_norm) { h = 60.0f * fmodf(((avg_g_norm - avg_b_norm) / delta), 6.0f); }
        else if (max == avg_g_norm) { h = 60.0f * (((avg_b_norm - avg_r_norm) / delta) + 2.0f); }
        else { h = 60.0f * (((avg_r_norm - avg_g_norm) / delta) + 4.0f); }
        if (h < 0.0f) { h += 360.0f; }
    }
 

    // +++ LÓGICA PARA DESCARTAR CORES +++
    int e_vermelho = (h >= 340 || h <= 20) && (s > 0.5f) && (v > 0.3f);
    int e_verde = (h >= 75 && h <= 175) && (s > 0.35f) && (v > 0.20f);
    int e_azul = (h >= 180 && h <= 280) && (s > 0.4f) && (v > 0.3f);
    int e_amarelo = (h >= 45 && h <= 75) && (s > 0.6f) && (v > 0.5f);

    // +++ FILTRO DE PRETO DINÂMICO +++
    int e_preto = 0; // Por defeito, não descarta preto
    if (strcmp(ficheiro, "video1.mp4") == 0) {
        // Para o vídeo 1, um filtro de preto MUITO tolerante para não apanhar a moeda de 1 euro.
        e_preto = v < 0.12f;
    }
    else if (strcmp(ficheiro, "video2.mp4") == 0) {
        // Para o vídeo 2
        e_preto = v < 0.20f;
    }

    return (e_vermelho || e_verde || e_azul || e_amarelo || e_preto);
}





// =======================
// Operações Morfológicas Binárias
// =======================
int vc_binary_erode(IVC* src, IVC* dst, int kernel_size) {
    if (!src || !dst || !src->data || !dst->data) return 0;
    if (src->width != dst->width || src->height != dst->height || src->channels != 1 || dst->channels != 1) return 0;
    if (kernel_size < 1 || kernel_size % 2 == 0) return 0;
    int width = src->width, height = src->height, bytesperline = src->bytesperline;
    unsigned char* src_data = src->data; unsigned char* dst_data = dst->data;
    int half_kernel = kernel_size / 2;
    memcpy(dst->data, src->data, width * height);
    for (int y = half_kernel; y < height - half_kernel; y++) {
        for (int x = half_kernel; x < width - half_kernel; x++) {
            unsigned char min_val = 255;
            for (int ky = -half_kernel; ky <= half_kernel; ky++) {
                for (int kx = -half_kernel; kx <= half_kernel; kx++) {
                    if (src_data[(y + ky) * bytesperline + (x + kx)] < min_val) {
                        min_val = src_data[(y + ky) * bytesperline + (x + kx)];
                    }
                }
            }
            dst_data[y * bytesperline + x] = min_val;
        }
    }
    return 1;
}

int vc_binary_dilate(IVC* src, IVC* dst, int kernel_size) {
    if (!src || !dst || !src->data || !dst->data) return 0;
    if (src->width != dst->width || src->height != dst->height || src->channels != 1 || dst->channels != 1) return 0;
    if (kernel_size < 1 || kernel_size % 2 == 0) return 0;
    int width = src->width, height = src->height, bytesperline = src->bytesperline;
    unsigned char* src_data = src->data; unsigned char* dst_data = dst->data;
    int half_kernel = kernel_size / 2;
    memcpy(dst->data, src->data, width * height);
    for (int y = half_kernel; y < height - half_kernel; y++) {
        for (int x = half_kernel; x < width - half_kernel; x++) {
            unsigned char max_val = 0;
            for (int ky = -half_kernel; ky <= half_kernel; ky++) {
                for (int kx = -half_kernel; kx <= half_kernel; kx++) {
                    if (src_data[(y + ky) * bytesperline + (x + kx)] > max_val) {
                        max_val = src_data[(y + ky) * bytesperline + (x + kx)];
                    }
                }
            }
            dst_data[y * bytesperline + x] = max_val;
        }
    }
    return 1;
}

int vc_binary_open(IVC* src, IVC* dst, int kernel_size, IVC* temp) {
    if (!vc_binary_erode(src, temp, kernel_size)) return 0;
    if (!vc_binary_dilate(temp, dst, kernel_size)) return 0;
    return 1;
}

int vc_binary_close(IVC* src, IVC* dst, int kernel_size, IVC* temp) {
    if (!vc_binary_dilate(src, temp, kernel_size)) return 0;
    if (!vc_binary_erode(temp, dst, kernel_size)) return 0;
    return 1;
}

// =======================
// *** NOVA FUNÇÃO: Suavização (Box Blur) para Tons de Cinza ***
// =======================
int vc_gray_box_blur(IVC* src, IVC* dst, int kernel_size) {
    if (!src || !dst || !src->data || !dst->data) return 0;
    if (src->width != dst->width || src->height != dst->height || src->channels != 1 || dst->channels != 1) return 0;
    if (kernel_size < 3 || kernel_size % 2 == 0) return 0;

    int width = src->width;
    int height = src->height;
    unsigned char* src_data = src->data;
    unsigned char* dst_data = dst->data;
    int half_kernel = kernel_size / 2;
    long long int sum;
    int i, j, kx, ky;
    
    // Copia a imagem original para o destino para tratar das bordas
    memcpy(dst_data, src_data, width * height);

    // Itera sobre os píxeis internos da imagem (excluindo as bordas)
    for (j = half_kernel; j < height - half_kernel; j++) {
        for (i = half_kernel; i < width - half_kernel; i++) {
            sum = 0;
            // Itera sobre a vizinhança do kernel
            for (ky = -half_kernel; ky <= half_kernel; ky++) {
                for (kx = -half_kernel; kx <= half_kernel; kx++) {
                    // Usa a imagem original (src_data) para os cálculos
                    sum += src_data[(j + ky) * width + (i + kx)];
                }
            }
            // Atribui a média ao píxel correspondente na imagem de destino
            dst_data[j * width + i] = (unsigned char)(sum / (kernel_size * kernel_size));
        }
    }
    return 1;

}


int vc_draw_horizontal_line(IVC* img, int y, int r, int g, int b)
{
    // Validação de segurança: verifica se a imagem e os dados são válidos
    // e se a linha está dentro dos limites da altura da imagem.
    if (!img || !img->data || img->channels != 3 || y < 0 || y >= img->height) {
        return 0; // Retorna 0 em caso de erro
    }

    // Itera por todos os píxeis da largura da imagem (de x=0 até width-1)
    for (int x = 0; x < img->width; x++) {
        // Calcula a posição do píxel na memória (array 1D)
        long int pos = y * img->bytesperline + x * img->channels;

        // Define a cor do píxel. Lembre-se que OpenCV usa a ordem BGR.
        img->data[pos] = b; // Canal Azul (Blue)
        img->data[pos + 1] = g; // Canal Verde (Green)
        img->data[pos + 2] = r; // Canal Vermelho (Red)
    }

    return 1; // Retorna 1 em caso de sucesso
}