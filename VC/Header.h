#ifndef HEADER_H
#define HEADER_H

 /* ============================================================================
  * Estruturas de Dados
  * ==========================================================================*/

  /**
   * Estrutura: IVC
   * Descrição: Representa uma imagem genérica, podendo ser a cores ou tons de cinzento.
   * Campos:
   *   - data: ponteiro para os dados da imagem
   *   - width: largura da imagem (píxeis)
   *   - height: altura da imagem (píxeis)
   *   - channels: número de canais (1=gray, 3=BGR)
   *   - levels: número de níveis de intensidade
   *   - bytesperline: número de bytes por linha
   */
typedef struct {
    unsigned char* data;
    int width, height;
    int channels;
    int levels;
    int bytesperline;
} IVC;

/**
 * Estrutura: OVC
 * Descrição: Representa um blob (objeto identificado na imagem) e as suas propriedades geométricas.
 * Campos:
 *   - x, y: coordenadas do canto superior esquerdo
 *   - width, height: dimensões do blob
 *   - xc, yc: coordenadas do centro de gravidade
 */
typedef struct {
    int x, y;
    int width, height;
    int xc, yc;
} OVC;

/* ============================================================================
 * Protótipos das Funções de Processamento de Imagem
 * ==========================================================================*/

 /**
  * Aloca memória para uma nova imagem IVC.
  */
IVC* vc_image_new(int width, int height, int channels, int levels);

/**
 * Liberta a memória alocada para uma imagem IVC.
 */
IVC* vc_image_free(IVC* image);

/**
 * Converte uma imagem RGB para tons de cinzento.
 */
int vc_rgb_to_gray(IVC* src, IVC* dst);

/**
 * Binariza uma imagem em tons de cinzento com base num limiar.
 */
int vc_gray_to_binary(IVC* src, IVC* dst, int threshold);

/**
 * Gera o negativo de uma imagem em tons de cinzento.
 */
int vc_gray_negative(IVC* srcdst);

/**
 * Desenha a bounding box de um blob numa imagem a cores.
 */
int vc_draw_bounding_box(IVC* img, OVC* blob);

/**
 * Desenha o centro de gravidade de um blob numa imagem a cores.
 */
int vc_draw_center_of_gravity(IVC* img, OVC* blob, int size);

/**
 * Determina se o blob deve ser descartado com base na cor média.
 */
int blob_e_cor_a_descartar(IVC* img_colorida, OVC* blob_info, const char* ficheiro);

/**
 * Desenha uma linha horizontal numa imagem a cores.
 */
int vc_draw_horizontal_line(IVC* img, int y, int r, int g, int b);

/* ============================================================================
 * Protótipos das Funções de Morfologia Binária
 * ==========================================================================*/

 /**
  * Realiza erosão binária numa imagem binária.
  */
int vc_binary_erode(IVC* src, IVC* dst, int kernel_size);

/**
 * Realiza dilatação binária numa imagem binária.
 */
int vc_binary_dilate(IVC* src, IVC* dst, int kernel_size);

/**
 * Realiza a operação de abertura binária.
 */
int vc_binary_open(IVC* src, IVC* dst, int kernel_size, IVC* temp);

/**
 * Realiza a operação de fecho binário.
 */
int vc_binary_close(IVC* src, IVC* dst, int kernel_size, IVC* temp);

#endif // HEADER_H
