#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cmath>
#include <chrono>
#include <iomanip>

extern "C" {
#include "Header.h" // Garanta que o nome do seu header está correto
}

// Limiar para binarização da imagem em tons de cinza
int limiarBinarizacao = 80;

// Função para mostrar o tempo decorrido
void mostrarTempo() {
    static bool iniciou = false;
    static auto tempoAnterior = std::chrono::steady_clock::now();
    if (!iniciou) {
        iniciou = true;
    }
    else {
        auto tempoAtual = std::chrono::steady_clock::now();
        double segundos = std::chrono::duration<double>(tempoAtual - tempoAnterior).count();
        std::cout << "Tempo: " << segundos << " segundos\n";
        std::cout << "Prima Enter para continuar...\n";
        std::cin.get();
    }
}

// Função para identificar o tipo de moeda
std::string identificarMoeda(double area, const std::string& ficheiro) {
    std::string tipo = "X";
    if (ficheiro == "video1.mp4") {
        if (area >= 2000 && area < 2900) tipo = "1c";
        else if (area >= 2900 && area < 3000) tipo = "2c";
        else if (area >= 3000 && area < 4000) tipo = "5c";
        else if (area >= 4000 && area < 6000) tipo = "10c";
        else if (area >= 6000 && area < 9000) tipo = "20c";
        else if (area >= 11500 && area < 13000) tipo = "50c";
        else if (area >= 9000 && area < 16000) tipo = "1euro";
        else if (area >= 16000 && area < 17000) tipo = "2euro";
    }
    else if (ficheiro == "video2.mp4") {
        if (area >= 2000 && area < 2600) tipo = "1c";
        else if (area >= 2600 && area < 2900) tipo = "2c";
        else if (area >= 2900 && area <= 3720) tipo = "5c";
        else if (area > 3720 && area < 4500) tipo = "10c";
        else if (area >= 4500 && area < 7800) tipo = "20c";
        else if (area >= 7800 && area < 7930) tipo = "50c";
        else if (area >= 7930 && area < 12000) tipo = "1euro";
        else if (area >= 12000 && area < 15000) tipo = "2euro";
    }
    return tipo;
}

// Função auxiliar para calcular manualmente as propriedades do blob
void calcularPropriedadesManualmente(const std::vector<cv::Point>& contorno, OVC& blob_info) {
    if (contorno.empty()) return;
    int min_x = contorno[0].x, max_x = contorno[0].x;
    int min_y = contorno[0].y, max_y = contorno[0].y;
    long long sum_x = 0, sum_y = 0;
    for (const auto& ponto : contorno) {
        sum_x += ponto.x; sum_y += ponto.y;
        if (ponto.x < min_x) min_x = ponto.x; if (ponto.x > max_x) max_x = ponto.x;
        if (ponto.y < min_y) min_y = ponto.y; if (ponto.y > max_y) max_y = ponto.y;
    }
    blob_info.x = min_x; blob_info.y = min_y;
    blob_info.width = max_x - min_x + 1; blob_info.height = max_y - min_y + 1;
    blob_info.xc = static_cast<int>(sum_x / contorno.size());
    blob_info.yc = static_cast<int>(sum_y / contorno.size());
}

