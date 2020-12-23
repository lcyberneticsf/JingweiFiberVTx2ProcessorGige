#ifndef AIDI_FACTORY_THREAD_H
#define AIDI_FACTORY_THREAD_H


#include "common/threadSafeQueue.hpp"
#include "common/aidi_internal_thread.h"
#include "infer/batch_aidi_image.h"
#include "infer/aidi_image.h"
#include "infer/aidi_factory_param_wrapper.h"
#include "infer/aidi.h"

namespace aq {
    class AidiFactoryThread : public AidiInternalThread{
    public:
        AidiFactoryThread(std::string check_code="");
        virtual ~AidiFactoryThread();
        void set_param(FactoryClientParamWrapper param);
        void set_batch_size(std::vector<int> batch_size);
        void set_task_queue(ThreadSafeQueue<BatchAidiImage>* task_queue);
        void set_result_queue(ThreadSafeQueue<std::vector<aq::BaseDetectResult> >* result_queue);
        virtual void run();
    private:
        ThreadSafeQueue<BatchAidiImage>* task_queue_;
        ThreadSafeQueue<std::vector<aq::BaseDetectResult>>* result_queue_;
        std::string check_code_;
        FactoryClientParamWrapper param_;
        std::vector<int> batch_size_;
        AIDI* client_;

        void setup();
    };

}
#endif // AIDI_FACTORY_THREAD_H
