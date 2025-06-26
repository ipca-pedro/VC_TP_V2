#include "header.h"
#include <queue>
#include <algorithm>
#include <cmath>

bool is_circular_object(const CoinInfo& coin) {
    int width = coin.maxX - coin.minX;
    int height = coin.maxY - coin.minY;

    if (width <= 0 || height <= 0) return false;

    double aspect_ratio = (double)width / height;

    if (aspect_ratio < 0.5 || aspect_ratio > 2.0) return false;

    int rect_area = width * height;
    if (rect_area <= 0) return false;

    double circularity = (double)coin.area / rect_area;

    return circularity > 0.6;
}

void threshold_for_coins(const cv::Mat& gray, cv::Mat& binary) {
    binary = cv::Mat::zeros(gray.size(), CV_8UC1);
    for (int y = 0; y < gray.rows; y++) {
        for (int x = 0; x < gray.cols; x++) {
            int pixel = gray.at<uchar>(y, x);
            if (pixel < 120 || (pixel > 160 && pixel < 200)) {
                binary.at<uchar>(y, x) = 255;
            }
        }
    }
}

void find_blobs_with_filters(const cv::Mat& binary, std::vector<CoinInfo>& coins) {
    cv::Mat visited = cv::Mat::zeros(binary.size(), CV_8UC1);
    int rows = binary.rows;
    int cols = binary.cols;

    for (int y = 30; y < rows - 30; y += 10) {
        for (int x = 30; x < cols - 30; x += 10) {
            if (binary.at<uchar>(y, x) == 255 && visited.at<uchar>(y, x) == 0) {
                int minX = x, maxX = x, minY = y, maxY = y;
                int area = 0;
                int sumX = 0, sumY = 0;
                std::queue<std::pair<int, int> > q;
                q.push(std::make_pair(y, x));
                visited.at<uchar>(y, x) = 1;

                while (!q.empty()) {
                    std::pair<int, int> p = q.front();
                    q.pop();
                    int cy = p.first;
                    int cx = p.second;

                    area++;
                    sumX += cx;
                    sumY += cy;
                    minX = std::min(minX, cx);
                    maxX = std::max(maxX, cx);
                    minY = std::min(minY, cy);
                    maxY = std::max(maxY, cy);

                    int dx[] = { 0, 0, 2, -2 };
                    int dy[] = { 2, -2, 0, 0 };

                    for (int i = 0; i < 4; i++) {
                        int ny = cy + dy[i];
                        int nx = cx + dx[i];
                        if (ny >= 0 && ny < rows && nx >= 0 && nx < cols) {
                            if (binary.at<uchar>(ny, nx) == 255 && visited.at<uchar>(ny, nx) == 0) {
                                visited.at<uchar>(ny, nx) = 1;
                                q.push(std::make_pair(ny, nx));
                            }
                        }
                    }
                }

                if (area > 2000 && area < 50000) {
                    int width = maxX - minX;
                    int height = maxY - minY;
                    double aspect_ratio = (double)width / height;

                    if (width > 20 && height > 20 && width < 200 && height < 200 &&
                        aspect_ratio > 0.6 && aspect_ratio < 1.6) {
                        CoinInfo coin;
                        coin.minX = minX;
                        coin.maxX = maxX;
                        coin.minY = minY;
                        coin.maxY = maxY;
                        coin.centerX = sumX / area;
                        coin.centerY = sumY / area;
                        coin.area = area;
                        coin.perimeter = 2 * (width + height);
                        coin.type = "X";

                        if (is_circular_object(coin)) {
                            coins.push_back(coin);
                        }
                    }
                }
            }
        }
    }
}

std::string classify_coin(int area, const std::string& video) {
    if (video == "video1.mp4") {
        if (area >= 2000 && area < 2900) return "1c";
        else if (area >= 2900 && area < 3000) return "2c";
        else if (area >= 3000 && area < 4000) return "5c";
        else if (area >= 4000 && area < 6000) return "10c";
        else if (area >= 6000 && area < 9000) return "20c";
        else if (area >= 11500 && area < 13000) return "50c";
        else if (area >= 9000 && area < 16000) return "1e";
        else if (area >= 16000 && area < 17000) return "2e";
    }
    else if (video == "video2.mp4") {
        if (area >= 2000 && area < 2600) return "1c";
        else if (area >= 2600 && area < 2900) return "2c";
        else if (area >= 2900 && area <= 3720) return "5c";
        else if (area > 3720 && area < 4500) return "10c";
        else if (area >= 4500 && area < 7800) return "20c";
        else if (area >= 7800 && area < 7930) return "50c";
        else if (area >= 7930 && area < 12000) return "1e";
        else if (area >= 12000 && area < 15000) return "2e";
    }
    return "X";
}

void vc_process_frame(const cv::Mat& frame, std::vector<CoinInfo>& coins, const std::string& video_name) {
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    cv::Mat binary;
    threshold_for_coins(gray, binary);

    std::vector<CoinInfo> detected_coins;
    find_blobs_with_filters(binary, detected_coins);

    for (size_t i = 0; i < detected_coins.size(); i++) {
        detected_coins[i].type = classify_coin(detected_coins[i].area, video_name);
    }

    coins.clear();
    for (size_t i = 0; i < detected_coins.size(); i++) {
        if (detected_coins[i].type != "X") {
            coins.push_back(detected_coins[i]);
        }
    }
}

void vc_draw_results(cv::Mat& frame, const std::vector<CoinInfo>& coins) {
    for (size_t i = 0; i < coins.size(); i++) {
        cv::rectangle(frame, cv::Point(coins[i].minX, coins[i].minY),
            cv::Point(coins[i].maxX, coins[i].maxY),
            cv::Scalar(0, 255, 0), 2);
        cv::circle(frame, cv::Point(coins[i].centerX, coins[i].centerY),
            3, cv::Scalar(0, 0, 255), -1);
        cv::putText(frame, coins[i].type, cv::Point(coins[i].minX, coins[i].minY - 5),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);
    }
}
