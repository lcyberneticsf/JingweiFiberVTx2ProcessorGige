#ifndef DNN_CLIENT_STRUCT_H
#define DNN_CLIENT_STRUCT_H

#include "dllexport.h"
#include "opencv2/opencv.hpp"

namespace aq {
    struct BaseDefect{
        cv::Size2f size;
        cv::Size2f rotate_size;
        float angle = 0.0;
        cv::Point2f center;
        cv::Point tl;// tl for location
        double area = 0;
        int type = -1;
        std::string type_name = "";  // classify class_name
        float score = 0.0;
        std::vector<cv::Point> contours;  // detect result
        std::string rect_index = "";
        double length = 0.0; // defects length
    };

    struct DLL_EXPORT BaseDetectResult{
        cv::Mat source_image; // source image
        cv::Mat result_image; // render result image, do not render now
        std::vector<BaseDefect> defects; // defect vector
        std::string to_json();
        void from_json(const std::string json_str);
    };

    // json to vector<BaseDetectResult>
    void DLL_EXPORT all_from_json(const std::string& json, std::vector<BaseDetectResult>& results);

    // OCRYesResult is for mixture tasks which may be binary-class or ocr recognization.
    // binary-class: image with rect, score, class_type_name
    // ocr: ocrstr, score
    struct OCRYesResult{
        bool is_ocr_task = true;
        bool keep_image = false;
        int result_size = 0;

        std::string ocr_str;
        std::string date_time_str;
        int class_num;
        std::string class_type_name;
        float score;
        cv::Mat result_img;
    };


    enum PostDealType{
        POST_DEAL_NMS,
        POST_DEAL_SOFT_NMS
    };


}
#endif // DNN_CLIENT_STRUCT_H
