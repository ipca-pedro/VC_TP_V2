#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"

// Aloca memória para uma nova imagem IVC
IVC* vc_image_new(int width, int height, int channels, int levels) {
    IVC* image = (IVC*)malloc(sizeof(IVC));
    if (image == NULL) return NULL;
    image->width = width;
    image->height = height;
    image->channels = channels;
    image->levels = levels;
    image->bytesperline = width * channels;
    image->data = (unsigned char*)malloc(width * height * channels * sizeof(unsigned char));
    if (image->data == NULL) {
        free(image);
        return NULL;
    }
    return image;
}

// Liberta a memória de uma imagem IVC
int vc_image_free(IVC* image) {
    if (image != NULL) {
        if (image->data != NULL) free(image->data);
        free(image);
        return 1;
    }
    return 0;
}

// Converte uma imagem RGB para grayscale (usando a sua lógica original que funcionava bem)
IVC* vc_rgb_to_gray(IVC* src) {
    if (src == NULL || src->channels != 3) return NULL;
    IVC* gray = vc_image_new(src->width, src->height, 1, src->levels);
    if (gray == NULL) return NULL;
    for (int y = 0; y < src->height; y++) {
        for (int x = 0; x < src->width; x++) {
            int offset = (y * src->width + x) * 3;
            unsigned char r = src->data[offset];
            unsigned char g = src->data[offset + 1];
            unsigned char b = src->data[offset + 2];
            gray->data[y * gray->width + x] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
        }
    }
    return gray;
}

// Binariza uma imagem grayscale usando um limiar fixo
IVC* vc_gray_to_binary(IVC* src, int threshold) {
    if (src == NULL || src->channels != 1) return NULL; // Verifica se é grayscale

    IVC* bin = vc_image_new(src->width, src->height, 1, 255); // Cria imagem binária
    if (bin == NULL) return NULL;

    int total_pixels = src->width * src->height;
    for (int i = 0; i < total_pixels; i++) {
        bin->data[i] = (src->data[i] < threshold) ? 255 : 0;
    }

    return bin;
}

// Função para encontrar blobs, calcular as suas propriedades e etiquetá-los.
// vc.c

// Função para encontrar blobs, calcular as suas propriedades e etiquetá-los.
// Função para encontrar blobs, calcular as suas propriedades e etiquetá-los.
OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels) {
    unsigned char* datasrc = (unsigned char*)src->data;
    int* datadst = (int*)dst->data;
    int width = src->width;
    int height = src->height;
    int x, y, i;
    int label = 1;
    int max_labels = width * height / 4;
    OVC* blobs;

    if (!src || !dst || !src->data || !dst->data || !nlabels) return NULL;
    if (dst->width != width || dst->height != height || dst->channels != sizeof(int)) return NULL;

    blobs = (OVC*)calloc(max_labels, sizeof(OVC));
    if (!blobs) return NULL;

    memset(datadst, 0, width * height * sizeof(int));

    // Primeira passagem: etiquetagem de blobs
    for (y = 1; y < height; y++) {
        for (x = 1; x < width; x++) {
            if (datasrc[y * width + x] != 0) {
                int left_label = datadst[y * width + (x - 1)];
                int top_label = datadst[(y - 1) * width + x];

                if (left_label == 0 && top_label == 0) {
                    if (label < max_labels) datadst[y * width + x] = label++;
                }
                else if (left_label != 0 && top_label == 0) {
                    datadst[y * width + x] = left_label;
                }
                else if (left_label == 0 && top_label != 0) {
                    datadst[y * width + x] = top_label;
                }
                else {
                    if (left_label == top_label) {
                        datadst[y * width + x] = top_label;
                    }
                    else {
                        int min_label = (left_label < top_label) ? left_label : top_label;
                        int max_label = (left_label > top_label) ? left_label : top_label;
                        datadst[y * width + x] = min_label;
                        for (i = 0; i < y * width + x; i++) {
                            if (datadst[i] == max_label) datadst[i] = min_label;
                        }
                    }
                }
            }
        }
    }

    *nlabels = label - 1;

    // Inicializa os blobs para a segunda passagem
    for (i = 0; i < *nlabels; i++) {
        blobs[i].label = i + 1; // Atribui o rótulo
        blobs[i].x = width;
        blobs[i].y = height;
        blobs[i].width = 0;
        blobs[i].height = 0;
        blobs[i].area = 0;
        blobs[i].xc = 0;
        blobs[i].yc = 0;
    }

    long int* sum_x = (long int*)calloc(*nlabels + 1, sizeof(long int));
    long int* sum_y = (long int*)calloc(*nlabels + 1, sizeof(long int));

    // Segunda passagem: calcula as propriedades
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int current_label = datadst[y * width + x];
            if (current_label > 0) {
                OVC* blob = &blobs[current_label - 1];
                blob->area++;
                sum_x[current_label - 1] += x;
                sum_y[current_label - 1] += y;

                if (x < blob->x) blob->x = x;
                if (y < blob->y) blob->y = y;
                if (x > (blob->x + blob->width - 1)) blob->width = x - blob->x;
                if (y > (blob->y + blob->height - 1)) blob->height = y - blob->y;
            }
        }
    }

    for (i = 0; i < *nlabels; i++) {
        if (blobs[i].area > 0) {
            blobs[i].xc = sum_x[i] / blobs[i].area;
            blobs[i].yc = sum_y[i] / blobs[i].area;
        }
    }

    free(sum_x);
    free(sum_y);

    return blobs;
}

// Desenha a bounding box de um blob
int vc_draw_bounding_box(IVC* img, OVC* blob) {
    if (!img || !blob || !img->data) return 0;
    if (img->channels != 3) return 0;

    int x, y;
    int r = 0, g = 255, b = 0; // Verde

    for (y = blob->y; y < blob->y + blob->height; y++) {
        if (y < 0 || y >= img->height) continue;
        if (blob->x >= 0 && blob->x < img->width) {
            int pos1 = y * img->bytesperline + blob->x * img->channels;
            img->data[pos1] = b; img->data[pos1 + 1] = g; img->data[pos1 + 2] = r;
        }
        if ((blob->x + blob->width - 1) >= 0 && (blob->x + blob->width - 1) < img->width) {
            int pos2 = y * img->bytesperline + (blob->x + blob->width - 1) * img->channels;
            img->data[pos2] = b; img->data[pos2 + 1] = g; img->data[pos2 + 2] = r;
        }
    }

    for (x = blob->x; x < blob->x + blob->width; x++) {
        if (x < 0 || x >= img->width) continue;
        if (blob->y >= 0 && blob->y < img->height) {
            int pos1 = blob->y * img->bytesperline + x * img->channels;
            img->data[pos1] = b; img->data[pos1 + 1] = g; img->data[pos1 + 2] = r;
        }
        if ((blob->y + blob->height - 1) >= 0 && (blob->y + blob->height - 1) < img->height) {
            int pos2 = (blob->y + blob->height - 1) * img->bytesperline + x * img->channels;
            img->data[pos2] = b; img->data[pos2 + 1] = g; img->data[pos2 + 2] = r;
        }
    }

    return 1;
}

// Desenha o centro de gravidade de um blob
int vc_draw_center_of_gravity(IVC* img, OVC* blob, int size) {
    if (!img || !blob || !img->data) return 0;
    if (img->channels != 3) return 0;

    int x, y;
    int r = 255, g = 0, b = 0; // Vermelho

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