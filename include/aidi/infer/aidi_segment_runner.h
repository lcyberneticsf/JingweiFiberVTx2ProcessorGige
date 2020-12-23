#ifndef AIDI_SEGMENT_RUNNER_H
#define AIDI_SEGMENT_RUNNER_H

#include "common/threadSafeQueue.hpp"
#include "infer/batch_aidi_image.h"
#include "infer/aidi_image.h"
#include "infer/aidi_factory_param_wrapper.h"
#include "infer/aidi_segment_thread.h"

namespace aq {

    class DLL_EXPORT AidiSegmentRunner
    {
    public:
        AidiSegmentRunner(std::string check_code="");
        void set_param(SegmentClientParamWrapper param);

        void set_use_offset_threshold(bool use_offset_threshold); // default false
        void set_use_filter(bool use_filter);  // default false

        void set_threshold(float value);
        void set_filter_area(int min, int max);
        void set_filter_width(int min, int max);
        void set_filter_height(int min, int max);

        void start();
        void release();
        void set_test_batch_image(BatchAidiImage& batch);
        BatchAidiImage get_detect_result();
        virtual ~AidiSegmentRunner();
    private:
        ThreadSafeQueue<BatchAidiImage>* task_queue_;
        ThreadSafeQueue<BatchAidiImage>* result_queue_;
        SegmentClientParamWrapper param_;
        AidiSegmentThread* run_thread_;
        std::string check_code_;

        bool use_offset_threshold_ = false;
        bool use_filter_ = false;
        float threshold_ = 0.5f;
        int filter_min_area_ = 0;
        int filter_max_area_ = 100000;
        int filter_min_width_ = 0;
        int filter_max_width_ = 4000;
        int filter_min_height_ = 0;
        int filter_max_height_ = 4000;
        bool test_fast_without_filter = true;

        bool have_release_ = false;
        bool thread_start_ = false;
    };

}
#endif // AIDI_SEGMENT_RUNNER_H
