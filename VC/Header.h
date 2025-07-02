#pragma once

#ifndef VC_H
#define VC_H

// Estrutura de imagem
typedef struct {
    unsigned char* data;
    int width, height;
    int channels;
    int levels;
    int bytesperline;
} IVC;

// Estrutura para armazenar informações de um blob (Object)
typedef struct {
    int x, y;           // Posição do canto superior esquerdo da bounding box
    int width, height;  // Dimensões da bounding box
    int area;           // Área (número de píxeis)
    int xc, yc;         // Coordenadas do centro de massa (centroide)
    int label;          // <<< ADICIONE ESTA LINHA: Rótulo numérico do blob
} OVC;

// Alocação e libertação de memória
IVC* vc_image_new(int width, int height, int channels, int levels);
int vc_image_free(IVC* image);

// Conversão RGB -> Grayscale
IVC* vc_rgb_to_gray(IVC* src);

// Binarização simples
IVC* vc_gray_to_binary(IVC* src, int threshold);

// Análise de Blobs
OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels);

// Desenho
int vc_draw_bounding_box(IVC* img, OVC* blob);
int vc_draw_center_of_gravity(IVC* img, OVC* blob, int size);

#endif