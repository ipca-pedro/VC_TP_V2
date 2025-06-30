#pragma once

#ifndef VC_H
#define VC_H

// Estrutura de imagem
typedef struct {
    unsigned char* data;   // Dados da imagem
    int width, height;     // Dimens�es
    int channels;          // N�mero de canais (1 = Gray, 3 = RGB)
    int levels;            // N�veis de intensidade (ex: 255)
} IVC;

typedef struct {
    int x, y;
} Point;

// Aloca��o e liberta��o de mem�ria
IVC* vc_image_new(int width, int height, int channels, int levels);
int vc_image_free(IVC* image);

// Convers�o RGB -> Grayscale
IVC* vc_rgb_to_gray(IVC* src);

// Binariza��o simples (limiar fixo)
IVC* vc_gray_to_binary(IVC* src, int threshold);

// Desenho de ret�ngulo RGB
void draw_rectangle_rgb(IVC* image, int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b, int thickness);

#endif
