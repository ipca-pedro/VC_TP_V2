#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"

// Aloca mem?ria para uma imagem
IVC* vc_image_new(int width, int height, int channels, int levels) {
    IVC* image = (IVC*)malloc(sizeof(IVC));
    if (image == NULL) return NULL;

    image->width = width;
    image->height = height;
    image->channels = channels;
    image->levels = levels;
    image->data = (unsigned char*)malloc(width * height * channels);

    if (image->data == NULL) {
        free(image);
        return NULL;
    }

    return image;
}

// Liberta a mem?ria de uma imagem
int vc_image_free(IVC* image) {
    if (image != NULL) {
        if (image->data != NULL) free(image->data);
        free(image);
        return 1;
    }
    return 0;
}

// Converte imagem RGB para grayscale
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

            // Convers?o usando lumin?ncia
            gray->data[y * gray->width + x] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
        }
    }

    return gray;
}

// Binariza uma imagem grayscale com limiar fixo
IVC* vc_gray_to_binary(IVC* src, int threshold) {
    if (src == NULL || src->channels != 1) return NULL;

    IVC* bin = vc_image_new(src->width, src->height, 1, 255);
    if (bin == NULL) return NULL;

    for (int y = 0; y < src->height; y++) {
        for (int x = 0; x < src->width; x++) {
            int pos = y * src->width + x;
            bin->data[pos] = (src->data[pos] >= threshold) ? 255 : 0;
        }
    }

    return bin;
}


void draw_rectangle_rgb(IVC* image, int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b, int thickness) {
    if (image == NULL || image->channels != 3) return;

    for (int t = 0; t < thickness; t++) {
        for (int i = x; i < x + width; i++) {
            // Top border
            if (y + t >= 0 && y + t < image->height && i >= 0 && i < image->width) {
                int pos = ((y + t) * image->width + i) * 3;
                image->data[pos] = r;
                image->data[pos + 1] = g;
                image->data[pos + 2] = b;
            }
            // Bottom border
            if (y + height - 1 - t >= 0 && y + height - 1 - t < image->height && i >= 0 && i < image->width) {
                int pos = ((y + height - 1 - t) * image->width + i) * 3;
                image->data[pos] = r;
                image->data[pos + 1] = g;
                image->data[pos + 2] = b;
            }
        }
        for (int j = y; j < y + height; j++) {
            // Left border
            if (j >= 0 && j < image->height && x + t >= 0 && x + t < image->width) {
                int pos = (j * image->width + x + t) * 3;
                image->data[pos] = r;
                image->data[pos + 1] = g;
                image->data[pos + 2] = b;
            }
            // Right border
            if (j >= 0 && j < image->height && x + width - 1 - t >= 0 && x + width - 1 - t < image->width) {
                int pos = (j * image->width + x + width - 1 - t) * 3;
                image->data[pos] = r;
                image->data[pos + 1] = g;
                image->data[pos + 2] = b;
            }
        }
    }
}


