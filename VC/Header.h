#ifndef HEADER_H
#define HEADER_H

 /* ============================================================================
  * Estruturas de Dados
  * ==========================================================================*/

  /**
   * Estrutura: IVC
   * Descri��o: Representa uma imagem gen�rica, podendo ser a cores ou tons de cinzento.
   * Campos:
   *   - data: ponteiro para os dados da imagem
   *   - width: largura da imagem (p�xeis)
   *   - height: altura da imagem (p�xeis)
   *   - channels: n�mero de canais (1=gray, 3=BGR)
   *   - levels: n�mero de n�veis de intensidade
   *   - bytesperline: n�mero de bytes por linha
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
 * Descri��o: Representa um blob (objeto identificado na imagem) e as suas propriedades geom�tricas.
 * Campos:
 *   - x, y: coordenadas do canto superior esquerdo
 *   - width, height: dimens�es do blob
 *   - xc, yc: coordenadas do centro de gravidade
 */
typedef struct {
    int x, y;
    int width, height;
    int xc, yc;
} OVC;

/* ============================================================================
 * Prot�tipos das Fun��es de Processamento de Imagem
 * ==========================================================================*/

 /**
  * Aloca mem�ria para uma nova imagem IVC.
  */
IVC* vc_image_new(int width, int height, int channels, int levels);

/**
 * Liberta a mem�ria alocada para uma imagem IVC.
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
 * Determina se o blob deve ser descartado com base na cor m�dia.
 */
int blob_e_cor_a_descartar(IVC* img_colorida, OVC* blob_info, const char* ficheiro);

/**
 * Desenha uma linha horizontal numa imagem a cores.
 */
int vc_draw_horizontal_line(IVC* img, int y, int r, int g, int b);

/* ============================================================================
 * Prot�tipos das Fun��es de Morfologia Bin�ria
 * ==========================================================================*/

 /**
  * Realiza eros�o bin�ria numa imagem bin�ria.
  */
int vc_binary_erode(IVC* src, IVC* dst, int kernel_size);

/**
 * Realiza dilata��o bin�ria numa imagem bin�ria.
 */
int vc_binary_dilate(IVC* src, IVC* dst, int kernel_size);

/**
 * Realiza a opera��o de abertura bin�ria.
 */
int vc_binary_open(IVC* src, IVC* dst, int kernel_size, IVC* temp);

/**
 * Realiza a opera��o de fecho bin�rio.
 */
int vc_binary_close(IVC* src, IVC* dst, int kernel_size, IVC* temp);

#endif // HEADER_H
