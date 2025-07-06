#ifndef HEADER_H
#define HEADER_H

 /**
  * Estrutura: IVC
  * Descrição: Representa uma imagem genérica.
  */
typedef struct {
    unsigned char* data;    // Ponteiro para os dados da imagem
    int width, height;      // Largura e altura em píxeis
    int channels;           // Número de canais
    int levels;             // Níveis de intensidade
    int bytesperline;       // Número de bytes por linha
} IVC;

/**
 * Estrutura: OVC
 * Descrição: Representa um blob e as propriedades.
 */
typedef struct {
    int x, y;               // Coordenadas do canto superior esquerdo
    int width, height;      // Dimensões do blob
    int xc, yc;             // Coordenadas do centro de massa
} OVC;

 /**
  * Aloca memória para uma nova imagem IVC.
  */
IVC* vc_imagem_nova(int largura, int altura, int canais, int niveis);

/**
 * Liberta a memória alocada para uma imagem IVC.
 */
IVC* vc_imagem_free(IVC* imagem);

 /**
  * Converte uma imagem BGR para tons de cinzento.
  */
int vc_bgr_para_cinzento(IVC* origem, IVC* destino);

/**
 * Binariza uma imagem em tons de cinzento com base num limiar.
 */
int vc_cinzento_para_binario(IVC* origem, IVC* destino, int limiar);

/**
 * Gera o negativo de uma imagem em tons de cinzento
 */
int vc_cinzento_negativo(IVC* imagem);

/**
 * Aplica um filtro de suavização a uma imagem em tons de cinzento.
 */
int vc_cinzento_box_blur(IVC* origem, IVC* destino, int tamanho_kernel);

 /**
  * Realiza erosão binária numa imagem binária.
  */
int vc_binario_erosao(IVC* origem, IVC* destino, int tamanho_kernel);

/**
 * Realiza dilatação binária numa imagem binária.
 */
int vc_binario_dilatacao(IVC* origem, IVC* destino, int tamanho_kernel);

/**
 * Realiza a operação de abertura binária (erosão + dilatação).
 */
int vc_binario_abertura(IVC* origem, IVC* destino, int tamanho_kernel, IVC* temp);

/**
 * Realiza a operação de fecho binário (dilatação + erosão).
 */
int vc_binario_fecho(IVC* origem, IVC* destino, int tamanho_kernel, IVC* temp);

 /**
  * Determina se o blob deve ser descartado com base na cor média
  */
int vc_blob_cor_a_descartar(IVC* imagem_cor, OVC* info_blob, const char* nome_ficheiro);

/**
 * Desenha a caixa delimitadora de um blob numa imagem a cores.
 */
int vc_desenha_caixa_delimitadora(IVC* imagem, OVC* blob);

/**
 * Desenha o centro de massa de um blob numa imagem a cores.
 */
int vc_desenha_centro_massa(IVC* imagem, OVC* blob, int tamanho_cruz);

/**
 * Desenha uma linha horizontal numa imagem a cores.
 */
int vc_desenha_linha_horizontal(IVC* imagem, int y, int vermelho, int verde, int azul);


#endif // HEADER_H