int main() {
    std::string nomeVideo = "video1.mp4";
    int distMinima = 40;
    cv::VideoCapture video(nomeVideo);

    if (!video.isOpened()) {
        std::cerr << "Não foi possível abrir o vídeo.\n"; return 1;
    }

    int largura = static_cast<int>(video.get(cv::CAP_PROP_FRAME_WIDTH));
    int altura = static_cast<int>(video.get(cv::CAP_PROP_FRAME_HEIGHT));

    cv::namedWindow("Resultado", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("Binária", cv::WINDOW_AUTOSIZE);
    mostrarTempo();

    std::ofstream ficheiroCSV("moedas_stats.csv");
    ficheiroCSV << "Tipo,Area,CentroX,CentroY\n";

    const int linha_contagem_y = altura / 3;
    int proximo_id_objeto = 0;
    std::map<int, cv::Point> objetos_rastreados;
    std::map<int, bool> objetos_contados;
    std::map<std::string, int> contagemPorTipo;
    double valorTotalEuros = 0.0;
    int totalMoedas = 0;
    int tecla = 0;

    contagemPorTipo["1c"] = 0; contagemPorTipo["2c"] = 0; contagemPorTipo["5c"] = 0; contagemPorTipo["10c"] = 0;
    contagemPorTipo["20c"] = 0; contagemPorTipo["50c"] = 0; contagemPorTipo["1euro"] = 0; contagemPorTipo["2euro"] = 0;

    while (tecla != 'q') {
        cv::Mat frame;
        video >> frame;
        if (frame.empty()) break;

        IVC* imgVC = vc_image_new(largura, altura, 3, 255);
        memcpy(imgVC->data, frame.data, largura * altura * 3);

        IVC* imgCinza = vc_image_new(largura, altura, 1, 255);
        vc_rgb_to_gray(imgVC, imgCinza);

        IVC* imgBin = vc_image_new(largura, altura, 1, 255);
        vc_gray_to_binary(imgCinza, imgBin, limiarBinarizacao);
        vc_gray_negative(imgBin);

        cv::Mat binaria(altura, largura, CV_8UC1, imgBin->data);
        std::vector<std::vector<cv::Point>> contornos;

        cv::findContours(binaria, contornos, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        for (const auto& contorno : contornos) {
            double area = cv::contourArea(contorno);
            if (area < 1500) continue;

            OVC blob_info = { 0 };
            calcularPropriedadesManualmente(contorno, blob_info);

            cv::Point centro_atual(blob_info.xc, blob_info.yc);

            int id_associado = -1;
            double menor_dist = distMinima;
            for (auto const& par : objetos_rastreados) {
                int id = par.first;
                cv::Point pos = par.second;
                double dist = cv::norm(centro_atual - pos);
                if (dist < menor_dist) { menor_dist = dist; id_associado = id; }
            }

            if (id_associado != -1) {
                cv::Point pos_anterior = objetos_rastreados[id_associado];
                objetos_rastreados[id_associado] = centro_atual;
                if (pos_anterior.y >= linha_contagem_y && centro_atual.y < linha_contagem_y && !objetos_contados[id_associado]) {
                    totalMoedas++;
                    objetos_contados[id_associado] = true;
                    std::string tipo = identificarMoeda(area, nomeVideo);
                    if (tipo != "X") {
                        contagemPorTipo[tipo]++;
                        if (tipo == "1c") valorTotalEuros += 0.01; else if (tipo == "2c") valorTotalEuros += 0.02;
                        else if (tipo == "5c") valorTotalEuros += 0.05; else if (tipo == "10c") valorTotalEuros += 0.10;
                        else if (tipo == "20c") valorTotalEuros += 0.20; else if (tipo == "50c") valorTotalEuros += 0.50;
                        else if (tipo == "1euro") valorTotalEuros += 1.00; else if (tipo == "2euro") valorTotalEuros += 2.00;
                    }
                    ficheiroCSV << tipo << "," << area << "," << centro_atual.x << "," << centro_atual.y << "\n";
                }
            }
            else {
                if (centro_atual.y > linha_contagem_y) {
                    objetos_rastreados[proximo_id_objeto] = centro_atual;
                    objetos_contados[proximo_id_objeto] = false;
                    proximo_id_objeto++;
                }
            }

            // Desenho com a sua biblioteca na sua estrutura de imagem
            vc_draw_bounding_box(imgVC, &blob_info);
            vc_draw_center_of_gravity(imgVC, &blob_info, 5);
        }

        // Depois de todos os desenhos da sua biblioteca estarem feitos na imgVC,
        // copia-se o resultado para o frame do OpenCV.
        memcpy(frame.data, imgVC->data, largura * altura * 3);

        // Agora, desenhamos os textos e a linha diretamente no frame do OpenCV.
        // Isto garante que eles aparecem por cima de tudo.
        cv::line(frame, cv::Point(0, linha_contagem_y), cv::Point(largura, linha_contagem_y), cv::Scalar(0, 0, 255), 2);

        // Desenha as informações para cada blob novamente, mas desta vez no `frame` final
        for (const auto& contorno : contornos) {
            double area = cv::contourArea(contorno);
            if (area < 1500) continue;
            OVC blob_info = { 0 };
            calcularPropriedadesManualmente(contorno, blob_info);

            std::string tipo = identificarMoeda(area, nomeVideo);
            int y_pos_info = blob_info.y + 15; // Posição do texto
            int x_pos_info = blob_info.x + blob_info.width + 5; // Posição à direita da caixa

            std::string texto_pos = "x:" + std::to_string(blob_info.xc) + " y:" + std::to_string(blob_info.yc);
            std::string texto_area = "Area:" + std::to_string(static_cast<int>(area));
            std::string texto_tipo = "Tipo:" + tipo;

            cv::putText(frame, texto_pos, cv::Point(x_pos_info, y_pos_info), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);
            cv::putText(frame, texto_area, cv::Point(x_pos_info, y_pos_info + 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);
            cv::putText(frame, texto_tipo, cv::Point(x_pos_info, y_pos_info + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);
        }

        // Desenha o painel de controlo principal
        int pos_y_painel = 30;
        for (const auto& par : contagemPorTipo) {
            std::string texto = par.first + ": " + std::to_string(par.second);
            cv::putText(frame, texto, cv::Point(20, pos_y_painel), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 2);
            cv::putText(frame, texto, cv::Point(20, pos_y_painel), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
            pos_y_painel += 25;
        }
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << valorTotalEuros;
        std::string texto_total = "Total: " + stream.str() + " EUR";
        cv::putText(frame, texto_total, cv::Point(20, pos_y_painel + 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, texto_total, cv::Point(20, pos_y_painel + 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 1);

        cv::imshow("Resultado", frame);
        cv::imshow("Binária", binaria);

        vc_image_free(imgVC);
        vc_image_free(imgCinza);
        vc_image_free(imgBin);

        tecla = cv::waitKey(100) & 0xFF;
    }

    ficheiroCSV.close();

    std::cout << "\n=== Contagem Final ===\n";
    std::cout << "Moedas contadas: " << totalMoedas << "\n";
    for (const auto& par : contagemPorTipo)
        std::cout << "Moedas de " << par.first << ": " << par.second << "\n";
    std::cout << "Valor Total: " << std::fixed << std::setprecision(2) << valorTotalEuros << " EUR\n";

    mostrarTempo();
    video.release();
    cv::destroyAllWindows();
    return 0;
}