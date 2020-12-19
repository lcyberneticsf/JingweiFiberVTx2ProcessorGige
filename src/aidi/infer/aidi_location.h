#ifndef AIDILOCATION_H
#define AIDILOCATION_H

#include "infer/aidi_image.h"
#include "infer/batch_aidi_image.h"
#include "infer/aidi_factory_param_wrapper.h"
#include "client/dnn_location_client.h"

namespace aq {

    class DLL_EXPORT AidiLocation
    {
    public:
        AidiLocation(const std::string& check_code = "");
        virtual ~AidiLocation();

        void set_test_data(AidiImage &test_source_image);
        bool set_param(LocationClientParamWrapper param);

        void set_score_threshold(float thresh);
        void set_nms_threshold(float thresh);
        void set_test_batches(int n_batches);

        void initialize_test_model();
        void start_test();
        std::vector<float> get_test_rect();

    private:
        DnnLocationClient *client_;
    };

}

#endif // AIDILOCATION_H
