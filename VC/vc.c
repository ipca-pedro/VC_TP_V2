#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Header.h" // Deve corresponder ao nome do seu header

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