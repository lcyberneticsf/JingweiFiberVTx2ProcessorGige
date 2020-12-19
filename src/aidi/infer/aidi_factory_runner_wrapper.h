#ifndef AIDI_FACTORY_RUNNER_WRAPPER_H
#define AIDI_FACTORY_RUNNER_WRAPPER_H
#include "dllexport.h"
#include "infer/aidi_factory_param_wrapper.h"
#include "infer/batch_aidi_image_wrapper.h"

namespace aq {
    struct BaseDetectResult;
    class AidiImage;
    class AidiFactoryRunner;
    class DLL_EXPORT AidiFactoryRunnerWrapper{
    public:
        //! @brief if use hardware dog, do not need auth_code, if use
        //! network dog, need to pass auth_code
        AidiFactoryRunnerWrapper(const char* auth_code = "");
        virtual ~AidiFactoryRunnerWrapper();

        //! @param param json format param
        //! @brief "{\"device_number\":0,\"use_gpu\":true, \"use_runtime\":false, \"save_model_path_list\":
        //! [\"D:/aidi_benchmark/defect_sample_cloth_spot/Detect_0/model\"], \"operator_type_list\":[\"Segment\"]}"
        void set_param(const char* param);

        //! @brief start aidi factory runner
        void start();

        //! @brief destruct aidi factory runner
        void release();

        //! @brief set single image for test
        void set_test_image(aq::AidiImage image);

        //! @brief get json result \n
        //! @param result detect result with json format \n
        //! @brief [{"area":100, "width":100, "height":100, "contours":[{"x":0, "y":0}],
        //! "lx":0, "ly":0, "angle":0, "length":10, "rotate_width":10, "rotate_height":10,
        //! "score":0.5, "type":1, "type_name":"defect", "cx":0, "cy":0}] \n
        //! @brief cx:center_x, cy:center_y, lx:left_top_x, ly:left_top_y \n
        //! @brief width and height:boundingBox width and height \n
        //! @brief rotate_width and rotate_height : minAreaRect width and height \n
        //! @brief length: deprecated
        void get_detect_result(char* &result);

        //! @brief batch_size is the number of images that can be process together
        //! @param batch_size : json format string
        //! @brief [2, 3, 5], corresponding to save_model_path_list in param
        void set_batch_size(const char* batch_size);

        //! @brief set batch images
        void set_test_batch_image(aq::BatchAidiImageWrapper& batch);

        //! @brief only used in vs2013/c#/java/python, struct interface
        void set_param(aq::FactoryClientParamWrapper param);

        //! @brief only used in vs2013/c#/java/python, struct interface
        void set_batch_size(std::vector<int> batch_size);

        //! @brief only used in vs2013/c#/java/python, struct interface
        std::vector<std::string> get_detect_result();

        //! @brief only used in vs2013/c#/java/python, struct interface
        void get_detect_result(std::vector<aq::BaseDetectResult>& results);

    private:
        AidiFactoryRunner* runner_;
    };
}


#endif // AIDI_FACTORY_RUNNER_WRAPPER_H
