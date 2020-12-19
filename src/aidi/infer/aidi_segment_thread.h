#ifndef AIDI_SEGMENT_THREAD_H
#define AIDI_SEGMENT_THREAD_H

#include "common/threadSafeQueue.hpp"
#include "common/aidi_internal_thread.h"
#include "infer/batch_aidi_image.h"
#include "infer/aidi_image.h"
#include "infer/aidi_factory_param_wrapper.h"
#include "infer/aidi_segment.h"

namespace aq {

    class AidiSegmentThread : public AidiInternalThread{
    public:
        AidiSegmentThread(std::string check_code="");
        virtual ~AidiSegmentThread();
        void set_param(SegmentClientParamWrapper param);

        void set_use_offset_threshold(bool use_offset_threshold); // default false
        void set_use_filter(bool use_filter);  // default false

        void set_threshold(float value);
        void set_filter_area(int min, int max);
        void set_filter_width(int min, int max);
        void set_filter_height(int min, int max);

        void set_task_queue(ThreadSafeQueue<BatchAidiImage>* task_queue);
        void set_result_queue(ThreadSafeQueue<BatchAidiImage>* result_queue);
        virtual void run();
    private:
        ThreadSafeQueue<BatchAidiImage>* task_queue_;
        ThreadSafeQueue<BatchAidiImage>* result_queue_;
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

        SegmentClientParamWrapper param_;
        AidiSegment* client_;

        void setup();
        void run_with_filter();
        void run_without_filter();

    };

}
#endif // AIDI_SEGMENT_THREAD_H
