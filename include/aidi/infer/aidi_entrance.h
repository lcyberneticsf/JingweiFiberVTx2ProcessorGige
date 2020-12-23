#ifndef INFER_AIDI_ENTRANCE_H
#define INFER_AIDI_ENTRANCE_H

#include "aidi.h"
#include "aidi_image.h"

//#include <map>
// we use AIDIEntrance just because we need test all modules and we don't want set many path.


namespace aq {
    
    class DLL_EXPORT AIDIEntrance{
    public:
        AIDIEntrance(std::string check_code="");
        virtual ~AIDIEntrance();

        void set_model_root(std::string root_path);
        void initialize();   // default use_gpu=true, and device_num=0
        void initialize(bool use_gpu, int device_num);

        std::string entrance_test_image(AidiImage& image);

#if defined(JAVA_WRAPPER)        
        std::string entrance_test_image_ocr(AidiImage& image, bool is_ocr);
#endif

    protected:

        aq::AIDI* aidi_;
        std::string root_path_;

//        std::string current_path_;
//        int task_id_;
//        static std::map<int, bool> task_ocr_map;

    };



}



#endif // INFER_AIDI_ENTRANCE_H
