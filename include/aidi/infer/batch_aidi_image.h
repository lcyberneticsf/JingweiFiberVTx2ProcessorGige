#ifndef BATCH_AIDI_IMAGE_H
#define BATCH_AIDI_IMAGE_H

#include <opencv2/opencv.hpp>
#include <infer/aidi_image.h>

namespace aq {

    class DLL_EXPORT BatchAidiImage
    {
    public:
        BatchAidiImage();
        virtual ~BatchAidiImage();

        //! @brief add AidiImage to BatchAidiImage
        void add_image(AidiImage& image);
        void add_image(const char* image_path);
        void add_image(char *inbuf, int height, int width, int channels);
        void clear_images();

        //! @brief display add images
        //! @param wait_time window display time, 0 for infinite
        //! the same as cv::waitKey()
        void show_all_images(int wait_time);

        //! @brief get all images size
        int get_images_size();

        //! @brief get all images in vector<Mat>
        void get_all_images(std::vector<cv::Mat>& all_images);

        //! @brief set vector<Mat> to BatchAidiImage
        void set_image_list(std::vector<cv::Mat> images);

        //! @brief get single image by index
        AidiImage at(int index);
    private:
        std::vector<AidiImage> image_list_;
    };

}
#endif // BATCH_AIDI_IMAGE_H
