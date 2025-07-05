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

// Função para verificar se um blob é vermelho. Retorna 1 se for vermelho, 0 caso contrário.
int blob_e_vermelho(IVC* img_colorida, OVC* blob_info) {
    if (!img_colorida || !blob_info || !img_colorida->data || img_colorida->channels != 3) {
        return 0;
    }

    int roi_size = 5;
    int half_roi = roi_size / 2;
    long long sum_r = 0, sum_g = 0, sum_b = 0;
    int pixel_count = 0;
    int x, y;

    for (y = blob_info->yc - half_roi; y <= blob_info->yc + half_roi; y++) {
        for (x = blob_info->xc - half_roi; x <= blob_info->xc + half_roi; x++) {
            if (x >= 0 && x < img_colorida->width && y >= 0 && y < img_colorida->height) {
                long int pos = y * img_colorida->bytesperline + x * img_colorida->channels;
                // Lê na ordem BGR
                sum_b += img_colorida->data[pos];
                sum_g += img_colorida->data[pos + 1];
                sum_r += img_colorida->data[pos + 2];
                pixel_count++;
            }
        }
    }

    if (pixel_count == 0) return 0;

    // Calcula a cor média e normaliza para o intervalo [0, 1]
    float avg_r_norm = ((float)sum_r / pixel_count) / 255.0f;
    float avg_g_norm = ((float)sum_g / pixel_count) / 255.0f;
    float avg_b_norm = ((float)sum_b / pixel_count) / 255.0f;

    // Converte a cor média RGB normalizada para HSV
    float h, s, v;
    float max, min, delta;

    max = (avg_r_norm > avg_g_norm ? (avg_r_norm > avg_b_norm ? avg_r_norm : avg_b_norm) : (avg_g_norm > avg_b_norm ? avg_g_norm : avg_b_norm));
    min = (avg_r_norm < avg_g_norm ? (avg_r_norm < avg_b_norm ? avg_r_norm : avg_b_norm) : (avg_g_norm < avg_b_norm ? avg_g_norm : avg_b_norm));
    delta = max - min;

    v = max; // Value (Brilho)

    if (max == 0.0f) {
        s = 0.0f; // Saturation (Saturação)
    }
    else {
        s = delta / max;
    }

    if (s == 0.0f) {
        h = 0.0f; // Hue (Matiz)
    }
    else {
        if (max == avg_r_norm) {
            h = 60.0f * fmodf(((avg_g_norm - avg_b_norm) / delta), 6.0f);
        }
        else if (max == avg_g_norm) {
            h = 60.0f * (((avg_b_norm - avg_r_norm) / delta) + 2.0f);
        }
        else { // max == avg_b_norm
            h = 60.0f * (((avg_r_norm - avg_g_norm) / delta) + 4.0f);
        }
        if (h < 0.0f) {
            h += 360.0f;
        }
    }

    // --- CONDIÇÃO DE VERIFICAÇÃO EM HSV ---
    // O vermelho está nos extremos do círculo cromático (perto de 0 e perto de 360 graus)
    int is_red = (h >= 0 && h <= 20) || (h >= 340 && h <= 360);
    // A saturação tem que ser alta para não ser branco/cinzento
    int is_saturated = (s > 0.5f);
    // O brilho não pode ser muito baixo (preto)
    int is_bright = (v > 0.3f);

    if (is_red && is_saturated && is_bright) {
        return 1; // É vermelho
    }

    return 0; // Não é vermelho
}

// =======================
// Operações Morfológicas Binárias
// =======================

// Erosão binária
int vc_binary_erode(IVC* src, IVC* dst, int kernel_size) {
    if (!src || !dst || !src->data || !dst->data) return 0;
    if (src->width != dst->width || src->height != dst->height || src->channels != 1 || dst->channels != 1) return 0;
    if (kernel_size < 1 || kernel_size % 2 == 0) return 0;

    int width = src->width;
    int height = src->height;
    int bytesperline = src->bytesperline;
    unsigned char* src_data = src->data;
    unsigned char* dst_data = dst->data;
    int half_kernel = kernel_size / 2;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char min_val_neighborhood = 255;
            for (int ky = -half_kernel; ky <= half_kernel; ky++) {
                for (int kx = -half_kernel; kx <= half_kernel; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        if (src_data[ny * bytesperline + nx] < min_val_neighborhood) {
                            min_val_neighborhood = src_data[ny * bytesperline + nx];
                        }
                    }
                }
            }
            dst_data[y * bytesperline + x] = min_val_neighborhood;
        }
    }
    return 1;
}

// Dilatação binária
int vc_binary_dilate(IVC* src, IVC* dst, int kernel_size) {
    if (!src || !dst || !src->data || !dst->data) return 0;
    if (src->width != dst->width || src->height != dst->height || src->channels != 1 || dst->channels != 1) return 0;
    if (kernel_size < 1 || kernel_size % 2 == 0) return 0;

    int width = src->width;
    int height = src->height;
    int bytesperline = src->bytesperline;
    unsigned char* src_data = src->data;
    unsigned char* dst_data = dst->data;
    int half_kernel = kernel_size / 2;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char max_val_neighborhood = 0;
            for (int ky = -half_kernel; ky <= half_kernel; ky++) {
                for (int kx = -half_kernel; kx <= half_kernel; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        if (src_data[ny * bytesperline + nx] > max_val_neighborhood) {
                            max_val_neighborhood = src_data[ny * bytesperline + nx];
                        }
                    }
                }
            }
            dst_data[y * bytesperline + x] = max_val_neighborhood;
        }
    }
    return 1;
}

// Abertura binária (erosão seguida de dilatação)
int vc_binary_open(IVC* src, IVC* dst, int kernel_size, IVC* temp) {
    if (!src || !dst || !temp || !src->data || !dst->data || !temp->data) return 0;
    if (src->width != temp->width || src->height != temp->height || src->channels != temp->channels) return 0;
    if (temp->width != dst->width || temp->height != dst->height || temp->channels != dst->channels) return 0;

    if (!vc_binary_erode(src, temp, kernel_size)) {
        fprintf(stderr, "Erro: vc_binary_open falhou na erosao.\n");
        return 0;
    }
    if (!vc_binary_dilate(temp, dst, kernel_size)) {
        fprintf(stderr, "Erro: vc_binary_open falhou na dilatacao.\n");
        return 0;
    }
    return 1;
}

// Fecho binário (dilatação seguida de erosão)
int vc_binary_close(IVC* src, IVC* dst, int kernel_size, IVC* temp) {
    if (!src || !dst || !temp || !src->data || !dst->data || !temp->data) return 0;
    if (src->width != temp->width || src->height != temp->height || src->channels != temp->channels) return 0;
    if (temp->width != dst->width || temp->height != dst->height || temp->channels != dst->channels) return 0;

    if (!vc_binary_dilate(src, temp, kernel_size)) {
        fprintf(stderr, "Erro: vc_binary_close falhou na dilatacao.\n");
        return 0;
    }
    if (!vc_binary_erode(temp, dst, kernel_size)) {
        fprintf(stderr, "Erro: vc_binary_close falhou na erosao.\n");
        return 0;
    }
    return 1;
}
