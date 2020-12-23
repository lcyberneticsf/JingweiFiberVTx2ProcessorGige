#ifndef INFER_AIDI_FACTORY_CLIENT_PARAM_WRAPPER_H
#define INFER_AIDI_FACTORY_CLIENT_PARAM_WRAPPER_H

#include <string>
#include <vector>


namespace aq{

    struct FactoryClientParamWrapper{
        std::vector<std::string> save_model_path_list;
        std::vector<std::string> operator_name_list;
        std::vector<std::string> operator_type_list;
        int device_number = 0;
        bool use_gpu = true;
        bool use_runtime = false;

    };

    struct ClassifyClientParamWrapper{
        int batch_size = 1;
        bool use_gpu = true;

        int device_number = 0;
        std::string save_model_path = "";

        bool use_runtime = false;
        bool encrypt = true;
        std::string model_path = "";
        bool force_set_gpu = false;
    };

    struct LocationClientParamWrapper{

        std::string model_path = "";
        std::string deploy = "";
        std::string save_model_path = "";
        bool save_model_path_defined = false;

        //params about images
        int test_input_shape_w = 320;        //[h, w, c]
        int test_input_shape_h = 320;        //[h, w, c]
        int test_input_shape_c = 3;        //[h, w, c]
        int test_input_shape = 320;
        int batch_size = 1;

        bool use_gpu = true;
        int device_number = 0;
        bool test_model_initialized_ = false;
        bool long_detection = false;
        int split = 1;
        int split_w = 1;
        int split_h = 1;
        bool encrypt = true;
        bool use_runtime = false;

        bool force_set_gpu = false;

    };

    struct SegmentClientParamWrapper {
        int device_number;
        bool use_gpu;
        std::string save_model_path;
        int batch_size;
        bool use_runtime = false;
        bool encrypt = true;
        std::string model_path = "";
        bool force_set_gpu = false;
    };

}

#endif // INFER_AIDI_FACTORY_CLIENT_PARAM_WRAPPER_H
