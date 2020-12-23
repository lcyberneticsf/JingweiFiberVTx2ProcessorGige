#ifndef IMAGE_H
#define IMAGE_H

#include "opencv2/opencv.hpp"
#include <string>
#include "dllexport.h"

#ifdef PYTHON_WRAPPER
#include "common/conversion.h"
#endif

#define DEPRECATED_WARNING(message) do{ \
    std::cout << "[WARNING]: " << #message << "\n" \
    << __FUNCTION__ << " will be deprecated in future version," \
    << "please do not use it." << std::endl; \
}while(0)

#define REPLACE_WARNING(replace) do{ \
    std::cout << "[WARNING]: " << __FUNCTION__ << " was deprecated and replaced by " <<#replace \
    << ". please use " << #replace << " directly." << std::endl; \
    return replace; \
}while(0)

namespace aq {

class DLL_EXPORT AidiImage : public cv::Mat
{
public:
    AidiImage(): cv::Mat(){}
    AidiImage(const cv::Mat& mat): cv::Mat(mat){}
    AidiImage(int rows, int cols, int type) :cv::Mat(rows, cols, type){}
    AidiImage(int rows, int cols, int type, char *data, size_t step): cv::Mat(rows, cols, type, (void*)data, step){}
    virtual ~AidiImage(){}

    //! @brief get image from cv::Mat
    void from_mat(const cv::Mat& mat);

    //! @brief get cv::Mat from AidiImage
    cv::Mat& to_mat();

    //! @brief read image from file, the same as cv::imread
    void from_file(std::string path, int flag = -1);
    void from_file(const char* path, int flag = -1);

    //! @brief save image to file, the same as cv::imwrite
    bool to_file(std::string path);
    bool to_file(const char* path);

    //!
    void decode(std::string data, int flag= -1);

    //!
    std::string encode(std::string format);

    //! @brief render image, the same as cv::imshow and cv::waitKey(...)
    void show(int wait_time, std::string winname = "");
    void show(int wait_time, const char* winname);

#ifdef PYTHON_WRAPPER
    AidiImage(PyObject* np): cv::Mat(NDArrayConverter().toMat(np)){}
    void from_numpy(PyObject* np);
    PyObject* to_numpy();
#endif

    //! @brief set image form char*
    //! data range in HWC, uint8
    void from_chars(char *inbuf, int height, int width, int channels);

    //! @brief get char* from AidiImage
    void to_chars(char* outbuf, size_t len);

    //! @brief set image from vector<int>
    void from_IntVector(std::vector<int>& data, int height, int width, int channels = 1);

    //! @brief get vector<int> from AidiImage
    std::vector<int> to_IntVector();

    //! @brief set image from vector<float>, float32
    void from_FloatVector(std::vector<float>& data , int height, int width, int channels = 1);

    //! @brief get vector<float> from AidiImage, float32
    std::vector<float> to_FloatVector();

    //! @brief set image from string, the same as from char* because data.data() is char*
    void from_string(std::string& data, int rows, int cols, int channels = 1);

    //! @brief get string from AidiImage
    std::string to_string();

    //! @brief render result image
    //! @param result, get result by calling Aidi/AidiFactoryRunner/AidiFacoryRunnerWrapper.get_detect_result()
    //! draw_result can draw rect/contours/area/width/height to source image
    //! after draw_result, just call show(0) can see render result image
    bool draw_result(std::string result);
    bool draw_result(char* result);

    //! @brief judge whether AidiImage is empty
    bool empty() {return cv::Mat::empty();}

    //! @brief get image size
    int data_size();

    //! @brief get image byte size
    int data_byte_size();

    AidiImage get_region(const int xmin, const int ymin, const int width, const int height);

    //! @brief get image height
    int height() {return cv::Mat::rows;}

     //! @brief get image width
    int width() {return cv::Mat::cols;}

    //! @brief get image channels
    int channels() {return cv::Mat::channels();}

    //! @brief destruct AidiImage
    void release() {cv::Mat::release();}

    ///deprecated
    void get_mat(cv::Mat& image){
        DEPRECATED_WARNING("As subclass of cv::Mat, AidiImage can be convert to " \
                           "cv::Mat automatically now, \n or you can use to_mat().");
        image = to_mat();
    }

    ///deprecated
    cv::Mat& get_mat_const(){
        REPLACE_WARNING(to_mat());
    }
    ///deprecated
    void set_mat(const cv::Mat &image){
        DEPRECATED_WARNING("Suggest use constructor or from_mat to convert Mat to AidiImage. "
                           "just like 'AidiImage(mat)'or 'from_mat(mat)'.");
        (*this) = AidiImage(image);
    }
    ///deprecated
    int channel(){
        REPLACE_WARNING(channels());
    }
#ifdef PYTHON_WRAPPER
    ///deprecated
    void set_image_from_numpy(PyObject* pyobject){
        REPLACE_WARNING(from_numpy(pyobject));
    }
#endif
#if defined(PYTHON_WRAPPER) || defined(CSHARP_WRAPPER)
    ///deprecated
    void mat_to_char_ptr(char *outbuf, size_t len){
        REPLACE_WARNING(to_chars(outbuf, len));
    }
    ///deprecated
    void char_ptr_to_mat(char* inbuf, int height, int width, int channel){
        REPLACE_WARNING(from_chars(inbuf, height, width, channel));
    }
#endif
#if  defined(JAVA_WRAPPER)
    ///deprecated
    void png_data_to_mat(char* inbuf, size_t len){
        std::string buf(inbuf, len);
        REPLACE_WARNING(decode(buf, CV_LOAD_IMAGE_COLOR));
    }
    ///deprecated
    int png_data_size(){
        DEPRECATED_WARNING("this function was not needed in new version with alternative method encode()");
        std::vector<uchar> png_data;
        cv::imencode(".png", *this, png_data);
        return png_data.size();
    }
    ///deprecated
    void mat_to_png_data(char* outbuf, size_t len){
        DEPRECATED_WARNING("this function was not needed in new version with alternative method encode()");
        std::vector<uchar> png_data;
        cv::imencode(".png", *this, png_data);
        len = png_data.size();
        memcpy((void*)outbuf, (void*)png_data.data(), len);
    }
#endif
    ///deprecated
    void vector_int_to_mat(std::vector<int> data, int rows, int cols){
        REPLACE_WARNING(from_IntVector(data, rows, cols));
    }
    ///deprecated
    void vector_float_to_mat(std::vector<float> data, int rows, int cols){
        REPLACE_WARNING(from_FloatVector(data, rows, cols));
    }
    ///deprecated
    void mat_to_vector_int(std::vector<int>& data){
        data = [this](){REPLACE_WARNING(to_IntVector());}();
    }
    ///deprecated
    void mat_to_vector_float(std::vector<float>& data){
        data = [this](){REPLACE_WARNING(to_FloatVector());}();
    }
    ///deprecated
    void read_image(const std::string& path, int flag = 1){
        REPLACE_WARNING(from_file(path, flag));
    }
    ///deprecated
    void save_image(std::string path){
        [this](std::string file_path){REPLACE_WARNING(to_file(file_path));}(path);
    }
    ///deprecated
    void show_image(int wait_time){
        REPLACE_WARNING(show(wait_time, ""));
    }


};


}//namespace aq
#endif // IMAGE_H
