#ifndef HEADER_H
#define HEADER_H

/* ============================================================================
 * Estruturas de Dados
 * ==========================================================================*/

 /**
  * Estrutura: IVC (Imagem Vis�o Computacional)
  * Descri��o: Representa uma imagem gen�rica.
  * Nota: Os campos internos mant�m-se em ingl�s para facilitar a intera��o
  *       com as estruturas de dados do OpenCV (cv::Mat).
  */
typedef struct {
    unsigned char* data;    // Ponteiro para os dados da imagem
    int width, height;      // Largura e altura em p�xeis
    int channels;           // N�mero de canais (1=gray, 3=BGR)
    int levels;             // N�veis de intensidade (ex: 256)
    int bytesperline;       // N�mero de bytes por linha
} IVC;

/**
 * Estrutura: OVC (Objeto Vis�o Computacional)
 * Descri��o: Representa um blob (objeto identificado) e as suas propriedades.
 */
typedef struct {
    int x, y;               // Coordenadas do canto superior esquerdo
    int width, height;      // Dimens�es do blob (bounding box)
    int xc, yc;             // Coordenadas do centro de massa (centroide)
} OVC;


/* ============================================================================
 * Prot�tipos das Fun��es de Gest�o de Imagem
 * ==========================================================================*/

 /**
  * Aloca mem�ria para uma nova imagem IVC.
  */
IVC* vc_imagem_nova(int largura, int altura, int canais, int niveis);

/**
 * Liberta a mem�ria alocada para uma imagem IVC.
 */
IVC* vc_imagem_free(IVC* imagem);


/* ============================================================================
 * Prot�tipos das Fun��es de Convers�o e Filtragem
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
 * Aplica um filtro de suaviza��o (box blur) a uma imagem em tons de cinzento.
 */
int vc_cinzento_box_blur(IVC* origem, IVC* destino, int tamanho_kernel);


/* ============================================================================
 * Prot�tipos das Fun��es de Morfologia Bin�ria
 * ==========================================================================*/

 /**
  * Realiza eros�o bin�ria numa imagem bin�ria.
  */
int vc_binario_erosao(IVC* origem, IVC* destino, int tamanho_kernel);

/**
 * Realiza dilata��o bin�ria numa imagem bin�ria.
 */
int vc_binario_dilatacao(IVC* origem, IVC* destino, int tamanho_kernel);

/**
 * Realiza a opera��o de abertura bin�ria (eros�o + dilata��o).
 */
int vc_binario_abertura(IVC* origem, IVC* destino, int tamanho_kernel, IVC* temp);

/**
 * Realiza a opera��o de fecho bin�rio (dilata��o + eros�o).
 */
int vc_binario_fecho(IVC* origem, IVC* destino, int tamanho_kernel, IVC* temp);


/* ============================================================================
 * Prot�tipos das Fun��es de An�lise e Desenho
 * ==========================================================================*/

 /**
  * Determina se o blob deve ser descartado com base na cor m�dia (convers�o HSV manual).
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