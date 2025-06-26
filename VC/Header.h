#ifndef VC_H
#define VC_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

struct CoinInfo {
    int minX = 0;
    int minY = 0;
    int maxX = 0;
    int maxY = 0;
    int centerX = 0;
    int centerY = 0;
    int area = 0;
    int perimeter = 0;
    std::string type = "";
};

extern std::vector<CoinInfo> global_coins;

void vc_process_frame(const cv::Mat& frame, std::vector<CoinInfo>& coins, const std::string& video_name);
void vc_draw_results(cv::Mat& frame, const std::vector<CoinInfo>& coins);

#endif
