#ifndef AIDI_CALCULATION_RESULT_WRAPPER_H
#define AIDI_CALCULATION_RESULT_WRAPPER_H

#include <vector>

namespace aq{

    struct LocationResultWrapper{
        double precise;
        double recall;
        int all_gt_num;
        int all_pred_num;
        int recall_num;
        int precise_num;
    };

    struct SegmentResultWrapper{
        double recall;
        double ap;
        double center_recall;
        double center_ap;
        long long int all_gt_num;
        long long int recall_num;
        long long int all_center_num;
        long long int center_recall_num;
    };

    struct ClassifyResultWrapper{
        double acc;
        double recall;
    };

    struct FeatureLocationResultWrapper{
        double ap;
        double recall;
        double position_ap;
        double position_recall;
    };

    struct DetectionResultWrapper{
        std::vector<float> precise;
        std::vector<float> recall;
        std::vector<float> ap;
        double mAP ;
        float rect_recall;
        float rect_precise;
        float rect_loc_recall;
        float rect_loc_precise;
        int all_gt_num;
        int all_pred_num;
     };

}

#endif // AIDI_CALCULATION_RESULT_WRAPPER_H
