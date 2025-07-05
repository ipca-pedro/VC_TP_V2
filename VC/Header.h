#ifndef HEADER_H
#define HEADER_H

// Estruturas
typedef struct {
    unsigned char* data;
    int width, height;
    int channels;
    int levels;
    int bytesperline;
} IVC;

typedef struct {
    int x, y;
    int width, height;
    int xc, yc;
} OVC;

// Prototipos das funções
IVC* vc_image_new(int width, int height, int channels, int levels);
IVC* vc_image_free(IVC* image);
int vc_rgb_to_gray(IVC* src, IVC* dst);
int vc_gray_to_binary(IVC* src, IVC* dst, int threshold);
int vc_gray_negative(IVC* srcdst);
int vc_draw_bounding_box(IVC* img, OVC* blob);
int vc_draw_center_of_gravity(IVC* img, OVC* blob, int size);
int blob_e_vermelho(IVC* img_colorida, OVC* blob_info);

// Morfologia
int vc_binary_erode(IVC* src, IVC* dst, int kernel_size);
int vc_binary_dilate(IVC* src, IVC* dst, int kernel_size);
int vc_binary_open(IVC* src, IVC* dst, int kernel_size, IVC* temp);
int vc_binary_close(IVC* src, IVC* dst, int kernel_size, IVC* temp);

#endif // HEADER_H
