#ifndef INFER_AIDI_H
#define INFER_AIDI_H

#include "infer/aidi_image.h"
#include "infer/aidi_factory_param_wrapper.h"
#include "infer/batch_aidi_image.h"
#include "client/dnn_client_struct.h"


namespace aq{
    class DnnFactoryClient;
    class DLL_EXPORT AIDI{
	public:
        AIDI(std::string check_code="");
        virtual ~AIDI();
        void set_param(aq::FactoryClientParamWrapper);
        void initial_test_model();
        void set_batch_size(std::vector<int> batch_size);

        std::string start_test(aq::AidiImage image);
        void start_test(aq::AidiImage image, aq::BaseDetectResult& reuslt);
        std::vector<std::string> start_test(aq::BatchAidiImage batch_images);
        void start_test(aq::BatchAidiImage batch_images, std::vector<aq::BaseDetectResult>& results);

#if defined(PYTHON_WRAPPER)  || defined(CSHARP_WRAPPER)        
		std::string start_test(uchar* data, int width, int height);
#endif

#if  defined(JAVA_WRAPPER)
        std::string start_test(char* inbuf, size_t len, int height, int width);

        std::string start_test_ocr(AidiImage& image, bool is_ocr);
        std::string start_test_ocr(char* inbuf, size_t len, int height, int width, bool is_ocr);

#endif   // end of python, csharp and java wrapper

	private:
		DnnFactoryClient* client_;
        bool flag_ = false;
//        std::string transform_defects_json(std::vector<aq::BaseDefect> defects);
	};

}

#endif  // INFER_AIDI_H


