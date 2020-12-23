#ifndef BATCH_AIDI_IMAGE_WRAPPER_H
#define BATCH_AIDI_IMAGE_WRAPPER_H

#include "dllexport.h"


namespace aq {
    class AidiImage;
    class BatchAidiImage;
    class DLL_EXPORT BatchAidiImageWrapper{
    public:
        BatchAidiImageWrapper();
        virtual ~BatchAidiImageWrapper();
        void add_image(AidiImage image);
        void add_image(const char* image_path);

        //! @brief set image form char*
        //! data range in HWC, uint8
        void add_image(char *inbuf, int height, int width, int channels);
        void clear_images();
        void show_all_images(int wait_time);
        int get_images_size();
        BatchAidiImage get_batch_image();
    private:
        BatchAidiImage* batch_image_;
    };

}


#endif // BATCH_AIDI_IMAGE_WRAPPER_H
