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

// Estrutura para armazenar informa��es de um blob (Object)
typedef struct {
    int x, y;           // Posi��o do canto superior esquerdo da bounding box
    int width, height;  // Dimens�es da bounding box
    int area;           // �rea (n�mero de p�xeis)
    int xc, yc;         // Coordenadas do centro de massa (centroide)
    int label;          // <<< ADICIONE ESTA LINHA: R�tulo num�rico do blob
} OVC;

// Aloca��o e liberta��o de mem�ria
IVC* vc_image_new(int width, int height, int channels, int levels);
int vc_image_free(IVC* image);

// Convers�o RGB -> Grayscale
IVC* vc_rgb_to_gray(IVC* src);

// Binariza��o simples
IVC* vc_gray_to_binary(IVC* src, int threshold);

// An�lise de Blobs
OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels);

// Desenho
int vc_draw_bounding_box(IVC* img, OVC* blob);
int vc_draw_center_of_gravity(IVC* img, OVC* blob, int size);

#endif