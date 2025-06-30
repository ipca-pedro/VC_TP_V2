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

int limiarBinarizacao = 80;

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

std::string identificarMoeda(double area, const std::string& ficheiro, std::map<std::string, int>& contador) {
    if (ficheiro == "video1.mp4") {
        if (area >= 2000 && area < 2900) { contador["1c"]++; return "1c"; }
        else if (area >= 2900 && area < 3000) { contador["2c"]++; return "2c"; }
        else if (area >= 3000 && area < 4000) { contador["5c"]++; return "5c"; }
        else if (area >= 4000 && area < 6000) { contador["10c"]++; return "10c"; }
        else if (area >= 6000 && area < 9000) { contador["20c"]++; return "20c"; }
        else if (area >= 11500 && area < 13000) { contador["50c"]++; return "50c"; }
        else if (area >= 9000 && area < 16000) { contador["1euro"]++; return "1€"; }
        else if (area >= 16000 && area < 17000) { contador["2euro"]++; return "2€"; }
    }
    else if (ficheiro == "video2.mp4") {
        if (area >= 2000 && area < 2600) { contador["1c"]++; return "1c"; }
        else if (area >= 2600 && area < 2900) { contador["2c"]++; return "2c"; }
        else if (area >= 2900 && area <= 3720) { contador["5c"]++; return "5c"; }
        else if (area > 3720 && area < 4500) { contador["10c"]++; return "10c"; }
        else if (area >= 4500 && area < 7800) { contador["20c"]++; return "20c"; }
        else if (area >= 7800 && area < 7930) { contador["50c"]++; return "50c"; }
        else if (area >= 7930 && area < 12000) { contador["1euro"]++; return "1€"; }
        else if (area >= 12000 && area < 15000) { contador["2euro"]++; return "2€"; }
    }
    return "X";
}

int main() {
    std::string nomeVideo = "video1.mp4";
    int distMinima = (nomeVideo == "video2.mp4") ? 25 : 15;
    cv::VideoCapture video(nomeVideo);

    if (!video.isOpened()) {
        std::cerr << "Não foi possível abrir o vídeo.\n";
        return 1;
    }

    int largura = static_cast<int>(video.get(cv::CAP_PROP_FRAME_WIDTH));
    int altura = static_cast<int>(video.get(cv::CAP_PROP_FRAME_HEIGHT));

    cv::namedWindow("Resultado", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("Binária", cv::WINDOW_AUTOSIZE);

    mostrarTempo();

    std::ofstream ficheiroCSV("moedas_stats.csv");
    ficheiroCSV << "Tipo,Area,Perimetro,Circularidade,CentroX,CentroY\n";

    std::vector<cv::Point> centrosMoedas;
    std::map<std::string, int> totalPorTipo;
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

        cv::Mat binaria(altura, largura, CV_8UC1, imgBin->data);
        binaria = binaria.clone();

        vc_image_free(imgCinza);
        vc_image_free(imgBin);

        std::vector<std::vector<cv::Point>> contornos;
        cv::findContours(binaria, contornos, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        for (const auto& contorno : contornos) {
            if (contorno.empty()) continue;

            double area = std::fabs(cv::contourArea(contorno));
            double perimetro = cv::arcLength(contorno, true);
            if (area < 2000) continue;

            double circ = 4 * CV_PI * area / (perimetro * perimetro);

            cv::Rect caixa = cv::boundingRect(contorno);
            cv::Point centro(caixa.x + caixa.width / 2, caixa.y + caixa.height / 2);

            bool repetida = false;
            for (const auto& c : centrosMoedas) {
                if (cv::norm(c - centro) < distMinima) {
                    repetida = true;
                    break;
                }

            }
            if (repetida) continue;

            centrosMoedas.push_back(centro);

            draw_rectangle_rgb(imgVC, caixa.x, caixa.y, caixa.width, caixa.height, 0, 255, 0, 2);

            std::string tipo = identificarMoeda(area, nomeVideo, totalPorTipo);

            draw_rectangle_rgb(imgVC, centro.x - 1, centro.y - 1, 3, 3, 255, 0, 0, 1);
            cv::putText(frame, tipo, cv::Point(centro.x - 20, centro.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 2);

            ficheiroCSV << tipo << "," << static_cast<int>(area) << "," << static_cast<int>(perimetro) << "," << circ << "," << centro.x << "," << centro.y << "\n";

            if (tipo != "X") totalMoedas++;
        }

        memcpy(frame.data, imgVC->data, largura * altura * 3);
        vc_image_free(imgVC);

        cv::putText(frame, "Total: " + std::to_string(totalMoedas), cv::Point(20, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 255), 2);
        cv::imshow("Resultado", frame);
        cv::imshow("Binária", binaria);

        tecla = cv::waitKey(100) & 0xFF;

    }

    ficheiroCSV.close();

    std::cout << "\n=== Contagem Final ===\n";
    std::cout << "Moedas encontradas: " << totalMoedas << "\n";
    for (const auto& par : totalPorTipo)
        std::cout << "Moedas de " << par.first << ": " << par.second << "\n";

    mostrarTempo();
    video.release();
    cv::destroyAllWindows();
    return 0;
}
