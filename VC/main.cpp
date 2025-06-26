#include "header.h"
#include <iostream>
#include <fstream>
#include <map>
#include <cmath>

std::vector<CoinInfo> global_coins;

int main() {
    std::string video_file = "video1.mp4";
    cv::VideoCapture capture(video_file);

    if (!capture.isOpened()) {
        std::cerr << "Erro ao abrir vídeo\n";
        return -1;
    }

    cv::namedWindow("Resultado", cv::WINDOW_AUTOSIZE);
    std::ofstream output("estatisticas_moedas.csv");
    output << "Tipo,Area,Perimetro,CentroX,CentroY\n";

    cv::Mat frame;
    int frame_count = 0;

    while (capture.read(frame)) {
        if (frame.empty()) break;

        frame_count++;

        if (frame_count % 10 == 0) {
            std::vector<CoinInfo> coins;
            vc_process_frame(frame, coins, video_file);

            for (size_t i = 0; i < coins.size(); i++) {
                if (coins[i].type != "X") {
                    bool is_new = true;
                    for (size_t j = 0; j < global_coins.size(); j++) {
                        double distance = std::sqrt(
                            std::pow((double)(coins[i].centerX - global_coins[j].centerX), 2) +
                            std::pow((double)(coins[i].centerY - global_coins[j].centerY), 2)
                        );
                        if (distance < 50) {
                            is_new = false;
                            break;
                        }
                    }

                    if (is_new) {
                        global_coins.push_back(coins[i]);
                        output << coins[i].type << "," << coins[i].area << ","
                            << coins[i].perimeter << "," << coins[i].centerX << ","
                            << coins[i].centerY << "\n";
                    }
                }
            }
        }

        vc_draw_results(frame, global_coins);

        std::map<std::string, int> current_counts;
        for (size_t i = 0; i < global_coins.size(); i++) {
            if (global_coins[i].type != "X") {
                current_counts[global_coins[i].type]++;
            }
        }

        int total = 0;
        for (std::map<std::string, int>::const_iterator it = current_counts.begin();
            it != current_counts.end(); ++it) {
            total += it->second;
        }

        cv::putText(frame, "Total: " + std::to_string(total),
            cv::Point(20, 30), cv::FONT_HERSHEY_SIMPLEX,
            0.8, cv::Scalar(0, 255, 255), 2);

        cv::imshow("Resultado", frame);
        if (cv::waitKey(100) == 'q') break;
    }

    output.close();

    std::cout << "\n===== Estatísticas Finais =====\n";
    std::map<std::string, int> current_counts;
    for (size_t i = 0; i < global_coins.size(); i++) {
        if (global_coins[i].type != "X") {
            current_counts[global_coins[i].type]++;
        }
    }

    for (std::map<std::string, int>::const_iterator it = current_counts.begin();
        it != current_counts.end(); ++it) {
        std::cout << "Moedas de " << it->first << ": " << it->second << "\n";
    }

    capture.release();
    cv::destroyAllWindows();
    return 0;
}
