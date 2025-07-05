#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cmath>
#include <chrono>
#include <iomanip>

extern "C" {
#include "Header.h" 
}

// Limiar para binarização da imagem em tons de cinza
int limiarBinarizacao = 95;

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

std::string identificarMoeda(double area, const std::string& ficheiro) {
    std::string tipo = "X";

    if (ficheiro == "video1.mp4") {
        // Intervalos baseados na ordem de tamanho físico e nos dados observados
        // NOTA: Devido à grande variação de área causada pela perspetiva,
        // esta classificação pode não ser 100% precisa em todos os frames.
        if (area >= 10000 && area < 12000) { tipo = "1c"; }
        else if (area >= 12700 && area < 14850) { tipo = "2c"; }
        else if (area >= 17600 && area < 19150) { tipo = "5c"; }
        else if (area >= 14900 && area < 16500) { tipo = "10c"; }
        else if (area >= 19250 && area < 20150) { tipo = "20c"; }
        else if (area >= 23400 && area < 24900) { tipo = "50c"; }
        else if (area >= 20151 && area < 40000) { tipo = "1euro"; }
        else if (area >= 25200 && area < 31000) { tipo = "2euro"; }
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
        IVC* imgTemp = vc_image_new(largura, altura, 1, 255);

        // Binarização e negativo
        vc_gray_to_binary(imgCinza, imgBin, limiarBinarizacao);
        vc_gray_negative(imgBin);

        // Operação morfológica para melhorar o contorno
        // Exemplo: abertura para remover ruído e fecho para fechar buracos
        vc_binary_open(imgBin, imgBin, 3, imgTemp);   // kernel 3x3, ajusta se necessário
        vc_binary_close(imgBin, imgBin, 3, imgTemp);  // kernel 3x3, ajusta se necessário

        vc_image_free(imgTemp);


        cv::Mat binaria(altura, largura, CV_8UC1, imgBin->data);
        std::vector<std::vector<cv::Point>> contornos;

        cv::findContours(binaria, contornos, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // +++ Listas temporárias para guardar informações dos blobs válidos +++
        std::vector<OVC> blobs_validos;
        std::vector<double> areas_validas;

        for (const auto& contorno : contornos) {
            double area = cv::contourArea(contorno);
            if (area < 1500) continue;

            OVC blob_info = { 0 };
            calcularPropriedadesManualmente(contorno, blob_info);

            if (blob_e_vermelho(imgVC, &blob_info)) {
                continue;
            }

            blobs_validos.push_back(blob_info);
            areas_validas.push_back(area);

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

            vc_draw_bounding_box(imgVC, &blob_info);
            vc_draw_center_of_gravity(imgVC, &blob_info, 5);
        }

        memcpy(frame.data, imgVC->data, largura * altura * 3);

        // +++ Segundo Loop: Desenha o texto de depuração para cada blob válido +++
        for (size_t i = 0; i < blobs_validos.size(); i++) {
            OVC blob_info = blobs_validos[i];
            double area = areas_validas[i];

            std::string tipo = identificarMoeda(area, nomeVideo);
            int y_pos_info = blob_info.y + 15;
            int x_pos_info = blob_info.x + blob_info.width + 5;

            // Prepara os textos
            std::string texto_pos = "x:" + std::to_string(blob_info.xc) + " y:" + std::to_string(blob_info.yc);
            std::string texto_area = "Area:" + std::to_string(static_cast<int>(area));
            std::string texto_tipo = "Tipo:" + tipo;

            // Desenha os textos no frame do OpenCV
            cv::putText(frame, texto_pos, cv::Point(x_pos_info, y_pos_info), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);
            cv::putText(frame, texto_area, cv::Point(x_pos_info, y_pos_info + 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);
            cv::putText(frame, texto_tipo, cv::Point(x_pos_info, y_pos_info + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);
        }

        // Desenha a linha e o painel de controlo principal
        cv::line(frame, cv::Point(0, linha_contagem_y), cv::Point(largura, linha_contagem_y), cv::Scalar(0, 0, 255), 2);

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
        if (tecla == 'p') {
            // Pausa: espera até pressionar 'p' novamente ou 'q' para sair
            while (true) {
                int tecla_pausa = cv::waitKey(0) & 0xFF;
                if (tecla_pausa == 'p' || tecla_pausa == 'q') {
                    tecla = tecla_pausa;
                    break;
                }
            }
        }
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
