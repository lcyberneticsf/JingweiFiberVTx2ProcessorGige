#ifndef AIDICLASSIFY_H
#define AIDICLASSIFY_H

#include "infer/aidi_image.h"
#include "infer/aidi_factory_param_wrapper.h"
#ifndef _TX2_TRT_
#include "client/dnn_classify_client.h"
#else
#include "client/dnn_classify_client_tx2.h"
#endif
namespace aq {

    class DLL_EXPORT AidiClassify
    {
    public:
        AidiClassify(const std::string& check_code = "");
        virtual ~AidiClassify();

        void set_param(ClassifyClientParamWrapper);
        void initial_test_model();

        int start_test(AidiImage &image);

    private:
        DnnClassifyClient *client_;
    };

}

#endif // AIDICLASSIFY_H
