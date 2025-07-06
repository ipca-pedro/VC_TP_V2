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

/**
 * Função: mostrarTempoProcessamento
 * Descrição: Mostra o tempo decorrido desde a última chamada e aguarda input do utilizador.
 */
void mostrarTempoProcessamento() {
    static bool primeiro_ciclo = true;
    static auto tempo_anterior = std::chrono::steady_clock::now();

    if (primeiro_ciclo) {
        primeiro_ciclo = false;
    }
    else {
        auto tempo_atual = std::chrono::steady_clock::now();
        double segundos_decorridos = std::chrono::duration<double>(tempo_atual - tempo_anterior).count();
        std::cout << "Tempo de processamento: " << segundos_decorridos << " segundos.\n";
        std::cout << "Pressione qualquer tecla para continuar...\n";
        std::cin.get();
    }
}

/**
 * Função: identificarTipoMoeda
 * Descrição: Identifica o tipo de moeda com base na sua área e no vídeo de origem.
 */
std::string identificarTipoMoeda(double area, const std::string& nome_video) {
    std::string tipo_moeda = "Desconhecida";

    // Regras de área para o video1.mp4
    if (nome_video == "video1.mp4") {
        if (area >= 10600 && area < 11400) tipo_moeda = "1c";
        else if (area >= 13800 && area < 14850) tipo_moeda = "2c";
        else if (area >= 17800 && area < 18900) tipo_moeda = "5c";
        else if (area >= 14800 && area < 15800) tipo_moeda = "10c";
        else if (area >= 18500 && area < 20000) tipo_moeda = "20c";
        else if (area >= 23300 && area < 24300) tipo_moeda = "50c";
        else if (area >= 20500 && area < 22300) tipo_moeda = "1euro";
        else if (area >= 26200 && area < 27300) tipo_moeda = "2euro";
    }
    // Regras de área para o video2.mp4
    else if (nome_video == "video2.mp4") {
        if (area >= 10000 && area < 12100) tipo_moeda = "1c";
        else if (area >= 13400 && area < 15090) tipo_moeda = "2c";
        else if (area >= 17100 && area < 19500) tipo_moeda = "5c";
        else if (area >= 15100 && area < 17000) tipo_moeda = "10c";
        else if (area >= 19600 && area < 21900) tipo_moeda = "20c";
        else if (area >= 23700 && area < 26000) tipo_moeda = "50c";
        else if (area >= 22000 && area < 23600) tipo_moeda = "1euro";
        else if (area >= 27000 && area < 28200) tipo_moeda = "2euro";
    }

    return tipo_moeda;
}

/**
 * Função: calcularPropriedadesBlobManualmente
 * Descrição: Calcula a caixa delimitadora e o centroide de um contorno.
 */
void calcularPropriedadesBlobManualmente(const std::vector<cv::Point>& contorno, OVC& info_blob) {
    if (contorno.empty()) return;

    int min_x = contorno[0].x, max_x = contorno[0].x;
    int min_y = contorno[0].y, max_y = contorno[0].y;
    long long soma_x = 0, soma_y = 0;

    for (const auto& ponto : contorno) {
        soma_x += ponto.x;
        soma_y += ponto.y;
        if (ponto.x < min_x) min_x = ponto.x;
        if (ponto.x > max_x) max_x = ponto.x;
        if (ponto.y < min_y) min_y = ponto.y;
        if (ponto.y > max_y) max_y = ponto.y;
    }

    info_blob.x = min_x;
    info_blob.y = min_y;
    info_blob.width = max_x - min_x + 1;
    info_blob.height = max_y - min_y + 1;
    info_blob.xc = static_cast<int>(soma_x / contorno.size());
    info_blob.yc = static_cast<int>(soma_y / contorno.size());
}

/**
 * Função principal (main)
 */
