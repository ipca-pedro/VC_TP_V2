# Projeto: Contagem e Classificação Automática de Moedas em Vídeo

## Objetivo
O objetivo deste projeto é desenvolver um programa em C++ capaz de identificar, contar e classificar automaticamente moedas presentes em vídeos (`video1.mp4` e `video2.mp4`), apresentando o valor total detetado, bem como estatísticas detalhadas sobre as moedas observadas.

## Estado Atual do Projeto
- Leitura e processamento de vídeo: Implementado.
- Segmentação e binarização de imagem: Implementado.
- Deteção de contornos e cálculo de propriedades das moedas: Implementado.
- Rastreamento e contagem de moedas: Implementado.
- Classificação do tipo de moeda: Implementado.
- Geração de ficheiro CSV com estatísticas: Implementado.
- Visualização dos resultados e estatísticas em tempo real: Implementado.
- Respeito ao limite de funções OpenCV: Em conformidade (após substituir `cv::norm` por cálculo manual).

## Estratégia de Implementação

1. **Leitura do vídeo**
   - Utilizamos a classe `cv::VideoCapture` para abrir e ler os frames do vídeo.

2. **Pré-processamento**
   - Conversão do frame para escala de cinzentos usando funções próprias (`vc_rgb_to_gray`).
   - Binarização da imagem (`vc_gray_to_binary`) e inversão (`vc_gray_negative`) para facilitar a segmentação das moedas.

3. **Segmentação e análise de blobs**
   - Utilização de `cv::findContours` para detetar os contornos das moedas na imagem binária.
   - Cálculo da área de cada contorno com `cv::contourArea`.
   - Cálculo manual das propriedades de cada blob (bounding box, centroide) para evitar dependência de funções extra do OpenCV.

4. **Rastreamento e contagem**
   - Para cada moeda detetada, é calculada a distância ao centroide de moedas rastreadas anteriormente (usando cálculo manual da distância euclidiana).
   - Se a moeda atravessar uma linha de contagem (definida a 1/3 da altura da imagem), é contabilizada e classificada.

5. **Classificação das moedas**
   - A classificação é feita com base na área do contorno e no vídeo em análise, usando a função `identificarMoeda`.

6. **Visualização e estatísticas**
   - Desenho da bounding box e centroide de cada moeda na imagem.
   - Exibição do tipo de moeda, contagem por tipo e valor total na janela de resultados.
   - Escrita dos dados de cada moeda num ficheiro CSV para análise posterior.

## Funções do OpenCV Utilizadas

### Permitidas pelo exemplo do professor:
- `cv::VideoCapture`
- `cv::Mat`
- `cv::namedWindow`
- `cv::imshow`
- `cv::waitKey`
- `cv::destroyAllWindows`
- `cv::putText`
- `cv::Point`
- `cv::Scalar`

### Adicionais (dentro do limite de 3):
- `cv::findContours` — para detetar contornos das moedas.
- `cv::contourArea` — para calcular a área de cada moeda.
- `cv::line` — para desenhar a linha de contagem na imagem.