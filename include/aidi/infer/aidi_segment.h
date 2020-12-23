#ifndef AIDISEGMENT_H
#define AIDISEGMENT_H

#include "infer/aidi_image.h"
#include "infer/batch_aidi_image.h"
#include "infer/aidi_factory_param_wrapper.h"
#include "client/dnn_segment_client.h"

namespace aq {

    class DLL_EXPORT AidiSegment
    {
    public:
        AidiSegment(const std::string& check_code = "");
        virtual ~AidiSegment();

        void set_param(SegmentClientParamWrapper);

        void set_threshold(float value);
        void set_use_filter(bool use_filter); // must set true if want use filter in two-class.
        void set_filter_area(int min, int max);
        void set_filter_width(int min, int max);
        void set_filter_height(int min, int max);

        // develop, used in multi-classes. not related content in json.
        void set_target_filter_infos(int label_cls, bool use_filter, int area_min, int area_max,
                                     int width_min, int width_max, int height_min, int height_max); 

                                     
        void initial_test_model();
        aq::AidiImage start_test(aq::AidiImage &source_image);
        aq::BatchAidiImage start_test(aq::BatchAidiImage &source_images);

        aq::AidiImage start_test_without_filter(aq::AidiImage& sourde_image); // no filter no mater whatever setting.
        aq::BatchAidiImage start_test_without_filter(aq::BatchAidiImage& source_images);

    private:
        DnnSegmentClient *client_;
    };

}

#endif // AIDISEGMENT_H