int main() {
    // --- Configurações Iniciais ---
    std::string nome_video = "video2.mp4";
    int limiar_binarizacao;

    if (nome_video == "video1.mp4") {
        limiar_binarizacao = 100;
    }
    else if (nome_video == "video2.mp4") {
        limiar_binarizacao = 120;
    }

    int distancia_minima_tracking = 40;
    cv::VideoCapture video(nome_video);
    if (!video.isOpened()) {
        std::cerr << "Erro: Nao foi possivel abrir o ficheiro de video.\n";
        return 1;
    }

    int largura = static_cast<int>(video.get(cv::CAP_PROP_FRAME_WIDTH));
    int altura = static_cast<int>(video.get(cv::CAP_PROP_FRAME_HEIGHT));

    // --- Criação das Janelas e Início do Temporizador ---
    cv::namedWindow("Resultado Final", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("Imagem Binaria", cv::WINDOW_AUTOSIZE);
    mostrarTempoProcessamento();

    // --- Inicialização de Variáveis para Contagem e Tracking ---
    std::ofstream ficheiro_csv("estatisticas_moedas.csv");
    ficheiro_csv << "TipoMoeda,Area,CentroX,CentroY\n";

    const int linha_de_contagem_y = altura / 3;
    int proximo_id_objeto = 0;
    std::map<int, cv::Point> objetos_rastreados;
    std::map<int, bool> objetos_ja_contados;
    std::map<std::string, int> contagem_por_tipo;
    double valor_total_euros = 0.0;
    int total_moedas_contadas = 0;

    std::vector<std::string> tipos = { "1c", "2c", "5c", "10c", "20c", "50c", "1euro", "2euro" };
    for (const auto& tipo : tipos) contagem_por_tipo[tipo] = 0;

    int tecla_pressionada = 0;
    while (tecla_pressionada != 'q') {
        cv::Mat frame_original;
        video >> frame_original;
        if (frame_original.empty()) break;

        // --- 1. PREPARAÇÃO E PROCESSAMENTO DA IMAGEM ---
        IVC* img_cor = vc_imagem_nova(largura, altura, 3, 255);
        memcpy(img_cor->data, frame_original.data, largura * altura * 3);

        IVC* img_cinza = vc_imagem_nova(largura, altura, 1, 255);
        vc_bgr_para_cinzento(img_cor, img_cinza);

        IVC* img_binaria = vc_imagem_nova(largura, altura, 1, 255);
        IVC* img_temp = vc_imagem_nova(largura, altura, 1, 255);

        vc_cinzento_para_binario(img_cinza, img_binaria, limiar_binarizacao);
        vc_cinzento_negativo(img_binaria);
        vc_binario_abertura(img_binaria, img_binaria, 3, img_temp);
        vc_binario_fecho(img_binaria, img_binaria, 3, img_temp);

        vc_imagem_free(img_temp);

        // --- 2. ANÁLISE DE BLOBS E TRACKING ---
        cv::Mat imagem_binaria_opencv(altura, largura, CV_8UC1, img_binaria->data);
        std::vector<std::vector<cv::Point>> contornos;
        cv::findContours(imagem_binaria_opencv, contornos, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // Vetores para guardar os blobs válidos e as suas propriedades correspondentes
        std::vector<OVC> blobs_validos_frame;
        std::vector<double> areas_validas_frame;
        std::vector<double> circularidades_validas_frame; // NOVO: Vetor para guardar a circularidade

        for (const auto& contorno : contornos) {
            double area = cv::contourArea(contorno);
            if (area < 1500) continue;

            OVC info_blob = { 0 };
            calcularPropriedadesBlobManualmente(contorno, info_blob);

            if (vc_blob_cor_a_descartar(img_cor, &info_blob, nome_video.c_str())) {
                continue;
            }

            /*
            // --- FILTRO DE PROPORÇÃO (MÉTODO ANTIGO, COMENTADO) ---
            float proporcao = (float)info_blob.width / (float)info_blob.height;
            if (proporcao < 0.9f || proporcao > 1.1f) {
                continue;
            }
            */

            // --- FILTRO DE CIRCULARIDADE (MÉTODO ATUAL) ---
            double perimetro = cv::arcLength(contorno, true);
            if (perimetro == 0) continue;
            double circularidade = (4 * 3.14159265359 * area) / (perimetro * perimetro);
            if (circularidade < 0.40) {
                continue;
            }

            // Se o blob passou todos os filtros, guarda todas as suas informações
            blobs_validos_frame.push_back(info_blob);
            areas_validas_frame.push_back(area);
            circularidades_validas_frame.push_back(circularidade); // NOVO: Guarda o valor da circularidade

            // --- LÓGICA DE TRACKING E CONTAGEM ---
            cv::Point centro_atual(info_blob.xc, info_blob.yc);
            int id_associado = -1;
            double menor_distancia = distancia_minima_tracking;

            for (auto const& par : objetos_rastreados) {
                int id = par.first;
                cv::Point pos_anterior = par.second;
                double dist = std::sqrt(std::pow(centro_atual.x - pos_anterior.x, 2) + std::pow(centro_atual.y - pos_anterior.y, 2));

                if (dist < menor_distancia) {
                    menor_distancia = dist;
                    id_associado = id;
                }
            }

            std::string tipo_moeda = identificarTipoMoeda(area, nome_video);

            if (id_associado != -1) {
                cv::Point pos_anterior = objetos_rastreados[id_associado];
                objetos_rastreados[id_associado] = centro_atual;

                if (pos_anterior.y >= linha_de_contagem_y && centro_atual.y < linha_de_contagem_y && !objetos_ja_contados[id_associado]) {
                    total_moedas_contadas++;
                    objetos_ja_contados[id_associado] = true;
                    if (tipo_moeda != "Desconhecida") {
                        contagem_por_tipo[tipo_moeda]++;
                        if (tipo_moeda == "1c") valor_total_euros += 0.01; else if (tipo_moeda == "2c") valor_total_euros += 0.02;
                        else if (tipo_moeda == "5c") valor_total_euros += 0.05; else if (tipo_moeda == "10c") valor_total_euros += 0.10;
                        else if (tipo_moeda == "20c") valor_total_euros += 0.20; else if (tipo_moeda == "50c") valor_total_euros += 0.50;
                        else if (tipo_moeda == "1euro") valor_total_euros += 1.00; else if (tipo_moeda == "2euro") valor_total_euros += 2.00;
                    }
                    ficheiro_csv << tipo_moeda << "," << area << "," << centro_atual.x << "," << centro_atual.y << "\n";
                }
            }
            else {
                if (centro_atual.y > linha_de_contagem_y) {
                    objetos_rastreados[proximo_id_objeto] = centro_atual;
                    objetos_ja_contados[proximo_id_objeto] = false;
                    proximo_id_objeto++;
                }
            }
        } // --- Fim do loop de processamento de contornos ---

        // --- 3. DESENHO DOS RESULTADOS E EXIBIÇÃO ---
        vc_desenha_linha_horizontal(img_cor, linha_de_contagem_y, 255, 0, 0);

        for (const auto& blob : blobs_validos_frame) {
            vc_desenha_caixa_delimitadora(img_cor, (OVC*)&blob);
            vc_desenha_centro_massa(img_cor, (OVC*)&blob, 5);
        }

        memcpy(frame_original.data, img_cor->data, largura * altura * 3);

        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // +++ DESENHAR O TEXTO DAS MOEDAS (COM CIRCULARIDADE) +++
        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        for (size_t i = 0; i < blobs_validos_frame.size(); ++i) {
            OVC blob_info = blobs_validos_frame[i];
            double area = areas_validas_frame[i];
            double circularidade = circularidades_validas_frame[i]; // NOVO: Obtém o valor guardado

            std::string tipo_moeda = identificarTipoMoeda(area, nome_video);

            if (tipo_moeda != "Desconhecida") {
                int x_pos = blob_info.x;
                int y_pos = blob_info.y - 10;
                if (y_pos < 10) y_pos = blob_info.y + blob_info.height + 20;

                // Formata o valor da circularidade para ter 2 casas decimais
                std::stringstream ss;
                ss << std::fixed << std::setprecision(2) << circularidade;
                std::string circ_texto = ss.str();

                // Cria o texto final para depuração
                std::string texto_info = tipo_moeda + " (C:" + circ_texto + ")";

                cv::putText(frame_original, texto_info, cv::Point(x_pos, y_pos), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 2);
                cv::putText(frame_original, texto_info, cv::Point(x_pos, y_pos), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 0), 1);
            }
        }

        // Desenha o painel de informações
        int pos_y_painel = 30;
        for (const auto& par : contagem_por_tipo) {
            std::string texto = par.first + ": " + std::to_string(par.second);
            cv::putText(frame_original, texto, cv::Point(20, pos_y_painel), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 2);
            cv::putText(frame_original, texto, cv::Point(20, pos_y_painel), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
            pos_y_painel += 25;
        }
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << valor_total_euros;
        std::string texto_total = "Total: " + stream.str() + " EUR";
        cv::putText(frame_original, texto_total, cv::Point(20, pos_y_painel + 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame_original, texto_total, cv::Point(20, pos_y_painel + 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 1);

        // Exibe as janelas
        cv::imshow("Resultado Final", frame_original);
        cv::imshow("Imagem Binaria", imagem_binaria_opencv);

        vc_imagem_free(img_cor);
        vc_imagem_free(img_cinza);
        vc_imagem_free(img_binaria);

        // --- Gestão de Input do Utilizador ---
        tecla_pressionada = cv::waitKey(1) & 0xFF;
        if (tecla_pressionada == 'p') {
            while (true) {
                int tecla_pausa = cv::waitKey(0) & 0xFF;
                if (tecla_pausa == 'p' || tecla_pausa == 'q') {
                    tecla_pressionada = tecla_pausa;
                    break;
                }
            }
        }
    } // --- Fim do loop 'while' ---

    // --- Finalização ---
    ficheiro_csv.close();

    std::cout << "\n=== Contagem Final ===\n";
    std::cout << "Total de moedas contadas: " << total_moedas_contadas << "\n";
    for (const auto& par : contagem_por_tipo) {
        std::cout << " - Moedas de " << par.first << ": " << par.second << "\n";
    }
    std::cout << "Valor Total Acumulado: " << std::fixed << std::setprecision(2) << valor_total_euros << " EUR\n";

    mostrarTempoProcessamento();
    video.release();
    cv::destroyAllWindows();
    return 0;
}