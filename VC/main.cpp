#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cmath>
#include <chrono>

extern "C" {
#include "header.h"
}

// Limiar para binariza��o da imagem em tons de cinza
int limiarBinarizacao = 80;

// Fun��o para mostrar o tempo decorrido
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

// Fun��o para identificar o tipo de moeda
std::string identificarMoeda(double area, const std::string& ficheiro) {
    if (ficheiro == "video1.mp4") {
        if (area >= 2000 && area < 2900) return "1c";
        else if (area >= 2900 && area < 3000) return "2c";
        else if (area >= 3000 && area < 4000) return "5c";
        else if (area >= 4000 && area < 6000) return "10c";
        else if (area >= 6000 && area < 9000) return "20c";
        else if (area >= 11500 && area < 13000) return "50c";
        else if (area >= 9000 && area < 16000) return "1euro";
        else if (area >= 16000 && area < 17000) return "2euro";
    }
    else if (ficheiro == "video2.mp4") {
        if (area >= 2000 && area < 2600) return "1c";
        else if (area >= 2600 && area < 2900) return "2c";
        else if (area >= 2900 && area <= 3720) return "5c";
        else if (area > 3720 && area < 4500) return "10c";
        else if (area >= 4500 && area < 7800) return "20c";
        else if (area >= 7800 && area < 7930) return "50c";
        else if (area >= 7930 && area < 12000) return "1euro";
        else if (area >= 12000 && area < 15000) return "2euro";
    }
    return "X";
}

int main() {
    std::string nomeVideo = "video1.mp4";
    int distMinima = 40;
    cv::VideoCapture video(nomeVideo);

    if (!video.isOpened()) {
        std::cerr << "N�o foi poss�vel abrir o v�deo.\n";
        return 1;
    }

    int largura = static_cast<int>(video.get(cv::CAP_PROP_FRAME_WIDTH));
    int altura = static_cast<int>(video.get(cv::CAP_PROP_FRAME_HEIGHT));

    cv::namedWindow("Resultado", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("Bin�ria", cv::WINDOW_AUTOSIZE);

    mostrarTempo();

    std::ofstream ficheiroCSV("moedas_stats.csv");
    ficheiroCSV << "Tipo,Area,CentroX,CentroY\n";

    const int linha_contagem_y = altura * 2 / 3;
    int proximo_id_objeto = 0;
    std::map<int, cv::Point> objetos_rastreados;
    std::map<int, bool> objetos_contados;

    std::map<std::string, int> contagemPorTipo;
    int totalMoedas = 0;
    int tecla = 0;

    while (tecla != 'q') {
        cv::Mat frame;
        video >> frame;
        if (frame.empty()) break;

        IVC* imgVC = vc_image_new(largura, altura, 3, 255);
        memcpy(imgVC->data, frame.data, largura * altura * 3);

        IVC* imgCinza = vc_rgb_to_gray(imgVC);
        IVC* imgBin = vc_gray_to_binary(imgCinza, limiarBinarizacao);
        IVC* imgLabels = vc_image_new(largura, altura, sizeof(int), 0);

        int n_labels = 0;
        OVC* blobs = vc_binary_blob_labelling(imgBin, imgLabels, &n_labels);

        for (int i = 0; i < n_labels; i++) {
            OVC* blob = &blobs[i];
            if (blob->area < 1500) continue;

            cv::Point centro_atual(blob->xc, blob->yc);
            int id_associado = -1;
            double menor_dist = distMinima;

            // +++ CORRE��O DE SINTAXE APLICADA AQUI +++
            // Usa a sintaxe antiga e compat�vel para iterar sobre o map
            for (auto const& par : objetos_rastreados) {
                int id = par.first;
                cv::Point pos = par.second;
                double dist = cv::norm(centro_atual - pos);

                if (dist < menor_dist) {
                    menor_dist = dist;
                    id_associado = id;
                }
            }

            if (id_associado != -1) { // Encontrou um objeto correspondente
                objetos_rastreados[id_associado] = centro_atual;

                // Verifica se cruzou a linha E AINDA N�O FOI CONTADO
                if (centro_atual.y > linha_contagem_y && objetos_contados.count(id_associado) > 0 && !objetos_contados[id_associado]) {
                    totalMoedas++;
                    objetos_contados[id_associado] = true; // Marca como contado
                    std::string tipo = identificarMoeda(blob->area, nomeVideo);
                    if (tipo != "X") contagemPorTipo[tipo]++;
                    std::cout << "Moeda Contada! ID: " << id_associado << ", Tipo: " << tipo << ", Total: " << totalMoedas << std::endl;
                    ficheiroCSV << tipo << "," << blob->area << "," << blob->xc << "," << blob->yc << "\n";
                }
            }
            else { // N�o encontrou, � um novo objeto
                objetos_rastreados[proximo_id_objeto] = centro_atual;
                objetos_contados[proximo_id_objeto] = false; // Novo objeto, ainda n�o foi contado
                proximo_id_objeto++;
            }

            // Desenha sempre, para visualiza��o
            vc_draw_bounding_box(imgVC, blob);
            vc_draw_center_of_gravity(imgVC, blob, 5);
        }

        if (blobs) free(blobs);

        // Prepara as imagens para exibi��o
        memcpy(frame.data, imgVC->data, largura * altura * 3);
        cv::line(frame, cv::Point(0, linha_contagem_y), cv::Point(largura, linha_contagem_y), cv::Scalar(0, 0, 255), 2);
        cv::Mat binaria(altura, largura, CV_8UC1, imgBin->data);
        cv::putText(frame, "Total: " + std::to_string(totalMoedas), cv::Point(20, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 255), 2);

        // Exibe as imagens
        cv::imshow("Resultado", frame);
        cv::imshow("Bin�ria", binaria);

        // Liberta a mem�ria
        vc_image_free(imgVC);
        vc_image_free(imgCinza);
        vc_image_free(imgBin);
        vc_image_free(imgLabels);

        tecla = cv::waitKey(100) & 0xFF;
    }

    ficheiroCSV.close();

    std::cout << "\n=== Contagem Final ===\n";
    std::cout << "Moedas encontradas: " << totalMoedas << "\n";
    for (const auto& par : contagemPorTipo)
        std::cout << "Moedas de " << par.first << ": " << par.second << "\n";

    mostrarTempo();
    video.release();
    cv::destroyAllWindows();
    return 0;
}