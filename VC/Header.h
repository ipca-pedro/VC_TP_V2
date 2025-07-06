#ifndef HEADER_H
#define HEADER_H

/* ============================================================================
 * Estruturas de Dados
 * ==========================================================================*/

 /**
  * Estrutura: IVC (Imagem Visão Computacional)
  * Descrição: Representa uma imagem genérica.
  * Nota: Os campos internos mantêm-se em inglês para facilitar a interação
  *       com as estruturas de dados do OpenCV (cv::Mat).
  */
typedef struct {
    unsigned char* data;    // Ponteiro para os dados da imagem
    int width, height;      // Largura e altura em píxeis
    int channels;           // Número de canais (1=gray, 3=BGR)
    int levels;             // Níveis de intensidade (ex: 256)
    int bytesperline;       // Número de bytes por linha
} IVC;

/**
 * Estrutura: OVC (Objeto Visão Computacional)
 * Descrição: Representa um blob (objeto identificado) e as suas propriedades.
 */
typedef struct {
    int x, y;               // Coordenadas do canto superior esquerdo
    int width, height;      // Dimensões do blob (bounding box)
    int xc, yc;             // Coordenadas do centro de massa (centroide)
} OVC;


/* ============================================================================
 * Protótipos das Funções de Gestão de Imagem
 * ==========================================================================*/

 /**
  * Aloca memória para uma nova imagem IVC.
  */
IVC* vc_imagem_nova(int largura, int altura, int canais, int niveis);

/**
 * Liberta a memória alocada para uma imagem IVC.
 */
IVC* vc_imagem_free(IVC* imagem);


/* ============================================================================
 * Protótipos das Funções de Conversão e Filtragem
 * ==========================================================================*/

 /**
  * Converte uma imagem BGR (cores) para tons de cinzento.
  */
int vc_bgr_para_cinzento(IVC* origem, IVC* destino);

/**
 * Binariza uma imagem em tons de cinzento com base num limiar.
 */
int vc_cinzento_para_binario(IVC* origem, IVC* destino, int limiar);

/**
 * Gera o negativo de uma imagem em tons de cinzento (in-place).
 */
int vc_cinzento_negativo(IVC* imagem);

/**
 * Aplica um filtro de suavização (box blur) a uma imagem em tons de cinzento.
 */
int vc_cinzento_box_blur(IVC* origem, IVC* destino, int tamanho_kernel);


/* ============================================================================
 * Protótipos das Funções de Morfologia Binária
 * ==========================================================================*/

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


/* ============================================================================
 * Protótipos das Funções de Análise e Desenho
 * ==========================================================================*/

 /**
  * Determina se o blob deve ser descartado com base na cor média (conversão HSV manual).
  */
int vc_blob_cor_a_descartar(IVC* imagem_cor, OVC* info_blob, const char* nome_ficheiro);

/**
 * Desenha a caixa delimitadora (bounding box) de um blob numa imagem a cores.
 */
int vc_desenha_caixa_delimitadora(IVC* imagem, OVC* blob);

/**
 * Desenha o centro de massa (centroide) de um blob numa imagem a cores.
 */
int vc_desenha_centro_massa(IVC* imagem, OVC* blob, int tamanho_cruz);

/**
 * Desenha uma linha horizontal numa imagem a cores.
 */
int vc_desenha_linha_horizontal(IVC* imagem, int y, int vermelho, int verde, int azul);


#endif // HEADER_H