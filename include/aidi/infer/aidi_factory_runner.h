#ifndef AIDI_FACTORY_RUNNER_H
#define AIDI_FACTORY_RUNNER_H


#include "common/threadSafeQueue.hpp"
#include "infer/batch_aidi_image.h"
#include "infer/aidi_image.h"
#include "infer/aidi_factory_param_wrapper.h"
#include "infer/aidi_factory_thread.h"
#include "dllexport.h"

namespace aq {
    class DLL_EXPORT AidiFactoryRunner
    {
    public:
        AidiFactoryRunner(std::string check_code="");
        void set_param(aq::FactoryClientParamWrapper param);
        void start();
        void release();
        void set_batch_size(std::vector<int> batch_size);
        void set_test_batch_image(aq::BatchAidiImage batch);
        std::vector<std::string> get_detect_result();
        void get_detect_result(std::vector<aq::BaseDetectResult>& results);
        virtual ~AidiFactoryRunner();
    private:
        ThreadSafeQueue<BatchAidiImage>* task_queue_;
        ThreadSafeQueue<std::vector<aq::BaseDetectResult> >* result_queue_;
        FactoryClientParamWrapper param_;
        AidiFactoryThread* run_thread_;
        std::string check_code_;
        std::vector<int> batch_size_;
        bool have_release_ = false;
        bool thread_start_ = false;
    };

}

#endif // AIDI_FACTORY_RUNNER_H
