#ifndef AIDICALCULATION_H
#define AIDICALCULATION_H

#include "aidi_calculation_result_wrapper.h"
#include <common/calculation.h>
#include <string>
#include <vector>
#include <map>

namespace aq{

    class DLL_EXPORT AidiCalculation{
    public:
        AidiCalculation();
        virtual ~AidiCalculation();
        void setIou_thresh(float value);
        void set_image_format(std::string format=".bmp");

        LocationResultWrapper get_location_result(std::string label_file_path, std::string test_result_file_path, int sample_number);
        LocationResultWrapper get_location_result(std::string label_file_path, std::string test_result_file_path, std::vector<int> indexs);

        SegmentResultWrapper get_detect_result(std::string label_file_path, std::string test_result_file_path, int sample_number);
        SegmentResultWrapper get_detect_result(std::string label_file_path, std::string test_result_file_path, std::vector<int> indexs);

        ClassifyResultWrapper get_classify_result(std::string label_file_path, std::string test_result_file_path, int sample_number);
        ClassifyResultWrapper get_classify_result(std::string label_file_path, std::string test_result_file_path, std::vector<int> indexs);

        FeatureLocationResultWrapper get_feature_location_result(std::string label_file_path, std::string test_result_file_path, std::map<std::string, int> label_dict, std::vector<int> indexs);
        DetectionResultWrapper get_detection_result(std::string label_file_path, std::string test_result_file_path, std::vector<int> indexs, std::vector<std::string> label_name_list);
        DetectionResultWrapper get_detection_result_normal(std::string label_file_path, std::string test_result_file_path, std::vector<int> indexs);

    private:
        aq::Calculation *cal_;
    };

}

#endif
